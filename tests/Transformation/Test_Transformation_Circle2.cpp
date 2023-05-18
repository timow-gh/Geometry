#include <Geometry/Transformation/TransformCircle.hpp>
#include <gtest/gtest.h>
#include <linal/hmat_translation.hpp>
#include <linal/vec2.hpp>


class fixture_transformation_circle2 : public ::testing::Test {
protected:
  Geometry::Circle2d m_circle2d{linal::vec2d{1, 1}, 0.5};
};

TEST_F(fixture_transformation_circle2, circle2_trafo)
{
  auto translationVec = linal::vec2d{1, 1};
  auto trafo = linal::hcoord::create_translation(translationVec[0], translationVec[1], 0.0);
  auto tCircle2d = Geometry::transformation(m_circle2d, trafo);
  auto expectedOrigin = linal::vec2d{2, 2};
  EXPECT_EQ(expectedOrigin, tCircle2d.getOrigin());
}
