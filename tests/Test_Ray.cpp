

#include <Geometry/Ray.hpp>
#include <LinAl/LinearAlgebra.hpp>
#include <gtest/gtest.h>

using namespace Geometry;
using namespace LinAl;

TEST(Ray_Constructor, Default_Constructor)
{
    Ray ray{LinAl::Vec3f{}, LinAl::X_VEC3F};
}

