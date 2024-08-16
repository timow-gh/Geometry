#include <Geometry/Cuboid.hpp>
#include <Geometry/HalfedgeMesh/CalcVertexNormals.hpp>
#include <Geometry/HalfedgeMesh/Facet.hpp>
#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <Geometry/HalfedgeMeshBuilder/CuboidMeshBuilder.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

using HalfedgeMesh_t = HalfedgeMesh<MeshTraits<float, std::size_t>>;

class CalcVertexNormalsTest : public ::testing::Test {
protected:
  CalcVertexNormalsTest()
      : m_heMesh(CuboidMeshBuilder<float, std::size_t>().set_cuboid(Cuboid<float>({0, 0, 0}, {1, 1, 1})).build())
      , m_facet(m_heMesh->getFacets().front())
  {
  }

  std::unique_ptr<HalfedgeMesh_t> m_heMesh;
  HalfedgeMesh_t::Facet_t m_facet;
};

TEST_F(CalcVertexNormalsTest, vertexNormals)
{
  auto normals = calc_vertex_normals(*m_heMesh);
  ASSERT_EQ(normals.size(), m_heMesh->getMeshPoints().size());
}