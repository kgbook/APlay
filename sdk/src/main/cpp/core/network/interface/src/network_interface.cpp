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

#include "network_interface.hpp"

namespace aplay {
namespace core {
namespace network {

bool parse_ipv4_address(const std::string& text, std::uint32_t& address) {
    std::uint32_t result = 0;
    std::size_t start = 0;
    for (int part = 0; part < 4; ++part) {
        const std::size_t dot = text.find('.', start);
        const std::size_t end = dot == std::string::npos ? text.size() : dot;
        if (end == start) {
            return false;
        }

        int value = 0;
        for (std::size_t i = start; i < end; ++i) {
            if (text[i] < '0' || text[i] > '9') {
                return false;
            }
            value = value * 10 + (text[i] - '0');
            if (value > 255) {
                return false;
            }
        }

        result = (result << 8) | static_cast<std::uint32_t>(value);
        if (part < 3 && dot == std::string::npos) {
            return false;
        }
        start = end + 1;
    }

    if (start <= text.size()) {
        return false;
    }
    address = result;
    return true;
}

} // namespace network
} // namespace core
} // namespace aplay
