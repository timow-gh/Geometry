#include <Geometry/Cylinder.hpp>
#include <Geometry/Mesh/MakeTriangleMesh.hpp>
#include <Geometry/Mesh/MeshCorefine.hpp>
#include <Geometry/Mesh/MeshFromTriangles.hpp>
#include <Geometry/Mesh/MeshGlobalTopology.hpp>
#include <Geometry/Mesh/MeshVerify.hpp>
#include <Geometry/Mesh/TriangleHalfedgeMesh.hpp>
#include <Geometry/Mesh/detail/Corefine.hpp>
#include <Geometry/Segment.hpp>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <span>
#include <utility>
#include <vector>

using namespace Geometry;

// A named namespace rather than an anonymous one, so that its aliases cannot hide library names
// (MSVC C4459).
namespace MeshCorefineTesting
{

using Mesh = TriangleHalfedgeMesh3d;
using VertexHandle = Mesh::VertexHandle;
using EdgeHandle = Mesh::EdgeHandle;
using FaceHandle = Mesh::FaceHandle;
using Vec3 = Mesh::vec_t;
using Triangle = std::array<std::uint32_t, 3>;
using Result = CorefineResult<double, std::uint32_t>;

constexpr double tolerance = 1e-12;

Mesh make_mesh(const std::vector<Vec3>& positions, const std::vector<Triangle>& triangles)
{
  auto result = make_mesh_from_triangles(std::span<const Vec3>{positions}, std::span<const Triangle>{triangles});
  EXPECT_TRUE(result.has_value());
  return std::move(result.mesh);
}

// Outward-oriented box; vertex i has the max coordinate on axis k iff bit k of i is set.
Mesh make_box(const Vec3& min, const Vec3& max)
{
  std::vector<Vec3> corners;
  corners.reserve(8);
  for (std::uint32_t i = 0; i < 8; ++i)
  {
    corners.emplace_back((i & 1U) != 0 ? max[0] : min[0], (i & 2U) != 0 ? max[1] : min[1], (i & 4U) != 0 ? max[2] : min[2]);
  }
  return make_mesh(corners,
                   {Triangle{0, 4, 6}, Triangle{0, 6, 2}, Triangle{1, 3, 7}, Triangle{1, 7, 5}, Triangle{0, 1, 5}, Triangle{0, 5, 4},
                    Triangle{2, 6, 7}, Triangle{2, 7, 3}, Triangle{0, 2, 3}, Triangle{0, 3, 1}, Triangle{4, 5, 7}, Triangle{4, 7, 6}});
}

Vec3 face_normal(const Mesh& mesh, const FaceHandle face)
{
  const auto corners = mesh.vertices_around_face(face);
  return detail::triangle_orientation(mesh.get_position(corners[0]), mesh.get_position(corners[1]), mesh.get_position(corners[2]));
}

double signed_volume(const Mesh& mesh)
{
  double volume = 0.0;
  for (const FaceHandle face : mesh.faces())
  {
    const auto corners = mesh.vertices_around_face(face);
    volume += linal::dot(mesh.get_position(corners[0]), linal::cross(mesh.get_position(corners[1]), mesh.get_position(corners[2])));
  }
  return volume / 6.0;
}

void expect_closed_manifold(const Mesh& mesh)
{
  EXPECT_TRUE(mesh.has_valid_connectivity());
  EXPECT_TRUE(verify_manifold(mesh));
  EXPECT_TRUE(verify_closed(mesh));
}

// The endpoint positions of an edge, the lexicographically smaller first.
std::array<std::array<double, 3>, 2> endpoints(const Mesh& mesh, const EdgeHandle edge)
{
  const auto halfedge = mesh.get_edge(edge).halfedge;
  const Vec3& source = mesh.get_position(mesh.source_vertex(halfedge));
  const Vec3& target = mesh.get_position(mesh.target_vertex(halfedge));
  std::array<std::array<double, 3>, 2> ends{std::array<double, 3>{source[0], source[1], source[2]},
                                            std::array<double, 3>{target[0], target[1], target[2]}};
  std::ranges::sort(ends);
  return ends;
}

// Edge i of A and edge i of B join the same two positions, exactly.
void expect_matching_curves(const Mesh& meshA, const Mesh& meshB, const Result& result)
{
  ASSERT_EQ(result.intersectionEdgesA.size(), result.intersectionEdgesB.size());
  for (std::size_t i = 0; i < result.intersectionEdgesA.size(); ++i)
  {
    EXPECT_EQ(endpoints(meshA, result.intersectionEdgesA[i]), endpoints(meshB, result.intersectionEdgesB[i])) << "edge " << i;
  }
}

// Number of intersection edges at every vertex, by storage index.
std::vector<std::size_t> curve_degrees(const Mesh& mesh, const std::vector<EdgeHandle>& edges)
{
  std::vector<std::size_t> degrees(mesh.vertex_storage_size(), 0);
  for (const EdgeHandle edge : edges)
  {
    const auto halfedge = mesh.get_edge(edge).halfedge;
    ++degrees[mesh.source_vertex(halfedge).get_value()];
    ++degrees[mesh.target_vertex(halfedge).get_value()];
  }
  return degrees;
}

// Whether the edges form closed loops: every vertex on the curve has exactly two curve edges.
bool forms_closed_loops(const Mesh& mesh, const std::vector<EdgeHandle>& edges)
{
  return std::ranges::all_of(curve_degrees(mesh, edges), [](const std::size_t degree) { return degree == 0 || degree == 2; });
}

// Number of connected components of the curve.
std::size_t curve_component_count(const Mesh& mesh, const std::vector<EdgeHandle>& edges)
{
  std::vector<std::size_t> parent(mesh.vertex_storage_size());
  std::iota(parent.begin(), parent.end(), std::size_t{0});
  const auto findRoot = [&parent](std::size_t vertex) {
    while (parent[vertex] != vertex)
    {
      vertex = parent[vertex] = parent[parent[vertex]];
    }
    return vertex;
  };
  for (const EdgeHandle edge : edges)
  {
    const auto halfedge = mesh.get_edge(edge).halfedge;
    parent[findRoot(mesh.source_vertex(halfedge).get_value())] = findRoot(mesh.target_vertex(halfedge).get_value());
  }
  const std::vector<std::size_t> degrees = curve_degrees(mesh, edges);
  std::size_t count = 0;
  for (std::size_t vertex = 0; vertex < parent.size(); ++vertex)
  {
    count += degrees[vertex] > 0 && findRoot(vertex) == vertex ? 1U : 0U;
  }
  return count;
}

// Every face lies in a side of the box [min, max] and faces outward, so no refined face folded over.
void expect_faces_on_box(const Mesh& mesh, const Vec3& min, const Vec3& max)
{
  for (const FaceHandle face : mesh.faces())
  {
    const auto corners = mesh.vertices_around_face(face);
    const Vec3 centroid{(mesh.get_position(corners[0]) + mesh.get_position(corners[1]) + mesh.get_position(corners[2])) / 3.0};
    const Vec3 normal = face_normal(mesh, face);
    bool onSide = false;
    for (std::uint8_t axis = 0; axis < 3 && !onSide; ++axis)
    {
      if (std::abs(centroid[axis] - max[axis]) <= tolerance)
      {
        onSide = true;
        EXPECT_GT(normal[axis], 0.0) << "face " << face.get_value();
      }
      else if (std::abs(centroid[axis] - min[axis]) <= tolerance)
      {
        onSide = true;
        EXPECT_LT(normal[axis], 0.0) << "face " << face.get_value();
      }
    }
    EXPECT_TRUE(onSide) << "face " << face.get_value();
  }
}

TEST(MeshCorefineTest, shifted_boxes_get_matching_closed_loops)
{
  // Dyadic offsets keep every intersection point exactly representable and avoid coplanar faces.
  const Vec3 minA{0.0, 0.0, 0.0};
  const Vec3 maxA{2.0, 2.0, 2.0};
  const Vec3 minB{1.0, 0.5, 0.25};
  const Vec3 maxB{3.0, 2.5, 2.25};
  Mesh boxA = make_box(minA, maxA);
  Mesh boxB = make_box(minB, maxB);
  const double volumeA = signed_volume(boxA);
  const double volumeB = signed_volume(boxB);

  const Result result = corefine(std::move(boxA), std::move(boxB));
  ASSERT_TRUE(result.has_value());
  ASSERT_FALSE(result.intersectionEdgesA.empty());
  const Mesh& meshA = result.meshA;
  const Mesh& meshB = result.meshB;

  for (const Mesh* mesh : {&meshA, &meshB})
  {
    expect_closed_manifold(*mesh);
    EXPECT_EQ(euler_characteristic(*mesh), 2);
  }
  EXPECT_NEAR(signed_volume(meshA), volumeA, tolerance);
  EXPECT_NEAR(signed_volume(meshB), volumeB, tolerance);
  expect_faces_on_box(meshA, minA, maxA);
  expect_faces_on_box(meshB, minB, maxB);
  expect_matching_curves(meshA, meshB, result);
  EXPECT_TRUE(forms_closed_loops(meshA, result.intersectionEdgesA));
  EXPECT_TRUE(forms_closed_loops(meshB, result.intersectionEdgesB));
  EXPECT_EQ(curve_component_count(meshA, result.intersectionEdgesA), 1U);
  EXPECT_EQ(curve_component_count(meshB, result.intersectionEdgesB), 1U);
}

TEST(MeshCorefineTest, refinement_maps_every_key_to_one_vertex_per_mesh)
{
  Mesh meshA = make_box(Vec3{0.0, 0.0, 0.0}, Vec3{2.0, 2.0, 2.0});
  Mesh meshB = make_box(Vec3{1.0, 0.5, 0.25}, Vec3{3.0, 2.5, 2.25});
  const std::size_t originalVertexCountA = meshA.vertex_storage_size();

  const auto result = detail::corefine_in_place(meshA, meshB);
  ASSERT_TRUE(result.has_value());
  const auto& graph = result.corefinement.graph;
  const auto& refinementA = result.corefinement.onMeshA;
  const auto& refinementB = result.corefinement.onMeshB;

  for (std::size_t i = 0; i < graph.points.size(); ++i)
  {
    const VertexHandle vertexA = refinementA.vertexOfPoint[i];
    const VertexHandle vertexB = refinementB.vertexOfPoint[i];
    ASSERT_TRUE(vertexA.is_valid() && vertexB.is_valid());
    EXPECT_EQ(refinementA.pointOfVertex[vertexA.get_value()], i);
    EXPECT_EQ(refinementB.pointOfVertex[vertexB.get_value()], i);
    // Both meshes place the point at the graph's one rounded position.
    EXPECT_EQ(meshA.get_position(vertexA), graph.points[i].position());
    EXPECT_EQ(meshB.get_position(vertexB), graph.points[i].position());
    if (vertexA.get_value() >= originalVertexCountA)
    {
      // Inserted vertices hand out their definition, not a bare position.
      EXPECT_EQ(detail::implicit_point_of(meshA, refinementA, graph, vertexA).kind(), graph.points[i].kind());
    }
  }
  for (std::size_t segment = 0; segment < graph.segments.size(); ++segment)
  {
    for (const auto* refinement : {&refinementA, &refinementB})
    {
      const Mesh& mesh = refinement == &refinementA ? meshA : meshB;
      const EdgeHandle edge = refinement->edgeOfSegment[segment];
      ASSERT_TRUE(edge.is_valid());
      EXPECT_TRUE(refinement->isIntersectionEdge[edge.get_value()]);
      const auto halfedge = mesh.find_halfedge(refinement->vertexOfPoint[graph.segments[segment][0]],
                                               refinement->vertexOfPoint[graph.segments[segment][1]]);
      ASSERT_TRUE(halfedge.is_valid());
      EXPECT_EQ(mesh.get_halfedge(halfedge).edge, edge);
    }
  }
  const auto marked = [](const std::vector<bool>& flags) { return static_cast<std::size_t>(std::ranges::count(flags, true)); };
  EXPECT_EQ(marked(refinementA.isIntersectionEdge), graph.segments.size());
  EXPECT_EQ(marked(refinementB.isIntersectionEdge), graph.segments.size());
}

// A tetrahedron with its apex inside the box [0, 2]^3 and its base above it, so that its three side
// faces cross the box's top face and the diagonal splitting it. Two-decimal coordinates are not
// representable, so intersection points are rounded.
Mesh make_piercing_tetrahedron()
{
  return make_mesh({Vec3{1.34, 1.24, 1.65}, Vec3{1.61, 1.35, 2.48}, Vec3{0.42, 1.39, 2.30}, Vec3{1.03, 0.30, 2.82}},
                   {Triangle{0, 2, 1}, Triangle{0, 3, 2}, Triangle{0, 1, 3}, Triangle{1, 2, 3}});
}

TEST(MeshCorefineTest, collinear_points_split_the_edge_they_lie_on)
{
  // A side face of the tetrahedron crosses the box's top face along a line through three points: where
  // it enters the top face, where it crosses the diagonal, and where it leaves. Once the entry and exit
  // points are joined by a sub-edge, the diagonal point lies on it, but its rounded position does not.
  // Only deciding by the points' definitions splits that sub-edge; deciding by rounded positions left a
  // sliver face.
  const Vec3 min{0.0, 0.0, 0.0};
  const Vec3 max{2.0, 2.0, 2.0};
  Mesh tetrahedron = make_piercing_tetrahedron();
  const double volumeB = signed_volume(tetrahedron);

  const Result result = corefine(make_box(min, max), std::move(tetrahedron));
  ASSERT_TRUE(result.has_value());
  const Mesh& meshA = result.meshA;
  const Mesh& meshB = result.meshB;

  for (const Mesh* mesh : {&meshA, &meshB})
  {
    expect_closed_manifold(*mesh);
    for (const FaceHandle face : mesh->faces())
    {
      EXPECT_GT(linal::length(face_normal(*mesh, face)), 1e-9) << "sliver face " << face.get_value();
    }
  }
  EXPECT_NEAR(signed_volume(meshA), 8.0, tolerance);
  EXPECT_NEAR(signed_volume(meshB), volumeB, tolerance);
  expect_faces_on_box(meshA, min, max);
  expect_matching_curves(meshA, meshB, result);
  EXPECT_TRUE(forms_closed_loops(meshA, result.intersectionEdgesA));
  EXPECT_EQ(curve_component_count(meshA, result.intersectionEdgesA), 1U);
}

TEST(MeshCorefineTest, cylinder_through_box_gets_two_loops)
{
  // The axis avoids the box diagonals x = y, so no side edge of the cylinder hits one exactly.
  const Vec3 minA{0.0, 0.0, 0.0};
  const Vec3 maxA{2.0, 2.0, 2.0};
  Mesh box = make_box(minA, maxA);
  auto cylinder = make_triangle_mesh(Cylinder<double>{Segment3<double>{Vec3{0.7, 1.2, -1.0}, Vec3{0.7, 1.2, 3.0}}, 0.45}, 16);
  ASSERT_TRUE(cylinder.has_value());
  const double volumeA = signed_volume(box);
  const double volumeB = signed_volume(cylinder.mesh);

  const Result result = corefine(std::move(box), std::move(cylinder.mesh));
  ASSERT_TRUE(result.has_value());
  const Mesh& meshA = result.meshA;
  const Mesh& meshB = result.meshB;

  for (const Mesh* mesh : {&meshA, &meshB})
  {
    expect_closed_manifold(*mesh);
    EXPECT_EQ(euler_characteristic(*mesh), 2);
  }
  EXPECT_NEAR(signed_volume(meshA), volumeA, 1e-9);
  EXPECT_NEAR(signed_volume(meshB), volumeB, 1e-9);
  expect_faces_on_box(meshA, minA, maxA);
  expect_matching_curves(meshA, meshB, result);
  EXPECT_TRUE(forms_closed_loops(meshA, result.intersectionEdgesA));
  EXPECT_TRUE(forms_closed_loops(meshB, result.intersectionEdgesB));
  // One loop where the cylinder enters the bottom face, one where it leaves the top face.
  EXPECT_EQ(curve_component_count(meshA, result.intersectionEdgesA), 2U);
  EXPECT_EQ(curve_component_count(meshB, result.intersectionEdgesB), 2U);
}

TEST(MeshCorefineTest, coplanar_contact_refines_the_shared_plane)
{
  // B's min-x face lies in A's max-x face but is shifted, so diagonals and sides cross.
  const Vec3 minA{0.0, 0.0, 0.0};
  const Vec3 maxA{1.0, 1.0, 1.0};
  const Vec3 minB{1.0, 0.5, 0.25};
  const Vec3 maxB{2.0, 1.5, 1.25};

  const Result result = corefine(make_box(minA, maxA), make_box(minB, maxB));
  ASSERT_TRUE(result.has_value());
  ASSERT_FALSE(result.intersectionEdgesA.empty());
  const Mesh& meshA = result.meshA;
  const Mesh& meshB = result.meshB;

  for (const Mesh* mesh : {&meshA, &meshB})
  {
    expect_closed_manifold(*mesh);
    EXPECT_EQ(euler_characteristic(*mesh), 2);
  }
  expect_faces_on_box(meshA, minA, maxA);
  expect_faces_on_box(meshB, minB, maxB);
  expect_matching_curves(meshA, meshB, result);
  for (const EdgeHandle edge : result.intersectionEdgesA)
  {
    const auto ends = endpoints(meshA, edge);
    EXPECT_EQ(ends[0][0], 1.0);
    EXPECT_EQ(ends[1][0], 1.0);
  }
  // The overlap rectangle [0.5, 1] x [0.25, 1] in the plane x = 1 is bounded by curve edges.
  double boundaryLength = 0.0;
  for (const EdgeHandle edge : result.intersectionEdgesA)
  {
    const auto ends = endpoints(meshA, edge);
    const bool alongY = ends[0][2] == ends[1][2] && (ends[0][2] == 0.25 || ends[0][2] == 1.0);
    const bool alongZ = ends[0][1] == ends[1][1] && (ends[0][1] == 0.5 || ends[0][1] == 1.0);
    if (alongY || alongZ)
    {
      boundaryLength += std::hypot(ends[1][1] - ends[0][1], ends[1][2] - ends[0][2]);
    }
  }
  EXPECT_NEAR(boundaryLength, 2.0 * (0.5 + 0.75), tolerance);
}

TEST(MeshCorefineTest, boxes_sharing_a_face_mark_its_edges_without_new_vertices)
{
  const Result result = corefine(make_box(Vec3{0.0, 0.0, 0.0}, Vec3{1.0, 1.0, 1.0}), make_box(Vec3{1.0, 0.0, 0.0}, Vec3{2.0, 1.0, 1.0}));
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.meshA.vertex_count(), 8U);
  EXPECT_EQ(result.meshB.vertex_count(), 8U);
  // The four sides of the shared square and its diagonal.
  EXPECT_EQ(result.intersectionEdgesA.size(), 5U);
  expect_matching_curves(result.meshA, result.meshB, result);
}

TEST(MeshCorefineTest, triangle_pierced_by_box_recovers_the_loop_by_flips)
{
  // The box pierces the triangle's interior: its four vertical edges and four side diagonals cross
  // the plane z = 0, so eight points inside one face must be joined into an octagon.
  const Result result = corefine(make_mesh({Vec3{-4.0, -4.0, 0.0}, Vec3{8.0, -4.0, 0.0}, Vec3{-4.0, 8.0, 0.0}}, {Triangle{0, 1, 2}}),
                                 make_box(Vec3{0.0, 0.0, -1.0}, Vec3{1.0, 1.0, 1.0}));
  ASSERT_TRUE(result.has_value());
  const Mesh& meshA = result.meshA;
  const Mesh& meshB = result.meshB;

  EXPECT_TRUE(meshA.has_valid_connectivity());
  EXPECT_TRUE(verify_manifold(meshA));
  EXPECT_EQ(meshA.vertex_count(), 11U);
  double area = 0.0;
  for (const FaceHandle face : meshA.faces())
  {
    const Vec3 normal = face_normal(meshA, face);
    EXPECT_GT(normal[2], 0.0) << "face " << face.get_value();
    area += normal[2] / 2.0;
  }
  EXPECT_NEAR(area, 72.0, tolerance);
  expect_closed_manifold(meshB);
  expect_matching_curves(meshA, meshB, result);
  EXPECT_EQ(result.intersectionEdgesA.size(), 8U);
  EXPECT_TRUE(forms_closed_loops(meshA, result.intersectionEdgesA));
  EXPECT_EQ(curve_component_count(meshA, result.intersectionEdgesA), 1U);
}

TEST(MeshCorefineTest, isolated_touch_point_becomes_a_vertex_without_edges)
{
  // A tetrahedron stands on its apex inside the interior of a box's top face.
  const Result result = corefine(make_box(Vec3{0.0, 0.0, 0.0}, Vec3{2.0, 2.0, 2.0}),
                                 make_mesh({Vec3{1.0, 0.5, 2.0}, Vec3{0.0, 0.0, 3.0}, Vec3{2.0, 0.0, 3.0}, Vec3{1.0, 2.0, 3.0}},
                                           {Triangle{0, 2, 1}, Triangle{0, 3, 2}, Triangle{0, 1, 3}, Triangle{1, 2, 3}}));
  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result.intersectionEdgesA.empty());
  EXPECT_EQ(result.meshA.vertex_count(), 9U);
  EXPECT_EQ(result.meshA.face_count(), 14U);
  EXPECT_EQ(result.meshB.vertex_count(), 4U);
  expect_closed_manifold(result.meshA);
}

TEST(MeshCorefineTest, disjoint_boxes_stay_unchanged)
{
  const Result result = corefine(make_box(Vec3{0.0, 0.0, 0.0}, Vec3{1.0, 1.0, 1.0}), make_box(Vec3{2.0, 0.0, 0.0}, Vec3{3.0, 1.0, 1.0}));
  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result.intersectionEdgesA.empty());
  EXPECT_TRUE(result.intersectionEdgesB.empty());
  EXPECT_EQ(result.meshA.face_count(), 12U);
  EXPECT_EQ(result.meshB.face_count(), 12U);
}

TEST(MeshCorefineTest, corefining_again_changes_nothing)
{
  Result first = corefine(make_box(Vec3{0.0, 0.0, 0.0}, Vec3{2.0, 2.0, 2.0}), make_box(Vec3{1.0, 0.5, 0.25}, Vec3{3.0, 2.5, 2.25}));
  ASSERT_TRUE(first.has_value());
  const std::size_t vertexCountA = first.meshA.vertex_count();
  const std::size_t faceCountB = first.meshB.face_count();

  // The curve already runs along edges of both meshes, so it consists of vertices and edges only.
  const Result second = corefine(std::move(first.meshA), std::move(first.meshB));
  ASSERT_TRUE(second.has_value());
  EXPECT_EQ(second.meshA.vertex_count(), vertexCountA);
  EXPECT_EQ(second.meshB.face_count(), faceCountB);
  EXPECT_EQ(second.intersectionEdgesA.size(), first.intersectionEdgesA.size());
}

// Positions by vertex handle and corners by face handle: equal for two meshes iff they are the same
// mesh, handle for handle. Positions are kept as bits, so that a NaN coordinate equals itself.
struct MeshGeometry
{
  std::vector<std::array<std::uint64_t, 3>> positionBits;
  std::vector<std::array<std::uint32_t, 3>> faceCorners;

  bool operator==(const MeshGeometry&) const = default;
};

MeshGeometry geometry_of(const Mesh& mesh)
{
  MeshGeometry geometry;
  for (const VertexHandle vertex : mesh.vertices())
  {
    const Vec3& position = mesh.get_position(vertex);
    geometry.positionBits.push_back(
        {std::bit_cast<std::uint64_t>(position[0]), std::bit_cast<std::uint64_t>(position[1]), std::bit_cast<std::uint64_t>(position[2])});
  }
  for (const FaceHandle face : mesh.faces())
  {
    const auto corners = mesh.vertices_around_face(face);
    geometry.faceCorners.push_back({corners[0].get_value(), corners[1].get_value(), corners[2].get_value()});
  }
  return geometry;
}

// Corefines copies of meshA and meshB, expects the failure, and expects both meshes back unchanged.
void expect_failure_hands_back_the_meshes(const Mesh& meshA, const Mesh& meshB, const CorefineStatus expected)
{
  const Result result = corefine(Mesh{meshA}, Mesh{meshB});
  EXPECT_EQ(result.error, expected);
  EXPECT_FALSE(result.has_value());
  EXPECT_TRUE(result.intersectionEdgesA.empty());
  EXPECT_TRUE(result.intersectionEdgesB.empty());
  EXPECT_TRUE(geometry_of(result.meshA) == geometry_of(meshA));
  EXPECT_TRUE(geometry_of(result.meshB) == geometry_of(meshB));
}

TEST(MeshCorefineTest, failures_hand_back_the_meshes)
{
  const Mesh box = make_box(Vec3{0.0, 0.0, 0.0}, Vec3{1.0, 1.0, 1.0});
  const double nan = std::numeric_limits<double>::quiet_NaN();
  const double infinity = std::numeric_limits<double>::infinity();

  const Mesh withNan = make_mesh({Vec3{0.5, 0.5, 0.5}, Vec3{nan, 0.0, 0.0}, Vec3{0.0, 2.0, 0.0}}, {Triangle{0, 1, 2}});
  expect_failure_hands_back_the_meshes(box, withNan, CorefineStatus::NonFiniteCoordinates);
  const Mesh withInfinity = make_mesh({Vec3{0.5, 0.5, 0.5}, Vec3{infinity, 0.0, 0.0}, Vec3{0.0, 2.0, 0.0}}, {Triangle{0, 1, 2}});
  expect_failure_hands_back_the_meshes(withInfinity, box, CorefineStatus::NonFiniteCoordinates);

  // A collinear triangle through the box: zero area, so it has no plane.
  const Mesh collinear = make_mesh({Vec3{-1.0, 0.5, 0.5}, Vec3{0.5, 0.5, 0.5}, Vec3{2.0, 0.5, 0.5}}, {Triangle{0, 1, 2}});
  expect_failure_hands_back_the_meshes(box, collinear, CorefineStatus::DegenerateFace);
  expect_failure_hands_back_the_meshes(collinear, box, CorefineStatus::DegenerateFace);
  // Rejected even far away from the other mesh.
  const Mesh farCollinear = make_mesh({Vec3{9.0, 0.5, 0.5}, Vec3{10.0, 0.5, 0.5}, Vec3{11.0, 0.5, 0.5}}, {Triangle{0, 1, 2}});
  expect_failure_hands_back_the_meshes(box, farCollinear, CorefineStatus::DegenerateFace);
}

TEST(MeshCorefineTest, coincident_points_are_a_degenerate_intersection)
{
  // Near 2^20 doubles are 2^-32 apart. A tetrahedron's apex lies one such step above the box's top
  // face, so its three side edges cross the face within a fraction of a step of the apex, and all
  // three crossings round to one position.
  const double center = 1048576.0;
  const double step = std::ldexp(1.0, -32);
  ASSERT_EQ(std::nextafter(center, 2.0 * center), center + step);
  const Mesh box = make_box(Vec3{center - 1.0, center - 1.0, center - 1.0}, Vec3{center + 1.0, center + 1.0, center});
  // The apex stays off the top face's diagonal x = y.
  const Mesh tetrahedron = make_mesh({Vec3{center + 0.25, center - 0.25, center + step},
                                      Vec3{center + 0.05, center - 0.45, center - 0.9},
                                      Vec3{center + 0.45, center - 0.35, center - 0.9},
                                      Vec3{center + 0.2, center - 0.05, center - 0.9}},
                                     {Triangle{0, 1, 2}, Triangle{0, 2, 3}, Triangle{0, 3, 1}, Triangle{1, 3, 2}});
  expect_failure_hands_back_the_meshes(box, tetrahedron, CorefineStatus::DegenerateIntersection);
}

TEST(MeshCorefineTest, explicit_copy_keeps_the_operand)
{
  const Mesh kept = make_box(Vec3{0.0, 0.0, 0.0}, Vec3{2.0, 2.0, 2.0});
  const MeshGeometry before = geometry_of(kept);
  const Result result = corefine(Mesh{kept}, make_box(Vec3{1.0, 0.5, 0.25}, Vec3{3.0, 2.5, 2.25}));
  ASSERT_TRUE(result.has_value());
  EXPECT_GT(result.meshA.vertex_count(), kept.vertex_count());
  EXPECT_TRUE(geometry_of(kept) == before);
}

// Whether corefine accepts lvalues; it must not, so that every copy is visible at the call site. A
// concept, so that the ill-formed call yields false instead of a compile error.
template <typename TMesh>
concept CorefinesLvalues = requires(TMesh& first, TMesh& second) { corefine(first, second); };
template <typename TMesh>
concept CorefinesRvalues = requires(TMesh&& first, TMesh&& second) { corefine(std::move(first), std::move(second)); };
static_assert(!CorefinesLvalues<Mesh>);
static_assert(CorefinesRvalues<Mesh>);

} // namespace MeshCorefineTesting
