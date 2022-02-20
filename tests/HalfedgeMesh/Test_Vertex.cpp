

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
        : m_heMesh(
              CuboidMeshBuilder<float_t>().setCuboid(Cuboid<float_t>({0, 0, 0}, {1, 1, 1})).build())
        , m_vertex(m_heMesh->getVertices().front())
    {
    }

    std::unique_ptr<HalfedgeMesh<float_t>> m_heMesh;
    Vertex<float_t> m_vertex;
};

TEST_F(VertexTest, getPoint)
{
    Vec3f point = m_vertex.getPoint();
    const auto& constVertex = m_vertex;
    const Vec3f& cPoint = constVertex.getPoint();
}

TEST_F(VertexTest, vertexIndex)
{
    std::size_t index = m_vertex.getIndex();
    const auto& constVertex = m_vertex;
    index = constVertex.getIndex();
}

TEST_F(VertexTest, getHalfedge)
{
    Halfedge<float_t> halfedge = m_vertex.getHalfedge();
    const auto& constVertex = m_vertex;
    const Halfedge<float_t>& cHalfedge = constVertex.getHalfedge();
}

TEST_F(VertexTest, halfedgeIndex)
{
    std::size_t heIndex = m_vertex.getHalfedgeIndex();
    m_vertex.setHalfedgeIndex(0);
    EXPECT_EQ(0, m_vertex.getHalfedgeIndex());
}

TEST_F(VertexTest, isValid)
{
    EXPECT_TRUE(m_vertex.isValid());
}
