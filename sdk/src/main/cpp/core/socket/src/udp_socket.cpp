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

#include "impl/udp_socket.hpp"
#include "socket_internal.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cstring>

namespace aplay {
namespace core {
namespace socket {

UdpSocket::UdpSocket() : fd_(-1) {}

UdpSocket::UdpSocket(int fd) : fd_(fd) {}

UdpSocket::~UdpSocket() {
    close();
}

UdpSocket::UdpSocket(UdpSocket&& other) : fd_(other.fd_) {
    other.fd_ = -1;
}

UdpSocket& UdpSocket::operator=(UdpSocket&& other) {
    if (this != &other) {
        close();
        fd_ = other.fd_;
        other.fd_ = -1;
    }
    return *this;
}

int UdpSocket::fd() const {
    return fd_;
}

bool UdpSocket::valid() const {
    return fd_ != -1;
}

void UdpSocket::close() {
    internal::close_fd(fd_);
}

bool UdpSocket::send_to(const std::uint8_t* bytes, std::size_t length,
                        const Ipv4Endpoint& endpoint) const {
    if (!valid() || bytes == NULL || length == 0) {
        return false;
    }
    const sockaddr_in addr = internal::to_sockaddr(endpoint);
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
    endpoint = internal::from_sockaddr(from);
    return static_cast<int>(received);
}

UdpSocket open_ipv4_udp_multicast_socket(std::uint16_t port,
                                         const std::string& multicast_address,
                                         std::uint32_t interface_address) {
    int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        return UdpSocket();
    }

    int one = 1;
    unsigned char ttl = 255;
    unsigned char loop = 1;
    ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
#ifdef SO_REUSEPORT
    ::setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one));
#endif
    ::setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
    ::setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));

    if (interface_address != 0) {
        in_addr iface;
        std::memset(&iface, 0, sizeof(iface));
        iface.s_addr = htonl(interface_address);
        ::setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, &iface, sizeof(iface));
    }

    sockaddr_in bind_addr;
    std::memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = htons(port);
    bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (::bind(fd, reinterpret_cast<sockaddr*>(&bind_addr), sizeof(bind_addr)) == -1) {
        internal::close_fd(fd);
        return UdpSocket();
    }

    ip_mreq mreq;
    std::memset(&mreq, 0, sizeof(mreq));
    if (::inet_pton(AF_INET, multicast_address.c_str(), &mreq.imr_multiaddr) != 1) {
        internal::close_fd(fd);
        return UdpSocket();
    }
    mreq.imr_interface.s_addr =
        interface_address == 0 ? htonl(INADDR_ANY) : htonl(interface_address);
    if (::setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) == -1) {
        if (interface_address == 0) {
            internal::close_fd(fd);
            return UdpSocket();
        }
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        if (::setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) == -1) {
            internal::close_fd(fd);
            return UdpSocket();
        }
    }

    return UdpSocket(fd);
}

} // namespace socket
} // namespace core
} // namespace aplay
