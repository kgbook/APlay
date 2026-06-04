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

#include "mdns.hpp"
#include "mdns_internal.hpp"

#include <algorithm>
#include <cctype>
#include <limits>
#include <utility>

namespace aplay {
namespace protocol {
namespace mdns {
namespace internal {

bool PacketWriter::put_u8(std::uint8_t value) {
    if (bytes_.size() + 1 > kMaxPacketSize) {
        return false;
    }
    bytes_.push_back(value);
    return true;
}

bool PacketWriter::put_u16(std::uint16_t value) {
    return put_u8(static_cast<std::uint8_t>((value >> 8) & 0xff)) &&
           put_u8(static_cast<std::uint8_t>(value & 0xff));
}

bool PacketWriter::put_u32(std::uint32_t value) {
    return put_u8(static_cast<std::uint8_t>((value >> 24) & 0xff)) &&
           put_u8(static_cast<std::uint8_t>((value >> 16) & 0xff)) &&
           put_u8(static_cast<std::uint8_t>((value >> 8) & 0xff)) &&
           put_u8(static_cast<std::uint8_t>(value & 0xff));
}

bool PacketWriter::put_bytes(const std::uint8_t* data, std::size_t length) {
    if (bytes_.size() + length > kMaxPacketSize) {
        return false;
    }
    if (length == 0) {
        return true;
    }
    bytes_.insert(bytes_.end(), data, data + length);
    return true;
}

bool PacketWriter::put_name(const std::string& name) {
    std::size_t start = 0;
    while (start < name.size()) {
        const std::size_t dot = name.find('.', start);
        const std::size_t end = dot == std::string::npos ? name.size() : dot;
        const std::size_t length = end - start;
        if (length > 63 || !put_u8(static_cast<std::uint8_t>(length))) {
            return false;
        }
        if (length > 0 &&
            !put_bytes(reinterpret_cast<const std::uint8_t*>(name.data() + start), length)) {
            return false;
        }
        if (dot == std::string::npos) {
            break;
        }
        start = dot + 1;
    }
    return put_u8(0);
}

std::size_t PacketWriter::size() const {
    return bytes_.size();
}

std::uint8_t& PacketWriter::operator[](std::size_t pos) {
    return bytes_[pos];
}

const std::vector<std::uint8_t>& PacketWriter::bytes() const {
    return bytes_;
}

std::vector<std::uint8_t> PacketWriter::take() {
    return std::move(bytes_);
}

std::uint16_t read_u16(const std::uint8_t* bytes) {
    return static_cast<std::uint16_t>((static_cast<std::uint16_t>(bytes[0]) << 8) | bytes[1]);
}

std::uint32_t read_u32(const std::uint8_t* bytes) {
    return (static_cast<std::uint32_t>(bytes[0]) << 24) |
           (static_cast<std::uint32_t>(bytes[1]) << 16) |
           (static_cast<std::uint32_t>(bytes[2]) << 8) |
           static_cast<std::uint32_t>(bytes[3]);
}

bool read_name(const std::uint8_t* packet, std::size_t packet_len, std::size_t& offset,
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

bool name_equals(const std::string& left, const std::string& right) {
    return left.size() == right.size() &&
           std::equal(left.begin(), left.end(), right.begin(), [](char a, char b) {
               return std::tolower(static_cast<unsigned char>(a)) ==
                      std::tolower(static_cast<unsigned char>(b));
           });
}

bool query_type_matches(std::uint16_t query_type, std::uint16_t record_type) {
    return query_type == record_type || query_type == kTypeAny;
}

} // namespace internal

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

bool TxtRecord::add(const std::string& key, const std::string& value) {
    if (key.empty() || key.find('=') != std::string::npos) {
        return false;
    }
    const std::size_t length = key.size() + 1 + value.size();
    if (length > std::numeric_limits<std::uint8_t>::max() || bytes.size() + 1 + length > 512) {
        return false;
    }
    bytes.push_back(static_cast<std::uint8_t>(length));
    bytes.insert(bytes.end(), key.begin(), key.end());
    bytes.push_back('=');
    bytes.insert(bytes.end(), value.begin(), value.end());
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
        summary.questions.push_back(std::move(question));
    }

    for (std::uint16_t i = 0; i < ancount; ++i) {
        RecordSummary record;
        if (!MdnsParser::parse_record(bytes, length, offset, record)) {
            return false;
        }
        summary.answers.push_back(std::move(record));
    }

    return true;
}

bool parse_packet(const std::uint8_t* bytes, std::size_t length, PacketSummary& summary) {
    return MdnsParser::parse_packet(bytes, length, summary);
}

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
