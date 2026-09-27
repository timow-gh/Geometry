#ifndef GEOMETRY_MESH_DETAIL_EDGECOLLAPSECHECKS_HPP
#define GEOMETRY_MESH_DETAIL_EDGECOLLAPSECHECKS_HPP

#include "Geometry/Mesh/MeshTopology.hpp"
#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Mesh/detail/FaceGeometry.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <array>
#include <cstdint>
#include <initializer_list>
#include <linal/vec.hpp>

namespace Geometry
{
namespace detail
{

/**
 * \internal
 * \brief The vertex opposite \p halfedge in its face, or an invalid handle for a boundary halfedge.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle
opposite_vertex(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle halfedge) noexcept
{
  using VertexHandle = typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle;

  if (mesh.is_boundary(halfedge))
  {
    return VertexHandle{};
  }
  return mesh.target_vertex(mesh.get_halfedge(halfedge).next);
}

/**
 * \internal
 * \brief Whether every edge of \p face is a boundary edge, i.e. the face is a component on its own.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD bool is_isolated_face(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                    typename TriangleHalfedgeMesh<T, D, TIndex>::FaceHandle face) noexcept
{
  for (const auto halfedge : mesh.halfedges_around_face(face))
  {
    if (!mesh.is_boundary(mesh.get_halfedge(halfedge).twin))
    {
      return false;
    }
  }
  return true;
}

/**
 * \internal
 * \brief Link condition of the edge (p, q) under \p halfedge (p -> q): every common neighbour of
 * p and q is a vertex opposite the edge.
 *
 * Probes q's fan for each neighbour of p instead of tagging p's one-ring in a side buffer: valence
 * is small in practice, and this keeps the check allocation-free. O(valence(p) * valence(q)).
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD bool satisfies_link_condition(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                            typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle halfedge) noexcept
{
  using VertexHandle = typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle;

  const VertexHandle removed = mesh.source_vertex(halfedge);
  const VertexHandle survivor = mesh.target_vertex(halfedge);
  const VertexHandle leftApex = opposite_vertex(mesh, halfedge);
  const VertexHandle rightApex = opposite_vertex(mesh, mesh.get_halfedge(halfedge).twin);

  // Both faces sharing one apex means they are the same triangle glued along all three edges.
  if (leftApex.is_valid() && leftApex == rightApex)
  {
    return false;
  }

  for (auto neighbour = mesh.vertices(removed).circulator(); neighbour.is_valid(); ++neighbour)
  {
    const VertexHandle candidate = neighbour.get_vertexhandle();
    if (candidate == survivor || candidate == leftApex || candidate == rightApex)
    {
      continue;
    }
    if (mesh.find_halfedge(survivor, candidate).is_valid())
    {
      return false;
    }
  }
  return true;
}

/**
 * \internal
 * \brief Whether \p apex, a vertex opposite a collapsing edge, is an interior vertex of valence 3.
 *
 * Once the link condition holds, such an apex is adjacent only to the edge's endpoints and the
 * other apex, so its three faces plus the other face on the edge close into a tetrahedron. The
 * collapse would fold that component into two coincident triangles.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD bool is_tetrahedron_apex(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                       typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle apex)
{
  return apex.is_valid() && !is_boundary(mesh, apex) && valence(mesh, apex) == 3;
}

/**
 * \internal
 * \brief Whether moving a triangle's corners from \p before to \p after flips or flattens it.
 *
 * A triangle that was already degenerate has no orientation to lose, so it counts as inverted only
 * if it stays degenerate. Works in 2D and 3D via \c triangle_orientation.
 */
template <typename T, std::uint8_t D>
GEO_NODISCARD bool triangle_inverts(const std::array<linal::vec<T, D>, 3>& before,
                                    const std::array<linal::vec<T, D>, 3>& after) noexcept
{
  const auto orientationBefore = triangle_orientation(before[0], before[1], before[2]);
  const auto orientationAfter = triangle_orientation(after[0], after[1], after[2]);
  if (orientation_dot(orientationAfter, orientationAfter) == T{0})
  {
    return true;
  }
  return orientation_dot(orientationBefore, orientationBefore) != T{0}
         && orientation_dot(orientationBefore, orientationAfter) <= T{0};
}

/**
 * \internal
 * \brief The face across \p side once \p collapsing has been collapsed, or an invalid handle across a
 * boundary.
 *
 * Usually the face on the twin of \p side. The exception is a face that the collapse removes: it is
 * squashed into two coincident edges that get glued, so the face beyond the other glued edge becomes
 * the new neighbour. Precondition: \p side does not itself belong to a removed face, and
 * \c is_collapse_ok holds (which rules out both glued edges bordering removed faces).
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD typename TriangleHalfedgeMesh<T, D, TIndex>::FaceHandle
face_across_after_collapse(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                           typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle collapsing,
                           typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle side) noexcept
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using FaceHandle = typename Mesh::FaceHandle;

  const HalfedgeHandle opposite = mesh.get_halfedge(side).twin;
  const FaceHandle across = mesh.get_halfedge(opposite).face;
  for (const HalfedgeHandle removedSide : {collapsing, mesh.get_halfedge(collapsing).twin})
  {
    if (mesh.is_boundary(removedSide) || across != mesh.get_halfedge(removedSide).face)
    {
      continue;
    }
    const HalfedgeHandle next = mesh.get_halfedge(removedSide).next;
    const HalfedgeHandle prev = mesh.get_halfedge(removedSide).prev;
    GEO_ASSERT(opposite == next || opposite == prev);
    const HalfedgeHandle glued = opposite == next ? prev : next;
    const FaceHandle beyond = mesh.get_halfedge(mesh.get_halfedge(glued).twin).face;
    GEO_ASSERT(!beyond.is_valid() || (beyond != mesh.get_halfedge(collapsing).face
                                      && beyond != mesh.get_halfedge(mesh.get_halfedge(collapsing).twin).face));
    return beyond;
  }
  return across;
}

} // namespace detail
} // namespace Geometry

#endif // GEOMETRY_MESH_DETAIL_EDGECOLLAPSECHECKS_HPP
