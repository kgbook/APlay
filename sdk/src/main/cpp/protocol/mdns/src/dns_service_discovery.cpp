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

#include "ALog.h"
#include "mdns_responder.hpp"
#include "network_interface.hpp"
#include "singleton.hpp"

#include <array>
#include <string>

namespace aplay {
namespace protocol {
namespace mdns {

const char* const kLogTag = "APlayReceiver";
const char* const kReceiverName = "APlayReceiver";

class DNSServiceDiscoveryRuntime : public aplay::core::Singleton<DNSServiceDiscoveryRuntime> {
public:
    int start(const ServicePorts& ports) {
        if (started_) {
            return 0;
        }

        ResponderConfig config;
        config.host_name = std::string(kReceiverName) + ".local";

        config.ipv4_addresses = aplay::core::network::ipv4_multicast_interface_addresses();
        if (!config.ipv4_addresses.empty()) {
            config.ipv4_address = config.ipv4_addresses[0];
            for (std::size_t i = 0; i < config.ipv4_addresses.size(); ++i) {
                LOGI(kLogTag, "detected IPv4 multicast interface: %s",
                     aplay::core::network::format_ipv4_address(config.ipv4_addresses[i]).c_str());
            }
        } else {
            aplay::core::network::parse_ipv4_address("127.0.0.1", config.ipv4_address);
            LOGW(kLogTag, "no IPv4 multicast interface found, falling back to 127.0.0.1");
        }

        LOGI(kLogTag, "mDNS host: %s", config.host_name.c_str());

        AirPlayServiceProfile airplay_profile;
        airplay_profile.receiver_name = kReceiverName;
        airplay_profile.port = ports.airplay;
        config.airplay = make_airplay_service(airplay_profile);

        RaopServiceProfile raop_profile;
        raop_profile.receiver_name = kReceiverName;
        raop_profile.port = ports.raop;
        config.raop = make_raop_service(raop_profile);

        LOGI(kLogTag, "registered AirPlay service: instance=%s type=%s port=%u",
             config.airplay.instance.c_str(), config.airplay.type.c_str(),
             static_cast<unsigned>(config.airplay.port));
        LOGI(kLogTag, "registered RAOP service:    instance=%s type=%s port=%u",
             config.raop.instance.c_str(), config.raop.type.c_str(),
             static_cast<unsigned>(config.raop.port));

        Responder& responder = Responder::instance();
        responder.set_config(config);

        if (responder.start() != 0) {
            LOGE(kLogTag, "failed to start mDNS responder on UDP 5353");
            return 1;
        }

        const ResponderConfig& active_config = responder.config();
        const std::array<std::uint8_t, 16> empty_ipv6{};
        if (active_config.ipv6_address != empty_ipv6) {
            LOGI(kLogTag, "detected IPv6: %s",
                 aplay::core::network::format_ipv6_address(active_config.ipv6_address).c_str());
        } else {
            LOGW(kLogTag, "no IPv6 multicast interface found");
        }

        started_ = true;
        LOGI(kLogTag, "DNS service discovery running");
        return 0;
    }

    void stop() {
        if (!started_) {
            return;
        }

        Responder::instance().stop();
        started_ = false;
        LOGI(kLogTag, "DNS service discovery stopped");
    }

private:
    friend class aplay::core::Singleton<DNSServiceDiscoveryRuntime>;

    DNSServiceDiscoveryRuntime() {}
    ~DNSServiceDiscoveryRuntime() {}

    bool started_ = false;
};

ServicePorts::ServicePorts() : airplay(0), raop(0) {}

int DNSServiceDiscovery::start(const ServicePorts& ports) {
    return DNSServiceDiscoveryRuntime::instance().start(ports);
}

void DNSServiceDiscovery::stop() {
    DNSServiceDiscoveryRuntime::instance().stop();
}

} // namespace mdns
} // namespace protocol
} // namespace aplay
