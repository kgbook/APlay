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
#include <string>

namespace aplay {
namespace core {
namespace network {

bool parse_ipv4_address(const std::string& text, std::uint32_t& address);

} // namespace network
} // namespace core
} // namespace aplay

#endif // APLAY_CORE_NETWORK_INTERFACE_HPP
