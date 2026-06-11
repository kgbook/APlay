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

#ifndef APLAY_CORE_POLL_HPP
#define APLAY_CORE_POLL_HPP

#include <cstdint>
#include <vector>

namespace aplay {
namespace core {

static const std::uint32_t kPollReadable = 0x01;
static const std::uint32_t kPollWritable = 0x02;
static const std::uint32_t kPollError = 0x04;

struct PollEvent {
    int fd = -1;
    std::uint32_t events = 0;
};

class Poller {
public:
    class Impl;

    Poller();
    ~Poller();

    Poller(const Poller&) = delete;
    Poller& operator=(const Poller&) = delete;

    bool add(int fd, std::uint32_t events);
    bool update(int fd, std::uint32_t events);
    void remove(int fd);
    int wait(std::vector<PollEvent>& events, int timeout_ms);

private:
    Impl* impl_;
};

} // namespace core
} // namespace aplay

#endif // APLAY_CORE_POLL_HPP
