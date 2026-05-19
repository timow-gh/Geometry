#include <Geometry/Cuboid.hpp>
#include <Geometry/HalfedgeMesh/CalcTriangleIndices.hpp>
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

class TestTriangleIndices : public ::testing::Test {
protected:
  TestTriangleIndices()
      : m_heMesh(CuboidMeshBuilder<float, std::size_t>().set_cuboid(Cuboid<float>({0, 0, 0}, {1, 1, 1})).build())
      , m_halfedge(m_heMesh->getHalfedges().front())
  {
  }

  std::unique_ptr<HalfedgeMesh_t> m_heMesh;
  Halfedge_t m_halfedge;
};

TEST_F(TestTriangleIndices, calc_triangle_indices)
{
  Geometry::CuboidMeshBuilder<float, std::size_t> cuboidMeshBuilder;
  std::unique_ptr<HalfedgeMesh_t> cuboidMesh = cuboidMeshBuilder.set_origin({1.0F, -1.0F, 0.0F}).set_diagonal({1.0F, 1.0F, 1.0F}).build();
  std::vector<std::uint32_t> indices = Geometry::calc_triangle_indices<std::uint32_t>(cuboidMesh->getFacets());
  EXPECT_EQ(indices.size(), 36);
}
