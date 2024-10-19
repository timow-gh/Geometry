#include <Geometry/Cone.hpp>
#include <Geometry/Transform/TransformCone.hpp>
#include <gtest/gtest.h>
#include <linal/vec.hpp>

class fixture_cone : public ::testing::Test {
protected:
  Geometry::Cone<double> m_cone{Geometry::Segment3d{linal::double3{1, 1, 1}, linal::double3{2, 2, 2}}, 0.5};
};

TEST_F(fixture_cone, cone_trafo)
{
  auto translationVec = linal::double3{1, 1, 1};
  linal::hmatd trafo = linal::hmatd::identity();
  trafo.set_translation(translationVec);
  auto tCone = Geometry::transform(m_cone, trafo);
  auto expectedOrigin = linal::double3{2, 2, 2};
  EXPECT_EQ(expectedOrigin, tCone.get_segment().get_source());
}
