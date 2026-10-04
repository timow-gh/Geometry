#ifndef GEOMETRY_MESH_MESHFROMTRIANGLES_HPP
#define GEOMETRY_MESH_MESHFROMTRIANGLES_HPP

#include "Geometry/Mesh/MeshVerify.hpp"
#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Mesh/detail/MeshEditing.hpp"
#include "Geometry/Mesh/detail/MeshResult.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <algorithm>
#include <array>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <linal/vec.hpp>
#include <span>
#include <utility>
#include <vector>

namespace Geometry
{

/** \brief Reason \c make_mesh_from_triangles could not build a mesh; Ok on success. */
enum class MeshFromTrianglesStatus
{
  Ok,
  // More vertices or halfedges than the mesh's handle type can number.
  IndexCapacityExceeded,
  // A triangle names a vertex index past the end of the positions.
  VertexIndexOutOfRange,
  // A triangle repeats a vertex index, so it has no three distinct sides.
  DegenerateTriangle,
  // More than two triangles share an edge.
  NonManifoldEdge,
  // Two triangles run along their shared edge in the same direction: their windings disagree, or
  // one duplicates the other. Twin halfedges must run in opposite directions.
  InconsistentOrientation,
  // The triangles around a vertex form several fans that meet only at that vertex.
  NonManifoldVertex
};

/**
 * \brief Mesh built by \c make_mesh_from_triangles, or the reason it could not be built.
 *
 * A reported failure always carries an empty mesh, never a partial one.
 */
template <typename T, std::uint8_t D, typename TIndex>
struct MeshFromTrianglesResult
{
  TriangleHalfedgeMesh<T, D, TIndex> mesh;
  MeshFromTrianglesStatus error = MeshFromTrianglesStatus::Ok;

  GEO_NODISCARD bool has_value() const noexcept { return detail::mesh_result_ok(error); }
  GEO_NODISCARD explicit operator bool() const noexcept { return has_value(); }
};

namespace detail
{

/**
 * \internal
 * \brief One side of an input triangle, keyed by its undirected edge so that sorting brings all
 * sides of one edge next to each other.
 *
 * Sorting replaces a hash map from vertex pairs to halfedges: one allocation, no hashing, and a
 * result that does not depend on the triangle order.
 */
template <typename TIndex>
struct TriangleSide
{
  // Endpoint vertex indices, the smaller one first.
  TIndex low{};
  TIndex high{};
  // 3 * triangle + corner: the side runs from that corner to the next one of the triangle. It is
  // also the handle value of the halfedge built for the side, and orders the sides of one edge.
  TIndex halfedge{};

  GEO_NODISCARD constexpr auto operator<=>(const TriangleSide&) const noexcept = default;
};

/** \internal \brief Vertex index at which the side with halfedge value \p halfedge starts. O(1). */
template <typename TIndex>
GEO_NODISCARD constexpr TIndex side_source(const std::span<const std::array<TIndex, 3>> triangles, const TIndex halfedge) noexcept
{
  return triangles[static_cast<std::size_t>(halfedge) / 3][static_cast<std::size_t>(halfedge) % 3];
}

/** \internal \brief Vertex index at which the side with halfedge value \p halfedge ends. O(1). */
template <typename TIndex>
GEO_NODISCARD constexpr TIndex side_target(const std::span<const std::array<TIndex, 3>> triangles, const TIndex halfedge) noexcept
{
  return triangles[static_cast<std::size_t>(halfedge) / 3][(static_cast<std::size_t>(halfedge) + 1) % 3];
}

/**
 * \internal
 * \brief Checks that every triangle names three distinct vertices below \p vertexCount.
 *
 * \return \c Ok, \c VertexIndexOutOfRange or \c DegenerateTriangle for the first offending
 * triangle. O(F).
 */
template <typename TIndex>
GEO_NODISCARD MeshFromTrianglesStatus validate_triangle_indices(const std::span<const std::array<TIndex, 3>> triangles,
                                                                const std::size_t vertexCount) noexcept
{
  for (const std::array<TIndex, 3>& triangle : triangles)
  {
    if (std::ranges::any_of(triangle, [vertexCount](TIndex vertex) { return static_cast<std::size_t>(vertex) >= vertexCount; }))
    {
      return MeshFromTrianglesStatus::VertexIndexOutOfRange;
    }
    if (triangle[0] == triangle[1] || triangle[1] == triangle[2] || triangle[2] == triangle[0])
    {
      return MeshFromTrianglesStatus::DegenerateTriangle;
    }
  }
  return MeshFromTrianglesStatus::Ok;
}

/**
 * \internal
 * \brief All triangle sides, sorted so that the sides of each undirected edge are adjacent and in
 * halfedge order.
 *
 * \pre \c 3 * triangles.size() fits \p TIndex. O(F log F).
 */
template <typename TIndex>
GEO_NODISCARD std::vector<TriangleSide<TIndex>> make_sorted_sides(const std::span<const std::array<TIndex, 3>> triangles)
{
  using Side = TriangleSide<TIndex>;

  std::vector<Side> sides;
  sides.reserve(3 * triangles.size());
  for (std::size_t i = 0; i < 3 * triangles.size(); ++i)
  {
    const auto halfedge = static_cast<TIndex>(i);
    const TIndex source = side_source(triangles, halfedge);
    const TIndex target = side_target(triangles, halfedge);
    sides.push_back(Side{std::min(source, target), std::max(source, target), halfedge});
  }
  std::ranges::sort(sides);
  return sides;
}

/**
 * \internal
 * \brief One past the last side that shares the undirected edge of \p sides[\p first].
 *
 * \pre \p sides is sorted and \p first is in range. O(sides of that edge).
 */
template <typename TIndex>
GEO_NODISCARD std::size_t edge_group_end(const std::span<const TriangleSide<TIndex>> sides, const std::size_t first) noexcept
{
  GEO_ASSERT(first < sides.size());
  std::size_t last = first + 1;
  while (last < sides.size() && sides[last].low == sides[first].low && sides[last].high == sides[first].high)
  {
    ++last;
  }
  return last;
}

/** \internal \brief Edge counts of a valid side list, or the reason the sides cannot be paired. */
struct EdgeTally
{
  MeshFromTrianglesStatus error = MeshFromTrianglesStatus::Ok;
  std::size_t edgeCount{0};
  // Edges with a single side; each needs a boundary halfedge.
  std::size_t boundaryCount{0};
};

/**
 * \internal
 * \brief Counts edges and boundary edges, and rejects edges a halfedge mesh cannot represent: more
 * than two sides, or two sides in the same direction.
 *
 * Runs before the mesh is touched, so a failure costs no mesh allocation and the build can reserve
 * exact storage. O(F).
 */
template <typename TIndex>
GEO_NODISCARD EdgeTally tally_edges(const std::span<const TriangleSide<TIndex>> sides,
                                    const std::span<const std::array<TIndex, 3>> triangles) noexcept
{
  EdgeTally tally;
  for (std::size_t first = 0, last = 0; first < sides.size(); first = last)
  {
    last = edge_group_end(sides, first);
    if (last - first > 2)
    {
      return EdgeTally{MeshFromTrianglesStatus::NonManifoldEdge};
    }
    if (last - first == 2 && side_source(triangles, sides[first].halfedge) == side_source(triangles, sides[first + 1].halfedge))
    {
      return EdgeTally{MeshFromTrianglesStatus::InconsistentOrientation};
    }
    ++tally.edgeCount;
    tally.boundaryCount += last - first == 1 ? 1U : 0U;
  }
  return tally;
}

/**
 * \internal
 * \brief Appends one face per triangle with its three halfedges linked into a cycle, but without
 * twins or edges yet.
 *
 * Triangle f becomes face f with halfedges 3f, 3f + 1, 3f + 2, which is what the halfedge values
 * of \c TriangleSide refer to. \pre \p mesh has no halfedges or faces yet. O(F).
 */
template <typename T, std::uint8_t D, typename TIndex>
void add_triangle_faces(TriangleHalfedgeMesh<T, D, TIndex>& mesh, const std::span<const std::array<TIndex, 3>> triangles)
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using VertexHandle = typename Mesh::VertexHandle;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using FaceHandle = typename Mesh::FaceHandle;

  const auto connectivity = mesh.connectivity();
  GEO_ASSERT(connectivity.halfedge_storage_size() == 0 && connectivity.face_storage_size() == 0);
  for (const std::array<TIndex, 3>& triangle : triangles)
  {
    const FaceHandle face = connectivity.new_face();
    std::array<HalfedgeHandle, 3> halfedges{};
    for (std::size_t corner = 0; corner < 3; ++corner)
    {
      halfedges[corner] = connectivity.new_halfedge();
      connectivity.halfedge(halfedges[corner]).targetVertex = VertexHandle{triangle[(corner + 1) % 3]};
    }
    detail::link_triangle(mesh, face, halfedges[0], halfedges[1], halfedges[2]);
  }
}

/**
 * \internal
 * \brief Creates one edge per group of sides: two sides become twins, a lone side gets a new
 * boundary halfedge as its twin.
 *
 * Edges are numbered in side order and store their lowest face-side halfedge, so a boundary edge
 * stores its face-side halfedge as \c add_triangle does. Boundary halfedges are appended after the
 * face halfedges, still unlinked. \pre \p sides passed \c tally_edges. O(F).
 */
template <typename T, std::uint8_t D, typename TIndex>
void pair_sides(TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                const std::span<const TriangleSide<TIndex>> sides,
                const std::span<const std::array<TIndex, 3>> triangles)
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using VertexHandle = typename Mesh::VertexHandle;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using EdgeHandle = typename Mesh::EdgeHandle;

  const auto connectivity = mesh.connectivity();
  for (std::size_t first = 0, last = 0; first < sides.size(); first = last)
  {
    last = edge_group_end(sides, first);
    GEO_ASSERT(last - first <= 2);
    const HalfedgeHandle inner{sides[first].halfedge};
    HalfedgeHandle twin{};
    if (last - first == 2)
    {
      twin = HalfedgeHandle{sides[first + 1].halfedge};
    }
    else
    {
      twin = connectivity.new_halfedge();
      connectivity.halfedge(twin).targetVertex = VertexHandle{side_source(triangles, sides[first].halfedge)};
    }
    const EdgeHandle edge = connectivity.new_edge(inner);
    connectivity.halfedge(inner).twin = twin;
    connectivity.halfedge(twin).twin = inner;
    connectivity.halfedge(inner).edge = edge;
    connectivity.halfedge(twin).edge = edge;
  }
}

/**
 * \internal
 * \brief The boundary halfedge that leaves the target of \p boundary on the same fan, i.e. its
 * successor in the boundary loop.
 *
 * Rotates around the target from face to face until the fan ends. At a vertex where several fans
 * meet, each fan's boundary is closed on its own, which leaves a well-formed structure for
 * \c verify_vertex_manifold to reject. The rotation ends: it cannot return to its start, whose
 * predecessor would have to be the face-less \p boundary. \pre Face halfedges are linked and
 * paired. O(valence).
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle
next_boundary_halfedge(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                       const typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle boundary) noexcept
{
  using HalfedgeHandle = typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle;

  GEO_ASSERT(mesh.is_boundary(boundary));
  HalfedgeHandle outgoing = mesh.get_halfedge(boundary).twin;
  while (true)
  {
    const HalfedgeHandle candidate = mesh.get_halfedge(mesh.get_halfedge(outgoing).prev).twin;
    if (mesh.is_boundary(candidate))
    {
      return candidate;
    }
    outgoing = candidate;
  }
}

/**
 * \internal
 * \brief Links every boundary halfedge, from \p firstBoundary to the end of storage, to its
 * successor in its boundary loop. O(boundary halfedges * valence).
 */
template <typename T, std::uint8_t D, typename TIndex>
void link_boundary_loops(TriangleHalfedgeMesh<T, D, TIndex>& mesh, const std::size_t firstBoundary) noexcept
{
  using HalfedgeHandle = typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle;

  const auto connectivity = mesh.connectivity();
  for (std::size_t i = firstBoundary; i < mesh.halfedge_storage_size(); ++i)
  {
    const HalfedgeHandle boundary{static_cast<TIndex>(i)};
    connectivity.link(boundary, detail::next_boundary_halfedge(mesh, boundary));
  }
}

/**
 * \internal
 * \brief Gives every used vertex an outgoing halfedge, a boundary one where the vertex has one, as
 * the boundary representative rule requires. \pre All halfedges are linked. O(H).
 */
template <typename T, std::uint8_t D, typename TIndex>
void assign_vertex_halfedges(TriangleHalfedgeMesh<T, D, TIndex>& mesh) noexcept
{
  using HalfedgeHandle = typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle;

  const auto connectivity = mesh.connectivity();
  for (const HalfedgeHandle halfedge : mesh.halfedges())
  {
    HalfedgeHandle& stored = connectivity.vertex(mesh.source_vertex(halfedge)).halfedge;
    if (!stored.is_valid() || mesh.is_boundary(halfedge))
    {
      stored = halfedge;
    }
  }
}

} // namespace detail

/**
 * \brief Builds a mesh from an indexed triangle list, accepting the triangles in any order.
 *
 * Use it where \c add_triangle cannot be: \c add_triangle only accepts a triangle that attaches
 * along an existing edge, while triangle lists (e.g. the selected patches of a mesh Boolean, or a
 * loaded file) come in arbitrary order. It rebuilds what \c make_vertex_buffer and
 * \c make_triangle_index_buffer flatten, up to a cyclic rotation of each triangle.
 *
 * Handles follow the input, so callers can map results back without a lookup table: vertex i is
 * \p positions[i], and face f is \p triangles[f], with stored halfedge 3f running from
 * \p triangles[f][0] to \p triangles[f][1]. Unreferenced positions become isolated vertices; remove
 * them first if they are unwanted. No edge is a crease.
 *
 * Topology only: positions are not inspected, so zero-area triangles and self-intersections are
 * accepted. Each edge must have one side (boundary) or two sides in opposite directions, and the
 * triangles around each vertex must form a single fan. Two opposite copies of one triangle form a
 * valid closed surface and are accepted.
 *
 * Allocation failures propagate as exceptions. O(V + F log F), plus an O(H * valence)
 * connectivity check in debug builds.
 *
 * \param positions Vertex positions; a triangle refers to them by index.
 * \param triangles Vertex indices per triangle, in winding order.
 * \return The mesh, or an empty mesh and the reason for the first defect found.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD MeshFromTrianglesResult<T, D, TIndex>
make_mesh_from_triangles(const std::span<const linal::vec<T, D>> positions, const std::span<const std::array<TIndex, 3>> triangles)
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using Result = MeshFromTrianglesResult<T, D, TIndex>;
  using Status = MeshFromTrianglesStatus;
  using VertexHandle = typename Mesh::VertexHandle;
  using Side = detail::TriangleSide<TIndex>;

  const auto failure = [](Status error) { return Result{Mesh{}, error}; };

  // The largest index value is the invalid handle, so every handle value must stay below it.
  constexpr auto handleLimit = static_cast<std::size_t>(std::numeric_limits<TIndex>::max());
  if (positions.size() >= handleLimit || triangles.size() >= handleLimit / 3)
  {
    return failure(Status::IndexCapacityExceeded);
  }
  if (const Status error = detail::validate_triangle_indices(triangles, positions.size()); error != Status::Ok)
  {
    return failure(error);
  }

  const std::vector<Side> sides = detail::make_sorted_sides(triangles);
  const detail::EdgeTally tally = detail::tally_edges(std::span<const Side>{sides}, triangles);
  if (tally.error != Status::Ok)
  {
    return failure(tally.error);
  }
  const std::size_t faceHalfedgeCount = 3 * triangles.size();
  if (tally.boundaryCount >= handleLimit - faceHalfedgeCount)
  {
    return failure(Status::IndexCapacityExceeded);
  }

  Mesh mesh;
  mesh.reserve({.vertices = positions.size(), .edges = tally.edgeCount, .faces = triangles.size()});
  for (std::size_t i = 0; i < positions.size(); ++i)
  {
    [[maybe_unused]] const VertexHandle vertex = mesh.add_vertex(positions[i]);
    GEO_ASSERT(static_cast<std::size_t>(vertex.get_value()) == i);
  }
  detail::add_triangle_faces(mesh, triangles);
  detail::pair_sides(mesh, std::span<const Side>{sides}, triangles);
  detail::link_boundary_loops(mesh, faceHalfedgeCount);
  detail::assign_vertex_halfedges(mesh);
  GEO_ASSERT(mesh.halfedge_storage_size() == faceHalfedgeCount + tally.boundaryCount);

  if (!verify_vertex_manifold(mesh))
  {
    return failure(Status::NonManifoldVertex);
  }
  GEO_ASSERT(mesh.has_valid_connectivity());
  return Result{std::move(mesh), Status::Ok};
}

} // namespace Geometry

#endif // GEOMETRY_MESH_MESHFROMTRIANGLES_HPP
