#include "gtest/gtest.h"

#include "nbbs-defs.h"

extern "C" {
        #include "nbbs.h"
}

TEST(NBBS, helpers)
{
        uint8_t status = 0;

        /* Left occupy */
        status = nb_mark(status, 0);
        EXPECT_EQ(OCC_LEFT, status);
        status = nb_unmark(status, 0);
        EXPECT_EQ(0, status);

        /* Right occupy */
        status = nb_mark(status, 1);
        EXPECT_EQ(OCC_RIGHT, status);
        status = nb_unmark(status, 1);
        EXPECT_EQ(0, status);

        /* Full occupied */
        status = BUSY;
        status = nb_unmark(status, 0);
        EXPECT_EQ(OCC | OCC_RIGHT, status);
        status = nb_unmark(status, 1);
        EXPECT_EQ(OCC, status);
        status = 0;

        /* Left coalescing */
        status = nb_set_coal(status, 0);
        EXPECT_EQ(COAL_LEFT, status);
        EXPECT_NE(0, nb_is_coal(status, 0));
        EXPECT_EQ(0, nb_clean_coal(status, 0));
        status = 0;

        /* Right coalescing */
        status = nb_set_coal(status, 1);
        EXPECT_EQ(COAL_RIGHT, status);
        EXPECT_NE(0, nb_is_coal(status, 1));
        EXPECT_EQ(0, nb_clean_coal(status, 1));
        status = 0;

        /* Buddy occupied? */
        status = nb_mark(status, 0);
        EXPECT_NE(0, nb_is_occ_buddy(status, 1));
        status = nb_mark(status, 1);
        EXPECT_NE(0, nb_is_occ_buddy(status, 0));

        /* Buddy coalescing? */
        status = nb_set_coal(status, 0);
        EXPECT_NE(0, nb_is_coal_buddy(status, 1));
        status = nb_set_coal(status, 1);
        EXPECT_NE(0, nb_is_coal_buddy(status, 0));

        /* Is node free? */
        status = 0;
        EXPECT_NE(0, nb_is_free(status));
        status |= BUSY;
        EXPECT_EQ(0, nb_is_free(status));

        /* Node level */
        EXPECT_EQ(0, nb_level(1));
        EXPECT_EQ(1, nb_level(2));
        EXPECT_EQ(10, nb_level(1024));
        EXPECT_EQ(31, nb_level(std::numeric_limits<uint32_t>::max()));
}
