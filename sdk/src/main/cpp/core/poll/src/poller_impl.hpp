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

#ifndef APLAY_CORE_POLLER_IMPL_HPP
#define APLAY_CORE_POLLER_IMPL_HPP

#include "poll.hpp"

namespace aplay {
namespace core {

class Poller::Impl {
public:
    virtual ~Impl() {}

    virtual bool add(int fd, std::uint32_t events) = 0;
    virtual bool update(int fd, std::uint32_t events) = 0;
    virtual void remove(int fd) = 0;
    virtual int wait(std::vector<PollEvent>& events, int timeout_ms) = 0;
};

Poller::Impl* create_epoll_poller_impl();
Poller::Impl* create_poll_poller_impl();

} // namespace core
} // namespace aplay

#endif // APLAY_CORE_POLLER_IMPL_HPP
