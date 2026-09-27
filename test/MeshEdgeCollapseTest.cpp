#include <Geometry/Cylinder.hpp>
#include <Geometry/Mesh/AddTriangle.hpp>
#include <Geometry/Mesh/MakeTriangleMesh.hpp>
#include <Geometry/Mesh/MeshEdgeCollapse.hpp>
#include <Geometry/Mesh/MeshEdgeCollapseChecks.hpp>
#include <Geometry/Mesh/MeshEuler.hpp>
#include <Geometry/Mesh/MeshManifold.hpp>
#include <Geometry/Mesh/MeshOrientation.hpp>
#include <Geometry/Mesh/MeshTopology.hpp>
#include <Geometry/Mesh/TriangleHalfedgeMesh.hpp>
#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <numbers>
#include <random>
#include <utility>
#include <vector>

using namespace Geometry;

namespace
{

// 2D collapses live in MeshEdgeCollapse2DTest.cpp: instantiating the library templates for a second mesh
// type in this file would make their local Mesh/handle aliases hide these ones (MSVC C4459).
using Mesh = TriangleHalfedgeMesh3d;
using VertexHandle = Mesh::VertexHandle;
using HalfedgeHandle = Mesh::HalfedgeHandle;
using EdgeHandle = Mesh::EdgeHandle;
using FaceHandle = Mesh::FaceHandle;

// Regular n x n vertex grid in the xy-plane, each cell split along its (i,j)-(i+1,j+1) diagonal and
// wound counter-clockwise. Vertex (i, j) has handle value j * n + i. For n = 3 the center vertex 4
// is the only interior vertex (valence 6); every other vertex is on the single boundary loop.
Mesh make_grid(std::size_t size)
{
  Mesh mesh;
  std::vector<VertexHandle> vertices;
  for (std::size_t j = 0; j < size; ++j)
  {
    for (std::size_t i = 0; i < size; ++i)
    {
      vertices.push_back(mesh.add_vertex({static_cast<double>(i), static_cast<double>(j), 0.0}));
    }
  }
  const auto at = [&](std::size_t i, std::size_t j) { return vertices[j * size + i]; };
  // add_triangle rejects a triangle touching the mesh at a used corner through two new edges (a
  // second fan), so every triangle must attach along an existing edge. Row 0 grows through each
  // cell's upper triangle first (it shares the previous cell's right edge); later rows grow through
  // the lower triangle first (it shares the row below).
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

// Handle of grid vertex (i, j) in a 3 x 3 grid.
constexpr VertexHandle grid3(std::uint32_t i, std::uint32_t j)
{
  return VertexHandle{j * 3 + i};
}

Mesh make_single_triangle()
{
  Mesh mesh;
  const VertexHandle vertex0 = mesh.add_vertex({0.0, 0.0, 0.0});
  const VertexHandle vertex1 = mesh.add_vertex({1.0, 0.0, 0.0});
  const VertexHandle vertex2 = mesh.add_vertex({0.0, 1.0, 0.0});
  EXPECT_TRUE(add_triangle(mesh, vertex0, vertex1, vertex2).is_valid());
  return mesh;
}

// Two triangles sharing the interior diagonal (vertex1, vertex2); all four vertices are on the boundary.
Mesh make_two_adjacent_triangles()
{
  Mesh mesh;
  const VertexHandle vertex0 = mesh.add_vertex({0.0, 0.0, 0.0});
  const VertexHandle vertex1 = mesh.add_vertex({1.0, 0.0, 0.0});
  const VertexHandle vertex2 = mesh.add_vertex({0.0, 1.0, 0.0});
  const VertexHandle vertex3 = mesh.add_vertex({1.0, 1.0, 0.0});
  EXPECT_TRUE(add_triangle(mesh, vertex0, vertex1, vertex2).is_valid());
  EXPECT_TRUE(add_triangle(mesh, vertex2, vertex1, vertex3).is_valid());
  return mesh;
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

// Closed triangular bipyramid: equator ring a, b, c plus poles north and south. Every equator vertex
// neighbours the other two, so collapsing an equator edge violates the link condition (the third
// equator vertex is a common neighbour but not opposite the edge).
struct Bipyramid
{
  Mesh mesh;
  VertexHandle ringA{};
  VertexHandle ringB{};
  VertexHandle ringC{};
  VertexHandle north{};
  VertexHandle south{};
};

Bipyramid make_bipyramid()
{
  Bipyramid result;
  Mesh& mesh = result.mesh;
  result.ringA = mesh.add_vertex({1.0, 0.0, 0.0});
  result.ringB = mesh.add_vertex({-0.5, 0.8, 0.0});
  result.ringC = mesh.add_vertex({-0.5, -0.8, 0.0});
  result.north = mesh.add_vertex({0.0, 0.0, 1.0});
  result.south = mesh.add_vertex({0.0, 0.0, -1.0});
  const auto [ringA, ringB, ringC, north, south] =
      std::array<VertexHandle, 5>{result.ringA, result.ringB, result.ringC, result.north, result.south};
  EXPECT_TRUE(add_triangle(mesh, north, ringA, ringB).is_valid());
  EXPECT_TRUE(add_triangle(mesh, north, ringB, ringC).is_valid());
  EXPECT_TRUE(add_triangle(mesh, north, ringC, ringA).is_valid());
  EXPECT_TRUE(add_triangle(mesh, south, ringB, ringA).is_valid());
  EXPECT_TRUE(add_triangle(mesh, south, ringC, ringB).is_valid());
  EXPECT_TRUE(add_triangle(mesh, south, ringA, ringC).is_valid());
  return result;
}

Mesh make_closed_cylinder(std::size_t segments)
{
  const Cylinder<double> cylinder{Segment3<double>{{0.0, 0.0, 0.0}, {0.0, 0.0, 2.0}}, 1.0};
  auto result = make_triangle_mesh(cylinder, segments);
  EXPECT_TRUE(result.has_value());
  return std::move(result.mesh);
}

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

std::vector<HalfedgeHandle> legal_collapses(const Mesh& mesh)
{
  std::vector<HalfedgeHandle> result;
  for (const HalfedgeHandle halfedge : mesh.halfedges())
  {
    if (is_collapse_ok(mesh, halfedge) == CollapseStatus::Ok)
    {
      result.push_back(halfedge);
    }
  }
  return result;
}

} // namespace

// --- is_collapse_ok: one test per rejection reason -------------------------------------------

TEST(MeshEdgeCollapse, RejectsInvalidAndDeletedHalfedges)
{
  Mesh mesh = make_grid(3);
  EXPECT_EQ(is_collapse_ok(mesh, HalfedgeHandle{}), CollapseStatus::InvalidHandle);

  const HalfedgeHandle halfedge = mesh.find_halfedge(grid3(1, 1), grid3(2, 1));
  ASSERT_EQ(collapse_halfedge(mesh, halfedge), CollapseStatus::Ok);
  EXPECT_EQ(is_collapse_ok(mesh, halfedge), CollapseStatus::InvalidHandle);
  EXPECT_EQ(collapse_halfedge(mesh, halfedge), CollapseStatus::InvalidHandle);
}

TEST(MeshEdgeCollapse, RejectsEveryEdgeOfIsolatedTriangle)
{
  const Mesh mesh = make_single_triangle();
  for (const HalfedgeHandle halfedge : mesh.halfedges())
  {
    EXPECT_EQ(is_collapse_ok(mesh, halfedge), CollapseStatus::IsolatedTriangle);
  }
}

TEST(MeshEdgeCollapse, RejectsInteriorEdgeBetweenBoundaryVertices)
{
  // Fig. 7.4 (top): collapsing through the interior would pinch the boundary into one vertex.
  const Mesh twoTriangles = make_two_adjacent_triangles();
  const HalfedgeHandle diagonal = twoTriangles.find_halfedge(VertexHandle{1}, VertexHandle{2});
  EXPECT_EQ(is_collapse_ok(twoTriangles, diagonal), CollapseStatus::InteriorEdgeBetweenBoundaryVertices);

  const Mesh grid = make_grid(3);
  const HalfedgeHandle cellDiagonal = grid.find_halfedge(grid3(1, 0), grid3(2, 1));
  EXPECT_EQ(is_collapse_ok(grid, cellDiagonal), CollapseStatus::InteriorEdgeBetweenBoundaryVertices);
}

TEST(MeshEdgeCollapse, RejectsLinkConditionViolation)
{
  // Fig. 7.4 (bottom): the one-rings of the endpoints share a third, non-opposite vertex.
  const Bipyramid bipyramid = make_bipyramid();
  const HalfedgeHandle equatorEdge = bipyramid.mesh.find_halfedge(bipyramid.ringA, bipyramid.ringB);
  EXPECT_EQ(is_collapse_ok(bipyramid.mesh, equatorEdge), CollapseStatus::LinkCondition);
}

TEST(MeshEdgeCollapse, RejectsEveryEdgeOfTetrahedron)
{
  const Mesh mesh = make_tetrahedron();
  for (const HalfedgeHandle halfedge : mesh.halfedges())
  {
    EXPECT_EQ(is_collapse_ok(mesh, halfedge), CollapseStatus::Tetrahedron);
  }
}

TEST(MeshEdgeCollapse, VerdictIsSymmetricInDirection)
{
  const std::vector<Mesh> meshes{make_grid(4), make_bipyramid().mesh, make_tetrahedron(),
                                 make_two_adjacent_triangles(), make_closed_cylinder(6)};
  for (const Mesh& mesh : meshes)
  {
    for (const HalfedgeHandle halfedge : mesh.halfedges())
    {
      EXPECT_EQ(is_collapse_ok(mesh, halfedge), is_collapse_ok(mesh, mesh.get_halfedge(halfedge).twin));
    }
  }
}

TEST(MeshEdgeCollapse, RejectedCollapseLeavesMeshUntouched)
{
  Mesh mesh = make_two_adjacent_triangles();
  const HalfedgeHandle diagonal = mesh.find_halfedge(VertexHandle{1}, VertexHandle{2});
  const ElementCounts before = counts_of(mesh);

  EXPECT_EQ(collapse_halfedge(mesh, diagonal), CollapseStatus::InteriorEdgeBetweenBoundaryVertices);

  EXPECT_EQ(counts_of(mesh), before);
  EXPECT_FALSE(mesh.has_garbage());
  expect_structurally_valid(mesh);
}

// --- collapse_halfedge: legal configurations --------------------------------------------------

TEST(MeshEdgeCollapse, CollapsesInteriorVertexIntoBoundaryNeighbour)
{
  Mesh mesh = make_grid(3);
  const ElementCounts before = counts_of(mesh);
  const std::ptrdiff_t eulerBefore = euler_characteristic(mesh);
  const auto survivorPosition = mesh.get_position(grid3(2, 1));

  ASSERT_EQ(collapse_halfedge(mesh, mesh.find_halfedge(grid3(1, 1), grid3(2, 1))), CollapseStatus::Ok);

  EXPECT_EQ(counts_of(mesh), (ElementCounts{before.vertices - 1, before.edges - 3, before.faces - 2}));
  EXPECT_EQ(euler_characteristic(mesh), eulerBefore);
  EXPECT_TRUE(mesh.is_deleted(grid3(1, 1)));
  EXPECT_EQ(mesh.get_position(grid3(2, 1)), survivorPosition);
  // The survivor inherits the removed vertex's neighbours.
  EXPECT_TRUE(mesh.find_halfedge(grid3(2, 1), grid3(0, 0)).is_valid());
  EXPECT_TRUE(mesh.find_halfedge(grid3(2, 1), grid3(0, 1)).is_valid());
  EXPECT_TRUE(mesh.find_halfedge(grid3(2, 1), grid3(1, 2)).is_valid());
  EXPECT_EQ(boundary_loops(mesh).size(), 1U);
  expect_structurally_valid(mesh);
}

TEST(MeshEdgeCollapse, CollapsesBoundaryVertexIntoInteriorNeighbour)
{
  // Topologically legal: the connectivity equals the opposite direction's, only the survivor differs.
  Mesh mesh = make_grid(3);
  const ElementCounts before = counts_of(mesh);

  ASSERT_EQ(collapse_halfedge(mesh, mesh.find_halfedge(grid3(2, 1), grid3(1, 1))), CollapseStatus::Ok);

  EXPECT_EQ(counts_of(mesh), (ElementCounts{before.vertices - 1, before.edges - 3, before.faces - 2}));
  // The interior survivor takes the removed vertex's place on the boundary; the collapsed edge was
  // interior, so the boundary keeps all 8 of its edges.
  EXPECT_TRUE(is_boundary(mesh, grid3(1, 1)));
  ASSERT_EQ(boundary_loops(mesh).size(), 1U);
  EXPECT_EQ(boundary_loops(mesh).front().size(), 8U);
  expect_structurally_valid(mesh);
}

TEST(MeshEdgeCollapse, CollapsesBoundaryEdgeFromEitherSide)
{
  for (const bool fromBoundaryHalfedge : {false, true})
  {
    Mesh mesh = make_grid(3);
    const ElementCounts before = counts_of(mesh);
    const HalfedgeHandle interiorSide = mesh.find_halfedge(grid3(0, 0), grid3(1, 0));
    ASSERT_FALSE(mesh.is_boundary(interiorSide));
    const HalfedgeHandle halfedge = fromBoundaryHalfedge ? mesh.get_halfedge(interiorSide).twin : interiorSide;

    ASSERT_EQ(collapse_halfedge(mesh, halfedge), CollapseStatus::Ok);

    // A boundary edge has one face: 1 vertex, 2 edges, 1 face go, and the boundary loop shrinks by one.
    EXPECT_EQ(counts_of(mesh), (ElementCounts{before.vertices - 1, before.edges - 2, before.faces - 1}));
    EXPECT_EQ(euler_characteristic(mesh), 1);
    ASSERT_EQ(boundary_loops(mesh).size(), 1U);
    EXPECT_EQ(boundary_loops(mesh).front().size(), 7U);
    expect_structurally_valid(mesh);
  }
}

TEST(MeshEdgeCollapse, ClosedSurfaceCollapseKeepsItClosed)
{
  Bipyramid bipyramid = make_bipyramid();
  Mesh& mesh = bipyramid.mesh;

  ASSERT_EQ(collapse_halfedge(mesh, mesh.find_halfedge(bipyramid.north, bipyramid.ringA)), CollapseStatus::Ok);

  // The bipyramid loses a pole and becomes a tetrahedron, which admits no further collapse.
  EXPECT_EQ(counts_of(mesh), (ElementCounts{4, 6, 4}));
  EXPECT_TRUE(verify_closed(mesh));
  EXPECT_EQ(euler_characteristic(mesh), 2);
  expect_structurally_valid(mesh);
  for (const HalfedgeHandle halfedge : mesh.halfedges())
  {
    EXPECT_EQ(is_collapse_ok(mesh, halfedge), CollapseStatus::Tetrahedron);
  }
}

TEST(MeshEdgeCollapse, MergedEdgeKeepsCrease)
{
  Mesh mesh = make_grid(3);
  // (1,1) -> (2,1) removes edge (1,1)-(1,0) by gluing it onto (2,1)-(1,0).
  mesh.set_crease(mesh.get_halfedge(mesh.find_halfedge(grid3(1, 1), grid3(1, 0))).edge, true);

  ASSERT_EQ(collapse_halfedge(mesh, mesh.find_halfedge(grid3(1, 1), grid3(2, 1))), CollapseStatus::Ok);

  const HalfedgeHandle merged = mesh.find_halfedge(grid3(2, 1), grid3(1, 0));
  ASSERT_TRUE(merged.is_valid());
  EXPECT_TRUE(mesh.is_crease(mesh.get_halfedge(merged).edge));
  std::size_t creaseCount = 0;
  for (const EdgeHandle edge : mesh.edges())
  {
    creaseCount += mesh.is_crease(edge) ? 1U : 0U;
  }
  EXPECT_EQ(creaseCount, 1U);
}

TEST(MeshEdgeCollapse, GarbageCollectionAfterCollapses)
{
  Mesh mesh = make_grid(4);
  ASSERT_EQ(collapse_halfedge(mesh, mesh.find_halfedge(VertexHandle{5}, VertexHandle{6})), CollapseStatus::Ok);
  ASSERT_EQ(collapse_halfedge(mesh, mesh.find_halfedge(VertexHandle{10}, VertexHandle{6})), CollapseStatus::Ok);
  const ElementCounts before = counts_of(mesh);

  mesh.garbage_collection();

  EXPECT_FALSE(mesh.has_garbage());
  EXPECT_EQ(counts_of(mesh), before);
  EXPECT_EQ(mesh.vertex_storage_size(), before.vertices);
  expect_structurally_valid(mesh);
}

// --- collapse_edge ------------------------------------------------------------------------------

TEST(MeshEdgeCollapse, CollapseEdgePlacesSurvivorAtPosition)
{
  Mesh mesh = make_grid(3);
  const HalfedgeHandle halfedge = mesh.find_halfedge(grid3(1, 1), grid3(2, 1));
  const EdgeHandle edge = mesh.get_halfedge(halfedge).edge;
  const Mesh::vec_t midpoint{1.5, 1.0, 0.0};

  const auto result = collapse_edge(mesh, edge, midpoint);

  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result.survivor == grid3(1, 1) || result.survivor == grid3(2, 1));
  EXPECT_FALSE(mesh.is_deleted(result.survivor));
  EXPECT_NE(mesh.is_deleted(grid3(1, 1)), mesh.is_deleted(grid3(2, 1)));
  EXPECT_EQ(mesh.get_position(result.survivor), midpoint);
  EXPECT_TRUE(mesh.is_deleted(edge));
  expect_structurally_valid(mesh);
}

TEST(MeshEdgeCollapse, CollapseEdgeReportsRejection)
{
  Mesh mesh = make_tetrahedron();
  const auto rejected = collapse_edge(mesh, EdgeHandle{0}, Mesh::vec_t{});
  EXPECT_FALSE(rejected.has_value());
  EXPECT_EQ(rejected.status, CollapseStatus::Tetrahedron);
  EXPECT_FALSE(rejected.survivor.is_valid());

  const auto invalid = collapse_edge(mesh, EdgeHandle{}, Mesh::vec_t{});
  EXPECT_EQ(invalid.status, CollapseStatus::InvalidHandle);
}

// --- collapse_inverts_faces -----------------------------------------------------------------------

TEST(MeshEdgeCollapse, InversionCheckIn3D)
{
  const Mesh mesh = make_grid(3);
  const HalfedgeHandle halfedge = mesh.find_halfedge(grid3(1, 1), grid3(2, 1));

  EXPECT_FALSE(collapse_inverts_faces(mesh, halfedge, mesh.get_position(grid3(2, 1))));
  EXPECT_FALSE(collapse_inverts_faces(mesh, halfedge, Mesh::vec_t{1.5, 1.0, 0.0}));
  // Far outside the one-ring, fan triangles on the far side fold over.
  EXPECT_TRUE(collapse_inverts_faces(mesh, halfedge, Mesh::vec_t{-10.0, -10.0, 0.0}));
  // Merging onto a neighbour's position flattens the faces shared with that neighbour.
  EXPECT_TRUE(collapse_inverts_faces(mesh, halfedge, mesh.get_position(grid3(0, 1))));
}

// --- collapse_exceeds_geometry_limits -------------------------------------------------------------

namespace
{

struct FoldingCollapse
{
  Mesh mesh;
  HalfedgeHandle halfedge;
};

// Face (a, b, p) faces +z; face (b, a, c) hangs down across edge ab, about 101 degrees away. Moving
// p to q turns the first face by less than 90 degrees -- so it does not count as inverted -- but
// lays it into the plane of the second one, wound the other way: a complete fold. The returned
// halfedge is p -> q, whose collapse is topologically legal.
FoldingCollapse make_folding_collapse()
{
  FoldingCollapse result;
  Mesh& mesh = result.mesh;
  const VertexHandle cornerA = mesh.add_vertex({0.0, 0.0, 0.0});
  const VertexHandle cornerB = mesh.add_vertex({1.0, 0.0, 0.0});
  const VertexHandle cornerC = mesh.add_vertex({0.5, 0.2, -1.0});
  const VertexHandle removed = mesh.add_vertex({0.5, 1.0, 0.0});
  const VertexHandle survivor = mesh.add_vertex({0.5, 0.1, -0.5});
  EXPECT_TRUE(add_triangle(mesh, cornerA, cornerB, removed).is_valid());
  EXPECT_TRUE(add_triangle(mesh, cornerB, cornerA, cornerC).is_valid());
  EXPECT_TRUE(add_triangle(mesh, removed, cornerB, survivor).is_valid());
  EXPECT_TRUE(add_triangle(mesh, cornerA, removed, survivor).is_valid());
  result.halfedge = mesh.find_halfedge(removed, survivor);
  return result;
}

} // namespace

TEST(MeshEdgeCollapse, GeometryLimitsCatchFoldTheInversionCheckMisses)
{
  const auto [mesh, halfedge] = make_folding_collapse();
  const Mesh::vec_t& position = mesh.get_position(mesh.target_vertex(halfedge));
  ASSERT_EQ(is_collapse_ok(mesh, halfedge), CollapseStatus::Ok);

  EXPECT_FALSE(collapse_inverts_faces(mesh, halfedge, position));
  EXPECT_TRUE(collapse_exceeds_geometry_limits(mesh, halfedge, position));

  // The fold alone decides: without a corner limit the verdict stands, without a fold limit it goes.
  const double straightAngle = std::numbers::pi;
  EXPECT_TRUE(collapse_exceeds_geometry_limits(mesh, halfedge, position,
                                               MeshGeometryLimits<double>{.maxCornerAngle = straightAngle}));
  EXPECT_FALSE(collapse_exceeds_geometry_limits(mesh, halfedge, position, MeshGeometryLimits<double>{straightAngle, straightAngle}));
}

TEST(MeshEdgeCollapse, GeometryLimitsBoundCornerAngles)
{
  // Merging the grid center into its right neighbour stretches the face below into (0,0)-(1,0)-(2,1),
  // whose corner at (1,0) opens to 135 degrees: fine by default, too wide for a 120 degree limit.
  const Mesh mesh = make_grid(3);
  const HalfedgeHandle halfedge = mesh.find_halfedge(grid3(1, 1), grid3(2, 1));
  const Mesh::vec_t& position = mesh.get_position(grid3(2, 1));

  EXPECT_FALSE(collapse_exceeds_geometry_limits(mesh, halfedge, position));
  EXPECT_TRUE(collapse_exceeds_geometry_limits(mesh, halfedge, position,
                                               MeshGeometryLimits<double>{.maxCornerAngle = std::numbers::pi * 120.0 / 180.0}));
}

// --- check_collapse and the safe / topology-only collapses ------------------------------------------

TEST(MeshEdgeCollapse, CheckCollapseReportsTopologyBeforeGeometry)
{
  const Mesh tetrahedron = make_tetrahedron();
  EXPECT_EQ(check_collapse(tetrahedron, tetrahedron.get_edge(EdgeHandle{0}).halfedge, Mesh::vec_t{}),
            CollapseStatus::Tetrahedron);

  const Mesh grid = make_grid(3);
  const HalfedgeHandle halfedge = grid.find_halfedge(grid3(1, 1), grid3(2, 1));
  const Mesh::vec_t& target = grid.get_position(grid3(2, 1));
  EXPECT_EQ(check_collapse(grid, halfedge, target), CollapseStatus::Ok);
  EXPECT_EQ(check_collapse(grid, halfedge, Mesh::vec_t{-10.0, -10.0, 0.0}), CollapseStatus::InvertsFaces);
  EXPECT_EQ(check_collapse(grid, halfedge, target, MeshGeometryLimits<double>{.maxCornerAngle = std::numbers::pi * 120.0 / 180.0}),
            CollapseStatus::InvertsFaces);
}

TEST(MeshEdgeCollapse, CollapseHalfedgeRefusesFoldTopologyOnlyPerformsIt)
{
  auto [mesh, halfedge] = make_folding_collapse();
  const ElementCounts before = counts_of(mesh);

  EXPECT_EQ(collapse_halfedge(mesh, halfedge), CollapseStatus::InvertsFaces);
  EXPECT_EQ(counts_of(mesh), before);
  EXPECT_FALSE(mesh.has_garbage());

  EXPECT_EQ(collapse_halfedge_topology_only(mesh, halfedge), CollapseStatus::Ok);
  EXPECT_EQ(counts_of(mesh).vertices, before.vertices - 1);
  expect_structurally_valid(mesh);
}

TEST(MeshEdgeCollapse, CollapseEdgeRefusesFoldTopologyOnlyPerformsIt)
{
  Mesh mesh = make_grid(3);
  const EdgeHandle edge = mesh.get_halfedge(mesh.find_halfedge(grid3(1, 1), grid3(2, 1))).edge;
  const Mesh::vec_t farAway{-10.0, -10.0, 0.0};
  const ElementCounts before = counts_of(mesh);

  const auto refused = collapse_edge(mesh, edge, farAway);
  EXPECT_EQ(refused.status, CollapseStatus::InvertsFaces);
  EXPECT_FALSE(refused.survivor.is_valid());
  EXPECT_EQ(counts_of(mesh), before);
  EXPECT_FALSE(mesh.has_garbage());

  const auto forced = collapse_edge_topology_only(mesh, edge, farAway);
  ASSERT_TRUE(forced.has_value());
  EXPECT_EQ(mesh.get_position(forced.survivor), farAway);
  expect_structurally_valid(mesh);
}

// --- fuzz ---------------------------------------------------------------------------------------

namespace
{

// Collapses random topologically legal halfedges until none is left, checking the Euler operator's
// bookkeeping and every structural invariant after each step. Geometry is ignored: collapsing down to
// a tetrahedron necessarily folds the surface.
void collapse_until_stuck(Mesh& mesh, std::uint32_t seed)
{
  std::mt19937 generator(seed);
  const std::ptrdiff_t eulerCharacteristic = euler_characteristic(mesh);
  const bool closed = verify_closed(mesh);

  for (std::vector<HalfedgeHandle> candidates = legal_collapses(mesh); !candidates.empty();
       candidates = legal_collapses(mesh))
  {
    std::uniform_int_distribution<std::size_t> pick(0, candidates.size() - 1);
    const HalfedgeHandle halfedge = candidates[pick(generator)];
    const bool boundaryEdge = mesh.is_boundary(mesh.get_halfedge(halfedge).edge);
    const VertexHandle survivor = mesh.target_vertex(halfedge);
    const auto survivorPosition = mesh.get_position(survivor);
    const ElementCounts before = counts_of(mesh);

    ASSERT_EQ(collapse_halfedge_topology_only(mesh, halfedge), CollapseStatus::Ok);

    const ElementCounts expected = boundaryEdge
                                       ? ElementCounts{before.vertices - 1, before.edges - 2, before.faces - 1}
                                       : ElementCounts{before.vertices - 1, before.edges - 3, before.faces - 2};
    ASSERT_EQ(counts_of(mesh), expected);
    ASSERT_EQ(euler_characteristic(mesh), eulerCharacteristic);
    ASSERT_EQ(mesh.get_position(survivor), survivorPosition);
    ASSERT_EQ(verify_closed(mesh), closed);
    ASSERT_TRUE(mesh.has_valid_connectivity());
    ASSERT_TRUE(verify_manifold(mesh));
    ASSERT_TRUE(is_consistently_oriented(mesh));
  }

  mesh.garbage_collection();
  EXPECT_FALSE(mesh.has_garbage());
  EXPECT_EQ(euler_characteristic(mesh), eulerCharacteristic);
  expect_structurally_valid(mesh);
  EXPECT_TRUE(legal_collapses(mesh).empty());
}

} // namespace

TEST(MeshEdgeCollapseFuzz, ClosedSphereCollapsesDownToTetrahedron)
{
  for (std::uint32_t seed = 1; seed <= 5; ++seed)
  {
    Mesh mesh = make_closed_cylinder(12);
    collapse_until_stuck(mesh, seed);
    // Every triangulated sphere except the tetrahedron has a contractible edge (Steinitz).
    EXPECT_EQ(counts_of(mesh), (ElementCounts{4, 6, 4})) << "seed " << seed;
  }
}

TEST(MeshEdgeCollapseFuzz, OpenGridStaysValidUntilStuck)
{
  for (std::uint32_t seed = 1; seed <= 5; ++seed)
  {
    Mesh mesh = make_grid(6);
    collapse_until_stuck(mesh, seed);
    EXPECT_EQ(boundary_loops(mesh).size(), 1U) << "seed " << seed;
  }
}
