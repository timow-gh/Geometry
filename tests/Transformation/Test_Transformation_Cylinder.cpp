#include <Geometry/Cylinder.hpp>
#include <Geometry/Transform/TransformCylinder.hpp>
#include <gtest/gtest.h>
#include <linal/hmat_translation.hpp>
#include <linal/vec3.hpp>


class fixture_cylinder : public ::testing::Test {
protected:
  Geometry::Cylinder<double> m_cylinder{Geometry::Segment3d{linal::vec3d{1, 1, 1}, linal::vec3d{2, 2, 2}}, 0.5};
};

TEST_F(fixture_cylinder, cone_trafo)
{
  auto translationVec = linal::vec3d{1, 1, 1};
  auto trafo = linal::hcoord::hmat_translation(translationVec[0], translationVec[1], translationVec[2]);
  auto tcylinder = Geometry::transform(m_cylinder, trafo);
  auto expectedOrigin = linal::vec3d{2, 2, 2};
  EXPECT_EQ(expectedOrigin, tcylinder.get_segment().get_source());
}
