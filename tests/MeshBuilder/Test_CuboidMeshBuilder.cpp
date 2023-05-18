#include <Geometry/Cuboid.hpp>
#include <Geometry/HalfedgeMeshBuilder/CuboidMeshBuilder.hpp>
#include <gtest/gtest.h>
#include <linal/vec3.hpp>

using namespace Geometry;

TEST(MeshBuilder, createACubeMesh)
{
  auto res = CuboidMeshBuilder<double, std::size_t>().setCuboid(Cuboid<double>{linal::ZERO_VEC3D, linal::vec3d{1, 1, 1}}).build();
  EXPECT_TRUE(res);
}
