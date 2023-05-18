#include <Geometry/Cone.hpp>
#include <Geometry/HalfedgeMeshBuilder/ConeMeshBuilder.hpp>
#include <gtest/gtest.h>
#include <linal/vec3.hpp>

using namespace Geometry;

TEST(ConeMeshBuilder, createAConeMesh)
{
  auto coneMesh = ConeMeshBuilder<double, std::size_t>().setCone(Cone<double>(linal::ZERO_VEC3D, linal::Z_VEC3D, 10.0)).build();
  EXPECT_TRUE(coneMesh);
}
