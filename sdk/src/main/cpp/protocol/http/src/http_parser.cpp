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
#include <cstdlib>
#include <sstream>

namespace aplay {
namespace protocol {
namespace http {

static std::string trim_ows(const std::string& value) {
    std::size_t begin = 0;
    while (begin < value.size() && (value[begin] == ' ' || value[begin] == '\t')) {
        ++begin;
    }

    std::size_t end = value.size();
    while (end > begin && (value[end - 1] == ' ' || value[end - 1] == '\t')) {
        --end;
    }

    return value.substr(begin, end - begin);
}

static bool parse_content_length(const std::string& value, std::size_t& length) {
    if (value.empty()) {
        return false;
    }
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] < '0' || value[i] > '9') {
            return false;
        }
    }
    length = static_cast<std::size_t>(std::strtoul(value.c_str(), NULL, 10));
    return true;
}

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

ParserLimits::ParserLimits()
    : max_start_line_length(4096),
      max_header_count(64),
      max_header_name_length(128),
      max_header_value_length(8192),
      max_body_length(1024 * 1024) {}

RequestParser::RequestParser(const ParserLimits& limits) : limits_(limits) {}

ParseStatus RequestParser::parse(const std::string& bytes, Request& request,
                                 std::string& error) const {
    request = Request();
    error.clear();

    const std::size_t header_end = bytes.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        if (bytes.size() > limits_.max_start_line_length + limits_.max_header_value_length) {
            error = "headers too long";
            return ParseStatus::Error;
        }
        return ParseStatus::Incomplete;
    }

    std::istringstream lines(bytes.substr(0, header_end));
    std::string line;
    if (!std::getline(lines, line)) {
        error = "missing request line";
        return ParseStatus::Error;
    }
    if (!line.empty() && line[line.size() - 1] == '\r') {
        line.erase(line.size() - 1);
    }
    if (line.size() > limits_.max_start_line_length) {
        error = "request line too long";
        return ParseStatus::Error;
    }

    std::istringstream start_line(line);
    if (!(start_line >> request.method >> request.target >> request.version)) {
        error = "invalid request line";
        return ParseStatus::Error;
    }

    std::size_t content_length = 0;
    while (std::getline(lines, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r') {
            line.erase(line.size() - 1);
        }
        const std::size_t colon = line.find(':');
        if (colon == std::string::npos) {
            error = "invalid header";
            return ParseStatus::Error;
        }
        if (request.headers.size() >= limits_.max_header_count) {
            error = "too many headers";
            return ParseStatus::Error;
        }

        Header header;
        header.name = trim_ows(line.substr(0, colon));
        header.value = trim_ows(line.substr(colon + 1));
        if (header.name.empty() || header.name.size() > limits_.max_header_name_length) {
            error = "invalid header name";
            return ParseStatus::Error;
        }
        if (header.value.size() > limits_.max_header_value_length) {
            error = "header value too long";
            return ParseStatus::Error;
        }
        if (request.header_value("Content-Length").empty() &&
            header_name_equals(header.name, "Content-Length")) {
            if (!parse_content_length(header.value, content_length)) {
                error = "invalid content length";
                return ParseStatus::Error;
            }
            if (content_length > limits_.max_body_length) {
                error = "body too large";
                return ParseStatus::Error;
            }
        }
        request.headers.push_back(header);
    }

    const std::size_t body_begin = header_end + 4;
    if (bytes.size() < body_begin + content_length) {
        return ParseStatus::Incomplete;
    }
    request.body = bytes.substr(body_begin, content_length);
    return ParseStatus::Complete;
}

} // namespace http
} // namespace protocol
} // namespace aplay
