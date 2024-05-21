#include "gtest/gtest.h"

#include <algorithm>

#include "nbbs-defs.h"

extern "C" {
        #include "nbbs.h"
}

TEST(NBBS, statistics)
{
        uint8_t *playground = static_cast<uint8_t*>(
                std::aligned_alloc(nbbs_max_size, nbbs_total_memory)
        );
        std::fill_n(playground, nbbs_total_memory / sizeof(uint8_t), 0);

        EXPECT_EQ(0, nb_init((uint64_t) playground, nbbs_total_memory));

        /* nb_stat_min_size() */
        EXPECT_EQ(nbbs_min_size, nb_stat_min_size());

        /* nb_stat_max_order() */
        EXPECT_EQ(nbbs_max_order, nb_stat_max_order());

        /* nb_stat_depth() */
        EXPECT_EQ(nbbs_depth, nb_stat_depth());

        /* nb_stat_tree_size() */
        EXPECT_EQ(std::exp2(nbbs_depth + 1), nb_stat_tree_size());

        /* nb_stat_index_size() */
        EXPECT_EQ((nbbs_total_memory / nbbs_min_size) * sizeof(uint32_t),
                nb_stat_index_size()
        );

        /* nb_stat_base_level() */
        EXPECT_EQ(nbbs_depth - nbbs_max_order, nb_stat_base_level());

        /* nb_stat_max_size() */
        EXPECT_EQ(std::exp2(nbbs_max_order) * nbbs_min_size, 
                nb_stat_max_size()
        );

        /* nb_stat_total_memory() */
        EXPECT_EQ(nbbs_total_memory, nb_stat_total_memory());

        /* nb_stat_block_size() */
        for (uint32_t i = 0; i < nbbs_max_order; i++) {
                EXPECT_EQ(std::exp2(i) * nbbs_min_size, nb_stat_block_size(i));
        }

        /* nb_stat_total_blocks() */
        for (uint32_t i = 0; i < nbbs_max_order; i++) {
                EXPECT_EQ((nbbs_total_memory / (std::exp2(i) * nbbs_min_size)),
                        nb_stat_total_blocks(i)
                );
        }

        std::vector<void*> allocs = {};

        /* nb_stat_used_blocks() */
        for (uint32_t i = 0; i <= nbbs_max_order; i++) {
                ASSERT_EQ(0, nb_stat_used_blocks(i));
        }

        /* Alloc on all orders 3x times */
        for (uint32_t i = 0; i <= nbbs_max_order; i++) {
                void *alloc = (void*) nb_alloc(nb_stat_block_size(i));
                EXPECT_NE((void*) 0, alloc);
                allocs.push_back(alloc);

                alloc = (void*) nb_alloc(nb_stat_block_size(i));
                EXPECT_NE((void*) 0, alloc);
                allocs.push_back(alloc);

                alloc = (void*) nb_alloc(nb_stat_block_size(i));
                EXPECT_NE((void*) 0, alloc);
                allocs.push_back(alloc);
        }

        /* nb_stat_used_blocks() */
        for (uint32_t i = 0; i <= nbbs_max_order; i++) {
                ASSERT_EQ(3, nb_stat_used_blocks(i));
        }

        /* Release all allocated blocks */
        for (auto alloc : allocs) {
                nb_free(alloc);
        }

        EXPECT_EQ((3 * (nbbs_max_order + 1)), nb_stat_release_count());
}
