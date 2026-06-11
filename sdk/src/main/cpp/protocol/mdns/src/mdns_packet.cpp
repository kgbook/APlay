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

#include "mdns_packet.hpp"
#include "mdns_constants.hpp"

namespace aplay {
namespace protocol {
namespace mdns {

static bool read_name(const std::uint8_t* packet, std::size_t packet_len, std::size_t& offset,
               std::string& name) {
    std::size_t pos = offset;
    std::string out;
    bool jumped = false;
    int jumps = 0;

    while (pos < packet_len) {
        const std::uint8_t length = packet[pos++];
        if (length == 0) {
            if (!jumped) {
                offset = pos;
            }
            name = out;
            return true;
        }

        if ((length & 0xc0) == 0xc0) {
            if (pos >= packet_len || ++jumps > 16) {
                return false;
            }
            const std::size_t ptr = (static_cast<std::size_t>(length & 0x3f) << 8) | packet[pos++];
            if (ptr >= packet_len) {
                return false;
            }
            if (!jumped) {
                offset = pos;
            }
            pos = ptr;
            jumped = true;
            continue;
        }

        if ((length & 0xc0) != 0 || pos + length > packet_len) {
            return false;
        }
        if (!out.empty()) {
            out.push_back('.');
        }
        out.append(reinterpret_cast<const char*>(packet + pos), length);
        pos += length;
    }

    return false;
}

bool put_name(core::binary::Writer& packet, const std::string& name) {
    std::size_t start = 0;
    while (start < name.size()) {
        const std::size_t dot = name.find('.', start);
        const std::size_t end = dot == std::string::npos ? name.size() : dot;
        const std::size_t length = end - start;
        if (length > 63 || !packet.put_u8(static_cast<std::uint8_t>(length))) {
            return false;
        }
        if (length > 0 &&
            !packet.put_bytes(reinterpret_cast<const std::uint8_t*>(name.data() + start),
                              length)) {
            return false;
        }
        if (dot == std::string::npos) {
            break;
        }
        start = dot + 1;
    }
    return packet.put_u8(0);
}

bool PacketParser::parse_record(const std::uint8_t* bytes, std::size_t length,
                                std::size_t& offset, RecordSummary& record) {
    if (!read_name(bytes, length, offset, record.name) || offset + 10 > length) {
        return false;
    }
    record.type = core::binary::read_u16_be(bytes + offset);
    record.dns_class = core::binary::read_u16_be(bytes + offset + 2);
    record.ttl = core::binary::read_u32_be(bytes + offset + 4);
    const std::uint16_t rdlength = core::binary::read_u16_be(bytes + offset + 8);
    offset += 10;
    if (offset + rdlength > length) {
        return false;
    }

    const std::size_t rdata_offset = offset;
    if (record.type == kTypePtr) {
        std::size_t ptr_offset = offset;
        if (!read_name(bytes, length, ptr_offset, record.ptr_name)) {
            return false;
        }
    } else if (record.type == kTypeSrv) {
        if (rdlength < 6) {
            return false;
        }
        record.srv_port = core::binary::read_u16_be(bytes + offset + 4);
        std::size_t target_offset = offset + 6;
        if (!read_name(bytes, length, target_offset, record.srv_target)) {
            return false;
        }
    } else if (record.type == kTypeA) {
        if (rdlength != 4) {
            return false;
        }
        record.ipv4_address = core::binary::read_u32_be(bytes + offset);
    } else if (record.type == kTypeAaaa) {
        if (rdlength != record.ipv6_address.size()) {
            return false;
        }
        for (std::size_t i = 0; i < record.ipv6_address.size(); ++i) {
            record.ipv6_address[i] = bytes[offset + i];
        }
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

bool PacketParser::parse_question(const std::uint8_t* bytes, std::size_t length,
                                  std::size_t& offset, QuestionSummary& question) {
    if (!read_name(bytes, length, offset, question.name) || offset + 4 > length) {
        return false;
    }
    question.type = core::binary::read_u16_be(bytes + offset);
    question.dns_class = core::binary::read_u16_be(bytes + offset + 2);
    offset += 4;
    return true;
}

bool PacketParser::parse_packet(const std::uint8_t* bytes, std::size_t length,
                                PacketSummary& summary) {
    if (length < 12) {
        return false;
    }

    summary = PacketSummary();
    summary.flags = core::binary::read_u16_be(bytes + 2);
    const std::uint16_t qdcount = core::binary::read_u16_be(bytes + 4);
    const std::uint16_t ancount = core::binary::read_u16_be(bytes + 6);
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
        if (!PacketParser::parse_record(bytes, length, offset, record)) {
            return false;
        }
        summary.answers.push_back(record);
    }

    return true;
}

bool parse_packet(const std::uint8_t* bytes, std::size_t length, PacketSummary& summary) {
    return PacketParser::parse_packet(bytes, length, summary);
}

std::vector<std::uint8_t> build_ptr_query(const std::vector<std::string>& names,
                                          bool request_unicast) {
    core::binary::Writer packet(kMaxPacketSize);
    packet.put_u16_be(0);
    packet.put_u16_be(0);
    packet.put_u16_be(static_cast<std::uint16_t>(names.size()));
    packet.put_u16_be(0);
    packet.put_u16_be(0);
    packet.put_u16_be(0);
    for (const std::string& name : names) {
        put_name(packet, name);
        packet.put_u16_be(kTypePtr);
        packet.put_u16_be(kClassIn | (request_unicast ? kUnicastResponse : 0));
    }
    return packet.take();
}

} // namespace mdns
} // namespace protocol
} // namespace aplay
