#include <Geometry/Cuboid.hpp>
#include <Geometry/HalfedgeMesh/CalcVertexNormals.hpp>
#include <Geometry/HalfedgeMesh/Facet.hpp>
#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <Geometry/HalfedgeMeshBuilder/CuboidMeshBuilder.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

using HalfedgeMesh_t = HalfedgeMesh<MeshTraits<float, std::size_t>>;

class CalcVertexFacetsTest : public ::testing::Test {
protected:
  CalcVertexFacetsTest()
      : m_heMesh(CuboidMeshBuilder<float, std::size_t>().set_cuboid(Cuboid<float>({0, 0, 0}, {1, 1, 1})).build())
      , m_facet(m_heMesh->get_facets().front())
  {
  }

  std::unique_ptr<HalfedgeMesh_t> m_heMesh;
  HalfedgeMesh_t::Facet_t m_facet;
};

TEST_F(CalcVertexFacetsTest, CubeFacets)
{
  const auto& vertices = m_heMesh->get_vertices();
  EXPECT_EQ(vertices.size(), 8);

  const auto& halfedges = m_heMesh->get_halfedges();
  EXPECT_EQ(halfedges.size(), 36);

  const auto& facets = m_heMesh->get_facets();
  EXPECT_EQ(facets.size(), 12);
}

TEST_F(CalcVertexFacetsTest, CubeVertexFacets)
{
  const auto& vertex = m_heMesh->get_vertices().front();
  const auto& facets = calc_vertex_facets(vertex);
  EXPECT_EQ(facets.size(), 6);
}