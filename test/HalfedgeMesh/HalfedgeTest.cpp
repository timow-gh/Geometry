#include <Geometry/Cuboid.hpp>
#include <Geometry/HalfedgeMesh/CalcFaceHalfedges.hpp>
#include <Geometry/HalfedgeMesh/Facet.hpp>
#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <Geometry/HalfedgeMeshBuilder/CuboidMeshBuilder.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

using HalfedgeMesh_t = HalfedgeMesh<MeshTraits<float, std::size_t>>;
using Vertex_t = HalfedgeMesh_t::Vertex_t;
using Halfedge_t = HalfedgeMesh_t::Halfedge_t;
using Facet_t = HalfedgeMesh_t::Facet_t;

using VertexIndex_t = HalfedgeMesh_t::VertexIndex_t;
using HalfedgeIndex_t = HalfedgeMesh_t::HalfedgeIndex_t;
using FacetIndex_t = HalfedgeMesh_t::FacetIndex_t;

class TestHalfedge : public ::testing::Test {
protected:
  TestHalfedge()
      : m_heMesh(CuboidMeshBuilder<float, std::size_t>().set_cuboid(Cuboid<float>({0, 0, 0}, {1, 1, 1})).build())
      , m_halfedge(m_heMesh->get_halfedges().front())
  {
  }

  std::unique_ptr<HalfedgeMesh_t> m_heMesh;
  Halfedge_t m_halfedge;
};

TEST_F(TestHalfedge, get_facet)
{
  Facet_t facet = m_halfedge.get_facet();
  EXPECT_TRUE(facet.get_halfedge_index().is_valid());
}

TEST_F(TestHalfedge, get_facet_index)
{
  FacetIndex_t facetIndex = m_halfedge.get_facet_index();
  EXPECT_TRUE(facetIndex.is_valid());
}

TEST_F(TestHalfedge, set_facet_index)
{
  FacetIndex_t facetIndex = m_halfedge.get_facet_index();
  m_halfedge.set_facet_index(FacetIndex_t{facetIndex.get_value() + 1});
  EXPECT_NE(m_halfedge.get_facet_index(), facetIndex);
}

TEST_F(TestHalfedge, get_next)
{
  const Halfedge_t& nextHe = m_halfedge.get_next();
  const auto cHe = m_halfedge;
  const Halfedge_t& cNextHe = cHe.get_next();
  EXPECT_EQ(nextHe, cNextHe);
}

TEST_F(TestHalfedge, set_next_index)
{
  HalfedgeIndex_t nextHeIndex = m_halfedge.get_next_index();
  HalfedgeIndex_t modifiedHeIndex = HalfedgeIndex_t{nextHeIndex.get_value() + 1};
  m_halfedge.set_next_index(modifiedHeIndex);
  EXPECT_EQ(modifiedHeIndex, m_halfedge.get_next_index());
}

TEST_F(TestHalfedge, get_previous)
{
  const Halfedge_t& nextHe = m_halfedge.get_previous();
  const auto cHe = m_halfedge;
  const Halfedge_t& cNextHe = cHe.get_previous();
  EXPECT_EQ(nextHe, cNextHe);
}

TEST_F(TestHalfedge, set_previous_index)
{
  HalfedgeIndex_t prevIndex = m_halfedge.get_previous_index();
  HalfedgeIndex_t modifiedHeIndex = HalfedgeIndex_t{prevIndex.get_value() + 1};
  m_halfedge.set_previous_index(modifiedHeIndex);
  EXPECT_EQ(modifiedHeIndex, m_halfedge.get_previous_index());
}

TEST_F(TestHalfedge, get_opposite)
{
  Halfedge_t oppHe = m_halfedge.get_opposite();
  const auto cHe = m_halfedge;
  Halfedge_t cOppHe = cHe.get_opposite();
  EXPECT_EQ(oppHe, cOppHe);
}

TEST_F(TestHalfedge, set_opposite_index)
{
  HalfedgeIndex_t oppIdx = m_halfedge.get_opposite_index();
  HalfedgeIndex_t modifiedHeIndex = HalfedgeIndex_t{oppIdx.get_value() + 1};
  m_halfedge.set_opposite_index(modifiedHeIndex);
  EXPECT_EQ(modifiedHeIndex, m_halfedge.get_opposite_index());
}

TEST_F(TestHalfedge, get_vertex)
{
  const Vertex_t& vertex = m_halfedge.get_vertex();
  const auto cHe = m_halfedge;
  const Vertex_t& cVertex = cHe.get_vertex();
  EXPECT_EQ(vertex, cVertex);
}

TEST_F(TestHalfedge, set_vertex_index)
{
  VertexIndex_t vertexIdx = m_halfedge.get_vertex_index();
  VertexIndex_t modifiedIndex = VertexIndex_t{vertexIdx.get_value() + 1};
  m_halfedge.set_vertex_index(modifiedIndex);
  EXPECT_EQ(modifiedIndex, m_halfedge.get_vertex_index());
}

TEST_F(TestHalfedge, get_next_vertex)
{
  const Vertex_t& nextVertex = m_halfedge.get_next_vertex();
  const auto cHe = m_halfedge;
  const Vertex_t& cNextVertex = cHe.get_next_vertex();
  EXPECT_EQ(nextVertex, cNextVertex);
}
