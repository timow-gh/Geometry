#include <Geometry/Ray.hpp>
#include <linal/vec3.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

TEST(Ray_Constructor, Default_Constructor)
{
    Ray ray{linal::vec3f{}, linal::X_VEC3F};
}

