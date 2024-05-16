#include "gtest/gtest.h"

#include "nbbs-defs.h"

extern "C" {
        #include "nbbs.h"
}

TEST(NBBS, private)
{
        /* Leftmost index - nodes '2^N' */
        EXPECT_EQ(std::exp2(nbbs_depth),
                __nb_leftmost(1, nbbs_depth));
        EXPECT_EQ(std::exp2(nbbs_depth),
                __nb_leftmost(2, nbbs_depth));
        EXPECT_EQ(std::exp2(nbbs_depth),
                __nb_leftmost(8, nbbs_depth));
        EXPECT_EQ(std::exp2(nbbs_depth),
                __nb_leftmost(std::exp2(nbbs_depth), nbbs_depth));

        /* Leftmost index - nodes '2^N - 1' */
        EXPECT_EQ(std::exp2(nbbs_depth) + std::exp2(nbbs_depth - 1),
                __nb_leftmost(3, nbbs_depth));
        EXPECT_EQ(std::exp2(nbbs_depth + 1) - (std::exp2(nbbs_depth) / 8),
                __nb_leftmost(15, nbbs_depth));
        EXPECT_EQ(std::exp2(nbbs_depth + 1) - 2,
                __nb_leftmost(std::exp2(nbbs_depth) - 1, nbbs_depth));
}
