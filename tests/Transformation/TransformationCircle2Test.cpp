#include <Geometry/Transform/TransformCircle.hpp>
#include <gtest/gtest.h>

class fixture_transformation_circle2 : public ::testing::Test {
protected:
  Geometry::Circle2d m_circle2d{linal::double2{1, 1}, 0.5};
};

TEST_F(fixture_transformation_circle2, circle2_trafo)
{
  auto translationVec = linal::double3{1, 1, 0};
  linal::hmatd trafo = linal::hmatd::identity();
  trafo.set_translation(translationVec);
  auto tCircle2d = Geometry::transform(m_circle2d, trafo);
  auto expectedOrigin = linal::double2{2, 2};
  EXPECT_EQ(expectedOrigin, tCircle2d.get_origin());
}
