#include "gtest/gtest.h"

#include "nbbs-defs.h"

extern "C" {
        #include "nbbs.h"
}

TEST(NBBS, free_single)
{
        uint8_t *playground = static_cast<uint8_t*>(
                std::aligned_alloc(nbbs_max_size, nbbs_total_memory)
        );
        std::fill_n(playground, nbbs_total_memory / sizeof(uint8_t), 0);

        EXPECT_EQ(0, nb_init((uint64_t) playground, nbbs_total_memory));

        std::vector<void*> allocs1 = {};
        std::vector<void*> allocs2 = {};

        /* NULL does nothing - shouldn't crash */
        nb_free((void*) 0);

        /* Try on all orders */
        for (uint32_t i = 0; i <= nbbs_max_order; i++) {
                uint64_t block_count = nb_stat_total_blocks(i);

                /* Alloc all */
                for (uint64_t j = 0; j < block_count; j++) {
                        void *alloc = nb_alloc(nb_stat_block_size(i));
                
                        ASSERT_NE((void*) 0, alloc);
                        allocs1.push_back(alloc);
                }

                /* Verify all allocated */
                ASSERT_EQ(0, block_count - nb_stat_used_blocks(i));

                /* Free all */
                for (auto alloc : allocs1) {
                        nb_free(alloc);
                }
                
                /* Verify all freed */
                ASSERT_EQ(0, nb_stat_used_blocks(i));

                /* Alloc all again */
                for (uint64_t j = 0; j < block_count; j++) {
                        void *alloc = nb_alloc(nb_stat_block_size(i));
                
                        ASSERT_NE((void*) 0, alloc);
                        allocs2.push_back(alloc);
                }

                /* Same allocs should happen */
                ASSERT_EQ(allocs1, allocs2);

                /* Clean */
                for (auto alloc : allocs1) {
                        nb_free(alloc);
                }

                allocs1.clear();
                allocs2.clear();
        }

        /* Coalescing */
        // This alloc is going to prevent others to coalesce
        void *guard = nb_alloc(nb_stat_block_size(nbbs_min_size));
        ASSERT_NE((void*) 0, guard);

        /* Alloc rest of the blocks on max_order */
        // We decrement by 1 because the first one is occuiped by 'guard'
        for (uint32_t i = 0; i < nb_stat_total_blocks(nbbs_max_order)-1; i++) {
                ASSERT_NE((void*) 0, nb_alloc(nbbs_max_size));
        }

        /* Can't alloc due to 'guard' occupying higher blocks */
        ASSERT_EQ((void*) 0x0, nb_alloc(nbbs_max_size));

        /* Free and cause coalesing */
        nb_free(guard);

        /* Can alloc */
        ASSERT_NE((void*) 0, nb_alloc(nbbs_max_size));
}
