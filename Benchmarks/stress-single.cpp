#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>
#include <random>

#include "bench.hpp"

int stress_single(std::ofstream& ofs, unsigned dur)
{
        ofs << FUNC_NAME << "\n";

        bench_alloc_init();

        std::random_device rd;
        std::mt19937 rng{rd()};  
        std::uniform_int_distribution<> dis(0, nb_stat_max_order());
        
        std::vector<void*> allocs = {};
        auto target = BENCH_STRESS_UPPER;
        
        auto start = std::chrono::high_resolution_clock::now();
        std::cout << FUNC_NAME << "start" << std::endl;

        ofs << "thread0: ";
        for(;;) {
                auto now = std::chrono::high_resolution_clock::now();
                auto elapsed = std::chrono::duration_cast
                        <std::chrono::seconds>(now- start).count();
                if (dur < elapsed) {
                        break;
                }

                std::cout << FUNC_NAME << ": elapsed: "
                          << elapsed << " / " << dur << "\r" << std::flush;

                float usage = (float) nb_stat_used_memory() /
                        nb_stat_total_memory() * 100;

                /* Alloc or free */
                if (usage < target) {
                        auto alloc_size = nb_stat_block_size(
                                dis(rng));

                        auto start = std::chrono::high_resolution_clock::now();
                        void *ptr = (void*) BENCH_MALLOC(alloc_size);
                        auto durr = std::chrono::high_resolution_clock::now() - start;

                        auto us = std::chrono::duration_cast
                                <std::chrono::microseconds>(durr).count();
                        ofs << "alloc (" << us << "us, " << usage << "%), ";

                        allocs.push_back(ptr);
                } else if (allocs.size()) {
                        void *addr = allocs.back();

                        auto start = std::chrono::high_resolution_clock::now();
                        BENCH_FREE(addr);
                        auto dur = std::chrono::high_resolution_clock::now() - start;

                        auto us = std::chrono::duration_cast
                                <std::chrono::microseconds>(dur).count();
                        ofs << "free (" << us << "us, " << usage << "%), ";

                        allocs.pop_back();
                }

                /* Memory usage check */
                usage = ((float) nb_stat_used_memory() /
                        nb_stat_total_memory()) * 100;
                if (BENCH_STRESS_UPPER <= usage) {
                        target = BENCH_STRESS_LOWER;
                } else if (usage < BENCH_STRESS_LOWER) {
                        target = BENCH_STRESS_UPPER;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(
                        BENCH_STRESS_PERIOD));
        }
        std::cout << "\n" << FUNC_NAME << "done" << std::endl;

        return 0;
}
