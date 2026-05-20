#include <Geometry/Cuboid.hpp>
#include <Geometry/HalfedgeMesh/Facet.hpp>
#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <Geometry/HalfedgeMeshBuilder/CuboidMeshBuilder.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

using HalfedgeMesh_t = HalfedgeMesh<MeshTraits<float, std::size_t>>;

class FacetTest : public ::testing::Test {
protected:
  FacetTest()
      : m_heMesh(CuboidMeshBuilder<float, std::size_t>().set_cuboid(Cuboid<float>({0, 0, 0}, {1, 1, 1})).build())
      , m_facet(m_heMesh->get_facets().front())
  {
  }

  std::unique_ptr<HalfedgeMesh_t> m_heMesh;
  HalfedgeMesh_t::Facet_t m_facet;
};

TEST_F(FacetTest, get_halfedge)
{
  [[maybe_unused]] HalfedgeMesh_t::Halfedge_t& halfedge = m_facet.get_halfedge();
  const auto& constFacet = m_facet;
  [[maybe_unused]] const HalfedgeMesh_t::Halfedge_t& cHe = constFacet.get_halfedge();
}

TEST_F(FacetTest, getHeIndex)
{
  [[maybe_unused]] HalfedgeMesh_t::HalfedgeIndex_t heIndex = m_facet.get_halfedge_index();
  const auto& constFacet = m_facet;
  [[maybe_unused]] HalfedgeMesh_t::HalfedgeIndex_t cHeIndex = constFacet.get_halfedge_index();
}