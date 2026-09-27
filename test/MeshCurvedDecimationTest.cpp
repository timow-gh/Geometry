#include <Geometry/Cylinder.hpp>
#include <Geometry/Mesh/AddTriangle.hpp>
#include <Geometry/Mesh/MakeTriangleMesh.hpp>
#include <Geometry/Mesh/MeshEdgeCollapse.hpp>
#include <Geometry/Mesh/MeshEdgeCollapseChecks.hpp>
#include <Geometry/Mesh/MeshGlobalTopology.hpp>
#include <Geometry/Mesh/MeshOrientation.hpp>
#include <Geometry/Mesh/MeshQuality.hpp>
#include <Geometry/Mesh/MeshTopology.hpp>
#include <Geometry/Mesh/MeshVerify.hpp>
#include <Geometry/Mesh/MeshVertexDecimation.hpp>
#include <Geometry/Mesh/TriangleHalfedgeMesh.hpp>
#include <Geometry/Mesh/detail/FaceGeometry.hpp>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <numbers>
#include <optional>
#include <random>
#include <vector>

using namespace Geometry;

namespace
{

using Mesh = TriangleHalfedgeMesh3d;
using VertexHandle = Mesh::VertexHandle;
using EdgeHandle = Mesh::EdgeHandle;
using FaceHandle = Mesh::FaceHandle;
using Position = Mesh::vec_t;

constexpr std::size_t surfaceResolution = 11;
constexpr double surfaceExtent = 4.0;
constexpr std::size_t decimatedVertexCount = 45;

// The limits the example decimates with. The default corner limit only rejects nearly flat
// triangles; on this surface the operators then still leave slivers standing edge-on across it
// (160 degrees still let one through), so the example demands 150.
const MeshGeometryLimits<double> exampleLimits{.maxCornerAngle = std::numbers::pi * 150.0 / 180.0};

// The wavy height field of examples/example_mesh_operations.cpp, built in the same order, so these
// tests replay the example's decimation exactly. Planar grids cannot expose fold-overs: in the plane a
// face can only flip, which the per-face orientation test already catches exactly.
Mesh make_height_field()
{
  Mesh mesh;
  std::vector<VertexHandle> vertices;
  const double step = surfaceExtent / static_cast<double>(surfaceResolution - 1);
  for (std::size_t j = 0; j < surfaceResolution; ++j)
  {
    for (std::size_t i = 0; i < surfaceResolution; ++i)
    {
      const double u = static_cast<double>(i) * step;
      const double v = static_cast<double>(j) * step;
      vertices.push_back(mesh.add_vertex({u, v, 1.0 + 0.35 * std::sin(1.6 * u) * std::cos(1.3 * v)}));
    }
  }
  const auto at = [&](std::size_t i, std::size_t j) { return vertices[j * surfaceResolution + i]; };
  for (std::size_t j = 0; j + 1 < surfaceResolution; ++j)
  {
    for (std::size_t i = 0; i + 1 < surfaceResolution; ++i)
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

// Largest angle, in degrees, between the normals of two faces sharing an edge: how close the surface
// comes to folding onto itself. Reported on failure to judge the fold threshold.
double largest_normal_angle_degrees(const Mesh& mesh)
{
  double largest = 0.0;
  for (const EdgeHandle edge : mesh.edges())
  {
    const auto halfedge = mesh.get_edge(edge).halfedge;
    const FaceHandle face = mesh.get_halfedge(halfedge).face;
    const FaceHandle neighbour = mesh.get_halfedge(mesh.get_halfedge(halfedge).twin).face;
    if (!face.is_valid() || !neighbour.is_valid())
    {
      continue;
    }
    const auto normal = detail::mesh_face_normal(mesh, face);
    const auto neighbourNormal = detail::mesh_face_normal(mesh, neighbour);
    if (normal && neighbourNormal)
    {
      const double cosine = std::clamp(linal::dot(*normal, *neighbourNormal), -1.0, 1.0);
      largest = std::max(largest, std::acos(cosine) * 180.0 / std::numbers::pi);
    }
  }
  return largest;
}

// A height field stays one: every face must still face +z, otherwise the surface folded over.
::testing::AssertionResult is_valid_height_field(const Mesh& mesh)
{
  if (!mesh.has_valid_connectivity() || !verify_manifold(mesh))
  {
    return ::testing::AssertionFailure() << "broken connectivity or not manifold";
  }
  if (has_folded_edges(mesh))
  {
    return ::testing::AssertionFailure() << "folded edge; largest angle between adjacent normals "
                                         << largest_normal_angle_degrees(mesh) << " deg";
  }
  if (euler_characteristic(mesh) != 1)
  {
    return ::testing::AssertionFailure() << "Euler characteristic " << euler_characteristic(mesh) << " != 1";
  }
  for (const FaceHandle face : mesh.faces())
  {
    const auto areaVector = detail::mesh_face_area_vector(mesh, face);
    if (areaVector[2] > 0.0)
    {
      continue;
    }
    auto failure = ::testing::AssertionFailure();
    failure << "face " << face.get_value() << " no longer faces +z, area vector (" << areaVector[0] << ", " << areaVector[1]
            << ", " << areaVector[2] << "), corners";
    for (const VertexHandle corner : mesh.vertices_around_face(face))
    {
      const Position& position = mesh.get_position(corner);
      failure << " " << corner.get_value() << (is_boundary(mesh, corner) ? "b" : "") << "(" << position[0] << ", "
              << position[1] << ", " << position[2] << ")";
    }
    return failure << "; largest angle between adjacent normals " << largest_normal_angle_degrees(mesh) << " deg";
  }
  return ::testing::AssertionSuccess();
}

// Removes vertices in the example's fixed pseudo-random order until the target count is reached or
// no vertex can be removed, checking the height field after every removal.
template <typename TRemove>
void decimate_by_vertex_removal(Mesh& mesh, TRemove remove)
{
  std::mt19937 generator(7);
  bool progress = true;
  while (progress && mesh.vertex_count() > decimatedVertexCount)
  {
    std::vector<VertexHandle> candidates(mesh.vertices().begin(), mesh.vertices().end());
    std::shuffle(candidates.begin(), candidates.end(), generator);
    progress = false;
    for (const VertexHandle vertex : candidates)
    {
      if (mesh.vertex_count() <= decimatedVertexCount)
      {
        break;
      }
      if (!mesh.is_deleted(vertex) && remove(mesh, vertex))
      {
        progress = true;
        ASSERT_TRUE(is_valid_height_field(mesh)) << "after removing vertex " << vertex.get_value();
      }
    }
  }
}

// The example's merged position: the boundary endpoint when only one endpoint is on the boundary,
// otherwise the midpoint.
Position merged_position(const Mesh& mesh, EdgeHandle edge)
{
  const auto halfedge = mesh.get_edge(edge).halfedge;
  const VertexHandle source = mesh.source_vertex(halfedge);
  const VertexHandle target = mesh.target_vertex(halfedge);
  const bool sourceOnBoundary = is_boundary(mesh, source);
  const bool targetOnBoundary = is_boundary(mesh, target);
  if (sourceOnBoundary != targetOnBoundary)
  {
    return mesh.get_position(sourceOnBoundary ? source : target);
  }
  return Position{0.5 * (mesh.get_position(source) + mesh.get_position(target))};
}

bool is_edge_collapse_acceptable(const Mesh& mesh, EdgeHandle edge)
{
  return check_collapse(mesh, mesh.get_edge(edge).halfedge, merged_position(mesh, edge), exampleLimits) == CollapseStatus::Ok;
}

} // namespace

TEST(MeshCurvedDecimation, HalfedgeCollapseRemovalKeepsHeightField)
{
  Mesh mesh = make_height_field();
  ASSERT_TRUE(is_valid_height_field(mesh));
  decimate_by_vertex_removal(mesh, [](Mesh& target, VertexHandle vertex) {
    return decimate_vertex_by_collapse(target, vertex, exampleLimits).has_value();
  });
  EXPECT_EQ(mesh.vertex_count(), decimatedVertexCount);
}

TEST(MeshCurvedDecimation, RetriangulatingRemovalKeepsHeightField)
{
  Mesh mesh = make_height_field();
  decimate_by_vertex_removal(mesh, [](Mesh& target, VertexHandle vertex) {
    return decimate_vertex_by_retriangulation(target, vertex, exampleLimits) == VertexDecimationStatus::Ok;
  });
  EXPECT_EQ(mesh.vertex_count(), decimatedVertexCount);
}

TEST(MeshCurvedDecimation, ShortestEdgeCollapseKeepsHeightField)
{
  Mesh mesh = make_height_field();
  while (mesh.vertex_count() > decimatedVertexCount)
  {
    std::optional<EdgeHandle> shortest;
    double shortestLength = std::numeric_limits<double>::max();
    for (const EdgeHandle edge : mesh.edges())
    {
      const auto halfedge = mesh.get_edge(edge).halfedge;
      const double length =
          linal::length(Position{mesh.get_position(mesh.target_vertex(halfedge)) - mesh.get_position(mesh.source_vertex(halfedge))});
      if (length < shortestLength && is_edge_collapse_acceptable(mesh, edge))
      {
        shortest = edge;
        shortestLength = length;
      }
    }
    ASSERT_TRUE(shortest.has_value()) << "decimation stalled at " << mesh.vertex_count() << " vertices";
    ASSERT_TRUE(collapse_edge(mesh, *shortest, merged_position(mesh, *shortest), exampleLimits).has_value());
    ASSERT_TRUE(is_valid_height_field(mesh)) << "after collapsing edge " << shortest->get_value();
  }
}

// The example's cylinder panels: a closed surface with 90 degree creases between caps and side,
// decimated to half its vertices under the example's limits. The limits must not stall the
// decimation at the creases, and the result must still enclose its volume.
template <typename TRemove>
void expect_cylinder_decimates_cleanly(TRemove remove)
{
  const Cylinder<double> cylinder{Segment3d{{0.0, 0.0, 0.5}, {0.0, 0.0, 3.5}}, 1.5};
  auto creation = make_triangle_mesh(cylinder, 24);
  ASSERT_TRUE(creation.has_value());
  Mesh mesh = std::move(creation.mesh);
  const std::size_t targetVertexCount = mesh.vertex_count() / 2;

  std::mt19937 generator(7);
  bool progress = true;
  while (progress && mesh.vertex_count() > targetVertexCount)
  {
    std::vector<VertexHandle> candidates(mesh.vertices().begin(), mesh.vertices().end());
    std::shuffle(candidates.begin(), candidates.end(), generator);
    progress = false;
    for (const VertexHandle vertex : candidates)
    {
      if (mesh.vertex_count() <= targetVertexCount)
      {
        break;
      }
      progress = (!mesh.is_deleted(vertex) && remove(mesh, vertex)) || progress;
    }
  }

  EXPECT_EQ(mesh.vertex_count(), targetVertexCount);
  EXPECT_TRUE(mesh.has_valid_connectivity());
  EXPECT_TRUE(verify_manifold(mesh));
  EXPECT_FALSE(has_degenerate_faces(mesh));
  EXPECT_FALSE(has_folded_edges(mesh));
  EXPECT_EQ(mesh_orientation(mesh), MeshOrientation::Outward);
}

TEST(MeshCurvedDecimation, CylinderHalfedgeCollapseRemoval)
{
  expect_cylinder_decimates_cleanly([](Mesh& target, VertexHandle vertex) {
    return decimate_vertex_by_collapse(target, vertex, exampleLimits).has_value();
  });
}

TEST(MeshCurvedDecimation, CylinderRetriangulatingRemoval)
{
  expect_cylinder_decimates_cleanly([](Mesh& target, VertexHandle vertex) {
    return decimate_vertex_by_retriangulation(target, vertex, exampleLimits) == VertexDecimationStatus::Ok;
  });
}
