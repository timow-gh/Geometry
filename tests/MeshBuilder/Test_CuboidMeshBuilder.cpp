#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include <Geometry/Cuboid.hpp>
#include <Geometry/HalfedgeMeshBuilder/CuboidMeshBuilder.hpp>
#include <LinAl/LinearAlgebra.hpp>
#include <gtest/gtest.h>

using namespace Geometry;
using namespace LinAl;

TEST(MeshBuilder, createACubeMesh)
{
    auto cube = CuboidMeshBuilder<double_t>()
                    .setCuboid(Cuboid<double_t>{ZERO_VEC3D, Vec3d{1, 1, 1}})
                    .build();
    EXPECT_TRUE(cube);
}

#pragma clang diagnostic pop