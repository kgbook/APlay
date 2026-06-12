/**
 *  Copyright (C) 2026  kgbook
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 */

#include "http.hpp"

#include <algorithm>
#include <cctype>

namespace aplay {
namespace protocol {
namespace http {

static std::string lowercase(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

std::string Request::header_value(const std::string& name) const {
    const std::string wanted = lowercase(name);
    for (std::size_t i = 0; i < headers.size(); ++i) {
        if (lowercase(headers[i].name) == wanted) {
            return headers[i].value;
        }
    }
    return std::string();
}

} // namespace http
} // namespace protocol
} // namespace aplay
