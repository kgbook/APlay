#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: aplay_harness_rtp_replay <csv-fixture>\n";
        return 2;
    }

    std::ifstream input(argv[1]);
    if (!input) {
        std::cerr << "failed to open RTP fixture: " << argv[1] << '\n';
        return 1;
    }

    std::string line;
    unsigned previous_seq = 0;
    bool have_previous = false;
    int packets = 0;
    int gaps = 0;

    while (std::getline(input, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream row(line);
        unsigned seq = 0;
        unsigned timestamp = 0;
        unsigned payload_len = 0;
        char comma1 = 0;
        char comma2 = 0;
        row >> seq >> comma1 >> timestamp >> comma2 >> payload_len;
        if (!row || comma1 != ',' || comma2 != ',' || payload_len == 0) {
            std::cerr << "invalid RTP fixture row: " << line << '\n';
            return 1;
        }

        if (have_previous && seq != previous_seq + 1) {
            ++gaps;
        }
        previous_seq = seq;
        have_previous = true;
        ++packets;
    }

    if (packets == 0) {
        std::cerr << "RTP fixture has no packets\n";
        return 1;
    }

    std::cout << "{\"packets\":" << packets << ",\"gaps\":" << gaps << "}\n";
    return 0;
}
