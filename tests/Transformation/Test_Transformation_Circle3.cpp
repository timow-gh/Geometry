#include <Geometry/Transform/TransformCircle.hpp>
#include <gtest/gtest.h>
#include <linal/hmat_rotation.hpp>
#include <linal/hmat_translation.hpp>
#include <linal/vec3.hpp>
#include <linal/utils/constants.hpp>


class fixture_transformation_circle3 : public ::testing::Test {
protected:
  Geometry::Circle3d m_circle3{linal::vec3d{1, 1, 1}, 0.5, linal::Z_VEC3D};
};

TEST_F(fixture_transformation_circle3, circle3_translation)
{
  auto translationVec = linal::vec3d{1, 1, 1};
  auto trafo = linal::hcoord::create_translation(translationVec[0], translationVec[1], translationVec[2]);
  auto tCircle = Geometry::transform(m_circle3, trafo);
  auto expectedOrigin = linal::vec3d{2, 2, 2};
  EXPECT_EQ(expectedOrigin, tCircle.get_origin());
}

TEST_F(fixture_transformation_circle3, circle3_rotY)
{
  auto rotY = linal::hcoord::rot_y(linal::PI_HALF_D);
  auto tCircle = Geometry::transform(m_circle3, rotY);
  auto expectedOrigin = linal::vec3d{1, 1, -1};
  EXPECT_EQ(expectedOrigin, tCircle.get_origin());
}
