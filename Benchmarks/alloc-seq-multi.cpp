#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>

#include "bench.hpp"

void alloc_seq_multi_runner(std::ostringstream& os, int tid)
{
        auto alloc_size = nb_stat_block_size(0);

        std::cout << FUNC_NAME << ": thread" << tid << ": start\n" << std::flush;

        for (unsigned j = 0; j < BENCH_BATCH_SIZE; j++) {
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
                float usage = (float) nb_stat_used_memory() /
                        nb_stat_total_memory() * 100;
                
                os << "alloc (" << us << "us, " << usage << "%), ";
        }
        std::cout << FUNC_NAME << ": thread" << tid << ": done\n" << std::flush;
}

int alloc_seq_multi(std::ofstream& ofs, unsigned ic, unsigned tc)
{
        ofs << FUNC_NAME << "\n";

        bench_alloc_init();

        std::vector<std::ostringstream> streams(tc);
        std::vector<std::thread> threads = {};

        std::cout << FUNC_NAME << "main: start" << std::endl;

        for (unsigned i = 0; i < ic; i++) {
                std::cout << FUNC_NAME << "main: iter: "
                          << i << " / " << ic << "\r";
                
                for (unsigned j = 0; j < tc; j++) {
                        streams.push_back(std::ostringstream());
                }

                for (unsigned j = 0; j < tc; j++) {
                        threads.push_back(
                                std::thread(alloc_seq_multi_runner,
                                        std::ref(streams[j]), j)
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
        std::cout << FUNC_NAME << "main: done" << std::endl;

        return 0;
}
