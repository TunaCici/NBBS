#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

#include "bench.hpp"

void alloc_seq_multi_runner(std::ostringstream& os, int tid)
{
        for (auto j = 0; j < BENCH_BATCH_SIZE; j++) {
                auto start = std::chrono::high_resolution_clock::now();
                void *ptr = (void*) BENCH_MALLOC(BENCH_ALLOC_SIZE);
                auto durr = std::chrono::high_resolution_clock::now() - start;

                if (!ptr) {
                        std::cerr << "Error: alloc_seq_multi: "
                                  << "thread #" << tid << " BENCH_MALLOC failed"
                                  << std::endl;
                        std::exit(1);
                }
                auto us = std::chrono::duration_cast<std::chrono::microseconds>(durr).count();
                float mem_usage = (float) nb_stat_used_memory() /
                        nb_stat_total_memory() * 100;
                
                os << "(" << us << "us, " << mem_usage << "%), ";
        }
}

int alloc_seq_multi(std::ofstream& of, unsigned long iterations, unsigned long thread_count)
{
        of << "alloc_seq_multi\n";

        /* Force the OS to map/alloc all the arena - preheating */
        void *arena = (void*) NB_MALLOC(ARENA_SIZE);
        memset(arena, 0x0, ARENA_SIZE);

        /* Initialize */
        if (nb_init((uint64_t) arena, ARENA_SIZE) != 0) {
                std::cerr << "Error: alloc_seq_multi: initialization failed" << std::endl;
                return 1;
        }

        std::vector<std::ostringstream> streams = {};
        std::vector<std::thread> threads = {};
        for (auto i = 0; i < iterations; i++) {
                for (auto i = 0; i < thread_count; i++) {
                        streams.push_back(std::ostringstream());
                }

                for (auto j = 0; j < thread_count; j++) {
                        threads.push_back(
                                std::thread(alloc_seq_multi_runner,
                                        std::ref(streams[j]), j)
                        );
                }

                /* Wait for them */
                for (auto& thread : threads) { thread.join(); }

                for (auto j = 0; j < thread_count; j++) {
                        of << "iter: " << i << ": "
                           << "thread #" << j << ": " << streams[j].str();
                        of << "\n";
                }

                streams.clear();
                threads.clear();
        }

        return 0;
}
