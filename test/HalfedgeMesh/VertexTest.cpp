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
        , m_vertex(m_heMesh->get_vertices().front())
    {
    }

    std::unique_ptr<HalfedgeMesh_t> m_heMesh;
    HalfedgeMesh_t::Vertex_t m_vertex;
};

TEST_F(VertexTest, is_valid)
{
    EXPECT_TRUE(m_vertex.is_valid());
}

TEST_F(VertexTest, get_vector)
{
    [[maybe_unused]] linal::float3 point = m_vertex.get_vector();
    const auto& constVertex = m_vertex;
    [[maybe_unused]] const linal::float3& cPoint = constVertex.get_vector();
}

TEST_F(VertexTest, vertexIndex)
{
    [[maybe_unused]] HalfedgeMesh_t::VertexIndex_t index = m_vertex.get_index();
    const auto& constVertex = m_vertex;
    index = constVertex.get_index();
}

TEST_F(VertexTest, get_halfedge)
{
    [[maybe_unused]] auto halfedges = m_vertex.calc_halfedges();
    const auto& constVertex = m_vertex;
    [[maybe_unused]] const auto& cHalfedges = constVertex.calc_halfedges();
}

TEST_F(VertexTest, halfedgeIndex)
{
    [[maybe_unused]] auto heIndices = m_vertex.get_halfedge_indices();
}
