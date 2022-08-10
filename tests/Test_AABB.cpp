#include <Geometry/AABB.hpp>
#include <LinAl/LinearAlgebra.hpp>
#include <gtest/gtest.h>

using namespace Geometry;
using namespace LinAl;

class AABBTest3f : public ::testing::Test {
  protected:
    // Circle lies in the xy plane
    AABB<float_t, 3> aabb{LinAl::Vec3f{0, 0, 0}, 1.0f};
};

TEST_F(AABBTest3f, aabb_constructor)
{
    LinAl::Vec3d expectedOrigin{0, 0, 0};
    EXPECT_EQ(aabb.getOrigin(), expectedOrigin);

    for (auto value: aabb.getExtend())
        EXPECT_EQ(value, 1.0f);
}