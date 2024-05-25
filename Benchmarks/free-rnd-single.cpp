#include <iostream>
#include <vector>
#include <chrono>
#include <random>

#include "bench.hpp"

int free_rnd_single(std::ofstream& of, unsigned long iterations)
{
        of << "free_rnd_single\n";

        /* Force the OS to map/alloc all the arena - preheating */
        void *arena = (void*) NB_MALLOC(ARENA_SIZE);
        memset(arena, 0x0, ARENA_SIZE);

        /* Initialize */
        if (nb_init((uint64_t) arena, ARENA_SIZE) != 0) {
                std::cerr << "Error: free_rnd_single: initialization failed" << std::endl;
                return 1;
        }

        /* Allocate all memory */
        std::random_device rd;
        std::mt19937 rng{rd()};  
        std::uniform_int_distribution<> dis(0, nb_stat_max_order());

        std::vector<void*> allocs = {};
        auto total_allocs = iterations * BENCH_BATCH_SIZE;
        for (int i = 0; i < total_allocs; i++) {
                auto bench_alloc_size = nb_stat_block_size(
                        dis(rng));

                void *ptr = (void*) BENCH_MALLOC(bench_alloc_size);
                if (!ptr) {
                        std::cerr << "Error: free_rnd_single: BENCH_MALLOC failed" << std::endl;
                        return 1;
                }

                allocs.push_back(ptr);
        }

        for (auto i = 0; i < iterations; i++) {
                of << "iter: " << i  << ": ";

                for (auto j = 0; j < BENCH_BATCH_SIZE; j++) {
                        void *addr = allocs.back();

                        auto start = std::chrono::high_resolution_clock::now();
                        BENCH_FREE(addr);
                        auto durr = std::chrono::high_resolution_clock::now() - start;

                        auto us = std::chrono::duration_cast<std::chrono::microseconds>(durr).count();
                        float mem_usage = (float) nb_stat_used_memory() /
                                nb_stat_total_memory() * 100;
                        
                        of << "(" << us << "us, " << mem_usage << "%), ";

                        allocs.pop_back();
                }
                of << "\n";
        }

        return 0;
}
