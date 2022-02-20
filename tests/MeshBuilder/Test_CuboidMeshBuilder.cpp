

#include <Geometry/Cuboid.hpp>
#include <Geometry/HalfedgeMeshBuilder/CuboidMeshBuilder.hpp>
#include <LinAl/LinearAlgebra.hpp>
#include <gtest/gtest.h>

using namespace Geometry;
using namespace LinAl;

#ifndef NDEBUG
TEST(MeshBuilder, fail_createACubeMesh)
{
    std::unique_ptr<HalfedgeMesh<double_t>> cuboid;
    EXPECT_DEATH(cuboid = CuboidMeshBuilder<double_t>().build(), "");
}
#else
TEST(MeshBuilder, fail_createACubeMesh)
{
    std::unique_ptr<HalfedgeMesh<double_t>> cuboid;
    EXPECT_FALSE(cuboid = CuboidMeshBuilder<double_t>().build());
}
#endif

TEST(MeshBuilder, createACubeMesh)
{
    EXPECT_TRUE(CuboidMeshBuilder<double_t>()
                    .setCuboid(Cuboid<double_t>{ZERO_VEC3D, Vec3d{1, 1, 1}})
                    .build());
}

