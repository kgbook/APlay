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

#include "mdns_harness.hpp"

#include "ALog.h"

#include <iostream>
#include <string>

const char kDefaultReceiverName[] = "APlayHarness";
const char kDefaultDeviceId[] = "02:00:00:00:00:01";

int main(int argc, char** argv) {
    if (argc < 2 || argc > 5) {
        LOGE("mdns_replay",
             "usage: aplay_harness_mdns_replay <pcap-capture> [receiver-name] [device-id] [host-name]");
        return 2;
    }

    const std::string receiver_name = argc >= 3 ? argv[2] : kDefaultReceiverName;
    const std::string device_id = argc >= 4 ? argv[3] : kDefaultDeviceId;
    const std::string host_name = argc >= 5 ? argv[4] : receiver_name + ".local";
    if (!aplay::protocol::mdns::validate_harness_replay(argv[1], receiver_name, device_id,
                                                        host_name)) {
        return 1;
    }

    std::cout << "{\"mdns\":\"ok\",\"capture\":\"" << argv[1] << "\"}\n";
    return 0;
}
