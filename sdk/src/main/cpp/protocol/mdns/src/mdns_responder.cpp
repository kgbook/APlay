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

#include "impl/mdns_responder.hpp"
#include "impl/mdns_parser.hpp"
#include "mdns_internal.hpp"
#include "mdns_responder_thread.hpp"

#include "event_loop.hpp"
#include "network_interface.hpp"
#include "socket.hpp"

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

core::socket::Ipv6Endpoint multicast_endpoint_ipv6(std::uint32_t scope_id) {
    return core::socket::make_ipv6_endpoint(internal::kMulticastAddressIpv6, kPort, scope_id);
}

} // namespace

class MdnsResponder::Impl {
public:
    explicit Impl(MdnsResponder& responder)
        : responder_(responder),
          ipv4_socket_(),
          ipv6_socket_(),
          loop_(),
          thread_(std::bind(&Impl::run_once, this)) {}

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
        }
        const std::array<std::uint8_t, 16> empty_ipv6{};
        if (config.ipv6_address == empty_ipv6) {
            core::network::default_ipv6_address(config.ipv6_address);
        }
        responder_.config_ = config;

        ipv4_socket_ = core::socket::open_ipv4_udp_multicast_socket(
            kPort, internal::kMulticastAddress, responder_.config_.ipv4_address);
        ipv6_socket_ =
            core::socket::open_ipv6_udp_multicast_socket(kPort, internal::kMulticastAddressIpv6);
        if (!ipv4_socket_.valid() && !ipv6_socket_.valid()) {
            return -1;
        }

        bool has_reader = false;
        if (ipv4_socket_.valid()) {
            if (loop_.add_reader(ipv4_socket_.fd(), std::bind(&Impl::handle_ipv4_readable, this,
                                                              std::placeholders::_1))) {
                has_reader = true;
            } else {
                ipv4_socket_.close();
            }
        }
        if (ipv6_socket_.valid()) {
            if (loop_.add_reader(ipv6_socket_.fd(), std::bind(&Impl::handle_ipv6_readable, this,
                                                              std::placeholders::_1))) {
                has_reader = true;
            } else {
                ipv6_socket_.close();
            }
        }
        if (!has_reader) {
            return -1;
        }

        thread_.start();
        return 0;
    }

    void stop() {
        loop_.stop();
        thread_.stopAndJoin();

        std::lock_guard<std::mutex> lock(mutex_);
        if (ipv4_socket_.valid()) {
            loop_.remove(ipv4_socket_.fd());
            ipv4_socket_.close();
        }
        if (ipv6_socket_.valid()) {
            loop_.remove(ipv6_socket_.fd());
            ipv6_socket_.close();
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
        send_ipv4_packets(packets, multicast_endpoint(), false);
        send_ipv6_multicast_packets(packets, 0);
    }

private:
    bool run_once() {
        return loop_.run_once(250);
    }

    void handle_ipv4_readable(int fd) {
        (void)fd;
        std::array<std::uint8_t, internal::kMaxPacketSize> buffer;
        core::socket::Ipv4Endpoint from;
        int received = -1;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            received = ipv4_socket_.receive_from(buffer.data(), buffer.size(), from);
        }
        if (received <= 0) {
            return;
        }

        ResponsePlan plan;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            plan = responder_.handle_query(buffer.data(), static_cast<std::size_t>(received));
        }

        send_ipv4_packets(plan.packets, multicast_endpoint(), false);
        if (plan.wants_unicast || from.port != kPort) {
            send_ipv4_packets(plan.packets, from, true);
        }
    }

    void handle_ipv6_readable(int fd) {
        (void)fd;
        std::array<std::uint8_t, internal::kMaxPacketSize> buffer;
        core::socket::Ipv6Endpoint from;
        int received = -1;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            received = ipv6_socket_.receive_from(buffer.data(), buffer.size(), from);
        }
        if (received <= 0) {
            return;
        }

        ResponsePlan plan;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            plan = responder_.handle_query(buffer.data(), static_cast<std::size_t>(received));
        }

        send_ipv6_packets(plan.packets, from.scope_id, false);
        if (plan.wants_unicast || from.port != kPort) {
            send_ipv6_packets(plan.packets, from, true);
        }
    }

    void send_ipv4_packets(const std::vector<std::vector<std::uint8_t> >& packets,
                           const core::socket::Ipv4Endpoint& endpoint,
                           bool allow_empty_endpoint) {
        if (!allow_empty_endpoint && endpoint.address == 0) {
            return;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        for (std::size_t i = 0; i < packets.size(); ++i) {
            ipv4_socket_.send_to(packets[i].data(), packets[i].size(), endpoint);
        }
    }

    void send_ipv6_packets(const std::vector<std::vector<std::uint8_t> >& packets,
                           std::uint32_t scope_id, bool allow_empty_endpoint) {
        send_ipv6_packets(packets, multicast_endpoint_ipv6(scope_id), allow_empty_endpoint);
    }

    void send_ipv6_multicast_packets(const std::vector<std::vector<std::uint8_t> >& packets,
                                     std::uint32_t fallback_scope_id) {
        std::vector<unsigned int> indices = core::socket::ipv6_multicast_interface_indices();
        if (indices.empty() && fallback_scope_id != 0) {
            indices.push_back(fallback_scope_id);
        }
        if (indices.empty()) {
            send_ipv6_packets(packets, fallback_scope_id, false);
            return;
        }
        for (std::size_t i = 0; i < indices.size(); ++i) {
            send_ipv6_packets(packets, indices[i], false);
        }
    }

    void send_ipv6_packets(const std::vector<std::vector<std::uint8_t> >& packets,
                           const core::socket::Ipv6Endpoint& endpoint,
                           bool allow_empty_endpoint) {
        const std::array<std::uint8_t, 16> empty{};
        if (!allow_empty_endpoint && endpoint.address == empty) {
            return;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        for (std::size_t i = 0; i < packets.size(); ++i) {
            ipv6_socket_.send_to(packets[i].data(), packets[i].size(), endpoint);
        }
    }

    MdnsResponder& responder_;
    std::mutex mutex_;
    core::socket::UdpSocket ipv4_socket_;
    core::socket::UdpSocket ipv6_socket_;
    core::EventLoop loop_;
    MdnsResponderThread thread_;
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
