#ifndef GEOMETRY_MESH_DETAIL_MESHEDITING_HPP
#define GEOMETRY_MESH_DETAIL_MESHEDITING_HPP

#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <cstdint>

namespace Geometry
{
namespace detail
{

/**
 * \internal
 * \brief Appends an edge between \p source and \p target and returns its halfedge source -> target.
 *
 * Only the pair itself is set up (targets, twins, edge); next/prev and faces are left to the caller,
 * which places both halfedges into face cycles or boundary loops. Reserve the storage first to keep
 * the operator from failing half-way. O(1) amortized.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle
new_edge_between(TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                 typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle source,
                 typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle target)
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using EdgeHandle = typename Mesh::EdgeHandle;

  GEO_ASSERT(source != target);
  const auto connectivity = mesh.connectivity();
  const HalfedgeHandle forward = connectivity.new_halfedge();
  const HalfedgeHandle backward = connectivity.new_halfedge();
  const EdgeHandle edge = connectivity.new_edge(forward);
  connectivity.halfedge(forward).targetVertex = target;
  connectivity.halfedge(backward).targetVertex = source;
  connectivity.halfedge(forward).twin = backward;
  connectivity.halfedge(backward).twin = forward;
  connectivity.halfedge(forward).edge = edge;
  connectivity.halfedge(backward).edge = edge;
  return forward;
}

/**
 * \internal
 * \brief Makes \p first, \p second, \p third the halfedge cycle of \p face and stores \p first as its
 * representative.
 *
 * Rewrites next, prev and face of all three, so halfedges taken over from another face need no
 * unlinking first; the source of each halfedge is then the target of the one before it. O(1).
 */
template <typename T, std::uint8_t D, typename TIndex>
void link_triangle(TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                   typename TriangleHalfedgeMesh<T, D, TIndex>::FaceHandle face,
                   typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle first,
                   typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle second,
                   typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle third) noexcept
{
  const auto connectivity = mesh.connectivity();
  GEO_ASSERT(connectivity.halfedge(first).targetVertex != connectivity.halfedge(second).targetVertex
             && connectivity.halfedge(second).targetVertex != connectivity.halfedge(third).targetVertex
             && connectivity.halfedge(third).targetVertex != connectivity.halfedge(first).targetVertex);
  connectivity.link(first, second);
  connectivity.link(second, third);
  connectivity.link(third, first);
  connectivity.halfedge(first).face = face;
  connectivity.halfedge(second).face = face;
  connectivity.halfedge(third).face = face;
  connectivity.face(face).set_halfedgehandle(first);
}

} // namespace detail
} // namespace Geometry

#endif // GEOMETRY_MESH_DETAIL_MESHEDITING_HPP
