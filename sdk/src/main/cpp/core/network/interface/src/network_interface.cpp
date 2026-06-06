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

#include "network_interface.hpp"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>

#include <cstring>

namespace aplay {
namespace core {
namespace network {

bool parse_ipv4_address(const std::string& text, std::uint32_t& address) {
    std::uint32_t result = 0;
    std::size_t start = 0;
    for (int part = 0; part < 4; ++part) {
        const std::size_t dot = text.find('.', start);
        const std::size_t end = dot == std::string::npos ? text.size() : dot;
        if (end == start) {
            return false;
        }

        int value = 0;
        for (std::size_t i = start; i < end; ++i) {
            if (text[i] < '0' || text[i] > '9') {
                return false;
            }
            value = value * 10 + (text[i] - '0');
            if (value > 255) {
                return false;
            }
        }

        result = (result << 8) | static_cast<std::uint32_t>(value);
        if (part < 3 && dot == std::string::npos) {
            return false;
        }
        start = end + 1;
    }

    if (start <= text.size()) {
        return false;
    }
    address = result;
    return true;
}

bool parse_ipv6_address(const std::string& text, std::array<std::uint8_t, 16>& address) {
    std::array<std::uint8_t, 16> result{};
    if (::inet_pton(AF_INET6, text.c_str(), result.data()) != 1) {
        return false;
    }
    address = result;
    return true;
}

bool default_ipv6_address(std::array<std::uint8_t, 16>& address) {
    ifaddrs* addrs = NULL;
    if (::getifaddrs(&addrs) != 0) {
        return false;
    }

    bool found = false;
    std::array<std::uint8_t, 16> result{};
    for (ifaddrs* entry = addrs; entry != NULL; entry = entry->ifa_next) {
        if (entry->ifa_addr == NULL || entry->ifa_addr->sa_family != AF_INET6 ||
            (entry->ifa_flags & IFF_UP) == 0 ||
            (entry->ifa_flags & IFF_MULTICAST) == 0 ||
            (entry->ifa_flags & IFF_LOOPBACK) != 0) {
            continue;
        }

        const sockaddr_in6* addr = reinterpret_cast<const sockaddr_in6*>(entry->ifa_addr);
        if (IN6_IS_ADDR_UNSPECIFIED(&addr->sin6_addr) ||
            IN6_IS_ADDR_LOOPBACK(&addr->sin6_addr)) {
            continue;
        }

        std::memcpy(result.data(), addr->sin6_addr.s6_addr, result.size());
        found = true;
        if (IN6_IS_ADDR_LINKLOCAL(&addr->sin6_addr)) {
            break;
        }
    }
    ::freeifaddrs(addrs);

    if (!found) {
        return false;
    }
    address = result;
    return true;
}

} // namespace network
} // namespace core
} // namespace aplay
