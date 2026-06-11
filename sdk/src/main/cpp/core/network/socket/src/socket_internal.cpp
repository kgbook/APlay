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

#include "socket_internal.hpp"

#include <arpa/inet.h>
#include <unistd.h>

#include <cstring>

namespace aplay {
namespace core {
namespace socket {

void close_fd(int& fd) {
    if (fd != -1) {
        ::close(fd);
        fd = -1;
    }
}

sockaddr_in to_sockaddr(const Ipv4Endpoint& endpoint) {
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(endpoint.port);
    addr.sin_addr.s_addr = htonl(endpoint.address);
    return addr;
}

Ipv4Endpoint from_sockaddr(const sockaddr_in& addr) {
    Ipv4Endpoint endpoint;
    endpoint.address = ntohl(addr.sin_addr.s_addr);
    endpoint.port = ntohs(addr.sin_port);
    return endpoint;
}

sockaddr_in6 to_sockaddr(const Ipv6Endpoint& endpoint) {
    sockaddr_in6 addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(endpoint.port);
    std::memcpy(addr.sin6_addr.s6_addr, endpoint.address.data(), endpoint.address.size());
    addr.sin6_scope_id = endpoint.scope_id;
    return addr;
}

Ipv6Endpoint from_sockaddr(const sockaddr_in6& addr) {
    Ipv6Endpoint endpoint;
    std::memcpy(endpoint.address.data(), addr.sin6_addr.s6_addr, endpoint.address.size());
    endpoint.port = ntohs(addr.sin6_port);
    endpoint.scope_id = addr.sin6_scope_id;
    return endpoint;
}

} // namespace socket
} // namespace core
} // namespace aplay
