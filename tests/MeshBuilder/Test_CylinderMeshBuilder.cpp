#include <Geometry/Cylinder.hpp>
#include <Geometry/HalfedgeMeshBuilder/CylinderMeshBuilder.hpp>
#include <LinAl/LinearAlgebra.hpp>
#include <gtest/gtest.h>

using namespace Geometry;
using namespace LinAl;

TEST(CylinderMeshBuilder, createCylinderMesh)
{
  auto cylinderMesh =
      CylinderMeshBuilder<double, std::size_t>().setCylinder(Cylinder<double>(Segment3d{ZERO_VEC3D, ZERO_VEC3D + Z_VEC3D}, 10.0)).build();
  EXPECT_TRUE(cylinderMesh);
}
