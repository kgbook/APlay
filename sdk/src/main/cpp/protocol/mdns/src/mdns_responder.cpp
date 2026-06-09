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

#include <algorithm>
#include <array>
#include <chrono>
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

typedef std::pair<std::uint32_t, std::vector<std::vector<std::uint8_t> > > Ipv4PacketSet;

} // namespace

class MdnsResponder::Impl {
public:
    explicit Impl(MdnsResponder& responder)
        : responder_(responder),
          ipv4_socket_(),
          ipv6_socket_(),
          ipv4_multicast_interface_addresses_(),
          loop_(),
          thread_(std::bind(&Impl::run_once, this)) {}

    ~Impl() {
        stop();
    }

    int start(int announce_interval_ms) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (thread_.isRunning()) {
            return 0;
        }

        stopped_ = false;
        announce_interval_ms_ = announce_interval_ms;
        last_announce_time_ = std::chrono::steady_clock::time_point::min();

        ResponderConfig config = responder_.config_;
        const bool configured_ipv4 =
            config.ipv4_address != 0 || !config.ipv4_addresses.empty();
        if (configured_ipv4) {
            if (config.ipv4_address == 0 && !config.ipv4_addresses.empty()) {
                config.ipv4_address = config.ipv4_addresses[0];
            }
            ipv4_multicast_interface_addresses_ = config.ipv4_addresses;
            if (config.ipv4_address != 0 &&
                std::find(ipv4_multicast_interface_addresses_.begin(),
                          ipv4_multicast_interface_addresses_.end(),
                          config.ipv4_address) == ipv4_multicast_interface_addresses_.end()) {
                ipv4_multicast_interface_addresses_.insert(
                    ipv4_multicast_interface_addresses_.begin(), config.ipv4_address);
            }
        } else {
            ipv4_multicast_interface_addresses_ =
                core::network::ipv4_multicast_interface_addresses();
            config.ipv4_addresses = ipv4_multicast_interface_addresses_;
            if (!config.ipv4_addresses.empty()) {
                config.ipv4_address = config.ipv4_addresses[0];
            } else {
                core::network::default_ipv4_address(config.ipv4_address);
            }
        }
        const std::array<std::uint8_t, 16> empty_ipv6{};
        if (config.ipv6_address == empty_ipv6) {
            core::network::default_ipv6_address(config.ipv6_address);
        }
        responder_.config_ = config;

        const std::uint32_t ipv4_socket_interface =
            configured_ipv4 && ipv4_multicast_interface_addresses_.size() <= 1 ?
                responder_.config_.ipv4_address :
                0;
        ipv4_socket_ = core::socket::open_ipv4_udp_multicast_socket(
            kPort, internal::kMulticastAddress, ipv4_socket_interface);
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
        if (stopped_) {
            return;
        }
        stopped_ = true;

        loop_.stop();
        thread_.stopAndJoin();

        announce(0);

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
        const std::vector<Ipv4PacketSet> ipv4_packet_sets =
            build_ipv4_multicast_packet_sets(true, true, true, ttl);
        const std::vector<std::vector<std::uint8_t> > ipv6_packets =
            build_ipv6_packet_set(true, true, true, ttl);

        for (std::size_t i = 0; i < ipv4_packet_sets.size(); ++i) {
            send_ipv4_packets(ipv4_packet_sets[i].second, multicast_endpoint(), false,
                              ipv4_packet_sets[i].first);
        }
        send_ipv6_multicast_packets(ipv6_packets, 0);
    }

private:
    bool run_once()
    {
        if (announce_interval_ms_ > 0) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - last_announce_time_).count();
            if (last_announce_time_ == std::chrono::steady_clock::time_point::min() ||
                elapsed >= announce_interval_ms_) {
                announce(kServiceTtl);
                last_announce_time_ = now;
            }
        }
        return loop_.run_once(250);
    }

    void handle_ipv4_readable(const int fd) {
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
            plan = responder_.handle_query(buffer.data(), static_cast<std::size_t>(received),
                                           AddressFamily::Ipv4, 0);
        }

        const std::vector<Ipv4PacketSet> multicast_packet_sets =
            build_ipv4_multicast_packet_sets(plan.include_airplay, plan.include_raop,
                                             plan.include_host, kServiceTtl);
        for (std::size_t i = 0; i < multicast_packet_sets.size(); ++i) {
            send_ipv4_packets(multicast_packet_sets[i].second, multicast_endpoint(), false,
                              multicast_packet_sets[i].first);
        }
        if (plan.wants_unicast || from.port != kPort) {
            send_ipv4_packets(build_ipv4_unicast_packet_set(plan), from, true);
        }
    }

    void handle_ipv6_readable(const int fd) {
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
            plan = responder_.handle_query(buffer.data(), static_cast<std::size_t>(received),
                                           AddressFamily::Ipv6, 0);
        }

        send_ipv6_packets(plan.packets, from.scope_id, false);
        if (plan.wants_unicast || from.port != kPort) {
            send_ipv6_packets(plan.packets, from, true);
        }
    }

    void send_ipv4_multicast_packets(const std::vector<std::vector<std::uint8_t> >& packets) {
        std::vector<std::uint32_t> addresses;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            addresses = ipv4_multicast_interface_addresses_;
        }
        if (addresses.empty()) {
            send_ipv4_packets(packets, multicast_endpoint(), false);
            return;
        }
        for (std::size_t i = 0; i < addresses.size(); ++i) {
            send_ipv4_packets(packets, multicast_endpoint(), false, addresses[i]);
        }
    }

    std::vector<Ipv4PacketSet> build_ipv4_multicast_packet_sets(bool include_airplay,
                                                                bool include_raop,
                                                                bool include_host,
                                                                std::uint32_t ttl) {
        std::vector<Ipv4PacketSet> packet_sets;
        std::lock_guard<std::mutex> lock(mutex_);
        if (ipv4_multicast_interface_addresses_.empty()) {
            const std::vector<std::vector<std::uint8_t> > packets =
                responder_.build_response_packets(include_airplay, include_raop, include_host, ttl,
                                                  AddressFamily::Ipv4, 0);
            if (!packets.empty()) {
                packet_sets.push_back(Ipv4PacketSet(0, packets));
            }
            return packet_sets;
        }

        for (std::size_t i = 0; i < ipv4_multicast_interface_addresses_.size(); ++i) {
            const std::uint32_t address = ipv4_multicast_interface_addresses_[i];
            const std::vector<std::vector<std::uint8_t> > packets =
                responder_.build_response_packets(include_airplay, include_raop, include_host, ttl,
                                                  AddressFamily::Ipv4, address);
            if (!packets.empty()) {
                packet_sets.push_back(Ipv4PacketSet(address, packets));
            }
        }
        return packet_sets;
    }

    std::vector<std::vector<std::uint8_t> > build_ipv4_unicast_packet_set(
        const ResponsePlan& plan) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::uint32_t address = responder_.config_.ipv4_address;
        if (address == 0 && !ipv4_multicast_interface_addresses_.empty()) {
            address = ipv4_multicast_interface_addresses_[0];
        }
        return responder_.build_response_packets(plan.include_airplay, plan.include_raop,
                                                plan.include_host, kServiceTtl,
                                                AddressFamily::Ipv4, address);
    }

    std::vector<std::vector<std::uint8_t> > build_ipv6_packet_set(bool include_airplay,
                                                                  bool include_raop,
                                                                  bool include_host,
                                                                  std::uint32_t ttl) {
        std::lock_guard<std::mutex> lock(mutex_);
        return responder_.build_response_packets(include_airplay, include_raop, include_host, ttl,
                                                AddressFamily::Ipv6, 0);
    }

    void send_ipv4_packets(const std::vector<std::vector<std::uint8_t> >& packets,
                           const core::socket::Ipv4Endpoint& endpoint,
                           bool allow_empty_endpoint,
                           std::uint32_t multicast_interface_address = 0) {
        if (!allow_empty_endpoint && endpoint.address == 0) {
            return;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        if (multicast_interface_address != 0) {
            ipv4_socket_.set_ipv4_multicast_interface(multicast_interface_address);
        }
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
        std::vector<unsigned int> indices = core::network::ipv6_multicast_interface_indices();
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
    std::vector<std::uint32_t> ipv4_multicast_interface_addresses_;
    core::EventLoop loop_;
    MdnsResponderThread thread_;
    bool stopped_ = false;
    int announce_interval_ms_ = 0;
    std::chrono::steady_clock::time_point last_announce_time_;
};

MdnsResponder::MdnsResponder()
    : config_(), impl_(new Impl(*this)) {}

MdnsResponder::~MdnsResponder() {
    delete impl_;
}

void MdnsResponder::set_config(ResponderConfig config) {
    impl_->set_config(std::move(config));
}

int MdnsResponder::start(int announce_interval_ms) {
    return impl_->start(announce_interval_ms);
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
