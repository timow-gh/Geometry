#include <Geometry/Cylinder.hpp>
#include <Geometry/Transformation/TransformCylinder.hpp>
#include <gtest/gtest.h>
#include <linal/HMatTranslation.hpp>
#include <linal/Vec3.hpp>

using namespace linal;

class fixture_cylinder : public ::testing::Test {
protected:
  Geometry::Cylinder<double> m_cylinder{Geometry::Segment3d{linal::Vec3d{1, 1, 1}, linal::Vec3d{2, 2, 2}}, 0.5};
};

TEST_F(fixture_cylinder, cone_trafo)
{
  auto translationVec = linal::Vec3d{1, 1, 1};
  auto trafo = linal::hcoord::create_translation(translationVec[0], translationVec[1], translationVec[2]);
  auto tcylinder = Geometry::transformation(m_cylinder, trafo);
  auto expectedOrigin = linal::Vec3d{2, 2, 2};
  EXPECT_EQ(expectedOrigin, tcylinder.getSegment().getSource());
}
