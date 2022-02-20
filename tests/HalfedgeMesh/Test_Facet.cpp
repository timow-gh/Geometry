

#include <Geometry/Cuboid.hpp>
#include <Geometry/HalfedgeMesh/Facet.hpp>
#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <Geometry/HalfedgeMeshBuilder/CuboidMeshBuilder.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

class FacetTest : public ::testing::Test {
  protected:
    FacetTest()
        : m_heMesh(
              CuboidMeshBuilder<float_t>().setCuboid(Cuboid<float_t>({0, 0, 0}, {1, 1, 1})).build())
        , m_facet(m_heMesh->getFacets().front())
    {
    }

    std::unique_ptr<HalfedgeMesh<float_t>> m_heMesh;
    Facet<float_t> m_facet;
};

TEST_F(FacetTest, getHalfedge)
{
    Halfedge<float_t>& he = m_facet.getHalfedge();
    const auto& constFacet = m_facet;
    const Halfedge<float_t>& cHe = constFacet.getHalfedge();
}

TEST_F(FacetTest, getHeIndex)
{
    std::size_t heIndex = m_facet.getHalfedgeIndex();
    const auto& constFacet = m_facet;
    std::size_t cHeIndex = constFacet.getHalfedgeIndex();
}