#include "aplay/runtime.hpp"

#include <iostream>
#include <sstream>
#include <string_view>
#include <vector>

int main() {
    std::ostringstream out;
    std::ostringstream err;
    const std::vector<std::string_view> args = {"aplay_harness_smoke", "--name", "APlayHarness", "--smoke-run", "1"};
    const aplay::linux_app::ParseResult parsed = aplay::linux_app::parse_arguments(args, err);
    if (parsed.exit_code != 0) {
        std::cerr << err.str();
        return parsed.exit_code;
    }

    const int rc = aplay::linux_app::run_runtime(parsed.options, out, err);
    if (rc != 0) {
        std::cerr << err.str();
        return rc;
    }

    const std::string output = out.str();
    if (output.find("APlay Linux app smoke runtime starting") == std::string::npos ||
        output.find("APlay Linux app smoke runtime stopped") == std::string::npos) {
        std::cerr << "smoke runtime did not produce expected lifecycle markers\n";
        return 1;
    }

    return 0;
}
