/**
 *  Copyright (C) 2026  kgbook
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 */

#include "airplay.hpp"

#include "ALog.h"

namespace aplay {
namespace streaming {
namespace airplay {
namespace {

bool add_txt(protocol::mdns::TxtRecord& txt, const std::string& key,
             const std::string& value) {
    if (!txt.add(key, value)) {
        LOGE("airplay", "failed to add AirPlay TXT item: %s", key.c_str());
        return false;
    }
    return true;
}

const char* bool_text(bool value) {
    return value ? "true" : "false";
}

} // namespace

protocol::mdns::Service make_airplay_service(const ServiceProfile& profile) {
    protocol::mdns::Service service;
    service.type = "_airplay._tcp.local";
    service.instance = profile.receiver_name + "._airplay._tcp.local";
    service.port = profile.port;
    service.registered = true;

    add_txt(service.txt, "deviceid", profile.device_id);
    add_txt(service.txt, "features", profile.features);
    if (profile.include_password_required) {
        add_txt(service.txt, "pw", bool_text(profile.password_required));
    }
    add_txt(service.txt, "flags", profile.flags);
    add_txt(service.txt, "model", profile.model);
    add_txt(service.txt, "srcvers", profile.source_version);
    add_txt(service.txt, "vv", profile.protocol_version);
    return service;
}

protocol::mdns::Service make_airplay_service(const std::string& receiver_name,
                                             std::uint16_t port) {
    ServiceProfile profile;
    profile.receiver_name = receiver_name;
    profile.port = port;
    return make_airplay_service(profile);
}

} // namespace airplay
} // namespace streaming
} // namespace aplay
