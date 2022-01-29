#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include <Geometry/Cuboid.hpp>
#include <Geometry/HalfedgeMesh/CalcFaceHalfedges.hpp>
#include <Geometry/HalfedgeMesh/Facet.hpp>
#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <Geometry/HalfedgeMeshBuilder/CuboidMeshBuilder.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

class TestHalfedge : public ::testing::Test {
  protected:
    TestHalfedge()
        : m_heMesh(
              CuboidMeshBuilder<float_t>().setCuboid(Cuboid<float_t>({0, 0, 0}, {1, 1, 1})).build())
        , m_halfedge(m_heMesh->getHalfedges().front())
    {
    }

    std::unique_ptr<HalfedgeMesh<float_t>> m_heMesh;
    Halfedge<float_t> m_halfedge;
};

TEST_F(TestHalfedge, getFacet)
{
    Facet<float_t> facet = m_halfedge.getFacet();
    EXPECT_NE(facet.getHalfedgeIndex(), INVALID_INDEX);
}

TEST_F(TestHalfedge, getFacetIndex)
{
    std::size_t facetIndex = m_halfedge.getFacetIndex();
    EXPECT_NE(facetIndex, INVALID_INDEX);
}

TEST_F(TestHalfedge, setFacetIndex)
{
    std::size_t facetIndex = m_halfedge.getFacetIndex();
    m_halfedge.setFacetIndex(facetIndex + 1);
    EXPECT_NE(m_halfedge.getFacetIndex(), facetIndex);
}

TEST_F(TestHalfedge, getNext)
{
    const Halfedge<float_t>& nextHe = m_halfedge.getNext();
    const auto cHe = m_halfedge;
    const Halfedge<float_t>& cNextHe = cHe.getNext();
    EXPECT_EQ(nextHe, cNextHe);
}

TEST_F(TestHalfedge, setNextIndex)
{
    std::size_t nextHeIndex = m_halfedge.getNextIndex();
    m_halfedge.setNextIndex(++nextHeIndex);
    EXPECT_EQ(nextHeIndex, m_halfedge.getNextIndex());
}

TEST_F(TestHalfedge, getPrevious)
{
    const Halfedge<float_t>& nextHe = m_halfedge.getPrevious();
    const auto cHe = m_halfedge;
    const Halfedge<float_t>& cNextHe = cHe.getPrevious();
    EXPECT_EQ(nextHe, cNextHe);
}

TEST_F(TestHalfedge, setPreviousIndex)
{
    std::size_t prevIndex = m_halfedge.getPreviousIndex();
    m_halfedge.setPreviousIndex(++prevIndex);
    EXPECT_EQ(prevIndex, m_halfedge.getPreviousIndex());
}

TEST_F(TestHalfedge, getOpposite)
{
    const Halfedge<float_t>& oppHe = m_halfedge.getOpposite();
    const auto cHe = m_halfedge;
    const Halfedge<float_t>& cOppHe = cHe.getOpposite();
    EXPECT_EQ(oppHe, cOppHe);
}

TEST_F(TestHalfedge, setOppositeIndex)
{
    std::size_t oppIdx = m_halfedge.getOppositeIndex();
    m_halfedge.setOppositeIndex(++oppIdx);
    EXPECT_EQ(oppIdx, m_halfedge.getOppositeIndex());
}

TEST_F(TestHalfedge, getVertex)
{
    const Vertex<float_t>& vertex = m_halfedge.getVertex();
    const auto cHe = m_halfedge;
    const Vertex<float_t>& cVertex = cHe.getVertex();
    EXPECT_EQ(vertex, cVertex);
}

TEST_F(TestHalfedge, setVertexIndex)
{
    std::size_t vertexIdx = m_halfedge.getVertexIndex();
    m_halfedge.setVertexIndex(++vertexIdx);
    EXPECT_EQ(vertexIdx, m_halfedge.getVertexIndex());
}

TEST_F(TestHalfedge, getNextVertex)
{
    const Vertex<float_t>& nextVertex = m_halfedge.getNextVertex();
    const auto cHe = m_halfedge;
    const Vertex<float_t>& cNextVertex = cHe.getNextVertex();
    EXPECT_EQ(nextVertex, cNextVertex);
}
