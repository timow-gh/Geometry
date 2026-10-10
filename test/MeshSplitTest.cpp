#include <Geometry/Cuboid.hpp>
#include <Geometry/Cylinder.hpp>
#include <Geometry/Mesh/AddTriangle.hpp>
#include <Geometry/Mesh/MakeTriangleMesh.hpp>
#include <Geometry/Mesh/MeshDelete.hpp>
#include <Geometry/Mesh/MeshFromTriangles.hpp>
#include <Geometry/Mesh/MeshGlobalTopology.hpp>
#include <Geometry/Mesh/MeshOrientation.hpp>
#include <Geometry/Mesh/MeshSplit.hpp>
#include <Geometry/Mesh/MeshTopology.hpp>
#include <Geometry/Mesh/MeshVerify.hpp>
#include <Geometry/Mesh/TriangleHalfedgeMesh.hpp>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <random>
#include <span>
#include <tuple>
#include <utility>
#include <vector>

using namespace Geometry;

namespace
{

using Mesh = TriangleHalfedgeMesh3d;
using VertexHandle = Mesh::VertexHandle;
using HalfedgeHandle = Mesh::HalfedgeHandle;
using EdgeHandle = Mesh::EdgeHandle;
using FaceHandle = Mesh::FaceHandle;
using Vec3 = Mesh::vec_t;
using Index = Mesh::handle_value_type;
using Triangle = std::array<Index, 3>;

// n x n grid in the xy-plane as in MeshEdgeCollapseTest.cpp; vertex (i, j) has handle value j * n + i.
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
  const auto vertexAt = [&](std::size_t column, std::size_t row) { return vertices[row * size + column]; };
  // Every triangle must attach along an existing edge (add_triangle refuses a second fan at a
  // corner): row 0 grows through each cell's upper triangle first, later rows through the lower.
  for (std::size_t j = 0; j + 1 < size; ++j)
  {
    for (std::size_t i = 0; i + 1 < size; ++i)
    {
      const std::array<VertexHandle, 3> lower{vertexAt(i, j), vertexAt(i + 1, j), vertexAt(i + 1, j + 1)};
      const std::array<VertexHandle, 3> upper{vertexAt(i, j), vertexAt(i + 1, j + 1), vertexAt(i, j + 1)};
      for (const auto& triangle : j == 0 ? std::array{upper, lower} : std::array{lower, upper})
      {
        EXPECT_TRUE(add_triangle(mesh, triangle).is_valid());
      }
    }
  }
  return mesh;
}

// A 4 x 4 grid with its interior face (1,1), (2,1), (2,2) deleted. add_triangle stores the face side
// of a new boundary edge, so only a deletion leaves edges whose stored halfedge is on the boundary.
Mesh make_grid_with_hole()
{
  Mesh mesh = make_grid(4);
  const HalfedgeHandle halfedge = mesh.find_halfedge(VertexHandle{5}, VertexHandle{6});
  EXPECT_EQ(delete_face(mesh, mesh.get_halfedge(halfedge).face), DeleteStatus::Ok);
  return mesh;
}

// A single triangle: all three sides are boundary edges, on one boundary loop of length 3.
Mesh make_isolated_triangle()
{
  Mesh mesh;
  const VertexHandle vertex0 = mesh.add_vertex({0.0, 0.0, 0.0});
  const VertexHandle vertex1 = mesh.add_vertex({1.0, 0.0, 0.0});
  const VertexHandle vertex2 = mesh.add_vertex({0.0, 1.0, 0.0});
  EXPECT_TRUE(add_triangle(mesh, vertex0, vertex1, vertex2).is_valid());
  return mesh;
}

// Two triangles over the same three vertices, wound oppositely: a closed surface whose two faces share
// their apex across every edge. add_triangle refuses the second triangle; make_mesh_from_triangles
// accepts it.
Mesh make_pillow()
{
  const std::array<Vec3, 3> positions{Vec3{0.0, 0.0, 0.0}, Vec3{1.0, 0.0, 0.0}, Vec3{0.0, 1.0, 0.0}};
  const std::array<Triangle, 2> triangles{Triangle{0, 1, 2}, Triangle{1, 0, 2}};
  auto creation = make_mesh_from_triangles(std::span<const Vec3>{positions}, std::span<const Triangle>{triangles});
  EXPECT_TRUE(creation.has_value());
  return std::move(creation.mesh);
}

Mesh make_cube()
{
  auto creation = make_triangle_mesh(Cuboid<double>{{1.0, 1.0, 1.0}});
  EXPECT_TRUE(creation.has_value());
  return std::move(creation.mesh);
}

Mesh make_cylinder()
{
  auto creation = make_triangle_mesh(Cylinder<double>{Segment3<double>{{0.0, 0.0, 0.0}, {0.0, 0.0, 2.0}}, 1.0}, 12);
  EXPECT_TRUE(creation.has_value());
  return std::move(creation.mesh);
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

struct StorageSizes
{
  std::size_t vertices{};
  std::size_t edges{};
  std::size_t halfedges{};
  std::size_t faces{};

  bool operator==(const StorageSizes&) const = default;
};

StorageSizes storage_sizes_of(const Mesh& mesh)
{
  return {mesh.vertex_storage_size(), mesh.edge_storage_size(), mesh.halfedge_storage_size(), mesh.face_storage_size()};
}

// Whether every element in the first storageSize slots of original is still at its handle in mesh, live
// or deleted as before.
template <typename THandle>
bool keeps_handles(const Mesh& original, const Mesh& mesh, std::size_t storageSize)
{
  for (Index i = 0; i < storageSize; ++i)
  {
    if (mesh.is_live(THandle{i}) != original.is_live(THandle{i}))
    {
      return false;
    }
  }
  return true;
}

// Every link and flag of the connectivity, in storage order, so that a test can show a rejected
// operation left the mesh exactly as it was rather than merely valid.
struct ConnectivityRecords
{
  std::vector<std::tuple<VertexHandle, HalfedgeHandle, HalfedgeHandle, HalfedgeHandle, FaceHandle, EdgeHandle>> halfedges;
  std::vector<std::tuple<HalfedgeHandle, bool, bool>> edges;
  std::vector<std::tuple<HalfedgeHandle, bool>> vertices;
  std::vector<std::tuple<HalfedgeHandle, bool>> faces;

  bool operator==(const ConnectivityRecords&) const = default;
};

ConnectivityRecords connectivity_records(const Mesh& mesh)
{
  ConnectivityRecords records;
  for (Index i = 0; i < mesh.halfedge_storage_size(); ++i)
  {
    const auto& halfedge = mesh.get_halfedge(HalfedgeHandle{i});
    records.halfedges.emplace_back(halfedge.targetVertex, halfedge.twin, halfedge.next, halfedge.prev, halfedge.face, halfedge.edge);
  }
  for (Index i = 0; i < mesh.edge_storage_size(); ++i)
  {
    const auto& edge = mesh.get_edge(EdgeHandle{i});
    records.edges.emplace_back(edge.halfedge, edge.crease, edge.deleted);
  }
  for (Index i = 0; i < mesh.vertex_storage_size(); ++i)
  {
    const auto& vertex = mesh.get_vertex(VertexHandle{i});
    records.vertices.emplace_back(vertex.halfedge, vertex.deleted);
  }
  for (Index i = 0; i < mesh.face_storage_size(); ++i)
  {
    const auto& face = mesh.get_face(FaceHandle{i});
    records.faces.emplace_back(face.get_halfedgehandle(), face.is_deleted());
  }
  return records;
}

// Sum of origin tetrahedra; unchanged by a split at a point on the split edge or face.
double signed_volume(const Mesh& mesh)
{
  double volume = 0.0;
  for (const FaceHandle face : mesh.faces())
  {
    const auto corners = mesh.vertices_around_face(face);
    volume += linal::dot(mesh.get_position(corners[0]),
                         linal::cross(mesh.get_position(corners[1]), mesh.get_position(corners[2])))
              / 6.0;
  }
  return volume;
}

Vec3 edge_midpoint(const Mesh& mesh, EdgeHandle edge)
{
  const HalfedgeHandle halfedge = mesh.get_edge(edge).halfedge;
  return (mesh.get_position(mesh.source_vertex(halfedge)) + mesh.get_position(mesh.target_vertex(halfedge))) / 2.0;
}

Vec3 face_centroid(const Mesh& mesh, FaceHandle face)
{
  const auto corners = mesh.vertices_around_face(face);
  return (mesh.get_position(corners[0]) + mesh.get_position(corners[1]) + mesh.get_position(corners[2])) / 3.0;
}

bool face_has_corner(const Mesh& mesh, FaceHandle face, VertexHandle vertex)
{
  const auto corners = mesh.vertices_around_face(face);
  return corners[0] == vertex || corners[1] == vertex || corners[2] == vertex;
}

void expect_structurally_valid(const Mesh& mesh)
{
  EXPECT_TRUE(mesh.has_valid_connectivity());
  EXPECT_TRUE(verify_manifold(mesh));
  EXPECT_TRUE(is_consistently_oriented(mesh));
}

void expect_closed_and_outward(const Mesh& mesh)
{
  EXPECT_TRUE(verify_closed(mesh));
  EXPECT_EQ(mesh_orientation(mesh), MeshOrientation::Outward);
}

} // namespace

// --- split_edge ---------------------------------------------------------------------------------

TEST(MeshSplit, SplitsInteriorEdgeIntoFourFaces)
{
  Mesh mesh = make_cube();
  const ElementCounts before = counts_of(mesh);
  const double volume = signed_volume(mesh);
  const EdgeHandle edge = *mesh.edges().begin();
  const HalfedgeHandle forward = mesh.get_edge(edge).halfedge;
  const VertexHandle start = mesh.source_vertex(forward);
  const VertexHandle end = mesh.target_vertex(forward);
  const FaceHandle leftFace = mesh.get_halfedge(forward).face;
  const FaceHandle rightFace = mesh.get_halfedge(mesh.get_halfedge(forward).twin).face;
  const Vec3 midpoint = edge_midpoint(mesh, edge);

  const VertexHandle middle = split_edge(mesh, edge, midpoint);

  ASSERT_TRUE(middle.is_valid());
  EXPECT_EQ(mesh.get_position(middle), midpoint);
  EXPECT_EQ(counts_of(mesh), (ElementCounts{before.vertices + 1, before.edges + 3, before.faces + 2}));
  EXPECT_EQ(euler_characteristic(mesh), 2);
  EXPECT_EQ(valence(mesh, middle), 4U);
  EXPECT_FALSE(mesh.find_halfedge(start, end).is_valid());

  // The edge keeps its stored halfedge and now ends at the new vertex; both faces keep the half at
  // the start vertex.
  EXPECT_EQ(mesh.find_halfedge(start, middle), forward);
  EXPECT_EQ(mesh.get_halfedge(forward).edge, edge);
  EXPECT_TRUE(mesh.find_halfedge(middle, end).is_valid());
  EXPECT_TRUE(face_has_corner(mesh, leftFace, start) && face_has_corner(mesh, leftFace, middle));
  EXPECT_TRUE(face_has_corner(mesh, rightFace, start) && face_has_corner(mesh, rightFace, middle));

  expect_structurally_valid(mesh);
  expect_closed_and_outward(mesh);
  EXPECT_NEAR(signed_volume(mesh), volume, 1e-12);
}

TEST(MeshSplit, SplitsBoundaryEdgeIntoTwoFaces)
{
  Mesh mesh = make_grid(3);
  const ElementCounts before = counts_of(mesh);
  const HalfedgeHandle bottom = mesh.find_halfedge(VertexHandle{0}, VertexHandle{1});
  ASSERT_TRUE(bottom.is_valid());

  const VertexHandle middle = split_edge(mesh, mesh.get_halfedge(bottom).edge, Vec3{0.5, 0.0, 0.0});

  ASSERT_TRUE(middle.is_valid());
  EXPECT_EQ(counts_of(mesh), (ElementCounts{before.vertices + 1, before.edges + 2, before.faces + 1}));
  EXPECT_EQ(euler_characteristic(mesh), 1);
  EXPECT_TRUE(is_boundary(mesh, middle));
  EXPECT_EQ(valence(mesh, middle), 3U);
  const auto loops = boundary_loops(mesh);
  ASSERT_EQ(loops.size(), 1U);
  EXPECT_EQ(loops.front().size(), 9U);
  expect_structurally_valid(mesh);
}

// On a boundary edge only one face is split, and the side the edge stores its halfedge on decides
// whether that face lies left or right of it; either way the same elements keep their handles.
TEST(MeshSplit, SplitEdgeKeepsHandlesOnBoundaryEdge)
{
  std::size_t storedOnFaceSide = 0;
  std::size_t storedOnBoundary = 0;
  for (const Mesh& original : {make_grid(3), make_grid_with_hole()})
  {
    for (const EdgeHandle edge : original.edges())
    {
      if (!original.is_boundary(edge))
      {
        continue;
      }
      Mesh mesh = original;
      const HalfedgeHandle forward = mesh.get_edge(edge).halfedge;
      const HalfedgeHandle backward = mesh.get_halfedge(forward).twin;
      const VertexHandle start = mesh.source_vertex(forward);
      const VertexHandle end = mesh.target_vertex(forward);
      const bool faceOnForward = !mesh.is_boundary(forward);
      ++(faceOnForward ? storedOnFaceSide : storedOnBoundary);
      const FaceHandle face = mesh.get_halfedge(faceOnForward ? forward : backward).face;
      const FaceHandle newFace{static_cast<Index>(mesh.face_storage_size())};

      const VertexHandle middle = split_edge(mesh, edge, edge_midpoint(mesh, edge));

      ASSERT_TRUE(middle.is_valid());
      // The edge keeps its stored halfedge, on the same side, now ending at the new vertex.
      EXPECT_EQ(mesh.get_edge(edge).halfedge, forward);
      EXPECT_EQ(mesh.find_halfedge(start, middle), forward);
      EXPECT_EQ(mesh.is_boundary(forward), !faceOnForward);
      // The split face keeps the half at the start vertex; the new face holds the half at the end.
      EXPECT_TRUE(face_has_corner(mesh, face, start) && face_has_corner(mesh, face, middle));
      EXPECT_FALSE(face_has_corner(mesh, face, end));
      ASSERT_TRUE(mesh.is_live(newFace));
      EXPECT_TRUE(face_has_corner(mesh, newFace, middle) && face_has_corner(mesh, newFace, end));
      EXPECT_FALSE(face_has_corner(mesh, newFace, start));
      // The new vertex is on the boundary, so it is represented by its outgoing boundary halfedge.
      EXPECT_TRUE(mesh.is_boundary(mesh.get_vertex(middle).halfedge));
      expect_structurally_valid(mesh);
    }
  }
  EXPECT_GT(storedOnFaceSide, 0U);
  EXPECT_GT(storedOnBoundary, 0U);
}

TEST(MeshSplit, SplitsEdgeOfIsolatedTriangle)
{
  const Mesh original = make_isolated_triangle();
  for (const EdgeHandle edge : original.edges())
  {
    Mesh mesh = original;

    const VertexHandle middle = split_edge(mesh, edge, edge_midpoint(mesh, edge));

    ASSERT_TRUE(middle.is_valid());
    EXPECT_EQ(counts_of(mesh), (ElementCounts{4, 5, 2}));
    EXPECT_EQ(euler_characteristic(mesh), 1);
    EXPECT_EQ(valence(mesh, middle), 3U);
    const auto loops = boundary_loops(mesh);
    ASSERT_EQ(loops.size(), 1U);
    EXPECT_EQ(loops.front().size(), 4U);
    expect_structurally_valid(mesh);
  }
}

// Both faces of a pillow edge have the same apex, so a split would join the new vertex to that apex by
// two edges.
TEST(MeshSplit, SplitEdgeRejectsEdgeWithSharedApex)
{
  const Mesh original = make_pillow();
  ASSERT_EQ(original.face_count(), 2U);
  const ConnectivityRecords records = connectivity_records(original);
  for (const EdgeHandle edge : original.edges())
  {
    Mesh mesh = original;

    EXPECT_FALSE(split_edge(mesh, edge, edge_midpoint(mesh, edge)).is_valid());

    EXPECT_TRUE(connectivity_records(mesh) == records);
    EXPECT_EQ(mesh.vertex_storage_size(), original.vertex_storage_size());
  }
}

// Covers every edge configuration a split has to handle: interior, and boundary with the stored
// halfedge on either side, at vertices of every boundary and valence kind, and amid tombstones.
TEST(MeshSplit, SplittingAnyEdgeKeepsMeshValid)
{
  std::size_t storedOnBoundary = 0;
  std::size_t twinOnBoundary = 0;
  std::size_t endRepresentativeReplaced = 0;
  for (const Mesh& original : {make_grid(3), make_grid(4), make_grid_with_hole(), make_cube()})
  {
    for (const EdgeHandle edge : original.edges())
    {
      Mesh mesh = original;
      const bool boundary = mesh.is_boundary(edge);
      const HalfedgeHandle forward = mesh.get_edge(edge).halfedge;
      const HalfedgeHandle backward = mesh.get_halfedge(forward).twin;
      storedOnBoundary += mesh.is_boundary(forward) ? 1U : 0U;
      twinOnBoundary += boundary && !mesh.is_boundary(forward) ? 1U : 0U;
      // The end vertex no longer starts backward after the split, so it needs a new representative.
      endRepresentativeReplaced += mesh.get_vertex(mesh.target_vertex(forward)).halfedge == backward ? 1U : 0U;
      const ElementCounts before = counts_of(mesh);

      ASSERT_TRUE(split_edge(mesh, edge, edge_midpoint(mesh, edge)).is_valid());

      const std::size_t splitFaces = boundary ? 1U : 2U;
      EXPECT_EQ(counts_of(mesh), (ElementCounts{before.vertices + 1, before.edges + 1 + splitFaces, before.faces + splitFaces}));
      EXPECT_EQ(euler_characteristic(mesh), euler_characteristic(original));
      EXPECT_EQ(boundary_loop_count(mesh), boundary_loop_count(original));
      expect_structurally_valid(mesh);
    }
  }
  EXPECT_GT(storedOnBoundary, 0U);
  EXPECT_GT(twinOnBoundary, 0U);
  EXPECT_GT(endRepresentativeReplaced, 0U);
}

TEST(MeshSplit, SplitEdgeKeepsCreaseOnBothHalves)
{
  const auto expect_crease_on_both_halves = [](Mesh& mesh, EdgeHandle edge) {
    const bool crease = mesh.is_crease(edge);
    const HalfedgeHandle forward = mesh.get_edge(edge).halfedge;
    const VertexHandle end = mesh.target_vertex(forward);

    const VertexHandle middle = split_edge(mesh, edge, edge_midpoint(mesh, edge));

    ASSERT_TRUE(middle.is_valid());
    EXPECT_EQ(mesh.is_crease(edge), crease);
    EXPECT_EQ(mesh.is_crease(mesh.get_halfedge(mesh.find_halfedge(middle, end)).edge), crease);
    // The new edges to the apexes are not feature lines.
    for (auto outgoing = std::as_const(mesh).outgoing_halfedges(middle).circulator(); outgoing.is_valid(); ++outgoing)
    {
      const VertexHandle neighbour = mesh.target_vertex(outgoing.get_halfedgehandle());
      if (neighbour != end && neighbour != mesh.source_vertex(forward))
      {
        EXPECT_FALSE(mesh.is_crease(mesh.get_halfedge(outgoing.get_halfedgehandle()).edge));
      }
    }
  };

  for (const bool crease : {true, false})
  {
    Mesh cube = make_cube();
    const auto cubeEdges = cube.edges();
    const auto edge = std::ranges::find_if(cubeEdges, [&](EdgeHandle candidate) { return cube.is_crease(candidate) == crease; });
    ASSERT_NE(edge, cubeEdges.end());
    expect_crease_on_both_halves(cube, *edge);

    // A boundary edge has a single apex.
    Mesh grid = make_grid(3);
    const HalfedgeHandle bottom = grid.find_halfedge(VertexHandle{0}, VertexHandle{1});
    ASSERT_TRUE(bottom.is_valid());
    grid.set_crease(grid.get_halfedge(bottom).edge, crease);
    expect_crease_on_both_halves(grid, grid.get_halfedge(bottom).edge);
  }
}

// Corefinement keeps side arrays indexed by storage, which stay aligned only because a split appends
// its new elements and leaves every earlier one at its handle.
TEST(MeshSplit, SplitsOnlyAppendElements)
{
  const auto expect_appended = [](const Mesh& original, const Mesh& mesh, const StorageSizes& added) {
    const StorageSizes before = storage_sizes_of(original);
    EXPECT_EQ(storage_sizes_of(mesh),
              (StorageSizes{before.vertices + added.vertices, before.edges + added.edges,
                            before.halfedges + added.halfedges, before.faces + added.faces}));
    EXPECT_TRUE(keeps_handles<VertexHandle>(original, mesh, before.vertices));
    EXPECT_TRUE(keeps_handles<EdgeHandle>(original, mesh, before.edges));
    EXPECT_TRUE(keeps_handles<HalfedgeHandle>(original, mesh, before.halfedges));
    EXPECT_TRUE(keeps_handles<FaceHandle>(original, mesh, before.faces));
  };

  for (const Mesh& original : {make_grid_with_hole(), make_cube()})
  {
    for (const EdgeHandle edge : original.edges())
    {
      Mesh mesh = original;
      ASSERT_TRUE(split_edge(mesh, edge, edge_midpoint(mesh, edge)).is_valid());
      expect_appended(original, mesh, original.is_boundary(edge) ? StorageSizes{1, 2, 4, 1} : StorageSizes{1, 3, 6, 2});
    }
    for (const FaceHandle face : original.faces())
    {
      Mesh mesh = original;
      ASSERT_TRUE(split_face(mesh, face, face_centroid(mesh, face)).is_valid());
      expect_appended(original, mesh, StorageSizes{1, 3, 6, 2});
    }
  }
}

TEST(MeshSplit, SplitEdgeRejectsInvalidAndDeletedEdge)
{
  Mesh mesh = make_grid(3);
  EXPECT_FALSE(split_edge(mesh, EdgeHandle{}, Vec3{}).is_valid());

  // Deleting the corner face (1,0), (2,0), (2,1) drops its two boundary edges.
  const HalfedgeHandle cornerEdge = mesh.find_halfedge(VertexHandle{1}, VertexHandle{2});
  ASSERT_TRUE(cornerEdge.is_valid());
  const EdgeHandle deleted = mesh.get_halfedge(cornerEdge).edge;
  ASSERT_EQ(delete_face(mesh, mesh.get_halfedge(cornerEdge).face), DeleteStatus::Ok);
  ASSERT_TRUE(mesh.is_deleted(deleted));
  const ElementCounts before = counts_of(mesh);
  const std::size_t vertexStorage = mesh.vertex_storage_size();

  EXPECT_FALSE(split_edge(mesh, deleted, Vec3{}).is_valid());
  EXPECT_EQ(counts_of(mesh), before);
  EXPECT_EQ(mesh.vertex_storage_size(), vertexStorage);
}

// --- split_face ---------------------------------------------------------------------------------

TEST(MeshSplit, SplitsFaceIntoThree)
{
  Mesh mesh = make_cube();
  const ElementCounts before = counts_of(mesh);
  const double volume = signed_volume(mesh);
  const FaceHandle face = *mesh.faces().begin();
  const auto corners = mesh.vertices_around_face(face);
  const HalfedgeHandle stored = mesh.get_face(face).get_halfedgehandle();
  const Vec3 centroid = face_centroid(mesh, face);

  const VertexHandle center = split_face(mesh, face, centroid);

  ASSERT_TRUE(center.is_valid());
  EXPECT_EQ(mesh.get_position(center), centroid);
  EXPECT_EQ(counts_of(mesh), (ElementCounts{before.vertices + 1, before.edges + 3, before.faces + 2}));
  EXPECT_EQ(euler_characteristic(mesh), 2);
  EXPECT_EQ(valence(mesh, center), 3U);
  for (const VertexHandle corner : corners)
  {
    EXPECT_TRUE(mesh.find_halfedge(center, corner).is_valid());
  }
  // The face keeps the triangle over its stored halfedge.
  EXPECT_EQ(mesh.get_halfedge(stored).face, face);
  EXPECT_TRUE(face_has_corner(mesh, face, center));

  expect_structurally_valid(mesh);
  expect_closed_and_outward(mesh);
  EXPECT_NEAR(signed_volume(mesh), volume, 1e-12);
}

TEST(MeshSplit, SplittingAnyFaceKeepsMeshValid)
{
  for (const Mesh& original : {make_grid(3), make_grid_with_hole(), make_cube()})
  {
    for (const FaceHandle face : original.faces())
    {
      Mesh mesh = original;
      const ElementCounts before = counts_of(mesh);

      ASSERT_TRUE(split_face(mesh, face, face_centroid(mesh, face)).is_valid());

      EXPECT_EQ(counts_of(mesh), (ElementCounts{before.vertices + 1, before.edges + 3, before.faces + 2}));
      EXPECT_EQ(euler_characteristic(mesh), euler_characteristic(original));
      EXPECT_EQ(boundary_loop_count(mesh), boundary_loop_count(original));
      expect_structurally_valid(mesh);
    }
  }
}

TEST(MeshSplit, SplitsIsolatedTriangle)
{
  Mesh mesh = make_isolated_triangle();
  const FaceHandle face = *mesh.faces().begin();

  const VertexHandle center = split_face(mesh, face, face_centroid(mesh, face));

  ASSERT_TRUE(center.is_valid());
  EXPECT_EQ(counts_of(mesh), (ElementCounts{4, 6, 3}));
  EXPECT_EQ(euler_characteristic(mesh), 1);
  EXPECT_FALSE(is_boundary(mesh, center));
  EXPECT_EQ(valence(mesh, center), 3U);
  const auto loops = boundary_loops(mesh);
  ASSERT_EQ(loops.size(), 1U);
  EXPECT_EQ(loops.front().size(), 3U);
  expect_structurally_valid(mesh);
}

// A cube face has two crease sides on the cube's edges and a smooth one on its diagonal, so both flags
// must survive the split.
TEST(MeshSplit, SplitFaceKeepsSideCreases)
{
  Mesh mesh = make_cube();
  const FaceHandle face = *mesh.faces().begin();
  const auto sides = mesh.halfedges_around_face(face);
  const auto corners = mesh.vertices_around_face(face);
  std::array<bool, 3> creases{};
  for (std::size_t i = 0; i < 3; ++i)
  {
    creases[i] = mesh.is_crease(mesh.get_halfedge(sides[i]).edge);
  }
  ASSERT_EQ(std::ranges::count(creases, true), 2);

  const VertexHandle center = split_face(mesh, face, face_centroid(mesh, face));

  ASSERT_TRUE(center.is_valid());
  for (std::size_t i = 0; i < 3; ++i)
  {
    EXPECT_EQ(mesh.is_crease(mesh.get_halfedge(sides[i]).edge), creases[i]);
  }
  for (const VertexHandle corner : corners)
  {
    EXPECT_FALSE(mesh.is_crease(mesh.get_halfedge(mesh.find_halfedge(center, corner)).edge));
  }
}

// A pillow's edges cannot be split, but its faces can: the result has a tetrahedron's connectivity, a
// closed surface again.
TEST(MeshSplit, SplitsPillowFace)
{
  Mesh mesh = make_pillow();
  ASSERT_EQ(mesh.face_count(), 2U);
  const FaceHandle face = *mesh.faces().begin();

  const VertexHandle center = split_face(mesh, face, face_centroid(mesh, face));

  ASSERT_TRUE(center.is_valid());
  EXPECT_EQ(counts_of(mesh), (ElementCounts{4, 6, 4}));
  EXPECT_EQ(euler_characteristic(mesh), 2);
  EXPECT_EQ(valence(mesh, center), 3U);
  EXPECT_TRUE(verify_closed(mesh));
  expect_structurally_valid(mesh);
}

TEST(MeshSplit, SplitFaceRejectsInvalidAndDeletedFace)
{
  Mesh mesh = make_grid(3);
  EXPECT_FALSE(split_face(mesh, FaceHandle{}, Vec3{}).is_valid());

  const FaceHandle face = *mesh.faces().begin();
  ASSERT_EQ(delete_face(mesh, face), DeleteStatus::Ok);
  const ElementCounts before = counts_of(mesh);

  EXPECT_FALSE(split_face(mesh, face, Vec3{}).is_valid());
  EXPECT_EQ(counts_of(mesh), before);
}

// --- repeated splits ----------------------------------------------------------------------------

// Corefinement splits the same region over and over, so new vertices meet new vertices and sub-edges
// get split again.
TEST(MeshSplitFuzz, RepeatedSplitsStayValid)
{
  for (std::uint32_t seed = 1; seed <= 5; ++seed)
  {
    Mesh mesh = make_cylinder();
    const double volume = signed_volume(mesh);
    std::mt19937 generator(seed);

    for (int step = 0; step < 150; ++step)
    {
      const ElementCounts before = counts_of(mesh);
      if (step % 2 == 0)
      {
        std::uniform_int_distribution<std::uint32_t> pick(0, static_cast<std::uint32_t>(mesh.edge_storage_size() - 1));
        const EdgeHandle edge{pick(generator)};
        ASSERT_TRUE(split_edge(mesh, edge, edge_midpoint(mesh, edge)).is_valid());
        ASSERT_EQ(counts_of(mesh), (ElementCounts{before.vertices + 1, before.edges + 3, before.faces + 2}));
      }
      else
      {
        std::uniform_int_distribution<std::uint32_t> pick(0, static_cast<std::uint32_t>(mesh.face_storage_size() - 1));
        const FaceHandle face{pick(generator)};
        ASSERT_TRUE(split_face(mesh, face, face_centroid(mesh, face)).is_valid());
        ASSERT_EQ(counts_of(mesh), (ElementCounts{before.vertices + 1, before.edges + 3, before.faces + 2}));
      }
      ASSERT_TRUE(mesh.has_valid_connectivity()) << "seed " << seed << ", step " << step;
      ASSERT_TRUE(verify_manifold(mesh)) << "seed " << seed << ", step " << step;
    }

    EXPECT_FALSE(mesh.has_garbage());
    EXPECT_EQ(euler_characteristic(mesh), 2);
    EXPECT_TRUE(is_consistently_oriented(mesh));
    expect_closed_and_outward(mesh);
    EXPECT_NEAR(signed_volume(mesh), volume, 1e-9) << "seed " << seed;
  }
}
