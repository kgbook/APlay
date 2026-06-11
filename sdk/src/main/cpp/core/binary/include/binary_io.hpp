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
#include <vector>

namespace aplay {
namespace core {
namespace binary {

std::uint16_t read_u16_be(const std::uint8_t* bytes);
std::uint32_t read_u32_be(const std::uint8_t* bytes);

class Writer {
public:
    explicit Writer(std::size_t max_size);

    bool put_u8(std::uint8_t value);
    bool put_u16_be(std::uint16_t value);
    bool put_u32_be(std::uint32_t value);
    bool put_bytes(const std::uint8_t* data, std::size_t length);

    std::size_t size() const;
    std::uint8_t& operator[](std::size_t pos);
    std::vector<std::uint8_t> take();

private:
    std::size_t max_size_;
    std::vector<std::uint8_t> bytes_;
};

} // namespace binary
} // namespace core
} // namespace aplay
