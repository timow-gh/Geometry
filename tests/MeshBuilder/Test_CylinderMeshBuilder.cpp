#include <Geometry/Cylinder.hpp>
#include <Geometry/HalfedgeMeshBuilder/CylinderMeshBuilder.hpp>
#include <gtest/gtest.h>
#include <linal/vec3.hpp>

using namespace Geometry;

TEST(CylinderMeshBuilder, createCylinderMesh)
{
  auto cylinderMesh = CylinderMeshBuilder<double, std::size_t>()
                          .set_cylinder(Cylinder<double>(Segment3d{linal::ZERO_VEC3D, linal::ZERO_VEC3D + linal::Z_VEC3D}, 10.0))
                          .build();
  EXPECT_TRUE(cylinderMesh);
}
