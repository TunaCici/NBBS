#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <random>

#include "bench.hpp"

int stress_single(std::ofstream& of, unsigned long iterations)
{
        of << "stress_single\n";

        /* Force the OS to map/alloc all the arena - preheating */
        void *arena = (void*) NB_MALLOC(ARENA_SIZE);
        memset(arena, 0x0, ARENA_SIZE);

        /* Initialize */
        if (nb_init((uint64_t) arena, ARENA_SIZE) != 0) {
                std::cerr << "Error: stress_single: initialization failed" << std::endl;
                return 1;
        }

        std::random_device rd;
        std::mt19937 rng{rd()};  
        std::uniform_int_distribution<> dis(0, nb_stat_max_order());
        std::vector<void*> allocs = {};
        auto target = BENCH_STRESS_UPPER;

        for (auto i = 0; i < iterations; i++) {
                of << "iter: " << i  << ": ";

                auto start = std::chrono::high_resolution_clock::now();

                for(;;) {
                        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                                std::chrono::high_resolution_clock::now() - start).count();
                        if (BENCH_STRESS_RUN < elapsed) {
                                break;
                        }

                        float mem_usage = ((float) nb_stat_used_memory() /
                                nb_stat_total_memory()) * 100;

                        /* Alloc or free */
                        if (mem_usage < target) {
                                auto bench_alloc_size = nb_stat_block_size(
                                        dis(rng));

                                auto start = std::chrono::high_resolution_clock::now();
                                void *ptr = (void*) BENCH_MALLOC(bench_alloc_size);
                                auto durr = std::chrono::high_resolution_clock::now() - start;

                                auto us = std::chrono::duration_cast<std::chrono::microseconds>(durr).count();
                                of << "alloc(" << us << "us, " << mem_usage << "%), ";

                                allocs.push_back(ptr);
                        } else if (allocs.size()) {
                                void *addr = allocs.back();

                                auto start = std::chrono::high_resolution_clock::now();
                                BENCH_FREE(addr);
                                auto durr = std::chrono::high_resolution_clock::now() - start;

                                auto us = std::chrono::duration_cast<std::chrono::microseconds>(durr).count();
                                of << "free(" << us << "us, " << mem_usage << "%), ";

                                allocs.pop_back();
                        }

                        /* Border check */
                        mem_usage = ((float) nb_stat_used_memory() /
                                nb_stat_total_memory()) * 100;
                        if (BENCH_STRESS_UPPER <= mem_usage) {
                                target = BENCH_STRESS_LOWER;
                        } else if (mem_usage < BENCH_STRESS_LOWER) {
                                target = BENCH_STRESS_UPPER;
                        }

                        std::this_thread::sleep_for(std::chrono::milliseconds(
                                BENCH_STRESS_PERIOD
                        ));
                }

                of << "\n";
        }

        return 0;
}
