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

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace aplay {
namespace protocol {
namespace mdns {

struct RecordSummary {
    std::string name;
    std::uint16_t type = 0;
    std::uint16_t dns_class = 0;
    std::uint32_t ttl = 0;
    std::string ptr_name;
    std::string srv_target;
    std::uint16_t srv_port = 0;
    std::uint32_t ipv4_address = 0; // Network byte order.
    std::array<std::uint8_t, 16> ipv6_address{};
    std::vector<std::string> txt;
};

} // namespace mdns
} // namespace protocol
} // namespace aplay
