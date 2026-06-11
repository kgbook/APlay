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

#include "event_loop.hpp"
#include "poll.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <map>
#include <atomic>
#include <vector>

namespace aplay {
namespace core {

static void set_nonblocking(int fd) {
    const int flags = ::fcntl(fd, F_GETFL, 0);
    if (flags != -1) {
        ::fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }
}

class EventLoop::Impl {
public:
    Impl() : stopping_(false) {
        wake_fds_[0] = -1;
        wake_fds_[1] = -1;
        if (::pipe(wake_fds_) == 0) {
            set_nonblocking(wake_fds_[0]);
            set_nonblocking(wake_fds_[1]);
            poller_.add(wake_fds_[0], kPollReadable);
        }
    }

    ~Impl() {
        if (wake_fds_[0] != -1) {
            poller_.remove(wake_fds_[0]);
            ::close(wake_fds_[0]);
        }
        if (wake_fds_[1] != -1) {
            ::close(wake_fds_[1]);
        }
    }

    bool add_reader(const int fd, const FdCallback& callback) {
        if (fd == -1 || !callback) {
            return false;
        }
        stopping_.store(false);
        callbacks_[fd] = callback;
        return poller_.add(fd, kPollReadable) || poller_.update(fd, kPollReadable);
    }

    void remove(int fd) {
        callbacks_.erase(fd);
        poller_.remove(fd);
    }

    bool run_once(int timeout_ms) {
        std::vector<PollEvent> events;
        const int ready = poller_.wait(events, timeout_ms);
        if (ready < 0) {
            return false;
        }
        for (std::size_t i = 0; i < events.size(); ++i) {
            if (events[i].fd == wake_fds_[0]) {
                consume_wake();
                continue;
            }
            std::map<int, FdCallback>::iterator it = callbacks_.find(events[i].fd);
            if (it != callbacks_.end()) {
                it->second(events[i].fd);
            }
        }
        return !stopping_.load();
    }

    void run() {
        stopping_.store(false);
        while (!stopping_.load() && run_once(-1)) {
        }
    }

    void stop() {
        stopping_.store(true);
        wake();
    }

    void wake() {
        if (wake_fds_[1] == -1) {
            return;
        }
        const char byte = 1;
        ::write(wake_fds_[1], &byte, 1);
    }

private:
    void consume_wake() const
    {
        char byte;
        while (::read(wake_fds_[0], &byte, 1) > 0) {
        }
    }

    Poller poller_;
    std::map<int, FdCallback> callbacks_;
    std::atomic<bool> stopping_;
    int wake_fds_[2];
};

EventLoop::EventLoop() : impl_(new Impl()) {}

EventLoop::~EventLoop() {
    delete impl_;
}

bool EventLoop::add_reader(const int fd, const FdCallback& callback) const
{
    return impl_->add_reader(fd, callback);
}

void EventLoop::remove(int fd) const
{
    impl_->remove(fd);
}

bool EventLoop::run_once(int timeout_ms) const
{
    return impl_->run_once(timeout_ms);
}

void EventLoop::run() const
{
    impl_->run();
}

void EventLoop::stop() const
{
    impl_->stop();
}

void EventLoop::wake() const
{
    impl_->wake();
}

} // namespace core
} // namespace aplay
