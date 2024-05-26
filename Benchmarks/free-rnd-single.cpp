#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <random>

#include "bench.hpp"

int free_rnd_single(std::ofstream& ofs, unsigned ic)
{
        ofs << FUNC_NAME << "\n";

        bench_alloc_init();

        std::random_device rd;
        std::mt19937 rng{rd()};  
        std::uniform_int_distribution<> dis(0, nb_stat_max_order());

        std::vector<void*> allocs = {};

        auto total_allocs = ic * BENCH_BATCH_SIZE;

        /* Allocate enough memory - random order */
        for (unsigned i = 0; i < total_allocs; i++) {
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

        std::cout << FUNC_NAME << ": start" << std::endl;

        /* Free them all */
        for (unsigned i = 0; i < ic; i++) {
                ofs << "iter: " << i << ": ";

                std::cout << FUNC_NAME << ": iter: "
                          << i << " / " << ic << "\r";

                for (unsigned j = 0; j < BENCH_BATCH_SIZE; j++) {
                        void *addr = allocs.back();

                        auto start = std::chrono::high_resolution_clock::now();
                        BENCH_FREE(addr);
                        auto durr = std::chrono::high_resolution_clock::now() - start;

                        auto us = std::chrono::duration_cast
                                <std::chrono::microseconds>(durr).count();
                        float usage = (float) nb_stat_used_memory() /
                                nb_stat_total_memory() * 100;
                        
                        ofs << "(" << us << "us, " << usage << "%), ";

                        allocs.pop_back();
                }
                ofs << "\n";
        }
        std::cout << FUNC_NAME << ": done" << std::endl;

        return 0;
}
