#ifndef GEOMETRY_MESH_MESHEDGECOLLAPSE_HPP
#define GEOMETRY_MESH_MESHEDGECOLLAPSE_HPP

#include "Geometry/Mesh/MeshEdgeCollapseChecks.hpp"
#include "Geometry/Mesh/MeshEdgeCollapseStatus.hpp"
#include "Geometry/Mesh/MeshQuality.hpp"
#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Mesh/detail/EdgeCollapse.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <cstdint>

namespace Geometry
{

/**
 * \brief Halfedge collapse: removes the source p of \p halfedge by merging it into the target q,
 * which keeps its position.
 *
 * The degree-of-freedom-free Euler operator of incremental decimation (Kobbelt et al. 98): it is
 * both an edge collapse with the merged vertex placed at q and a vertex decimation whose hole is
 * fan-triangulated from q. Removes 1 vertex, 3 edges and 2 faces (1, 2, 1 on the boundary), so the
 * Euler characteristic is preserved. Removed elements are tombstoned, so every other handle stays
 * valid until \c garbage_collection(). Refuses collapses that would fold the surface, per
 * \c check_collapse; use \c collapse_halfedge_topology_only to skip the geometry.
 * O(valence(p) * valence(q)).
 *
 * \return \c CollapseStatus::Ok after collapsing, otherwise the reason the mesh was left untouched.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD CollapseStatus collapse_halfedge(TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                               typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle halfedge,
                                               const MeshGeometryLimits<T>& limits = {})
{
  if (!mesh.is_live(halfedge))
  {
    return CollapseStatus::InvalidHandle;
  }
  const CollapseStatus status = check_collapse(mesh, halfedge, mesh.get_position(mesh.target_vertex(halfedge)), limits);
  if (status == CollapseStatus::Ok)
  {
    detail::collapse_halfedge_unchecked(mesh, halfedge);
  }
  return status;
}

/**
 * \brief \c collapse_halfedge checking topology only: the mesh stays a valid manifold, but the
 * surface may fold over itself.
 *
 * For callers that validate geometry themselves or deliberately accept fold-overs.
 * O(valence(p) * valence(q)).
 *
 * \return \c CollapseStatus::Ok after collapsing, otherwise the topological reason the mesh was left
 * untouched; never \c CollapseStatus::InvertsFaces.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD CollapseStatus collapse_halfedge_topology_only(TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                                             typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle halfedge)
{
  const CollapseStatus status = is_collapse_ok(mesh, halfedge);
  if (status == CollapseStatus::Ok)
  {
    detail::collapse_halfedge_unchecked(mesh, halfedge);
  }
  return status;
}

/**
 * \brief Edge collapse: merges both endpoints of \p edge into one vertex placed at \p position.
 *
 * A halfedge collapse followed by moving the survivor: the connectivity after collapsing (p, q) is
 * the same whichever endpoint survives, so the merged position is the collapse's only degree of
 * freedom. Which endpoint's handle survives is unspecified; use the returned one. Refuses collapses
 * that would fold the surface at \p position, per \c check_collapse; use
 * \c collapse_edge_topology_only to skip the geometry. O(valence(p) * valence(q)).
 *
 * \return The surviving vertex, or an invalid one with the reason the mesh was left untouched.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD CollapseResult<typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle>
collapse_edge(TriangleHalfedgeMesh<T, D, TIndex>& mesh,
              typename TriangleHalfedgeMesh<T, D, TIndex>::EdgeHandle edge,
              const typename TriangleHalfedgeMesh<T, D, TIndex>::vec_t& position,
              const MeshGeometryLimits<T>& limits = {})
{
  using VertexHandle = typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle;

  if (!mesh.is_live(edge))
  {
    return {VertexHandle{}, CollapseStatus::InvalidHandle};
  }
  const auto halfedge = mesh.get_edge(edge).halfedge;
  const CollapseStatus status = check_collapse(mesh, halfedge, position, limits);
  if (status != CollapseStatus::Ok)
  {
    return {VertexHandle{}, status};
  }
  return {detail::collapse_edge_unchecked(mesh, halfedge, position), CollapseStatus::Ok};
}

/**
 * \brief \c collapse_edge checking topology only: the mesh stays a valid manifold, but the surface
 * may fold over itself at \p position.
 *
 * For callers that validate geometry themselves or deliberately accept fold-overs.
 * O(valence(p) * valence(q)).
 *
 * \return The surviving vertex, or an invalid one with the topological reason the mesh was left
 * untouched; never \c CollapseStatus::InvertsFaces.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD CollapseResult<typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle>
collapse_edge_topology_only(TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                            typename TriangleHalfedgeMesh<T, D, TIndex>::EdgeHandle edge,
                            const typename TriangleHalfedgeMesh<T, D, TIndex>::vec_t& position)
{
  using VertexHandle = typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle;

  if (!mesh.is_live(edge))
  {
    return {VertexHandle{}, CollapseStatus::InvalidHandle};
  }
  const auto halfedge = mesh.get_edge(edge).halfedge;
  const CollapseStatus status = is_collapse_ok(mesh, halfedge);
  if (status != CollapseStatus::Ok)
  {
    return {VertexHandle{}, status};
  }
  return {detail::collapse_edge_unchecked(mesh, halfedge, position), CollapseStatus::Ok};
}

} // namespace Geometry

#endif // GEOMETRY_MESH_MESHEDGECOLLAPSE_HPP
