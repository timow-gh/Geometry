#include <Geometry/Cylinder.hpp>
#include <Geometry/HalfedgeMeshBuilder/CylinderMeshBuilder.hpp>
#include <gtest/gtest.h>
#include <linal/vec.hpp>

using namespace Geometry;

TEST(CylinderMeshBuilder, createCylinderMesh)
{
  auto cylinderMesh = CylinderMeshBuilder<double, std::size_t>()
                          .set_cylinder(Cylinder<double>(Segment3d{linal::double3{}, linal::double3{} + linal::double3Z}, 10.0))
                          .build();
  EXPECT_TRUE(cylinderMesh->is_valid());
  EXPECT_TRUE(cylinderMesh);
}
