#include <Core/Math/Constants.hpp>
#include <Geometry/Circle.hpp>
#include <Geometry/Fwd/FwdCircle.hpp>
#include <gtest/gtest.h>

using namespace Geometry;
using namespace LinAl;

class Circle3d_xz : public ::testing::Test {
  protected:
    // Circle lies in the xy plane
    Circle3d circle{LinAl::Vec3d{0, 0, 0}, 1.0, LinAl::Vec3d{0, -1.0, 0}};
};

TEST_F(Circle3d_xz, circleOrigin)
{
    LinAl::Vec3d expectedOrigin{0, 0, 0};
    EXPECT_EQ(circle.getOrigin(), expectedOrigin);
}

TEST_F(Circle3d_xz, circleNormal)
{
    LinAl::HVecd expectedNormal{0, 0, 1, 1};
    expectedNormal = circle.calcTransformation() * expectedNormal;
    LinAl::Vec3d expectedVec3dNormal{expectedNormal[0], expectedNormal[1], expectedNormal[2]};
    EXPECT_EQ(circle.getNormal(), expectedVec3dNormal);
}

class Circle3d_Translation : public ::testing::Test {
  protected:
    // Circle lies in the xy plane
    Circle3d circle{LinAl::Vec3d{10, 3, 4}, 1.0, LinAl::Vec3d{1.0, 0, 0}};
};

TEST_F(Circle3d_Translation, circleOrigin)
{
    LinAl::HVecd translation;
    LinAl::getTranslation(circle.calcTransformation(), translation);
    LinAl::Vec3d vec3dTranslation{translation[0], translation[1], translation[2]};
    EXPECT_EQ(circle.getOrigin(), vec3dTranslation);
}

TEST_F(Circle3d_Translation, globalCircleNormal)
{
    LinAl::HVecd expectedNormal{0, 0, 1, 1};
    expectedNormal = circle.calcTransformation() * expectedNormal;
    LinAl::Vec3d expectedVec3dNormal{expectedNormal[0], expectedNormal[1], expectedNormal[2]};
    LinAl::Vec3d translatedCircleNormal = circle.getNormal() + circle.getOrigin();
    EXPECT_EQ(translatedCircleNormal, expectedVec3dNormal);
}