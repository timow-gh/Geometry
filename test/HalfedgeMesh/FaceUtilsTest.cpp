#include <Geometry/Cuboid.hpp>
#include <Geometry/HalfedgeMesh/CalcFaceHalfedges.hpp>
#include <Geometry/HalfedgeMesh/Facet.hpp>
#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <Geometry/HalfedgeMeshBuilder/CuboidMeshBuilder.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

using HalfedgeMesh_t = HalfedgeMesh<MeshTraits<float, std::size_t>>;

class FaceUtilsTest : public ::testing::Test {
protected:
  FaceUtilsTest()
      : m_heMesh(CuboidMeshBuilder<float, std::size_t>().set_cuboid(Cuboid<float>({0, 0, 0}, {1, 1, 1})).build())
  {
  }

  std::unique_ptr<HalfedgeMesh_t> m_heMesh;
};

TEST_F(FaceUtilsTest, calc_halfedges)
{
  std::vector<HalfedgeMesh_t::Halfedge_t> halfedges = calc_halfedges(m_heMesh->getFacets().front());
  EXPECT_EQ(halfedges.size(), 3);
}
