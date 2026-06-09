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

#ifndef APLAY_CORE_NETWORK_INTERFACE_HPP
#define APLAY_CORE_NETWORK_INTERFACE_HPP

#include <cstdint>
#include <array>
#include <vector>
#include <string>

namespace aplay {
namespace core {
namespace network {

bool parse_ipv4_address(const std::string& text, std::uint32_t& address);
bool parse_ipv6_address(const std::string& text, std::array<std::uint8_t, 16>& address);

std::string format_ipv4_address(std::uint32_t address);

bool default_ipv4_address(std::uint32_t& address);
bool default_ipv6_address(std::array<std::uint8_t, 16>& address);

std::vector<std::uint32_t> ipv4_multicast_interface_addresses();
std::vector<unsigned int> ipv6_multicast_interface_indices();

} // namespace network
} // namespace core
} // namespace aplay

#endif // APLAY_CORE_NETWORK_INTERFACE_HPP
