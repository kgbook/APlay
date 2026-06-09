#include "aplay/linux/runtime.hpp"

#include "ALog.h"
#include "mdns.hpp"
#include "network_interface.hpp"

#include <array>
#include <csignal>
#include <string>
#include <unistd.h>

namespace {

volatile std::sig_atomic_t g_should_stop = 0;

void handle_signal(int) {
    g_should_stop = 1;
}

} // namespace

namespace aplay {
namespace linux_app {

int run_runtime() {
    const std::string receiver_name = "APlayReceiver";

    LOGI("APlayReceiver", "APlayReceiver 0.1.0 starting");

    aplay::protocol::mdns::ResponderConfig config;
    config.host_name = receiver_name + ".local";

    std::uint32_t ipv4 = 0;
    if (aplay::core::network::default_ipv4_address(ipv4)) {
        config.ipv4_address = ipv4;
        LOGI("APlayReceiver", "detected IPv4: %s",
             aplay::core::network::format_ipv4_address(ipv4).c_str());
    } else {
        aplay::core::network::parse_ipv4_address("127.0.0.1", config.ipv4_address);
        LOGW("APlayReceiver", "no IPv4 multicast interface found, falling back to 127.0.0.1");
    }

    LOGI("APlayReceiver", "mDNS host: %s", config.host_name.c_str());

    aplay::protocol::mdns::AirPlayServiceProfile airplay_profile;
    airplay_profile.receiver_name = receiver_name;
    config.airplay = aplay::protocol::mdns::make_airplay_service(airplay_profile);

    aplay::protocol::mdns::RaopServiceProfile raop_profile;
    raop_profile.receiver_name = receiver_name;
    config.raop = aplay::protocol::mdns::make_raop_service(raop_profile);

    LOGI("APlayReceiver", "registered AirPlay service: instance=%s type=%s port=%u",
         config.airplay.instance.c_str(), config.airplay.type.c_str(),
         static_cast<unsigned>(config.airplay.port));
    LOGI("APlayReceiver", "registered RAOP service:    instance=%s type=%s port=%u",
         config.raop.instance.c_str(), config.raop.type.c_str(),
         static_cast<unsigned>(config.raop.port));

    aplay::protocol::mdns::MdnsResponder& responder =
        aplay::protocol::mdns::MdnsResponder::instance();
    responder.set_config(config);

    if (responder.start() != 0) {
        LOGE("APlayReceiver", "failed to start mDNS responder on UDP 5353");
        return 1;
    }

    const auto& active_config = responder.config();
    const std::array<std::uint8_t, 16> empty_ipv6{};
    if (active_config.ipv6_address != empty_ipv6) {
        LOGI("APlayReceiver", "detected IPv6: %s",
             aplay::core::network::format_ipv6_address(active_config.ipv6_address).c_str());
    } else {
        LOGW("APlayReceiver", "no IPv6 multicast interface found");
    }

    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);

    LOGI("APlayReceiver", "mDNS responder running");
    while (!g_should_stop) {
        ::pause();
    }

    LOGI("APlayReceiver", "signal received, shutting down mDNS responder");
    responder.stop();
    LOGI("APlayReceiver", "APlayReceiver stopped");
    return 0;
}

} // namespace linux_app
} // namespace aplay
