#include <Geometry/AABB.hpp>
#include <Geometry/Transformation/TransformatonAABB.hpp>
#include <gtest/gtest.h>
#include <linal/hmat_translation.hpp>
#include <linal/vec3.hpp>

using namespace Geometry;

class fixture_transformation_aabb3d : public ::testing::Test {
protected:
  AABB3d m_aabb{linal::vec3d{0.5, 0.5, 0.5}, 1.0};
};

TEST_F(fixture_transformation_aabb3d, aabb)
{
  auto translationVec = linal::vec3d{0.5, 0.5, 0.5};
  auto trafo = linal::hcoord::create_translation(translationVec[0], translationVec[1], translationVec[2]);
  auto transformedAABB = Geometry::transformation(m_aabb, trafo);
  auto expectedMin = linal::vec3d{1, 1, 1};
  EXPECT_EQ(expectedMin, transformedAABB.get_min());
}
