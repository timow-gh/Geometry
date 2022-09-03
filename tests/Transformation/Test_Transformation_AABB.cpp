#include <Geometry/AABB.hpp>
#include <Geometry/Transformation/TransformatonAABB.hpp>
#include <LinAl/LinearAlgebra.hpp>
#include <gtest/gtest.h>

using namespace Geometry;
using namespace LinAl;

class fixture_transformation_aabb3d : public ::testing::Test {
  protected:
    AABB3d m_aabb{ZERO_VEC3D, 0.5};
};

TEST_F(fixture_transformation_aabb3d, aabb)
{
    auto expectedOrigin = LinAl::Vec3d{1, 1, 1};
    auto trafo = LinAl::createTranslation(vec3ToHVec(expectedOrigin));
    auto transformedAABB = Geometry::transformation(m_aabb, trafo);
    EXPECT_EQ(expectedOrigin, transformedAABB.getOrigin());
}
