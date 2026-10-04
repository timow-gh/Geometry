#ifndef GEOMETRY_MESH_DETAIL_EDGECOLLAPSE_HPP
#define GEOMETRY_MESH_DETAIL_EDGECOLLAPSE_HPP

#include "Geometry/Mesh/MeshEdgeCollapseChecks.hpp"
#include "Geometry/Mesh/MeshEdgeCollapseStatus.hpp"
#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Mesh/detail/EdgeCollapseChecks.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <cstdint>
#include <initializer_list>

namespace Geometry
{
namespace detail
{

/**
 * \internal
 * \brief Removes a face that a collapse has squashed into a 2-gon, gluing its two outer neighbours
 * into one edge.
 *
 * The 2-gon runs \p keptEdgeLoopHalfedge (s -> t) then \p removedEdgeLoopHalfedge (t -> s). Their
 * twins become twins of each other under the edge of \p keptEdgeLoopHalfedge; the edge of
 * \p removedEdgeLoopHalfedge is deleted and takes both loop halfedges with it. A crease on either
 * glued edge survives on the merged edge, so a feature line is not lost.
 */
template <typename T, std::uint8_t D, typename TIndex>
void remove_collapsed_loop(TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                           typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle keptEdgeLoopHalfedge,
                           typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle removedEdgeLoopHalfedge) noexcept
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using VertexHandle = typename Mesh::VertexHandle;
  using EdgeHandle = typename Mesh::EdgeHandle;
  using FaceHandle = typename Mesh::FaceHandle;

  const auto connectivity = mesh.connectivity();
  GEO_ASSERT(connectivity.halfedge(keptEdgeLoopHalfedge).next == removedEdgeLoopHalfedge);
  GEO_ASSERT(connectivity.halfedge(removedEdgeLoopHalfedge).next == keptEdgeLoopHalfedge);

  const HalfedgeHandle outerFromTarget = connectivity.halfedge(keptEdgeLoopHalfedge).twin;   // t -> s
  const HalfedgeHandle outerFromSource = connectivity.halfedge(removedEdgeLoopHalfedge).twin; // s -> t
  const EdgeHandle keptEdge = connectivity.halfedge(keptEdgeLoopHalfedge).edge;
  const EdgeHandle removedEdge = connectivity.halfedge(removedEdgeLoopHalfedge).edge;
  const FaceHandle face = connectivity.halfedge(keptEdgeLoopHalfedge).face;
  const VertexHandle source = connectivity.halfedge(removedEdgeLoopHalfedge).targetVertex;
  const VertexHandle target = connectivity.halfedge(keptEdgeLoopHalfedge).targetVertex;
  // Both glued halfedges on the boundary would be an edge without faces; is_collapse_ok rules it out.
  GEO_ASSERT(!connectivity.halfedge(outerFromTarget).is_boundary() || !connectivity.halfedge(outerFromSource).is_boundary());

  connectivity.halfedge(outerFromTarget).twin = outerFromSource;
  connectivity.halfedge(outerFromSource).twin = outerFromTarget;
  connectivity.halfedge(outerFromTarget).edge = keptEdge;
  connectivity.halfedge(outerFromSource).edge = keptEdge;
  connectivity.edge(keptEdge).halfedge = outerFromSource;
  connectivity.edge(keptEdge).crease = connectivity.edge(keptEdge).crease || connectivity.edge(removedEdge).crease;

  // Halfedge liveness follows the edge, so both loop halfedges move onto the deleted edge.
  connectivity.halfedge(keptEdgeLoopHalfedge).edge = removedEdge;

  if (connectivity.vertex(source).halfedge == keptEdgeLoopHalfedge)
  {
    connectivity.vertex(source).halfedge = outerFromSource;
  }
  if (connectivity.vertex(target).halfedge == removedEdgeLoopHalfedge)
  {
    connectivity.vertex(target).halfedge = outerFromTarget;
  }

  connectivity.mark_deleted(face);
  connectivity.mark_deleted(removedEdge);
}

/**
 * \internal
 * \brief Collapses \p halfedge (p -> q) without checking legality: p is deleted and its edges are
 * re-attached to q; the one or two faces on the edge are deleted and each of them merges its two
 * remaining edges into one.
 *
 * Removes 1 vertex, 3 edges and 2 faces (1 vertex, 2 edges, 1 face for a boundary edge), preserving
 * the Euler characteristic. Positions are untouched. Precondition: \c is_collapse_ok returns \c Ok.
 * O(valence(p)).
 */
template <typename T, std::uint8_t D, typename TIndex>
void collapse_halfedge_unchecked(TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                 typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle halfedge) noexcept
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using VertexHandle = typename Mesh::VertexHandle;

  GEO_ASSERT(is_collapse_ok(mesh, halfedge) == CollapseStatus::Ok);
  const auto connectivity = mesh.connectivity();

  const HalfedgeHandle opposite = connectivity.halfedge(halfedge).twin;
  const VertexHandle removed = mesh.source_vertex(halfedge);
  const VertexHandle survivor = mesh.target_vertex(halfedge);
  const VertexHandle leftApex = opposite_vertex(mesh, halfedge);
  const VertexHandle rightApex = opposite_vertex(mesh, opposite);
  const bool hasLeftFace = !mesh.is_boundary(halfedge);
  const bool hasRightFace = !mesh.is_boundary(opposite);

  const HalfedgeHandle leftNext = connectivity.halfedge(halfedge).next;  // q -> vl, or along the boundary
  const HalfedgeHandle leftPrev = connectivity.halfedge(halfedge).prev;  // vl -> p, or along the boundary
  const HalfedgeHandle rightNext = connectivity.halfedge(opposite).next; // p -> vr, or along the boundary
  const HalfedgeHandle rightPrev = connectivity.halfedge(opposite).prev; // vr -> q, or along the boundary

  // An outgoing halfedge of q that survives: the glued twin of vl -> p on the left, or the boundary
  // halfedge that continues past the removed edge.
  const HalfedgeHandle survivorOutgoing = hasLeftFace ? connectivity.halfedge(leftPrev).twin : leftNext;

  // Re-target every halfedge arriving at p. The fan walk reads only twin/next links, so rewriting
  // target vertices while walking is safe.
  HalfedgeHandle outgoing = halfedge;
  do
  {
    connectivity.halfedge(connectivity.halfedge(outgoing).twin).targetVertex = survivor;
    outgoing = mesh.next_in_outgoing_fan(outgoing);
  } while (outgoing != halfedge);

  connectivity.link(leftPrev, leftNext);
  connectivity.link(rightPrev, rightNext);

  // Each squashed face keeps the edge that already joined q to its apex.
  if (hasLeftFace)
  {
    remove_collapsed_loop(mesh, leftNext, leftPrev);
  }
  if (hasRightFace)
  {
    remove_collapsed_loop(mesh, rightPrev, rightNext);
  }

  connectivity.vertex(survivor).halfedge = survivorOutgoing;
  connectivity.mark_deleted(connectivity.halfedge(halfedge).edge);
  connectivity.mark_deleted(removed);

  for (const VertexHandle vertex : {survivor, leftApex, rightApex})
  {
    if (vertex.is_valid())
    {
      connectivity.restore_boundary_representative(vertex);
    }
  }
}

/**
 * \internal
 * \brief Collapses \p halfedge without checking legality and places the survivor at \p position.
 *
 * Precondition: \c is_collapse_ok returns \c Ok. O(valence(p)).
 *
 * \return The surviving vertex, the target of \p halfedge.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle
collapse_edge_unchecked(TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                        typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle halfedge,
                        const typename TriangleHalfedgeMesh<T, D, TIndex>::vec_t& position) noexcept
{
  const auto survivor = mesh.target_vertex(halfedge);
  collapse_halfedge_unchecked(mesh, halfedge);
  mesh.set_position(survivor, position);
  return survivor;
}

} // namespace detail
} // namespace Geometry

#endif // GEOMETRY_MESH_DETAIL_EDGECOLLAPSE_HPP
