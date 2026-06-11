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

#if defined(__linux__) || defined(__ANDROID__)

#include <cstring>

#include <sys/epoll.h>
#include <unistd.h>

namespace aplay {
namespace core {

static std::uint32_t to_native_events(std::uint32_t events) {
    std::uint32_t native = 0;
    if ((events & kPollReadable) != 0) {
        native |= EPOLLIN;
    }
    if ((events & kPollWritable) != 0) {
        native |= EPOLLOUT;
    }
    return native;
}

static std::uint32_t from_native_events(std::uint32_t events) {
    std::uint32_t out = 0;
    if ((events & EPOLLIN) != 0) {
        out |= kPollReadable;
    }
    if ((events & EPOLLOUT) != 0) {
        out |= kPollWritable;
    }
    if ((events & (EPOLLERR | EPOLLHUP)) != 0) {
        out |= kPollError;
    }
    return out;
}

class EpollPollerImpl : public Poller::Impl {
public:
    EpollPollerImpl() : epoll_fd_(::epoll_create1(EPOLL_CLOEXEC)) {}

    ~EpollPollerImpl() override {
        if (epoll_fd_ != -1) {
            ::close(epoll_fd_);
        }
    }

    bool add(int fd, std::uint32_t events) override {
        if (fd == -1 || epoll_fd_ == -1) {
            return false;
        }
        epoll_event event;
        std::memset(&event, 0, sizeof(event));
        event.events = to_native_events(events);
        event.data.fd = fd;
        return ::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event) == 0;
    }

    bool update(int fd, std::uint32_t events) override {
        if (fd == -1 || epoll_fd_ == -1) {
            return false;
        }
        epoll_event event;
        std::memset(&event, 0, sizeof(event));
        event.events = to_native_events(events);
        event.data.fd = fd;
        return ::epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &event) == 0;
    }

    void remove(int fd) override {
        if (fd != -1 && epoll_fd_ != -1) {
            ::epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, NULL);
        }
    }

    int wait(std::vector<PollEvent>& events, int timeout_ms) override {
        events.clear();
        if (epoll_fd_ == -1) {
            return -1;
        }
        epoll_event native_events[16];
        const int ready = ::epoll_wait(epoll_fd_, native_events, 16, timeout_ms);
        if (ready <= 0) {
            return ready;
        }
        for (int i = 0; i < ready; ++i) {
            PollEvent event;
            event.fd = native_events[i].data.fd;
            event.events = from_native_events(native_events[i].events);
            events.push_back(event);
        }
        return ready;
    }

private:
    int epoll_fd_;
};

Poller::Impl* create_epoll_poller_impl() {
    return new EpollPollerImpl();
}

} // namespace core
} // namespace aplay

#endif
