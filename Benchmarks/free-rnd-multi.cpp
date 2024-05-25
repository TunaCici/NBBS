#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <random>
#include <barrier>

#include "bench.hpp"

void free_rnd_multi_runner(std::ostringstream& os, std::barrier<>& sync_point,
                           int tid)
{
        std::random_device rd;
        std::mt19937 rng{rd()};  
        std::uniform_int_distribution<> dis(0, nb_stat_max_order());
        
        std::vector<void*> allocs = {};
        for (int i = 0; i < BENCH_BATCH_SIZE; i++) {
                auto bench_alloc_size = nb_stat_block_size(
                        dis(rng));

                void *ptr = (void*) BENCH_MALLOC(bench_alloc_size);
                if (!ptr) {
                        std::cerr << "Error: free_rnd_multi: BENCH_MALLOC failed" << std::endl;
                        std::exit(1);
                }

                allocs.push_back(ptr);
        }

        /* Wait for other threads to complete their allocs */
        sync_point.arrive_and_wait();

        for (auto j = 0; j < BENCH_BATCH_SIZE; j++) {
                void *addr = allocs.back();

                auto start = std::chrono::high_resolution_clock::now();
                BENCH_FREE(addr);
                auto durr = std::chrono::high_resolution_clock::now() - start;

                auto us = std::chrono::duration_cast<std::chrono::microseconds>(durr).count();
                float mem_usage = (float) nb_stat_used_memory() /
                        nb_stat_total_memory() * 100;
                
                os << "(" << us << "us, " << mem_usage << "%), ";

                allocs.pop_back();
        }
}

int free_rnd_multi(std::ofstream& of, unsigned long iterations, unsigned long thread_count)
{
        of << "free_rnd_multi\n";

        /* Force the OS to map/alloc all the arena - preheating */
        void *arena = (void*) NB_MALLOC(ARENA_SIZE);
        memset(arena, 0x0, ARENA_SIZE);

        /* Initialize */
        if (nb_init((uint64_t) arena, ARENA_SIZE) != 0) {
                std::cerr << "Error: free_rnd_multi: initialization failed" << std::endl;
                return 1;
        }

        std::vector<std::ostringstream> streams = {};
        std::vector<std::thread> threads = {};
        std::barrier sync_point(thread_count);

        for (auto i = 0; i < iterations; i++) {
                for (auto i = 0; i < thread_count; i++) {
                        streams.push_back(std::ostringstream());
                }

                for (auto j = 0; j < thread_count; j++) {
                        threads.push_back(
                                std::thread(free_rnd_multi_runner,
                                        std::ref(streams[j]),
                                        std::ref(sync_point), j)
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
