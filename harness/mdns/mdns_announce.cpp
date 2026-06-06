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

#include "ALog.h"

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace {

volatile std::sig_atomic_t g_should_stop = 0;

void handle_signal(int) {
    g_should_stop = 1;
}

} // namespace

int main(int argc, char** argv) {
    bool once = false;
    std::string receiver_name = "APlayExample";
    int interval_ms = 1000;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--once") {
            once = true;
        } else if (arg == "--interval-ms" && i + 1 < argc) {
            interval_ms = std::atoi(argv[++i]);
            if (interval_ms <= 0) {
                interval_ms = 1000;
            }
        } else {
            receiver_name = arg;
        }
    }

    aplay::protocol::mdns::ResponderConfig config;
    config.host_name = receiver_name + ".local";
    aplay::core::network::parse_ipv4_address("127.0.0.1", config.ipv4_address);
    aplay::core::network::parse_ipv6_address("::1", config.ipv6_address);

    aplay::protocol::mdns::AirPlayServiceProfile airplay_profile;
    airplay_profile.receiver_name = receiver_name;
    airplay_profile.model = "APlayExample";
    config.airplay = aplay::protocol::mdns::make_airplay_service(airplay_profile);

    aplay::protocol::mdns::RaopServiceProfile raop_profile;
    raop_profile.receiver_name = receiver_name;
    config.raop = aplay::protocol::mdns::make_raop_service(raop_profile);

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

    if (once) {
        return 0;
    }

    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);

    if (responder.start() != 0) {
        LOGE("mdns_announce", "failed to start mDNS responder on UDP 5353");
        return 1;
    }

    LOGI("mdns_announce",
         "mDNS responder broadcasting every %d ms; browse _airplay._tcp.local or _raop._tcp.local",
         interval_ms);
    while (!g_should_stop) {
        responder.announce();
        std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
    }
    responder.stop();
    return 0;
}
