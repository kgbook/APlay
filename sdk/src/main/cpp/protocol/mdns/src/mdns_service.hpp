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

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace aplay {
namespace protocol {
namespace mdns {

struct TxtRecord {
    std::vector<std::uint8_t> bytes;

    bool add(const std::string& key, const std::string& value);
};

struct Service {
    std::string type;
    std::string instance;
    std::uint16_t port = 0;
    TxtRecord txt;
    bool registered = false;
};

struct AirPlayServiceProfile {
    std::string receiver_name = "APlay";
    std::string device_id = "02:00:00:00:00:01";
    std::string features = "0x527FFEE6,0x0";
    std::string flags = "0x4";
    std::string model = "AppleTV6,2";
    std::string source_version = "220.68";
    std::string protocol_version = "2";
    bool password_required = false;
    bool include_password_required = false;
    std::uint16_t port = 7000;
};

struct RaopServiceProfile {
    std::string receiver_name = "APlay";
    std::string device_id = "02:00:00:00:00:01";
    std::string channels = "2";
    std::string codecs = "0,1,2,3";
    std::string encryption_types = "0,3,5";
    std::string features = "0x527FFEE6,0x0";
    std::string metadata_types = "0,1,2";
    std::string sample_rate = "44100";
    std::string sample_size = "16";
    std::string transport = "UDP";
    std::string txt_version = "1";
    std::string source_version = "220.68";
    std::string protocol_version = "2";
    bool password_required = false;
    bool include_password_required = false;
    std::uint16_t port = 7000;
};

Service make_airplay_service(const AirPlayServiceProfile& profile);
Service make_airplay_service(const std::string& receiver_name, std::uint16_t port);

Service make_raop_service(const RaopServiceProfile& profile);
Service make_raop_service(const std::string& receiver_name, std::uint16_t port);

} // namespace mdns
} // namespace protocol
} // namespace aplay
