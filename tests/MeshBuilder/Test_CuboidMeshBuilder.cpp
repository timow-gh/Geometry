#include <Geometry/Cuboid.hpp>
#include <Geometry/HalfedgeMeshBuilder/CuboidMeshBuilder.hpp>
#include <LinAl/LinearAlgebra.hpp>
#include <gtest/gtest.h>

using namespace Geometry;
using namespace LinAl;

TEST(MeshBuilder, createACubeMesh)
{
  auto res = CuboidMeshBuilder<double, std::size_t>().setCuboid(Cuboid<double>{ZERO_VEC3D, Vec3d{1, 1, 1}}).build();
  EXPECT_TRUE(res);
}
