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

#include "impl/mdns_responder.hpp"
#include "impl/mdns_parser.hpp"
#include "mdns_internal.hpp"

#include <array>

namespace aplay {
namespace protocol {
namespace mdns {
namespace {

using internal::PacketWriter;

bool begin_response(PacketWriter& packet, std::size_t& answer_count_pos) {
    if (!packet.put_u16(0) || !packet.put_u16(internal::kFlagResponse | internal::kFlagAuthoritative) ||
        !packet.put_u16(0)) {
        return false;
    }
    answer_count_pos = packet.size();
    return packet.put_u16(0) && packet.put_u16(0) && packet.put_u16(0);
}

void finish_response(PacketWriter& packet, std::size_t answer_count_pos,
                     std::uint16_t answers) {
    packet[answer_count_pos] = static_cast<std::uint8_t>((answers >> 8) & 0xff);
    packet[answer_count_pos + 1] = static_cast<std::uint8_t>(answers & 0xff);
}

bool begin_rr(PacketWriter& packet, const std::string& name, std::uint16_t type,
              std::uint16_t dns_class, std::uint32_t ttl, std::size_t& rdlength_pos) {
    if (!packet.put_name(name) || !packet.put_u16(type) || !packet.put_u16(dns_class) ||
        !packet.put_u32(ttl)) {
        return false;
    }
    rdlength_pos = packet.size();
    return packet.put_u16(0);
}

void end_rr(PacketWriter& packet, std::size_t rdlength_pos) {
    const std::size_t rdlength = packet.size() - rdlength_pos - 2;
    packet[rdlength_pos] = static_cast<std::uint8_t>((rdlength >> 8) & 0xff);
    packet[rdlength_pos + 1] = static_cast<std::uint8_t>(rdlength & 0xff);
}

bool add_ptr(PacketWriter& packet, const std::string& name, const std::string& ptr,
             std::uint32_t ttl) {
    std::size_t rdlength_pos = 0;
    if (!begin_rr(packet, name, kTypePtr, kClassIn, ttl, rdlength_pos) ||
        !packet.put_name(ptr)) {
        return false;
    }
    end_rr(packet, rdlength_pos);
    return true;
}

bool add_srv(PacketWriter& packet, const std::string& name, std::uint16_t port,
             const std::string& target, std::uint32_t ttl) {
    std::size_t rdlength_pos = 0;
    if (!begin_rr(packet, name, kTypeSrv, kClassIn | kCacheFlush, ttl, rdlength_pos) ||
        !packet.put_u16(0) || !packet.put_u16(0) || !packet.put_u16(port) ||
        !packet.put_name(target)) {
        return false;
    }
    end_rr(packet, rdlength_pos);
    return true;
}

bool add_txt(PacketWriter& packet, const std::string& name, const TxtRecord& txt,
             std::uint32_t ttl) {
    std::size_t rdlength_pos = 0;
    if (!begin_rr(packet, name, kTypeTxt, kClassIn | kCacheFlush, ttl, rdlength_pos) ||
        !packet.put_bytes(txt.bytes.data(), txt.bytes.size())) {
        return false;
    }
    end_rr(packet, rdlength_pos);
    return true;
}

bool add_a(PacketWriter& packet, const std::string& name, std::uint32_t addr,
           std::uint32_t ttl) {
    if (addr == 0) {
        return true;
    }

    std::size_t rdlength_pos = 0;
    const std::array<std::uint8_t, 4> bytes{
        static_cast<std::uint8_t>((addr >> 24) & 0xff),
        static_cast<std::uint8_t>((addr >> 16) & 0xff),
        static_cast<std::uint8_t>((addr >> 8) & 0xff),
        static_cast<std::uint8_t>(addr & 0xff),
    };
    if (!begin_rr(packet, name, kTypeA, kClassIn | kCacheFlush, ttl, rdlength_pos) ||
        !packet.put_bytes(bytes.data(), bytes.size())) {
        return false;
    }
    end_rr(packet, rdlength_pos);
    return true;
}

bool add_service_records(PacketWriter& packet, const Service& service,
                         const std::string& host_name, std::uint32_t ttl,
                         std::uint16_t& answers) {
    if (!service.registered) {
        return true;
    }

    if (!add_ptr(packet, std::string(internal::kServicesName), service.type, ttl) ||
        !add_ptr(packet, service.type, service.instance, ttl) ||
        !add_srv(packet, service.instance, service.port, host_name, ttl == 0 ? 0 : kHostTtl) ||
        !add_txt(packet, service.instance, service.txt, ttl)) {
        return false;
    }
    answers = static_cast<std::uint16_t>(answers + 4);
    return true;
}

bool build_response(const ResponderConfig& config, bool include_airplay, bool include_raop,
                    bool include_host, std::uint32_t ttl,
                    std::vector<std::uint8_t>& response) {
    PacketWriter packet;
    std::size_t answer_count_pos = 0;
    std::uint16_t answers = 0;
    if (!begin_response(packet, answer_count_pos)) {
        return false;
    }
    if (include_airplay &&
        !add_service_records(packet, config.airplay, config.host_name, ttl, answers)) {
        return false;
    }
    if (include_raop && !add_service_records(packet, config.raop, config.host_name, ttl, answers)) {
        return false;
    }
    if (include_host) {
        if (!add_a(packet, config.host_name, config.ipv4_address, ttl == 0 ? 0 : kHostTtl)) {
            return false;
        }
        if (config.ipv4_address != 0) {
            answers = static_cast<std::uint16_t>(answers + 1);
        }
    }
    if (answers == 0) {
        return false;
    }
    finish_response(packet, answer_count_pos, answers);
    response = packet.take();
    return true;
}

void append_response_packets(std::vector<std::vector<std::uint8_t>>& packets,
                             const ResponderConfig& config, bool include_airplay,
                             bool include_raop, bool include_host, std::uint32_t ttl) {
    if (include_airplay && include_raop && config.airplay.registered && config.raop.registered) {
        std::vector<std::uint8_t> combined;
        if (build_response(config, true, true, include_host, ttl, combined) &&
            combined.size() <= internal::kCombinedPacketMax) {
            packets.push_back(combined);
            return;
        }
    }

    if (include_airplay) {
        std::vector<std::uint8_t> packet;
        if (build_response(config, true, false, include_host, ttl, packet)) {
            packets.push_back(packet);
        }
    }
    if (include_raop) {
        std::vector<std::uint8_t> packet;
        if (build_response(config, false, true, include_host, ttl, packet)) {
            packets.push_back(packet);
        }
    }
    if (include_host && !include_airplay && !include_raop) {
        std::vector<std::uint8_t> packet;
        if (build_response(config, false, false, true, ttl, packet)) {
            packets.push_back(packet);
        }
    }
}

bool question_matches_service(const ResponderConfig& config, const std::string& name,
                              std::uint16_t type, bool& airplay, bool& raop, bool& host) {
    if (internal::name_equals(name, std::string(internal::kServicesName)) &&
        internal::query_type_matches(type, kTypePtr)) {
        airplay = airplay || config.airplay.registered;
        raop = raop || config.raop.registered;
        return airplay || raop;
    }

    if (config.airplay.registered &&
        ((internal::name_equals(name, config.airplay.type) &&
          internal::query_type_matches(type, kTypePtr)) ||
         (internal::name_equals(name, config.airplay.instance) &&
          (internal::query_type_matches(type, kTypeTxt) ||
           internal::query_type_matches(type, kTypeSrv))))) {
        airplay = true;
        host = true;
        return true;
    }

    if (config.raop.registered &&
        ((internal::name_equals(name, config.raop.type) &&
          internal::query_type_matches(type, kTypePtr)) ||
         (internal::name_equals(name, config.raop.instance) &&
          (internal::query_type_matches(type, kTypeTxt) ||
           internal::query_type_matches(type, kTypeSrv))))) {
        raop = true;
        host = true;
        return true;
    }

    if (internal::name_equals(name, config.host_name) &&
        internal::query_type_matches(type, kTypeA)) {
        host = true;
        return true;
    }

    return false;
}

} // namespace

const ResponderConfig& MdnsResponder::config() const {
    return config_;
}

std::vector<std::vector<std::uint8_t>> MdnsResponder::build_announcement(std::uint32_t ttl) const {
    std::vector<std::vector<std::uint8_t>> packets;
    append_response_packets(packets, config_, true, true, true, ttl);
    return packets;
}

std::vector<std::uint8_t> MdnsResponder::build_goodbye(const Service& service) const {
    ResponderConfig goodbye_config = config_;
    goodbye_config.airplay = Service();
    goodbye_config.raop = Service();
    bool include_airplay = false;
    bool include_raop = false;
    if (internal::name_equals(service.type, "_airplay._tcp.local")) {
        goodbye_config.airplay = service;
        include_airplay = true;
    } else if (internal::name_equals(service.type, "_raop._tcp.local")) {
        goodbye_config.raop = service;
        include_raop = true;
    }

    std::vector<std::uint8_t> packet;
    if (build_response(goodbye_config, include_airplay, include_raop, false, 0, packet)) {
        return packet;
    }
    return std::vector<std::uint8_t>();
}

ResponsePlan MdnsResponder::handle_query(const std::uint8_t* bytes, std::size_t length) const {
    ResponsePlan plan;
    if (length < 12 || (internal::read_u16(bytes + 2) & internal::kFlagResponse) != 0) {
        return plan;
    }

    const std::uint16_t qdcount = internal::read_u16(bytes + 4);
    std::size_t offset = 12;
    bool include_airplay = false;
    bool include_raop = false;
    bool include_host = false;

    for (std::uint16_t i = 0; i < qdcount; ++i) {
        QuestionSummary question;
        if (!MdnsParser::parse_question(bytes, length, offset, question)) {
            break;
        }

        if ((question.dns_class & kUnicastResponse) != 0) {
            plan.wants_unicast = true;
        }
        question_matches_service(config_, question.name, question.type, include_airplay,
                                 include_raop, include_host);
    }

    if (include_airplay || include_raop || include_host) {
        append_response_packets(plan.packets, config_, include_airplay, include_raop,
                                include_host, kServiceTtl);
    }
    return plan;
}

} // namespace mdns
} // namespace protocol
} // namespace aplay
