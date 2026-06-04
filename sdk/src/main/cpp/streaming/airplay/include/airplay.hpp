/**
 *  Copyright (C) 2026  kgbook
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 */

#pragma once

#include "mdns.hpp"

#include <cstdint>
#include <string>

namespace aplay {
namespace streaming {
namespace airplay {

struct ServiceProfile {
    std::string receiver_name = "APlay";
    std::string device_id = "02:00:00:00:00:01";
    std::string features = "0x527FFEE6,0x0";
    std::string flags = "0x4";
    std::string model = "APlayReceiver";
    std::string source_version = "220.68";
    std::string protocol_version = "2";
    bool password_required = false;
    bool include_password_required = false;
    std::uint16_t port = 7000;
};

protocol::mdns::Service make_airplay_service(const ServiceProfile& profile);
protocol::mdns::Service make_airplay_service(const std::string& receiver_name,
                                             std::uint16_t port);

} // namespace airplay
} // namespace streaming
} // namespace aplay
