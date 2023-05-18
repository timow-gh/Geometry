#include <Geometry/Cuboid.hpp>
#include <Geometry/HalfedgeMesh/CalcFaceHalfedges.hpp>
#include <Geometry/HalfedgeMesh/Facet.hpp>
#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <Geometry/HalfedgeMeshBuilder/CuboidMeshBuilder.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

class FaceUtilsTest : public ::testing::Test {
  protected:
    FaceUtilsTest() : m_heMesh(CuboidMeshBuilder<float, std::size_t>().setCuboid(Cuboid<float>({0, 0, 0}, {1, 1, 1})).build()) {}

    std::unique_ptr<HalfedgeMesh<float>> m_heMesh;
};

TEST_F(FaceUtilsTest, calcHalfedges)
{
    Core::TVector<HalfedgeMesh<float, std::size_t>::Halfedge_t> halfedges = calcHalfedges(m_heMesh->getFacets().front());
    EXPECT_EQ(halfedges.size(), 3);
}
