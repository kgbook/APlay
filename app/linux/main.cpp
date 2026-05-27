#include "aplay/runtime.hpp"

#include <iostream>
#include <string_view>
#include <vector>

int main(int argc, char** argv) {
    std::vector<std::string_view> args;
    args.reserve(static_cast<std::size_t>(argc));
    for (int i = 0; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }

    const aplay::linux_app::ParseResult parsed = aplay::linux_app::parse_arguments(args, std::cerr);
    if (parsed.exit_code != 0) {
        return parsed.exit_code;
    }

    return aplay::linux_app::run_runtime(parsed.options, std::cout, std::cerr);
}
