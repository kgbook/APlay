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
namespace raop {

struct ServiceProfile {
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

protocol::mdns::Service make_raop_service(const ServiceProfile& profile);
protocol::mdns::Service make_raop_service(const std::string& receiver_name,
                                          std::uint16_t port);

} // namespace raop
} // namespace streaming
} // namespace aplay
