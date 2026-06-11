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

#include "mdns_responder.hpp"
#include "binary_io.hpp"
#include "mdns_packet.hpp"

#include "event_loop.hpp"
#include "network_interface.hpp"
#include "thread.hpp"
#include "udp_socket.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace aplay {
namespace protocol {
namespace mdns {

const std::size_t kCombinedPacketMax = 1200;
const std::uint16_t kFlagResponse = 0x8000;
const std::uint16_t kFlagAuthoritative = 0x0400;
const char kServicesName[] = "_services._dns-sd._udp.local";
const char kMulticastAddress[] = "224.0.0.251";
const char kMulticastAddressIpv6[] = "ff02::fb";
const int kStartupAnnouncementCount = 4;
const int kStartupAnnouncementIntervalMs = 250;

static core::socket::Ipv4Endpoint multicast_endpoint() {
    return core::socket::make_ipv4_endpoint(kMulticastAddress, kPort);
}

static core::socket::Ipv6Endpoint multicast_endpoint_ipv6(std::uint32_t scope_id) {
    return core::socket::make_ipv6_endpoint(kMulticastAddressIpv6, kPort, scope_id);
}

static bool has_ipv6_address(const std::array<std::uint8_t, 16>& address) {
    const std::array<std::uint8_t, 16> empty{};
    return address != empty;
}

static bool name_equals(const std::string& left, const std::string& right) {
    return left.size() == right.size() &&
           std::equal(left.begin(), left.end(), right.begin(), [](char a, char b) {
               return std::tolower(static_cast<unsigned char>(a)) ==
                      std::tolower(static_cast<unsigned char>(b));
           });
}

static bool query_type_matches(std::uint16_t query_type, std::uint16_t record_type) {
    return query_type == record_type || query_type == kTypeAny;
}

typedef std::pair<std::uint32_t, std::vector<std::vector<std::uint8_t> > > Ipv4PacketSet;
typedef std::pair<std::uint32_t, std::vector<std::vector<std::uint8_t> > > Ipv6PacketSet;

static bool begin_response(core::binary::Writer& packet, std::size_t& answer_count_pos) {
    if (!packet.put_u16_be(0) ||
        !packet.put_u16_be(kFlagResponse | kFlagAuthoritative) ||
        !packet.put_u16_be(0)) {
        return false;
    }
    answer_count_pos = packet.size();
    return packet.put_u16_be(0) && packet.put_u16_be(0) && packet.put_u16_be(0);
}

static void finish_response(core::binary::Writer& packet, std::size_t answer_count_pos,
                     std::uint16_t answers) {
    packet[answer_count_pos] = static_cast<std::uint8_t>((answers >> 8) & 0xff);
    packet[answer_count_pos + 1] = static_cast<std::uint8_t>(answers & 0xff);
}

static bool begin_rr(core::binary::Writer& packet, const std::string& name, std::uint16_t type,
              std::uint16_t dns_class, std::uint32_t ttl, std::size_t& rdlength_pos) {
    if (!put_name(packet, name) || !packet.put_u16_be(type) ||
        !packet.put_u16_be(dns_class) || !packet.put_u32_be(ttl)) {
        return false;
    }
    rdlength_pos = packet.size();
    return packet.put_u16_be(0);
}

static void end_rr(core::binary::Writer& packet, std::size_t rdlength_pos) {
    const std::size_t rdlength = packet.size() - rdlength_pos - 2;
    packet[rdlength_pos] = static_cast<std::uint8_t>((rdlength >> 8) & 0xff);
    packet[rdlength_pos + 1] = static_cast<std::uint8_t>(rdlength & 0xff);
}

static bool add_ptr(core::binary::Writer& packet, const std::string& name, const std::string& ptr,
             std::uint32_t ttl) {
    std::size_t rdlength_pos = 0;
    if (!begin_rr(packet, name, kTypePtr, kClassIn, ttl, rdlength_pos) ||
        !put_name(packet, ptr)) {
        return false;
    }
    end_rr(packet, rdlength_pos);
    return true;
}

static bool add_srv(core::binary::Writer& packet, const std::string& name, std::uint16_t port,
             const std::string& target, std::uint32_t ttl) {
    std::size_t rdlength_pos = 0;
    if (!begin_rr(packet, name, kTypeSrv, kClassIn | kCacheFlush, ttl, rdlength_pos) ||
        !packet.put_u16_be(0) || !packet.put_u16_be(0) || !packet.put_u16_be(port) ||
        !put_name(packet, target)) {
        return false;
    }
    end_rr(packet, rdlength_pos);
    return true;
}

static bool add_txt(core::binary::Writer& packet, const std::string& name, const TxtRecord& txt,
             std::uint32_t ttl) {
    std::size_t rdlength_pos = 0;
    if (!begin_rr(packet, name, kTypeTxt, kClassIn | kCacheFlush, ttl, rdlength_pos) ||
        !packet.put_bytes(txt.bytes.data(), txt.bytes.size())) {
        return false;
    }
    end_rr(packet, rdlength_pos);
    return true;
}

static bool add_a(core::binary::Writer& packet, const std::string& name, std::uint32_t address,
           std::uint32_t ttl) {
    if (address == 0) {
        return true;
    }

    std::size_t rdlength_pos = 0;
    const std::array<std::uint8_t, 4> bytes{
        static_cast<std::uint8_t>((address >> 24) & 0xff),
        static_cast<std::uint8_t>((address >> 16) & 0xff),
        static_cast<std::uint8_t>((address >> 8) & 0xff),
        static_cast<std::uint8_t>(address & 0xff),
    };
    if (!begin_rr(packet, name, kTypeA, kClassIn | kCacheFlush, ttl, rdlength_pos) ||
        !packet.put_bytes(bytes.data(), bytes.size())) {
        return false;
    }
    end_rr(packet, rdlength_pos);
    return true;
}

static bool add_aaaa(core::binary::Writer& packet, const std::string& name,
              const std::array<std::uint8_t, 16>& address, std::uint32_t ttl) {
    if (!has_ipv6_address(address)) {
        return true;
    }

    std::size_t rdlength_pos = 0;
    if (!begin_rr(packet, name, kTypeAaaa, kClassIn | kCacheFlush, ttl, rdlength_pos) ||
        !packet.put_bytes(address.data(), address.size())) {
        return false;
    }
    end_rr(packet, rdlength_pos);
    return true;
}

static bool add_a_records(core::binary::Writer& packet, const ResponderConfig& config, std::uint32_t ttl,
                   std::uint16_t& answers, std::uint32_t ipv4_address) {
    std::vector<std::uint32_t> addresses;
    if (ipv4_address != 0) {
        addresses.push_back(ipv4_address);
    } else {
        addresses = config.ipv4_addresses;
        if (config.ipv4_address != 0 &&
            std::find(addresses.begin(), addresses.end(), config.ipv4_address) ==
                addresses.end()) {
            addresses.insert(addresses.begin(), config.ipv4_address);
        }
    }

    for (std::size_t i = 0; i < addresses.size(); ++i) {
        if (!add_a(packet, config.host_name, addresses[i], ttl == 0 ? 0 : kHostTtl)) {
            return false;
        }
        if (addresses[i] != 0) {
            answers = static_cast<std::uint16_t>(answers + 1);
        }
    }
    return true;
}

static bool add_host_records(core::binary::Writer& packet, const ResponderConfig& config, std::uint32_t ttl,
                      std::uint16_t& answers, AddressFamily family,
                      std::uint32_t ipv4_address) {
    if (family == AddressFamily::Ipv4) {
        return add_a_records(packet, config, ttl, answers, ipv4_address);
    }

    if (!add_aaaa(packet, config.host_name, config.ipv6_address, ttl == 0 ? 0 : kHostTtl)) {
        return false;
    }
    if (has_ipv6_address(config.ipv6_address)) {
        answers = static_cast<std::uint16_t>(answers + 1);
    }
    return true;
}

static bool add_service_records(core::binary::Writer& packet, const Service& service,
                         const std::string& host_name, std::uint32_t ttl,
                         std::uint16_t& answers) {
    if (!service.registered) {
        return true;
    }

    if (!add_ptr(packet, std::string(kServicesName), service.type, ttl) ||
        !add_ptr(packet, service.type, service.instance, ttl) ||
        !add_srv(packet, service.instance, service.port, host_name, ttl == 0 ? 0 : kHostTtl) ||
        !add_txt(packet, service.instance, service.txt, ttl)) {
        return false;
    }
    answers = static_cast<std::uint16_t>(answers + 4);
    return true;
}

static bool build_response(const ResponderConfig& config, bool include_airplay, bool include_raop,
                    bool include_host, std::uint32_t ttl, AddressFamily family,
                    std::uint32_t ipv4_address, std::vector<std::uint8_t>& response) {
    core::binary::Writer packet(kMaxPacketSize);
    std::size_t answer_count_pos = 0;
    std::uint16_t answers = 0;
    if (!begin_response(packet, answer_count_pos)) {
        return false;
    }
    if (include_airplay &&
        !add_service_records(packet, config.airplay, config.host_name, ttl, answers)) {
        return false;
    }
    if (include_raop && !add_service_records(packet, config.raop, config.host_name, ttl, answers)) {
        return false;
    }
    if (include_host &&
        !add_host_records(packet, config, ttl, answers, family, ipv4_address)) {
        return false;
    }
    if (answers == 0) {
        return false;
    }
    finish_response(packet, answer_count_pos, answers);
    response = packet.take();
    return true;
}

static void append_response_packets(std::vector<std::vector<std::uint8_t>>& packets,
                             const ResponderConfig& config, bool include_airplay,
                             bool include_raop, bool include_host, std::uint32_t ttl,
                             AddressFamily family, std::uint32_t ipv4_address) {
    if (include_airplay && include_raop && config.airplay.registered && config.raop.registered) {
        std::vector<std::uint8_t> combined;
        if (build_response(config, true, true, include_host, ttl, family, ipv4_address, combined) &&
            combined.size() <= kCombinedPacketMax) {
            packets.push_back(combined);
            return;
        }
    }

    if (include_airplay) {
        std::vector<std::uint8_t> packet;
        if (build_response(config, true, false, include_host, ttl, family, ipv4_address, packet)) {
            packets.push_back(packet);
        }
    }
    if (include_raop) {
        std::vector<std::uint8_t> packet;
        if (build_response(config, false, true, include_host, ttl, family, ipv4_address, packet)) {
            packets.push_back(packet);
        }
    }
    if (include_host && !include_airplay && !include_raop) {
        std::vector<std::uint8_t> packet;
        if (build_response(config, false, false, true, ttl, family, ipv4_address, packet)) {
            packets.push_back(packet);
        }
    }
}

static bool question_matches_service(const ResponderConfig& config, const std::string& name,
                              std::uint16_t type, AddressFamily family, bool& airplay,
                              bool& raop, bool& host) {
    if (name_equals(name, std::string(kServicesName)) &&
        query_type_matches(type, kTypePtr)) {
        airplay = airplay || config.airplay.registered;
        raop = raop || config.raop.registered;
        return airplay || raop;
    }

    if (config.airplay.registered &&
        ((name_equals(name, config.airplay.type) &&
          query_type_matches(type, kTypePtr)) ||
         (name_equals(name, config.airplay.instance) &&
          (query_type_matches(type, kTypeTxt) ||
           query_type_matches(type, kTypeSrv))))) {
        airplay = true;
        host = true;
        return true;
    }

    if (config.raop.registered &&
        ((name_equals(name, config.raop.type) &&
          query_type_matches(type, kTypePtr)) ||
         (name_equals(name, config.raop.instance) &&
          (query_type_matches(type, kTypeTxt) ||
           query_type_matches(type, kTypeSrv))))) {
        raop = true;
        host = true;
        return true;
    }

    if (name_equals(name, config.host_name) &&
        query_type_matches(type, family == AddressFamily::Ipv4 ? kTypeA : kTypeAaaa)) {
        host = true;
        return true;
    }

    return false;
}

class Responder::Impl : public core::Thread {
public:
    explicit Impl(Responder& responder)
        : core::Thread("aplay-mdns"),
          responder_(responder),
          ipv4_socket_(),
          ipv6_socket_(),
          ipv4_multicast_interface_addresses_(),
          ipv6_multicast_interfaces_(),
          loop_() {}

    ~Impl() {
        stop();
    }

    int start(int announce_interval_ms) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (isRunning()) {
            return 0;
        }

        close_sockets_locked();
        stopped_ = false;
        announce_interval_ms_ = announce_interval_ms;
        last_announce_time_ = std::chrono::steady_clock::time_point::min();
        startup_announcements_remaining_ = kStartupAnnouncementCount;

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
        ipv6_multicast_interfaces_ = core::network::ipv6_multicast_interfaces();
        const std::array<std::uint8_t, 16> empty_ipv6{};
        if (config.ipv6_address != empty_ipv6) {
            if (ipv6_multicast_interfaces_.empty()) {
                core::network::Ipv6MulticastInterface iface;
                iface.address = config.ipv6_address;
                ipv6_multicast_interfaces_.push_back(iface);
            }
        } else if (!ipv6_multicast_interfaces_.empty()) {
            config.ipv6_address = ipv6_multicast_interfaces_[0].address;
        } else {
            core::network::default_ipv6_address(config.ipv6_address);
        }
        responder_.config_ = config;

        const std::uint32_t ipv4_socket_interface =
            configured_ipv4 && ipv4_multicast_interface_addresses_.size() <= 1 ?
                responder_.config_.ipv4_address :
                0;
        ipv4_socket_ = core::socket::open_ipv4_udp_multicast_socket(
            kPort, kMulticastAddress, ipv4_socket_interface);
        ipv6_socket_ =
            core::socket::open_ipv6_udp_multicast_socket(kPort, kMulticastAddressIpv6);
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
            close_sockets_locked();
            return -1;
        }

        core::Thread::start();
        return 0;
    }

    void stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stopped_) {
                return;
            }
            stopped_ = true;
        }

        loop_.stop();
        stopAndJoin();

        announce(0);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        announce(0);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        announce(0);

        std::lock_guard<std::mutex> lock(mutex_);
        close_sockets_locked();
    }

    void close_sockets_locked() {
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
        const std::vector<Ipv6PacketSet> ipv6_packet_sets =
            build_ipv6_packet_sets(true, true, true, ttl);

        for (std::size_t i = 0; i < ipv4_packet_sets.size(); ++i) {
            send_ipv4_packets(ipv4_packet_sets[i].second, multicast_endpoint(), false,
                              ipv4_packet_sets[i].first);
        }
        send_ipv6_packet_sets(ipv6_packet_sets);
    }

private:
    bool runOnce() override {
        if (startup_announcements_remaining_ > 0) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - last_announce_time_).count();
            if (last_announce_time_ == std::chrono::steady_clock::time_point::min() ||
                elapsed >= kStartupAnnouncementIntervalMs) {
                announce(kServiceTtl);
                last_announce_time_ = now;
                --startup_announcements_remaining_;
            }
        } else if (announce_interval_ms_ > 0) {
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
        std::array<std::uint8_t, kMaxPacketSize> buffer;
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
        std::uint32_t response_address = 0;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            response_address = ipv4_address_for_peer_locked(from.address);
            plan = responder_.handle_query(buffer.data(), static_cast<std::size_t>(received),
                                           AddressFamily::Ipv4, response_address);
        }

        send_ipv4_packets(plan.packets, multicast_endpoint(), false, response_address);
        if (plan.wants_unicast || from.port != kPort) {
            send_ipv4_packets(plan.packets, from, true);
        }
    }

    void handle_ipv6_readable(const int fd) {
        (void)fd;
        std::array<std::uint8_t, kMaxPacketSize> buffer;
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
        core::network::Ipv6MulticastInterface response_interface;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            response_interface = ipv6_interface_for_peer_locked(from);
            ResponderConfig config = responder_.config_;
            config.ipv6_address = response_interface.address;
            plan = responder_.handle_query(config, buffer.data(),
                                           static_cast<std::size_t>(received),
                                           AddressFamily::Ipv6, 0);
        }

        send_ipv6_packets(plan.packets, response_interface.index, false);
        if (plan.wants_unicast || from.port != kPort) {
            send_ipv6_packets(plan.packets, from, true);
        }
    }

    std::uint32_t ipv4_address_for_peer_locked(std::uint32_t peer_address) const {
        std::uint32_t address = 0;
        if (core::network::local_ipv4_address_for_peer(peer_address, kPort, address) &&
            address != 0) {
            return address;
        }
        if (responder_.config_.ipv4_address != 0) {
            return responder_.config_.ipv4_address;
        }
        if (!ipv4_multicast_interface_addresses_.empty()) {
            return ipv4_multicast_interface_addresses_[0];
        }
        return 0;
    }

    core::network::Ipv6MulticastInterface ipv6_interface_for_peer_locked(
        const core::socket::Ipv6Endpoint& peer) const {
        if (peer.scope_id != 0) {
            for (std::size_t i = 0; i < ipv6_multicast_interfaces_.size(); ++i) {
                if (ipv6_multicast_interfaces_[i].index == peer.scope_id) {
                    return ipv6_multicast_interfaces_[i];
                }
            }
        }

        core::network::Ipv6MulticastInterface iface;
        std::uint32_t scope_id = peer.scope_id;
        if (core::network::local_ipv6_address_for_peer(peer.address, peer.scope_id, kPort,
                                                       iface.address, scope_id) &&
            has_ipv6_address(iface.address)) {
            iface.index = scope_id;
            return iface;
        }

        if (!ipv6_multicast_interfaces_.empty()) {
            return ipv6_multicast_interfaces_[0];
        }

        iface.address = responder_.config_.ipv6_address;
        iface.index = peer.scope_id;
        return iface;
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

    std::vector<Ipv6PacketSet> build_ipv6_packet_sets(bool include_airplay,
                                                      bool include_raop,
                                                      bool include_host,
                                                      std::uint32_t ttl) {
        std::vector<Ipv6PacketSet> packet_sets;
        std::lock_guard<std::mutex> lock(mutex_);
        ResponderConfig config = responder_.config_;
        if (ipv6_multicast_interfaces_.empty()) {
            const std::vector<std::vector<std::uint8_t> > packets =
                responder_.build_response_packets(config, include_airplay, include_raop,
                                                  include_host, ttl, AddressFamily::Ipv6, 0);
            if (!packets.empty()) {
                packet_sets.push_back(Ipv6PacketSet(0, packets));
            }
            return packet_sets;
        }

        for (std::size_t i = 0; i < ipv6_multicast_interfaces_.size(); ++i) {
            config.ipv6_address = ipv6_multicast_interfaces_[i].address;
            const std::vector<std::vector<std::uint8_t> > packets =
                responder_.build_response_packets(config, include_airplay, include_raop,
                                                  include_host, ttl, AddressFamily::Ipv6, 0);
            if (!packets.empty()) {
                packet_sets.push_back(Ipv6PacketSet(ipv6_multicast_interfaces_[i].index, packets));
            }
        }
        return packet_sets;
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

    void send_ipv6_packet_sets(const std::vector<Ipv6PacketSet>& packet_sets) {
        for (std::size_t i = 0; i < packet_sets.size(); ++i) {
            send_ipv6_packets(packet_sets[i].second, packet_sets[i].first, false);
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
        if (endpoint.scope_id != 0) {
            ipv6_socket_.set_ipv6_multicast_interface(endpoint.scope_id);
        }
        for (std::size_t i = 0; i < packets.size(); ++i) {
            ipv6_socket_.send_to(packets[i].data(), packets[i].size(), endpoint);
        }
    }

    Responder& responder_;
    std::mutex mutex_;
    core::socket::UdpSocket ipv4_socket_;
    core::socket::UdpSocket ipv6_socket_;
    std::vector<std::uint32_t> ipv4_multicast_interface_addresses_;
    std::vector<core::network::Ipv6MulticastInterface> ipv6_multicast_interfaces_;
    core::EventLoop loop_;
    bool stopped_ = false;
    int announce_interval_ms_ = 0;
    int startup_announcements_remaining_ = 0;
    std::chrono::steady_clock::time_point last_announce_time_;
};

const ResponderConfig& Responder::config() const {
    return config_;
}

std::vector<std::vector<std::uint8_t> > Responder::build_response_packets(
    bool include_airplay, bool include_raop, bool include_host, std::uint32_t ttl,
    AddressFamily family, std::uint32_t ipv4_address) const {
    return build_response_packets(config_, include_airplay, include_raop, include_host, ttl,
                                  family, ipv4_address);
}

std::vector<std::vector<std::uint8_t> > Responder::build_response_packets(
    const ResponderConfig& config, bool include_airplay, bool include_raop,
    bool include_host, std::uint32_t ttl, AddressFamily family,
    std::uint32_t ipv4_address) const {
    std::vector<std::vector<std::uint8_t> > packets;
    append_response_packets(packets, config, include_airplay, include_raop, include_host, ttl,
                            family, ipv4_address);
    return packets;
}

std::vector<std::vector<std::uint8_t> > Responder::build_announcement(
    std::uint32_t ttl, AddressFamily family, std::uint32_t ipv4_address) const {
    return build_response_packets(true, true, true, ttl, family, ipv4_address);
}

std::vector<std::uint8_t> Responder::build_goodbye(const Service& service) const {
    ResponderConfig goodbye_config = config_;
    goodbye_config.airplay = Service();
    goodbye_config.raop = Service();
    bool include_airplay = false;
    bool include_raop = false;
    if (name_equals(service.type, "_airplay._tcp.local")) {
        goodbye_config.airplay = service;
        include_airplay = true;
    } else if (name_equals(service.type, "_raop._tcp.local")) {
        goodbye_config.raop = service;
        include_raop = true;
    }

    std::vector<std::uint8_t> packet;
    if (build_response(goodbye_config, include_airplay, include_raop, false, 0,
                       AddressFamily::Ipv4, 0, packet)) {
        return packet;
    }
    return std::vector<std::uint8_t>();
}

ResponsePlan Responder::handle_query(const std::uint8_t* bytes, std::size_t length,
                                     AddressFamily family,
                                     std::uint32_t ipv4_address) const {
    return handle_query(config_, bytes, length, family, ipv4_address);
}

ResponsePlan Responder::handle_query(const ResponderConfig& config,
                                     const std::uint8_t* bytes, std::size_t length,
                                     AddressFamily family,
                                     std::uint32_t ipv4_address) const {
    ResponsePlan plan;
    if (length < 12 || (core::binary::read_u16_be(bytes + 2) & kFlagResponse) != 0) {
        return plan;
    }

    const std::uint16_t qdcount = core::binary::read_u16_be(bytes + 4);
    std::size_t offset = 12;
    for (std::uint16_t i = 0; i < qdcount; ++i) {
        QuestionSummary question;
        if (!PacketParser::parse_question(bytes, length, offset, question)) {
            break;
        }

        if ((question.dns_class & kUnicastResponse) != 0) {
            plan.wants_unicast = true;
        }
        question_matches_service(config, question.name, question.type, family,
                                 plan.include_airplay, plan.include_raop, plan.include_host);
    }

    if (plan.include_airplay || plan.include_raop || plan.include_host) {
        append_response_packets(plan.packets, config, plan.include_airplay, plan.include_raop,
                                plan.include_host, kServiceTtl, family, ipv4_address);
    }
    return plan;
}

Responder::Responder()
    : config_(), impl_(new Impl(*this)) {}

Responder::~Responder() {
    delete impl_;
}

void Responder::set_config(ResponderConfig config) {
    impl_->set_config(std::move(config));
}

int Responder::start(int announce_interval_ms) {
    return impl_->start(announce_interval_ms);
}

void Responder::stop() {
    impl_->stop();
}

void Responder::announce(std::uint32_t ttl) {
    impl_->announce(ttl);
}

} // namespace mdns
} // namespace protocol
} // namespace aplay
