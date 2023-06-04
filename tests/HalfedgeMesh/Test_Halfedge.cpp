#include <Geometry/Cuboid.hpp>
#include <Geometry/HalfedgeMesh/CalcFaceHalfedges.hpp>
#include <Geometry/HalfedgeMesh/Facet.hpp>
#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <Geometry/HalfedgeMeshBuilder/CuboidMeshBuilder.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

using HalfedgeMesh_t = HalfedgeMesh<float>;
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
        , m_halfedge(m_heMesh->getHalfedges().front())
    {
    }

    std::unique_ptr<HalfedgeMesh_t> m_heMesh;
    Halfedge_t m_halfedge;
};

TEST_F(TestHalfedge, getFacet)
{
    Facet_t facet = m_halfedge.getFacet();
    EXPECT_TRUE(facet.getHalfedgeIndex().is_valid());
}

TEST_F(TestHalfedge, getFacetIndex)
{
    FacetIndex_t facetIndex = m_halfedge.getFacetIndex();
    EXPECT_TRUE(facetIndex.is_valid());
}

TEST_F(TestHalfedge, setFacetIndex)
{
    FacetIndex_t facetIndex = m_halfedge.getFacetIndex();
    m_halfedge.setFacetIndex(FacetIndex_t{facetIndex.get_value() + 1});
    EXPECT_NE(m_halfedge.getFacetIndex(), facetIndex);
}

TEST_F(TestHalfedge, getNext)
{
    const Halfedge_t& nextHe = m_halfedge.getNext();
    const auto cHe = m_halfedge;
    const Halfedge_t& cNextHe = cHe.getNext();
    EXPECT_EQ(nextHe, cNextHe);
}

TEST_F(TestHalfedge, setNextIndex)
{
    HalfedgeIndex_t nextHeIndex = m_halfedge.getNextIndex();
    HalfedgeIndex_t modifiedHeIndex = HalfedgeIndex_t{nextHeIndex.get_value() + 1};
    m_halfedge.setNextIndex(modifiedHeIndex);
    EXPECT_EQ(modifiedHeIndex, m_halfedge.getNextIndex());
}

TEST_F(TestHalfedge, getPrevious)
{
    const Halfedge_t& nextHe = m_halfedge.getPrevious();
    const auto cHe = m_halfedge;
    const Halfedge_t& cNextHe = cHe.getPrevious();
    EXPECT_EQ(nextHe, cNextHe);
}

TEST_F(TestHalfedge, setPreviousIndex)
{
    HalfedgeIndex_t prevIndex = m_halfedge.getPreviousIndex();
    HalfedgeIndex_t modifiedHeIndex = HalfedgeIndex_t{prevIndex.get_value() + 1};
    m_halfedge.setPreviousIndex(modifiedHeIndex);
    EXPECT_EQ(modifiedHeIndex, m_halfedge.getPreviousIndex());
}

TEST_F(TestHalfedge, getOpposite)
{
    Halfedge_t oppHe = m_halfedge.getOpposite();
    const auto cHe = m_halfedge;
    Halfedge_t cOppHe = cHe.getOpposite();
    EXPECT_EQ(oppHe, cOppHe);
}

TEST_F(TestHalfedge, setOppositeIndex)
{
    HalfedgeIndex_t oppIdx = m_halfedge.getOppositeIndex();
    HalfedgeIndex_t modifiedHeIndex = HalfedgeIndex_t{oppIdx.get_value() + 1};
    m_halfedge.setOppositeIndex(modifiedHeIndex);
    EXPECT_EQ(modifiedHeIndex, m_halfedge.getOppositeIndex());
}

TEST_F(TestHalfedge, getVertex)
{
    const Vertex_t& vertex = m_halfedge.getVertex();
    const auto cHe = m_halfedge;
    const Vertex_t& cVertex = cHe.getVertex();
    EXPECT_EQ(vertex, cVertex);
}

TEST_F(TestHalfedge, setVertexIndex)
{
    VertexIndex_t vertexIdx = m_halfedge.getVertexIndex();
    VertexIndex_t modifiedIndex = VertexIndex_t{vertexIdx.get_value() + 1};
    m_halfedge.setVertexIndex(modifiedIndex);
    EXPECT_EQ(modifiedIndex, m_halfedge.getVertexIndex());
}

TEST_F(TestHalfedge, getNextVertex)
{
    const Vertex_t& nextVertex = m_halfedge.getNextVertex();
    const auto cHe = m_halfedge;
    const Vertex_t& cNextVertex = cHe.getNextVertex();
    EXPECT_EQ(nextVertex, cNextVertex);
}
