#include <Geometry/Intersect/IntersectInterval.hpp>
#include <Geometry/Interval.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

TEST(Interval, Disjunct_LHS)
{
    Interval lhs{0.0, 1.0};
    Interval rhs{-3.0, -2.0};
    Interval<double> result{};
    uint32_t res = Geometry::intersect(lhs, rhs, result);
    EXPECT_EQ(res, 0);
}

TEST(Interval, Disjunct_RHS)
{
    Interval lhs{0.0, 1.0};
    Interval rhs{2.0, 3.0};
    Interval<double> result{};
    uint32_t res = Geometry::intersect(lhs, rhs, result);
    EXPECT_EQ(res, 0);
}

TEST(Interval, Touching_LHS)
{
    Interval lhs{0.0, 1.0};
    Interval rhs{-1.0, 0.0};
    Interval<double> result{};
    uint32_t res = Geometry::intersect(lhs, rhs, result);
    EXPECT_EQ(res, 1);
    EXPECT_EQ(result.get_start(), 0.0);
}

TEST(Interval, Touching_RHS)
{
    Interval lhs{0.0, 1.0};
    Interval rhs{1.0, 2.0};
    Interval<double> result{};
    uint32_t res = Geometry::intersect(lhs, rhs, result);
    EXPECT_EQ(res, 1);
    EXPECT_EQ(result.get_start(), 1.0);
}

TEST(Interval, Intersecting)
{
    Interval lhs{0.0, 1.0};
    Interval rhs{0.5, 2.0};
    Interval<double> result{};
    uint32_t res = Geometry::intersect(lhs, rhs, result);
    EXPECT_EQ(res, 2);
    EXPECT_EQ(result, (Interval{0.5, 1.0}));
}
