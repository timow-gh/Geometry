#include <Core/Math/Constants.hpp>
#include <Geometry/Cuboid.hpp>
#include <gtest/gtest.h>

using namespace Geometry;
using namespace LinAl;

class Fixture_Cuboid : public ::testing::Test {
  protected:
    Cuboid<double> m_cuboid{ZERO_VEC3D, Z_VEC3D};
};

TEST_F(Fixture_Cuboid, transformation)
{
    auto expectedOrigin = LinAl::Vec3d{0, 0, 0};
    EXPECT_EQ(expectedOrigin, m_cuboid.getOrigin());
}
