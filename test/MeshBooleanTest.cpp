#include <Geometry/Cylinder.hpp>
#include <Geometry/Mesh/MakeTriangleMesh.hpp>
#include <Geometry/Mesh/MeshBoolean.hpp>
#include <Geometry/Mesh/MeshDelete.hpp>
#include <Geometry/Mesh/MeshFromTriangles.hpp>
#include <Geometry/Mesh/MeshGlobalTopology.hpp>
#include <Geometry/Mesh/MeshNormals.hpp>
#include <Geometry/Mesh/MeshOrientation.hpp>
#include <Geometry/Mesh/MeshVerify.hpp>
#include <Geometry/Mesh/TriangleHalfedgeMesh.hpp>
#include <Geometry/Mesh/detail/BooleanClassification.hpp>
#include <Geometry/Segment.hpp>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numbers>
#include <optional>
#include <span>
#include <utility>
#include <vector>

using namespace Geometry;

// A named namespace rather than an anonymous one, so that its aliases cannot hide library names
// (MSVC C4459).
namespace MeshBooleanTesting
{

using Mesh = TriangleHalfedgeMesh3d;
using EdgeHandle = Mesh::EdgeHandle;
using FaceHandle = Mesh::FaceHandle;
using VertexHandle = Mesh::VertexHandle;
using Vec3 = Mesh::vec_t;
using Triangle = std::array<std::uint32_t, 3>;
using Result = MeshBooleanResult<double, std::uint32_t>;
using Point = detail::ImplicitPoint<double>;
using detail::PatchClass;

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

// The checks every Boolean result must pass, plus its expected shape: component count, Euler
// characteristic (2 per sphere-like component, minus 2 per handle) and volume.
void expect_solid(const Result& result, const std::size_t components, const std::ptrdiff_t eulerCharacteristic, const double volume)
{
  ASSERT_TRUE(result.has_value()) << "status " << static_cast<int>(result.error);
  const Mesh& mesh = result.mesh;
  EXPECT_TRUE(mesh.has_valid_connectivity());
  EXPECT_TRUE(verify_manifold(mesh));
  EXPECT_TRUE(verify_closed(mesh));
  EXPECT_TRUE(std::ranges::none_of(mesh.vertices(), [&mesh](const VertexHandle vertex) { return is_isolated(mesh, vertex); }));
  EXPECT_EQ(connected_component_count(mesh), components);
  EXPECT_EQ(euler_characteristic(mesh), eulerCharacteristic);
  EXPECT_NEAR(signed_volume(mesh), volume, 1e-9);
  if (components > 0)
  {
    EXPECT_EQ(is_outward_oriented(mesh), std::optional<bool>{true});
  }
}

void expect_empty(const Result& result)
{
  ASSERT_TRUE(result.has_value()) << "status " << static_cast<int>(result.error);
  EXPECT_EQ(result.mesh.face_count(), 0U);
  EXPECT_EQ(result.mesh.vertex_count(), 0U);
}

// --- Classification building blocks -----------------------------------------------------------

Point explicit_point(const double x, const double y, const double z)
{
  return Point::create_explicit(Vec3{x, y, z});
}

TEST(MeshBooleanClassificationTest, convex_wedge_contains_only_what_is_inside_both_faces)
{
  // The edge of the unit box along z at x = 1, y = 1: forward face in the plane x = 1, backward face
  // in the plane y = 1, both facing out.
  const Point start = explicit_point(1.0, 1.0, 0.0);
  const Point end = explicit_point(1.0, 1.0, 1.0);
  const Point forwardApex = explicit_point(1.0, 0.0, 0.0);
  const Point backwardApex = explicit_point(0.0, 1.0, 1.0);
  const auto classify = [&](const Point& apex) { return detail::classify_against_wedge(start, end, apex, forwardApex, backwardApex); };

  EXPECT_EQ(classify(explicit_point(0.5, 0.5, 0.5)), PatchClass::Inside);
  EXPECT_EQ(classify(explicit_point(2.0, 0.5, 0.5)), PatchClass::Outside);
  EXPECT_EQ(classify(explicit_point(2.0, 2.0, 0.5)), PatchClass::Outside);
  // In the forward plane, beyond the edge: outside the box.
  EXPECT_EQ(classify(explicit_point(1.0, 2.0, 0.5)), PatchClass::Outside);
  // On the forward face, running start -> end like it: the same orientation.
  EXPECT_EQ(classify(explicit_point(1.0, 0.0, 0.5)), PatchClass::OnSame);
  // On the backward face, which runs end -> start.
  EXPECT_EQ(classify(explicit_point(0.0, 1.0, 0.5)), PatchClass::OnOpposite);
}

TEST(MeshBooleanClassificationTest, reflex_wedge_contains_what_is_inside_either_face)
{
  // The inner edge of an L-shaped solid {x < 1 or y < 1}: forward face in the plane y = 1 facing +y,
  // backward face in the plane x = 1 facing +x.
  const Point start = explicit_point(1.0, 1.0, 0.0);
  const Point end = explicit_point(1.0, 1.0, 1.0);
  const Point forwardApex = explicit_point(2.0, 1.0, 0.0);
  const Point backwardApex = explicit_point(1.0, 2.0, 1.0);
  const auto classify = [&](const Point& apex) { return detail::classify_against_wedge(start, end, apex, forwardApex, backwardApex); };

  EXPECT_EQ(classify(explicit_point(0.5, 0.5, 0.5)), PatchClass::Inside);
  EXPECT_EQ(classify(explicit_point(0.5, 2.0, 0.5)), PatchClass::Inside);
  EXPECT_EQ(classify(explicit_point(2.0, 0.5, 0.5)), PatchClass::Inside);
  EXPECT_EQ(classify(explicit_point(2.0, 2.0, 0.5)), PatchClass::Outside);
  // In the forward plane, beyond the edge: inside the L.
  EXPECT_EQ(classify(explicit_point(0.0, 1.0, 0.5)), PatchClass::Inside);
  EXPECT_EQ(classify(explicit_point(2.0, 1.0, 0.5)), PatchClass::OnSame);
  EXPECT_EQ(classify(explicit_point(1.0, 2.0, 0.5)), PatchClass::OnOpposite);
}

TEST(MeshBooleanClassificationTest, flat_wedge_is_split_by_the_projection_of_its_plane)
{
  // Two faces of the plane z = 0, facing up, on either side of the edge along x.
  const Point start = explicit_point(0.0, 0.0, 0.0);
  const Point end = explicit_point(1.0, 0.0, 0.0);
  const Point forwardApex = explicit_point(0.5, 1.0, 0.0);
  const Point backwardApex = explicit_point(0.5, -1.0, 0.0);
  const auto classify = [&](const Point& apex) { return detail::classify_against_wedge(start, end, apex, forwardApex, backwardApex); };

  EXPECT_EQ(classify(explicit_point(0.5, 0.5, -1.0)), PatchClass::Inside);
  EXPECT_EQ(classify(explicit_point(0.5, 0.5, 1.0)), PatchClass::Outside);
  EXPECT_EQ(classify(explicit_point(0.5, 3.0, 0.0)), PatchClass::OnSame);
  EXPECT_EQ(classify(explicit_point(0.5, -3.0, 0.0)), PatchClass::OnOpposite);

  // Both faces on one side of the edge, back to back: no wedge.
  const Point foldedApex = explicit_point(0.5, 2.0, 0.0);
  EXPECT_EQ(detail::classify_against_wedge(start, end, explicit_point(0.5, 0.5, 1.0), forwardApex, foldedApex), std::nullopt);
}

TEST(MeshBooleanClassificationTest, winding_number_separates_inside_from_outside)
{
  const Mesh box = make_box(Vec3{0.0, 0.0, 0.0}, Vec3{1.0, 1.0, 1.0});
  EXPECT_NEAR(detail::winding_number(box, Vec3{0.5, 0.25, 0.75}), 1.0, tolerance);
  EXPECT_NEAR(detail::winding_number(box, Vec3{0.999, 0.001, 0.5}), 1.0, tolerance);
  EXPECT_NEAR(detail::winding_number(box, Vec3{1.5, 0.5, 0.5}), 0.0, tolerance);
  EXPECT_NEAR(detail::winding_number(box, Vec3{-3.0, 7.0, 2.0}), 0.0, tolerance);
}

TEST(MeshBooleanClassificationTest, selection_keeps_each_coplanar_region_once)
{
  using detail::keeps_face;
  using detail::Operand;

  for (const BooleanOperation operation : {BooleanOperation::Union, BooleanOperation::Intersection, BooleanOperation::Difference})
  {
    EXPECT_FALSE(keeps_face(operation, Operand::B, PatchClass::OnSame));
    EXPECT_FALSE(keeps_face(operation, Operand::B, PatchClass::OnOpposite));
    EXPECT_NE(keeps_face(operation, Operand::A, PatchClass::OnSame), keeps_face(operation, Operand::A, PatchClass::OnOpposite));
  }
  EXPECT_TRUE(keeps_face(BooleanOperation::Union, Operand::A, PatchClass::OnSame));
  EXPECT_TRUE(keeps_face(BooleanOperation::Difference, Operand::A, PatchClass::OnOpposite));
  EXPECT_TRUE(keeps_face(BooleanOperation::Difference, Operand::B, PatchClass::Inside));
  EXPECT_FALSE(keeps_face(BooleanOperation::Difference, Operand::B, PatchClass::Outside));
}

// --- Booleans ---------------------------------------------------------------------------------

TEST(MeshBooleanTest, overlapping_boxes)
{
  // Dyadic offsets keep every intersection point exactly representable and avoid coplanar faces.
  const Mesh meshA = make_box(Vec3{0.0, 0.0, 0.0}, Vec3{2.0, 2.0, 2.0});
  const Mesh meshB = make_box(Vec3{1.0, 0.5, 0.25}, Vec3{3.0, 2.5, 2.25});
  const double common = 1.0 * 1.5 * 1.75;

  expect_solid(mesh_union(meshA, meshB), 1, 2, 16.0 - common);
  expect_solid(mesh_intersection(meshA, meshB), 1, 2, common);
  expect_solid(mesh_difference(meshA, meshB), 1, 2, 8.0 - common);
  expect_solid(mesh_difference(meshB, meshA), 1, 2, 8.0 - common);
}

TEST(MeshBooleanTest, box_inside_box)
{
  // No contact at all: the winding number classifies the whole inner box.
  const Mesh outer = make_box(Vec3{0.0, 0.0, 0.0}, Vec3{4.0, 4.0, 4.0});
  const Mesh inner = make_box(Vec3{1.0, 1.0, 1.0}, Vec3{2.0, 2.0, 2.0});

  expect_solid(mesh_union(outer, inner), 1, 2, 64.0);
  expect_solid(mesh_intersection(outer, inner), 1, 2, 1.0);
  // A cavity: the inner box becomes an inward shell.
  expect_solid(mesh_difference(outer, inner), 2, 4, 63.0);
  expect_empty(mesh_difference(inner, outer));
}

TEST(MeshBooleanTest, disjoint_boxes)
{
  const Mesh meshA = make_box(Vec3{0.0, 0.0, 0.0}, Vec3{1.0, 1.0, 1.0});
  const Mesh meshB = make_box(Vec3{2.0, 0.0, 0.0}, Vec3{3.0, 1.0, 1.0});

  expect_solid(mesh_union(meshA, meshB), 2, 4, 2.0);
  expect_empty(mesh_intersection(meshA, meshB));
  expect_solid(mesh_difference(meshA, meshB), 1, 2, 1.0);
}

TEST(MeshBooleanTest, boxes_sharing_a_face_merge_without_an_inner_wall)
{
  const Mesh meshA = make_box(Vec3{0.0, 0.0, 0.0}, Vec3{1.0, 1.0, 1.0});
  const Mesh meshB = make_box(Vec3{1.0, 0.0, 0.0}, Vec3{2.0, 1.0, 1.0});

  const Result united = mesh_union(meshA, meshB);
  expect_solid(united, 1, 2, 2.0);
  // The shared square is gone; only its rim remains, inside the merged sides.
  EXPECT_EQ(united.mesh.face_count(), 20U);
  expect_empty(mesh_intersection(meshA, meshB));
  expect_solid(mesh_difference(meshA, meshB), 1, 2, 1.0);
}

TEST(MeshBooleanTest, boxes_in_shifted_coplanar_contact)
{
  // B's min-x face lies in A's max-x face but is shifted, so the shared region is a rectangle that
  // neither face covers alone.
  const Mesh meshA = make_box(Vec3{0.0, 0.0, 0.0}, Vec3{1.0, 1.0, 1.0});
  const Mesh meshB = make_box(Vec3{1.0, 0.5, 0.25}, Vec3{2.0, 1.5, 1.25});

  expect_solid(mesh_union(meshA, meshB), 1, 2, 2.0);
  expect_empty(mesh_intersection(meshA, meshB));
  expect_solid(mesh_difference(meshA, meshB), 1, 2, 1.0);
  expect_solid(mesh_difference(meshB, meshA), 1, 2, 1.0);
}

TEST(MeshBooleanTest, flush_difference_cuts_a_through_hole)
{
  // B's top and bottom faces lie in A's: the cut is flush on both sides.
  const Mesh meshA = make_box(Vec3{0.0, 0.0, 0.0}, Vec3{2.0, 2.0, 2.0});
  const Mesh meshB = make_box(Vec3{0.5, 0.5, 0.0}, Vec3{1.5, 1.5, 2.0});

  // Genus 1: Euler characteristic 0.
  expect_solid(mesh_difference(meshA, meshB), 1, 0, 6.0);
  expect_solid(mesh_union(meshA, meshB), 1, 2, 8.0);
  expect_solid(mesh_intersection(meshA, meshB), 1, 2, 2.0);
  expect_empty(mesh_difference(meshB, meshA));
}

TEST(MeshBooleanTest, identical_boxes)
{
  const Mesh box = make_box(Vec3{0.0, 0.0, 0.0}, Vec3{1.0, 1.0, 1.0});

  const Result united = mesh_union(box, box);
  expect_solid(united, 1, 2, 1.0);
  EXPECT_EQ(united.mesh.face_count(), 12U);
  expect_solid(mesh_intersection(box, box), 1, 2, 1.0);
  expect_empty(mesh_difference(box, box));
}

TEST(MeshBooleanTest, tetrahedron_through_box_face)
{
  // Its side faces cross the box's top face along lines through three collinear intersection points
  // each (see MeshCorefineTest.collinear_points_split_the_edge_they_lie_on); deciding by rounded
  // positions left a sliver that the classification read as coplanar contact.
  const Mesh box = make_box(Vec3{0.0, 0.0, 0.0}, Vec3{2.0, 2.0, 2.0});
  const Mesh tetrahedron = make_mesh({Vec3{1.34, 1.24, 1.65}, Vec3{1.61, 1.35, 2.48}, Vec3{0.42, 1.39, 2.30}, Vec3{1.03, 0.30, 2.82}},
                                     {Triangle{0, 2, 1}, Triangle{0, 3, 2}, Triangle{0, 1, 3}, Triangle{1, 2, 3}});
  const double volume = signed_volume(tetrahedron);

  const Result common = mesh_intersection(box, tetrahedron);
  ASSERT_TRUE(common.has_value());
  const double commonVolume = signed_volume(common.mesh);
  EXPECT_GT(commonVolume, 0.0);
  EXPECT_LT(commonVolume, volume);
  expect_solid(common, 1, 2, commonVolume);
  expect_solid(mesh_union(box, tetrahedron), 1, 2, 8.0 + volume - commonVolume);
  // A pocket opening in the top face.
  expect_solid(mesh_difference(box, tetrahedron), 1, 2, 8.0 - commonVolume);
  expect_solid(mesh_difference(tetrahedron, box), 1, 2, volume - commonVolume);
}

TEST(MeshBooleanTest, cylinder_through_box)
{
  // The axis avoids the box diagonals x = y, so no side edge of the cylinder hits one exactly.
  constexpr std::size_t segments = 16;
  constexpr double radius = 0.45;
  const Mesh box = make_box(Vec3{0.0, 0.0, 0.0}, Vec3{2.0, 2.0, 2.0});
  auto cylinder = make_triangle_mesh(Cylinder<double>{Segment3<double>{Vec3{0.7, 1.2, -1.0}, Vec3{0.7, 1.2, 3.0}}, radius}, segments);
  ASSERT_TRUE(cylinder.has_value());
  const double crossSection = 0.5 * static_cast<double>(segments) * radius * radius * std::sin(2.0 * std::numbers::pi / static_cast<double>(segments));

  expect_solid(mesh_union(box, cylinder.mesh), 1, 2, 8.0 + 2.0 * crossSection);
  expect_solid(mesh_intersection(box, cylinder.mesh), 1, 2, 2.0 * crossSection);
  // A drilled hole: genus 1.
  expect_solid(mesh_difference(box, cylinder.mesh), 1, 0, 8.0 - 2.0 * crossSection);
  // The two ends sticking out of the box.
  expect_solid(mesh_difference(cylinder.mesh, box), 2, 4, 2.0 * crossSection);
}

TEST(MeshBooleanTest, boxes_touching_along_an_edge)
{
  const Mesh meshA = make_box(Vec3{0.0, 0.0, 0.0}, Vec3{1.0, 1.0, 1.0});
  const Mesh meshB = make_box(Vec3{1.0, 1.0, 0.0}, Vec3{2.0, 2.0, 1.0});

  const Result united = mesh_union(meshA, meshB);
  EXPECT_EQ(united.error, BooleanStatus::NonManifoldResult);
  EXPECT_EQ(united.mesh.face_count(), 0U);
  expect_empty(mesh_intersection(meshA, meshB));
  expect_solid(mesh_difference(meshA, meshB), 1, 2, 1.0);
}

TEST(MeshBooleanTest, solids_touching_at_a_vertex)
{
  // A tetrahedron stands on its apex inside the interior of a box's top face.
  const Mesh box = make_box(Vec3{0.0, 0.0, 0.0}, Vec3{2.0, 2.0, 2.0});
  const Mesh tetrahedron = make_mesh({Vec3{1.0, 0.5, 2.0}, Vec3{0.0, 0.0, 3.0}, Vec3{2.0, 0.0, 3.0}, Vec3{1.0, 2.0, 3.0}},
                                     {Triangle{0, 2, 1}, Triangle{0, 3, 2}, Triangle{0, 1, 3}, Triangle{1, 2, 3}});

  EXPECT_EQ(mesh_union(box, tetrahedron).error, BooleanStatus::NonManifoldResult);
  expect_empty(mesh_intersection(box, tetrahedron));
  expect_solid(mesh_difference(box, tetrahedron), 1, 2, 8.0);
  expect_solid(mesh_difference(tetrahedron, box), 1, 2, signed_volume(tetrahedron));

  // Corner to corner.
  const Mesh corner = make_box(Vec3{2.0, 2.0, 2.0}, Vec3{3.0, 3.0, 3.0});
  EXPECT_EQ(mesh_union(box, corner).error, BooleanStatus::NonManifoldResult);
}

TEST(MeshBooleanTest, empty_operand_is_the_empty_solid)
{
  const Mesh box = make_box(Vec3{0.0, 0.0, 0.0}, Vec3{1.0, 1.0, 1.0});
  const Mesh empty;

  expect_solid(mesh_union(box, empty), 1, 2, 1.0);
  expect_solid(mesh_union(empty, box), 1, 2, 1.0);
  expect_empty(mesh_intersection(box, empty));
  expect_solid(mesh_difference(box, empty), 1, 2, 1.0);
  expect_empty(mesh_difference(empty, box));
}

TEST(MeshBooleanTest, invalid_operands_are_reported)
{
  const Mesh box = make_box(Vec3{0.0, 0.0, 0.0}, Vec3{1.0, 1.0, 1.0});
  const auto statusWith = [&box](const Mesh& operand) {
    const Result asB = mesh_union(box, operand);
    const Result asA = mesh_union(operand, box);
    EXPECT_EQ(asA.error, asB.error);
    EXPECT_EQ(asB.mesh.face_count(), 0U);
    return asB.error;
  };

  Mesh nonFinite = box;
  nonFinite.set_position(VertexHandle{3}, Vec3{std::numeric_limits<double>::quiet_NaN(), 1.0, 0.0});
  EXPECT_EQ(statusWith(nonFinite), BooleanStatus::OperandNotFinite);

  // Deleting the middle face of a fan leaves its hub with two fans that meet only there.
  Mesh bowTie = make_mesh({Vec3{0.0, 0.0, 0.0}, Vec3{1.0, 0.0, 0.0}, Vec3{1.0, 1.0, 0.0}, Vec3{0.0, 1.0, 0.0}, Vec3{-1.0, 1.0, 0.0}},
                          {Triangle{0, 1, 2}, Triangle{0, 2, 3}, Triangle{0, 3, 4}});
  detail::delete_face_unchecked(bowTie, FaceHandle{1});
  EXPECT_EQ(statusWith(bowTie), BooleanStatus::OperandNotManifold);

  const Mesh open = make_mesh({Vec3{0.0, 0.0, 0.0}, Vec3{1.0, 0.0, 0.0}, Vec3{0.0, 1.0, 0.0}}, {Triangle{0, 1, 2}});
  EXPECT_EQ(statusWith(open), BooleanStatus::OperandNotClosed);

  // Vertex 0 moved onto the middle of the edge between vertices 4 and 6 flattens face (0, 4, 6).
  Mesh degenerate = box;
  degenerate.set_position(VertexHandle{0}, Vec3{0.0, 0.5, 1.0});
  EXPECT_EQ(statusWith(degenerate), BooleanStatus::OperandDegenerateFace);

  const Mesh inward = make_mesh({Vec3{0.0, 0.0, 0.0}, Vec3{1.0, 0.0, 0.0}, Vec3{0.0, 1.0, 0.0}, Vec3{0.0, 0.0, 1.0}},
                                {Triangle{0, 1, 2}, Triangle{0, 3, 1}, Triangle{0, 2, 3}, Triangle{1, 3, 2}});
  EXPECT_EQ(statusWith(inward), BooleanStatus::OperandNotOutward);

  // Two opposite copies of one triangle: closed, but enclosing no volume.
  const Mesh flat = make_mesh({Vec3{0.0, 0.0, 0.0}, Vec3{1.0, 0.0, 0.0}, Vec3{0.0, 1.0, 0.0}}, {Triangle{0, 1, 2}, Triangle{0, 2, 1}});
  EXPECT_EQ(statusWith(flat), BooleanStatus::OperandNotOutward);
}

// --- Creases ----------------------------------------------------------------------------------

Mesh make_creased_box(const Vec3& min, const Vec3& max)
{
  auto result = make_triangle_mesh(AABB<double, 3>{min, max});
  EXPECT_TRUE(result.has_value());
  return std::move(result.mesh);
}

std::vector<bool> crease_flags(const Mesh& mesh)
{
  std::vector<bool> flags;
  for (const EdgeHandle edge : mesh.edges())
  {
    flags.push_back(mesh.is_crease(edge));
  }
  return flags;
}

std::size_t crease_count(const Mesh& mesh)
{
  return static_cast<std::size_t>(std::ranges::count(crease_flags(mesh), true));
}

TEST(MeshBooleanTest, creases_follow_the_options)
{
  const Mesh meshA = make_creased_box(Vec3{0.0, 0.0, 0.0}, Vec3{2.0, 2.0, 2.0});
  const Mesh meshB = make_creased_box(Vec3{1.0, 0.5, 0.25}, Vec3{3.0, 2.5, 2.25});
  const double creaseAngle = default_crease_angle<double>;

  // Operand creases and the curve by angle: the box edges and the curve, exactly the sharp edges.
  const Result withCurve = mesh_union(meshA, meshB, MeshBooleanOptions<double>{.transferCreases = true, .intersectionCreaseAngle = creaseAngle});
  ASSERT_TRUE(withCurve.has_value());
  Mesh byAngle = withCurve.mesh;
  mark_creases_by_angle(byAngle, creaseAngle);
  EXPECT_EQ(crease_flags(withCurve.mesh), crease_flags(byAngle));

  // By default the curve, sharp as it is, stays smooth; every transferred crease is a sharp edge.
  const Result transferred = mesh_union(meshA, meshB);
  ASSERT_TRUE(transferred.has_value());
  EXPECT_LT(crease_count(transferred.mesh), crease_count(withCurve.mesh));
  EXPECT_GT(crease_count(transferred.mesh), 0U);
  const std::vector<bool> sharp = crease_flags(byAngle);
  const std::vector<bool> transferredFlags = crease_flags(transferred.mesh);
  for (std::size_t i = 0; i < sharp.size(); ++i)
  {
    EXPECT_TRUE(!transferredFlags[i] || sharp[i]) << "edge " << i;
  }

  const Result none = mesh_union(meshA, meshB, MeshBooleanOptions<double>{.transferCreases = false});
  ASSERT_TRUE(none.has_value());
  EXPECT_EQ(crease_count(none.mesh), 0U);
}

TEST(MeshBooleanTest, crease_on_a_merged_rim_is_dropped)
{
  // The shared square's rim is a box edge of both operands, but lies flat inside the merged sides.
  const Mesh meshA = make_creased_box(Vec3{0.0, 0.0, 0.0}, Vec3{1.0, 1.0, 1.0});
  const Mesh meshB = make_creased_box(Vec3{1.0, 0.0, 0.0}, Vec3{2.0, 1.0, 1.0});

  const Result united = mesh_union(meshA, meshB, MeshBooleanOptions<double>{.intersectionCreaseAngle = default_crease_angle<double>});
  ASSERT_TRUE(united.has_value());
  // The 12 edges of the merged box, the four along x split in two where the rim was.
  EXPECT_EQ(crease_count(united.mesh), 16U);
}

// --- Other instantiations ---------------------------------------------------------------------

TEST(MeshBooleanTest, float_coordinates)
{
  using FloatMesh = TriangleHalfedgeMesh<float, 3, std::uint32_t>;
  using FloatVec = FloatMesh::vec_t;
  auto boxA = make_triangle_mesh(AABB<float, 3>{FloatVec{0.0F, 0.0F, 0.0F}, FloatVec{2.0F, 2.0F, 2.0F}});
  auto boxB = make_triangle_mesh(AABB<float, 3>{FloatVec{1.0F, 0.5F, 0.25F}, FloatVec{3.0F, 2.5F, 2.25F}});
  ASSERT_TRUE(boxA.has_value() && boxB.has_value());

  const MeshBooleanResult<float, std::uint32_t> result = mesh_intersection(boxA.mesh, boxB.mesh);
  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(verify_closed(result.mesh));
  EXPECT_TRUE(verify_manifold(result.mesh));
  EXPECT_EQ(connected_component_count(result.mesh), 1U);
}

// --- Rounding gap -----------------------------------------------------------------------------

// Expected to fail until the ImplicitPoint predicates evaluate intersection points by their
// definition (see detail::ImplicitPoint). B's top edge runs across A's bottom-front edge 2^-60 above
// it, so the two faces of B at that edge cross A's edge at x = 1 -+ 2^-61, two distinct points closer
// together than the rounding error of their positions. The predicates on input points are exact here,
// but both points round to x = 1, and corefinement cannot tell them apart. Indirect predicates order
// the true points, so the Boolean succeeds.
TEST(MeshBooleanTest, DISABLED_intersection_points_closer_than_rounding)
{
  const Mesh box = make_box(Vec3{0.0, 0.0, 0.0}, Vec3{2.0, 2.0, 2.0});
  const double offset = std::ldexp(1.0, -60);
  const Mesh wedge = make_mesh({Vec3{1.0, -1.0, offset}, Vec3{1.0, 1.0, offset}, Vec3{0.5, 0.25, -1.0}, Vec3{1.5, -0.25, -1.0}},
                               {Triangle{0, 1, 2}, Triangle{0, 3, 1}, Triangle{0, 2, 3}, Triangle{1, 3, 2}});
  ASSERT_EQ(is_outward_oriented(wedge), std::optional<bool>{true});

  const Result united = mesh_union(box, wedge);
  ASSERT_TRUE(united.has_value()) << "status " << static_cast<int>(united.error);
  EXPECT_TRUE(verify_manifold(united.mesh));
  EXPECT_TRUE(verify_closed(united.mesh));
}

} // namespace MeshBooleanTesting
