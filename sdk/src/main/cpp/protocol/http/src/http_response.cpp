/**
 *  Copyright (C) 2026  kgbook
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 */

#include "http.hpp"

#include <cctype>
#include <sstream>

namespace aplay {
namespace protocol {
namespace http {

static bool header_name_equals(const std::string& left, const std::string& right) {
    if (left.size() != right.size()) {
        return false;
    }
    for (std::size_t i = 0; i < left.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(left[i])) !=
            std::tolower(static_cast<unsigned char>(right[i]))) {
            return false;
        }
    }
    return true;
}

Response::Response() : version("HTTP/1.1"), status_code(200), reason("OK") {}

void Response::set_header(const std::string& name, const std::string& value) {
    for (std::size_t i = 0; i < headers.size(); ++i) {
        if (header_name_equals(headers[i].name, name)) {
            headers[i].value = value;
            return;
        }
    }
    Header header;
    header.name = name;
    header.value = value;
    headers.push_back(header);
}

std::string Response::serialize() const {
    std::ostringstream out;
    out << version << " " << status_code << " " << reason << "\r\n";
    for (std::size_t i = 0; i < headers.size(); ++i) {
        out << headers[i].name << ": " << headers[i].value << "\r\n";
    }
    out << "\r\n";
    out << body;
    return out.str();
}

Response make_text_response(int status_code, const std::string& reason,
                            const std::string& body) {
    Response response;
    response.status_code = status_code;
    response.reason = reason;
    response.body = body;
    response.set_header("Content-Type", "text/plain; charset=utf-8");
    response.set_header("Content-Length", std::to_string(response.body.size()));
    response.set_header("Connection", "close");
    return response;
}

} // namespace http
} // namespace protocol
} // namespace aplay
