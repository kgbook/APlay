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

#include "binary_io.hpp"

#include <utility>

namespace aplay {
namespace core {
namespace binary {

std::uint16_t read_u16_be(const std::uint8_t* bytes) {
    return static_cast<std::uint16_t>((static_cast<std::uint16_t>(bytes[0]) << 8) | bytes[1]);
}

std::uint32_t read_u32_be(const std::uint8_t* bytes) {
    return (static_cast<std::uint32_t>(bytes[0]) << 24) |
           (static_cast<std::uint32_t>(bytes[1]) << 16) |
           (static_cast<std::uint32_t>(bytes[2]) << 8) |
           static_cast<std::uint32_t>(bytes[3]);
}

Writer::Writer(std::size_t max_size)
    : max_size_(max_size), bytes_() {}

bool Writer::put_u8(std::uint8_t value) {
    if (bytes_.size() + 1 > max_size_) {
        return false;
    }
    bytes_.push_back(value);
    return true;
}

bool Writer::put_u16_be(std::uint16_t value) {
    return put_u8(static_cast<std::uint8_t>((value >> 8) & 0xff)) &&
           put_u8(static_cast<std::uint8_t>(value & 0xff));
}

bool Writer::put_u32_be(std::uint32_t value) {
    return put_u8(static_cast<std::uint8_t>((value >> 24) & 0xff)) &&
           put_u8(static_cast<std::uint8_t>((value >> 16) & 0xff)) &&
           put_u8(static_cast<std::uint8_t>((value >> 8) & 0xff)) &&
           put_u8(static_cast<std::uint8_t>(value & 0xff));
}

bool Writer::put_bytes(const std::uint8_t* data, std::size_t length) {
    if (bytes_.size() + length > max_size_) {
        return false;
    }
    if (length == 0) {
        return true;
    }
    bytes_.insert(bytes_.end(), data, data + length);
    return true;
}

std::size_t Writer::size() const {
    return bytes_.size();
}

std::uint8_t& Writer::operator[](std::size_t pos) {
    return bytes_[pos];
}

std::vector<std::uint8_t> Writer::take() {
    return std::move(bytes_);
}

} // namespace binary
} // namespace core
} // namespace aplay
