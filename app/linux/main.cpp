#include "mdns.hpp"
#include "http.hpp"

#include "ALog.h"

#include <cstdint>
#include <csignal>
#include <unistd.h>

static volatile std::sig_atomic_t g_should_stop = 0;

static void handle_signal(int) {
    g_should_stop = 1;
}

static void wait_for_shutdown_signal() {
    while (!g_should_stop) {
        ::pause();
    }

    LOGI("APlayReceiver", "signal received, shutting down services");
}

int main() {
    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);
    std::signal(SIGPIPE, SIG_IGN);

    LOGI("APlayReceiver", "APlayReceiver 0.1.0 starting");

    aplay::protocol::http::HttpServer httpd;
    if (!httpd.start(0, aplay::protocol::http::make_receiver_response)) {
        return 1;
    }

    const std::uint16_t receiver_port = httpd.port();
    aplay::protocol::mdns::ServicePorts service_ports;
    service_ports.airplay = receiver_port;
    service_ports.raop = receiver_port;

    if (aplay::protocol::mdns::DNSServiceDiscovery::start(service_ports) != 0) {
        httpd.stop();
        return 1;
    }

    wait_for_shutdown_signal();

    httpd.stop();
    aplay::protocol::mdns::DNSServiceDiscovery::stop();
    LOGI("APlayReceiver", "APlayReceiver stopped");
    return 0;
}
