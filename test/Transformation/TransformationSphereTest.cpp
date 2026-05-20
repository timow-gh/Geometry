#include <Geometry/Sphere.hpp>
#include <Geometry/Transform/TransformSphere.hpp>
#include <gtest/gtest.h>
#include <linal/vec.hpp>

class fixture_sphere : public ::testing::Test {
protected:
  Geometry::Sphere<double> m_sphere{linal::double3{1, 1, 1}, 0.5};
};

TEST_F(fixture_sphere, cone_trafo)
{
  auto translationVec = linal::double3{1, 1, 1};
  linal::hmatd trafo = linal::hmatd::identity();
  trafo.set_translation(translationVec);
  auto tSphere = Geometry::transform(m_sphere, trafo);
  auto expectedOrigin = linal::double3{2, 2, 2};
  EXPECT_EQ(expectedOrigin, tSphere.get_origin());
}
