/**
 *  Copyright (C) 2026  kgbook
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 */

#include "raop.hpp"

#include "ALog.h"

#include <cctype>
#include <cstddef>

namespace aplay {
namespace streaming {
namespace raop {
namespace {

bool add_txt(protocol::mdns::TxtRecord& txt, const std::string& key,
             const std::string& value) {
    if (!txt.add(key, value)) {
        LOGE("raop", "failed to add RAOP TXT item: %s", key.c_str());
        return false;
    }
    return true;
}

const char* bool_text(bool value) {
    return value ? "true" : "false";
}

std::string device_id_for_instance(const std::string& device_id) {
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

} // namespace

protocol::mdns::Service make_raop_service(const ServiceProfile& profile) {
    protocol::mdns::Service service;
    service.type = "_raop._tcp.local";
    service.instance =
        device_id_for_instance(profile.device_id) + "@" + profile.receiver_name +
        "._raop._tcp.local";
    service.port = profile.port;
    service.registered = true;

    add_txt(service.txt, "ch", profile.channels);
    add_txt(service.txt, "cn", profile.codecs);
    add_txt(service.txt, "et", profile.encryption_types);
    add_txt(service.txt, "ft", profile.features);
    add_txt(service.txt, "md", profile.metadata_types);
    if (profile.include_password_required) {
        add_txt(service.txt, "pw", bool_text(profile.password_required));
    }
    add_txt(service.txt, "sr", profile.sample_rate);
    add_txt(service.txt, "ss", profile.sample_size);
    add_txt(service.txt, "tp", profile.transport);
    add_txt(service.txt, "txtvers", profile.txt_version);
    add_txt(service.txt, "vs", profile.source_version);
    add_txt(service.txt, "vv", profile.protocol_version);
    return service;
}

protocol::mdns::Service make_raop_service(const std::string& receiver_name,
                                          std::uint16_t port) {
    ServiceProfile profile;
    profile.receiver_name = receiver_name;
    profile.port = port;
    return make_raop_service(profile);
}

} // namespace raop
} // namespace streaming
} // namespace aplay
