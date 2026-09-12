#ifndef GEOMETRY_MESH_MESHMANIFOLD_HPP
#define GEOMETRY_MESH_MESHMANIFOLD_HPP

#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace Geometry
{

// User-facing manifold verification for a TriangleHalfedgeMesh. A triangle mesh represents a manifold
// surface (with boundary) when it is both edge-manifold and vertex-manifold.
//
// Contract: every algorithm and mesh member function ASSUMES the mesh is manifold and never calls
// these checks -- that assumption lets them use the cheapest implementation. These functions exist
// solely so the USER can decide when to verify manifoldness (for example after hand-building a mesh
// through the raw connectivity view); nothing in the library invokes them on the user's behalf. They
// are read-only and distinct from the connectivity consistency verified by
// TriangleHalfedgeMesh::has_valid_connectivity().

// Returns true if every edge is incident to at most two faces. In the halfedge kernel every edge owns
// two halfedges that are twins of each other, so edge-manifoldness reduces to twin symmetry: each
// halfedge's twin must point back. add_triangle preserves this by construction, so for meshes built
// solely through it this always holds; the check exists for verification and for meshes built by other
// means.
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD bool is_edge_manifold(const TriangleHalfedgeMesh<T, D, TIndex>& mesh)
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;

  for (const HalfedgeHandle halfedge : mesh.halfedges())
  {
    const HalfedgeHandle twin = mesh.get_halfedge(halfedge).twin;
    if (!twin.is_valid() || mesh.get_halfedge(twin).twin != halfedge)
    {
      return false;
    }
  }
  return true;
}

// Walks the single outgoing fan around `vertex` (twin.next orbit) and returns the number of outgoing
// halfedges reachable in it. The walk is bounded by the total halfedge count: a well-formed fan closes
// well within that bound, and a malformed (non-closing) chain on a mesh "built by other means" is
// detected by exceeding it rather than looping forever. The bound is signalled by returning
// std::nullopt so callers treat such a vertex as non-manifold instead of hanging.
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD std::optional<std::size_t>
bounded_single_fan_size(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                        typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle vertex)
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;

  const HalfedgeHandle start = mesh.get_vertex(vertex).halfedge;
  if (!start.is_valid())
  {
    return std::size_t{0};
  }

  const std::size_t limit = mesh.halfedge_count();
  std::size_t count = 0;
  HalfedgeHandle current = start;
  do
  {
    if (++count > limit)
    {
      return std::nullopt; // fan does not close within the halfedge count -> malformed
    }
    current = mesh.get_halfedge(mesh.get_halfedge(current).twin).next;
  } while (current != start);

  return count;
}

// Exhaustively verifies vertex-manifoldness: the faces incident to the vertex must form a single fan
// (one connected umbrella) rather than several fans meeting only at the vertex (as in two cones
// sharing an apex). Compares the single-fan orbit size against an independent count of every halfedge
// sourced at the vertex: a larger independent count means a second fan exists that the orbit does not
// reach, so the vertex is non-manifold. O(H) per vertex.
//
// This is a user-facing verification facility, not a precondition helper. Mesh algorithms ASSUME a
// manifold mesh and never call it; the user runs it explicitly (e.g. after hand-building a mesh
// through the raw connectivity view) to decide whether that assumption holds.
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD bool verify_vertex_manifold(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                          typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle vertex)
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;

  if (is_isolated(mesh, vertex))
  {
    return true;
  }

  const std::optional<std::size_t> singleFanCount = bounded_single_fan_size(mesh, vertex);
  if (!singleFanCount)
  {
    return false;
  }

  std::size_t incidentOutgoing = 0;
  for (const HalfedgeHandle halfedge : mesh.halfedges())
  {
    if (mesh.source_vertex(halfedge) == vertex)
    {
      ++incidentOutgoing;
    }
  }

  return *singleFanCount == incidentOutgoing;
}

// Whole-mesh exhaustive counterpart of verify_vertex_manifold. Runs in O(V + H): one pass tallies each
// vertex's total outgoing count, then one pass compares it against the bounded single-fan orbit size,
// avoiding the O(V*H) of calling the per-vertex verifier V times.
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD bool verify_vertex_manifold(const TriangleHalfedgeMesh<T, D, TIndex>& mesh)
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using VertexHandle = typename Mesh::VertexHandle;

  std::vector<std::size_t> outgoingCount(mesh.vertex_count(), 0);
  for (const HalfedgeHandle halfedge : mesh.halfedges())
  {
    ++outgoingCount[mesh.source_vertex(halfedge).get_value()];
  }

  for (const VertexHandle vertex : mesh.vertices())
  {
    if (is_isolated(mesh, vertex))
    {
      continue;
    }
    const std::optional<std::size_t> singleFanCount = bounded_single_fan_size(mesh, vertex);
    if (!singleFanCount || *singleFanCount != outgoingCount[vertex.get_value()])
    {
      return false;
    }
  }
  return true;
}

// Exhaustively verifies the mesh is a manifold surface: edge-manifold and vertex-manifold at every
// vertex. Boundaries are allowed. Like verify_vertex_manifold, this is a user-facing check that no
// algorithm calls -- algorithms assume manifoldness as a precondition.
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD bool verify_manifold(const TriangleHalfedgeMesh<T, D, TIndex>& mesh)
{
  return is_edge_manifold(mesh) && verify_vertex_manifold(mesh);
}

// Returns true if the mesh has no boundary (it is watertight): every edge has a twin.
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD bool is_closed(const TriangleHalfedgeMesh<T, D, TIndex>& mesh)
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using EdgeHandle = typename Mesh::EdgeHandle;

  for (const EdgeHandle edge : mesh.edges())
  {
    if (mesh.is_boundary(edge))
    {
      return false;
    }
  }
  return true;
}

} // namespace Geometry

#endif // GEOMETRY_MESH_MESHMANIFOLD_HPP
