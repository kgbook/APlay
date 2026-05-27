#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {

struct RequestSummary {
    std::string method;
    std::string url;
    std::string protocol;
    int headers = 0;
};

bool parse_fixture(const char* path, RequestSummary& summary) {
    std::ifstream input(path);
    if (!input) {
        std::cerr << "failed to open fixture: " << path << '\n';
        return false;
    }

    std::string line;
    if (!std::getline(input, line)) {
        std::cerr << "fixture is empty\n";
        return false;
    }

    std::istringstream request_line(line);
    request_line >> summary.method >> summary.url >> summary.protocol;
    if (summary.method.empty() || summary.url.empty() || summary.protocol.empty()) {
        std::cerr << "invalid request line: " << line << '\n';
        return false;
    }

    while (std::getline(input, line)) {
        if (line.empty() || line == "\r") {
            break;
        }
        ++summary.headers;
    }

    return true;
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: aplay_harness_protocol_replay <fixture>\n";
        return 2;
    }

    RequestSummary summary;
    if (!parse_fixture(argv[1], summary)) {
        return 1;
    }

    if (summary.method != "OPTIONS" || summary.protocol != "RTSP/1.0") {
        std::cerr << "phase0 protocol fixture must be an RTSP OPTIONS request\n";
        return 1;
    }

    std::cout << "{"
              << "\"method\":\"" << summary.method << "\","
              << "\"url\":\"" << summary.url << "\","
              << "\"protocol\":\"" << summary.protocol << "\","
              << "\"headers\":" << summary.headers
              << "}\n";
    return 0;
}
