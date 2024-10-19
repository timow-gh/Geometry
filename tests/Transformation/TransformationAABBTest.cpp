#include <Geometry/AABB.hpp>
#include <Geometry/Transform/TransformAABB.hpp>
#include <gtest/gtest.h>
#include <linal/vec.hpp>

using namespace Geometry;

class fixture_transformation_aabb3d : public ::testing::Test {
protected:
  AABB3d m_aabb{linal::double3{0.5, 0.5, 0.5}, 1.0};
};

TEST_F(fixture_transformation_aabb3d, aabb)
{
  auto translationVec = linal::double3{0.5, 0.5, 0.5};
  linal::hmatd trafo = linal::hmatd::identity();
  trafo.set_translation(translationVec);
  auto transformedAABB = Geometry::transform(m_aabb, trafo);
  auto expectedMin = linal::double3{1, 1, 1};
  EXPECT_EQ(expectedMin, transformedAABB.get_min());
}
