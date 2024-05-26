#include <iostream>
#include <fstream>
#include <chrono>
#include <random>

#include "bench.hpp"

static inline void do_iter(std::ofstream& ofs)
{
        std::random_device rd;
        std::mt19937 rng{rd()};  
        std::uniform_int_distribution<> dis(0, nb_stat_max_order());

        for (unsigned j = 0; j < BENCH_BATCH_SIZE; j++) {
                auto alloc_size = nb_stat_block_size(
                        dis(rng));

                auto start = std::chrono::high_resolution_clock::now();
                void *ptr = (void*) BENCH_MALLOC(alloc_size);
                auto durr = std::chrono::high_resolution_clock::now() - start;

                if (!ptr) {
                        std::cerr << FUNC_NAME
                                  << "BENCH_MALLOC fail" << std::endl;
                        std::exit(1);
                }

                auto us = std::chrono::duration_cast
                        <std::chrono::microseconds>(durr).count();
                float mem_usage = (float) nb_stat_used_memory() /
                        nb_stat_total_memory() * 100;
                
                ofs << "alloc (" << us << "us, " << mem_usage << "%), ";
        }
}

int alloc_rnd_single(std::ofstream& ofs, unsigned ic)
{
        ofs << FUNC_NAME << "\n";

        bench_alloc_init();

        std::cout << FUNC_NAME << ": start" << std::endl;

        for (unsigned i = 0; i < ic; i++) {
                ofs << "iter: " << i << ": ";

                std::cout << FUNC_NAME << ": iter: "
                          << i << " / " << ic << "\r";

                do_iter(ofs);
                
                ofs << "\n";
        }
        std::cout << FUNC_NAME << ": done" << std::endl;


        return 0;
}
