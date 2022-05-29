

#include <Geometry/Cuboid.hpp>
#include <Geometry/HalfedgeMesh/Facet.hpp>
#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <Geometry/HalfedgeMeshBuilder/CuboidMeshBuilder.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

class FacetTest : public ::testing::Test {
  protected:
    FacetTest()
        : m_heMesh(CuboidMeshBuilder<float_t>().setCuboid(Cuboid<float_t>({0, 0, 0}, {1, 1, 1})).build())
        , m_facet(m_heMesh->facets.front())
    {
    }

    std::unique_ptr<HalfedgeMesh<float_t>> m_heMesh;
    HalfedgeMesh<float_t>::Facet_t m_facet;
};

TEST_F(FacetTest, getHalfedge)
{
    [[maybe_unused]] HalfedgeMesh<float_t>::Halfedge_t& he = m_facet.getHalfedge();
    const auto& constFacet = m_facet;
    [[maybe_unused]] const HalfedgeMesh<float_t>::Halfedge_t& cHe = constFacet.getHalfedge();
}

TEST_F(FacetTest, getHeIndex)
{
    [[maybe_unused]] HalfedgeMesh<float_t>::HalfedgeIndex_t heIndex = m_facet.getHalfedgeIndex();
    const auto& constFacet = m_facet;
    [[maybe_unused]] HalfedgeMesh<float_t>::HalfedgeIndex_t cHeIndex = constFacet.getHalfedgeIndex();
}