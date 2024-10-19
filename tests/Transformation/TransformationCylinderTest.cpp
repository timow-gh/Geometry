#include <Geometry/Cylinder.hpp>
#include <Geometry/Transform/TransformCylinder.hpp>
#include <gtest/gtest.h>
#include <linal/vec.hpp>

class fixture_cylinder : public ::testing::Test {
protected:
  Geometry::Cylinder<double> m_cylinder{Geometry::Segment3d{linal::double3{1, 1, 1}, linal::double3{2, 2, 2}}, 0.5};
};

TEST_F(fixture_cylinder, cone_trafo)
{
  auto translationVec = linal::double3{1, 1, 1};
  linal::hmatd trafo = linal::hmatd::identity();
  trafo.set_translation(translationVec);
  auto tcylinder = Geometry::transform(m_cylinder, trafo);
  auto expectedOrigin = linal::double3{2, 2, 2};
  EXPECT_EQ(expectedOrigin, tcylinder.get_segment().get_source());
}
