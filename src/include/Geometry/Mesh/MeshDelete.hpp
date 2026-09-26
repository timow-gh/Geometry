#ifndef GEOMETRY_MESH_MESHDELETE_HPP
#define GEOMETRY_MESH_MESHDELETE_HPP

#include "Geometry/Mesh/MeshTopology.hpp"
#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace Geometry
{

/**
 * \brief Outcome of a face or vertex deletion; every value but \c Ok leaves the mesh untouched.
 */
enum class MeshDeleteStatus
{
  Ok,
  // The element is not in the mesh or already deleted.
  InvalidHandle,
  // The deletion would leave a vertex with two separate boundary gaps (a "bow-tie"), which the
  // manifold mesh cannot represent.
  NonManifoldVertex,
};

namespace detail
{

/**
 * \internal
 * \brief Deletes \p face without checking that the result stays vertex-manifold.
 *
 * The face's halfedges become boundary halfedges; an edge left without a face on either side is
 * deleted and spliced out of the boundary loops, and a vertex left without edges is deleted. The
 * splicing is local next/prev surgery, so it stays consistent even through an intermediate bow-tie
 * vertex; callers that delete several faces check manifoldness of the final state up front.
 * O(valence of the corners).
 */
template <typename T, std::uint8_t D, typename TIndex>
void delete_face_unchecked(TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                           typename TriangleHalfedgeMesh<T, D, TIndex>::FaceHandle face) noexcept
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using VertexHandle = typename Mesh::VertexHandle;
  using EdgeHandle = typename Mesh::EdgeHandle;
  using FaceHandle = typename Mesh::FaceHandle;

  GEO_ASSERT(mesh.contains(face) && !mesh.is_deleted(face));
  const auto connectivity = mesh.connectivity();
  const std::array<HalfedgeHandle, 3> faceHalfedges = mesh.halfedges_around_face(face);
  const std::array<VertexHandle, 3> corners{mesh.target_vertex(faceHalfedges[0]), mesh.target_vertex(faceHalfedges[1]),
                                            mesh.target_vertex(faceHalfedges[2])};

  // An edge whose other side is already boundary is left with no face at all.
  std::array<EdgeHandle, 3> faceless{};
  std::size_t facelessCount = 0;
  for (const HalfedgeHandle halfedge : faceHalfedges)
  {
    connectivity.halfedge(halfedge).face = FaceHandle{};
    if (mesh.is_boundary(connectivity.halfedge(halfedge).twin))
    {
      faceless[facelessCount++] = connectivity.halfedge(halfedge).edge;
    }
  }
  connectivity.mark_deleted(face);

  const auto link = [&](HalfedgeHandle prev, HalfedgeHandle next) {
    connectivity.halfedge(prev).next = next;
    connectivity.halfedge(next).prev = prev;
  };

  for (std::size_t i = 0; i < facelessCount; ++i)
  {
    const HalfedgeHandle first = connectivity.edge(faceless[i]).halfedge;
    const HalfedgeHandle second = connectivity.halfedge(first).twin;
    const VertexHandle firstTarget = connectivity.halfedge(first).targetVertex;
    const VertexHandle secondTarget = connectivity.halfedge(second).targetVertex;
    const HalfedgeHandle firstNext = connectivity.halfedge(first).next;
    const HalfedgeHandle firstPrev = connectivity.halfedge(first).prev;
    const HalfedgeHandle secondNext = connectivity.halfedge(second).next;
    const HalfedgeHandle secondPrev = connectivity.halfedge(second).prev;

    link(firstPrev, secondNext);
    link(secondPrev, firstNext);
    connectivity.mark_deleted(faceless[i]);

    // Each endpoint loses one outgoing halfedge; if the loop went straight back along the deleted
    // edge, that was the endpoint's last edge and the vertex is now isolated.
    if (connectivity.vertex(firstTarget).halfedge == second)
    {
      if (firstNext == second)
      {
        connectivity.mark_deleted(firstTarget);
      }
      else
      {
        connectivity.vertex(firstTarget).halfedge = firstNext;
      }
    }
    if (connectivity.vertex(secondTarget).halfedge == first)
    {
      if (secondNext == first)
      {
        connectivity.mark_deleted(secondTarget);
      }
      else
      {
        connectivity.vertex(secondTarget).halfedge = secondNext;
      }
    }
  }

  for (const VertexHandle corner : corners)
  {
    if (!mesh.is_deleted(corner))
    {
      connectivity.restore_boundary_representative(corner);
    }
  }
}

/**
 * \internal
 * \brief Whether removing a contiguous wedge of faces at \p vertex keeps it vertex-manifold.
 *
 * \p wedgeStart and \p wedgeEnd are the vertex's halfedges bounding the wedge (either direction).
 * An interior vertex only gains a gap; a boundary vertex keeps a single gap only if the wedge
 * touches its existing one, i.e. a bounding edge is already a boundary edge.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD bool wedge_removal_keeps_manifold(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                                typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle vertex,
                                                typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle wedgeStart,
                                                typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle wedgeEnd)
{
  return !is_boundary(mesh, vertex) || mesh.is_boundary(mesh.get_halfedge(wedgeStart).edge)
         || mesh.is_boundary(mesh.get_halfedge(wedgeEnd).edge);
}

} // namespace detail

/**
 * \brief Deletes \p face, leaving a hole (or growing the boundary) in its place.
 *
 * Edges left without any face and vertices left without any edge are deleted as well; all removed
 * elements are tombstoned. Rejected when a corner is a boundary vertex whose two edges in the face
 * are both interior: the face sits between two others there, and deleting it would split the
 * corner's fan into two. The Euler characteristic changes (it is a topological cut, not an Euler
 * operator). O(valence of the corners).
 *
 * \return \c MeshDeleteStatus::Ok after deleting, otherwise why the mesh was left untouched.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD MeshDeleteStatus delete_face(TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                           typename TriangleHalfedgeMesh<T, D, TIndex>::FaceHandle face)
{
  using HalfedgeHandle = typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle;

  if (!mesh.contains(face) || mesh.is_deleted(face))
  {
    return MeshDeleteStatus::InvalidHandle;
  }

  const std::array<HalfedgeHandle, 3> faceHalfedges = mesh.halfedges_around_face(face);
  for (std::size_t i = 0; i < 3; ++i)
  {
    const HalfedgeHandle incoming = faceHalfedges[i];
    const HalfedgeHandle outgoing = faceHalfedges[(i + 1) % 3];
    if (!detail::wedge_removal_keeps_manifold(mesh, mesh.target_vertex(incoming), incoming, outgoing))
    {
      return MeshDeleteStatus::NonManifoldVertex;
    }
  }

  detail::delete_face_unchecked(mesh, face);
  return MeshDeleteStatus::Ok;
}

/**
 * \brief Deletes \p vertex together with every face and edge around it, leaving a hole.
 *
 * Neighbours left without edges are deleted too. Rejected when a neighbour is a boundary vertex
 * whose wedge of removed faces does not touch its boundary gap: it would end up with two gaps. The
 * manifoldness of the final state is checked up front, so the faces can then be deleted in any
 * order. An isolated vertex is simply deleted. O(sum of the neighbours' valences).
 *
 * \return \c MeshDeleteStatus::Ok after deleting, otherwise why the mesh was left untouched.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD MeshDeleteStatus delete_vertex(TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                             typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle vertex)
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using FaceHandle = typename Mesh::FaceHandle;

  if (!mesh.contains(vertex) || mesh.is_deleted(vertex))
  {
    return MeshDeleteStatus::InvalidHandle;
  }
  if (is_isolated(mesh, vertex))
  {
    mesh.connectivity().mark_deleted(vertex);
    return MeshDeleteStatus::Ok;
  }

  // The removed faces at a neighbour are the (one or two) faces on its spoke; the wedge they form is
  // bounded by their far edges at the neighbour, or by the spoke itself on a side without a face.
  const Mesh& constMesh = mesh;
  for (auto outgoing = constMesh.outgoing_halfedges(vertex).circulator(); outgoing.is_valid(); ++outgoing)
  {
    const HalfedgeHandle spoke = outgoing.get_halfedgehandle();
    const HalfedgeHandle spokeTwin = mesh.get_halfedge(spoke).twin;
    const HalfedgeHandle wedgeStart = mesh.is_boundary(spoke) ? spoke : mesh.get_halfedge(spoke).next;
    const HalfedgeHandle wedgeEnd = mesh.is_boundary(spokeTwin) ? spokeTwin : mesh.get_halfedge(spokeTwin).prev;
    if (!detail::wedge_removal_keeps_manifold(mesh, mesh.target_vertex(spoke), wedgeStart, wedgeEnd))
    {
      return MeshDeleteStatus::NonManifoldVertex;
    }
  }

  for (const FaceHandle face : mesh.faces_around_vertex(vertex))
  {
    detail::delete_face_unchecked(mesh, face);
  }
  GEO_ASSERT(mesh.is_deleted(vertex));
  return MeshDeleteStatus::Ok;
}

} // namespace Geometry

#endif // GEOMETRY_MESH_MESHDELETE_HPP
