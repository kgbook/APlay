/**
 *  Copyright (C) 2026  kgbook
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 */

#include "http.hpp"

#include "fairplay.hpp"

#include <cctype>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

namespace aplay {
namespace protocol {
namespace http {

static const char* const kReceiverName = "APlayReceiver";
static const char* const kDeviceId = "02:00:00:00:00:01";
static const char* const kDeviceIdCompact = "020000000001";
static const char* const kFeatureText = "0x5A7FFEE6,0x0";
static const std::uint64_t kFeatures = 0x5A7FFEE6ULL;
static const char* const kModel = "AppleTV3,2";
static const char* const kPairId = "2e388006-13ba-4041-9a67-25dd4a43d536";
static const char* const kPublicKey =
    "00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF";
static const char* const kSourceVersion = "220.68";
static const char* const kProtocolVersion = "2";
static const std::uint8_t kPairVerifySignature[64] = {
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f};

static Response make_plist_response(const std::string& body) {
    Response response;
    response.body = body;
    response.set_header("Content-Type", "text/x-apple-plist+xml");
    response.set_header("Content-Length", std::to_string(response.body.size()));
    response.set_header("Connection", "close");
    return response;
}

static Response make_empty_response(const Request& request, int status_code,
                                    const std::string& reason) {
    Response response;
    response.version = request.version.empty() ? "HTTP/1.1" : request.version;
    response.status_code = status_code;
    response.reason = reason;
    response.set_header("Content-Length", "0");
    response.set_header("Connection", "close");
    const std::string cseq = request.header_value("CSeq");
    if (!cseq.empty()) {
        response.set_header("CSeq", cseq);
    }
    return response;
}

static Response make_binary_response(const Request& request,
                                     const std::vector<std::uint8_t>& body) {
    Response response;
    response.version = request.version.empty() ? "HTTP/1.1" : request.version;
    if (!body.empty()) {
        response.body.assign(reinterpret_cast<const char*>(body.data()), body.size());
    }
    response.set_header("Content-Type", "application/octet-stream");
    response.set_header("Content-Length", std::to_string(response.body.size()));
    response.set_header("Connection", "close");
    const std::string cseq = request.header_value("CSeq");
    if (!cseq.empty()) {
        response.set_header("CSeq", cseq);
    }
    return response;
}

static bool starts_with(const std::string& value, const std::string& prefix) {
    return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
}

static std::string path_from_target(const std::string& target) {
    std::string path = target;
    const std::size_t scheme = path.find("://");
    if (scheme != std::string::npos) {
        const std::size_t host_end = path.find('/', scheme + 3);
        path = host_end == std::string::npos ? "/" : path.substr(host_end);
    }

    const std::size_t query = path.find_first_of("?#");
    if (query != std::string::npos) {
        path = path.substr(0, query);
    }
    return path.empty() ? "/" : path;
}

static void add_txt(std::vector<std::uint8_t>& bytes, const std::string& key,
                    const std::string& value) {
    const std::string item = key + "=" + value;
    if (item.size() > 255) {
        return;
    }
    bytes.push_back(static_cast<std::uint8_t>(item.size()));
    bytes.insert(bytes.end(), item.begin(), item.end());
}

static std::vector<std::uint8_t> airplay_txt() {
    std::vector<std::uint8_t> bytes;
    add_txt(bytes, "deviceid", kDeviceId);
    add_txt(bytes, "features", kFeatureText);
    add_txt(bytes, "pw", "false");
    add_txt(bytes, "flags", "0x4");
    add_txt(bytes, "model", kModel);
    add_txt(bytes, "pk", kPublicKey);
    add_txt(bytes, "pi", kPairId);
    add_txt(bytes, "srcvers", kSourceVersion);
    add_txt(bytes, "vv", kProtocolVersion);
    return bytes;
}

static std::vector<std::uint8_t> raop_txt() {
    std::vector<std::uint8_t> bytes;
    add_txt(bytes, "ch", "2");
    add_txt(bytes, "cn", "0,1,2,3");
    add_txt(bytes, "da", "true");
    add_txt(bytes, "et", "0,3,5");
    add_txt(bytes, "ft", kFeatureText);
    add_txt(bytes, "am", kModel);
    add_txt(bytes, "md", "0,1,2");
    add_txt(bytes, "rhd", "0x0");
    add_txt(bytes, "pw", "false");
    add_txt(bytes, "sr", "44100");
    add_txt(bytes, "ss", "16");
    add_txt(bytes, "sf", "0x4");
    add_txt(bytes, "sv", "false");
    add_txt(bytes, "tp", "UDP");
    add_txt(bytes, "txtvers", "1");
    add_txt(bytes, "vs", kSourceVersion);
    add_txt(bytes, "vn", "65537");
    add_txt(bytes, "vv", kProtocolVersion);
    add_txt(bytes, "pk", kPublicKey);
    return bytes;
}

static std::string base64_encode(const std::vector<std::uint8_t>& bytes) {
    static const char alphabet[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string encoded;
    std::size_t i = 0;
    while (i < bytes.size()) {
        const std::size_t remaining = bytes.size() - i;
        const std::uint32_t a = bytes[i++];
        const std::uint32_t b = remaining > 1 ? bytes[i++] : 0;
        const std::uint32_t c = remaining > 2 ? bytes[i++] : 0;
        const std::uint32_t triple = (a << 16) | (b << 8) | c;

        encoded.push_back(alphabet[(triple >> 18) & 0x3f]);
        encoded.push_back(alphabet[(triple >> 12) & 0x3f]);
        encoded.push_back(remaining > 1 ? alphabet[(triple >> 6) & 0x3f] : '=');
        encoded.push_back(remaining > 2 ? alphabet[triple & 0x3f] : '=');
    }
    return encoded;
}

static int hex_value(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

static std::vector<std::uint8_t> hex_to_bytes(const std::string& hex) {
    std::vector<std::uint8_t> bytes;
    for (std::size_t i = 0; i + 1 < hex.size(); i += 2) {
        const int high = hex_value(hex[i]);
        const int low = hex_value(hex[i + 1]);
        if (high < 0 || low < 0) {
            return std::vector<std::uint8_t>();
        }
        bytes.push_back(static_cast<std::uint8_t>((high << 4) | low));
    }
    return bytes;
}

static std::vector<std::uint8_t> request_body_bytes(const Request& request) {
    return std::vector<std::uint8_t>(request.body.begin(), request.body.end());
}

static std::vector<std::uint8_t> public_key_bytes() {
    return hex_to_bytes(kPublicKey);
}

static std::string plist_data(const std::vector<std::uint8_t>& bytes) {
    return "<data>" + base64_encode(bytes) + "</data>";
}

static Response make_info_response(const Request& request) {
    std::ostringstream body;
    body << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
         << "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" "
         << "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
         << "<plist version=\"1.0\"><dict>"
         << "<key>txtAirPlay</key>" << plist_data(airplay_txt())
         << "<key>txtRAOP</key>" << plist_data(raop_txt())
         << "<key>deviceID</key><string>" << kDeviceId << "</string>"
         << "<key>macAddress</key><string>" << kDeviceId << "</string>"
         << "<key>name</key><string>" << kReceiverName << "</string>"
         << "<key>pk</key>" << plist_data(hex_to_bytes(kPublicKey))
         << "<key>features</key><integer>" << kFeatures << "</integer>"
         << "<key>pi</key><string>" << kPairId << "</string>"
         << "<key>vv</key><integer>2</integer>"
         << "<key>statusFlags</key><integer>68</integer>"
         << "<key>keepAliveLowPower</key><integer>1</integer>"
         << "<key>sourceVersion</key><string>" << kSourceVersion << "</string>"
         << "<key>keepAliveSendStatsAsBody</key><true/>"
         << "<key>model</key><string>" << kModel << "</string>"
         << "</dict></plist>\n";

    Response response = make_plist_response(body.str());
    response.version = request.version.empty() ? "HTTP/1.1" : request.version;
    const std::string cseq = request.header_value("CSeq");
    if (!cseq.empty()) {
        response.set_header("CSeq", cseq);
    }
    return response;
}

static Response make_server_info_response() {
    std::ostringstream body;
    body << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
         << "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" "
         << "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
         << "<plist version=\"1.0\"><dict>"
         << "<key>deviceid</key><string>" << kDeviceId << "</string>"
         << "<key>features</key><integer>" << kFeatures << "</integer>"
         << "<key>model</key><string>" << kModel << "</string>"
         << "<key>protovers</key><string>1.1</string>"
         << "<key>srcvers</key><string>" << kSourceVersion << "</string>"
         << "</dict></plist>\n";
    return make_plist_response(body.str());
}

static Response make_rtsp_options_response(const Request& request) {
    Response response = make_empty_response(request, 200, "OK");
    response.set_header("Public",
                        "ANNOUNCE, SETUP, RECORD, PAUSE, FLUSH, TEARDOWN, OPTIONS, "
                        "GET_PARAMETER, SET_PARAMETER, GET");
    return response;
}

static Response make_pair_setup_response(const Request& request) {
    const std::vector<std::uint8_t> body = request_body_bytes(request);
    if (body.size() != 32) {
        return make_empty_response(request, 400, "Bad Request");
    }
    return make_binary_response(request, public_key_bytes());
}

static Response make_pair_verify_response(const Request& request) {
    const std::vector<std::uint8_t> body = request_body_bytes(request);
    if (body.size() < 4) {
        return make_empty_response(request, 400, "Bad Request");
    }

    if (body[0] == 1) {
        if (body.size() != 68) {
            return make_empty_response(request, 400, "Bad Request");
        }
        std::vector<std::uint8_t> response = public_key_bytes();
        response.insert(response.end(), kPairVerifySignature,
                        kPairVerifySignature + sizeof(kPairVerifySignature));
        return make_binary_response(request, response);
    }

    if (body[0] == 0) {
        if (body.size() != 68) {
            return make_empty_response(request, 400, "Bad Request");
        }
        return make_binary_response(request, std::vector<std::uint8_t>());
    }

    return make_empty_response(request, 400, "Bad Request");
}

static Response make_fairplay_setup_response(const Request& request) {
    const std::vector<std::uint8_t> body = request_body_bytes(request);
    std::vector<std::uint8_t> response;
    if (aplay::crypto::fairplay::setup_response(body, response) ||
        aplay::crypto::fairplay::handshake_response(body, response)) {
        return make_binary_response(request, response);
    }
    return make_empty_response(request, 400, "Bad Request");
}

Response make_receiver_response(const Request& request) {
    const std::string path = path_from_target(request.target);
    if (request.method == "GET" && path == "/server-info") {
        return make_server_info_response();
    }

    if (request.method == "GET" && path == "/info") {
        return make_info_response(request);
    }

    if (request.method == "GET" && path == "/playback-info") {
        return make_plist_response(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" "
            "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
            "<plist version=\"1.0\"><dict>"
            "<key>duration</key><real>0</real>"
            "<key>loadedTimeRanges</key><array/>"
            "<key>playbackBufferEmpty</key><true/>"
            "<key>playbackBufferFull</key><false/>"
            "<key>playbackLikelyToKeepUp</key><true/>"
            "<key>position</key><real>0</real>"
            "<key>rate</key><real>0</real>"
            "<key>readyToPlay</key><false/>"
            "</dict></plist>\n");
    }

    if (request.method == "POST" && path == "/reverse") {
        Response response;
        response.status_code = 101;
        response.reason = "Switching Protocols";
        response.set_header("Upgrade", "PTTH/1.0");
        response.set_header("Connection", "Upgrade");
        response.set_header("Content-Length", "0");
        return response;
    }

    if (request.method == "OPTIONS" && starts_with(request.version, "RTSP/")) {
        return make_rtsp_options_response(request);
    }

    if (request.method == "POST" && path == "/pair-setup") {
        return make_pair_setup_response(request);
    }

    if (request.method == "POST" && path == "/pair-verify") {
        return make_pair_verify_response(request);
    }

    if (request.method == "POST" && path == "/fp-setup") {
        return make_fairplay_setup_response(request);
    }

    if (request.method == "POST" && path == "/pair-pin-start") {
        return make_empty_response(request, 200, "OK");
    }

    if (request.method == "POST" && path == "/pair-setup-pin") {
        return make_empty_response(request, 470, "Client Authentication Failure");
    }

    if (request.method == "GET_PARAMETER" || request.method == "SET_PARAMETER" ||
        request.method == "FLUSH" || request.method == "TEARDOWN" ||
        request.method == "PAUSE") {
        return make_empty_response(request, 200, "OK");
    }

    if (request.method == "ANNOUNCE" || request.method == "SETUP" ||
        request.method == "RECORD" || request.method == "PLAY") {
        return make_empty_response(request, 501, "Not Implemented");
    }

    return make_text_response(404, "Not Found", "not found\n");
}

} // namespace http
} // namespace protocol
} // namespace aplay
