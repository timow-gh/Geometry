#include <Geometry/Mesh/MeshFromTriangles.hpp>
#include <Geometry/Mesh/TriangleHalfedgeMesh.hpp>
#include <Geometry/Mesh/detail/BooleanIntersection.hpp>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <vector>

using namespace Geometry;

// A named namespace rather than an anonymous one, so that its aliases cannot hide library names
// (MSVC C4459).
namespace BooleanIntersectionTesting
{

using Mesh = TriangleHalfedgeMesh3d;
using FaceHandle = Mesh::FaceHandle;
using EdgeHandle = Mesh::EdgeHandle;
using Vec3 = Mesh::vec_t;
using Triangle = std::array<std::uint32_t, 3>;
using Key = detail::IntersectionKey<std::uint32_t>;
using Simplex = detail::MeshSimplex<std::uint32_t>;
using Kind = detail::SimplexKind;
using Intersection = detail::FaceIntersection<double, std::uint32_t>;
using Graph = detail::IntersectionGraph<double, std::uint32_t>;
using GraphStatus = detail::IntersectionGraphStatus;

constexpr double tolerance = 1e-12;

Mesh make_mesh(const std::vector<Vec3>& positions, const std::vector<Triangle>& triangles)
{
  auto result = make_mesh_from_triangles(std::span<const Vec3>{positions}, std::span<const Triangle>{triangles});
  EXPECT_TRUE(result.has_value());
  return std::move(result.mesh);
}

// One face; vertex i is the i-th argument.
Mesh make_triangle(const Vec3& first, const Vec3& second, const Vec3& third)
{
  return make_mesh({first, second, third}, {Triangle{0, 1, 2}});
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

Simplex vertex(const std::uint32_t index)
{
  return Simplex{Kind::Vertex, index};
}

Simplex face(const std::uint32_t index)
{
  return Simplex{Kind::Face, index};
}

// The edge joining two vertices, which must exist.
Simplex edge(const Mesh& mesh, const std::uint32_t first, const std::uint32_t second)
{
  for (const EdgeHandle handle : mesh.edges())
  {
    const auto stored = mesh.get_edge(handle).halfedge;
    const std::uint32_t source = mesh.source_vertex(stored).get_value();
    const std::uint32_t target = mesh.target_vertex(stored).get_value();
    if ((source == first && target == second) || (source == second && target == first))
    {
      return Simplex{Kind::Edge, handle.get_value()};
    }
  }
  ADD_FAILURE() << "no edge " << first << "-" << second;
  return {};
}

Intersection intersect(const Mesh& meshA, const Mesh& meshB)
{
  return detail::intersect_faces(meshA, FaceHandle{0}, meshB, FaceHandle{0});
}

std::vector<Key> sorted_keys(const Intersection& intersection)
{
  std::vector<Key> keys;
  for (const auto& point : intersection.points)
  {
    keys.push_back(point.key);
  }
  std::ranges::sort(keys);
  return keys;
}

std::vector<Key> sorted(std::vector<Key> keys)
{
  std::ranges::sort(keys);
  return keys;
}

void expect_near(const Vec3& actual, const Vec3& expected)
{
  EXPECT_NEAR(actual[0], expected[0], tolerance);
  EXPECT_NEAR(actual[1], expected[1], tolerance);
  EXPECT_NEAR(actual[2], expected[2], tolerance);
}

// The point with the key, which must exist.
const detail::ImplicitPoint<double>& point_of(const Intersection& intersection, const Key& key)
{
  const std::size_t index = intersection.find_point(key);
  EXPECT_LT(index, intersection.points.size());
  return intersection.points[std::min(index, intersection.points.size() - 1)].point;
}

bool has_segment(const Intersection& intersection, const Key& first, const Key& second)
{
  const std::size_t firstIndex = intersection.find_point(first);
  const std::size_t secondIndex = intersection.find_point(second);
  return std::ranges::any_of(intersection.segments, [&](const Intersection::Segment& segment) {
    return (segment[0] == firstIndex && segment[1] == secondIndex) || (segment[0] == secondIndex && segment[1] == firstIndex);
  });
}

// Every intersection test below uses this triangle as A: the right triangle with legs 4 in z = 0.
Mesh make_reference_triangle()
{
  return make_triangle(Vec3{0.0, 0.0, 0.0}, Vec3{4.0, 0.0, 0.0}, Vec3{0.0, 4.0, 0.0});
}

TEST(BooleanIntersectionTest, piercing_triangles_meet_in_one_segment)
{
  // B stands in the plane y = 1: its edge 0-1 pierces A's interior, A's edge 1-2 pierces B's.
  const Mesh meshA = make_reference_triangle();
  const Mesh meshB = make_triangle(Vec3{1.0, 1.0, -1.0}, Vec3{1.0, 1.0, 1.0}, Vec3{5.0, 1.0, 0.0});
  const Intersection intersection = intersect(meshA, meshB);

  const Key throughA{face(0), edge(meshB, 0, 1)};
  const Key throughB{edge(meshA, 1, 2), face(0)};
  EXPECT_TRUE(intersection.consistent);
  EXPECT_FALSE(intersection.coplanar);
  EXPECT_EQ(sorted_keys(intersection), sorted({throughA, throughB}));
  ASSERT_EQ(intersection.segments.size(), 1U);
  EXPECT_TRUE(has_segment(intersection, throughA, throughB));
  EXPECT_EQ(point_of(intersection, throughA).kind(), detail::ImplicitPointKind::EdgePlane);
  expect_near(point_of(intersection, throughA).position(), Vec3{1.0, 1.0, 0.0});
  expect_near(point_of(intersection, throughB).position(), Vec3{3.0, 1.0, 0.0});
}

TEST(BooleanIntersectionTest, edge_crossing_edge_is_found_once)
{
  // B's edge 0-1 crosses A's edge 0-1 at (2, 0, 0); both faces see that point from their side.
  const Mesh meshA = make_reference_triangle();
  const Mesh meshB = make_triangle(Vec3{2.0, -1.0, -1.0}, Vec3{2.0, 1.0, 1.0}, Vec3{2.0, 4.0, -1.0});
  const Intersection intersection = intersect(meshA, meshB);

  const Key edgeOnEdge{edge(meshA, 0, 1), edge(meshB, 0, 1)};
  const Key throughB{edge(meshA, 1, 2), face(0)};
  EXPECT_TRUE(intersection.consistent);
  EXPECT_EQ(sorted_keys(intersection), sorted({edgeOnEdge, throughB}));
  EXPECT_TRUE(has_segment(intersection, edgeOnEdge, throughB));
  expect_near(point_of(intersection, edgeOnEdge).position(), Vec3{2.0, 0.0, 0.0});
  expect_near(point_of(intersection, throughB).position(), Vec3{2.0, 2.0, 0.0});
}

TEST(BooleanIntersectionTest, edge_on_face_is_a_segment_between_its_vertices)
{
  // B's edge 0-1 lies in A's interior; B rises above A from it.
  const Mesh meshA = make_reference_triangle();
  const Mesh meshB = make_triangle(Vec3{1.0, 1.0, 0.0}, Vec3{2.0, 1.0, 0.0}, Vec3{1.0, 1.0, 3.0});
  const Intersection intersection = intersect(meshA, meshB);

  const Key first{face(0), vertex(0)};
  const Key second{face(0), vertex(1)};
  EXPECT_TRUE(intersection.consistent);
  EXPECT_EQ(sorted_keys(intersection), sorted({first, second}));
  EXPECT_TRUE(has_segment(intersection, first, second));
  EXPECT_EQ(point_of(intersection, first).kind(), detail::ImplicitPointKind::Explicit);
}

TEST(BooleanIntersectionTest, vertex_on_face_is_an_isolated_touch_point)
{
  // B touches A's interior with its vertex 0 and stays above A otherwise.
  const Mesh meshA = make_reference_triangle();
  const Mesh meshB = make_triangle(Vec3{1.0, 1.0, 0.0}, Vec3{2.0, 1.0, 1.0}, Vec3{1.0, 2.0, 1.0});
  const Intersection intersection = intersect(meshA, meshB);

  EXPECT_TRUE(intersection.consistent);
  EXPECT_EQ(sorted_keys(intersection), sorted({Key{face(0), vertex(0)}}));
  EXPECT_TRUE(intersection.segments.empty());
}

TEST(BooleanIntersectionTest, vertex_on_vertex_has_one_key_from_both_sides)
{
  // B's vertex 0 coincides with A's vertex 0; otherwise B is above A and A is on one side of B.
  const Mesh meshA = make_reference_triangle();
  const Mesh meshB = make_triangle(Vec3{0.0, 0.0, 0.0}, Vec3{-2.0, 0.0, 1.0}, Vec3{0.0, -2.0, 1.0});
  const Intersection intersection = intersect(meshA, meshB);

  EXPECT_TRUE(intersection.consistent);
  EXPECT_EQ(sorted_keys(intersection), sorted({Key{vertex(0), vertex(0)}}));
  EXPECT_TRUE(intersection.segments.empty());
}

TEST(BooleanIntersectionTest, collinear_edges_overlap_in_a_segment)
{
  // B stands in the plane y = 0 on A's edge 0-1; the edges overlap on x in [1, 4]. B's projection
  // runs clockwise, which the side tests must normalize.
  const Mesh meshA = make_reference_triangle();
  const Mesh meshB = make_triangle(Vec3{1.0, 0.0, 0.0}, Vec3{6.0, 0.0, 0.0}, Vec3{1.0, 0.0, 3.0});
  const Intersection intersection = intersect(meshA, meshB);

  const Key start{edge(meshA, 0, 1), vertex(0)};
  const Key end{vertex(1), edge(meshB, 0, 1)};
  EXPECT_TRUE(intersection.consistent);
  EXPECT_FALSE(intersection.coplanar);
  EXPECT_EQ(sorted_keys(intersection), sorted({start, end}));
  EXPECT_TRUE(has_segment(intersection, start, end));
}

TEST(BooleanIntersectionTest, coplanar_overlap_reports_polygon_corners_and_clipped_edges)
{
  // B's corner 0 lies inside A, and A's edge 1-2 cuts B's edges 0-1 and 2-0: the overlap is the
  // triangle (1, 1), (3, 1), (1, 3).
  const Mesh meshA = make_reference_triangle();
  const Mesh meshB = make_triangle(Vec3{1.0, 1.0, 0.0}, Vec3{5.0, 1.0, 0.0}, Vec3{1.0, 5.0, 0.0});
  const Intersection intersection = intersect(meshA, meshB);

  const Key inside{face(0), vertex(0)};
  const Key crossingB01{edge(meshA, 1, 2), edge(meshB, 0, 1)};
  const Key crossingB20{edge(meshA, 1, 2), edge(meshB, 2, 0)};
  EXPECT_TRUE(intersection.consistent);
  EXPECT_TRUE(intersection.coplanar);
  EXPECT_EQ(sorted_keys(intersection), sorted({inside, crossingB01, crossingB20}));
  EXPECT_EQ(point_of(intersection, crossingB01).kind(), detail::ImplicitPointKind::EdgeEdge);
  expect_near(point_of(intersection, crossingB01).position(), Vec3{3.0, 1.0, 0.0});
  expect_near(point_of(intersection, crossingB20).position(), Vec3{1.0, 3.0, 0.0});

  // A's edge 1-2 clipped to B, and B's edges 0-1 and 2-0 clipped to A.
  EXPECT_EQ(intersection.segments.size(), 3U);
  EXPECT_TRUE(has_segment(intersection, crossingB01, crossingB20));
  EXPECT_TRUE(has_segment(intersection, inside, crossingB01));
  EXPECT_TRUE(has_segment(intersection, inside, crossingB20));
}

TEST(BooleanIntersectionTest, coplanar_star_has_six_crossings)
{
  // Two triangles forming a six-pointed star: no corner of one lies in the other.
  const Mesh meshA = make_triangle(Vec3{0.0, 0.0, 0.0}, Vec3{12.0, 0.0, 0.0}, Vec3{6.0, 12.0, 0.0});
  const Mesh meshB = make_triangle(Vec3{0.0, 8.0, 0.0}, Vec3{6.0, -4.0, 0.0}, Vec3{12.0, 8.0, 0.0});
  const Intersection intersection = intersect(meshA, meshB);

  EXPECT_TRUE(intersection.consistent);
  EXPECT_TRUE(intersection.coplanar);
  ASSERT_EQ(intersection.points.size(), 6U);
  EXPECT_EQ(intersection.segments.size(), 6U);
  for (const auto& point : intersection.points)
  {
    EXPECT_EQ(point.key.simplexA.kind, Kind::Edge);
    EXPECT_EQ(point.key.simplexB.kind, Kind::Edge);
    EXPECT_EQ(point.point.position()[2], 0.0);
  }
  expect_near(point_of(intersection, Key{edge(meshA, 0, 1), edge(meshB, 0, 1)}).position(), Vec3{4.0, 0.0, 0.0});
  expect_near(point_of(intersection, Key{edge(meshA, 2, 0), edge(meshB, 0, 1)}).position(), Vec3{2.0, 4.0, 0.0});
}

TEST(BooleanIntersectionTest, identical_triangles_share_every_vertex_and_edge)
{
  const Vec3 first{0.0, 0.0, 0.0};
  const Vec3 second{4.0, 1.0, 2.0};
  const Vec3 third{1.0, 3.0, -1.0};
  const Mesh meshA = make_triangle(first, second, third);
  const std::vector<Key> expected = sorted({Key{vertex(0), vertex(0)}, Key{vertex(1), vertex(1)}, Key{vertex(2), vertex(2)}});

  for (const bool reversed : {false, true})
  {
    // Coplanar contact of two solids: the same triangle with opposite winding.
    const Mesh meshB = reversed ? make_mesh({first, second, third}, {Triangle{0, 2, 1}}) : make_triangle(first, second, third);
    const Intersection intersection = intersect(meshA, meshB);

    EXPECT_TRUE(intersection.consistent);
    EXPECT_TRUE(intersection.coplanar);
    EXPECT_EQ(sorted_keys(intersection), expected);
    EXPECT_EQ(intersection.segments.size(), 3U);
    EXPECT_TRUE(has_segment(intersection, Key{vertex(0), vertex(0)}, Key{vertex(1), vertex(1)}));
    EXPECT_TRUE(has_segment(intersection, Key{vertex(1), vertex(1)}, Key{vertex(2), vertex(2)}));
    EXPECT_TRUE(has_segment(intersection, Key{vertex(2), vertex(2)}, Key{vertex(0), vertex(0)}));
  }
}

TEST(BooleanIntersectionTest, disjoint_triangles_have_no_points)
{
  const Mesh meshA = make_reference_triangle();

  // Strictly above A's plane.
  const Intersection above = intersect(meshA, make_triangle(Vec3{0.0, 0.0, 1.0}, Vec3{4.0, 0.0, 2.0}, Vec3{0.0, 4.0, 1.0}));
  EXPECT_TRUE(above.points.empty());

  // In A's plane, beyond its hypotenuse.
  const Intersection coplanar = intersect(meshA, make_triangle(Vec3{3.0, 3.0, 0.0}, Vec3{6.0, 3.0, 0.0}, Vec3{3.0, 6.0, 0.0}));
  EXPECT_TRUE(coplanar.coplanar);
  EXPECT_TRUE(coplanar.points.empty());

  // Each face crosses the other's plane, but outside the other face.
  const Intersection apart = intersect(meshA, make_triangle(Vec3{5.0, 1.0, -1.0}, Vec3{5.0, 1.0, 1.0}, Vec3{6.0, 1.0, 0.0}));
  EXPECT_FALSE(apart.coplanar);
  EXPECT_TRUE(apart.points.empty());
}

// Degree of every point in the segment graph.
std::vector<std::size_t> point_degrees(const Graph& graph)
{
  std::vector<std::size_t> degrees(graph.points.size(), 0);
  for (const Graph::Segment& segment : graph.segments)
  {
    ++degrees[segment[0]];
    ++degrees[segment[1]];
  }
  return degrees;
}

// Whether point lies on the surface of the box [min, max].
bool is_on_box_surface(const Vec3& point, const Vec3& min, const Vec3& max)
{
  bool onSide = false;
  for (std::uint8_t axis = 0; axis < 3; ++axis)
  {
    if (point[axis] < min[axis] - tolerance || point[axis] > max[axis] + tolerance)
    {
      return false;
    }
    onSide = onSide || std::abs(point[axis] - min[axis]) <= tolerance || std::abs(point[axis] - max[axis]) <= tolerance;
  }
  return onSide;
}

TEST(BooleanIntersectionTest, graph_of_overlapping_boxes_is_closed_loops_on_both_surfaces)
{
  // Dyadic offsets keep every predicate exact and avoid coplanar faces and shared edges.
  const Vec3 minA{0.0, 0.0, 0.0};
  const Vec3 maxA{2.0, 2.0, 2.0};
  const Vec3 minB{1.0, 0.5, 0.25};
  const Vec3 maxB{3.0, 2.5, 2.25};
  const Mesh meshA = make_box(minA, maxA);
  const Mesh meshB = make_box(minB, maxB);

  const auto result = detail::compute_intersection_graph(meshA, meshB);
  ASSERT_TRUE(result.has_value());
  const Graph& graph = result.graph;
  ASSERT_FALSE(graph.points.empty());
  EXPECT_TRUE(std::ranges::is_sorted(graph.keys));
  EXPECT_EQ(std::ranges::adjacent_find(graph.keys), graph.keys.end());

  for (const std::size_t degree : point_degrees(graph))
  {
    EXPECT_EQ(degree, 2U);
  }
  for (std::size_t i = 0; i < graph.points.size(); ++i)
  {
    EXPECT_TRUE(is_on_box_surface(graph.points[i].position(), minA, maxA)) << "point " << i;
    EXPECT_TRUE(is_on_box_surface(graph.points[i].position(), minB, maxB)) << "point " << i;
    EXPECT_EQ(graph.find_point(graph.keys[i]), i);
  }

  // Every segment is listed under at least one face of each mesh, and every point inside an edge
  // or face is listed under it.
  std::vector<bool> listedOnA(graph.segments.size(), false);
  std::vector<bool> listedOnB(graph.segments.size(), false);
  for (const FaceHandle faceHandle : meshA.faces())
  {
    for (const std::size_t segment : graph.onMeshA.segmentsByFace.items_of(faceHandle.get_value()))
    {
      listedOnA[segment] = true;
    }
  }
  for (const FaceHandle faceHandle : meshB.faces())
  {
    for (const std::size_t segment : graph.onMeshB.segmentsByFace.items_of(faceHandle.get_value()))
    {
      listedOnB[segment] = true;
    }
  }
  EXPECT_TRUE(std::ranges::all_of(listedOnA, [](const bool listed) { return listed; }));
  EXPECT_TRUE(std::ranges::all_of(listedOnB, [](const bool listed) { return listed; }));
  for (std::size_t i = 0; i < graph.keys.size(); ++i)
  {
    const Simplex simplexA = graph.keys[i].simplexA;
    const auto& groupsA = simplexA.kind == Kind::Edge ? graph.onMeshA.pointsByEdge : graph.onMeshA.pointsByFace;
    if (simplexA.kind != Kind::Vertex)
    {
      const auto items = groupsA.items_of(simplexA.index);
      EXPECT_NE(std::ranges::find(items, i), items.end()) << "point " << i;
    }
  }
}

TEST(BooleanIntersectionTest, graph_of_disjoint_boxes_is_empty)
{
  const Mesh meshA = make_box(Vec3{0.0, 0.0, 0.0}, Vec3{1.0, 1.0, 1.0});
  const Mesh meshB = make_box(Vec3{2.0, 0.0, 0.0}, Vec3{3.0, 1.0, 1.0});

  const auto result = detail::compute_intersection_graph(meshA, meshB);
  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result.graph.keys.empty());
  EXPECT_TRUE(result.graph.segments.empty());
  EXPECT_EQ(result.graph.onMeshA.segmentsByFace.size(), 0U);
}

TEST(BooleanIntersectionTest, graph_of_boxes_sharing_a_face_matches_their_corners)
{
  // B's min-x face is A's max-x face, triangulated along the same diagonal.
  const Mesh meshA = make_box(Vec3{0.0, 0.0, 0.0}, Vec3{1.0, 1.0, 1.0});
  const Mesh meshB = make_box(Vec3{1.0, 0.0, 0.0}, Vec3{2.0, 1.0, 1.0});

  const auto result = detail::compute_intersection_graph(meshA, meshB);
  ASSERT_TRUE(result.has_value());
  const Graph& graph = result.graph;

  // A's corner i + 1 is B's corner i for the four corners of the shared face (odd i in A).
  const std::vector<Key> expected =
      sorted({Key{vertex(1), vertex(0)}, Key{vertex(3), vertex(2)}, Key{vertex(5), vertex(4)}, Key{vertex(7), vertex(6)}});
  EXPECT_EQ(graph.keys, expected);
  // The four sides of the square and its diagonal.
  EXPECT_EQ(graph.segments.size(), 5U);
}

TEST(BooleanIntersectionTest, graph_of_coplanar_contact_with_crossing_diagonals_stays_in_the_plane)
{
  // B's min-x face lies in A's max-x face but is shifted, so diagonals and sides cross.
  const Mesh meshA = make_box(Vec3{0.0, 0.0, 0.0}, Vec3{1.0, 1.0, 1.0});
  const Mesh meshB = make_box(Vec3{1.0, 0.5, 0.25}, Vec3{2.0, 1.5, 1.25});

  const auto result = detail::compute_intersection_graph(meshA, meshB);
  ASSERT_TRUE(result.has_value());
  const Graph& graph = result.graph;
  ASSERT_FALSE(graph.points.empty());

  bool hasEdgeEdgePoint = false;
  for (const auto& point : graph.points)
  {
    // Lifting along an edge in the plane x = 1 keeps x exact.
    EXPECT_EQ(point.position()[0], 1.0);
    hasEdgeEdgePoint = hasEdgeEdgePoint || point.kind() == detail::ImplicitPointKind::EdgeEdge;
  }
  EXPECT_TRUE(hasEdgeEdgePoint);
}

TEST(BooleanIntersectionTest, graph_keeps_isolated_touch_point)
{
  // A tetrahedron stands on its apex inside the interior of a box's top triangle (4, 5, 7).
  const Mesh meshA = make_box(Vec3{0.0, 0.0, 0.0}, Vec3{2.0, 2.0, 2.0});
  const Mesh meshB = make_mesh({Vec3{1.0, 0.5, 2.0}, Vec3{0.0, 0.0, 3.0}, Vec3{2.0, 0.0, 3.0}, Vec3{1.0, 2.0, 3.0}},
                               {Triangle{0, 2, 1}, Triangle{0, 3, 2}, Triangle{0, 1, 3}, Triangle{1, 2, 3}});

  const auto result = detail::compute_intersection_graph(meshA, meshB);
  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(result.graph.keys.size(), 1U);
  EXPECT_EQ(result.graph.keys[0].simplexA.kind, Kind::Face);
  EXPECT_EQ(result.graph.keys[0].simplexB, vertex(0));
  EXPECT_TRUE(result.graph.segments.empty());
  EXPECT_EQ(result.graph.onMeshA.pointsByFace.items_of(result.graph.keys[0].simplexA.index).size(), 1U);
}

TEST(BooleanIntersectionTest, graph_rejects_non_finite_positions)
{
  const Mesh meshA = make_box(Vec3{0.0, 0.0, 0.0}, Vec3{1.0, 1.0, 1.0});
  const double nan = std::numeric_limits<double>::quiet_NaN();
  const Mesh meshB = make_triangle(Vec3{0.5, 0.5, 0.5}, Vec3{nan, 0.0, 0.0}, Vec3{0.0, 2.0, 0.0});

  const auto result = detail::compute_intersection_graph(meshA, meshB);
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error, GraphStatus::NonFiniteCoordinates);
  EXPECT_TRUE(result.graph.keys.empty());
}

} // namespace BooleanIntersectionTesting
