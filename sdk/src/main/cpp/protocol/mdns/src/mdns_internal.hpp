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

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace aplay {
namespace protocol {
namespace mdns {
namespace internal {

static const std::size_t kMaxPacketSize = 1500;
static const std::size_t kCombinedPacketMax = 1200;
static const std::uint16_t kFlagResponse = 0x8000;
static const std::uint16_t kFlagAuthoritative = 0x0400;
static const char kServicesName[] = "_services._dns-sd._udp.local";
static const char kMulticastAddress[] = "224.0.0.251";
static const char kMulticastAddressIpv6[] = "ff02::fb";

class PacketWriter {
public:
    bool put_u8(std::uint8_t value);
    bool put_u16(std::uint16_t value);
    bool put_u32(std::uint32_t value);
    bool put_bytes(const std::uint8_t* data, std::size_t length);
    bool put_name(const std::string& name);

    std::size_t size() const;
    std::uint8_t& operator[](std::size_t pos);
    const std::vector<std::uint8_t>& bytes() const;
    std::vector<std::uint8_t> take();

private:
    std::vector<std::uint8_t> bytes_;
};

std::uint16_t read_u16(const std::uint8_t* bytes);
std::uint32_t read_u32(const std::uint8_t* bytes);
bool read_name(const std::uint8_t* packet, std::size_t packet_len, std::size_t& offset,
               std::string& name);
bool name_equals(const std::string& left, const std::string& right);
bool query_type_matches(std::uint16_t query_type, std::uint16_t record_type);

} // namespace internal
} // namespace mdns
} // namespace protocol
} // namespace aplay
