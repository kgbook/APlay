/**
 *  Copyright (C) 2026  kgbook
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 */

#include "socket.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <utility>

namespace aplay {
namespace core {
namespace socket {
namespace {

sockaddr_in to_sockaddr(const Ipv4Endpoint& endpoint) {
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(endpoint.port);
    addr.sin_addr.s_addr = htonl(endpoint.address);
    return addr;
}

Ipv4Endpoint from_sockaddr(const sockaddr_in& addr) {
    Ipv4Endpoint endpoint;
    endpoint.address = ntohl(addr.sin_addr.s_addr);
    endpoint.port = ntohs(addr.sin_port);
    return endpoint;
}

} // namespace

Fd::Fd() : fd_(-1) {}

Fd::Fd(int fd) : fd_(fd) {}

Fd::~Fd() {
    reset();
}

Fd::Fd(Fd&& other) : fd_(other.release()) {}

Fd& Fd::operator=(Fd&& other) {
    if (this != &other) {
        reset(other.release());
    }
    return *this;
}

int Fd::get() const {
    return fd_;
}

bool Fd::valid() const {
    return fd_ != -1;
}

int Fd::release() {
    const int fd = fd_;
    fd_ = -1;
    return fd;
}

void Fd::reset(int fd) {
    if (fd_ != -1) {
        ::close(fd_);
    }
    fd_ = fd;
}

UdpSocket::UdpSocket() : fd_() {}

UdpSocket::UdpSocket(int fd) : fd_(fd) {}

UdpSocket::UdpSocket(UdpSocket&& other) : fd_(std::move(other.fd_)) {}

UdpSocket& UdpSocket::operator=(UdpSocket&& other) {
    if (this != &other) {
        fd_ = std::move(other.fd_);
    }
    return *this;
}

int UdpSocket::fd() const {
    return fd_.get();
}

bool UdpSocket::valid() const {
    return fd_.valid();
}

void UdpSocket::close() {
    fd_.reset();
}

bool UdpSocket::send_to(const std::uint8_t* bytes, std::size_t length,
                        const Ipv4Endpoint& endpoint) const {
    if (!valid() || bytes == NULL || length == 0) {
        return false;
    }
    const sockaddr_in addr = to_sockaddr(endpoint);
    return ::sendto(fd(), bytes, length, 0, reinterpret_cast<const sockaddr*>(&addr),
                    sizeof(addr)) == static_cast<ssize_t>(length);
}

int UdpSocket::receive_from(std::uint8_t* bytes, std::size_t length,
                            Ipv4Endpoint& endpoint) const {
    if (!valid() || bytes == NULL || length == 0) {
        return -1;
    }
    sockaddr_in from;
    std::memset(&from, 0, sizeof(from));
    socklen_t from_len = sizeof(from);
    const ssize_t received =
        ::recvfrom(fd(), bytes, length, 0, reinterpret_cast<sockaddr*>(&from), &from_len);
    if (received < 0) {
        return -1;
    }
    endpoint = from_sockaddr(from);
    return static_cast<int>(received);
}

std::uint32_t default_ipv4_address_for_route(const std::string& remote_address,
                                             std::uint16_t remote_port) {
    Fd fd(::socket(AF_INET, SOCK_DGRAM, 0));
    if (!fd.valid()) {
        return 0;
    }

    sockaddr_in remote;
    std::memset(&remote, 0, sizeof(remote));
    remote.sin_family = AF_INET;
    remote.sin_port = htons(remote_port);
    if (::inet_pton(AF_INET, remote_address.c_str(), &remote.sin_addr) != 1) {
        return 0;
    }

    sockaddr_in local;
    std::memset(&local, 0, sizeof(local));
    socklen_t local_len = sizeof(local);
    if (::connect(fd.get(), reinterpret_cast<sockaddr*>(&remote), sizeof(remote)) != 0 ||
        ::getsockname(fd.get(), reinterpret_cast<sockaddr*>(&local), &local_len) != 0) {
        return 0;
    }
    return ntohl(local.sin_addr.s_addr);
}

Ipv4Endpoint make_ipv4_endpoint(const std::string& address, std::uint16_t port) {
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    Ipv4Endpoint endpoint;
    if (::inet_pton(AF_INET, address.c_str(), &addr.sin_addr) == 1) {
        endpoint.address = ntohl(addr.sin_addr.s_addr);
        endpoint.port = port;
    }
    return endpoint;
}

UdpSocket open_ipv4_udp_multicast_socket(std::uint16_t port,
                                         const std::string& multicast_address,
                                         std::uint32_t interface_address) {
    Fd fd(::socket(AF_INET, SOCK_DGRAM, 0));
    if (!fd.valid()) {
        return UdpSocket();
    }

    int one = 1;
    unsigned char ttl = 255;
    unsigned char loop = 1;
    ::setsockopt(fd.get(), SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
#ifdef SO_REUSEPORT
    ::setsockopt(fd.get(), SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one));
#endif
    ::setsockopt(fd.get(), IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
    ::setsockopt(fd.get(), IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));

    if (interface_address != 0) {
        in_addr iface;
        std::memset(&iface, 0, sizeof(iface));
        iface.s_addr = htonl(interface_address);
        ::setsockopt(fd.get(), IPPROTO_IP, IP_MULTICAST_IF, &iface, sizeof(iface));
    }

    sockaddr_in bind_addr;
    std::memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = htons(port);
    bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (::bind(fd.get(), reinterpret_cast<sockaddr*>(&bind_addr), sizeof(bind_addr)) == -1) {
        return UdpSocket();
    }

    ip_mreq mreq;
    std::memset(&mreq, 0, sizeof(mreq));
    if (::inet_pton(AF_INET, multicast_address.c_str(), &mreq.imr_multiaddr) != 1) {
        return UdpSocket();
    }
    mreq.imr_interface.s_addr =
        interface_address == 0 ? htonl(INADDR_ANY) : htonl(interface_address);
    if (::setsockopt(fd.get(), IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) == -1) {
        if (interface_address == 0) {
            return UdpSocket();
        }
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        if (::setsockopt(fd.get(), IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) == -1) {
            return UdpSocket();
        }
    }

    return UdpSocket(fd.release());
}

} // namespace socket
} // namespace core
} // namespace aplay
