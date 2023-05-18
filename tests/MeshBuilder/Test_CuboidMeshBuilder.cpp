#include <Geometry/Cuboid.hpp>
#include <Geometry/HalfedgeMeshBuilder/CuboidMeshBuilder.hpp>
#include <gtest/gtest.h>
#include <linal/Vec3.hpp>

using namespace Geometry;
using namespace linal;

TEST(MeshBuilder, createACubeMesh)
{
  auto res = CuboidMeshBuilder<double, std::size_t>().setCuboid(Cuboid<double>{ZERO_VEC3D, Vec3d{1, 1, 1}}).build();
  EXPECT_TRUE(res);
}
