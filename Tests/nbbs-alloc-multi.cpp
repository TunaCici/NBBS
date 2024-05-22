#include "gtest/gtest.h"

#include <iostream>
#include <thread>

#include "nbbs-defs.h"

extern "C" {
        #include "nbbs.h"
}

/* Try to allocate min_size blocks until all is full */
void thread_alloc(int id) {
        for(;;) {
                int *alloc = (int*) nb_alloc(nbbs_min_size);
                if (!alloc) {
                        return;
                }

                std::fill_n(alloc, nbbs_min_size / sizeof(int), id);
                
                /* No one but the owner should've accessed the alloc */
                for (auto i = 0; i < nbbs_min_size / sizeof(int); i++) {
                        ASSERT_EQ(alloc[i], id);
                }
        }
}

TEST(NBBS, alloc_multi)
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
                        std::thread(thread_alloc, i + 1)
                );
        }
        
        /* Wait for them */
        for (std::thread& thread : threads) {
                thread.join();
        }

        /* Check for corruption */
        EXPECT_EQ(0, nb_stat_release_count());
        EXPECT_EQ(nbbs_total_memory, nb_stat_used_memory());
        EXPECT_EQ(nb_stat_total_blocks(0), nb_stat_used_blocks(0));
}
