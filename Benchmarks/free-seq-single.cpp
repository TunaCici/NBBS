#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
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
                auto dur = std::chrono::high_resolution_clock::now() - start;

                auto us = std::chrono::duration_cast
                        <std::chrono::microseconds>(dur).count();
                float usage = (float) nb_stat_used_memory() /
                        nb_stat_total_memory();
                
                ofs << std::fixed << std::setprecision(6)
                    << "free (" << us << "us, " << usage << "%), ";
                
                allocs.pop_back();
        }

        return 0;
}

int free_seq_single(std::ofstream& ofs)
{
        ofs << FUNC_NAME << "\n";

        bench_alloc_init();

        std::vector<void*> allocs = {};

        auto total_allocs = nb_stat_total_blocks(nb_stat_max_order());
        auto alloc_size = nb_stat_max_size();

        /* Allocate all the memory */
        for (unsigned i = 0; i < total_allocs; i++) {
                void *ptr = (void*) BENCH_MALLOC(alloc_size);
                if (!ptr) {
                        std::cerr << FUNC_NAME
                                  << ": BENCH_MALLOC fail" << std::endl;
                        std::exit(1);
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
