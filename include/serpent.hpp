#pragma once

#include <cstdint>
#include <array>
#include <bit>
#include <span>
#include "sboxes.hpp"
#include <array>
#include "./utils/mem_utils.hpp"

/**
 * @file serpent.hpp
 * @brief High performance implementation of Serpent using bitslice and 512-bit registers
 * 
 * This implementation manages to process 16 blocks in parallel using bitslicing techniques
 * reaching a theoretical max of ~1.5 C/B (If your CPU supports AVX-512), or ~3 C/B (If your CPU doesn't support AVX-512)
 * 
 * @author Leonardo
 * @date 2026
 */


 /**
 * @union Serpent_IV
 * @brief IV for Serpent Cipher
 */

union alignas(16) Serpent_IV {
    std::array<uint8_t, 16> bytes_data;
    v128u vector_form;
};

/**
 * @class Serpent
 * @brief Main class for the Serpent cipher.
 * 
 * AES finalist, offering 32 rounds of security.
 * This implementation focuses on aggresive optimization through SIMD instructions.
 * 
 * @note Requires AVX-512 support
 */

#if defined(__clang__)
    #define PRAGMA_UNROLL_16 _Pragma("clang loop unroll_count(16)")
    #define PRAGMA_UNROLL_FULL _Pragma("clang loop unroll(full)")
#elif defined(__GNUC__)
    #define PRAGMA_UNROLL_16 _Pragma("GCC unroll 16")
    #define PRAGMA_UNROLL_FULL _Pragma("GCC unroll 32")
#else
    #error "Compiler not supported, use GCC or Clang"
#endif

struct Serpent {
private:
    v128u subkeys[33];
    Serpent_IV iv;
    Serpent_IV org_iv;

    void expandKey(const std::array<uint32_t, 8> user_key);

    inline v512u _rotl_512(v512u v, uint8_t amount) {
        return (v << amount) | (v >> (32 - amount));
    }

    inline v512u _rotr_512(v512u v, uint8_t amount) {
        return (v >> amount) | (v << (32 - amount));
    }

    void linear_transform(v512u& x0, v512u& x1, v512u& x2, v512u& x3) {
        x0 = _rotl_512(x0, 13);
        x2 = _rotl_512(x2, 3);
        x1 = x1 ^ x0 ^ x2;
        x3 = x3 ^ x2 ^ (x0 << 3);
        x1 = _rotl_512(x1, 1);
        x3 = _rotl_512(x3, 7);
        x0 = x0 ^ x1 ^ x3;
        x2 = x2 ^ x3 ^ (x1 << 7);
        x0 = _rotl_512(x0, 5);
        x2 = _rotl_512(x2, 22);
    }

    void inv_linear_transform(v512u& x0, v512u& x1, v512u& x2, v512u& x3) {
        x2 = _rotr_512(x2, 22);
        x0 = _rotr_512(x0, 5);
        x2 = x2 ^ x3 ^ (x1 << 7);
        x0 = x0 ^ x1 ^ x3;
        x3 = _rotr_512(x3, 7);
        x1 = _rotr_512(x1, 1);
        x3 = x3 ^ x2 ^ (x0 << 3);
        x1 = x1 ^ x0 ^ x2;
        x2 = _rotr_512(x2, 3);
        x0 = _rotr_512(x0, 13);
    }

    inline void s_box_16(v128u* block, uint8_t s_box) {
        v512u r0 { block[0][0], block[1][0], block[2][0], block[3][0], block[4][0], block[5][0], block[6][0], block[7][0], block[8][0], block[9][0], block[10][0], block[11][0], block[12][0], block[13][0], block[14][0], block[15][0] };
        v512u r1 { block[0][1], block[1][1], block[2][1], block[3][1], block[4][1], block[5][1], block[6][1], block[7][1], block[8][1], block[9][1], block[10][1], block[11][1], block[12][1], block[13][1], block[14][1], block[15][1] };
        v512u r2 { block[0][2], block[1][2], block[2][2], block[3][2], block[4][2], block[5][2], block[6][2], block[7][2], block[8][2], block[9][2], block[10][2], block[11][2], block[12][2], block[13][2], block[14][2], block[15][2] };
        v512u r3 { block[0][3], block[1][3], block[2][3], block[3][3], block[4][3], block[5][3], block[6][3], block[7][3], block[8][3], block[9][3], block[10][3], block[11][3], block[12][3], block[13][3], block[14][3], block[15][3] };
        
        Bitsliced_SBoxes::call_specific<v512u>(r0, r1, r2, r3, s_box);

        PRAGMA_UNROLL_16
        for(int i = 0; i < 16; ++i) {
            block[i][0] = r0[i];
            block[i][1] = r1[i];
            block[i][2] = r2[i];
            block[i][3] = r3[i];
        }
    }

    inline void inv_s_box_16(v128u* block, uint8_t s_box) {
        v512u r0 { block[0][0], block[1][0], block[2][0], block[3][0], block[4][0], block[5][0], block[6][0], block[7][0], block[8][0], block[9][0], block[10][0], block[11][0], block[12][0], block[13][0], block[14][0], block[15][0] };
        v512u r1 { block[0][1], block[1][1], block[2][1], block[3][1], block[4][1], block[5][1], block[6][1], block[7][1], block[8][1], block[9][1], block[10][1], block[11][1], block[12][1], block[13][1], block[14][1], block[15][1] };
        v512u r2 { block[0][2], block[1][2], block[2][2], block[3][2], block[4][2], block[5][2], block[6][2], block[7][2], block[8][2], block[9][2], block[10][2], block[11][2], block[12][2], block[13][2], block[14][2], block[15][2] };
        v512u r3 { block[0][3], block[1][3], block[2][3], block[3][3], block[4][3], block[5][3], block[6][3], block[7][3], block[8][3], block[9][3], block[10][3], block[11][3], block[12][3], block[13][3], block[14][3], block[15][3] };
        
        Bitsliced_SBoxes::inv_call_specific<v512u>(r0, r1, r2, r3, s_box);

        PRAGMA_UNROLL_16
        for(int i = 0; i < 16; ++i) {
            block[i][0] = r0[i];
            block[i][1] = r1[i];
            block[i][2] = r2[i];
            block[i][3] = r3[i];
        }
    }

    void process_16_blocks(v128u* blocks) {
        uint8_t curr_sbox = 0;

        v512u r0, r1, r2, r3;

        PRAGMA_UNROLL_16
        for(int i = 0; i < 16; ++i) {
            r0[i] = blocks[i][0]; r1[i] = blocks[i][1];
            r2[i] = blocks[i][2]; r3[i] = blocks[i][3];
        }

        PRAGMA_UNROLL_FULL
        for(size_t i = 0; i < 32; i++) {
            r0 ^= (v512u)subkeys[i][0];
            r1 ^= (v512u)subkeys[i][1];
            r2 ^= (v512u)subkeys[i][2];
            r3 ^= (v512u)subkeys[i][3];

            Bitsliced_SBoxes::call_specific<v512u>(r0, r1, r2, r3, curr_sbox);

            if(i < 31) {
                linear_transform(r0, r1, r2, r3);
            } else {
                r0 ^= (v512u)subkeys[32][0];
                r1 ^= (v512u)subkeys[32][1];
                r2 ^= (v512u)subkeys[32][2];
                r3 ^= (v512u)subkeys[32][3];
            }

            curr_sbox = (curr_sbox + 1) & 7;
        }

        PRAGMA_UNROLL_16
        for(int i = 0; i < 16; ++i) {
            blocks[i][0] = r0[i]; blocks[i][1] = r1[i];
            blocks[i][2] = r2[i]; blocks[i][3] = r3[i];
        }
    }

public:
    // 12 bytes of IV + 4 bytes of counter

    Serpent(const std::array<uint32_t, 8>& user_key, Serpent_IV iv) : iv(iv), org_iv(iv) {
        CryptoHelper::lock_memory(this, sizeof(Serpent));
        expandKey(user_key);
    }

    ~Serpent() {
        CryptoHelper::secure_zero_memory(this, sizeof(Serpent));
        CryptoHelper::unlock_memory(this, sizeof(Serpent));
    }

    Serpent& operator=(const Serpent&) = delete;
    Serpent& operator=(const Serpent&&) = delete;
    Serpent(const Serpent&) = delete;
    Serpent(Serpent&&) = delete;

    void process_data(const std::span<const uint8_t> plaintext, std::span<uint8_t> ciphertext) {
        size_t i = 0;
        v128u keystreams[16];

        for (; i + 256 <= plaintext.size(); i += 256) {
            for (size_t j = 0; j < 16; j++) {
                keystreams[j] = iv.vector_form;
                iv.vector_form += 1;
            }

            process_16_blocks(keystreams);

            const v128u* p_in = reinterpret_cast<const v128u*>(&plaintext[i]);
            v128u* p_out = reinterpret_cast<v128u*>(&ciphertext[i]);

            for (size_t j = 0; j < 16; j++) {
                p_out[j] = p_in[j] ^ keystreams[j];
            }
        }

        if (i < plaintext.size()) {
            for (size_t j = 0; j < 16; j++) {
                keystreams[j] = iv.vector_form;
                iv.vector_form += 1;
            }

            process_16_blocks(keystreams);

            size_t remaining = plaintext.size() - i;
            uint8_t* final_ks = reinterpret_cast<uint8_t*>(keystreams);
            for (size_t j = 0; j < remaining; j++) {
                ciphertext[i + j] = plaintext[i + j] ^ final_ks[j];
            }
        }
    }

    void reset() {
        iv = org_iv;
    }
};

void Serpent::expandKey(const std::array<uint32_t, 8> user_key) {
    std::array<uint32_t, 140> prekey;

    constexpr uint32_t golden_ratio = 0x9e3779b9;

    for(int i = 0; i < 8; ++i) {
        prekey[i] = user_key[i];
    }

    for(size_t i=8; i < 140; i++) {
        uint32_t temp = prekey[i-8] ^ prekey[i-5] ^ prekey[i-3] ^ prekey[i-1] ^ golden_ratio ^ (i - 8);
        prekey[i] = std::rotl(temp, 11);
    }

    uint8_t curr_sbox = 3;
    uint8_t count = 0;

    for(size_t i=8; i < 140; i += 4) {
        v128u r0 = prekey[i], r1 = prekey[i+1], r2 = prekey[i+2], r3 = prekey[i+3];

        Bitsliced_SBoxes::call_specific<v128u>(r0, r1, r2, r3, curr_sbox);

        v128u curr {r0[0], r1[0], r2[0], r3[0]};

        subkeys[count] = curr;
        count++;

        curr_sbox = (curr_sbox - 1 + 8) % 8;
    }
}
