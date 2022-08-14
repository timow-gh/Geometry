#include <Geometry/AABB.hpp>
#include <LinAl/LinearAlgebra.hpp>
#include <gtest/gtest.h>

using namespace Geometry;
using namespace LinAl;

class AABBTest2f : public ::testing::Test {
  protected:
    AABB2f aabb{{0, 0}, 1.0f};
};

TEST_F(AABBTest2f, aabb2f_constructor)
{
    LinAl::Vec2d expectedOrigin{0, 0};
    EXPECT_EQ(aabb.getOrigin(), expectedOrigin);

    for (auto value: aabb.getExtend())
        EXPECT_EQ(value, 1.0f);
}

class AABBTest3f : public ::testing::Test {
  protected:
    AABB3f aabb{{0, 0, 0}, 1.0f};
};

TEST_F(AABBTest3f, aabb3f_constructor)
{
    LinAl::Vec3d expectedOrigin{0, 0, 0};
    EXPECT_EQ(aabb.getOrigin(), expectedOrigin);

    for (auto value: aabb.getExtend())
        EXPECT_EQ(value, 1.0f);
}

class AABBTest2d : public ::testing::Test {
  protected:
    AABB2d aabb{{0, 0}, 1.0};
};

TEST_F(AABBTest2d, aabb2f_constructor)
{
    LinAl::Vec2d expectedOrigin{0, 0};
    EXPECT_EQ(aabb.getOrigin(), expectedOrigin);

    for (auto value: aabb.getExtend())
        EXPECT_EQ(value, 1.0);
}

class AABBTest3d : public ::testing::Test {
  protected:
    AABB3d aabb{{0, 0, 0}, 1.0f};
};

TEST_F(AABBTest3d, aabb3f_constructor)
{
    LinAl::Vec3d expectedOrigin{0, 0, 0};
    EXPECT_EQ(aabb.getOrigin(), expectedOrigin);

    for (auto value: aabb.getExtend())
        EXPECT_EQ(value, 1.0f);
}
