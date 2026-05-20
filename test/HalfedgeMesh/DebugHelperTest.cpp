#include "Geometry/Cuboid.hpp"
#include "Geometry/HalfedgeMesh/HalfedgeMesh.hpp"
#include "Geometry/HalfedgeMeshBuilder/MeshBuilder.hpp"
#include <Geometry/HalfedgeMesh/DebugHelper.hpp>
#include <gtest/gtest.h>
#include <ostream>

using namespace Geometry;

using HalfedgeMesh_t = HalfedgeMesh<MeshTraits<float, std::size_t>>;

class DebugHelperTest : public ::testing::Test {
protected:
};

TEST_F(DebugHelperTest, print_inline_linal_vec)
{
  std::ostringstream oss;
  print_inline(oss, linal::vec<float, 3>{1.0F, 2.0F, 3.0F});
  EXPECT_EQ(oss.str(), "vec: 1 2 3");
}

TEST_F(DebugHelperTest, print_linal_vec)
{
  std::ostringstream oss;
  print(oss, linal::vec<float, 3>{1.0F, 2.0F, 3.0F});
  EXPECT_EQ(oss.str(), "vec: 1 2 3\n");
}

TEST_F(DebugHelperTest, print_linal_vecs)
{
  std::vector<linal::vec<float, 3>> vecs = {linal::vec<float, 3>{1.0F, 2.0F, 3.0F}, linal::vec<float, 3>{4.0F, 5.0F, 6.0F}};
  std::ostringstream oss;
  print(oss, vecs);
  EXPECT_EQ(oss.str(), "vec: 1 2 3\nvec: 4 5 6\n");
}

TEST_F(DebugHelperTest, print_vertex)
{
  MeshBuilder<float, std::size_t> builder;
  auto mesh = builder.build(Cuboid<float>{linal::float3{1.0F, 2.0F, 3.0F}, linal::float3{4.0F, 5.0F, 6.0F}});
  std::ostringstream oss;
  print(oss, mesh->get_vertex(0));
  EXPECT_EQ(oss.str(), "Vertex index: 0; vec: 1 2 3\n");
}

TEST_F(DebugHelperTest, print_vertices)
{
  MeshBuilder<float, std::size_t> builder;
  auto mesh = builder.build(Cuboid<float>{linal::float3{1.0F, 2.0F, 3.0F}, linal::float3{4.0F, 5.0F, 6.0F}});

  using Vertex_t = Vertex<MeshTraits<float, std::size_t>>;

  std::ostringstream oss;
  std::vector<Vertex_t> vertices = {mesh->get_vertex(0), mesh->get_vertex(1)};
  print(oss, vertices);
  EXPECT_EQ(oss.str(), "Vertex index: 0; vec: 1 2 3\nVertex index: 1; vec: 1 2 9\n");
}

TEST_F(DebugHelperTest, print_inline_facet)
{
  MeshBuilder<float, std::size_t> builder;
  auto mesh = builder.build(Cuboid<float>{linal::float3{1.0F, 2.0F, 3.0F}, linal::float3{4.0F, 5.0F, 6.0F}});
  std::ostringstream oss;
  print_inline(oss, mesh->get_facet(0));
  EXPECT_EQ(oss.str(), "Facet halfedge index: 0");
}

TEST_F(DebugHelperTest, print_facet)
{
  MeshBuilder<float, std::size_t> builder;
  auto mesh = builder.build(Cuboid<float>{linal::float3{1.0F, 2.0F, 3.0F}, linal::float3{4.0F, 5.0F, 6.0F}});
  std::ostringstream oss;
  print(oss, mesh->get_facet(0));
  EXPECT_EQ(oss.str(), "Facet halfedge index: 0\n");
}

TEST_F(DebugHelperTest, print_facets)
{
  MeshBuilder<float, std::size_t> builder;
  auto mesh = builder.build(Cuboid<float>{linal::float3{1.0F, 2.0F, 3.0F}, linal::float3{4.0F, 5.0F, 6.0F}});

  using Facet_t = Facet<MeshTraits<float, std::size_t>>;

  std::ostringstream oss;
  std::vector<Facet_t> facets = {mesh->get_facet(0), mesh->get_facet(1)};
  print(oss, facets);
  EXPECT_EQ(oss.str(), "Facet halfedge index: 0\nFacet halfedge index: 1\n");
}

TEST_F(DebugHelperTest, print_halfedge)
{
  MeshBuilder<float, std::size_t> builder;
  auto mesh = builder.build(Cuboid<float>{linal::float3{1.0F, 2.0F, 3.0F}, linal::float3{4.0F, 5.0F, 6.0F}});
  std::ostringstream oss;
  print(oss, mesh->get_halfedge(0));
  EXPECT_EQ(oss.str(),
            "Halfedge index: 0; Halfedge vertex index: 0; Opposite halfedge index: 0; Vertex vector, vec: 1 2 3; Facet index: 0\n");
}

TEST_F(DebugHelperTest, print_halfedges)
{
  MeshBuilder<float, std::size_t> builder;
  auto mesh = builder.build(Cuboid<float>{linal::float3{1.0F, 2.0F, 3.0F}, linal::float3{4.0F, 5.0F, 6.0F}});

  using Halfedge_t = Halfedge<MeshTraits<float, std::size_t>>;

  std::ostringstream oss;
  std::vector<Halfedge_t> halfedges = {mesh->get_halfedge(0), mesh->get_halfedge(1)};
  print(oss, halfedges);
  EXPECT_EQ(oss.str(),
            "Halfedge index: 0; Halfedge vertex index: 0; Opposite halfedge index: 0; Vertex vector, vec: 1 2 3; Facet index: 0\nHalfedge "
            "index: 0; Halfedge vertex index: 1; Opposite halfedge index: 1; Vertex vector, vec: 1 2 9; Facet index: 0\n");
}
