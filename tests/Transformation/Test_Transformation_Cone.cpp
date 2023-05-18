#include <Geometry/Cone.hpp>
#include <Geometry/Transformation/TransformCone.hpp>
#include <gtest/gtest.h>
#include <linal/HMatTranslation.hpp>
#include <linal/Vec3.hpp>

using namespace linal;

class fixture_cone : public ::testing::Test {
protected:
  Geometry::Cone<double> m_cone{Geometry::Segment3d{linal::Vec3d{1, 1, 1}, linal::Vec3d{2, 2, 2}}, 0.5};
};

TEST_F(fixture_cone, cone_trafo)
{
  auto translationVec = linal::Vec3d{1, 1, 1};
  auto trafo = linal::hcoord::create_translation(translationVec[0], translationVec[1], translationVec[2]);
  auto tCone = Geometry::transformation(m_cone, trafo);
  auto expectedOrigin = linal::Vec3d{2, 2, 2};
  EXPECT_EQ(expectedOrigin, tCone.get_segment().getSource());
}
