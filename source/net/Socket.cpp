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
#include "net/Socket.h"
#ifndef _WIN32
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#ifdef _WIN32

struct Startup
{
    Startup()
    {
        WSAData wsaData;
        int err = WSAStartup(MAKEWORD(2, 2), &wsaData);

        if(err != 0)
        {
            throw runtime_error("WSAStartup failed");
        }
    }

    ~Startup()
    {
        WSACleanup();
    }
};

static Startup g_startup;

Socket::Socket()
{
    sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if(sock_ == INVALID_SOCKET)
    {
        throw runtime_error("Failed to create socket");
    }

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sock_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0)
    {
        throw runtime_error("Failed to bind socket");
    }    
}

Socket::~Socket()
{
    closesocket(sock_);
}

void Socket::read(Packet& packet)
{
    sockaddr_in from;
    int fromLen = sizeof(from);

    size_t recvLen = recvfrom(sock_,
        reinterpret_cast<char*>(packet.data.data()),
        static_cast<int>(packet.data.size()),
        /*flags*/ 0,
        reinterpret_cast<sockaddr*>(&from),
        &fromLen);

    if(recvLen == SOCKET_ERROR)
    {
        throw runtime_error("recvfrom failed");
    }

    packet.remoteIp = htonl(from.sin_addr.s_addr);
    packet.remotePort = htons(from.sin_port);
    packet.size = recvLen;
}

void Socket::write(const Packet& packet)
{
    assert(packet.size < packet.data.size());

    sockaddr_in to;
    memset(&to, 0, sizeof(to));
    to.sin_family = AF_INET;
    to.sin_port = htons(packet.remotePort);
    to.sin_addr.s_addr = htonl(packet.remoteIp);

    int sendLen = sendto(sock_,
        reinterpret_cast<const char*>(packet.data.data()),
        static_cast<int>(packet.size),
        /*flags*/ 0,
        reinterpret_cast<sockaddr*>(&to),
        sizeof(to));

    if(sendLen != packet.size)
    {
        throw runtime_error("sendto failed");
    }
}

#else // ndef _WIN32

Socket::Socket()
{
    fd_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if(fd_ < 0)
    {
        throw runtime_error("Failed to create socket");
    }

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = INADDR_ANY;
}

Socket::~Socket()
{
    close(fd_);
}

void Socket::read(Packet& packet)
{
    sockaddr_in from;
    socklen_t fromLen = sizeof(from);

    ssize_t recvLen = recvfrom(fd_,
        packet.data.data(),
        packet.data.size(),
        /*flags*/ 0,
        reinterpret_cast<sockaddr*>(&from),
        &fromLen);

    if(recvLen < 0)
    {
        throw runtime_error("recvfrom failed");
    }

    packet.remoteIp = htonl(from.sin_addr.s_addr);
    packet.remotePort = htons(from.sin_port);
    packet.size = recvLen;
}

void Socket::write(const Packet& packet)
{
    assert(packet.size < packet.data.size());

    sockaddr_in to;
    memset(&to, 0, sizeof(to));
    to.sin_family = AF_INET;
    to.sin_port = htons(packet.remotePort);
    to.sin_addr.s_addr = htonl(packet.remoteIp);

    ssize_t sendLen = sendto(fd_,
        packet.data.data(),
        packet.size,
        /*flags*/ 0,
        reinterpret_cast<sockaddr*>(&to),
        sizeof(to));

    if(sendLen < 0 || static_cast<size_t>(sendLen) != packet.size)
    {
        throw runtime_error("sendto failed");
    }
}

#endif // ndef _WIN32
