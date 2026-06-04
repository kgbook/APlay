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

#include "ipv4_endpoint.hpp"
#include "socket_internal.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cstring>

namespace aplay {
namespace core {
namespace socket {

std::uint32_t default_ipv4_address_for_route(const std::string& remote_address,
                                             std::uint16_t remote_port) {
    int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        return 0;
    }

    sockaddr_in remote;
    std::memset(&remote, 0, sizeof(remote));
    remote.sin_family = AF_INET;
    remote.sin_port = htons(remote_port);
    if (::inet_pton(AF_INET, remote_address.c_str(), &remote.sin_addr) != 1) {
        internal::close_fd(fd);
        return 0;
    }

    sockaddr_in local;
    std::memset(&local, 0, sizeof(local));
    socklen_t local_len = sizeof(local);
    if (::connect(fd, reinterpret_cast<sockaddr*>(&remote), sizeof(remote)) != 0 ||
        ::getsockname(fd, reinterpret_cast<sockaddr*>(&local), &local_len) != 0) {
        internal::close_fd(fd);
        return 0;
    }
    internal::close_fd(fd);
    return ntohl(local.sin_addr.s_addr);
}

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

} // namespace socket
} // namespace core
} // namespace aplay
