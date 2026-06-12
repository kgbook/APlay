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

#ifndef APLAY_CORE_TCP_SOCKET_HPP
#define APLAY_CORE_TCP_SOCKET_HPP

#include "endpoint.hpp"

#include <cstddef>
#include <cstdint>

namespace aplay {
namespace core {
namespace socket {

class TcpSocket {
public:
    TcpSocket();
    explicit TcpSocket(int fd);
    ~TcpSocket();

    TcpSocket(const TcpSocket&) = delete;
    TcpSocket& operator=(const TcpSocket&) = delete;
    TcpSocket(TcpSocket&& other);
    TcpSocket& operator=(TcpSocket&& other);

    int fd() const;
    bool valid() const;
    void close();

    bool set_reuse_address(bool enabled) const;
    bool set_nonblocking(bool enabled) const;
    bool bind_to(const Ipv4Endpoint& endpoint) const;
    bool local_endpoint(Ipv4Endpoint* endpoint) const;
    bool listen(int backlog) const;
    TcpSocket accept(Ipv4Endpoint* peer) const;
    bool connect_to(const Ipv4Endpoint& endpoint);
    int send(const std::uint8_t* bytes, std::size_t length) const;
    int receive(std::uint8_t* bytes, std::size_t length) const;

private:
    int fd_;
};

TcpSocket open_ipv4_tcp_socket();
TcpSocket open_ipv6_tcp_socket();

} // namespace socket
} // namespace core
} // namespace aplay

#endif // APLAY_CORE_TCP_SOCKET_HPP
