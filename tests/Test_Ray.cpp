#include <Geometry/Ray.hpp>
#include <linal/Vec3.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

TEST(Ray_Constructor, Default_Constructor)
{
    Ray ray{linal::Vec3f{}, linal::X_VEC3F};
}

