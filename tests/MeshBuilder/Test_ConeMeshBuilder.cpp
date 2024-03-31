#include <Geometry/Cone.hpp>
#include <Geometry/HalfedgeMeshBuilder/ConeMeshBuilder.hpp>
#include <gtest/gtest.h>
#include <linal/vec.hpp>

using namespace Geometry;

TEST(ConeMeshBuilder, createAConeMesh)
{
  auto coneMesh = ConeMeshBuilder<double, std::size_t>().set_cone(Cone<double>(linal::double3{}, linal::double3Z, 10.0)).build();
  EXPECT_TRUE(coneMesh);
}
