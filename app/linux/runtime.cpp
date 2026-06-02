#include "aplay/runtime.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <limits>
#include <string>
#include <thread>

namespace aplay::linux_app {
namespace {

constexpr int kUsageError = 2;
constexpr int kMaxSmokeRunMs = 5000;

bool parse_int(std::string_view text, int& value) {
    if (text.empty()) {
        return false;
    }

    int parsed = 0;
    for (char ch : text) {
        if (ch < '0' || ch > '9') {
            return false;
        }
        const int digit = ch - '0';
        if (parsed > (std::numeric_limits<int>::max() - digit) / 10) {
            return false;
        }
        parsed = parsed * 10 + digit;
    }

    value = parsed;
    return true;
}

} // namespace

ParseResult parse_arguments(const std::vector<std::string_view>& args, std::ostream& err) {
    ParseResult result;

    for (std::size_t i = 1; i < args.size(); ++i) {
        const std::string_view arg = args[i];
        if (arg == "-h" || arg == "--help") {
            result.options.show_help = true;
        } else if (arg == "-v" || arg == "--version") {
            result.options.show_version = true;
        } else if (arg == "-n" || arg == "--name") {
            if (i + 1 >= args.size()) {
                err << "missing value for " << arg << '\n';
                result.exit_code = kUsageError;
                return result;
            }
            result.options.server_name = std::string(args[++i]);
        } else if (arg == "--smoke-run") {
            result.options.smoke_run = true;
            if (i + 1 < args.size()) {
                int parsed = 0;
                if (parse_int(args[i + 1], parsed)) {
                    result.options.smoke_run_ms = std::clamp(parsed, 1, kMaxSmokeRunMs);
                    ++i;
                }
            }
        } else {
            err << "unknown option: " << arg << '\n';
            result.exit_code = kUsageError;
            return result;
        }
    }

    return result;
}

int run_runtime(const RuntimeOptions& options, std::ostream& out, std::ostream& err) {
    if (options.show_help) {
        print_help(out, "aplay");
        return 0;
    }

    if (options.show_version) {
        print_version(out);
        return 0;
    }

    if (!options.smoke_run) {
        err << "no runtime mode selected; use --help for available options\n";
        return kUsageError;
    }

    out << "APlay Linux app smoke runtime starting: name=" << options.server_name
        << " duration_ms=" << options.smoke_run_ms << '\n';
    std::this_thread::sleep_for(std::chrono::milliseconds(options.smoke_run_ms));
    out << "APlay Linux app smoke runtime stopped\n";
    return 0;
}

void print_help(std::ostream& out, std::string_view argv0) {
    out << "Usage: " << argv0 << " [--help] [--version] [--name NAME] --smoke-run [MS]\n"
        << "\n"
        << "APlay Linux app is the host UI/business entry for the UxPlay-compatible receiver.\n"
        << "OSAL currently carries codec/render platform modules and native binding layout; utils carries cross-platform helpers.\n"
        << "\n"
        << "Options:\n"
        << "  -h, --help        Show this help.\n"
        << "  -v, --version     Show version.\n"
        << "  -n, --name NAME   Set advertised server name for runtime tests.\n"
        << "  --smoke-run [MS]  Start the minimal Linux app loop for MS milliseconds.\n";
}

void print_version(std::ostream& out) {
    const aplay::core::RuntimeInfo info = aplay::core::runtime_info();
    out << info.name << " Linux app " << info.version << " phase0\n";
}

} // namespace aplay::linux_app
