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
 * reaching a theoretical max of ~3 C/B
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

    void process_16_blocks(v128u* blocks) {
        uint8_t curr_sbox = 0;

        v512u r0, r1, r2, r3;
        for(int i = 0; i < 16; ++i) {
            r0[i] = blocks[i][0]; r1[i] = blocks[i][1];
            r2[i] = blocks[i][2]; r3[i] = blocks[i][3];
        }

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

        for(int i = 0; i < 16; ++i) {
            blocks[i][0] = r0[i]; blocks[i][1] = r1[i];
            blocks[i][2] = r2[i]; blocks[i][3] = r3[i];
        }
    }

    void deprocess_16_blocks(v128u* blocks) {
        // Useless for CTR, just here for fun

        v512u r0, r1, r2, r3;
        for(int i = 0; i < 16; ++i) {
            r0[i] = blocks[i][0]; r1[i] = blocks[i][1];
            r2[i] = blocks[i][2]; r3[i] = blocks[i][3];
        }

        r0 ^= (v512u)subkeys[32][0];
        r1 ^= (v512u)subkeys[32][1];
        r2 ^= (v512u)subkeys[32][2];
        r3 ^= (v512u)subkeys[32][3];

        Bitsliced_SBoxes::inv_call_specific<v512u>(r0, r1, r2, r3, 7);

        r0 ^= (v512u)subkeys[31][0];
        r1 ^= (v512u)subkeys[31][1];
        r2 ^= (v512u)subkeys[31][2];
        r3 ^= (v512u)subkeys[31][3];

        int curr_sbox = 6;

        for(int i = 30; i >= 0; --i) {
            inv_linear_transform(r0, r1, r2, r3);

            Bitsliced_SBoxes::inv_call_specific<v512u>(r0, r1, r2, r3, curr_sbox);

            r0 ^= (v512u)subkeys[i][0];
            r1 ^= (v512u)subkeys[i][1];
            r2 ^= (v512u)subkeys[i][2];
            r3 ^= (v512u)subkeys[i][3];

            curr_sbox = (curr_sbox + 8 - 1) & 7;
        }

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
