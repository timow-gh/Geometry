#include <Geometry/GeometryAssert.hpp>
#include <Geometry/Ray.hpp>
#include <LinAl/LinearAlgebra.hpp>
#include <gtest/gtest.h>

using namespace Geometry;
using namespace LinAl;

TEST(Ray_Constructor, Default_Constructor)
{
    Ray ray{LinAl::Vec3f{}, LinAl::X_VEC3F};
}

TEST(Sphere, valid)
{
    Ray3<double_t> ray{LinAl::Vec3d{0, 0, 0}, LinAl::Vec3d{0, 0, 0}};
#ifdef NDEBUG
    EXPECT_DEATH(GEOMETRY_PRECONDITION_RAY_DIRECTION_ASSERT(ray), "");
#else
    EXPECT_DEATH(GEOMETRY_PRECONDITION_RAY_DIRECTION_DEBUG_ASSERT(ray), "");
#endif
}
