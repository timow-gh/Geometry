#include <Geometry/Cylinder.hpp>
#include <Geometry/HalfedgeMeshBuilder/CylinderMeshBuilder.hpp>
#include <gtest/gtest.h>
#include <linal/Vec3.hpp>

using namespace Geometry;
using namespace linal;

TEST(CylinderMeshBuilder, createCylinderMesh)
{
  auto cylinderMesh =
      CylinderMeshBuilder<double, std::size_t>().setCylinder(Cylinder<double>(Segment3d{ZERO_VEC3D, ZERO_VEC3D + Z_VEC3D}, 10.0)).build();
  EXPECT_TRUE(cylinderMesh);
}
