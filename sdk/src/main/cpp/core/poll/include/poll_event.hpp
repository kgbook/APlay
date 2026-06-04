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

#ifndef APLAY_CORE_POLL_EVENT_HPP
#define APLAY_CORE_POLL_EVENT_HPP

#include <cstdint>

namespace aplay {
namespace core {

static const std::uint32_t kPollReadable = 0x01;
static const std::uint32_t kPollWritable = 0x02;
static const std::uint32_t kPollError = 0x04;

struct PollEvent {
    int fd = -1;
    std::uint32_t events = 0;
};

} // namespace core
} // namespace aplay

#endif // APLAY_CORE_POLL_EVENT_HPP
