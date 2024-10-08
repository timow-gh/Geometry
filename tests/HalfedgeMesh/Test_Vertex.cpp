#include <Geometry/Cuboid.hpp>
#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <Geometry/HalfedgeMesh/Vertex.hpp>
#include <Geometry/HalfedgeMeshBuilder/CuboidMeshBuilder.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

using HalfedgeMesh_t = HalfedgeMesh<MeshTraits<float, std::size_t>>;

class VertexTest : public ::testing::Test {
  protected:
    VertexTest()
        : m_heMesh(CuboidMeshBuilder<float, std::size_t>().set_cuboid(Cuboid<float>({0, 0, 0}, {1, 1, 1})).build())
        , m_vertex(m_heMesh->getVertices().front())
    {
    }

    std::unique_ptr<HalfedgeMesh_t> m_heMesh;
    HalfedgeMesh_t::Vertex_t m_vertex;
};

TEST_F(VertexTest, is_valid)
{
    EXPECT_TRUE(m_vertex.is_valid());
}

TEST_F(VertexTest, getVector)
{
    [[maybe_unused]] linal::float3 point = m_vertex.getVector();
    const auto& constVertex = m_vertex;
    [[maybe_unused]] const linal::float3& cPoint = constVertex.getVector();
}

TEST_F(VertexTest, vertexIndex)
{
    [[maybe_unused]] HalfedgeMesh_t::VertexIndex_t index = m_vertex.getIndex();
    const auto& constVertex = m_vertex;
    index = constVertex.getIndex();
}

TEST_F(VertexTest, getHalfedge)
{
    [[maybe_unused]] auto halfedges = m_vertex.calcHalfedges();
    const auto& constVertex = m_vertex;
    [[maybe_unused]] const auto& cHalfedges = constVertex.calcHalfedges();
}

TEST_F(VertexTest, halfedgeIndex)
{
    [[maybe_unused]] auto heIndices = m_vertex.getHalfedgeIndices();
}
