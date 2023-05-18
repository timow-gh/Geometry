#include <Geometry/AABB.hpp>
#include <Geometry/Transformation/TransformatonAABB.hpp>
#include <gtest/gtest.h>
#include <linal/HMat.hpp>
#include <linal/HMatTranslation.hpp>
#include <linal/HVec.hpp>
#include <linal/Vec3.hpp>

using namespace Geometry;
using namespace linal;

class fixture_transformation_aabb3d : public ::testing::Test {
protected:
  AABB3d m_aabb{ZERO_VEC3D, 0.5};
};

TEST_F(fixture_transformation_aabb3d, aabb)
{
  auto expectedOrigin = linal::Vec3d{1, 1, 1};
  auto trafo = linal::hcoord::create_translation(expectedOrigin[0], expectedOrigin[1], expectedOrigin[2]);
  auto transformedAABB = Geometry::transformation(m_aabb, trafo);
  EXPECT_EQ(expectedOrigin, transformedAABB.getOrigin());
}
