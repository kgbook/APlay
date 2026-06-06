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

#ifndef APLAY_CORE_UDP_SOCKET_HPP
#define APLAY_CORE_UDP_SOCKET_HPP

#include "ipv4or6_endpoint.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace aplay {
namespace core {
namespace socket {

class UdpSocket {
public:
    UdpSocket();
    explicit UdpSocket(int fd);
    ~UdpSocket();

    UdpSocket(const UdpSocket&) = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;
    UdpSocket(UdpSocket&& other);
    UdpSocket& operator=(UdpSocket&& other);

    int fd() const;
    bool valid() const;
    void close();

    bool send_to(const std::uint8_t* bytes, std::size_t length,
                 const Ipv4Endpoint& endpoint) const;
    bool send_to(const std::uint8_t* bytes, std::size_t length,
                 const Ipv6Endpoint& endpoint) const;
    int receive_from(std::uint8_t* bytes, std::size_t length, Ipv4Endpoint& endpoint) const;
    int receive_from(std::uint8_t* bytes, std::size_t length, Ipv6Endpoint& endpoint) const;
    bool set_ipv4_multicast_interface(std::uint32_t interface_address) const;

private:
    int fd_;
};

UdpSocket open_ipv4_udp_multicast_socket(std::uint16_t port,
                                         const std::string& multicast_address,
                                         std::uint32_t interface_address);
UdpSocket open_ipv6_udp_multicast_socket(std::uint16_t port,
                                         const std::string& multicast_address);
} // namespace socket
} // namespace core
} // namespace aplay

#endif // APLAY_CORE_UDP_SOCKET_HPP
