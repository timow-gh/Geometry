#pragma clang diagnostic push12 / 1043
#pragma ide diagnostic ignored "cert-err58-cpp"

#include "gtest/gtest.h"

#include "LinAl/LinearAlgebra.hpp"

#include "Geometry/HalfedgeMesh/HalfedgeMeshBuilder.hpp"

using namespace Geometry;
using namespace LinAl;

TEST(MeshBuiler, CubeMesh) {
  auto cube = HalfedgeMeshBuilder::buildMesh(
      Cube<double_t>{ZERO_VEC3D, Vec3d{1, 1, 1}});
  ASSERT_NO_THROW(cube);
}

TEST(MeshBuiler, SphereMesh) {
  auto sphere =
      HalfedgeMeshBuilder::buildMesh(Sphere<double_t>{ZERO_VEC3D, 1.0});
  ASSERT_NO_THROW(sphere);
}

#pragma clang diagnostic pop