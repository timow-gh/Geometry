#include <Geometry/Cuboid.hpp>
#include <Geometry/HalfedgeMesh/CalcVertexNormals.hpp>
#include <Geometry/HalfedgeMesh/Facet.hpp>
#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <Geometry/HalfedgeMeshBuilder/CuboidMeshBuilder.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

class CalcVertexNormalsTest : public ::testing::Test {
protected:
  CalcVertexNormalsTest()
      : m_heMesh(CuboidMeshBuilder<float, std::size_t>().set_cuboid(Cuboid<float>({0, 0, 0}, {1, 1, 1})).build())
      , m_facet(m_heMesh->getFacets().front())
  {
  }

  std::unique_ptr<HalfedgeMesh<float>> m_heMesh;
  HalfedgeMesh<float>::Facet_t m_facet;
};

TEST_F(CalcVertexNormalsTest, vertexNormals)
{
  auto normals = calc_vertex_normals(*m_heMesh);
  ASSERT_EQ(normals.size(), m_heMesh->getMeshPoints().size());
}