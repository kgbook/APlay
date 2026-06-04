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
#include "airplay.hpp"
#include "raop.hpp"

#include "ALog.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

namespace {

using aplay::protocol::mdns::RecordSummary;
using aplay::protocol::mdns::MdnsResponder;
using aplay::protocol::mdns::ResponderConfig;

bool require(bool condition, const char* message) {
    if (!condition) {
        LOGE("mdns_replay", "%s", message);
        return false;
    }
    return true;
}

ResponderConfig make_config() {
    ResponderConfig config;
    config.host_name = "iMac.local";
    if (!aplay::core::network::parse_ipv4_address("192.168.1.101", config.ipv4_address)) {
        LOGE("mdns_replay", "failed to parse fixture IPv4 address");
    }

    aplay::streaming::airplay::ServiceProfile airplay_profile;
    airplay_profile.receiver_name = "UxPlay@iMac";
    airplay_profile.device_id = "B2:46:EE:39:57:63";
    airplay_profile.model = "AppleTV3,2";
    airplay_profile.include_password_required = true;
    airplay_profile.port = 42609;
    config.airplay = aplay::streaming::airplay::make_airplay_service(airplay_profile);

    aplay::streaming::raop::ServiceProfile raop_profile;
    raop_profile.receiver_name = "UxPlay@iMac";
    raop_profile.device_id = "B2:46:EE:39:57:63";
    raop_profile.include_password_required = true;
    raop_profile.port = 42609;
    config.raop = aplay::streaming::raop::make_raop_service(raop_profile);
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

bool validate_generated_response() {
    MdnsResponder& responder = MdnsResponder::instance();
    responder.set_config(make_config());
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
        !require(summary.answers.size() == 9, "combined response must include 9 answers")) {
        return false;
    }

    const std::vector<RecordSummary>& answers = summary.answers;
    if (!require(has_ptr(answers, "_services._dns-sd._udp.local", "_airplay._tcp.local"),
                 "missing AirPlay service enumeration PTR") ||
        !require(has_ptr(answers, "_services._dns-sd._udp.local", "_raop._tcp.local"),
                 "missing RAOP service enumeration PTR") ||
        !require(has_ptr(answers, "_airplay._tcp.local", "UxPlay@iMac._airplay._tcp.local"),
                 "missing AirPlay instance PTR") ||
        !require(has_ptr(answers, "_raop._tcp.local",
                         "B246EE395763@UxPlay@iMac._raop._tcp.local"),
                 "missing RAOP instance PTR") ||
        !require(has_record(answers, "UxPlay@iMac._airplay._tcp.local",
                            aplay::protocol::mdns::kTypeSrv),
                 "missing AirPlay SRV") ||
        !require(has_record(answers, "B246EE395763@UxPlay@iMac._raop._tcp.local",
                            aplay::protocol::mdns::kTypeSrv),
                 "missing RAOP SRV") ||
        !require(has_record(answers, "iMac.local", aplay::protocol::mdns::kTypeA),
                 "missing host A record") ||
        !require(has_txt_item(answers, "UxPlay@iMac._airplay._tcp.local",
                              "features=0x527FFEE6,0x0"),
                 "missing AirPlay features TXT") ||
        !require(has_txt_item(answers, "B246EE395763@UxPlay@iMac._raop._tcp.local",
                              "sr=44100"),
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
             answer.type == aplay::protocol::mdns::kTypeA) &&
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

bool validate_pcap_fixture(const char* path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        LOGE("mdns_replay", "failed to open pcap fixture: %s", path);
        return false;
    }

    const std::vector<unsigned char> data{std::istreambuf_iterator<char>(input),
                                          std::istreambuf_iterator<char>()};
    return require(pcap_contains_dns_name(data, "_airplay._tcp.local"),
                   "pcap is missing _airplay._tcp.local DNS name") &&
           require(pcap_contains_dns_name(data, "_raop._tcp.local"),
                   "pcap is missing _raop._tcp.local DNS name") &&
           require(pcap_contains_dns_name(data, "_services._dns-sd._udp.local"),
                   "pcap is missing _services._dns-sd._udp.local DNS name") &&
           require(pcap_contains_dns_name(data, "UxPlay@iMac._airplay._tcp.local"),
                   "pcap is missing UxPlay AirPlay instance") &&
           require(pcap_contains_dns_name(data, "B246EE395763@UxPlay@iMac._raop._tcp.local"),
                   "pcap is missing UxPlay RAOP instance");
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        LOGE("mdns_replay", "usage: aplay_example_mdns_replay <pcapng-fixture>");
        return 2;
    }

    if (!validate_generated_response() || !validate_pcap_fixture(argv[1])) {
        return 1;
    }

    std::cout << "{\"mdns\":\"ok\",\"fixture\":\"" << argv[1] << "\"}\n";
    return 0;
}
