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

#include "binary_io.hpp"
#include "mdns_types.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace aplay {
namespace protocol {
namespace mdns {

const std::size_t kMaxPacketSize = 1500;

bool put_name(core::binary::Writer& packet, const std::string& name);

class PacketParser {
public:
    static bool parse_packet(const std::uint8_t* bytes, std::size_t length,
                             PacketSummary& summary);
    static bool parse_question(const std::uint8_t* bytes, std::size_t length,
                               std::size_t& offset, QuestionSummary& question);
    static bool parse_record(const std::uint8_t* bytes, std::size_t length,
                             std::size_t& offset, RecordSummary& record);
};

bool parse_packet(const std::uint8_t* bytes, std::size_t length, PacketSummary& summary);

std::vector<std::uint8_t> build_ptr_query(const std::vector<std::string>& names,
                                          bool request_unicast);

} // namespace mdns
} // namespace protocol
} // namespace aplay
