#include <Geometry/Intersection/IntersectionInterval.hpp>
#include <Geometry/Interval.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

TEST(Interval, Disjunct_LHS)
{
    Interval a{0.0, 1.0};
    Interval b{-3.0, -2.0};
    Interval<double> result{};
    uint32_t res = Geometry::intersection(a, b, result);
    EXPECT_EQ(res, 0);
}

TEST(Interval, Disjunct_RHS)
{
    Interval a{0.0, 1.0};
    Interval b{2.0, 3.0};
    Interval<double> result{};
    uint32_t res = Geometry::intersection(a, b, result);
    EXPECT_EQ(res, 0);
}

TEST(Interval, Touching_LHS)
{
    Interval a{0.0, 1.0};
    Interval b{-1.0, 0.0};
    Interval<double> result{};
    uint32_t res = Geometry::intersection(a, b, result);
    EXPECT_EQ(res, 1);
    EXPECT_EQ(result.getStart(), 0.0);
}

TEST(Interval, Touching_RHS)
{
    Interval a{0.0, 1.0};
    Interval b{1.0, 2.0};
    Interval<double> result{};
    uint32_t res = Geometry::intersection(a, b, result);
    EXPECT_EQ(res, 1);
    EXPECT_EQ(result.getStart(), 1.0);
}

TEST(Interval, Intersecting)
{
    Interval a{0.0, 1.0};
    Interval b{0.5, 2.0};
    Interval<double> result{};
    uint32_t res = Geometry::intersection(a, b, result);
    EXPECT_EQ(res, 2);
    EXPECT_EQ(result, (Interval{0.5, 1.0}));
}
