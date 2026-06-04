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

#include "poller_impl.hpp"

namespace aplay {
namespace core {

namespace {

Poller::Impl* create_poller_impl() {
#if defined(__linux__) || defined(__ANDROID__)
    return create_epoll_poller_impl();
#else
    return create_poll_poller_impl();
#endif
}

} // namespace

Poller::Poller() : impl_(create_poller_impl()) {}

Poller::~Poller() {
    delete impl_;
}

bool Poller::add(int fd, std::uint32_t events) {
    return impl_->add(fd, events);
}

bool Poller::update(int fd, std::uint32_t events) {
    return impl_->update(fd, events);
}

void Poller::remove(int fd) {
    impl_->remove(fd);
}

int Poller::wait(std::vector<PollEvent>& events, int timeout_ms) {
    return impl_->wait(events, timeout_ms);
}

} // namespace core
} // namespace aplay
