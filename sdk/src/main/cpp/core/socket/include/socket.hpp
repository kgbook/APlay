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

#ifndef APLAY_CORE_SOCKET_HPP
#define APLAY_CORE_SOCKET_HPP

#include <cstddef>
#include <cstdint>
#include <string>

namespace aplay {
namespace core {
namespace socket {

struct Ipv4Endpoint {
    std::uint32_t address = 0; // Network byte order.
    std::uint16_t port = 0;
};

class Fd {
public:
    Fd();
    explicit Fd(int fd);
    ~Fd();

    Fd(const Fd&) = delete;
    Fd& operator=(const Fd&) = delete;
    Fd(Fd&& other);
    Fd& operator=(Fd&& other);

    int get() const;
    bool valid() const;
    int release();
    void reset(int fd = -1);

private:
    int fd_;
};

class UdpSocket {
public:
    UdpSocket();
    explicit UdpSocket(int fd);

    UdpSocket(const UdpSocket&) = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;
    UdpSocket(UdpSocket&& other);
    UdpSocket& operator=(UdpSocket&& other);

    int fd() const;
    bool valid() const;
    void close();

    bool send_to(const std::uint8_t* bytes, std::size_t length,
                 const Ipv4Endpoint& endpoint) const;
    int receive_from(std::uint8_t* bytes, std::size_t length, Ipv4Endpoint& endpoint) const;

private:
    Fd fd_;
};

std::uint32_t default_ipv4_address_for_route(const std::string& remote_address,
                                             std::uint16_t remote_port);
Ipv4Endpoint make_ipv4_endpoint(const std::string& address, std::uint16_t port);
UdpSocket open_ipv4_udp_multicast_socket(std::uint16_t port,
                                         const std::string& multicast_address,
                                         std::uint32_t interface_address);

} // namespace socket
} // namespace core
} // namespace aplay

#endif // APLAY_CORE_SOCKET_HPP
