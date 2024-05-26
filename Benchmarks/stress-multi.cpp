#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <thread>
#include <random>
#include <atomic>

#include "bench.hpp"

std::atomic<float> target = BENCH_STRESS_UPPER;

void stress_multi_runner(std::ostringstream& os, std::vector<void*>& allocs,
                         unsigned dur, unsigned tid)
{
        std::random_device rd;
        std::mt19937 rng{rd()};  
        std::uniform_int_distribution<> dis(0, nb_stat_max_order());

        auto start = std::chrono::high_resolution_clock::now();

        for(;;) {
                auto now = std::chrono::high_resolution_clock::now();
                auto elapsed = std::chrono::duration_cast
                        <std::chrono::seconds>(now- start).count();
                if (dur < elapsed) {
                        break;
                }

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
                        os << "alloc (" << us << "us, " << usage << "%), ";

                        allocs.push_back(ptr);
                } else if (allocs.size()) {
                        void *addr = allocs.back();

                        auto start = std::chrono::high_resolution_clock::now();
                        BENCH_FREE(addr);
                        auto durr = std::chrono::high_resolution_clock::now() - start;

                        auto us = std::chrono::duration_cast
                                <std::chrono::microseconds>(durr).count();
                        os << "free (" << us << "us, " << usage << "%), ";

                        allocs.pop_back();
                }


                std::this_thread::sleep_for(std::chrono::milliseconds(
                        BENCH_STRESS_PERIOD));
        }
}

int stress_multi(std::ofstream& ofs, unsigned dur, unsigned tc)
{
        ofs << FUNC_NAME << "\n";

        bench_alloc_init();

        std::vector<std::vector<void*>> allocs(tc);
        std::vector<std::ostringstream> streams(tc);
        std::vector<std::thread> threads = {};

        /* Output streams for all threads */
        for (unsigned i = 0; i < tc; i++) {
                streams.push_back(std::ostringstream());
        }

        /* Create [and start] the threads */
        for (unsigned i = 0; i < tc; i++) {
                threads.push_back(
                        std::thread(stress_multi_runner,
                                std::ref(streams[i]),
                                std::ref(allocs[i]), dur, i));
        }

        /* Monitor memory usage */
        auto start = std::chrono::high_resolution_clock::now();
        std::cout << FUNC_NAME << ": main: start" << std::endl;

        for(;;) {
                auto now = std::chrono::high_resolution_clock::now();
                auto elapsed = std::chrono::duration_cast
                        <std::chrono::seconds>(now- start).count();
                if (dur < elapsed) {
                        break;
                }

                std::cout << FUNC_NAME << ": main: elapsed: "
                          << elapsed << " / " << dur << "s\r" << std::flush;

                /* Memory usage check */
                float usage = (float) nb_stat_used_memory() /
                        nb_stat_total_memory() * 100;
                if (BENCH_STRESS_UPPER <= usage) {
                        target = BENCH_STRESS_LOWER;
                } else if (usage < BENCH_STRESS_LOWER) {
                        target = BENCH_STRESS_UPPER;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(
                        BENCH_STRESS_PERIOD));
        }
        std::cout << std::endl;

        /* Wait for them */
        for (auto& thread : threads) { thread.join(); }

        for (unsigned i = 0; i < tc; i++) {
                ofs << "thread #" << i << ": " << streams[i].str()
                    << "\n";
        }
        std::cout << FUNC_NAME << ": main: done" << std::endl;

        return 0;
}
