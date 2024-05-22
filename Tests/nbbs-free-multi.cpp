#include "gtest/gtest.h"

#include <iostream>
#include <thread>
#include <random>

#include "nbbs-defs.h"

extern "C" {
        #include "nbbs.h"
}

/* Random 'order' generator */
thread_local std::mt19937 mt(std::random_device{}());
thread_local std::uniform_int_distribution<> dist(0, nbbs_max_order);

/* Try to allocate and free until interation runs out */
void thread_free(int id) {
        for(auto i = 0; i < nbbs_iter_count; i++) {
                int *alloc = 0;
                uint64_t alloc_size = nb_stat_block_size(dist(mt));

                alloc = (int*) nb_alloc(alloc_size);
                if (!alloc) {
                        continue;
                }

                std::fill_n(alloc, alloc_size / sizeof(int), id);
                
                /* No one but the owner should've accessed the alloc */
                for (auto i = 0; i < alloc_size / sizeof(int); i++) {
                        ASSERT_EQ(alloc[i], id);
                }

                nb_free(alloc);
        }
}

TEST(NBBS, free_multi)
{
        uint8_t *playground = static_cast<uint8_t*>(
                std::aligned_alloc(nbbs_max_size, nbbs_total_memory)
        );
        std::fill_n(playground, nbbs_total_memory / sizeof(uint8_t), 0);

        EXPECT_EQ(0, nb_init((uint64_t) playground, nbbs_total_memory));

        std::vector<std::thread> threads  = {};

        /* Spawn threads */
        for (int i = 0; i < nbbs_thread_count; i++) {
                threads.push_back(
                        std::thread(thread_free, i + 1)
                );
        }
        
        /* Wait for them */
        for (std::thread& thread : threads) {
                thread.join();
        }
}
