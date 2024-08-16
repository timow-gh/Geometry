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
      , m_facet(m_heMesh->getFacets().front())
  {
  }

  std::unique_ptr<HalfedgeMesh_t> m_heMesh;
  HalfedgeMesh_t::Facet_t m_facet;
};

TEST_F(FacetTest, getHalfedge)
{
  [[maybe_unused]] HalfedgeMesh_t::Halfedge_t& he = m_facet.getHalfedge();
  const auto& constFacet = m_facet;
  [[maybe_unused]] const HalfedgeMesh_t::Halfedge_t& cHe = constFacet.getHalfedge();
}

TEST_F(FacetTest, getHeIndex)
{
  [[maybe_unused]] HalfedgeMesh_t::HalfedgeIndex_t heIndex = m_facet.getHalfedgeIndex();
  const auto& constFacet = m_facet;
  [[maybe_unused]] HalfedgeMesh_t::HalfedgeIndex_t cHeIndex = constFacet.getHalfedgeIndex();
}