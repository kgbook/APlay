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

#include <algorithm>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <cstring>

namespace aplay {
namespace core {
namespace network {

bool parse_ipv4_address(const std::string& text, std::uint32_t& address) {
    ::in_addr result{};
    if (::inet_pton(AF_INET, text.c_str(), &result) != 1) {
        return false;
    }
    address = ntohl(result.s_addr);
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

std::string format_ipv4_address(std::uint32_t address) {
    ::in_addr addr{};
    addr.s_addr = htonl(address);
    char buf[INET_ADDRSTRLEN] = {};
    ::inet_ntop(AF_INET, &addr, buf, sizeof(buf));
    return buf;
}

std::string format_ipv6_address(const std::array<std::uint8_t, 16>& address) {
    char buf[INET6_ADDRSTRLEN] = {};
    ::inet_ntop(AF_INET6, address.data(), buf, sizeof(buf));
    return buf;
}

bool default_ipv4_address(std::uint32_t& address) {
    ifaddrs* addrs = NULL;
    if (::getifaddrs(&addrs) != 0) {
        return false;
    }

    bool found = false;
    std::uint32_t result = 0;
    for (ifaddrs* entry = addrs; entry != NULL; entry = entry->ifa_next) {
        if (entry->ifa_addr == NULL || entry->ifa_addr->sa_family != AF_INET ||
            (entry->ifa_flags & IFF_UP) == 0 ||
            (entry->ifa_flags & IFF_MULTICAST) == 0 ||
            (entry->ifa_flags & IFF_LOOPBACK) != 0) {
            continue;
        }

        const sockaddr_in* addr = reinterpret_cast<const sockaddr_in*>(entry->ifa_addr);
        if (addr->sin_addr.s_addr == htonl(INADDR_ANY) ||
            (ntohl(addr->sin_addr.s_addr) >> 24) == 127) {
            continue;
        }

        result = ntohl(addr->sin_addr.s_addr);
        found = true;
        break;
    }
    ::freeifaddrs(addrs);

    if (!found) {
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

std::vector<std::uint32_t> ipv4_multicast_interface_addresses() {
    std::vector<std::uint32_t> addresses;
    ifaddrs* addrs = NULL;
    if (::getifaddrs(&addrs) != 0) {
        return addresses;
    }

    for (ifaddrs* entry = addrs; entry != NULL; entry = entry->ifa_next) {
        if (entry->ifa_addr == NULL || entry->ifa_addr->sa_family != AF_INET ||
            (entry->ifa_flags & IFF_UP) == 0 ||
            (entry->ifa_flags & IFF_MULTICAST) == 0 ||
            (entry->ifa_flags & IFF_LOOPBACK) != 0) {
            continue;
            }

        const sockaddr_in* addr = reinterpret_cast<const sockaddr_in*>(entry->ifa_addr);
        const std::uint32_t address = ntohl(addr->sin_addr.s_addr);
        if (address == INADDR_ANY || (address >> 24) == 127) {
            continue;
        }
        if (std::find(addresses.begin(), addresses.end(), address) == addresses.end()) {
            addresses.push_back(address);
        }
    }
    ::freeifaddrs(addrs);
    return addresses;
}

std::vector<unsigned int> ipv6_multicast_interface_indices() {
    std::vector<unsigned int> indices;
    ifaddrs* addrs = NULL;
    if (::getifaddrs(&addrs) != 0) {
        return indices;
    }

    for (ifaddrs* entry = addrs; entry != NULL; entry = entry->ifa_next) {
        if (entry->ifa_name == NULL || entry->ifa_addr == NULL ||
            entry->ifa_addr->sa_family != AF_INET6 ||
            (entry->ifa_flags & IFF_UP) == 0 ||
            (entry->ifa_flags & IFF_MULTICAST) == 0) {
            continue;
            }

        const unsigned int index = ::if_nametoindex(entry->ifa_name);
        if (index != 0 && std::find(indices.begin(), indices.end(), index) == indices.end()) {
            indices.push_back(index);
        }
    }
    ::freeifaddrs(addrs);
    return indices;
}

} // namespace network
} // namespace core
} // namespace aplay
