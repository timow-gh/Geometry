#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include <Core/Math/Constants.hpp>
#include <Geometry/Cone.hpp>
#include <gtest/gtest.h>

using namespace Geometry;
using namespace LinAl;

class Fixture_Cone : public ::testing::Test {
  protected:
    Cone<double_t> m_cone{Segment3d{ZERO_VEC3D, Z_VEC3D}, 1.0};
};

TEST_F(Fixture_Cone, height)
{
    auto height = m_cone.height();
    EXPECT_DOUBLE_EQ(height, 1.0);
}

TEST_F(Fixture_Cone, slantHeight)
{
    auto slantHeight = m_cone.slantHeight();
    EXPECT_DOUBLE_EQ(slantHeight, std::sqrt(2.0));
}

TEST_F(Fixture_Cone, openingAngle)
{
    auto openingAngle = m_cone.openingAngle();
    // rad = 90 * Core::PI / 180
    EXPECT_DOUBLE_EQ(openingAngle, 90 * Core::PI<double_t> / 180);
}

#pragma clang diagnostic pop