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
    primary_(nullptr),
    clientBegin_(net_clock::now()),
    thread_(bind(&SessionManager::run, this))
{}

void SessionManager::addLocked(unique_ptr<Session> session)
{
    lock_guard<mutex> lock(mutex_);

    if(primary_ == nullptr)
    {
        primary_ = session.get();
    }

    sessions_.push_back(move(session));
}

void SessionManager::stop()
{
    {
        lock_guard<mutex> lock(mutex_);
        done_ = true;
    }

    thread_.join();
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

void SessionManager::setPrimary(Session* primary)
{
    primary_ = primary;
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
            socket_.recv(packet);

            while(packet.address.ip() != 0)
            {
                handle(packet);
                socket_.recv(packet);
            }
        }

        tick();
    }
}

void SessionManager::handle(const Packet& packet)
{
    for(auto it = sessions_.begin(); it != sessions_.end(); ++it)
    {
        if((*it)->address() == packet.address)
        {
            try
            {
                (*it)->handle(packet);
            }
            catch(const runtime_error& e)
            {
                LOG(Net, Error) << "session at " << (*it)->address() << " threw an error: " << e.what() << "\n";
                it = sessions_.erase(it);
            }

            return;
        }
    }

    LOG(Net, Warn) << "dropped a packet from " << packet.address << "\n";
}

void SessionManager::tick()
{
    net_time_point now = net_clock::now();

    for(auto it = sessions_.begin(); it != sessions_.end(); /**/)
    {
        (*it)->tick(now);

        for(BlobPtr& blob : (*it)->blobAssembler())
        {
            blobs_.push_back(move(blob));
        }

        (*it)->blobAssembler().clear();

        if((*it)->dead())
        {
            it = sessions_.erase(it);

            if(primary_ == it->get())
            {
                primary_ = nullptr;
            }
        }
        else
        {
            ++it;
        }
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
