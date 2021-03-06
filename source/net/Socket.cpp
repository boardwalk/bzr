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
#include "Core.h"
#include "Log.h"
#ifdef _WIN32
#include <ws2tcpip.h>
#else
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#define CLOSE_SOCKET closesocket
typedef SSIZE_T ssize_t;
static const SocketType kBadSocket = INVALID_SOCKET;
static const int kSocketError = SOCKET_ERROR;
static int getError() { return WSAGetLastError(); }
static bool isWouldBlockError(int err) { return err == WSAEWOULDBLOCK; }
#else
#define CLOSE_SOCKET close
static const SocketType kBadSocket = -1;
static const int kSocketError = -1;
static int getError() { return errno; }
static bool isWouldBlockError(int err) { return errno == EAGAIN || errno == EWOULDBLOCK; }
#endif

Socket::Socket()
{
    sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if(sock_ == kBadSocket)
    {
        throw runtime_error("Failed to create socket");
    }

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(::bind(sock_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0)
    {
        throw runtime_error("Failed to bind socket");
    }

#ifdef _WIN32
    u_long nonblock = 1;
    if(ioctlsocket(sock_, FIONBIO, &nonblock) != 0)
#else
    int flags = fcntl(sock_, F_GETFL);
    if(fcntl(sock_, F_SETFL, flags | O_NONBLOCK) != 0)
#endif
    {
        throw runtime_error("Failed to set nonblocking mode");
    }
}

Socket::~Socket()
{
    CLOSE_SOCKET(sock_);
}

bool Socket::wait(chrono::microseconds timeout)
{
    int nfds = static_cast<int>(sock_) + 1;

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sock_, &readfds);

    timeval tv;
    tv.tv_sec = static_cast<long>(timeout.count() / 1000000);
    tv.tv_usec = static_cast<long>(timeout.count() % 1000000);

    int ret = select(nfds, &readfds, nullptr, nullptr, &tv);

    if(ret == kSocketError)
    {
        throw runtime_error("select failed");
    }

    return ret == 1;
}

bool Socket::recv(Packet& packet)
{
BEGIN:
    sockaddr_in from;
    socklen_t fromLen = sizeof(from);

    ssize_t recvLen = recvfrom(sock_,
        reinterpret_cast<char*>(&packet.header),
        static_cast<int>(sizeof(packet.header) + sizeof(packet.payload)),
        /*flags*/ 0,
        reinterpret_cast<sockaddr*>(&from),
        &fromLen);

    if(recvLen == kSocketError)
    {
        int err = getError();

        if(isWouldBlockError(err))
        {
            return false;
        }

        throw runtime_error("recvfrom failed");
    }

    packet.address = Address(htonl(from.sin_addr.s_addr), htons(from.sin_port));

    if(recvLen < static_cast<ssize_t>(sizeof(PacketHeader)))
    {
        LOG(Net, Warn) << packet.address << " dropping packet smaller than header\n";
        goto BEGIN;
    }

    if(recvLen != static_cast<ssize_t>(sizeof(PacketHeader) + packet.header.size))
    {
        LOG(Net, Warn) << packet.address << " dropping packet with bad size in header\n";
        goto BEGIN;
    }

    return true;
}

void Socket::send(const Packet& packet)
{
    assert(packet.header.size <= sizeof(packet.payload));

BEGIN:
    sockaddr_in to;
    memset(&to, 0, sizeof(to));
    to.sin_family = AF_INET;
    to.sin_port = htons(packet.address.port());
    to.sin_addr.s_addr = htonl(packet.address.ip());

    ssize_t sendLen = sendto(sock_,
        reinterpret_cast<const char*>(&packet.header),
        static_cast<int>(sizeof(packet.header) + packet.header.size),
        /*flags*/ 0,
        reinterpret_cast<sockaddr*>(&to),
        sizeof(to));

    if(sendLen == kSocketError)
    {
        int err = getError();

        // this should only happen if we somehow fill up the the os's buffers (e.g. never)
        // we'll just wait until the fd is writable
        if(isWouldBlockError(err))
        {
            int nfds = static_cast<int>(sock_) + 1;

            fd_set writefds;
            FD_ZERO(&writefds);
            FD_SET(sock_, &writefds);

            if(select(nfds, nullptr, &writefds, nullptr, nullptr) != 1)
            {
                throw runtime_error("select failed");
            }

            goto BEGIN;
        }

        LOG(Net, Error) << "sendto failed with " << err << "\n";

        throw runtime_error("sendto failed");
    }
}

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
#endif
