#include <Geometry/Sphere.hpp>
#include <Geometry/Transformation/TransformSphere.hpp>
#include <gtest/gtest.h>
#include <linal/HMatTranslation.hpp>
#include <linal/Vec3.hpp>

using namespace linal;

class fixture_sphere : public ::testing::Test {
protected:
  Geometry::Sphere<double> m_sphere{linal::Vec3d{1, 1, 1}, 0.5};
};

TEST_F(fixture_sphere, cone_trafo)
{
  auto translationVec = linal::Vec3d{1, 1, 1};
  auto trafo = linal::hcoord::create_translation(translationVec[0], translationVec[1], translationVec[2]);
  auto tSphere = Geometry::transformation(m_sphere, trafo);
  auto expectedOrigin = linal::Vec3d{2, 2, 2};
  EXPECT_EQ(expectedOrigin, tSphere.getOrigin());
}
