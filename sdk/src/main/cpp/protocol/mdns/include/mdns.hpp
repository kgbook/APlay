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

#include "singleton.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace aplay {
namespace protocol {
namespace mdns {

static const std::uint16_t kPort = 5353;
static const std::uint32_t kServiceTtl = 4500;
static const std::uint32_t kHostTtl = 120;

static const std::uint16_t kTypeA = 1;
static const std::uint16_t kTypePtr = 12;
static const std::uint16_t kTypeTxt = 16;
static const std::uint16_t kTypeSrv = 33;
static const std::uint16_t kTypeAny = 255;
static const std::uint16_t kClassIn = 1;
static const std::uint16_t kCacheFlush = 0x8000;
static const std::uint16_t kUnicastResponse = 0x8000;

struct TxtRecord {
    std::vector<std::uint8_t> bytes;

    bool add(const std::string& key, const std::string& value);
};

struct Service {
    std::string type;
    std::string instance;
    std::uint16_t port = 0;
    TxtRecord txt;
    bool registered = false;
};

struct ResponderConfig {
    std::string host_name = "APlay.local";
    std::uint32_t ipv4_address = 0; // Network byte order.
    Service airplay;
    Service raop;
};

struct ResponsePlan {
    std::vector<std::vector<std::uint8_t>> packets;
    bool wants_unicast = false;
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
    std::vector<std::string> txt;
};

struct PacketSummary {
    std::uint16_t flags = 0;
    std::vector<QuestionSummary> questions;
    std::vector<RecordSummary> answers;
};

class MdnsResponder : public core::Singleton<MdnsResponder> {
public:
    void set_config(ResponderConfig config);
    const ResponderConfig& config() const;

    std::vector<std::vector<std::uint8_t>> build_announcement(std::uint32_t ttl) const;
    std::vector<std::uint8_t> build_goodbye(const Service& service) const;
    ResponsePlan handle_query(const std::uint8_t* bytes, std::size_t length) const;

    int start();
    void stop();
    void announce(std::uint32_t ttl = kServiceTtl);

private:
    friend class core::Singleton<MdnsResponder>;

    MdnsResponder();
    ~MdnsResponder();

    ResponderConfig config_;

    class Impl;
    Impl* impl_;
};

class MdnsParser {
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
