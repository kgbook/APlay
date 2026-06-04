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

#include "mdns_parser.hpp"
#include "mdns_constants.hpp"
#include "mdns_internal.hpp"

namespace aplay {
namespace protocol {
namespace mdns {

bool MdnsParser::parse_record(const std::uint8_t* bytes, std::size_t length,
                              std::size_t& offset, RecordSummary& record) {
    if (!internal::read_name(bytes, length, offset, record.name) || offset + 10 > length) {
        return false;
    }
    record.type = internal::read_u16(bytes + offset);
    record.dns_class = internal::read_u16(bytes + offset + 2);
    record.ttl = internal::read_u32(bytes + offset + 4);
    const std::uint16_t rdlength = internal::read_u16(bytes + offset + 8);
    offset += 10;
    if (offset + rdlength > length) {
        return false;
    }

    const std::size_t rdata_offset = offset;
    if (record.type == kTypePtr) {
        std::size_t ptr_offset = offset;
        if (!internal::read_name(bytes, length, ptr_offset, record.ptr_name)) {
            return false;
        }
    } else if (record.type == kTypeSrv) {
        if (rdlength < 6) {
            return false;
        }
        record.srv_port = internal::read_u16(bytes + offset + 4);
        std::size_t target_offset = offset + 6;
        if (!internal::read_name(bytes, length, target_offset, record.srv_target)) {
            return false;
        }
    } else if (record.type == kTypeA) {
        if (rdlength != 4) {
            return false;
        }
        record.ipv4_address = internal::read_u32(bytes + offset);
    } else if (record.type == kTypeTxt) {
        std::size_t txt_offset = offset;
        while (txt_offset < rdata_offset + rdlength) {
            const std::uint8_t item_length = bytes[txt_offset++];
            if (txt_offset + item_length > rdata_offset + rdlength) {
                return false;
            }
            record.txt.emplace_back(reinterpret_cast<const char*>(bytes + txt_offset),
                                    item_length);
            txt_offset += item_length;
        }
    }

    offset = rdata_offset + rdlength;
    return true;
}

bool MdnsParser::parse_question(const std::uint8_t* bytes, std::size_t length,
                                std::size_t& offset, QuestionSummary& question) {
    if (!internal::read_name(bytes, length, offset, question.name) || offset + 4 > length) {
        return false;
    }
    question.type = internal::read_u16(bytes + offset);
    question.dns_class = internal::read_u16(bytes + offset + 2);
    offset += 4;
    return true;
}

bool MdnsParser::parse_packet(const std::uint8_t* bytes, std::size_t length,
                              PacketSummary& summary) {
    if (length < 12) {
        return false;
    }

    summary = PacketSummary();
    summary.flags = internal::read_u16(bytes + 2);
    const std::uint16_t qdcount = internal::read_u16(bytes + 4);
    const std::uint16_t ancount = internal::read_u16(bytes + 6);
    std::size_t offset = 12;

    for (std::uint16_t i = 0; i < qdcount; ++i) {
        QuestionSummary question;
        if (!parse_question(bytes, length, offset, question)) {
            return false;
        }
        summary.questions.push_back(question);
    }

    for (std::uint16_t i = 0; i < ancount; ++i) {
        RecordSummary record;
        if (!MdnsParser::parse_record(bytes, length, offset, record)) {
            return false;
        }
        summary.answers.push_back(record);
    }

    return true;
}

bool parse_packet(const std::uint8_t* bytes, std::size_t length, PacketSummary& summary) {
    return MdnsParser::parse_packet(bytes, length, summary);
}

} // namespace mdns
} // namespace protocol
} // namespace aplay
