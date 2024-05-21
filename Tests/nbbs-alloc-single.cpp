#include "gtest/gtest.h"

#include "nbbs-defs.h"

extern "C" {
        #include "nbbs.h"
}

TEST(NBBS, alloc_single)
{
        uint8_t *playground = static_cast<uint8_t*>(
                std::aligned_alloc(nbbs_max_size, nbbs_total_memory)
        );
        std::fill_n(playground, nbbs_total_memory / sizeof(uint8_t), 0);

        EXPECT_EQ(0, nb_init((uint64_t) playground, nbbs_total_memory));

        std::vector<int*> allocs = {};

        /* Boundry */
        ASSERT_NE((void*) 0, nb_alloc(0));
        ASSERT_NE((void*) 0, nb_alloc(nbbs_min_size));
        ASSERT_NE((void*) 0, nb_alloc(nbbs_max_size));
        EXPECT_EQ((void*) 0, nb_alloc(nbbs_max_size + 1));

        /* Allocate (on all orders) */
        for (uint32_t i = 0; i <= nbbs_max_order; i++) {
                int *alloc = 0;
                uint64_t alloc_size = nb_stat_block_size(i);

                alloc = (int*) nb_alloc(alloc_size);
                ASSERT_NE((void*) 0, alloc);

                allocs.push_back(alloc);
        }

        /* Write */
        for (size_t i = 0; i < allocs.size(); i++) {
                int elem_count = nb_stat_block_size(i) / sizeof(int);

                EXPECT_EQ(
                        allocs[i] + elem_count,
                        std::fill_n(allocs[i], elem_count, i + 1)
                );
        }

        /* Verify */
        for (size_t i = 0; i < allocs.size(); i++) {
                for (uint64_t j = 0 ; j < nb_stat_block_size(i); i += sizeof(int)) {
                        EXPECT_EQ((int) (i + 1), allocs[i][j]);
                }
        }

        /* Allocate rest of the blocks (order 0) */
        int free_blocks = nb_stat_total_blocks(0);

        for (uint32_t i = 0; i <= nbbs_max_order; i++) {
                int64_t used_blocks = nb_stat_used_blocks(i) * (1 << i);
                free_blocks -= used_blocks;
        }

        for (int i = 0; i < free_blocks; i++) {
                EXPECT_NE((void*) 0, nb_alloc(nbbs_min_size));
        }

        /* No memory left */
        EXPECT_EQ((void*) 0, nb_alloc(nbbs_min_size));
}
