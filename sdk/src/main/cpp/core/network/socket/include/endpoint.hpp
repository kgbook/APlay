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

#ifndef APLAY_CORE_SOCKET_ENDPOINT_HPP
#define APLAY_CORE_SOCKET_ENDPOINT_HPP

#include <array>
#include <cstdint>
#include <string>

namespace aplay {
namespace core {
namespace socket {

struct Ipv4Endpoint {
    std::uint32_t address = 0; // Host byte order.
    std::uint16_t port = 0;
};

struct Ipv6Endpoint {
    std::array<std::uint8_t, 16> address{};
    std::uint16_t port = 0;
    std::uint32_t scope_id = 0;
};

Ipv4Endpoint make_ipv4_endpoint(const std::string& address, std::uint16_t port);
Ipv6Endpoint make_ipv6_endpoint(const std::string& address, std::uint16_t port,
                                std::uint32_t scope_id);

} // namespace socket
} // namespace core
} // namespace aplay

#endif // APLAY_CORE_SOCKET_ENDPOINT_HPP
