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

static chrono::microseconds kMaxTimeout = chrono::seconds(1);

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

SessionManager::SessionManager() : done_(false), thread_(bind(&SessionManager::run, this))
{}

void SessionManager::addLocked(unique_ptr<Session> session)
{
    lock_guard<mutex> lock(mutex_);
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

bool SessionManager::exists(uint32_t ip, uint16_t port) const
{
    for(const unique_ptr<Session>& session : sessions_)
    {
        if(session->serverIp() == ip && session->serverPort() == port)
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

            while(packet.size != 0)
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
    for(unique_ptr<Session>& session : sessions_)
    {
        if(session->serverIp() == packet.remoteIp && session->serverPort() == packet.remotePort)
        {
            session->handle(packet);
            return;
        }
    }

    // TODO proper logging
    fprintf(stderr, "WARNING: dropped packet from %08x:%d\n", packet.remoteIp, packet.remotePort);
}

void SessionManager::tick()
{
    net_time_point now = net_clock::now();

    for(auto it = sessions_.begin(); it != sessions_.end(); /**/)
    {
        (*it)->tick(now);

        if((*it)->dead())
        {
            it = sessions_.erase(it);
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
