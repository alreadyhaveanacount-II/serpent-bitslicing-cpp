#include "../include/utils/benchmark.hpp"
#include <cstdlib>

int main() {
    Benchmarking::set_high_priority();
    Benchmarking::test_cipher();

    return EXIT_SUCCESS;
}