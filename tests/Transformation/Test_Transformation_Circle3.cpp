#include <Geometry/Transformation/TransformCircle.hpp>
#include <gtest/gtest.h>
#include <linal/HMat.hpp>
#include <linal/HMatTranslation.hpp>
#include <linal/HVec.hpp>
#include <linal/Vec3.hpp>

using namespace linal;

class fixture_transformation_circle3 : public ::testing::Test {
protected:
  Geometry::Circle3d m_circle3{linal::Vec3d{1, 1, 1}, 0.5, linal::Z_VEC3D};
};

TEST_F(fixture_transformation_circle3, circle3_trafo)
{
  auto translationVec = linal::Vec3d{1, 1, 1};
  auto trafo = linal::hcoord::create_translation(translationVec[0], translationVec[1], translationVec[2]);
  auto tCircle = Geometry::transformation(m_circle3, trafo);
  auto expectedOrigin = linal::Vec3d{2, 2, 2};
  EXPECT_EQ(expectedOrigin, tCircle.getOrigin());
}
