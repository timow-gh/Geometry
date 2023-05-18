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
  AABB3d m_aabb{linal::Vec3d{0.5, 0.5, 0.5}, 1.0};
};

TEST_F(fixture_transformation_aabb3d, aabb)
{
  auto translationVec = linal::Vec3d{0.5, 0.5, 0.5};
  auto trafo = linal::hcoord::create_translation(translationVec[0], translationVec[1], translationVec[2]);
  auto transformedAABB = Geometry::transformation(m_aabb, trafo);
  auto expectedOrigin = linal::Vec3d{1, 1, 1};
  EXPECT_EQ(expectedOrigin, transformedAABB.getOrigin());
}
