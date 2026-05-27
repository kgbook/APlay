#pragma once

#include <iosfwd>
#include <string>
#include <string_view>
#include <vector>

namespace aplay::linux_app {

struct RuntimeOptions {
    bool show_help = false;
    bool show_version = false;
    bool smoke_run = false;
    int smoke_run_ms = 100;
    std::string server_name = "APlay";
};

struct ParseResult {
    RuntimeOptions options;
    int exit_code = 0;
};

ParseResult parse_arguments(const std::vector<std::string_view>& args, std::ostream& err);
int run_runtime(const RuntimeOptions& options, std::ostream& out, std::ostream& err);
void print_help(std::ostream& out, std::string_view argv0);
void print_version(std::ostream& out);

} // namespace aplay::linux_app
