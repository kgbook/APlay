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

#include "mdns_constants.hpp"
#include "mdns_types.hpp"
#include "singleton.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace aplay {
namespace protocol {
namespace mdns {

class MdnsResponder : public core::Singleton<MdnsResponder> {
public:
    void set_config(ResponderConfig config);
    const ResponderConfig& config() const;

    std::vector<std::vector<std::uint8_t>> build_announcement(
        std::uint32_t ttl, AddressFamily family, std::uint32_t ipv4_address) const;
    std::vector<std::uint8_t> build_goodbye(const Service& service) const;
    ResponsePlan handle_query(const std::uint8_t* bytes, std::size_t length,
                              AddressFamily family, std::uint32_t ipv4_address) const;

    int start();
    void stop();
    void announce(std::uint32_t ttl = kServiceTtl);

private:
    friend class core::Singleton<MdnsResponder>;

    MdnsResponder();
    ~MdnsResponder();

    ResponderConfig config_;

    std::vector<std::vector<std::uint8_t>> build_response_packets(
        bool include_airplay, bool include_raop, bool include_host, std::uint32_t ttl,
        AddressFamily family, std::uint32_t ipv4_address) const;

    class Impl;
    Impl* impl_;
};

} // namespace mdns
} // namespace protocol
} // namespace aplay
