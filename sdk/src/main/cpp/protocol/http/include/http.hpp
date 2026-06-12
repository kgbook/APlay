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

#ifndef APLAY_PROTOCOL_HTTP_HPP
#define APLAY_PROTOCOL_HTTP_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace aplay {
namespace protocol {
namespace http {

struct Header {
    std::string name;
    std::string value;
};

struct Request {
    std::string method;
    std::string target;
    std::string version;
    std::vector<Header> headers;
    std::string body;

    std::string header_value(const std::string& name) const;
};

struct Response {
    std::string version;
    int status_code;
    std::string reason;
    std::vector<Header> headers;
    std::string body;

    Response();

    void set_header(const std::string& name, const std::string& value);
    std::string serialize() const;
};

enum class ParseStatus {
    Complete,
    Incomplete,
    Error,
};

struct ParserLimits {
    std::size_t max_start_line_length;
    std::size_t max_header_count;
    std::size_t max_header_name_length;
    std::size_t max_header_value_length;
    std::size_t max_body_length;

    ParserLimits();
};

class RequestParser {
public:
    explicit RequestParser(const ParserLimits& limits = ParserLimits());

    ParseStatus parse(const std::string& bytes, Request& request,
                      std::string& error) const;

private:
    ParserLimits limits_;
};

typedef std::function<Response(const Request& request)> RequestHandler;

class HttpServer {
public:
    HttpServer();
    ~HttpServer();

    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;

    bool start(std::uint16_t port, const RequestHandler& handler);
    void stop();
    bool running() const;
    std::uint16_t port() const;

private:
    class Impl;
    Impl* impl_;
};

Response make_text_response(int status_code, const std::string& reason,
                            const std::string& body);
Response make_receiver_response(const Request& request);

} // namespace http
} // namespace protocol
} // namespace aplay

#endif // APLAY_PROTOCOL_HTTP_HPP
