#include <iostream>
#include <sstream>
#include <fstream>
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

        /* Allocate blocks - random order */
        for (unsigned i = 0; i < BENCH_BATCH_SIZE; i++) {
                auto alloc_size = nb_stat_block_size(
                        dis(rng));

                void *ptr = (void*) BENCH_MALLOC(alloc_size);
                if (!ptr) {
                        std::cerr << FUNC_NAME
                                  << "BENCH_MALLOC fail" << std::endl;
                        std::exit(1);
                }

                allocs.push_back(ptr);
        }

        /* Wait for other threads to complete their allocs */
        sync_point.arrive_and_wait();

        std::cout << FUNC_NAME << ": thread" << tid << ": start\n" << std::flush;

        /* Free them all */
        for (unsigned j = 0; j < BENCH_BATCH_SIZE; j++) {
                void *addr = allocs.back();

                auto start = std::chrono::high_resolution_clock::now();
                BENCH_FREE(addr);
                auto durr = std::chrono::high_resolution_clock::now() - start;

                auto us = std::chrono::duration_cast
                        <std::chrono::microseconds>(durr).count();
                float usage = (float) nb_stat_used_memory() /
                        nb_stat_total_memory() * 100;
                
                os << "free (" << us << "us, " << usage << "%), ";

                allocs.pop_back();
        }
        std::cout << FUNC_NAME << ": thread" << tid << ": done\n" << std::flush;

}

int free_rnd_multi(std::ofstream& ofs, unsigned ic, unsigned tc)
{
        ofs << FUNC_NAME << "\n";

        bench_alloc_init();

        std::vector<std::ostringstream> streams(tc);
        std::vector<std::thread> threads(tc);
        std::barrier sync_point(tc);

        std::cout << FUNC_NAME << ": main: start" << std::endl;

        for (unsigned i = 0; i < ic; i++) {
                std::cout << FUNC_NAME << "main: iter: "
                          << i << " / " << ic << "\r";
                
                for (unsigned j = 0; j < tc; j++) {
                        streams.push_back(std::ostringstream());
                }

                for (unsigned j = 0; j < tc; j++) {
                        threads.push_back(
                                std::thread(free_rnd_multi_runner,
                                        std::ref(streams[j]),
                                        std::ref(sync_point), j)
                        );
                }

                /* Wait for them */
                for (auto& thread : threads) { thread.join(); }

                for (unsigned j = 0; j < tc; j++) {
                        ofs << "iter: " << i << ": "
                            << "thread" << j << ": " << streams[j].str();
                        ofs << "\n";
                }

                streams.clear();
                threads.clear();
        }
        std::cout << FUNC_NAME << ": main: done" << std::endl;

        return 0;
}
