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

#include "impl/mdns_parser.hpp"
#include "impl/mdns_constants.hpp"
#include "mdns_internal.hpp"

namespace aplay {
namespace protocol {
namespace mdns {

std::vector<std::uint8_t> build_ptr_query(const std::vector<std::string>& names,
                                          bool request_unicast) {
    internal::PacketWriter packet;
    packet.put_u16(0);
    packet.put_u16(0);
    packet.put_u16(static_cast<std::uint16_t>(names.size()));
    packet.put_u16(0);
    packet.put_u16(0);
    packet.put_u16(0);
    for (const std::string& name : names) {
        packet.put_name(name);
        packet.put_u16(kTypePtr);
        packet.put_u16(kClassIn | (request_unicast ? kUnicastResponse : 0));
    }
    return packet.take();
}

} // namespace mdns
} // namespace protocol
} // namespace aplay
