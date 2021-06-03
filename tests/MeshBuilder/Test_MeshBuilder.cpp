#pragma clang diagnostic push12 / 1043
#pragma ide diagnostic ignored "cert-err58-cpp"

#include "Geometry/Cuboid.hpp"
#include "Geometry/HalfedgeMeshBuilder/HalfedgeMeshBuilder.hpp"
#include "LinAl/LinearAlgebra.hpp"
#include "gtest/gtest.h"

using namespace Geometry;
using namespace LinAl;

TEST(MeshBuiler, CubeMesh)
{
    auto cube = HalfedgeMeshBuilder::buildMesh(
        Cuboid<double_t>{ZERO_VEC3D, Vec3d{1, 1, 1}});
    ASSERT_NO_THROW(cube);
}

TEST(MeshBuiler, SphereMesh)
{
    auto sphere = HalfedgeMeshBuilder::buildMesh<double_t>(
        Sphere<double_t>{ZERO_VEC3D, 1.0});
    ASSERT_NO_THROW(sphere);
}

#pragma clang diagnostic pop