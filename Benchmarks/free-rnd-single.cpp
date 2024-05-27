#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <random>
#include <thread>
#include <algorithm>
#include <iomanip>

#include "bench.hpp"

static inline int do_work(std::ofstream& ofs, std::vector<void*>& allocs)
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
                
                ofs << std::fixed << std::setprecision(6)
                    << "free (" << us << "us, " << usage << "%), ";
                
                allocs.pop_back();
        }

        return 0;
}

int free_rnd_single(std::ofstream& ofs)
{
        ofs << FUNC_NAME << "\n";

        bench_alloc_init();

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

        std::cout << FUNC_NAME << ": start" << std::endl;

        /* Free them all */
        ofs << "thread0: ";
        while (allocs.size()) {
                if(do_work(ofs, allocs)) {
                        break;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(
                        BENCH_STRESS_PERIOD));
        }

        std::cout << FUNC_NAME << ": done" << std::endl;

        return 0;
}
