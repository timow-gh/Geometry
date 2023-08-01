#include <Geometry/Cone.hpp>
#include <Geometry/Transform/TransformCone.hpp>
#include <gtest/gtest.h>
#include <linal/hmat_translation.hpp>
#include <linal/vec3.hpp>


class fixture_cone : public ::testing::Test {
protected:
  Geometry::Cone<double> m_cone{Geometry::Segment3d{linal::vec3d{1, 1, 1}, linal::vec3d{2, 2, 2}}, 0.5};
};

TEST_F(fixture_cone, cone_trafo)
{
  auto translationVec = linal::vec3d{1, 1, 1};
  auto trafo = linal::hcoord::hmat_translation(translationVec[0], translationVec[1], translationVec[2]);
  auto tCone = Geometry::transform(m_cone, trafo);
  auto expectedOrigin = linal::vec3d{2, 2, 2};
  EXPECT_EQ(expectedOrigin, tCone.get_segment().get_source());
}
