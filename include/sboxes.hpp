#pragma once

#include <cstdint>

/**
 * @file sboxes.hpp
 * @brief This file contains the bitsliced implementations of the S-boxes used in the Serpent cipher.
 * 
 * Source comes from Dag Arne Osvik's paper "Speeding up Serpent", presented at the 
 * 3rd AES Candidate Conference (April 2000). The implementations utilize optimized 
 * Boolean circuits to achieve high performance via bitslicing.
 * 
 * @see https://asmaes.sourceforge.net/serpent/serpentImplementation.pdf
 */

#if defined(__clang__)
    typedef uint32_t v512u __attribute__((ext_vector_type(16)));
    typedef uint32_t v128u __attribute__((ext_vector_type(4)));
#elif defined(__GNUC__)
    typedef uint32_t v512u __attribute__((vector_size(64)));
    typedef uint32_t v128u __attribute__((vector_size(16)));
#else
    #error "Compiler not supported, use GCC or Clang"
#endif

/**
 * @namespace Bitsliced_SBoxes
 * @brief Bitsliced functions for the SBoxes
 */

namespace Bitsliced_SBoxes {
    template <typename T>
    inline void s0(T& r0, T& r1, T& r2, T& r3) noexcept {
        T r4;

        r3 ^= r0; r4 = r1;
        r1 &= r3; r4 ^= r2;
        r1 ^= r0; r0 |= r3;
        r0 ^= r4; r4 ^= r3;
        r3 ^= r2; r2 |= r1;
        r2 ^= r4; r4 = ~r4;
        r4 |= r1; r1 ^= r3;
        r1 ^= r4; r3 |= r0;
        r1 ^= r3; r4 ^= r3;

        T temp_r0 = r0; 

        r0 = r1; r1 = r4; r3 = temp_r0;

        // r1, r4, r2, r0
    }

    template <typename T>
    inline void inv_s0(T& r0, T& r1, T& r2, T& r3) noexcept {
        T r4;

        r2 = ~r2; r4 = r1;
        r1 |= r0; r4 = ~r4;
        r1 ^= r2; r2 |= r4;
        r1 ^= r3; r0 ^= r4;
        r2 ^= r0; r0 &= r3;
        r4 ^= r0; r0 |= r1;
        r0 ^= r2; r3 ^= r4;
        r2 ^= r1; r3 ^= r0;
        r3 ^= r1; r2 &= r3; r4 ^= r2;

        T temp_r1 = r1;

        r1 = r4; r2 = temp_r1;

        // r0, r4, r1, r3
    }
    
    ////////////////////////////////////////

    template <typename T>
    inline void s1(T& r0, T& r1, T& r2, T& r3) noexcept {
        T r4;

        r0 = ~r0; r2 = ~r2;
        r4 = r0; r0 &= r1;
        r2 ^= r0; r0 |= r3;
        r3 ^= r2; r1 ^= r0;
        r0 ^= r4; r4 |= r1;
        r1 ^= r3; r2 |= r0;
        r2 &= r4; r0 ^= r1;
        r1 &= r2;
        r1 ^= r0; r0 &= r2;
        r0 ^= r4;

        T temp_r0 = r0; T temp_r1 = r1;

        r0 = r2; r1 = temp_r0; r2 = r3; r3 = temp_r1;

        // r2, r0, r3, r1
    }

    template <typename T>
    inline void inv_s1(T& r0, T& r1, T& r2, T& r3) noexcept {
        T r4;

        r4 = r1; r1 ^= r3;
        r3 &= r1; r4 ^= r2;
        r3 ^= r0; r0 |= r1;
        r2 ^= r3; r0 ^= r4;
        r0 |= r2; r1 ^= r3;
        r0 ^= r1; r1 |= r3;
        r1 ^= r0; r4 = ~r4;
        r4 ^= r1; r1 |= r0;
        r1 ^= r0; r1 |= r4; r3 ^= r1;

        T temp_r0 = r0; T temp_r2 = r2;

        r0 = r4; r1 = temp_r0; r2 = r3; r3 = temp_r2;

        // r4, r0, r3, r2
    }

    ////////////////////////////////////////

    template <typename T>
    inline void s2(T& r0, T& r1, T& r2, T& r3) noexcept {
        T r4;
        
        r4 = r0; r0 &= r2;
        r0 ^= r3; r2 ^= r1;
        r2 ^= r0; r3 |= r4;
        r3 ^= r1; r4 ^= r2;
        r1 = r3; r3 |= r4;
        r3 ^= r0; r0 &= r1;
        r4 ^= r0; r1 ^= r3;
        r1 ^= r4; r4 = ~r4;

        T temp_r1 = r1;

        r0 = r2; r1 = r3; r2 = temp_r1; r3 = r4;

        // r2, r3, r1, r4
    }

    template <typename T>
    inline void inv_s2(T& r0, T& r1, T& r2, T& r3) noexcept {
        T r4;
        
        r2 ^= r3; r3 ^= r0;
        r4 = r3; r3 &= r2;
        r3 ^= r1; r1 |= r2;
        r1 ^= r4; r4 &= r3;
        r2 ^= r3; r4 &= r0;
        r4 ^= r2; r2 &= r1;
        r2 |= r0; r3 = ~r3;
        r2 ^= r3; r0 ^= r3;
        r0 &= r1; r3 ^= r4;
        r3 ^= r0;

        r0 = r1; r1 = r4;

        // r1, r4, r2, r3
    }

    ////////////////////////////////////////

    template <typename T>
    inline void s3(T& r0, T& r1, T& r2, T& r3) noexcept {
        T r4;

        r4 = r0; r0 |= r3;
        r3 ^= r1; r1 &= r4;
        r4 ^= r2; r2 ^= r3;
        r3 &= r0; r4 |= r1;
        r3 ^= r4; r0 ^= r1;
        r4 &= r0; r1 ^= r3;
        r4 ^= r2; r1 |= r0;
        r1 ^= r2; r0 ^= r3;
        r2 = r1; r1 |= r3;
        r1 ^= r0;

        r0 = r1; r1 = r2; r2 = r3; r3 = r4;

        // r1, r2, r3, r4
    }

    template <typename T>
    inline void inv_s3(T& r0, T& r1, T& r2, T& r3) noexcept {
        T r4;

        r4 = r2; r2 ^= r1;
        r0 ^= r2; r4 &= r2;
        r4 ^= r0; r0 &= r1;
        r1 ^= r3; r3 |= r4;
        r2 ^= r3; r0 ^= r3;
        r1 ^= r4; r3 &= r2;
        r3 ^= r1; r1 ^= r0;
        r1 |= r2; r0 ^= r3;
        r1 ^= r4;
        r0 ^= r1;

        T temp_r0 = r0;

        r0 = r2; r2 = r3; r3 = temp_r0;

        // r2, r1, r3, r0
    }

    ////////////////////////////////////////

    template <typename T>
    inline void s4(T& r0, T& r1, T& r2, T& r3) noexcept {
        T r4;

        r1 ^= r3; r3 = ~r3;
        r2 ^= r3; r3 ^= r0;
        r4 = r1; r1 &= r3;
        r1 ^= r2; r4 ^= r3;
        r0 ^= r4; r2 &= r4;
        r2 ^= r0; r0 &= r1;
        r3 ^= r0; r4 |= r1;
        r4 ^= r0; r0 |= r3;
        r0 ^= r2; r2 &= r3;
        r0 = ~r0; r4 ^= r2;

        T temp_r0 = r0;

        r0 = r1; r1 = r4; r2 = temp_r0;

        // r1, r4, r0, r3
    }

    template <typename T>
    inline void inv_s4(T& r0, T& r1, T& r2, T& r3) noexcept {
        T r4;

        r4 = r2; r2 &= r3;
        r2 ^= r1; r1 |= r3;
        r1 &= r0; r4 ^= r2;
        r4 ^= r1; r1 &= r2;
        r0 = ~r0; r3 ^= r4;
        r1 ^= r3; r3 &= r0;
        r3 ^= r2; r0 ^= r1;
        r2 &= r0; r3 ^= r0;
        r2 ^= r4;
        r2 |= r3; r3 ^= r0;
        r2 ^= r1;

        r1 = r3; r3 = r4;
    }

    ////////////////////////////////////////

    template <typename T>
    inline void s5(T& r0, T& r1, T& r2, T& r3) noexcept {
        T r4;

        r0 ^= r1; r1 ^= r3;
        r3 = ~r3; r4 = r1;
        r1 &= r0; r2 ^= r3;
        r1 ^= r2; r2 |= r4;
        r4 ^= r3; r3 &= r1;
        r3 ^= r0; r4 ^= r1;
        r4 ^= r2; r2 ^= r0;
        r0 &= r3; r2 = ~r2;
        r0 ^= r4; r4 |= r3;
        r2 ^= r4;

        T temp_r0 = r0; T temp_r2 = r2;

        r0 = r1; r1 = r3; r2 = temp_r0; r3 = temp_r2;

        // r1, r3, r0, r2
    }

    template <typename T>
    inline void inv_s5(T& r0, T& r1, T& r2, T& r3) noexcept {
        T r4;

        r1 = ~r1; r4 = r3;
        r2 ^= r1; r3 |= r0;
        r3 ^= r2; r2 |= r1;
        r2 &= r0; r4 ^= r3;
        r2 ^= r4; r4 |= r0;
        r4 ^= r1; r1 &= r2;
        r1 ^= r3; r4 ^= r2;
        r3 &= r4; r4 ^= r1;
        r3 ^= r4; r4 = ~r4;
        r3 ^= r0;

        T temp_r2 = r2;

        r0 = r1; r1 = r4; r2 = r3; r3 = temp_r2;

        // r1, r4, r3, r2
    }

    ////////////////////////////////////////

    template <typename T>
    inline void s6(T& r0, T& r1, T& r2, T& r3) noexcept {
        T r4;

        r2 = ~r2; r4 = r3;
        r3 &= r0; r0 ^= r4;
        r3 ^= r2; r2 |= r4;
        r1 ^= r3; r2 ^= r0;
        r0 |= r1; r2 ^= r1;
        r4 ^= r0; r0 |= r3;
        r0 ^= r2; r4 ^= r3;
        r4 ^= r0; r3 = ~r3;
        r2 &= r4;
        r2 ^= r3;

        T temp_r2 = r2;

        r2 = r4; r3 = temp_r2;

        // r0, r1, r4, r2
    }

    template <typename T>
    inline void inv_s6(T& r0, T& r1, T& r2, T& r3) noexcept {
        T r4;

        r0 ^= r2; r4 = r2;
        r2 &= r0; r4 ^= r3;
        r2 = ~r2; r3 ^= r1;
        r2 ^= r3; r4 |= r0;
        r0 ^= r2; r3 ^= r4;
        r4 ^= r1; r1 &= r3;
        r1 ^= r0; r0 ^= r3;
        r0 |= r2; r3 ^= r1;
        r4 ^= r0;

        r0 = r1; r1 = r2; r2 = r4;

        // r1, r2, r4, r3
    }

    ////////////////////////////////////////

    template <typename T>
    inline void s7(T& r0, T& r1, T& r2, T& r3) noexcept {
        T r4;

        r4 = r1; r1 |= r2;
        r1 ^= r3; r4 ^= r2;
        r2 ^= r1; r3 |= r4;
        r3 &= r0; r4 ^= r2;
        r3 ^= r1; r1 |= r4;
        r1 ^= r0; r0 |= r4;
        r0 ^= r2; r1 ^= r4;
        r2 ^= r1; r1 &= r0;
        r1 ^= r4; r2 = ~r2;
        r2 |= r0;
        r4 ^= r2;

        T temp_r0 = r0; T temp_r1 = r1;

        r0 = r4; r1 = r3; r2 = temp_r1; r3 = temp_r0;

        // r4, r3, r1, r0
    }

    template <typename T>
    inline void inv_s7(T& r0, T& r1, T& r2, T& r3) noexcept {
        T r4;

        r4 = r2; r2 ^= r0;
        r0 &= r3; r4 |= r3;
        r2 = ~r2; r3 ^= r1;
        r1 |= r0; r0 ^= r2;
        r2 &= r4; r3 &= r4;
        r1 ^= r2; r2 ^= r0;
        r0 |= r2; r4 ^= r1;
        r0 ^= r3; r3 ^= r4;
        r4 |= r0; r3 ^= r2;
        r4 ^= r2;

        T temp_r0 = r0; T temp_r1 = r1;

        r0 = r3; r1 = temp_r0; r2 = temp_r1; r3 = r4;

        // r3, r0, r1, r4
    }

    ////////////////////////////////
    // helper function

    template <typename T>
    inline void call_specific(T& r0, T& r1, T& r2, T& r3, uint8_t which) noexcept {
        switch(which) {
            case 0:
                s0(r0, r1, r2, r3);
                break;
            case 1:
            s1(r0, r1, r2, r3);
                break;
            case 2:
            s2(r0, r1, r2, r3);
                break;
            case 3:
            s3(r0, r1, r2, r3);
                break;
            case 4:
            s4(r0, r1, r2, r3);
                break;
            case 5:
            s5(r0, r1, r2, r3);
                break;
            case 6:
            s6(r0, r1, r2, r3);
                break;
            case 7:
            s7(r0, r1, r2, r3);
                break;
            default:
                return;
        }
    }

    template <typename T>
    inline void inv_call_specific(T& r0, T& r1, T& r2, T& r3, uint8_t which) noexcept {
        switch(which) {
            case 0:
                inv_s0(r0, r1, r2, r3);
                break;
            case 1:
            inv_s1(r0, r1, r2, r3);
                break;
            case 2:
            inv_s2(r0, r1, r2, r3);
                break;
            case 3:
            inv_s3(r0, r1, r2, r3);
                break;
            case 4:
            inv_s4(r0, r1, r2, r3);
                break;
            case 5:
            inv_s5(r0, r1, r2, r3);
                break;
            case 6:
            inv_s6(r0, r1, r2, r3);
                break;
            case 7:
            inv_s7(r0, r1, r2, r3);
                break;
            default:
                return;
        }
    }
}
