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

#include "impl/ipv4or6_endpoint.hpp"
#include "socket_internal.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>

namespace aplay {
namespace core {
namespace socket {
Ipv4Endpoint make_ipv4_endpoint(const std::string& address, std::uint16_t port) {
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    Ipv4Endpoint endpoint;
    if (::inet_pton(AF_INET, address.c_str(), &addr.sin_addr) == 1) {
        endpoint.address = ntohl(addr.sin_addr.s_addr);
        endpoint.port = port;
    }
    return endpoint;
}

Ipv6Endpoint make_ipv6_endpoint(const std::string& address, std::uint16_t port,
                                std::uint32_t scope_id) {
    Ipv6Endpoint endpoint;
    in6_addr addr;
    std::memset(&addr, 0, sizeof(addr));
    if (::inet_pton(AF_INET6, address.c_str(), &addr) == 1) {
        std::memcpy(endpoint.address.data(), addr.s6_addr, endpoint.address.size());
        endpoint.port = port;
        endpoint.scope_id = scope_id;
    }
    return endpoint;
}

} // namespace socket
} // namespace core
} // namespace aplay
