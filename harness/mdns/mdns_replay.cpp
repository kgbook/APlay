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
#include "network_interface.hpp"

#include "ALog.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

namespace {

using aplay::protocol::mdns::RecordSummary;
using aplay::protocol::mdns::MdnsResponder;
using aplay::protocol::mdns::ResponderConfig;

const char kDefaultReceiverName[] = "APlayHarness";
const char kDefaultDeviceId[] = "02:00:00:00:00:01";
const char kDefaultIpv4Address[] = "127.0.0.1";
const char kDefaultIpv6Address[] = "::1";

bool require(bool condition, const char* message) {
    if (!condition) {
        LOGE("mdns_replay", "%s", message);
        return false;
    }
    return true;
}

ResponderConfig make_config(const std::string& receiver_name, const std::string& device_id) {
    ResponderConfig config;
    config.host_name = receiver_name + ".local";
    if (!aplay::core::network::parse_ipv4_address(kDefaultIpv4Address, config.ipv4_address)) {
        LOGE("mdns_replay", "failed to parse harness IPv4 address");
    }
    if (!aplay::core::network::parse_ipv6_address(kDefaultIpv6Address, config.ipv6_address)) {
        LOGE("mdns_replay", "failed to parse harness IPv6 address");
    }

    aplay::protocol::mdns::AirPlayServiceProfile airplay_profile;
    airplay_profile.receiver_name = receiver_name;
    airplay_profile.device_id = device_id;
    airplay_profile.model = "APlayHarness";
    airplay_profile.include_password_required = true;
    airplay_profile.port = 42609;
    config.airplay = aplay::protocol::mdns::make_airplay_service(airplay_profile);

    aplay::protocol::mdns::RaopServiceProfile raop_profile;
    raop_profile.receiver_name = receiver_name;
    raop_profile.device_id = device_id;
    raop_profile.include_password_required = true;
    raop_profile.port = 42609;
    config.raop = aplay::protocol::mdns::make_raop_service(raop_profile);
    return config;
}

bool has_record(const std::vector<RecordSummary>& records, const std::string& name,
                std::uint16_t type) {
    return std::any_of(records.begin(), records.end(), [&name, type](const RecordSummary& record) {
        return record.name == name && record.type == type;
    });
}

bool has_ptr(const std::vector<RecordSummary>& records, const std::string& name,
             const std::string& ptr) {
    return std::any_of(records.begin(), records.end(), [&name, &ptr](const RecordSummary& record) {
        return record.name == name && record.type == aplay::protocol::mdns::kTypePtr &&
               record.ptr_name == ptr;
    });
}

bool has_txt_item(const std::vector<RecordSummary>& records, const std::string& name,
                  const std::string& item) {
    return std::any_of(records.begin(), records.end(), [&name, &item](const RecordSummary& record) {
        return record.name == name && record.type == aplay::protocol::mdns::kTypeTxt &&
               std::find(record.txt.begin(), record.txt.end(), item) != record.txt.end();
    });
}

std::vector<unsigned char> dns_name_pattern(const std::string& name) {
    std::vector<unsigned char> pattern;
    std::size_t start = 0;
    while (start < name.size()) {
        const std::size_t dot = name.find('.', start);
        const std::size_t end = dot == std::string::npos ? name.size() : dot;
        pattern.push_back(static_cast<unsigned char>(end - start));
        pattern.insert(pattern.end(), name.begin() + static_cast<std::ptrdiff_t>(start),
                       name.begin() + static_cast<std::ptrdiff_t>(end));
        if (dot == std::string::npos) {
            break;
        }
        start = dot + 1;
    }
    pattern.push_back(0);
    return pattern;
}

bool pcap_contains_dns_name(const std::vector<unsigned char>& pcap, const std::string& name) {
    const std::vector<unsigned char> pattern = dns_name_pattern(name);
    return std::search(pcap.begin(), pcap.end(), pattern.begin(), pattern.end()) != pcap.end();
}

bool pcap_contains_bytes(const std::vector<unsigned char>& pcap,
                         const std::vector<unsigned char>& pattern) {
    return std::search(pcap.begin(), pcap.end(), pattern.begin(), pattern.end()) != pcap.end();
}

bool pcap_contains_dns_record_type(const std::vector<unsigned char>& pcap,
                                   const std::string& name, std::uint16_t type) {
    std::vector<unsigned char> pattern = dns_name_pattern(name);
    pattern.push_back(static_cast<unsigned char>((type >> 8) & 0xff));
    pattern.push_back(static_cast<unsigned char>(type & 0xff));
    return pcap_contains_bytes(pcap, pattern);
}

bool validate_generated_response() {
    MdnsResponder& responder = MdnsResponder::instance();
    const ResponderConfig config = make_config(kDefaultReceiverName, kDefaultDeviceId);
    responder.set_config(config);
    const std::vector<std::uint8_t> query =
        aplay::protocol::mdns::build_ptr_query({"_airplay._tcp.local", "_raop._tcp.local"}, true);
    const aplay::protocol::mdns::ResponsePlan plan =
        responder.handle_query(query.data(), query.size());
    if (!require(plan.wants_unicast, "QU query should request a unicast response") ||
        !require(plan.packets.size() == 1, "AirPlay + RAOP query should fit in one response")) {
        return false;
    }

    aplay::protocol::mdns::PacketSummary summary;
    if (!require(aplay::protocol::mdns::MdnsParser::parse_packet(
                     plan.packets[0].data(), plan.packets[0].size(), summary),
                 "generated response must parse") ||
        !require(summary.answers.size() == 10, "combined response must include 10 answers")) {
        return false;
    }

    const std::vector<RecordSummary>& answers = summary.answers;
    if (!require(has_ptr(answers, "_services._dns-sd._udp.local", "_airplay._tcp.local"),
                 "missing AirPlay service enumeration PTR") ||
        !require(has_ptr(answers, "_services._dns-sd._udp.local", "_raop._tcp.local"),
                 "missing RAOP service enumeration PTR") ||
        !require(has_ptr(answers, "_airplay._tcp.local", config.airplay.instance),
                 "missing AirPlay instance PTR") ||
        !require(has_ptr(answers, "_raop._tcp.local", config.raop.instance),
                 "missing RAOP instance PTR") ||
        !require(has_record(answers, config.airplay.instance, aplay::protocol::mdns::kTypeSrv),
                 "missing AirPlay SRV") ||
        !require(has_record(answers, config.raop.instance, aplay::protocol::mdns::kTypeSrv),
                 "missing RAOP SRV") ||
        !require(has_record(answers, config.host_name, aplay::protocol::mdns::kTypeA),
                 "missing host A record") ||
        !require(has_record(answers, config.host_name, aplay::protocol::mdns::kTypeAaaa),
                 "missing host AAAA record") ||
        !require(has_txt_item(answers, config.airplay.instance, "features=0x527FFEE6,0x0"),
                 "missing AirPlay features TXT") ||
        !require(has_txt_item(answers, config.raop.instance, "sr=44100"),
                 "missing RAOP sample-rate TXT")) {
        return false;
    }

    for (const RecordSummary& answer : answers) {
        if (answer.type == aplay::protocol::mdns::kTypePtr &&
            !require((answer.dns_class & aplay::protocol::mdns::kCacheFlush) == 0,
                     "PTR records must not set cache flush")) {
            return false;
        }
        if ((answer.type == aplay::protocol::mdns::kTypeSrv ||
             answer.type == aplay::protocol::mdns::kTypeTxt ||
             answer.type == aplay::protocol::mdns::kTypeA ||
             answer.type == aplay::protocol::mdns::kTypeAaaa) &&
            !require((answer.dns_class & aplay::protocol::mdns::kCacheFlush) != 0,
                     "unique records must set cache flush")) {
            return false;
        }
    }

    const std::vector<std::uint8_t> goodbye = responder.build_goodbye(responder.config().airplay);
    aplay::protocol::mdns::PacketSummary goodbye_summary;
    return require(aplay::protocol::mdns::MdnsParser::parse_packet(
                       goodbye.data(), goodbye.size(), goodbye_summary),
                   "goodbye response must parse") &&
           require(!goodbye_summary.answers.empty(), "goodbye response must have answers") &&
           require(std::all_of(goodbye_summary.answers.begin(), goodbye_summary.answers.end(),
                               [](const RecordSummary& record) { return record.ttl == 0; }),
                   "goodbye answers must use TTL 0");
}

std::string raop_instance_name(const std::string& receiver_name,
                               const std::string& device_id) {
    std::string normalized;
    normalized.reserve(device_id.size());
    for (std::size_t i = 0; i < device_id.size(); ++i) {
        const unsigned char c = static_cast<unsigned char>(device_id[i]);
        if (std::isxdigit(c)) {
            normalized.push_back(static_cast<char>(std::toupper(c)));
        }
    }
    return normalized + "@" + receiver_name + "._raop._tcp.local";
}

bool validate_pcap_capture(const char* path, const std::string& receiver_name,
                           const std::string& device_id, const std::string& host_name) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        LOGE("mdns_replay", "failed to open pcap capture: %s", path);
        return false;
    }

    const std::vector<unsigned char> data{std::istreambuf_iterator<char>(input),
                                          std::istreambuf_iterator<char>()};
    return require(pcap_contains_bytes(data, {0xe0, 0x00, 0x00, 0xfb}),
                   "pcap is missing IPv4 mDNS multicast address 224.0.0.251") &&
           require(pcap_contains_bytes(data,
                                       {0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfb}),
                   "pcap is missing IPv6 mDNS multicast address ff02::fb") &&
           require(pcap_contains_dns_name(data, "_airplay._tcp.local"),
                   "pcap is missing _airplay._tcp.local DNS name") &&
           require(pcap_contains_dns_name(data, "_raop._tcp.local"),
                   "pcap is missing _raop._tcp.local DNS name") &&
           require(pcap_contains_dns_name(data, "_services._dns-sd._udp.local"),
                   "pcap is missing _services._dns-sd._udp.local DNS name") &&
           require(pcap_contains_dns_name(data, receiver_name + "._airplay._tcp.local"),
                   "pcap is missing receiver AirPlay instance") &&
           require(pcap_contains_dns_name(data, raop_instance_name(receiver_name, device_id)),
                   "pcap is missing receiver RAOP instance") &&
           require(pcap_contains_dns_record_type(data, host_name,
                                                aplay::protocol::mdns::kTypeA),
                   "pcap is missing receiver host A record") &&
           require(pcap_contains_dns_record_type(data, host_name,
                                                aplay::protocol::mdns::kTypeAaaa),
                   "pcap is missing receiver host AAAA record");
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2 || argc > 5) {
        LOGE("mdns_replay",
             "usage: aplay_harness_mdns_replay <pcap-capture> [receiver-name] [device-id] [host-name]");
        return 2;
    }

    const std::string receiver_name = argc >= 3 ? argv[2] : kDefaultReceiverName;
    const std::string device_id = argc >= 4 ? argv[3] : kDefaultDeviceId;
    const std::string host_name = argc >= 5 ? argv[4] : receiver_name + ".local";
    if (!validate_generated_response() ||
        !validate_pcap_capture(argv[1], receiver_name, device_id, host_name)) {
        return 1;
    }

    std::cout << "{\"mdns\":\"ok\",\"capture\":\"" << argv[1] << "\"}\n";
    return 0;
}
