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
  for (std::uint32_t i = 0; i < 8; ++i)
  {
    corners.push_back(Vec3{(i & 1U) != 0 ? max[0] : min[0], (i & 2U) != 0 ? max[1] : min[1], (i & 4U) != 0 ? max[2] : min[2]});
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
    count += degrees[vertex] > 0 && findRoot(vertex) == vertex ? 1 : 0;
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
  Mesh meshA = make_box(minA, maxA);
  Mesh meshB = make_box(minB, maxB);
  const double volumeA = signed_volume(meshA);
  const double volumeB = signed_volume(meshB);

  const Result result = corefine(meshA, meshB);
  ASSERT_TRUE(result.has_value());
  ASSERT_FALSE(result.intersectionEdgesA.empty());

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

TEST(MeshCorefineTest, cylinder_through_box_gets_two_loops)
{
  // The axis avoids the box diagonals x = y, so no side edge of the cylinder hits one exactly.
  const Vec3 minA{0.0, 0.0, 0.0};
  const Vec3 maxA{2.0, 2.0, 2.0};
  Mesh meshA = make_box(minA, maxA);
  auto cylinder = make_triangle_mesh(Cylinder<double>{Segment3<double>{Vec3{0.7, 1.2, -1.0}, Vec3{0.7, 1.2, 3.0}}, 0.45}, 16);
  ASSERT_TRUE(cylinder.has_value());
  Mesh meshB = std::move(cylinder.mesh);
  const double volumeA = signed_volume(meshA);
  const double volumeB = signed_volume(meshB);

  const Result result = corefine(meshA, meshB);
  ASSERT_TRUE(result.has_value());

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
  Mesh meshA = make_box(minA, maxA);
  Mesh meshB = make_box(minB, maxB);

  const Result result = corefine(meshA, meshB);
  ASSERT_TRUE(result.has_value());
  ASSERT_FALSE(result.intersectionEdgesA.empty());

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
  Mesh meshA = make_box(Vec3{0.0, 0.0, 0.0}, Vec3{1.0, 1.0, 1.0});
  Mesh meshB = make_box(Vec3{1.0, 0.0, 0.0}, Vec3{2.0, 1.0, 1.0});

  const Result result = corefine(meshA, meshB);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(meshA.vertex_count(), 8U);
  EXPECT_EQ(meshB.vertex_count(), 8U);
  // The four sides of the shared square and its diagonal.
  EXPECT_EQ(result.intersectionEdgesA.size(), 5U);
  expect_matching_curves(meshA, meshB, result);
}

TEST(MeshCorefineTest, triangle_pierced_by_box_recovers_the_loop_by_flips)
{
  // The box pierces the triangle's interior: its four vertical edges and four side diagonals cross
  // the plane z = 0, so eight points inside one face must be joined into an octagon.
  Mesh meshA = make_mesh({Vec3{-4.0, -4.0, 0.0}, Vec3{8.0, -4.0, 0.0}, Vec3{-4.0, 8.0, 0.0}}, {Triangle{0, 1, 2}});
  Mesh meshB = make_box(Vec3{0.0, 0.0, -1.0}, Vec3{1.0, 1.0, 1.0});

  const Result result = corefine(meshA, meshB);
  ASSERT_TRUE(result.has_value());

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
  Mesh meshA = make_box(Vec3{0.0, 0.0, 0.0}, Vec3{2.0, 2.0, 2.0});
  Mesh meshB = make_mesh({Vec3{1.0, 0.5, 2.0}, Vec3{0.0, 0.0, 3.0}, Vec3{2.0, 0.0, 3.0}, Vec3{1.0, 2.0, 3.0}},
                         {Triangle{0, 2, 1}, Triangle{0, 3, 2}, Triangle{0, 1, 3}, Triangle{1, 2, 3}});

  const Result result = corefine(meshA, meshB);
  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result.intersectionEdgesA.empty());
  EXPECT_EQ(meshA.vertex_count(), 9U);
  EXPECT_EQ(meshA.face_count(), 14U);
  EXPECT_EQ(meshB.vertex_count(), 4U);
  expect_closed_manifold(meshA);
}

TEST(MeshCorefineTest, disjoint_boxes_stay_unchanged)
{
  Mesh meshA = make_box(Vec3{0.0, 0.0, 0.0}, Vec3{1.0, 1.0, 1.0});
  Mesh meshB = make_box(Vec3{2.0, 0.0, 0.0}, Vec3{3.0, 1.0, 1.0});

  const Result result = corefine(meshA, meshB);
  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result.intersectionEdgesA.empty());
  EXPECT_TRUE(result.intersectionEdgesB.empty());
  EXPECT_EQ(meshA.face_count(), 12U);
  EXPECT_EQ(meshB.face_count(), 12U);
}

TEST(MeshCorefineTest, corefining_again_changes_nothing)
{
  Mesh meshA = make_box(Vec3{0.0, 0.0, 0.0}, Vec3{2.0, 2.0, 2.0});
  Mesh meshB = make_box(Vec3{1.0, 0.5, 0.25}, Vec3{3.0, 2.5, 2.25});
  const Result first = corefine(meshA, meshB);
  ASSERT_TRUE(first.has_value());
  const std::size_t vertexCountA = meshA.vertex_count();
  const std::size_t faceCountB = meshB.face_count();

  // The curve already runs along edges of both meshes, so it consists of vertices and edges only.
  const Result second = corefine(meshA, meshB);
  ASSERT_TRUE(second.has_value());
  EXPECT_EQ(meshA.vertex_count(), vertexCountA);
  EXPECT_EQ(meshB.face_count(), faceCountB);
  EXPECT_EQ(second.intersectionEdgesA.size(), first.intersectionEdgesA.size());
}

TEST(MeshCorefineTest, failures_leave_both_meshes_untouched)
{
  const Mesh box = make_box(Vec3{0.0, 0.0, 0.0}, Vec3{1.0, 1.0, 1.0});
  const double nan = std::numeric_limits<double>::quiet_NaN();

  Mesh meshA = box;
  Mesh nonFinite = make_mesh({Vec3{0.5, 0.5, 0.5}, Vec3{nan, 0.0, 0.0}, Vec3{0.0, 2.0, 0.0}}, {Triangle{0, 1, 2}});
  const Result nonFiniteResult = corefine(meshA, nonFinite);
  EXPECT_EQ(nonFiniteResult.error, CorefineStatus::NonFiniteCoordinates);
  EXPECT_TRUE(nonFiniteResult.intersectionEdgesA.empty());
  EXPECT_EQ(meshA.vertex_count(), box.vertex_count());
  EXPECT_EQ(meshA.face_count(), box.face_count());

  // A collinear triangle through the box: zero area, so it has no plane.
  Mesh collinear = make_mesh({Vec3{-1.0, 0.5, 0.5}, Vec3{0.5, 0.5, 0.5}, Vec3{2.0, 0.5, 0.5}}, {Triangle{0, 1, 2}});
  const Result collinearResult = corefine(meshA, collinear);
  EXPECT_EQ(collinearResult.error, CorefineStatus::DegenerateFace);
  EXPECT_EQ(meshA.vertex_count(), box.vertex_count());
  EXPECT_EQ(collinear.vertex_count(), 3U);
}

} // namespace MeshCorefineTesting
