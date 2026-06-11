#include "mdns.hpp"

#include "ALog.h"

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

    LOGI("APlayReceiver", "APlayReceiver 0.1.0 starting");

    if (aplay::protocol::mdns::DNSServiceDiscovery::start() != 0) {
        return 1;
    }

    wait_for_shutdown_signal();

    aplay::protocol::mdns::DNSServiceDiscovery::stop();
    LOGI("APlayReceiver", "APlayReceiver stopped");
    return 0;
}
