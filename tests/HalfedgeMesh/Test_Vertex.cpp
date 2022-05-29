#include <Geometry/Cuboid.hpp>
#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <Geometry/HalfedgeMesh/Vertex.hpp>
#include <Geometry/HalfedgeMeshBuilder/CuboidMeshBuilder.hpp>
#include <gtest/gtest.h>

using namespace Geometry;
using namespace LinAl;

class VertexTest : public ::testing::Test {
  protected:
    VertexTest()
        : m_heMesh(CuboidMeshBuilder<float_t, std::size_t>().setCuboid(Cuboid<float_t>({0, 0, 0}, {1, 1, 1})).build())
        , m_vertex(m_heMesh->vertices.front())
    {
    }

    std::unique_ptr<HalfedgeMesh<float_t>> m_heMesh;
    HalfedgeMesh<float_t>::Vertex_t m_vertex;
};

TEST_F(VertexTest, isValid)
{
    EXPECT_TRUE(m_vertex.isValid());
}

TEST_F(VertexTest, getVector)
{
    [[maybe_unused]] Vec3f point = m_vertex.getVector();
    const auto& constVertex = m_vertex;
    [[maybe_unused]] const Vec3f& cPoint = constVertex.getVector();
}

TEST_F(VertexTest, vertexIndex)
{
    [[maybe_unused]] HalfedgeMesh<float_t>::VertexIndex_t index = m_vertex.getIndex();
    const auto& constVertex = m_vertex;
    index = constVertex.getIndex();
}

TEST_F(VertexTest, getHalfedge)
{
    [[maybe_unused]] HalfedgeMesh<float_t>::Halfedge_t halfedge = m_vertex.getHalfedge();
    const auto& constVertex = m_vertex;
    [[maybe_unused]] const HalfedgeMesh<float_t>::Halfedge_t& cHalfedge = constVertex.getHalfedge();
}

TEST_F(VertexTest, halfedgeIndex)
{
    [[maybe_unused]] HalfedgeMesh<float_t>::HalfedgeIndex_t heIndex = m_vertex.getHalfedgeIndex();
    m_vertex.setHalfedgeIndex(HalfedgeMesh<float_t>::HalfedgeIndex_t{0});
    EXPECT_EQ(0, m_vertex.getHalfedgeIndex().getValue());
}
