#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include "Geometry/Cuboid.hpp"
#include "Geometry/HalfedgeMeshBuilder/HalfedgeMeshBuilder.hpp"
#include "LinAl/LinearAlgebra.hpp"
#include "gtest/gtest.h"

using namespace Geometry;
using namespace LinAl;

TEST(MeshBuiler, CubeMesh)
{
    auto cube = HalfedgeMeshBuilder().buildMesh(
        Cuboid<double_t>{ZERO_VEC3D, Vec3d{1, 1, 1}});
    EXPECT_TRUE(cube);
}

#pragma clang diagnostic pop