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
#ifndef BZR_NET_SESSIONMANAGER_H
#define BZR_NET_SESSIONMANAGER_H

#include "net/Session.h"
#include "net/Socket.h"
#include <mutex>
#include <thread>

class SessionManager : Noncopyable
{
public:
    SessionManager();

    // for external use
    void addLocked(unique_ptr<Session> session);
    void stop();

    // for internal use
    void add(unique_ptr<Session> session);
    bool exists(Address address) const;
    void setPrimary(Session* primary);
    void send(const Packet& packet);
    net_time_point getClientBegin() const;

private:
    void run();
    void handle(const Packet& packet);
    void tick();

    chrono::microseconds getReadTimeout() const;

    mutex mutex_; // protects all class variables
    bool done_;
    Socket socket_;
    Session* primary_;
    vector<unique_ptr<Session>> sessions_;
    net_time_point clientBegin_;
    vector<BlobPtr> blobs_;
    thread thread_;
};

#endif