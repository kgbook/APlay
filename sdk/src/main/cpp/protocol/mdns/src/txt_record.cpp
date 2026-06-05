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

#include "impl/txt_record.hpp"

#include <limits>

namespace aplay {
namespace protocol {
namespace mdns {

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

} // namespace mdns
} // namespace protocol
} // namespace aplay
