#include <Geometry/Cylinder.hpp>
#include <Geometry/Mesh/AddTriangle.hpp>
#include <Geometry/Mesh/MakeTriangleMesh.hpp>
#include <Geometry/Mesh/MeshEdgeCollapseChecks.hpp>
#include <Geometry/Mesh/MeshEuler.hpp>
#include <Geometry/Mesh/MeshManifold.hpp>
#include <Geometry/Mesh/MeshOrientation.hpp>
#include <Geometry/Mesh/MeshTopology.hpp>
#include <Geometry/Mesh/MeshVertexRemoval.hpp>
#include <Geometry/Mesh/TriangleHalfedgeMesh.hpp>
#include <Geometry/Mesh/detail/FaceGeometry.hpp>
#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <random>
#include <utility>
#include <vector>

using namespace Geometry;

namespace
{

using Mesh = TriangleHalfedgeMesh3d;
using VertexHandle = Mesh::VertexHandle;
using FaceHandle = Mesh::FaceHandle;
using Position = Mesh::vec_t;

// n x n grid in the xy-plane as in MeshEdgeCollapseTest.cpp; vertex (i, j) has handle value j * n + i.
// Each vertex is displaced by up to +-jitter in x and y; jitter <= 0.1 keeps every triangle
// counter-clockwise.
Mesh make_grid(std::size_t size, double jitter = 0.0, std::uint32_t seed = 0)
{
  std::mt19937 generator(seed);
  std::uniform_real_distribution<double> offset(-jitter, jitter);

  Mesh mesh;
  std::vector<VertexHandle> vertices;
  for (std::size_t j = 0; j < size; ++j)
  {
    for (std::size_t i = 0; i < size; ++i)
    {
      const double x = static_cast<double>(i) + offset(generator);
      const double y = static_cast<double>(j) + offset(generator);
      vertices.push_back(mesh.add_vertex({x, y, 0.0}));
    }
  }
  const auto at = [&](std::size_t i, std::size_t j) { return vertices[j * size + i]; };
  // Every triangle must attach along an existing edge (add_triangle refuses a second fan at a
  // corner): row 0 grows through each cell's upper triangle first, later rows through the lower.
  for (std::size_t j = 0; j + 1 < size; ++j)
  {
    for (std::size_t i = 0; i + 1 < size; ++i)
    {
      const std::array<VertexHandle, 3> lower{at(i, j), at(i + 1, j), at(i + 1, j + 1)};
      const std::array<VertexHandle, 3> upper{at(i, j), at(i + 1, j + 1), at(i, j + 1)};
      for (const auto& triangle : j == 0 ? std::array{upper, lower} : std::array{lower, upper})
      {
        EXPECT_TRUE(add_triangle(mesh, triangle).is_valid());
      }
    }
  }
  return mesh;
}

// Planar fan: an interior center vertex (handle 0) surrounded by a counter-clockwise boundary ring
// (handles 1..n).
Mesh make_fan(const std::vector<Position>& ring)
{
  Mesh mesh;
  const VertexHandle center = mesh.add_vertex({0.0, 0.0, 0.0});
  std::vector<VertexHandle> ringVertices;
  for (const Position& position : ring)
  {
    ringVertices.push_back(mesh.add_vertex(position));
  }
  for (std::size_t i = 0; i < ringVertices.size(); ++i)
  {
    EXPECT_TRUE(add_triangle(mesh, center, ringVertices[i], ringVertices[(i + 1) % ringVertices.size()]).is_valid());
  }
  return mesh;
}

// Eight-pointed star whose shortest spoke (to (0.7, 0)) points at a spike: merging the center into
// it folds the face over the notch at (0.5, -0.5). The notches (length ~0.707) are all safe targets.
const std::vector<Position> starRing{{0.7, 0.0, 0.0},   {0.5, 0.5, 0.0},  {0.0, 3.0, 0.0},  {-0.5, 0.5, 0.0},
                                     {-3.0, 0.0, 0.0},  {-0.5, -0.5, 0.0}, {0.0, -3.0, 0.0}, {0.5, -0.5, 0.0}};

// Twisted pinwheel whose kernel contains no ring vertex, so merging the center into any neighbour
// folds some face over (found by exhaustive search, see the collapse_inverts_faces checks below).
const std::vector<Position> pinwheelRing{{1.9, 2.3, 0.0},  {0.5, 0.9, 0.0},   {-3.0, 0.4, 0.0},
                                         {-1.0, 0.0, 0.0}, {1.1, -2.8, 0.0}, {0.5, -0.9, 0.0}};

struct ElementCounts
{
  std::size_t vertices{};
  std::size_t edges{};
  std::size_t faces{};

  bool operator==(const ElementCounts&) const = default;
};

ElementCounts counts_of(const Mesh& mesh)
{
  return {mesh.vertex_count(), mesh.edge_count(), mesh.face_count()};
}

void expect_structurally_valid(const Mesh& mesh)
{
  EXPECT_TRUE(mesh.has_valid_connectivity());
  EXPECT_TRUE(verify_manifold(mesh));
  EXPECT_TRUE(is_consistently_oriented(mesh));
}

// For meshes in the xy-plane: every face still faces +z, i.e. nothing folded over or flattened.
bool all_faces_face_up(const Mesh& mesh)
{
  for (const FaceHandle face : mesh.faces())
  {
    if (detail::mesh_face_area_vector(mesh, face)[2] <= 0.0)
    {
      return false;
    }
  }
  return true;
}

Mesh make_tetrahedron()
{
  Mesh mesh;
  const VertexHandle vertex0 = mesh.add_vertex({0.0, 0.0, 0.0});
  const VertexHandle vertex1 = mesh.add_vertex({1.0, 0.0, 0.0});
  const VertexHandle vertex2 = mesh.add_vertex({0.0, 1.0, 0.0});
  const VertexHandle vertex3 = mesh.add_vertex({0.0, 0.0, 1.0});
  EXPECT_TRUE(add_triangle(mesh, vertex0, vertex2, vertex1).is_valid());
  EXPECT_TRUE(add_triangle(mesh, vertex0, vertex1, vertex3).is_valid());
  EXPECT_TRUE(add_triangle(mesh, vertex1, vertex2, vertex3).is_valid());
  EXPECT_TRUE(add_triangle(mesh, vertex2, vertex0, vertex3).is_valid());
  return mesh;
}

} // namespace

TEST(MeshVertexRemoval, RejectsInvalidAndDeletedVertices)
{
  Mesh mesh = make_grid(3);
  EXPECT_EQ(remove_vertex(mesh, VertexHandle{}).status, CollapseStatus::InvalidHandle);

  ASSERT_TRUE(remove_vertex(mesh, VertexHandle{4}).has_value());
  const auto again = remove_vertex(mesh, VertexHandle{4});
  EXPECT_EQ(again.status, CollapseStatus::InvalidHandle);
  EXPECT_FALSE(again.survivor.is_valid());
}

TEST(MeshVertexRemoval, DeletesIsolatedVertex)
{
  Mesh mesh = make_grid(3);
  const VertexHandle isolated = mesh.add_vertex({5.0, 5.0, 0.0});

  const auto result = remove_vertex(mesh, isolated);

  EXPECT_TRUE(result.has_value());
  EXPECT_FALSE(result.survivor.is_valid());
  EXPECT_TRUE(mesh.is_deleted(isolated));
  EXPECT_EQ(mesh.vertex_count(), 9U);
  expect_structurally_valid(mesh);
}

TEST(MeshVertexRemoval, CollapsesInteriorVertexIntoNearestNeighbour)
{
  Mesh mesh = make_grid(3);
  // Pull (0, 1) towards the center so it is the unique nearest neighbour.
  const VertexHandle nearest{3};
  mesh.set_position(nearest, {0.5, 1.0, 0.0});
  const ElementCounts before = counts_of(mesh);

  const auto result = remove_vertex(mesh, VertexHandle{4});

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.survivor, nearest);
  EXPECT_EQ(mesh.get_position(nearest), (Position{0.5, 1.0, 0.0}));
  EXPECT_TRUE(mesh.is_deleted(VertexHandle{4}));
  EXPECT_EQ(counts_of(mesh), (ElementCounts{before.vertices - 1, before.edges - 3, before.faces - 2}));
  EXPECT_TRUE(all_faces_face_up(mesh));
  expect_structurally_valid(mesh);
}

TEST(MeshVertexRemoval, RemovesBoundaryCorner)
{
  Mesh mesh = make_grid(3);
  const ElementCounts before = counts_of(mesh);

  const auto result = remove_vertex(mesh, VertexHandle{0});

  ASSERT_TRUE(result.has_value());
  // The corner's shortest legal spokes are the two boundary edges; each takes one face with it.
  EXPECT_TRUE(result.survivor == VertexHandle{1} || result.survivor == VertexHandle{3});
  EXPECT_EQ(counts_of(mesh), (ElementCounts{before.vertices - 1, before.edges - 2, before.faces - 1}));
  EXPECT_EQ(boundary_loops(mesh).size(), 1U);
  EXPECT_TRUE(all_faces_face_up(mesh));
  expect_structurally_valid(mesh);
}

TEST(MeshVertexRemoval, SkipsNearestNeighbourWhoseCollapseFoldsOver)
{
  Mesh mesh = make_fan(starRing);
  const VertexHandle center{0};
  const VertexHandle spike{1};
  ASSERT_TRUE(collapse_inverts_faces(mesh, mesh.find_halfedge(center, spike), mesh.get_position(spike)));

  const auto result = remove_vertex(mesh, center);

  ASSERT_TRUE(result.has_value());
  // The next-nearest spokes are the two notches beside the spike, at equal distance.
  EXPECT_TRUE(result.survivor == VertexHandle{2} || result.survivor == VertexHandle{8});
  EXPECT_TRUE(all_faces_face_up(mesh));
  expect_structurally_valid(mesh);
}

TEST(MeshVertexRemoval, ReportsFoldOverWhenEveryLegalCollapseFolds)
{
  Mesh mesh = make_fan(pinwheelRing);
  const VertexHandle center{0};
  for (std::uint32_t ring = 1; ring <= pinwheelRing.size(); ++ring)
  {
    const auto halfedge = mesh.find_halfedge(center, VertexHandle{ring});
    ASSERT_EQ(is_collapse_ok(mesh, halfedge), CollapseStatus::Ok);
    ASSERT_TRUE(collapse_inverts_faces(mesh, halfedge, mesh.get_position(VertexHandle{ring})));
  }
  const ElementCounts before = counts_of(mesh);

  const auto result = remove_vertex(mesh, center);

  EXPECT_EQ(result.status, CollapseStatus::InvertsFaces);
  EXPECT_FALSE(result.survivor.is_valid());
  EXPECT_EQ(counts_of(mesh), before);
  EXPECT_FALSE(mesh.has_garbage());
}

TEST(MeshVertexRemoval, ReportsTopologicalReasonWhenNoCollapseIsLegal)
{
  Mesh tetrahedron = make_tetrahedron();
  EXPECT_EQ(remove_vertex(tetrahedron, VertexHandle{0}).status, CollapseStatus::Tetrahedron);
  EXPECT_FALSE(tetrahedron.has_garbage());

  Mesh triangle;
  const VertexHandle vertex0 = triangle.add_vertex({0.0, 0.0, 0.0});
  const VertexHandle vertex1 = triangle.add_vertex({1.0, 0.0, 0.0});
  const VertexHandle vertex2 = triangle.add_vertex({0.0, 1.0, 0.0});
  ASSERT_TRUE(add_triangle(triangle, vertex0, vertex1, vertex2).is_valid());
  EXPECT_EQ(remove_vertex(triangle, vertex0).status, CollapseStatus::IsolatedTriangle);
  EXPECT_FALSE(triangle.has_garbage());
}

TEST(MeshVertexRemovalFuzz, PlanarDecimationNeverFoldsOver)
{
  for (std::uint32_t seed = 1; seed <= 5; ++seed)
  {
    Mesh mesh = make_grid(8, 0.1, seed);
    ASSERT_TRUE(all_faces_face_up(mesh));
    std::mt19937 generator(seed);
    std::size_t removedCount = 0;

    for (int attempt = 0; attempt < 200; ++attempt)
    {
      const std::vector<VertexHandle> live(mesh.vertices().begin(), mesh.vertices().end());
      std::uniform_int_distribution<std::size_t> pick(0, live.size() - 1);
      const ElementCounts before = counts_of(mesh);

      const auto result = remove_vertex(mesh, live[pick(generator)]);
      if (!result.has_value())
      {
        EXPECT_EQ(counts_of(mesh), before);
        continue;
      }

      ++removedCount;
      ASSERT_EQ(mesh.vertex_count(), before.vertices - 1);
      ASSERT_EQ(euler_characteristic(mesh), 1);
      ASSERT_TRUE(all_faces_face_up(mesh)) << "seed " << seed << ", attempt " << attempt;
      ASSERT_TRUE(mesh.has_valid_connectivity());
      ASSERT_TRUE(verify_manifold(mesh));
    }

    EXPECT_GT(removedCount, 30U) << "seed " << seed;
    mesh.garbage_collection();
    expect_structurally_valid(mesh);
    EXPECT_TRUE(all_faces_face_up(mesh));
  }
}

TEST(MeshVertexRemovalFuzz, ClosedSurfaceDecimationKeepsTopology)
{
  const Cylinder<double> cylinder{Segment3<double>{{0.0, 0.0, 0.0}, {0.0, 0.0, 2.0}}, 1.0};
  auto creation = make_triangle_mesh(cylinder, 16);
  ASSERT_TRUE(creation.has_value());
  Mesh mesh = std::move(creation.mesh);
  std::mt19937 generator(7);

  for (int attempt = 0; attempt < 100 && mesh.vertex_count() > 4; ++attempt)
  {
    const std::vector<VertexHandle> live(mesh.vertices().begin(), mesh.vertices().end());
    std::uniform_int_distribution<std::size_t> pick(0, live.size() - 1);
    if (!remove_vertex(mesh, live[pick(generator)]).has_value())
    {
      continue;
    }
    ASSERT_EQ(euler_characteristic(mesh), 2);
    ASSERT_TRUE(verify_closed(mesh));
    ASSERT_TRUE(mesh.has_valid_connectivity());
    ASSERT_TRUE(verify_manifold(mesh));
    ASSERT_TRUE(is_consistently_oriented(mesh));
  }
  EXPECT_LT(mesh.vertex_count(), 20U);
}

// --- remove_vertex_retriangulate --------------------------------------------------------------

TEST(MeshVertexRetriangulation, RejectsInvalidAndDeletesIsolatedVertex)
{
  Mesh mesh = make_grid(3);
  EXPECT_EQ(remove_vertex_retriangulate(mesh, VertexHandle{}), VertexRemovalStatus::InvalidHandle);

  const VertexHandle isolated = mesh.add_vertex({5.0, 5.0, 0.0});
  EXPECT_EQ(remove_vertex_retriangulate(mesh, isolated), VertexRemovalStatus::Ok);
  EXPECT_TRUE(mesh.is_deleted(isolated));
  EXPECT_EQ(remove_vertex_retriangulate(mesh, isolated), VertexRemovalStatus::InvalidHandle);
}

TEST(MeshVertexRetriangulation, RemovesInteriorVertex)
{
  Mesh mesh = make_grid(3);
  const ElementCounts before = counts_of(mesh);

  ASSERT_EQ(remove_vertex_retriangulate(mesh, VertexHandle{4}), VertexRemovalStatus::Ok);

  EXPECT_TRUE(mesh.is_deleted(VertexHandle{4}));
  EXPECT_EQ(counts_of(mesh), (ElementCounts{before.vertices - 1, before.edges - 3, before.faces - 2}));
  EXPECT_EQ(euler_characteristic(mesh), 1);
  EXPECT_EQ(boundary_loops(mesh).front().size(), 8U);
  EXPECT_TRUE(all_faces_face_up(mesh));
  expect_structurally_valid(mesh);
  mesh.garbage_collection();
  expect_structurally_valid(mesh);
}

TEST(MeshVertexRetriangulation, PrefersWellShapedTriangles)
{
  // Rhombus ring: splitting along the short diagonal gives two fat triangles (cost 7), the long one
  // two slivers (cost 13).
  Mesh mesh = make_fan({{2.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {-2.0, 0.0, 0.0}, {0.0, -1.0, 0.0}});

  ASSERT_EQ(remove_vertex_retriangulate(mesh, VertexHandle{0}), VertexRemovalStatus::Ok);

  EXPECT_TRUE(mesh.find_halfedge(VertexHandle{2}, VertexHandle{4}).is_valid());
  EXPECT_FALSE(mesh.find_halfedge(VertexHandle{1}, VertexHandle{3}).is_valid());
  EXPECT_EQ(mesh.face_count(), 2U);
  EXPECT_TRUE(all_faces_face_up(mesh));
  expect_structurally_valid(mesh);
}

TEST(MeshVertexRetriangulation, SucceedsWhereEveryFanFoldsOver)
{
  Mesh mesh = make_fan(pinwheelRing);
  ASSERT_EQ(remove_vertex(mesh, VertexHandle{0}).status, CollapseStatus::InvertsFaces);

  ASSERT_EQ(remove_vertex_retriangulate(mesh, VertexHandle{0}), VertexRemovalStatus::Ok);

  EXPECT_EQ(mesh.face_count(), pinwheelRing.size() - 2);
  EXPECT_EQ(mesh.edge_count(), 2 * pinwheelRing.size() - 3);
  EXPECT_TRUE(all_faces_face_up(mesh));
  expect_structurally_valid(mesh);
}

TEST(MeshVertexRetriangulation, RemovesBoundaryVertex)
{
  Mesh mesh = make_grid(3);
  const ElementCounts before = counts_of(mesh);

  // (1, 0): the hole is closed by a new boundary edge (0,0)-(2,0).
  ASSERT_EQ(remove_vertex_retriangulate(mesh, VertexHandle{1}), VertexRemovalStatus::Ok);

  EXPECT_EQ(counts_of(mesh), (ElementCounts{before.vertices - 1, before.edges - 2, before.faces - 1}));
  EXPECT_TRUE(mesh.find_halfedge(VertexHandle{0}, VertexHandle{2}).is_valid());
  ASSERT_EQ(boundary_loops(mesh).size(), 1U);
  EXPECT_EQ(boundary_loops(mesh).front().size(), 7U);
  EXPECT_TRUE(all_faces_face_up(mesh));
  expect_structurally_valid(mesh);
}

TEST(MeshVertexRetriangulation, RemovesEarVertex)
{
  Mesh mesh = make_grid(3);
  const ElementCounts before = counts_of(mesh);

  // Corner (2, 0) has a single face; removing it just drops that face.
  ASSERT_EQ(remove_vertex_retriangulate(mesh, VertexHandle{2}), VertexRemovalStatus::Ok);

  EXPECT_EQ(counts_of(mesh), (ElementCounts{before.vertices - 1, before.edges - 2, before.faces - 1}));
  EXPECT_TRUE(mesh.is_boundary(mesh.get_halfedge(mesh.find_halfedge(VertexHandle{1}, VertexHandle{5})).edge));
  EXPECT_EQ(boundary_loops(mesh).front().size(), 7U);
  expect_structurally_valid(mesh);
}

TEST(MeshVertexRetriangulation, ReportsTopologicalObstructions)
{
  Mesh triangle;
  const VertexHandle vertex0 = triangle.add_vertex({0.0, 0.0, 0.0});
  const VertexHandle vertex1 = triangle.add_vertex({1.0, 0.0, 0.0});
  const VertexHandle vertex2 = triangle.add_vertex({0.0, 1.0, 0.0});
  ASSERT_TRUE(add_triangle(triangle, vertex0, vertex1, vertex2).is_valid());
  EXPECT_EQ(remove_vertex_retriangulate(triangle, vertex0), VertexRemovalStatus::IsolatedTriangle);
  EXPECT_FALSE(triangle.has_garbage());

  Mesh tetrahedron = make_tetrahedron();
  EXPECT_EQ(remove_vertex_retriangulate(tetrahedron, VertexHandle{0}), VertexRemovalStatus::Tetrahedron);
  EXPECT_FALSE(tetrahedron.has_garbage());

  // A two-face fan around a boundary vertex whose two boundary neighbours are already joined by a
  // back face: closing the hole would duplicate that edge.
  Mesh closedBack;
  const VertexHandle hub = closedBack.add_vertex({0.0, 0.0, 0.0});
  const VertexHandle right = closedBack.add_vertex({1.0, 0.0, 0.0});
  const VertexHandle top = closedBack.add_vertex({0.0, 1.0, 0.0});
  const VertexHandle left = closedBack.add_vertex({-1.0, 0.0, 0.0});
  ASSERT_TRUE(add_triangle(closedBack, hub, right, top).is_valid());
  ASSERT_TRUE(add_triangle(closedBack, hub, top, left).is_valid());
  ASSERT_TRUE(add_triangle(closedBack, left, top, right).is_valid());
  ASSERT_TRUE(is_boundary(closedBack, hub));
  EXPECT_EQ(remove_vertex_retriangulate(closedBack, hub), VertexRemovalStatus::DuplicateEdge);
  EXPECT_FALSE(closedBack.has_garbage());
}

TEST(MeshVertexRetriangulationFuzz, PlanarDecimationNeverFoldsOver)
{
  for (std::uint32_t seed = 1; seed <= 5; ++seed)
  {
    Mesh mesh = make_grid(8, 0.1, seed);
    std::mt19937 generator(seed);
    std::size_t removedCount = 0;

    for (int attempt = 0; attempt < 200 && mesh.face_count() > 1; ++attempt)
    {
      const std::vector<VertexHandle> live(mesh.vertices().begin(), mesh.vertices().end());
      std::uniform_int_distribution<std::size_t> pick(0, live.size() - 1);
      const VertexHandle vertex = live[pick(generator)];
      const bool boundary = is_boundary(mesh, vertex);
      const ElementCounts before = counts_of(mesh);

      if (remove_vertex_retriangulate(mesh, vertex) != VertexRemovalStatus::Ok)
      {
        ASSERT_EQ(counts_of(mesh), before);
        continue;
      }

      ++removedCount;
      const ElementCounts expected = boundary ? ElementCounts{before.vertices - 1, before.edges - 2, before.faces - 1}
                                              : ElementCounts{before.vertices - 1, before.edges - 3, before.faces - 2};
      ASSERT_EQ(counts_of(mesh), expected) << "seed " << seed << ", attempt " << attempt;
      ASSERT_TRUE(all_faces_face_up(mesh)) << "seed " << seed << ", attempt " << attempt;
      ASSERT_TRUE(mesh.has_valid_connectivity());
      ASSERT_TRUE(verify_manifold(mesh));
    }

    EXPECT_GT(removedCount, 40U) << "seed " << seed;
    mesh.garbage_collection();
    expect_structurally_valid(mesh);
    EXPECT_TRUE(all_faces_face_up(mesh));
  }
}

TEST(MeshVertexRetriangulationFuzz, ClosedSurfaceDecimationKeepsTopology)
{
  const Cylinder<double> cylinder{Segment3<double>{{0.0, 0.0, 0.0}, {0.0, 0.0, 2.0}}, 1.0};
  auto creation = make_triangle_mesh(cylinder, 16);
  ASSERT_TRUE(creation.has_value());
  Mesh mesh = std::move(creation.mesh);
  std::mt19937 generator(11);
  std::size_t removedCount = 0;

  for (int attempt = 0; attempt < 200 && mesh.vertex_count() > 4; ++attempt)
  {
    const std::vector<VertexHandle> live(mesh.vertices().begin(), mesh.vertices().end());
    std::uniform_int_distribution<std::size_t> pick(0, live.size() - 1);
    if (remove_vertex_retriangulate(mesh, live[pick(generator)]) != VertexRemovalStatus::Ok)
    {
      continue;
    }
    ++removedCount;
    ASSERT_EQ(euler_characteristic(mesh), 2);
    ASSERT_TRUE(verify_closed(mesh));
    ASSERT_TRUE(mesh.has_valid_connectivity());
    ASSERT_TRUE(verify_manifold(mesh));
    ASSERT_TRUE(is_consistently_oriented(mesh));
  }
  EXPECT_GT(removedCount, 15U);
}
