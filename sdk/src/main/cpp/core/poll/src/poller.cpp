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

#include "poller.hpp"

#include <algorithm>
#include <cerrno>
#include <cstring>

#if defined(__linux__) || defined(__ANDROID__)
#include <sys/epoll.h>
#include <unistd.h>
#else
#include <poll.h>
#endif

namespace aplay {
namespace core {
namespace {

#if defined(__linux__) || defined(__ANDROID__)
std::uint32_t to_native_events(std::uint32_t events) {
    std::uint32_t native = 0;
    if ((events & kPollReadable) != 0) {
        native |= EPOLLIN;
    }
    if ((events & kPollWritable) != 0) {
        native |= EPOLLOUT;
    }
    return native;
}

std::uint32_t from_native_events(std::uint32_t events) {
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
#else
short to_native_events(std::uint32_t events) {
    short native = 0;
    if ((events & kPollReadable) != 0) {
        native = static_cast<short>(native | POLLIN);
    }
    if ((events & kPollWritable) != 0) {
        native = static_cast<short>(native | POLLOUT);
    }
    return native;
}

std::uint32_t from_native_events(short events) {
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
#endif

} // namespace

class Poller::Impl {
public:
    Impl() {
#if defined(__linux__) || defined(__ANDROID__)
        epoll_fd_ = ::epoll_create1(EPOLL_CLOEXEC);
#endif
    }

    ~Impl() {
#if defined(__linux__) || defined(__ANDROID__)
        if (epoll_fd_ != -1) {
            ::close(epoll_fd_);
        }
#endif
    }

    bool add(int fd, std::uint32_t events) {
        if (fd == -1) {
            return false;
        }
#if defined(__linux__) || defined(__ANDROID__)
        if (epoll_fd_ == -1) {
            return false;
        }
        epoll_event event;
        std::memset(&event, 0, sizeof(event));
        event.events = to_native_events(events);
        event.data.fd = fd;
        return ::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event) == 0;
#else
        remove(fd);
        pollfd item;
        std::memset(&item, 0, sizeof(item));
        item.fd = fd;
        item.events = to_native_events(events);
        poll_fds_.push_back(item);
        return true;
#endif
    }

    bool update(int fd, std::uint32_t events) {
        if (fd == -1) {
            return false;
        }
#if defined(__linux__) || defined(__ANDROID__)
        if (epoll_fd_ == -1) {
            return false;
        }
        epoll_event event;
        std::memset(&event, 0, sizeof(event));
        event.events = to_native_events(events);
        event.data.fd = fd;
        return ::epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &event) == 0;
#else
        for (std::size_t i = 0; i < poll_fds_.size(); ++i) {
            if (poll_fds_[i].fd == fd) {
                poll_fds_[i].events = to_native_events(events);
                return true;
            }
        }
        return add(fd, events);
#endif
    }

    void remove(int fd) {
        if (fd == -1) {
            return;
        }
#if defined(__linux__) || defined(__ANDROID__)
        if (epoll_fd_ != -1) {
            ::epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, NULL);
        }
#else
        poll_fds_.erase(std::remove_if(poll_fds_.begin(), poll_fds_.end(),
                                       [fd](const pollfd& item) { return item.fd == fd; }),
                        poll_fds_.end());
#endif
    }

    int wait(std::vector<PollEvent>& events, int timeout_ms) {
        events.clear();
#if defined(__linux__) || defined(__ANDROID__)
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
#else
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
            event.events = from_native_events(poll_fds_[i].revents);
            events.push_back(event);
        }
        return ready;
#endif
    }

private:
#if defined(__linux__) || defined(__ANDROID__)
    int epoll_fd_ = -1;
#else
    std::vector<pollfd> poll_fds_;
#endif
};

Poller::Poller() : impl_(new Impl()) {}

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
