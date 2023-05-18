

#include <Geometry/Cuboid.hpp>
#include <Geometry/HalfedgeMesh/Facet.hpp>
#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <Geometry/HalfedgeMeshBuilder/CuboidMeshBuilder.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

class FacetTest : public ::testing::Test {
protected:
  FacetTest()
      : m_heMesh(CuboidMeshBuilder<float, std::size_t>().setCuboid(Cuboid<float>({0, 0, 0}, {1, 1, 1})).build())
      , m_facet(m_heMesh->getFacets().front())
  {
  }

  std::unique_ptr<HalfedgeMesh<float>> m_heMesh;
  HalfedgeMesh<float>::Facet_t m_facet;
};

TEST_F(FacetTest, getHalfedge)
{
  [[maybe_unused]] HalfedgeMesh<float>::Halfedge_t& he = m_facet.getHalfedge();
  const auto& constFacet = m_facet;
  [[maybe_unused]] const HalfedgeMesh<float>::Halfedge_t& cHe = constFacet.getHalfedge();
}

TEST_F(FacetTest, getHeIndex)
{
  [[maybe_unused]] HalfedgeMesh<float>::HalfedgeIndex_t heIndex = m_facet.getHalfedgeIndex();
  const auto& constFacet = m_facet;
  [[maybe_unused]] HalfedgeMesh<float>::HalfedgeIndex_t cHeIndex = constFacet.getHalfedgeIndex();
}