#include <iostream>
#include <sstream>
#include <fstream>
#include <thread>
#include <chrono>
#include <random>
#include <barrier>
#include <iomanip>

#include "bench.hpp"

static inline int do_work(std::ostringstream& os, std::vector<void*>& allocs)
{
        auto batch_size = std::min((size_t) BENCH_BATCH_SIZE, allocs.size());

        for (unsigned j = 0; j < batch_size; j++) {
                void *addr = allocs.back();

                auto start = std::chrono::high_resolution_clock::now();
                BENCH_FREE(addr);
                auto durr = std::chrono::high_resolution_clock::now() - start;

                auto us = std::chrono::duration_cast
                        <std::chrono::microseconds>(durr).count();
                float usage = (float) nb_stat_used_memory() /
                        nb_stat_total_memory();
                
                os << std::fixed << std::setprecision(6)
                   << "free (" << us << "us, " << usage << "%), ";

                allocs.pop_back();
        }

        return 0;
}

static void free_rnd_multi_runner(std::ostringstream& os,
                                  std::barrier<>& sync_point)
{
        std::random_device rd;
        std::mt19937 rng{rd()};  
        std::uniform_int_distribution<> dis(0, nb_stat_max_order());
        
        std::vector<void*> allocs = {};

        /* Allocate memory until failure */
        for (;;) {
                auto alloc_size = nb_stat_block_size(
                        dis(rng));

                void *ptr = (void*) BENCH_MALLOC(alloc_size);
                if (!ptr) {
                        break;
                }

                allocs.push_back(ptr);
        }

        /* Wait for other threads to complete their allocs */
        sync_point.arrive_and_wait();

        /* Free them all */
        while (allocs.size()) {
                if (do_work(os, allocs)) {
                        break;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(
                        BENCH_STRESS_PERIOD));
        }
        
}

int free_rnd_multi(std::ofstream& ofs, unsigned tc)
{
        ofs << FUNC_NAME << "\n";

        bench_alloc_init();

        std::vector<std::ostringstream> streams(tc);
        std::vector<std::thread> threads = {};
        std::barrier sync_point(tc);

        for (unsigned j = 0; j < tc; j++) {
                streams.push_back(std::ostringstream());
        }

        std::cout << FUNC_NAME << ": main: start" << std::endl;
 
        for (unsigned j = 0; j < tc; j++) {
                threads.push_back(
                        std::thread(free_rnd_multi_runner,
                                std::ref(streams[j]),
                                std::ref(sync_point)));
        }

        /* Wait for them */
        for (auto& thread : threads) { thread.join(); }

        for (unsigned j = 0; j < tc; j++) {
                ofs << "thread" << j << ": " << streams[j].str() << "\n";
        }
                
        std::cout << FUNC_NAME << ": main: done" << std::endl;

        return 0;
}
