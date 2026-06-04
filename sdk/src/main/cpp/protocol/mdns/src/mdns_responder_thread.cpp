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

#include "mdns_responder_thread.hpp"

namespace aplay {
namespace protocol {
namespace mdns {

MdnsResponderThread::MdnsResponderThread(const std::function<bool()>& run_once)
    : core::Thread("aplay-mdns"), run_once_(run_once) {}

bool MdnsResponderThread::runOnce() {
    return run_once_();
}

} // namespace mdns
} // namespace protocol
} // namespace aplay
