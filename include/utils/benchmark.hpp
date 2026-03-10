#pragma once

#include <cstdint>
#include <stdexcept>
#define NOMINMAX
#include <chrono>
#include <limits>
#include <numeric>
#include <numeric>
#include <vector>
#include <algorithm>
#include <iostream>
#include "../serpent.hpp"

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#elif defined(__linux__) || defined(__GLIBC__)
#include <sched.h>
#include <emmintrin.h>
#include <x86intrin.h>
#endif


namespace Benchmarking {
	void set_high_priority() {
#if defined(_WIN32) || defined(_WIN64)
		DWORD_PTR cpuset = 0;

		cpuset |= (1ULL << 0);

		HANDLE currentThread = GetCurrentThread();
		SetThreadAffinityMask(currentThread, cpuset);
#elif defined(__linux__) || defined(__GLIBC__)
		cpu_set_t cpuset;
		CPU_ZERO(&cpuset);
		CPU_SET(0, &cpuset);
		pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
#endif
	}

	inline uint64_t read_cycles() {
		_mm_lfence();
		unsigned int ui;
		return __rdtscp(&ui);
	}

	struct PerformanceResults {
        double average_throughput = 0.0;
        double best_throughput = 0.0;
        double worst_throughput = 0.0;
        double throughput_amplitude = 0.0;
        double throughput_iqr = 0.0;
        double average_cpb = 0.0;
        std::chrono::duration<double> biggest_time = std::chrono::duration<double>::zero();
        std::chrono::duration<double> smallest_time = std::chrono::duration<double>::max();
        std::chrono::duration<double> average_time = std::chrono::duration<double>::zero();
        std::chrono::duration<double> time_amplitude = std::chrono::duration<double>::zero();
        std::chrono::duration<double> time_iqr = std::chrono::duration<double>::zero();

        void print(const std::string& title) const {
            const int label_w = 25;
            const int value_w = 15;

            std::cout << "\n=======================================================" << std::endl;
            std::cout << " " << title << std::endl;
            std::cout << "=======================================================" << std::endl;

            std::cout << std::fixed << std::setprecision(4);

            std::cout << "[ THROUGHPUT ]" << std::endl;
            std::cout << std::left << std::setw(label_w) << "  Best:" 
                    << std::right << std::setw(value_w) << best_throughput << "MB/s" << std::endl;
            std::cout << std::left << std::setw(label_w) << "  Worst:" 
                    << std::right << std::setw(value_w) 
                    << (worst_throughput == (std::numeric_limits<double>::max)() ? 0.0 : worst_throughput)
                    << "MB/s" << std::endl;
            std::cout << std::left << std::setw(label_w) << "  Average:" 
                    << std::right << std::setw(value_w) << average_throughput << "MB/s" << std::endl;
            std::cout << std::left << std::setw(label_w) << "  Amplitude:" 
                    << std::right << std::setw(value_w) << throughput_amplitude << "MB/s" << std::endl;
            std::cout << std::left << std::setw(label_w) << "  IQR:" 
                    << std::right << std::setw(value_w) << throughput_iqr << "MB/s" << std::endl;

            std::cout << "\n[ TIME (seconds) ]" << std::endl;
            std::cout << std::left << std::setw(label_w) << "  Smallest:" 
                    << std::right << std::setw(value_w) << smallest_time.count() << "s" << std::endl;
            std::cout << std::left << std::setw(label_w) << "  Biggest:" 
                    << std::right << std::setw(value_w) << biggest_time.count() << "s" << std::endl;
            std::cout << std::left << std::setw(label_w) << "  Average:" 
                    << std::right << std::setw(value_w) << average_time.count() << "s" << std::endl;
            std::cout << std::left << std::setw(label_w) << "  Amplitude:" 
                    << std::right << std::setw(value_w) << time_amplitude.count() << "s" << std::endl;
            std::cout << std::left << std::setw(label_w) << "  IQR:" 
                    << std::right << std::setw(value_w) << time_iqr.count() << "s" << std::endl;

            std::cout << "[ EFFICIENCY ]" << std::endl;
            std::cout << std::left << std::setw(label_w) << "  Average CPB:" 
                    << std::right << std::setw(value_w) << average_cpb << " c/B" << std::endl;
            std::cout << "=======================================================\n" << std::endl;
        }
    };

	struct PerformanceMetric {
	private:
		std::vector<std::chrono::duration<double>> times;
		std::vector<double> throughputs;
		std::vector<uint64_t> total_cycles;
		double bytes_per_run;
	public:
		PerformanceMetric(size_t reserve_size, double bytes_): bytes_per_run(bytes_) {
			times.reserve(reserve_size);
			throughputs.reserve(reserve_size);
			total_cycles.reserve(reserve_size);
		}

		void pushMetrics(std::chrono::duration<double> time, double throughput, double cpb) {
			times.push_back(time);
			throughputs.push_back(throughput);
			total_cycles.push_back(cpb);
		}

		PerformanceResults finish() {
			if (throughputs.empty()) {
				throw std::runtime_error("No benchmarks to evaluate");
			}

			PerformanceResults r;
			const double n = static_cast<double>(throughputs.size());

			double throughput_sum = std::accumulate(throughputs.begin(), throughputs.end(), 0.0);
			r.average_throughput = throughput_sum / n;

			auto time_sum = std::accumulate(times.begin(), times.end(), std::chrono::duration<double>::zero());
			r.average_time = time_sum / n;

			auto [min_t_it, max_t_it] = std::minmax_element(throughputs.begin(), throughputs.end());
			r.worst_throughput = *min_t_it;
			r.best_throughput = *max_t_it;
			r.throughput_amplitude = r.best_throughput - r.worst_throughput;

			auto [min_time_it, max_time_it] = std::minmax_element(times.begin(), times.end());
			r.smallest_time = *min_time_it;
			r.biggest_time = *max_time_it;
			r.time_amplitude = r.biggest_time - r.smallest_time;

			auto get_iqr = [](auto& vec) {
				auto q1_it = vec.begin() + vec.size() / 4;
				auto q3_it = vec.begin() + 3 * vec.size() / 4;
				std::nth_element(vec.begin(), q1_it, vec.end());
				std::nth_element(q1_it, q3_it, vec.end());
				return *q3_it - *q1_it;
			};

			r.throughput_iqr = get_iqr(throughputs);
			r.time_iqr = get_iqr(times);

			double avg_cycles = std::accumulate(total_cycles.begin(), total_cycles.end(), 0.0) / total_cycles.size();
        	r.average_cpb = avg_cycles / static_cast<double>(bytes_per_run);

			return r;
		}
	};

	static size_t constexpr WARMUP_ITERATIONS = 30;
	static size_t constexpr WARMUP_DATA_SIZE = 1024 * 1024 * 10; // 5 MB

	static size_t constexpr TEST_ITERATIONS = 30;
	static size_t constexpr TEST_DATA_SIZE = 1024 * 1024 * 8; // 8 MB

	static size_t constexpr KB = 1024;

    void run_tests(const size_t data_size, const size_t rounds, PerformanceMetric& metrics, Serpent& test, bool verbose=false) {
		std::vector<uint8_t> plaintext(data_size, 0xAA);
		std::vector<uint8_t> output(data_size, 0);
		
		for (size_t i = 0; i < rounds; i++) {
			uint64_t start_cycles = read_cycles();

			auto start = std::chrono::steady_clock::now();
			test.process_data(plaintext, output);
			asm volatile("" : : "g"(plaintext.data()) : "memory"); 

			uint64_t end_cycles = read_cycles();
        	auto end = std::chrono::steady_clock::now();

			std::chrono::duration<double> duration = end - start;
			uint64_t total_cycles = end_cycles - start_cycles;
			double throughput_mbps = (data_size / (1024.0 * 1024.0)) / duration.count();
			double cpb = static_cast<double>(total_cycles) / data_size;

			if (verbose) {
				std::cout << std::endl << "Test " << (i + 1) << std::endl;
				std::cout << "Took: " << duration.count() << "s" << std::endl;
				std::cout << "Throughput: " << throughput_mbps << "MB/s" << std::endl;
				std::cout << "CPB: " << cpb << std::endl;
				std::cout << "========================================" << std::endl;
			}

			metrics.pushMetrics(duration, throughput_mbps, total_cycles);
		}
	}

    void test_cipher(bool verbose=false) {
		std::array<uint32_t, 8> test_key { 
			0x2ee10b8e, 0x053ef2fc, 0xb05bf4ab, 0x9014e2ae, 
			0x5C63A1B7, 0x0F48E9D2, 0x31B6C5A8, 0x94D27F02
		};

		Serpent_IV iv;
		iv.vector_form = v128u { 100, 370, 89, 0 };

        Serpent test(test_key, iv);
		
        {
            std::vector<uint8_t> warm_in(WARMUP_DATA_SIZE, 0xAA);
			std::vector<uint8_t> warm_out(WARMUP_DATA_SIZE, 0);

            for (size_t i = 0; i < WARMUP_ITERATIONS; i++) {
                test.process_data(warm_in, warm_out);
            }

            std::cout << "Warmup done" << std::endl;
        }

		PerformanceMetric metric(TEST_ITERATIONS, TEST_DATA_SIZE);

		run_tests(TEST_DATA_SIZE, TEST_ITERATIONS, metric, test, verbose);

		PerformanceResults r = metric.finish();

		r.print("Serpent performance tests");
	}
}
