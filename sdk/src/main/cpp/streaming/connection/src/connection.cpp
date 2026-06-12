/**
 *  Copyright (C) 2026  kgbook
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 */

#include "connection.hpp"

#include "http.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <vector>

namespace aplay {
namespace streaming {
namespace connection {

static bool starts_with(const std::string& value, const std::string& prefix) {
    return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
}

static bool ends_with(const std::string& value, const std::string& suffix) {
    return value.size() >= suffix.size() &&
           value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
}

static std::string to_lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

static std::string first_request_line(const std::string& initial_data) {
    const std::size_t line_end = initial_data.find_first_of("\r\n");
    if (line_end == std::string::npos) {
        return initial_data;
    }
    return initial_data.substr(0, line_end);
}

static std::vector<std::string> split_tokens(const std::string& line) {
    std::vector<std::string> tokens;
    std::size_t start = 0;

    while (start < line.size()) {
        while (start < line.size() &&
               std::isspace(static_cast<unsigned char>(line[start])) != 0) {
            ++start;
        }

        std::size_t end = start;
        while (end < line.size() && std::isspace(static_cast<unsigned char>(line[end])) == 0) {
            ++end;
        }

        if (start < end) {
            tokens.push_back(line.substr(start, end - start));
        }
        start = end;
    }

    return tokens;
}

static bool is_rtsp_method(const std::string& method) {
    return method == "ANNOUNCE" || method == "DESCRIBE" || method == "FLUSH" ||
           method == "GET_PARAMETER" || method == "OPTIONS" || method == "PAUSE" ||
           method == "PLAY" || method == "RECORD" || method == "REDIRECT" ||
           method == "SET_PARAMETER" || method == "SETUP" || method == "TEARDOWN";
}

static bool is_http_method(const std::string& method) {
    return method == "DELETE" || method == "GET" || method == "HEAD" || method == "OPTIONS" ||
           method == "PATCH" || method == "POST" || method == "PUT";
}

static std::string path_without_query_or_fragment(const std::string& target) {
    const std::size_t query = target.find_first_of("?#");
    if (query == std::string::npos) {
        return target;
    }
    return target.substr(0, query);
}

static bool is_hls_target(const std::string& target) {
    const std::string path = to_lower(path_without_query_or_fragment(target));
    return starts_with(path, "/hls/") || path == "/hls" || ends_with(path, ".m3u8") ||
           ends_with(path, ".m3u") || ends_with(path, ".ts") || ends_with(path, ".m4s");
}

ConnectionClassification::ConnectionClassification()
    : category(ConnectionCategory::Unknown), method(), target(), version() {}

ConnectionClassification ConnectionClassifier::classify(const std::string& initial_data) const {
    ConnectionClassification classification;
    const std::vector<std::string> tokens = split_tokens(first_request_line(initial_data));
    if (tokens.size() < 3) {
        return classification;
    }

    classification.method = tokens[0];
    classification.target = tokens[1];
    classification.version = tokens[2];

    if (starts_with(classification.method, "HTTP/") ||
        starts_with(classification.method, "EVENT/")) {
        classification.category = ConnectionCategory::ReverseHttp;
        return classification;
    }

    if (starts_with(classification.version, "RTSP/") || is_rtsp_method(classification.method) ||
        starts_with(to_lower(classification.target), "rtsp://")) {
        classification.category = ConnectionCategory::Rtsp;
        return classification;
    }

    if (starts_with(classification.version, "HTTP/") || is_http_method(classification.method)) {
        classification.category =
            is_hls_target(classification.target) ? ConnectionCategory::Hls : ConnectionCategory::Http;
        return classification;
    }

    return classification;
}

ConnectionHandleStatus ConnectionDispatcher::dispatch(
    const ConnectionClassification& classification) const {
    switch (classification.category) {
        case ConnectionCategory::Http:
            return dispatch_http(classification);
        case ConnectionCategory::Hls:
            return dispatch_hls(classification);
        case ConnectionCategory::Rtsp:
            return dispatch_rtsp(classification);
        case ConnectionCategory::ReverseHttp:
            return ConnectionHandleStatus::Unhandled;
        case ConnectionCategory::Unknown:
            return ConnectionHandleStatus::Unhandled;
    }

    return ConnectionHandleStatus::Unhandled;
}

ConnectionHandleStatus ConnectionDispatcher::dispatch_http(
    const ConnectionClassification& classification) const {
    aplay::protocol::http::Request request;
    request.method = classification.method;
    request.target = classification.target;
    request.version = classification.version;
    const aplay::protocol::http::Response response =
        aplay::protocol::http::make_receiver_response(request);
    return response.status_code == 404 ? ConnectionHandleStatus::Unhandled
                                       : ConnectionHandleStatus::Handled;
}

ConnectionHandleStatus ConnectionDispatcher::dispatch_hls(
    const ConnectionClassification& classification) const {
    (void)classification;
    return ConnectionHandleStatus::NotImplemented;
}

ConnectionHandleStatus ConnectionDispatcher::dispatch_rtsp(
    const ConnectionClassification& classification) const {
    (void)classification;
    return ConnectionHandleStatus::NotImplemented;
}

const char* to_string(ConnectionCategory category) {
    switch (category) {
        case ConnectionCategory::Http:
            return "http";
        case ConnectionCategory::Hls:
            return "hls";
        case ConnectionCategory::Rtsp:
            return "rtsp";
        case ConnectionCategory::ReverseHttp:
            return "reverse_http";
        case ConnectionCategory::Unknown:
            return "unknown";
    }

    return "unknown";
}

const char* to_string(ConnectionHandleStatus status) {
    switch (status) {
        case ConnectionHandleStatus::Handled:
            return "handled";
        case ConnectionHandleStatus::Unhandled:
            return "unhandled";
        case ConnectionHandleStatus::NotImplemented:
            return "not_implemented";
    }

    return "unhandled";
}

} // namespace connection
} // namespace streaming
} // namespace aplay
