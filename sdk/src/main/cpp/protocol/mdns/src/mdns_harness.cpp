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

#if defined(APLAY_MDNS_HARNESS)

#include "mdns_harness.hpp"

#include "mdns_packet.hpp"
#include "mdns_responder.hpp"
#include "network_interface.hpp"

#include "ALog.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace aplay {
namespace protocol {
namespace mdns {

const char kHarnessReceiverName[] = "APlayHarness";
const char kHarnessDeviceId[] = "02:00:00:00:00:01";
const char kHarnessIpv4Address[] = "127.0.0.1";
const char kHarnessIpv6Address[] = "::1";

bool require_harness(bool condition, const char* message) {
    if (!condition) {
        LOGE("mdns_harness", "%s", message);
        return false;
    }
    return true;
}

ResponderConfig make_harness_config(const std::string& receiver_name,
                                    const std::string& device_id) {
    ResponderConfig config;
    config.host_name = receiver_name + ".local";
    if (!aplay::core::network::parse_ipv4_address(kHarnessIpv4Address, config.ipv4_address)) {
        LOGE("mdns_harness", "failed to parse harness IPv4 address");
    }
    if (!aplay::core::network::parse_ipv6_address(kHarnessIpv6Address, config.ipv6_address)) {
        LOGE("mdns_harness", "failed to parse harness IPv6 address");
    }

    AirPlayServiceProfile airplay_profile;
    airplay_profile.receiver_name = receiver_name;
    airplay_profile.device_id = device_id;
    airplay_profile.model = "APlayHarness";
    airplay_profile.include_password_required = true;
    airplay_profile.port = 42609;
    config.airplay = make_airplay_service(airplay_profile);

    RaopServiceProfile raop_profile;
    raop_profile.receiver_name = receiver_name;
    raop_profile.device_id = device_id;
    raop_profile.include_password_required = true;
    raop_profile.port = 42609;
    config.raop = make_raop_service(raop_profile);
    return config;
}

bool harness_has_record(const std::vector<RecordSummary>& records, const std::string& name,
                        std::uint16_t type) {
    return std::any_of(records.begin(), records.end(), [&name, type](const RecordSummary& record) {
        return record.name == name && record.type == type;
    });
}

bool harness_has_ptr(const std::vector<RecordSummary>& records, const std::string& name,
                     const std::string& ptr) {
    return std::any_of(records.begin(), records.end(), [&name, &ptr](const RecordSummary& record) {
        return record.name == name && record.type == kTypePtr && record.ptr_name == ptr;
    });
}

bool harness_has_txt_item(const std::vector<RecordSummary>& records, const std::string& name,
                          const std::string& item) {
    return std::any_of(records.begin(), records.end(), [&name, &item](const RecordSummary& record) {
        return record.name == name && record.type == kTypeTxt &&
               std::find(record.txt.begin(), record.txt.end(), item) != record.txt.end();
    });
}

std::vector<unsigned char> harness_dns_name_pattern(const std::string& name) {
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

bool harness_pcap_contains_dns_name(const std::vector<unsigned char>& pcap,
                                    const std::string& name) {
    const std::vector<unsigned char> pattern = harness_dns_name_pattern(name);
    return std::search(pcap.begin(), pcap.end(), pattern.begin(), pattern.end()) != pcap.end();
}

bool harness_pcap_contains_bytes(const std::vector<unsigned char>& pcap,
                                 const std::vector<unsigned char>& pattern) {
    return std::search(pcap.begin(), pcap.end(), pattern.begin(), pattern.end()) != pcap.end();
}

bool harness_pcap_contains_dns_record_type(const std::vector<unsigned char>& pcap,
                                           const std::string& name, std::uint16_t type) {
    std::vector<unsigned char> pattern = harness_dns_name_pattern(name);
    pattern.push_back(static_cast<unsigned char>((type >> 8) & 0xff));
    pattern.push_back(static_cast<unsigned char>(type & 0xff));
    return harness_pcap_contains_bytes(pcap, pattern);
}

bool validate_harness_generated_response_for_family(Responder& responder,
                                                    const ResponderConfig& config,
                                                    const std::vector<std::uint8_t>& query,
                                                    AddressFamily family,
                                                    std::uint16_t expected_host_type,
                                                    std::uint16_t rejected_host_type,
                                                    const char* family_name) {
    const ResponsePlan plan = responder.handle_query(
        query.data(), query.size(), family,
        family == AddressFamily::Ipv4 ? config.ipv4_address : 0);
    if (!require_harness(plan.wants_unicast, "QU query should request a unicast response") ||
        !require_harness(plan.packets.size() == 1,
                         "AirPlay + RAOP query should fit in one response")) {
        return false;
    }

    PacketSummary summary;
    if (!require_harness(PacketParser::parse_packet(plan.packets[0].data(),
                                                    plan.packets[0].size(), summary),
                         "generated response must parse") ||
        !require_harness(summary.answers.size() == 9,
                         "combined family-specific response must include 9 answers")) {
        return false;
    }

    const std::vector<RecordSummary>& answers = summary.answers;
    if (!require_harness(harness_has_ptr(answers, "_services._dns-sd._udp.local",
                                         "_airplay._tcp.local"),
                         "missing AirPlay service enumeration PTR") ||
        !require_harness(harness_has_ptr(answers, "_services._dns-sd._udp.local",
                                         "_raop._tcp.local"),
                         "missing RAOP service enumeration PTR") ||
        !require_harness(harness_has_ptr(answers, "_airplay._tcp.local",
                                         config.airplay.instance),
                         "missing AirPlay instance PTR") ||
        !require_harness(harness_has_ptr(answers, "_raop._tcp.local", config.raop.instance),
                         "missing RAOP instance PTR") ||
        !require_harness(harness_has_record(answers, config.airplay.instance, kTypeSrv),
                         "missing AirPlay SRV") ||
        !require_harness(harness_has_record(answers, config.raop.instance, kTypeSrv),
                         "missing RAOP SRV") ||
        !require_harness(harness_has_record(answers, config.host_name, expected_host_type),
                         "missing expected family host address record") ||
        !require_harness(!harness_has_record(answers, config.host_name, rejected_host_type),
                         "response contains cross-family host address record") ||
        !require_harness(harness_has_txt_item(answers, config.airplay.instance,
                                              "features=0x5A7FFEE6,0x0"),
                         "missing AirPlay features TXT") ||
        !require_harness(harness_has_txt_item(answers, config.raop.instance, "sr=44100"),
                         "missing RAOP sample-rate TXT")) {
        return false;
    }

    for (const RecordSummary& answer : answers) {
        if (answer.type == kTypePtr &&
            !require_harness((answer.dns_class & kCacheFlush) == 0,
                             "PTR records must not set cache flush")) {
            return false;
        }
        if ((answer.type == kTypeSrv || answer.type == kTypeTxt || answer.type == kTypeA ||
             answer.type == kTypeAaaa) &&
            !require_harness((answer.dns_class & kCacheFlush) != 0,
                             "unique records must set cache flush")) {
            return false;
        }
    }

    LOGI("mdns_harness", "generated %s response records=%zu", family_name,
         summary.answers.size());
    return true;
}

bool validate_harness_generated_response() {
    Responder& responder = Responder::instance();
    const ResponderConfig config = make_harness_config(kHarnessReceiverName, kHarnessDeviceId);
    responder.set_config(config);
    const std::vector<std::uint8_t> query =
        build_ptr_query({"_airplay._tcp.local", "_raop._tcp.local"}, true);
    if (!validate_harness_generated_response_for_family(
            responder, config, query, AddressFamily::Ipv4, kTypeA, kTypeAaaa, "IPv4") ||
        !validate_harness_generated_response_for_family(
            responder, config, query, AddressFamily::Ipv6, kTypeAaaa, kTypeA, "IPv6")) {
        return false;
    }

    const std::vector<std::uint8_t> goodbye = responder.build_goodbye(responder.config().airplay);
    PacketSummary goodbye_summary;
    return require_harness(PacketParser::parse_packet(goodbye.data(), goodbye.size(),
                                                      goodbye_summary),
                           "goodbye response must parse") &&
           require_harness(!goodbye_summary.answers.empty(),
                           "goodbye response must have answers") &&
           require_harness(std::all_of(goodbye_summary.answers.begin(),
                                       goodbye_summary.answers.end(),
                                       [](const RecordSummary& record) {
                                           return record.ttl == 0;
                                       }),
                           "goodbye answers must use TTL 0");
}

std::string harness_raop_instance_name(const std::string& receiver_name,
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

bool validate_harness_pcap_capture(const char* path, const std::string& receiver_name,
                                   const std::string& device_id,
                                   const std::string& host_name) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        LOGE("mdns_harness", "failed to open pcap capture: %s", path);
        return false;
    }

    const std::vector<unsigned char> data{std::istreambuf_iterator<char>(input),
                                          std::istreambuf_iterator<char>()};
    return require_harness(harness_pcap_contains_bytes(data, {0xe0, 0x00, 0x00, 0xfb}),
                           "pcap is missing IPv4 mDNS multicast address 224.0.0.251") &&
           require_harness(harness_pcap_contains_bytes(
                               data, {0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfb}),
                           "pcap is missing IPv6 mDNS multicast address ff02::fb") &&
           require_harness(harness_pcap_contains_dns_name(data, "_airplay._tcp.local"),
                           "pcap is missing _airplay._tcp.local DNS name") &&
           require_harness(harness_pcap_contains_dns_name(data, "_raop._tcp.local"),
                           "pcap is missing _raop._tcp.local DNS name") &&
           require_harness(harness_pcap_contains_dns_name(data,
                                                          "_services._dns-sd._udp.local"),
                           "pcap is missing _services._dns-sd._udp.local DNS name") &&
           require_harness(harness_pcap_contains_dns_name(
                               data, receiver_name + "._airplay._tcp.local"),
                           "pcap is missing receiver AirPlay instance") &&
           require_harness(harness_pcap_contains_dns_name(
                               data, harness_raop_instance_name(receiver_name, device_id)),
                           "pcap is missing receiver RAOP instance") &&
           require_harness(harness_pcap_contains_dns_record_type(data, host_name, kTypeA),
                           "pcap is missing receiver host A record") &&
           require_harness(harness_pcap_contains_dns_record_type(data, host_name, kTypeAaaa),
                           "pcap is missing receiver host AAAA record");
}

bool validate_harness_replay(const char* path, const std::string& receiver_name,
                             const std::string& device_id,
                             const std::string& host_name) {
    return validate_harness_generated_response() &&
           validate_harness_pcap_capture(path, receiver_name, device_id, host_name);
}

} // namespace mdns
} // namespace protocol
} // namespace aplay

#endif // APLAY_MDNS_HARNESS
