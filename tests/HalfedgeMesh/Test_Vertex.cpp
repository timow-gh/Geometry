#include <Geometry/Cuboid.hpp>
#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <Geometry/HalfedgeMesh/Vertex.hpp>
#include <Geometry/HalfedgeMeshBuilder/CuboidMeshBuilder.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

class VertexTest : public ::testing::Test {
  protected:
    VertexTest()
        : m_heMesh(CuboidMeshBuilder<float, std::size_t>().set_cuboid(Cuboid<float>({0, 0, 0}, {1, 1, 1})).build())
        , m_vertex(m_heMesh->getVertices().front())
    {
    }

    std::unique_ptr<HalfedgeMesh<float>> m_heMesh;
    HalfedgeMesh<float>::Vertex_t m_vertex;
};

TEST_F(VertexTest, is_valid)
{
    EXPECT_TRUE(m_vertex.is_valid());
}

TEST_F(VertexTest, getVector)
{
    [[maybe_unused]] linal::vec3f point = m_vertex.getVector();
    const auto& constVertex = m_vertex;
    [[maybe_unused]] const linal::vec3f& cPoint = constVertex.getVector();
}

TEST_F(VertexTest, vertexIndex)
{
    [[maybe_unused]] HalfedgeMesh<float>::VertexIndex_t index = m_vertex.getIndex();
    const auto& constVertex = m_vertex;
    index = constVertex.getIndex();
}

TEST_F(VertexTest, getHalfedge)
{
    [[maybe_unused]] HalfedgeMesh<float>::Halfedge_t& halfedge = m_vertex.getHalfedge();
    const auto& constVertex = m_vertex;
    [[maybe_unused]] const HalfedgeMesh<float>::Halfedge_t& cHalfedge = constVertex.getHalfedge();
}

TEST_F(VertexTest, halfedgeIndex)
{
    [[maybe_unused]] HalfedgeMesh<float>::HalfedgeIndex_t heIndex = m_vertex.getHalfedgeIndex();
    m_vertex.setHalfedgeIndex(HalfedgeMesh<float>::HalfedgeIndex_t{0});
    EXPECT_EQ(0, m_vertex.getHalfedgeIndex().get_value());
}
