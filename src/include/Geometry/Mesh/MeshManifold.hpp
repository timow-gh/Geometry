#ifndef GEOMETRY_MESH_MESHMANIFOLD_HPP
#define GEOMETRY_MESH_MESHMANIFOLD_HPP

#include "Geometry/Mesh/MeshTopology.hpp"
#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace Geometry
{

/**
 * \brief Verifies every edge is shared by at most two faces (the edge-manifold half of
 * the manifold contract).
 *
 * Expensive, explicit check: the library assumes manifoldness and never calls it for
 * you. Run it after building a mesh by means other than \c add_triangle, which
 * preserves the property by construction. O(H).
 *
 * \return \c true if the mesh is edge-manifold.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD bool verify_edge_manifold(const TriangleHalfedgeMesh<T, D, TIndex>& mesh)
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

namespace detail
{

/**
 * \internal
 * \brief Counts the outgoing halfedges in the single twin.next fan around \p vertex.
 *
 * Building block for the vertex-manifold checks; not part of the public API.
 * The walk is capped at the mesh's halfedge count and returns \c std::nullopt when a
 * malformed, non-closing fan would otherwise loop forever, so callers can treat such a
 * vertex as non-manifold.
 *
 * \return Fan size, or \c std::nullopt if the fan does not close within the bound.
 */
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

  const std::size_t limit = mesh.halfedge_storage_size();
  std::size_t count = 0;
  HalfedgeHandle current = start;
  do
  {
    if (++count > limit)
    {
      return std::nullopt; // fan does not close within the halfedge count -> malformed
    }
    current = mesh.next_in_outgoing_fan(current);
  } while (current != start);

  return count;
}

} // namespace detail

/**
 * \brief Verifies the faces around \p vertex form a single fan, not several fans that
 * meet only at the vertex (e.g. two cones sharing an apex).
 *
 * Expensive, explicit check for one suspect vertex; use the whole-mesh overload to
 * check every vertex. It compares the single-fan orbit against an independent count of
 * halfedges sourced at the vertex, so an unreachable second fan is detected. O(H).
 *
 * \return \c true if \p vertex is vertex-manifold.
 */
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

  const std::optional<std::size_t> singleFanCount = detail::bounded_single_fan_size(mesh, vertex);
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

/**
 * \brief Verifies vertex-manifoldness at every vertex.
 *
 * Prefer this over calling the per-vertex overload in a loop: a single outgoing-count
 * pass plus one fan-orbit pass runs in O(V + H) instead of O(V*H).
 *
 * \return \c true if every vertex is vertex-manifold.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD bool verify_vertex_manifold(const TriangleHalfedgeMesh<T, D, TIndex>& mesh)
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using VertexHandle = typename Mesh::VertexHandle;

  std::vector<std::size_t> outgoingCount(mesh.vertex_storage_size(), 0);
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
    const std::optional<std::size_t> singleFanCount = detail::bounded_single_fan_size(mesh, vertex);
    if (!singleFanCount || *singleFanCount != outgoingCount[vertex.get_value()])
    {
      return false;
    }
  }
  return true;
}

/**
 * \brief Verifies the mesh is a manifold surface: edge-manifold and vertex-manifold
 * everywhere, boundaries allowed.
 *
 * The single yes/no check to run before handing a hand-built mesh to the rest of the
 * library. Expensive; the library never calls it for you.
 *
 * \return \c true if the mesh is a manifold surface.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD bool verify_manifold(const TriangleHalfedgeMesh<T, D, TIndex>& mesh)
{
  return verify_edge_manifold(mesh) && verify_vertex_manifold(mesh);
}

/**
 * \brief Verifies the mesh is watertight -- it has no boundary edges.
 *
 * Run when an algorithm requires a closed surface (e.g. volume or inside/outside
 * queries). Distinct from and independent of manifoldness.
 *
 * \return \c true if the mesh has no boundary edges.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD bool verify_closed(const TriangleHalfedgeMesh<T, D, TIndex>& mesh)
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
