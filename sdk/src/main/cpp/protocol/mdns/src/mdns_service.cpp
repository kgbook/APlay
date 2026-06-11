/**
 *  Copyright (C) 2026  kgbook
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 */

#include "mdns_service.hpp"
#include "ALog.h"

#include <cctype>
#include <cstddef>
#include <limits>

namespace aplay {
namespace protocol {
namespace mdns {

static bool add_txt(TxtRecord& txt, const std::string& key, const std::string& value,
             const char* tag, const char* service_name) {
    if (!txt.add(key, value)) {
        LOGE(tag, "failed to add %s TXT item: %s", service_name, key.c_str());
        return false;
    }
    return true;
}

static const char* bool_text(bool value) {
    return value ? "true" : "false";
}

static std::string device_id_for_instance(const std::string& device_id) {
    std::string normalized;
    normalized.reserve(device_id.size());
    for (std::size_t i = 0; i < device_id.size(); ++i) {
        const unsigned char c = static_cast<unsigned char>(device_id[i]);
        if (std::isxdigit(c)) {
            normalized.push_back(static_cast<char>(std::toupper(c)));
        }
    }
    return normalized;
}

bool TxtRecord::add(const std::string& key, const std::string& value) {
    if (key.empty() || key.find('=') != std::string::npos) {
        return false;
    }
    const std::size_t length = key.size() + 1 + value.size();
    if (length > std::numeric_limits<std::uint8_t>::max() || bytes.size() + 1 + length > 512) {
        return false;
    }
    bytes.push_back(static_cast<std::uint8_t>(length));
    bytes.insert(bytes.end(), key.begin(), key.end());
    bytes.push_back('=');
    bytes.insert(bytes.end(), value.begin(), value.end());
    return true;
}

Service make_airplay_service(const AirPlayServiceProfile& profile) {
    Service service;
    service.type = "_airplay._tcp.local";
    service.instance = profile.receiver_name + "._airplay._tcp.local";
    service.port = profile.port;
    service.registered = true;

    add_txt(service.txt, "deviceid", profile.device_id, "airplay", "AirPlay");
    add_txt(service.txt, "features", profile.features, "airplay", "AirPlay");
    if (profile.include_password_required) {
        add_txt(service.txt, "pw", bool_text(profile.password_required), "airplay",
                "AirPlay");
    }
    add_txt(service.txt, "flags", profile.flags, "airplay", "AirPlay");
    add_txt(service.txt, "model", profile.model, "airplay", "AirPlay");
    add_txt(service.txt, "srcvers", profile.source_version, "airplay", "AirPlay");
    add_txt(service.txt, "vv", profile.protocol_version, "airplay", "AirPlay");
    return service;
}

Service make_airplay_service(const std::string& receiver_name, std::uint16_t port) {
    AirPlayServiceProfile profile;
    profile.receiver_name = receiver_name;
    profile.port = port;
    return make_airplay_service(profile);
}

Service make_raop_service(const RaopServiceProfile& profile) {
    Service service;
    service.type = "_raop._tcp.local";
    service.instance =
        device_id_for_instance(profile.device_id) + "@" + profile.receiver_name +
        "._raop._tcp.local";
    service.port = profile.port;
    service.registered = true;

    add_txt(service.txt, "ch", profile.channels, "raop", "RAOP");
    add_txt(service.txt, "cn", profile.codecs, "raop", "RAOP");
    add_txt(service.txt, "et", profile.encryption_types, "raop", "RAOP");
    add_txt(service.txt, "ft", profile.features, "raop", "RAOP");
    add_txt(service.txt, "md", profile.metadata_types, "raop", "RAOP");
    if (profile.include_password_required) {
        add_txt(service.txt, "pw", bool_text(profile.password_required), "raop",
                "RAOP");
    }
    add_txt(service.txt, "sr", profile.sample_rate, "raop", "RAOP");
    add_txt(service.txt, "ss", profile.sample_size, "raop", "RAOP");
    add_txt(service.txt, "tp", profile.transport, "raop", "RAOP");
    add_txt(service.txt, "txtvers", profile.txt_version, "raop", "RAOP");
    add_txt(service.txt, "vs", profile.source_version, "raop", "RAOP");
    add_txt(service.txt, "vv", profile.protocol_version, "raop", "RAOP");
    return service;
}

Service make_raop_service(const std::string& receiver_name, std::uint16_t port) {
    RaopServiceProfile profile;
    profile.receiver_name = receiver_name;
    profile.port = port;
    return make_raop_service(profile);
}

} // namespace mdns
} // namespace protocol
} // namespace aplay
