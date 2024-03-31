#include <Geometry/Cuboid.hpp>
#include <Geometry/HalfedgeMeshBuilder/CuboidMeshBuilder.hpp>
#include <gtest/gtest.h>
#include <linal/vec.hpp>

using namespace Geometry;

TEST(MeshBuilder, createACubeMesh)
{
  auto res = CuboidMeshBuilder<double, std::size_t>().set_cuboid(Cuboid<double>{linal::double3{}, linal::double3{1, 1, 1}}).build();
  EXPECT_TRUE(res);
}
