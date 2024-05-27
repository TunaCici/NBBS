#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>
#include <iomanip>

#include "bench.hpp"

static inline int do_work(std::ofstream& ofs)
{
        auto alloc_size = nb_stat_block_size(0);

        for (unsigned j = 0; j < BENCH_BATCH_SIZE; j++) {
                auto start = std::chrono::high_resolution_clock::now();
                void *ptr = (void*) BENCH_MALLOC(alloc_size);
                auto durr = std::chrono::high_resolution_clock::now() - start;

                if (!ptr) {
                        std::cerr << FUNC_NAME
                                  << ": BENCH_MALLOC fail."
                                  << "Exiting early..." << std::endl;
                        return 1;
                }

                auto us = std::chrono::duration_cast
                        <std::chrono::microseconds>(durr).count();
                float usage = (float) nb_stat_used_memory() /
                        nb_stat_total_memory();
                
                ofs << std::fixed << std::setprecision(6)
                    << "alloc (" << us << "us, " << usage << "%), ";
        }

        return 0;
}

int alloc_seq_single(std::ofstream& ofs, unsigned dur)
{
        ofs << FUNC_NAME << "\n";

        bench_alloc_init();

        std::cout << FUNC_NAME << ": start" << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        
        /* Allocate until either duration or the memory runs out */
        ofs << "thread0: ";
        for (;;) {
                auto now = std::chrono::high_resolution_clock::now();
                auto elapsed = std::chrono::duration_cast
                        <std::chrono::seconds>(now - start).count();
                if (dur < elapsed) {
                        break;
                }

                if(do_work(ofs)) {
                        break;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(
                        BENCH_STRESS_PERIOD));
        }
        std::cout << FUNC_NAME << ": done" << std::endl;

        return 0;
}
