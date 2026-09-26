#ifndef GEOMETRY_MESH_MESHVERTEXREMOVAL_HPP
#define GEOMETRY_MESH_MESHVERTEXREMOVAL_HPP

#include "Geometry/Mesh/MeshCollapse.hpp"
#include "Geometry/Mesh/MeshFaceGeometry.hpp"
#include "Geometry/Mesh/MeshTopology.hpp"
#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <linal/vec.hpp>
#include <optional>
#include <vector>

namespace Geometry
{

/**
 * \brief Removes \p vertex by collapsing it into the nearest neighbour that it can legally be merged
 * into without flipping or flattening a face.
 *
 * Vertex removal whose hole is fan-triangulated from the chosen neighbour, i.e. a halfedge collapse
 * (see \c collapse_halfedge) with the target picked here. The shortest edge is the choice: it is
 * cheap, deterministic, and moves the surface least; callers needing another criterion (e.g. a
 * quadric error) should rank the outgoing halfedges themselves and call \c collapse_halfedge.
 * Candidates are rejected by \c is_collapse_ok and \c collapse_inverts_faces (the survivor keeps its
 * position); ties go to the first candidate in fan order. An isolated vertex is simply deleted. The
 * removed elements are tombstoned. O(valence(v)^2 * valence(neighbour)).
 *
 * \return The neighbour that absorbed \p vertex (invalid for a deleted isolated vertex), or the mesh
 * untouched and a status: \c InvalidHandle for a missing or deleted vertex; \c InvertsFaces if some
 * collapse was topologically legal but every legal one folds the surface; otherwise the topological
 * reason the last candidate in fan order was rejected.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD CollapseResult<typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle>
remove_vertex(TriangleHalfedgeMesh<T, D, TIndex>& mesh, typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle vertex)
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using VertexHandle = typename Mesh::VertexHandle;
  using vec_t = typename Mesh::vec_t;

  if (!mesh.is_live(vertex))
  {
    return {VertexHandle{}, CollapseStatus::InvalidHandle};
  }
  if (is_isolated(mesh, vertex))
  {
    mesh.connectivity().mark_deleted(vertex);
    return {VertexHandle{}, CollapseStatus::Ok};
  }

  HalfedgeHandle shortest{};
  T shortestLengthSquared{};
  CollapseStatus lastTopologicalRejection = CollapseStatus::Ok;
  bool foldsOver = false;
  const Mesh& constMesh = mesh;
  for (auto outgoing = constMesh.outgoing_halfedges(vertex).circulator(); outgoing.is_valid(); ++outgoing)
  {
    const HalfedgeHandle candidate = outgoing.get_halfedgehandle();
    const CollapseStatus status = is_collapse_ok(mesh, candidate);
    if (status != CollapseStatus::Ok)
    {
      lastTopologicalRejection = status;
      continue;
    }
    const vec_t& targetPosition = mesh.get_position(mesh.target_vertex(candidate));
    if (collapse_inverts_faces(mesh, candidate, targetPosition))
    {
      foldsOver = true;
      continue;
    }
    const T lengthSquared = linal::length_squared(vec_t{targetPosition - mesh.get_position(vertex)});
    if (!shortest.is_valid() || lengthSquared < shortestLengthSquared)
    {
      shortest = candidate;
      shortestLengthSquared = lengthSquared;
    }
  }

  if (!shortest.is_valid())
  {
    return {VertexHandle{}, foldsOver ? CollapseStatus::InvertsFaces : lastTopologicalRejection};
  }
  const VertexHandle survivor = mesh.target_vertex(shortest);
  detail::collapse_halfedge_unchecked(mesh, shortest);
  return {survivor, CollapseStatus::Ok};
}

/**
 * \brief Outcome of \c remove_vertex_retriangulate; every value but \c Ok leaves the mesh untouched.
 */
enum class VertexRemovalStatus
{
  Ok,
  // The vertex is not in the mesh or already deleted.
  InvalidHandle,
  // The vertex's only face is a component on its own; removing it would leave a dangling edge.
  IsolatedTriangle,
  // The vertex is a corner of a closed tetrahedron; the hole's only triangle would duplicate the
  // opposite face.
  Tetrahedron,
  // Every triangulation of the hole needs an edge that already exists elsewhere in the mesh.
  DuplicateEdge,
  // Triangulations exist, but each contains a triangle that is degenerate or faces away from the
  // removed star.
  InvertsFaces,
};

namespace detail
{

/**
 * \internal
 * \brief The star of a vertex -- its faces and spokes -- and the hole polygon removing it leaves.
 *
 * \c ring lists the polygon corners counter-clockwise (in face winding order). \c ringHalfedges[i]
 * is the face-side halfedge ring[i] -> ring[i+1] opposite the vertex; for an interior vertex the
 * last one closes the polygon, for a boundary vertex the closing side ring.back() -> ring.front()
 * does not exist yet and the vertex's boundary halfedges \c boundaryIn (ring.front() -> vertex) and
 * \c boundaryOut (vertex -> ring.back()) are recorded instead.
 */
template <typename Mesh>
struct VertexStar
{
  std::vector<typename Mesh::VertexHandle> ring;
  std::vector<typename Mesh::HalfedgeHandle> ringHalfedges;
  std::vector<typename Mesh::FaceHandle> faces;
  std::vector<typename Mesh::EdgeHandle> spokes;
  bool boundary{false};
  typename Mesh::HalfedgeHandle boundaryIn{};
  typename Mesh::HalfedgeHandle boundaryOut{};
};

/**
 * \internal
 * \brief Collects the star of a non-isolated \p vertex. Heap-allocates: valence is unbounded.
 * O(valence).
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD VertexStar<TriangleHalfedgeMesh<T, D, TIndex>>
collect_vertex_star(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                    typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle vertex)
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;

  VertexStar<Mesh> star;
  const HalfedgeHandle start = mesh.get_vertex(vertex).halfedge;
  GEO_ASSERT(start.is_valid());
  star.boundary = mesh.is_boundary(start);
  if (star.boundary)
  {
    star.boundaryOut = start;
    star.boundaryIn = mesh.get_halfedge(start).prev;
  }

  HalfedgeHandle outgoing = start;
  do
  {
    star.spokes.push_back(mesh.get_halfedge(outgoing).edge);
    if (!mesh.is_boundary(outgoing))
    {
      star.faces.push_back(mesh.get_halfedge(outgoing).face);
      star.ringHalfedges.push_back(mesh.get_halfedge(outgoing).next);
    }
    outgoing = mesh.next_in_outgoing_fan(outgoing);
  } while (outgoing != start);

  // The twin.next orbit turns clockwise; the polygon is wanted in face winding order.
  std::reverse(star.faces.begin(), star.faces.end());
  std::reverse(star.ringHalfedges.begin(), star.ringHalfedges.end());
  for (const HalfedgeHandle ringHalfedge : star.ringHalfedges)
  {
    star.ring.push_back(mesh.source_vertex(ringHalfedge));
  }
  if (star.boundary)
  {
    star.ring.push_back(mesh.target_vertex(star.ringHalfedges.back()));
    GEO_ASSERT(mesh.source_vertex(star.boundaryIn) == star.ring.front());
    GEO_ASSERT(mesh.target_vertex(star.boundaryOut) == star.ring.back());
  }
  return star;
}

/**
 * \internal
 * \brief Minimum-cost triangulation of the polygon with corners 0 .. \p count - 1, in order.
 *
 * Dynamic programming over sub-polygons (Klincsek 80; Liepa 03 uses it for hole filling):
 * \p cost(i, m, j) prices triangle (i, m, j) with i < m < j, infinity meaning forbidden, and
 * \p isDiagonalAllowed(i, j) vetoes a diagonal. The side (0, count - 1) always counts as present.
 * Exact search, so a triangulation is found whenever one of finite cost exists. O(count^3) time,
 * O(count^2) memory.
 *
 * \return The count - 2 triangles as index triples (i, m, j), or \c std::nullopt if every
 * triangulation is forbidden.
 */
template <typename TCost, typename TCostFunction, typename TDiagonalAllowed>
GEO_NODISCARD std::optional<std::vector<std::array<std::size_t, 3>>>
triangulate_polygon(std::size_t count, const TCostFunction& cost, const TDiagonalAllowed& isDiagonalAllowed)
{
  GEO_ASSERT(count >= 3);
  constexpr TCost infinity = std::numeric_limits<TCost>::infinity();
  const auto slot = [count](std::size_t i, std::size_t j) { return i * count + j; };

  std::vector<TCost> best(count * count, infinity);
  std::vector<std::size_t> split(count * count, 0);
  for (std::size_t i = 0; i + 1 < count; ++i)
  {
    best[slot(i, i + 1)] = TCost{0};
  }

  for (std::size_t length = 2; length < count; ++length)
  {
    for (std::size_t i = 0; i + length < count; ++i)
    {
      const std::size_t j = i + length;
      const bool isClosingSide = i == 0 && j == count - 1;
      if (!isClosingSide && !isDiagonalAllowed(i, j))
      {
        continue;
      }
      for (std::size_t k = i + 1; k < j; ++k)
      {
        const TCost total = best[slot(i, k)] + best[slot(k, j)] + cost(i, k, j);
        if (total < best[slot(i, j)])
        {
          best[slot(i, j)] = total;
          split[slot(i, j)] = k;
        }
      }
    }
  }

  if (!(best[slot(0, count - 1)] < infinity))
  {
    return std::nullopt;
  }

  std::vector<std::array<std::size_t, 3>> triangles;
  triangles.reserve(count - 2);
  std::vector<std::array<std::size_t, 2>> pending{{0, count - 1}};
  while (!pending.empty())
  {
    const auto [first, last] = pending.back();
    pending.pop_back();
    if (last - first < 2)
    {
      continue;
    }
    const std::size_t apex = split[slot(first, last)];
    triangles.push_back({first, apex, last});
    pending.push_back({first, apex});
    pending.push_back({apex, last});
  }
  return triangles;
}

/**
 * \internal
 * \brief Removes a boundary vertex whose star is a single face that is not isolated: the face and
 * both spokes go, and the face's opposite halfedge joins the boundary.
 */
template <typename T, std::uint8_t D, typename TIndex>
void remove_ear_vertex_unchecked(TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                 typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle vertex,
                                 const VertexStar<TriangleHalfedgeMesh<T, D, TIndex>>& star) noexcept
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using EdgeHandle = typename Mesh::EdgeHandle;
  using FaceHandle = typename Mesh::FaceHandle;
  using VertexHandle = typename Mesh::VertexHandle;

  GEO_ASSERT(star.boundary && star.faces.size() == 1);
  const auto connectivity = mesh.connectivity();
  const HalfedgeHandle opposite = star.ringHalfedges.front();

  connectivity.halfedge(opposite).face = FaceHandle{};
  connectivity.link(connectivity.halfedge(star.boundaryIn).prev, opposite);
  connectivity.link(opposite, connectivity.halfedge(star.boundaryOut).next);

  connectivity.mark_deleted(star.faces.front());
  for (const EdgeHandle spoke : star.spokes)
  {
    connectivity.mark_deleted(spoke);
  }
  connectivity.mark_deleted(vertex);

  connectivity.vertex(star.ring.front()).halfedge = opposite;
  for (const VertexHandle corner : star.ring)
  {
    connectivity.restore_boundary_representative(corner);
  }
}

/**
 * \internal
 * \brief Replaces the star of \p vertex by \p triangles over its hole polygon, in place.
 *
 * The ring halfedges are reused as the polygon sides, the diagonals (plus, for a boundary vertex,
 * the closing side and its boundary twin) are created, then the star is tombstoned. All allocation
 * happens before the first link changes, so the mesh is never left half-rebuilt. Precondition: the
 * triangles triangulate the polygon and none of their diagonals exists in the mesh.
 */
template <typename T, std::uint8_t D, typename TIndex>
void retriangulate_star(TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                        typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle vertex,
                        const VertexStar<TriangleHalfedgeMesh<T, D, TIndex>>& star,
                        const std::vector<std::array<std::size_t, 3>>& triangles)
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using EdgeHandle = typename Mesh::EdgeHandle;
  using FaceHandle = typename Mesh::FaceHandle;

  const std::size_t count = star.ring.size();
  GEO_ASSERT(triangles.size() == count - 2);
  const auto connectivity = mesh.connectivity();
  const auto slot = [count](std::size_t from, std::size_t to) { return from * count + to; };

  // directed[slot(i, j)]: the halfedge ring[i] -> ring[j] of a polygon side or diagonal.
  std::vector<HalfedgeHandle> directed(count * count);
  const std::size_t newEdgeCount = count - 3 + (star.boundary ? 1U : 0U);
  connectivity.reserve_edges(newEdgeCount);
  connectivity.reserve_halfedges(2 * newEdgeCount);
  connectivity.reserve_faces(triangles.size());

  const auto new_edge_between = [&](std::size_t from, std::size_t to) {
    const HalfedgeHandle forward = connectivity.new_halfedge();
    const HalfedgeHandle backward = connectivity.new_halfedge();
    const EdgeHandle edge = connectivity.new_edge(forward);
    connectivity.halfedge(forward).targetVertex = star.ring[to];
    connectivity.halfedge(backward).targetVertex = star.ring[from];
    connectivity.halfedge(forward).twin = backward;
    connectivity.halfedge(backward).twin = forward;
    connectivity.halfedge(forward).edge = edge;
    connectivity.halfedge(backward).edge = edge;
    directed[slot(from, to)] = forward;
    directed[slot(to, from)] = backward;
  };

  for (std::size_t i = 0; i < star.ringHalfedges.size(); ++i)
  {
    directed[slot(i, (i + 1) % count)] = star.ringHalfedges[i];
  }
  if (star.boundary)
  {
    // The closing side replaces the vertex's two boundary halfedges in the boundary loop.
    new_edge_between(count - 1, 0);
    const HalfedgeHandle closingBoundary = directed[slot(0, count - 1)];
    connectivity.link(connectivity.halfedge(star.boundaryIn).prev, closingBoundary);
    connectivity.link(closingBoundary, connectivity.halfedge(star.boundaryOut).next);
  }
  for (const auto& [first, apex, last] : triangles)
  {
    for (const auto& [from, to] : {std::array{first, apex}, std::array{apex, last}})
    {
      if (!directed[slot(from, to)].is_valid())
      {
        new_edge_between(from, to);
      }
    }
  }

  for (const auto& [first, apex, last] : triangles)
  {
    const FaceHandle face = connectivity.new_face();
    const std::array<HalfedgeHandle, 3> sides{directed[slot(first, apex)], directed[slot(apex, last)],
                                              directed[slot(last, first)]};
    for (std::size_t i = 0; i < 3; ++i)
    {
      GEO_ASSERT(sides[i].is_valid());
      connectivity.link(sides[i], sides[(i + 1) % 3]);
      connectivity.halfedge(sides[i]).face = face;
    }
    connectivity.face(face).set_halfedgehandle(sides[0]);
  }

  for (const FaceHandle face : star.faces)
  {
    connectivity.mark_deleted(face);
  }
  for (const EdgeHandle spoke : star.spokes)
  {
    connectivity.mark_deleted(spoke);
  }
  connectivity.mark_deleted(vertex);

  for (std::size_t i = 0; i < count; ++i)
  {
    connectivity.vertex(star.ring[i]).halfedge = directed[slot(i, (i + 1) % count)];
    connectivity.restore_boundary_representative(star.ring[i]);
  }
}

} // namespace detail

/**
 * \brief Removes \p vertex and its star, and fills the hole with the best-shaped triangulation of
 * the surrounding polygon.
 *
 * The general form of vertex removal: where \c remove_vertex is restricted to fan triangulations
 * (a collapse into one neighbour), this searches all triangulations of the k-gon by dynamic
 * programming and keeps the one minimizing the summed triangle aspect cost (squared edge lengths
 * over area, smallest for equilateral triangles). Triangles that are degenerate, or face away from
 * the removed star's summed area vector, and diagonals that already exist in the mesh are excluded,
 * so the hole is refilled without fold-overs or duplicate edges. A boundary vertex's hole is closed
 * by a new boundary edge between its two boundary neighbours; a boundary vertex with a single face
 * just loses that face. Removes 1 vertex, 3 edges and 2 faces (1, 2, 1 on the boundary), preserving
 * the Euler characteristic, and tombstones the removed elements. An isolated vertex is simply
 * deleted. O(k^3 + k^2 * valence) for valence k.
 *
 * \return \c VertexRemovalStatus::Ok after removing, otherwise why the mesh was left untouched.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD VertexRemovalStatus remove_vertex_retriangulate(TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                                              typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle vertex)
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using FaceHandle = typename Mesh::FaceHandle;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;

  if (!mesh.is_live(vertex))
  {
    return VertexRemovalStatus::InvalidHandle;
  }
  if (is_isolated(mesh, vertex))
  {
    mesh.connectivity().mark_deleted(vertex);
    return VertexRemovalStatus::Ok;
  }

  const detail::VertexStar<Mesh> star = detail::collect_vertex_star(mesh, vertex);
  const std::size_t count = star.ring.size();

  if (star.boundary && star.faces.size() == 1)
  {
    if (mesh.is_boundary(mesh.get_halfedge(star.ringHalfedges.front()).twin))
    {
      return VertexRemovalStatus::IsolatedTriangle;
    }
    detail::remove_ear_vertex_unchecked(mesh, vertex, star);
    return VertexRemovalStatus::Ok;
  }

  if (!star.boundary && count == 3)
  {
    // The hole's single triangle already exists, wound the other way, iff all three outer sides
    // belong to one face.
    const FaceHandle outerFace = mesh.get_halfedge(mesh.get_halfedge(star.ringHalfedges[0]).twin).face;
    const bool sharedOuterFace =
        outerFace.is_valid()
        && std::all_of(star.ringHalfedges.begin(), star.ringHalfedges.end(), [&](HalfedgeHandle ringHalfedge) {
             return mesh.get_halfedge(mesh.get_halfedge(ringHalfedge).twin).face == outerFace;
           });
    if (sharedOuterFace)
    {
      return VertexRemovalStatus::Tetrahedron;
    }
  }

  if (star.boundary && mesh.find_halfedge(star.ring.back(), star.ring.front()).is_valid())
  {
    return VertexRemovalStatus::DuplicateEdge;
  }

  const auto isDiagonalAllowed = [&](std::size_t from, std::size_t to) {
    return !mesh.find_halfedge(star.ring[from], star.ring[to]).is_valid();
  };

  // The removed star's summed orientation is the reference the new triangles must agree with.
  const auto& center = mesh.get_position(vertex);
  auto reference = detail::triangle_orientation(center, mesh.get_position(mesh.source_vertex(star.ringHalfedges[0])),
                                                mesh.get_position(mesh.target_vertex(star.ringHalfedges[0])));
  for (std::size_t i = 1; i < star.ringHalfedges.size(); ++i)
  {
    reference = reference + detail::triangle_orientation(center, mesh.get_position(mesh.source_vertex(star.ringHalfedges[i])),
                                                         mesh.get_position(mesh.target_vertex(star.ringHalfedges[i])));
  }

  const auto aspectCost = [&](std::size_t first, std::size_t apex, std::size_t last) -> T {
    const auto& firstPosition = mesh.get_position(star.ring[first]);
    const auto& apexPosition = mesh.get_position(star.ring[apex]);
    const auto& lastPosition = mesh.get_position(star.ring[last]);
    const auto orientation = detail::triangle_orientation(firstPosition, apexPosition, lastPosition);
    if (!(detail::orientation_dot(orientation, reference) > T{0}))
    {
      return std::numeric_limits<T>::infinity();
    }
    using vec_t = typename Mesh::vec_t;
    const T squaredEdges = linal::length_squared(vec_t{apexPosition - firstPosition})
                           + linal::length_squared(vec_t{lastPosition - apexPosition})
                           + linal::length_squared(vec_t{firstPosition - lastPosition});
    return squaredEdges / std::sqrt(detail::orientation_dot(orientation, orientation));
  };

  const auto triangles = detail::triangulate_polygon<T>(count, aspectCost, isDiagonalAllowed);
  if (!triangles)
  {
    const auto anyCost = [](std::size_t, std::size_t, std::size_t) { return T{0}; };
    const bool topologicallyPossible = detail::triangulate_polygon<T>(count, anyCost, isDiagonalAllowed).has_value();
    return topologicallyPossible ? VertexRemovalStatus::InvertsFaces : VertexRemovalStatus::DuplicateEdge;
  }

  detail::retriangulate_star(mesh, vertex, star, *triangles);
  return VertexRemovalStatus::Ok;
}

} // namespace Geometry

#endif // GEOMETRY_MESH_MESHVERTEXREMOVAL_HPP
