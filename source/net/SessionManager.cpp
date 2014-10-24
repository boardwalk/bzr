/*
 * Bael'Zharon's Respite
 * Copyright (C) 2014 Daniel Skorupski
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include "net/SessionManager.h"
#include "Config.h"
#include "Core.h"
#include "Log.h"
#include <algorithm>

static const chrono::microseconds kMaxTimeout = chrono::seconds(1);

template<class Mutex>
class unlock_guard
{
public:
    unlock_guard(Mutex& mutex) : mutex_(mutex)
    {
        mutex_.unlock();
    }

    ~unlock_guard()
    {
        mutex_.lock();
    }

    unlock_guard& operator=(const unlock_guard&) = delete;

private:
    Mutex& mutex_;
};

SessionManager::SessionManager() :
    done_(false),
    clientBegin_(net_clock::now()),
    thread_(bind(&SessionManager::run, this))
{
    Config& config = Core::get().config();

    int serverIp = config.getInt("SessionManager.serverIp", 0);
    int serverPort = config.getInt("SessionManager.serverPort", 0);
    string accountName = config.getString("SessionManager.accountName", "");
    string accountKey = config.getString("SessionManager.accountTicket", "");

    config.erase("SessionManager");

    if(serverIp == 0)
    {
        LOG(Net, Warn) << "no login info in configuration\n";
        return;
    }

    Address address(serverIp, static_cast<uint16_t>(serverPort));

    lock_guard<mutex> lock(mutex_);
    unique_ptr<Session> session(new Session(*this, address, move(accountName), move(accountKey)));
    sessions_.push_back(move(session));
}

SessionManager::~SessionManager()
{
    {
        lock_guard<mutex> lock(mutex_);
        done_ = true;
    }

    thread_.join();
}

void SessionManager::handleBlobs()
{
    vector<BlobPtr> blobs;

    {
        lock_guard<mutex> lock(mutex_);

        for(unique_ptr<Session>& session : sessions_)
        {
            session->blobAssembler().getBlobs(blobs);

            if(!blobs.empty())
            {
                break;
            }
        }
    }

    for(BlobPtr& blob : blobs)
    {
        blobHandler_.handle(move(blob));
    }
}

void SessionManager::add(unique_ptr<Session> session)
{
    sessions_.push_back(move(session));
}

bool SessionManager::exists(Address address) const
{
    for(const unique_ptr<Session>& session : sessions_)
    {
        if(session->address() == address)
        {
            return true;
        }
    }

    return false;
}

void SessionManager::setPrimary(Session*)
{
    // FIXME
}

void SessionManager::send(const Packet& packet)
{
    socket_.send(packet);
}

net_time_point SessionManager::getClientBegin() const
{
    return clientBegin_;
}

void SessionManager::run()
{
    lock_guard<mutex> lock(mutex_);

    while(!done_)
    {
        bool readable;

        {
            unlock_guard<mutex> unlock(mutex_);
            readable = socket_.wait(getReadTimeout());
        }

        if(readable)
        {
            Packet packet;

            while(socket_.recv(packet))
            {
                handle(packet);
            }
        }

        tick();
    }
}

void SessionManager::handle(const Packet& packet)
{
    auto it = sessions_.begin();

    for(/**/; it != sessions_.end(); ++it)
    {
        if((*it)->address() == packet.address)
        {
            break;
        }
    }

    if(it == sessions_.end())
    {
        LOG(Net, Warn) << packet.address << " packet matches no session\n";
        return;
    }

    try
    {
        (*it)->handle(packet);
    }
    catch(const runtime_error& e)
    {
        LOG(Net, Error) << (*it)->address() << " threw an error: " << e.what() << "\n";
        sessions_.erase(it);
    }
}

void SessionManager::tick()
{
    net_time_point now = net_clock::now();

    for(auto it = sessions_.begin(); it != sessions_.end(); /**/)
    {
        try
        {
            (*it)->tick(now);
        }
        catch(const runtime_error& e)
        {
            LOG(Net, Error) << (*it)->address() << " threw an error: " << e.what() << "\n";
            it = sessions_.erase(it);
            continue;
        }

        ++it;
    }
}

chrono::microseconds SessionManager::getReadTimeout() const
{
    net_time_point now = net_clock::now();
    net_time_point nextTick = net_time_point::max();

    for(const unique_ptr<Session>& session : sessions_)
    {
        nextTick = min(nextTick, session->nextTick());
    }

    if(nextTick <= now)
    {
        // don't use a negative or zero or timeout, which means "wait forever"
        return chrono::microseconds(1);
    }

    // don't use a timeout bigger than kMaxTimeout, so we keep checking values of done_
    return min(chrono::duration_cast<chrono::microseconds>(nextTick - now), kMaxTimeout);
}
