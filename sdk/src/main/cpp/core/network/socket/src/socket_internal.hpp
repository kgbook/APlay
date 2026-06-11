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

#include "endpoint.hpp"

#include <netinet/in.h>

namespace aplay {
namespace core {
namespace socket {

void close_fd(int& fd);
sockaddr_in to_sockaddr(const Ipv4Endpoint& endpoint);
Ipv4Endpoint from_sockaddr(const sockaddr_in& addr);
sockaddr_in6 to_sockaddr(const Ipv6Endpoint& endpoint);
Ipv6Endpoint from_sockaddr(const sockaddr_in6& addr);

} // namespace socket
} // namespace core
} // namespace aplay
