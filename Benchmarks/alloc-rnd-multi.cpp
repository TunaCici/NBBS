#include <iostream>
#include <sstream>
#include <fstream>
#include <thread>
#include <chrono>
#include <random>

#include "bench.hpp"

static inline int do_work(std::ostringstream& os)
{
        static std::random_device rd;
        static std::mt19937 rng{rd()};  
        static std::uniform_int_distribution<> dis(0, nb_stat_max_order());

        for (unsigned j = 0; j < BENCH_BATCH_SIZE; j++) {
                auto alloc_size = nb_stat_block_size(
                        dis(rng));

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
                
                os << std::fixed << std::setprecision(6)
                   << "alloc (" << us << "us, " << usage << "%), ";
        }

        return 0;
}

static void alloc_rnd_multi_runner(std::ostringstream& os, unsigned dur)
{
        auto start = std::chrono::high_resolution_clock::now();

        /* Allocate until the memory runs out */
        for (;;) {
                auto now = std::chrono::high_resolution_clock::now();
                auto elapsed = std::chrono::duration_cast
                        <std::chrono::seconds>(now - start).count();
                if (dur < elapsed) {
                        break;
                }

                if (do_work(os)) {
                        break;
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(
                        BENCH_STRESS_PERIOD));
        }   
}

int alloc_rnd_multi(std::ofstream& ofs, unsigned dur, unsigned tc)
{
        ofs << FUNC_NAME << "\n";

        bench_alloc_init();

        std::vector<std::ostringstream> streams(tc);
        std::vector<std::thread> threads = {};

        for (unsigned j = 0; j < tc; j++) {
                streams.push_back(std::ostringstream());
        }

        std::cout << FUNC_NAME << ": main: start" << std::endl;

        for (unsigned j = 0; j < tc; j++) {
                threads.push_back(
                        std::thread(alloc_rnd_multi_runner,
                                std::ref(streams[j]), dur));
        }

        /* Wait for them */
        for (auto& thread : threads) { thread.join(); }

        for (unsigned j = 0; j < tc; j++) {
                ofs << std::fixed << std::setprecision(6)
                    << "thread" << j << ": " << streams[j].str() << "\n";
        }
        std::cout << FUNC_NAME << ": main: done" << std::endl;

        return 0;
}
