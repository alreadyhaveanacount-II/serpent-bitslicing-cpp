# Serpent

A high-performance bitsliced Serpent implementation optimized for AVX512 architectures.

---
## Building

### Windows (MSVC/PowerShell)
```
mkdir build && cd build
cmake .. -G "Ninja" -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Linux

```
mkdir build
cd build
cmake .. -G "Ninja" -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release
ninja
```

> **Note for Windows users:** Ensure `clang++` and `ninja` are in your PATH.

> **If you're on Linux:** If clang `clang++` and `ninja` are not installed, use the command: `sudo apt update && sudo apt install clang ninja-build`


---

## Features

- **Serpent Block Cipher**: Full 32-round implementation optimized using **Bitslicing**.
- **CTR Mode Operation**: High-performance stream cipher construction allowing massive parallelism.
- **Bitsliced S-Boxes**: Utilizes Dag Arne Osvik's optimized logical sequences for maximum efficiency.
- **High-Precision Benchmarking**: Performance tracking using Cycles Per Byte (CPB) via `RDTSCP` and `LFENCE` serialization.
- **Detailed Throughput Analysis**: MB/s metrics with Average, Best, and Worst case scenarios.
- **Statistical Analysis**: Includes Interquartile Range (IQR) to filter system noise and OS jitter.
- **Modern C++20 Architecture**: 
    - Type-safe data handling using `std::span`.
    - Object-oriented design with a clean, decoupled API.
- **Secure Memory Management**: Strategic use of memory zeroing and raw buffer access for performance.

---

## Security Notes

- **Educational / Experimental**: Not intended for production use.
- **Constant-time**: The implementation avoids all data-dependent branching and memory access patterns.
- **Verified against test vectors**: Verified against the [official test vectors](https://biham.cs.technion.ac.il).
- **PractRand Validated**: Passed 1GB+ testing with no significant anomalies (10k+ tests).

---

## Design Notes

- **Bitslicing**: Processes multiple blocks simultaneously by transposing data into bit-planes for SIMD-like efficiency.
- **Linear Transformation**: Optimized to maintain diffusion while staying within the bitsliced domain.


---

## Performance results

**Note: Benchmarks performed on an Intel i5-1334U. Results reflect performance without AVX-512 instructions, utilizing standard AVX/AVX2 SIMD sets.**

```
=======================================================
 Serpent performance tests
=======================================================
[ THROUGHPUT ]
  Best:                         829.2648MB/s
  Worst:                        610.4028MB/s
  Average:                      799.8079MB/s
  Amplitude:                    218.8620MB/s
  IQR:                           37.0288MB/s

[ TIME (seconds) ]
  Smallest:                       0.0096s
  Biggest:                        0.0131s
  Average:                        0.0100s
  Amplitude:                      0.0035s
  IQR:                            0.0005s
[ EFFICIENCY ]
  Average CPB:                    2.9850 c/B
=======================================================
```
