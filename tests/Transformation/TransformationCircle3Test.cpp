#include <Geometry/Transform/TransformCircle.hpp>
#include <gtest/gtest.h>


#include <linal/vec.hpp>
#include <linal/utils/constants.hpp>


class fixture_transformation_circle3 : public ::testing::Test {
protected:
  Geometry::Circle3d m_circle3{linal::double3{1, 1, 1}, 0.5, linal::double3Z};
};

TEST_F(fixture_transformation_circle3, circle3_translation)
{
  auto translationVec = linal::double3{1, 1, 1};
  linal::hmatd trafo = linal::hmatd::identity();
  trafo.set_translation(translationVec);
  auto tCircle = Geometry::transform(m_circle3, trafo);
  auto expectedOrigin = linal::double3{2, 2, 2};
  EXPECT_EQ(expectedOrigin, tCircle.get_origin());
}

TEST_F(fixture_transformation_circle3, circle3_rotY)
{
  linal::hmatd rotY = linal::hmatd::identity();
  linal::rot_y(rotY, linal::PI_HALF_D);
  auto tCircle = Geometry::transform(m_circle3, rotY);
  auto expectedOrigin = linal::double3{1, 1, -1};
  EXPECT_EQ(expectedOrigin, tCircle.get_origin());
}
