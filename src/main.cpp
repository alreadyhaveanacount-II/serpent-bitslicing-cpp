#include "../include/serpent.hpp"
#include "../include/utils/benchmark.hpp"
#include <cstdlib>
#include <vector>
#include <iostream>
#include <iomanip>
#include <span>

void print_hex(std::string_view label, std::span<const uint8_t> data) {
    std::cout << std::setw(10) << std::right << label << ": ";
    std::cout << std::hex << std::uppercase << std::setfill('0');
    for (uint8_t b : data) {
        std::cout << std::setw(2) << static_cast<int>(b);
    }
    std::cout << std::dec << std::endl;
}

bool test_vec_1() {
    /*
    Set 7, vector#255:
                           key=FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
                               FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
                        cipher=FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
                         plain=24B806363888422647BC5110E22A0F56
                     encrypted=FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
    */

    // Key: FFFFFFFF... (256 bits / 32 bytes)
    const std::array<uint32_t, 8> test_key = {
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
    };

    // Plain: 24B806363888422647BC5110E22A0F56
    Serpent_IV iv;
    iv.bytes_data = {
        0x24, 0xB8, 0x06, 0x36, 0x38, 0x88, 0x42, 0x26, 
        0x47, 0xBC, 0x51, 0x10, 0xE2, 0x2A, 0x0F, 0x56
    };

    Serpent e(test_key, iv);

    std::vector<uint8_t> pt(16, 0);
    std::vector<uint8_t> ct(16, 0);

    e.process_data(pt, ct);

    // Expected: FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

    std::array<uint8_t, 16> expected;
    expected.fill(0xFF);

    v128u v_ct = *reinterpret_cast<const v128u*>(ct.data());
    v128u v_exp = *reinterpret_cast<const v128u*>(expected.data());

    v128u diff = v_ct ^ v_exp;

    if ((diff[0] | diff[1] | diff[2] | diff[3]) != 0) {
        std::cerr << "[-] Test 1 FAILED!" << std::endl;
        print_hex("Obtained", ct);
        return false;
    }

    std::cout << "[+] Test 1 PASSED!" << std::endl;
    return true;
}

bool test_vec_2() {
    /*
    Set 8, vector#  0:
                           key=000102030405060708090A0B0C0D0E0F
                               101112131415161718191A1B1C1D1E1F
                         plain=3DA46FFA6F4D6F30CD258333E5A61369
                     encrypted=00112233445566778899AABBCCDDEEFF
    */

    const std::array<uint32_t, 8> test_key = {
        0x03020100, 0x07060504, 0x0B0A0908, 0x0F0E0D0C,
        0x13121110, 0x17161514, 0x1B1A1918, 0x1F1E1D1C
    };

    // Plain: 3DA46FFA6F4D6F30CD258333E5A61369
    Serpent_IV iv;
    iv.bytes_data = {
        0x3D, 0xA4, 0x6F, 0xFA, 0x6F, 0x4D, 0x6F, 0x30, 
        0xCD, 0x25, 0x83, 0x33, 0xE5, 0xA6, 0x13, 0x69
    };

    Serpent e(test_key, iv);

    std::vector<uint8_t> pt(16, 0);
    std::vector<uint8_t> ct(16, 0);

    e.process_data(pt, ct);

    // Expected: 00112233445566778899AABBCCDDEEFF
    std::array<uint8_t, 16> expected = {
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
    };

    v128u v_ct = *reinterpret_cast<const v128u*>(ct.data());
    v128u v_exp = *reinterpret_cast<const v128u*>(expected.data());

    v128u diff = v_ct ^ v_exp;

    if ((diff[0] | diff[1] | diff[2] | diff[3]) != 0) {
        std::cerr << "[-] Test 2 FAILED!" << std::endl;
        print_hex("Obtained", ct);
        return false;
    }

    std::cout << "[+] Test 2 PASSED!" << std::endl;
    return true;
}

bool test_vec_3() {
    /*
    Set 8, vector#  1:
                           key=2BD6459F82C5B300952C49104881FF48
                               2BD6459F82C5B300952C49104881FF48
                         plain=677C8DFAA08071743FD2B415D1B28AF2
                     encrypted=EA024714AD5C4D84EA024714AD5C4D84
    */

    // Convertendo a chave (2x a mesma sequência de 128 bits para formar 256)
    const std::array<uint32_t, 8> test_key = {
        0x9F45D62B, 0x00B3C582, 0x10492C95, 0x48FF8148, // 1ª metade
        0x9F45D62B, 0x00B3C582, 0x10492C95, 0x48FF8148  // 2ª metade
    };

    // Plain: 677C8DFAA08071743FD2B415D1B28AF2 (IV no CTR)
    Serpent_IV iv;
    iv.bytes_data = {
        0x67, 0x7C, 0x8D, 0xFA, 0xA0, 0x80, 0x71, 0x74, 
        0x3F, 0xD2, 0xB4, 0x15, 0xD1, 0xB2, 0x8A, 0xF2
    };

    Serpent e(test_key, iv);

    std::vector<uint8_t> pt(16, 0);
    std::vector<uint8_t> ct(16, 0);

    e.process_data(pt, ct);

    // Expected: EA024714AD5C4D84EA024714AD5C4D84
    alignas(16) std::array<uint8_t, 16> expected = {
        0xEA, 0x02, 0x47, 0x14, 0xAD, 0x5C, 0x4D, 0x84,
        0xEA, 0x02, 0x47, 0x14, 0xAD, 0x5C, 0x4D, 0x84
    };

    v128u v_ct = *reinterpret_cast<const v128u*>(ct.data());
    v128u v_exp = *reinterpret_cast<const v128u*>(expected.data());
    v128u diff = v_ct ^ v_exp;

    if ((diff[0] | diff[1] | diff[2] | diff[3]) != 0) {
        std::cerr << "[-] Test 3 FAILED!" << std::endl;
        print_hex("Obtained", ct);
        return false;
    }

    std::cout << "[+] Test 3 PASSED!" << std::endl;
    return true;
}


int main() {
    test_vec_1();
    test_vec_2();
    test_vec_3();

    // Benchmarking::set_high_priority();
    // Benchmarking::test_cipher();
    // Uncomment above lines to benchmark code
    
    return EXIT_SUCCESS;
}
