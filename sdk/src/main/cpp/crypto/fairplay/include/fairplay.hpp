/**
 *  Copyright (C) 2026  kgbook
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 */

#ifndef APLAY_CRYPTO_FAIRPLAY_HPP
#define APLAY_CRYPTO_FAIRPLAY_HPP

#include <cstdint>
#include <vector>

namespace aplay {
namespace crypto {
namespace fairplay {

bool setup_response(const std::vector<std::uint8_t>& request,
                    std::vector<std::uint8_t>& response);
bool handshake_response(const std::vector<std::uint8_t>& request,
                        std::vector<std::uint8_t>& response);

} // namespace fairplay
} // namespace crypto
} // namespace aplay

#endif // APLAY_CRYPTO_FAIRPLAY_HPP
