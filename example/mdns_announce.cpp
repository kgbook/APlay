/**
 *  Copyright (C) 2026  kgbook
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 */

#include "mdns.hpp"
#include "network_interface.hpp"
#include "airplay.hpp"
#include "raop.hpp"

#include "ALog.h"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

int main(int argc, char** argv) {
    bool serve = false;
    std::string receiver_name = "APlayExample";
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--serve") {
            serve = true;
        } else {
            receiver_name = arg;
        }
    }

    aplay::protocol::mdns::ResponderConfig config;
    config.host_name = receiver_name + ".local";
    aplay::core::network::parse_ipv4_address("127.0.0.1", config.ipv4_address);

    aplay::streaming::airplay::ServiceProfile airplay_profile;
    airplay_profile.receiver_name = receiver_name;
    airplay_profile.model = "APlayExample";
    config.airplay = aplay::streaming::airplay::make_airplay_service(airplay_profile);

    aplay::streaming::raop::ServiceProfile raop_profile;
    raop_profile.receiver_name = receiver_name;
    config.raop = aplay::streaming::raop::make_raop_service(raop_profile);

    aplay::protocol::mdns::MdnsResponder& responder =
        aplay::protocol::mdns::MdnsResponder::instance();
    responder.set_config(config);
    const std::vector<std::vector<std::uint8_t> > packets =
        responder.build_announcement(aplay::protocol::mdns::kServiceTtl);
    std::cout << "mDNS announcement packets=" << packets.size()
              << " receiver=" << receiver_name << '\n';
    for (std::size_t i = 0; i < packets.size(); ++i) {
        aplay::protocol::mdns::PacketSummary summary;
        if (!aplay::protocol::mdns::MdnsParser::parse_packet(packets[i].data(),
                                                             packets[i].size(), summary)) {
            LOGE("mdns_announce", "failed to parse generated mDNS packet");
            return 1;
        }
        std::cout << "packet[" << i << "] answers=" << summary.answers.size() << '\n';
    }

    if (!serve) {
        return 0;
    }

    if (responder.start() != 0) {
        LOGE("mdns_announce", "failed to start mDNS responder on UDP 5353");
        return 1;
    }

    responder.announce();
    LOGI("mdns_announce", "mDNS responder serving for 5 seconds; browse _airplay._tcp.local or _raop._tcp.local");
    std::this_thread::sleep_for(std::chrono::seconds(5));
    responder.stop();
    return 0;
}
