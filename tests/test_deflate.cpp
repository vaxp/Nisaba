#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include "nisaba/image/deflate.hpp"

using namespace nisaba::image;

void test_crc32() {
    std::cout << "[TEST] CRC-32 verification... ";
    // Standard test vector: "123456789" -> 0xCBF43926
    const std::string test_str = "123456789";
    uint32_t crc = crc32(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(test_str.data()), test_str.size()));
    assert(crc == 0xCBF43926U);
    std::cout << "PASSED (CRC: 0x" << std::hex << crc << std::dec << ")\n";
}

void test_adler32() {
    std::cout << "[TEST] Adler-32 verification... ";
    // Standard test vector: "123456789" -> 0x091E01DE
    const std::string test_str = "123456789";
    uint32_t adler = adler32(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(test_str.data()), test_str.size()));
    assert(adler == 0x091E01DEU);
    std::cout << "PASSED (Adler: 0x" << std::hex << adler << std::dec << ")\n";
}

void test_deflate_roundtrip() {
    std::cout << "[TEST] Deflate roundtrip verification... ";

    std::string text = "Nisaba Sovereign 2D Graphics Engine - Pure C++20 Zero Dependencies! "
                       "Rendering pixels with blazing speed, analytical precision, and mathematical elegance. "
                       "Repeat: Nisaba Sovereign 2D Graphics Engine - Pure C++20 Zero Dependencies! "
                       "Pulsar core, glowing gradients, glassmorphism, and sovereign PNG/JPEG codecs!";
    
    std::span<const uint8_t> input(reinterpret_cast<const uint8_t*>(text.data()), text.size());

    // Test levels 0, 1, 6
    for (int lvl : {0, 1, 6}) {
        auto compressed = deflate_compress(input, lvl);
        assert(compressed.has_value());
        assert(!compressed->empty());

        auto decompressed = deflate_decompress(*compressed);
        assert(decompressed.has_value());
        assert(decompressed->size() == input.size());
        assert(std::memcmp(decompressed->data(), input.data(), input.size()) == 0);
    }

    std::cout << "PASSED\n";
}

void test_zlib_roundtrip() {
    std::cout << "[TEST] Zlib wrapper roundtrip verification... ";

    std::vector<uint8_t> data(10000);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<uint8_t>((i * 7 + (i % 31)) % 256);
    }

    auto compressed = zlib_compress(data, 6);
    assert(compressed.has_value());
    assert(compressed->size() < data.size()); // Should compress well

    auto decompressed = zlib_decompress(*compressed);
    assert(decompressed.has_value());
    assert(decompressed->size() == data.size());
    assert(std::memcmp(decompressed->data(), data.data(), data.size()) == 0);

    std::cout << "PASSED (Ratio: " << (compressed->size() * 100.0 / data.size()) << "%)\n";
}

int main() {
    std::cout << "=== Running Nisaba Deflate & Checksum Tests ===\n";
    test_crc32();
    test_adler32();
    test_deflate_roundtrip();
    test_zlib_roundtrip();
    std::cout << "=== All Deflate Tests Passed! ===\n";
    return 0;
}
