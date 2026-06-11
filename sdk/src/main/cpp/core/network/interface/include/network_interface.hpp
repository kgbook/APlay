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

struct Ipv6MulticastInterface {
    std::array<std::uint8_t, 16> address{};
    unsigned int index = 0;
};

bool parse_ipv4_address(const std::string& text, std::uint32_t& address);
bool parse_ipv6_address(const std::string& text, std::array<std::uint8_t, 16>& address);

std::string format_ipv4_address(std::uint32_t address);
std::string format_ipv6_address(const std::array<std::uint8_t, 16>& address);

bool default_ipv4_address(std::uint32_t& address);
bool default_ipv6_address(std::array<std::uint8_t, 16>& address);

bool local_ipv4_address_for_peer(std::uint32_t peer_address, std::uint16_t peer_port,
                                 std::uint32_t& address);
bool local_ipv6_address_for_peer(const std::array<std::uint8_t, 16>& peer_address,
                                 std::uint32_t peer_scope_id, std::uint16_t peer_port,
                                 std::array<std::uint8_t, 16>& address,
                                 std::uint32_t& scope_id);

std::vector<std::uint32_t> ipv4_multicast_interface_addresses();
std::vector<Ipv6MulticastInterface> ipv6_multicast_interfaces();
std::vector<unsigned int> ipv6_multicast_interface_indices();

} // namespace network
} // namespace core
} // namespace aplay

#endif // APLAY_CORE_NETWORK_INTERFACE_HPP
