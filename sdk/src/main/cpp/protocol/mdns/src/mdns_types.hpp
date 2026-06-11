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

#include "mdns_service.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace aplay {
namespace protocol {
namespace mdns {

struct ResponderConfig {
    std::string host_name = "APlay.local";
    std::uint32_t ipv4_address = 0; // Host byte order.
    std::vector<std::uint32_t> ipv4_addresses; // Host byte order.
    std::array<std::uint8_t, 16> ipv6_address{};
    Service airplay;
    Service raop;
};

struct QuestionSummary {
    std::string name;
    std::uint16_t type = 0;
    std::uint16_t dns_class = 0;
};

struct RecordSummary {
    std::string name;
    std::uint16_t type = 0;
    std::uint16_t dns_class = 0;
    std::uint32_t ttl = 0;
    std::string ptr_name;
    std::string srv_target;
    std::uint16_t srv_port = 0;
    std::uint32_t ipv4_address = 0; // Network byte order.
    std::array<std::uint8_t, 16> ipv6_address{};
    std::vector<std::string> txt;
};

struct PacketSummary {
    std::uint16_t flags = 0;
    std::vector<QuestionSummary> questions;
    std::vector<RecordSummary> answers;
};

struct ResponsePlan {
    std::vector<std::vector<std::uint8_t> > packets;
    bool include_airplay = false;
    bool include_raop = false;
    bool include_host = false;
    bool wants_unicast = false;
};

} // namespace mdns
} // namespace protocol
} // namespace aplay
