

#include <Geometry/Interval.hpp>
#include <LinAl/LinearAlgebra.hpp>
#include <gtest/gtest.h>

using namespace Geometry;
using namespace LinAl;

TEST(Interval, Disjunct_LHS)
{
    Interval a{0.0, 1.0};
    Interval b{-3.0, -2.0};
    Interval<double_t> intersection{};
    uint32_t res = a.intersection(b, intersection);
    EXPECT_EQ(res, 0);
}

TEST(Interval, Disjunct_RHS)
{
    Interval a{0.0, 1.0};
    Interval b{2.0, 3.0};
    Interval<double_t> intersection{};
    uint32_t res = a.intersection(b, intersection);
    EXPECT_EQ(res, 0);
}

TEST(Interval, Touching_LHS)
{
    Interval a{0.0, 1.0};
    Interval b{-1.0, 0.0};
    Interval<double_t> intersection{};
    uint32_t res = a.intersection(b, intersection);
    EXPECT_EQ(res, 1);
    EXPECT_EQ(intersection.getStart(), 0.0);
}

TEST(Interval, Touching_RHS)
{
    Interval a{0.0, 1.0};
    Interval b{1.0, 2.0};
    Interval<double_t> intersection{};
    uint32_t res = a.intersection(b, intersection);
    EXPECT_EQ(res, 1);
    EXPECT_EQ(intersection.getStart(), 1.0);
}

TEST(Interval, Intersecting)
{
    Interval a{0.0, 1.0};
    Interval b{0.5, 2.0};
    Interval<double_t> intersection{};
    uint32_t res = a.intersection(b, intersection);
    EXPECT_EQ(res, 2);
    EXPECT_EQ(intersection, (Interval{0.5, 1.0}));
}

