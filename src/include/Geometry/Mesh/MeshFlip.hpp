#ifndef GEOMETRY_MESH_MESHFLIP_HPP
#define GEOMETRY_MESH_MESHFLIP_HPP

#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Mesh/detail/MeshEditing.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <cstdint>

namespace Geometry
{

/**
 * \brief Outcome of an edge flip validity check; every value but \c Ok leaves the mesh untouched.
 */
enum class FlipStatus
{
  Ok,
  // The edge is not in the mesh or already deleted.
  InvalidHandle,
  // The edge has a face on one side only, so there is no quad to flip in.
  BoundaryEdge,
  // The two apexes are already joined by an edge (or are the same vertex): the flipped edge would
  // duplicate it. This is also the case at an interior endpoint of valence 3, whose flip would leave
  // two triangles covering each other.
  DiagonalExists,
};

/**
 * \brief Checks whether flipping \p edge keeps the mesh a manifold of unchanged topology.
 *
 * Purely topological: a flip that is legal here can still fold the surface when the quad around the
 * edge is not strictly convex, so callers that need a valid geometry check that themselves.
 * O(valence of an apex).
 *
 * \return \c FlipStatus::Ok if the flip is legal, otherwise the first violated rule.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD FlipStatus is_flip_ok(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                    typename TriangleHalfedgeMesh<T, D, TIndex>::EdgeHandle edge) noexcept
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using VertexHandle = typename Mesh::VertexHandle;

  if (!mesh.is_live(edge))
  {
    return FlipStatus::InvalidHandle;
  }
  if (mesh.is_boundary(edge))
  {
    return FlipStatus::BoundaryEdge;
  }

  const HalfedgeHandle forward = mesh.get_edge(edge).halfedge;
  const HalfedgeHandle backward = mesh.get_halfedge(forward).twin;
  const VertexHandle leftApex = mesh.target_vertex(mesh.get_halfedge(forward).next);
  const VertexHandle rightApex = mesh.target_vertex(mesh.get_halfedge(backward).next);
  if (leftApex == rightApex || mesh.find_halfedge(leftApex, rightApex).is_valid())
  {
    return FlipStatus::DiagonalExists;
  }
  return FlipStatus::Ok;
}

namespace detail
{

/**
 * \internal
 * \brief Flips \p edge without checking legality: the edge (a, b) between faces (a, b, c) and
 * (b, a, d) becomes (d, c), between faces (c, a, d) and (d, b, c).
 *
 * The edge, its halfedges and both faces keep their handles; the face on the side of the edge's stored
 * halfedge becomes the one at a. The crease flag is cleared: it marked the feature line from a to b,
 * which the flipped edge no longer follows. Precondition: \c is_flip_ok returns \c Ok. O(1).
 */
template <typename T, std::uint8_t D, typename TIndex>
void flip_edge_unchecked(TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                         typename TriangleHalfedgeMesh<T, D, TIndex>::EdgeHandle edge) noexcept
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using VertexHandle = typename Mesh::VertexHandle;

  GEO_ASSERT(is_flip_ok(mesh, edge) == FlipStatus::Ok);
  const auto connectivity = mesh.connectivity();
  const HalfedgeHandle forward = connectivity.edge(edge).halfedge;    // a -> b
  const HalfedgeHandle backward = connectivity.halfedge(forward).twin; // b -> a
  const HalfedgeHandle forwardNext = connectivity.halfedge(forward).next;   // b -> c
  const HalfedgeHandle forwardPrev = connectivity.halfedge(forward).prev;   // c -> a
  const HalfedgeHandle backwardNext = connectivity.halfedge(backward).next; // a -> d
  const HalfedgeHandle backwardPrev = connectivity.halfedge(backward).prev; // d -> b
  const VertexHandle start = connectivity.halfedge(backward).targetVertex;
  const VertexHandle end = connectivity.halfedge(forward).targetVertex;

  // a and b lose the flipped edge; the next halfedge around each in its old face is still outgoing
  // and interior, so a boundary representative is never replaced.
  if (connectivity.vertex(start).halfedge == forward)
  {
    connectivity.vertex(start).halfedge = backwardNext;
  }
  if (connectivity.vertex(end).halfedge == backward)
  {
    connectivity.vertex(end).halfedge = forwardNext;
  }

  connectivity.halfedge(forward).targetVertex = connectivity.halfedge(forwardNext).targetVertex;
  connectivity.halfedge(backward).targetVertex = connectivity.halfedge(backwardNext).targetVertex;
  detail::link_triangle(mesh, connectivity.halfedge(forward).face, forward, forwardPrev, backwardNext);
  detail::link_triangle(mesh, connectivity.halfedge(backward).face, backward, backwardPrev, forwardNext);
  connectivity.edge(edge).crease = false;
}

} // namespace detail

/**
 * \brief Flips \p edge: replaces it by the other diagonal of the quad formed by its two faces.
 *
 * Changes no element counts, so the Euler characteristic is preserved, and keeps every handle valid
 * (see \c detail::flip_edge_unchecked for which face ends up where). Topology only, per
 * \c is_flip_ok: the caller decides whether the flip is geometrically sound, e.g. a constrained
 * triangulation flips only strictly convex quads. Flipping the same edge again joins the original
 * endpoints once more, with the halfedges reversed. O(valence of an apex).
 *
 * \return \c FlipStatus::Ok after flipping, otherwise why the mesh was left untouched.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD FlipStatus flip_edge(TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                   typename TriangleHalfedgeMesh<T, D, TIndex>::EdgeHandle edge) noexcept
{
  const FlipStatus status = is_flip_ok(mesh, edge);
  if (status == FlipStatus::Ok)
  {
    detail::flip_edge_unchecked(mesh, edge);
  }
  return status;
}

} // namespace Geometry

#endif // GEOMETRY_MESH_MESHFLIP_HPP
