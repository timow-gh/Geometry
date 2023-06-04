#include <gtest/gtest.h>
#include <Geometry/Triangle.hpp>

class testfixture_triangle : public ::testing::Test {
protected:
    Geometry::Triangle<double, 3> m_triangle;
};

TEST(Triangle, DefaultConstructor)
{
    Geometry::Triangle<double, 3> triangle;
    EXPECT_EQ(triangle.get_triangle_points().size(), 3);
}