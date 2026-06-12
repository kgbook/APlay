/**
 *  Copyright (C) 2026  kgbook
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 */

#pragma once

#include <string>

namespace aplay {
namespace streaming {
namespace connection {

enum class ConnectionCategory {
    Unknown,
    Http,
    Hls,
    Rtsp,
    ReverseHttp,
};

enum class ConnectionHandleStatus {
    Handled,
    Unhandled,
    NotImplemented,
};

struct ConnectionClassification {
    ConnectionClassification();

    ConnectionCategory category;
    std::string method;
    std::string target;
    std::string version;
};

class ConnectionClassifier {
public:
    ConnectionClassification classify(const std::string& initial_data) const;
};

class ConnectionDispatcher {
public:
    ConnectionHandleStatus dispatch(const ConnectionClassification& classification) const;

private:
    ConnectionHandleStatus dispatch_http(const ConnectionClassification& classification) const;
    ConnectionHandleStatus dispatch_hls(const ConnectionClassification& classification) const;
    ConnectionHandleStatus dispatch_rtsp(const ConnectionClassification& classification) const;
};

const char* to_string(ConnectionCategory category);
const char* to_string(ConnectionHandleStatus status);

} // namespace connection
} // namespace streaming
} // namespace aplay
