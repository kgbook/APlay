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

#ifndef APLAY_CORE_EVENT_LOOP_HPP
#define APLAY_CORE_EVENT_LOOP_HPP

#include <functional>
#include <map>

namespace aplay {
namespace core {

class EventLoop {
public:
    typedef std::function<void(int fd)> FdCallback;

    EventLoop();
    ~EventLoop();

    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    bool add_reader(int fd, FdCallback callback);
    void remove(int fd);
    bool run_once(int timeout_ms);
    void run();
    void stop();
    void wake();

private:
    class Impl;
    Impl* impl_;
};

} // namespace core
} // namespace aplay

#endif // APLAY_CORE_EVENT_LOOP_HPP
