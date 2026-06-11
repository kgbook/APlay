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

#include <cstdint>

namespace aplay {
namespace protocol {
namespace mdns {

static const std::uint16_t kPort = 5353;
static const std::uint32_t kServiceTtl = 4500;
static const std::uint32_t kHostTtl = 120;

enum class AddressFamily {
    Ipv4,
    Ipv6,
};

static const std::uint16_t kTypeA = 1;
static const std::uint16_t kTypePtr = 12;
static const std::uint16_t kTypeTxt = 16;
static const std::uint16_t kTypeAaaa = 28;
static const std::uint16_t kTypeSrv = 33;
static const std::uint16_t kTypeAny = 255;
static const std::uint16_t kClassIn = 1;
static const std::uint16_t kCacheFlush = 0x8000;
static const std::uint16_t kUnicastResponse = 0x8000;

} // namespace mdns
} // namespace protocol
} // namespace aplay
