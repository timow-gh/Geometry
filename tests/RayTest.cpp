#include <Geometry/Ray.hpp>
#include <linal/vec.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

TEST(Ray_Constructor, Default_Constructor)
{
    [[maybe_unused]] Ray ray{linal::float3{}, linal::float3X};
}

