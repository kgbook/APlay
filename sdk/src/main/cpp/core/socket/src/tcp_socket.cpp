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

#include "impl/tcp_socket.hpp"
#include "socket_internal.hpp"

#include <sys/socket.h>

namespace aplay {
namespace core {
namespace socket {

TcpSocket::TcpSocket() : fd_(-1) {}

TcpSocket::TcpSocket(int fd) : fd_(fd) {}

TcpSocket::~TcpSocket() {
    close();
}

TcpSocket::TcpSocket(TcpSocket&& other) : fd_(other.fd_) {
    other.fd_ = -1;
}

TcpSocket& TcpSocket::operator=(TcpSocket&& other) {
    if (this != &other) {
        close();
        fd_ = other.fd_;
        other.fd_ = -1;
    }
    return *this;
}

int TcpSocket::fd() const {
    return fd_;
}

bool TcpSocket::valid() const {
    return fd_ != -1;
}

void TcpSocket::close() {
    internal::close_fd(fd_);
}

bool TcpSocket::connect_to(const Ipv4Endpoint& endpoint) {
    if (!valid()) {
        return false;
    }
    const sockaddr_in addr = internal::to_sockaddr(endpoint);
    return ::connect(fd_, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) == 0;
}

int TcpSocket::send(const std::uint8_t* bytes, std::size_t length) const {
    if (!valid() || bytes == NULL || length == 0) {
        return -1;
    }
    const ssize_t sent = ::send(fd_, bytes, length, 0);
    return sent < 0 ? -1 : static_cast<int>(sent);
}

int TcpSocket::receive(std::uint8_t* bytes, std::size_t length) const {
    if (!valid() || bytes == NULL || length == 0) {
        return -1;
    }
    const ssize_t received = ::recv(fd_, bytes, length, 0);
    return received < 0 ? -1 : static_cast<int>(received);
}

TcpSocket open_ipv4_tcp_socket() {
    return TcpSocket(::socket(AF_INET, SOCK_STREAM, 0));
}

} // namespace socket
} // namespace core
} // namespace aplay
