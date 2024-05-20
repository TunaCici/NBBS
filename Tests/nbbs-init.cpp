#include "gtest/gtest.h"

#include <algorithm>

#include "nbbs-defs.h"

extern "C" {
        #include "nbbs.h"
}

TEST(NBBS, init)
{
        uint8_t *playground = static_cast<uint8_t*>(
                std::aligned_alloc(nbbs_max_size, nbbs_total_memory)
        );
        std::fill_n(playground, nbbs_total_memory / sizeof(uint8_t), 0);

        /* Empty base address OR size is NOT allowed */
        EXPECT_EQ(1, nb_init(0x0, 0));

        /* Less than min/alloc/page size is NOT allowed */
        EXPECT_EQ(1, nb_init((uint64_t) playground, nbbs_min_size - 1));

        /* Min/alloc/page size allowed */
        EXPECT_EQ(0, nb_init((uint64_t) playground, nbbs_min_size));

        /* Total memory */
        EXPECT_EQ(0, nb_init((uint64_t) playground, nbbs_total_memory));
}
