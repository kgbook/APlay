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
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

namespace aplay {
namespace core {
namespace network {

static bool is_ipv6_empty(const std::array<std::uint8_t, 16>& address) {
    const std::array<std::uint8_t, 16> empty{};
    return address == empty;
}

static bool is_ipv6_link_local(const std::array<std::uint8_t, 16>& address) {
    return address[0] == 0xfe && (address[1] & 0xc0) == 0x80;
}

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

bool local_ipv4_address_for_peer(std::uint32_t peer_address, std::uint16_t peer_port,
                                 std::uint32_t& address) {
    if (peer_address == 0) {
        return false;
    }

    const int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        return false;
    }

    sockaddr_in remote;
    std::memset(&remote, 0, sizeof(remote));
    remote.sin_family = AF_INET;
    remote.sin_port = htons(peer_port);
    remote.sin_addr.s_addr = htonl(peer_address);

    bool found = false;
    if (::connect(fd, reinterpret_cast<sockaddr*>(&remote), sizeof(remote)) == 0) {
        sockaddr_in local;
        std::memset(&local, 0, sizeof(local));
        socklen_t local_len = sizeof(local);
        if (::getsockname(fd, reinterpret_cast<sockaddr*>(&local), &local_len) == 0 &&
            local.sin_addr.s_addr != htonl(INADDR_ANY)) {
            address = ntohl(local.sin_addr.s_addr);
            found = true;
        }
    }

    ::close(fd);
    return found;
}

bool local_ipv6_address_for_peer(const std::array<std::uint8_t, 16>& peer_address,
                                 std::uint32_t peer_scope_id, std::uint16_t peer_port,
                                 std::array<std::uint8_t, 16>& address,
                                 std::uint32_t& scope_id) {
    if (is_ipv6_empty(peer_address)) {
        return false;
    }

    const int fd = ::socket(AF_INET6, SOCK_DGRAM, 0);
    if (fd == -1) {
        return false;
    }

    sockaddr_in6 remote;
    std::memset(&remote, 0, sizeof(remote));
    remote.sin6_family = AF_INET6;
    remote.sin6_port = htons(peer_port);
    remote.sin6_scope_id = peer_scope_id;
    std::memcpy(remote.sin6_addr.s6_addr, peer_address.data(), peer_address.size());

    bool found = false;
    if (::connect(fd, reinterpret_cast<sockaddr*>(&remote), sizeof(remote)) == 0) {
        sockaddr_in6 local;
        std::memset(&local, 0, sizeof(local));
        socklen_t local_len = sizeof(local);
        if (::getsockname(fd, reinterpret_cast<sockaddr*>(&local), &local_len) == 0) {
            std::array<std::uint8_t, 16> result{};
            std::memcpy(result.data(), local.sin6_addr.s6_addr, result.size());
            if (!is_ipv6_empty(result)) {
                address = result;
                scope_id = local.sin6_scope_id != 0 ? local.sin6_scope_id : peer_scope_id;
                found = true;
            }
        }
    }

    ::close(fd);
    return found;
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

std::vector<Ipv6MulticastInterface> ipv6_multicast_interfaces() {
    std::vector<Ipv6MulticastInterface> interfaces;
    ifaddrs* addrs = NULL;
    if (::getifaddrs(&addrs) != 0) {
        return interfaces;
    }

    for (ifaddrs* entry = addrs; entry != NULL; entry = entry->ifa_next) {
        if (entry->ifa_name == NULL || entry->ifa_addr == NULL ||
            entry->ifa_addr->sa_family != AF_INET6 ||
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

        const unsigned int index = ::if_nametoindex(entry->ifa_name);
        if (index == 0) {
            continue;
        }

        Ipv6MulticastInterface iface;
        std::memcpy(iface.address.data(), addr->sin6_addr.s6_addr, iface.address.size());
        iface.index = index;

        bool exists = false;
        for (std::size_t i = 0; i < interfaces.size(); ++i) {
            if (interfaces[i].index == iface.index) {
                if (!is_ipv6_link_local(interfaces[i].address) &&
                    is_ipv6_link_local(iface.address)) {
                    interfaces[i].address = iface.address;
                }
                exists = true;
                break;
            }
        }
        if (!exists) {
            interfaces.push_back(iface);
        }
    }
    ::freeifaddrs(addrs);
    return interfaces;
}

std::vector<unsigned int> ipv6_multicast_interface_indices() {
    std::vector<unsigned int> indices;
    const std::vector<Ipv6MulticastInterface> interfaces = ipv6_multicast_interfaces();
    for (std::size_t i = 0; i < interfaces.size(); ++i) {
        indices.push_back(interfaces[i].index);
    }
    return indices;
}

} // namespace network
} // namespace core
} // namespace aplay
