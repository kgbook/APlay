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

#if !defined(__linux__) && !defined(__ANDROID__)

#include <algorithm>
#include <cstring>

#include <poll.h>

namespace aplay {
namespace core {
namespace {

short to_poll_events(std::uint32_t events) {
    short native = 0;
    if ((events & kPollReadable) != 0) {
        native = static_cast<short>(native | POLLIN);
    }
    if ((events & kPollWritable) != 0) {
        native = static_cast<short>(native | POLLOUT);
    }
    return native;
}

std::uint32_t from_poll_events(short events) {
    std::uint32_t out = 0;
    if ((events & POLLIN) != 0) {
        out |= kPollReadable;
    }
    if ((events & POLLOUT) != 0) {
        out |= kPollWritable;
    }
    if ((events & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
        out |= kPollError;
    }
    return out;
}

class PollPollerImpl : public Poller::Impl {
public:
    bool add(int fd, std::uint32_t events) override {
        if (fd == -1) {
            return false;
        }
        remove(fd);
        pollfd item;
        std::memset(&item, 0, sizeof(item));
        item.fd = fd;
        item.events = to_poll_events(events);
        poll_fds_.push_back(item);
        return true;
    }

    bool update(int fd, std::uint32_t events) override {
        if (fd == -1) {
            return false;
        }
        for (std::size_t i = 0; i < poll_fds_.size(); ++i) {
            if (poll_fds_[i].fd == fd) {
                poll_fds_[i].events = to_poll_events(events);
                return true;
            }
        }
        return add(fd, events);
    }

    void remove(int fd) override {
        if (fd == -1) {
            return;
        }
        poll_fds_.erase(std::remove_if(poll_fds_.begin(), poll_fds_.end(),
                                       [fd](const pollfd& item) { return item.fd == fd; }),
                        poll_fds_.end());
    }

    int wait(std::vector<PollEvent>& events, int timeout_ms) override {
        events.clear();
        const int ready = ::poll(poll_fds_.empty() ? NULL : &poll_fds_[0],
                                 poll_fds_.size(), timeout_ms);
        if (ready <= 0) {
            return ready;
        }
        for (std::size_t i = 0; i < poll_fds_.size(); ++i) {
            if (poll_fds_[i].revents == 0) {
                continue;
            }
            PollEvent event;
            event.fd = poll_fds_[i].fd;
            event.events = from_poll_events(poll_fds_[i].revents);
            events.push_back(event);
        }
        return ready;
    }

private:
    std::vector<pollfd> poll_fds_;
};

} // namespace

Poller::Impl* create_poll_poller_impl() {
    return new PollPollerImpl();
}

} // namespace core
} // namespace aplay

#endif
