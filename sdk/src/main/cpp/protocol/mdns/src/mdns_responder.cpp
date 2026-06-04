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

#include "mdns.hpp"
#include "mdns_internal.hpp"

#include "event_loop.hpp"
#include "socket.hpp"
#include "thread.hpp"

#include <array>
#include <functional>
#include <mutex>
#include <utility>
#include <vector>

namespace aplay {
namespace protocol {
namespace mdns {
namespace {

core::socket::Ipv4Endpoint multicast_endpoint() {
    return core::socket::make_ipv4_endpoint(internal::kMulticastAddress, kPort);
}

} // namespace

class MdnsResponder::Impl {
public:
    explicit Impl(MdnsResponder& responder)
        : responder_(responder), socket_(), loop_(), thread_(*this) {}

    ~Impl() {
        stop();
    }

    int start() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (thread_.isRunning()) {
            return 0;
        }

        ResponderConfig config = responder_.config_;
        if (config.ipv4_address == 0) {
            config.ipv4_address =
                core::socket::default_ipv4_address_for_route(internal::kMulticastAddress, kPort);
            responder_.config_ = config;
        }

        socket_ = core::socket::open_ipv4_udp_multicast_socket(
            kPort, internal::kMulticastAddress, responder_.config_.ipv4_address);
        if (!socket_.valid()) {
            return -1;
        }

        if (!loop_.add_reader(socket_.fd(), std::bind(&Impl::handle_readable, this,
                                                      std::placeholders::_1))) {
            socket_.close();
            return -1;
        }

        thread_.start();
        return 0;
    }

    void stop() {
        loop_.stop();
        thread_.stopAndJoin();

        std::lock_guard<std::mutex> lock(mutex_);
        if (socket_.valid()) {
            loop_.remove(socket_.fd());
            socket_.close();
        }
    }

    void set_config(ResponderConfig config) {
        std::lock_guard<std::mutex> lock(mutex_);
        responder_.config_ = std::move(config);
    }

    void announce(std::uint32_t ttl) {
        std::vector<std::vector<std::uint8_t> > packets;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            packets = responder_.build_announcement(ttl);
        }
        send_packets(packets, multicast_endpoint(), false);
    }

private:
    class ResponderThread : public core::Thread {
    public:
        explicit ResponderThread(Impl& owner) : core::Thread("aplay-mdns"), owner_(owner) {}

    protected:
        bool runOnce() override {
            return owner_.loop_.run_once(250);
        }

    private:
        Impl& owner_;
    };

    void handle_readable(int fd) {
        (void)fd;
        std::array<std::uint8_t, internal::kMaxPacketSize> buffer;
        core::socket::Ipv4Endpoint from;
        int received = -1;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            received = socket_.receive_from(buffer.data(), buffer.size(), from);
        }
        if (received <= 0) {
            return;
        }

        ResponsePlan plan;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            plan = responder_.handle_query(buffer.data(), static_cast<std::size_t>(received));
        }

        send_packets(plan.packets, multicast_endpoint(), false);
        if (plan.wants_unicast || from.port != kPort) {
            send_packets(plan.packets, from, true);
        }
    }

    void send_packets(const std::vector<std::vector<std::uint8_t> >& packets,
                      const core::socket::Ipv4Endpoint& endpoint, bool allow_empty_endpoint) {
        if (!allow_empty_endpoint && endpoint.address == 0) {
            return;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        for (std::size_t i = 0; i < packets.size(); ++i) {
            socket_.send_to(packets[i].data(), packets[i].size(), endpoint);
        }
    }

    MdnsResponder& responder_;
    std::mutex mutex_;
    core::socket::UdpSocket socket_;
    core::EventLoop loop_;
    ResponderThread thread_;
};

MdnsResponder::MdnsResponder()
    : config_(), impl_(new Impl(*this)) {}

MdnsResponder::~MdnsResponder() {
    delete impl_;
}

void MdnsResponder::set_config(ResponderConfig config) {
    impl_->set_config(std::move(config));
}

int MdnsResponder::start() {
    return impl_->start();
}

void MdnsResponder::stop() {
    impl_->stop();
}

void MdnsResponder::announce(std::uint32_t ttl) {
    impl_->announce(ttl);
}

} // namespace mdns
} // namespace protocol
} // namespace aplay
