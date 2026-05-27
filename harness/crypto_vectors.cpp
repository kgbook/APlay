#include <array>
#include <cstdint>
#include <iostream>

namespace {

uint8_t xor_checksum(const std::array<uint8_t, 16>& data) {
    uint8_t checksum = 0;
    for (const uint8_t value : data) {
        checksum ^= value;
    }
    return checksum;
}

} // namespace

int main() {
    constexpr std::array<uint8_t, 16> vector = {
        0x00, 0x11, 0x22, 0x33,
        0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xaa, 0xbb,
        0xcc, 0xdd, 0xee, 0xff};

    if (xor_checksum(vector) != 0x00) {
        std::cerr << "phase0 checksum vector failed\n";
        return 1;
    }

    std::cout << "{\"crypto_vectors\":\"phase0-placeholder\",\"status\":\"pass\"}\n";
    return 0;
}
