#include "SphereTestHelper.h"
#include <Geometry/HalfedgeMesh/Vertex.hpp>
#include <Geometry/HalfedgeMeshBuilder/SphereMeshBuilder.hpp>
#include <Geometry/Sphere.hpp>
#include <cstddef>
#include <gtest/gtest.h>
#include <linal/utils/eps.hpp>
#include <linal/vec.hpp>

using namespace MeshTestHelper;
using namespace Geometry;

class TestMeshBuilderSphere : public ::testing::Test {
protected:
  Sphere<double> m_sphere;
  std::unique_ptr<HalfedgeMesh<double>> m_sphereMesh;

  TestMeshBuilderSphere()
      : m_sphere({linal::double3{}, 3.0})
  {
    m_sphereMesh = SphereMeshBuilder<double, std::size_t>().set_sphere(m_sphere).build();
    EXPECT_TRUE(m_sphereMesh->is_valid());
  }
};

// What checks do I have to write to make sure that a halfedge mesh is valid?
// The halfedge mesh is valid if:
// 1. Each vertex has a valid index.
// 2. Each halfedge has a valid index.
// 3. Each face has a valid index.
// 4. Each halfedge has a valid opposite halfedge.
// 5. Each halfedge has a valid next halfedge.
// 6. Each halfedge has a valid previous halfedge.
// 7. Each halfedge has a valid vertex.
// 8. Each halfedge has a valid face.
// 9. Each face has a valid halfedge.
// 10. Each vertex has a valid halfedge.
// 11. Each vertex has a valid point.
// 12. Each face has a valid normal.

TEST_F(TestMeshBuilderSphere, VertexIndices)
{
  for (const auto& vertex: m_sphereMesh->getVertices())
  {
    EXPECT_TRUE(vertex.is_valid());
  }
}

TEST_F(TestMeshBuilderSphere, MeshPoints)
{
  for (const auto& vertexPoint: m_sphereMesh->getMeshPoints().getPoints())
  {
    EXPECT_TRUE(isPointOnSphere(m_sphere, vertexPoint));
  }
}
