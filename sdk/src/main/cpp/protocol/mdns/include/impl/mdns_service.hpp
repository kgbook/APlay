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

#include "txt_record.hpp"

#include <cstdint>
#include <string>

namespace aplay {
namespace protocol {
namespace mdns {

struct Service {
    std::string type;
    std::string instance;
    std::uint16_t port = 0;
    TxtRecord txt;
    bool registered = false;
};

} // namespace mdns
} // namespace protocol
} // namespace aplay
