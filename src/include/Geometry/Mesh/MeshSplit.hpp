#ifndef GEOMETRY_MESH_MESHSPLIT_HPP
#define GEOMETRY_MESH_MESHSPLIT_HPP

#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Mesh/detail/MeshEditing.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace Geometry
{

/**
 * \brief Splits \p edge at a new vertex placed at \p position and connects that vertex to the apex
 * of each incident face.
 *
 * Each incident face becomes two, so an interior edge's two faces become four and a boundary edge's
 * one face becomes two. Adds 1 vertex, 3 edges and 2 faces (1, 2, 1 on the boundary), preserving the
 * Euler characteristic. Every existing handle stays valid and new elements are only appended, so side
 * arrays indexed by storage just grow. For a refinement that tracks where each element came from, what
 * keeps which handle is fixed: with a -> b the stored halfedge of \p edge, \p edge then joins a to
 * the new vertex and keeps that halfedge; a new edge joins the new vertex to b and inherits the crease
 * flag, so a feature line stays marked along both halves; each incident face keeps its handle for the
 * half at a. The edges to the apexes are not creases. Topology only: \p position is not checked, and
 * a point off the edge can fold the surface. O(1) amortized.
 *
 * \return The new vertex, or an invalid handle if \p edge is not live (the mesh is left untouched).
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle
split_edge(TriangleHalfedgeMesh<T, D, TIndex>& mesh,
           typename TriangleHalfedgeMesh<T, D, TIndex>::EdgeHandle edge,
           const typename TriangleHalfedgeMesh<T, D, TIndex>::vec_t& position)
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using VertexHandle = typename Mesh::VertexHandle;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using FaceHandle = typename Mesh::FaceHandle;

  if (!mesh.is_live(edge))
  {
    return VertexHandle{};
  }

  const auto connectivity = mesh.connectivity();
  const HalfedgeHandle forward = connectivity.edge(edge).halfedge; // a -> b
  const HalfedgeHandle backward = connectivity.halfedge(forward).twin; // b -> a
  const bool hasLeftFace = !mesh.is_boundary(forward);
  const bool hasRightFace = !mesh.is_boundary(backward);
  GEO_ASSERT(hasLeftFace || hasRightFace);

  // Reserving everything up front means nothing below can throw, so the mesh is never half-split.
  const std::size_t splitFaceCount = (hasLeftFace ? 1U : 0U) + (hasRightFace ? 1U : 0U);
  connectivity.reserve_vertices(1);
  connectivity.reserve_edges(1 + splitFaceCount);
  connectivity.reserve_halfedges(2 * (1 + splitFaceCount));
  connectivity.reserve_faces(splitFaceCount);

  [[maybe_unused]] const VertexHandle start = mesh.source_vertex(forward);
  const VertexHandle end = mesh.target_vertex(forward);
  const HalfedgeHandle forwardNext = connectivity.halfedge(forward).next;
  const HalfedgeHandle forwardPrev = connectivity.halfedge(forward).prev;
  const HalfedgeHandle backwardNext = connectivity.halfedge(backward).next;
  const HalfedgeHandle backwardPrev = connectivity.halfedge(backward).prev;

  const VertexHandle middle = connectivity.new_vertex(position);
  const HalfedgeHandle middleToEnd = detail::new_edge_between(mesh, middle, end);
  const HalfedgeHandle endToMiddle = connectivity.halfedge(middleToEnd).twin;
  connectivity.edge(connectivity.halfedge(middleToEnd).edge).crease = connectivity.edge(edge).crease;

  // forward becomes a -> middle. backward becomes middle -> a: its target stays, and its source
  // follows from the prev it is linked to below.
  connectivity.halfedge(forward).targetVertex = middle;

  if (hasLeftFace)
  {
    // (a, b, apex) becomes (a, middle, apex) and (middle, b, apex).
    const VertexHandle apex = connectivity.halfedge(forwardNext).targetVertex;
    const HalfedgeHandle middleToApex = detail::new_edge_between(mesh, middle, apex);
    const HalfedgeHandle apexToMiddle = connectivity.halfedge(middleToApex).twin;
    detail::link_triangle(mesh, connectivity.halfedge(forward).face, forward, middleToApex, forwardPrev);
    detail::link_triangle(mesh, connectivity.new_face(), middleToEnd, forwardNext, apexToMiddle);
  }
  else
  {
    connectivity.link(forward, middleToEnd);
    connectivity.link(middleToEnd, forwardNext);
  }

  if (hasRightFace)
  {
    // (b, a, apex) becomes (middle, a, apex) and (b, middle, apex).
    const VertexHandle apex = connectivity.halfedge(backwardNext).targetVertex;
    const HalfedgeHandle apexToMiddle = detail::new_edge_between(mesh, apex, middle);
    const HalfedgeHandle middleToApex = connectivity.halfedge(apexToMiddle).twin;
    detail::link_triangle(mesh, connectivity.halfedge(backward).face, backward, backwardNext, apexToMiddle);
    detail::link_triangle(mesh, connectivity.new_face(), endToMiddle, middleToApex, backwardPrev);
  }
  else
  {
    connectivity.link(backwardPrev, endToMiddle);
    connectivity.link(endToMiddle, backward);
  }

  // b no longer starts backward; its replacement lies on the same side of the edge, so a boundary
  // representative stays a boundary halfedge. The new vertex's boundary halfedge, if any, is the one
  // on the boundary side.
  if (connectivity.vertex(end).halfedge == backward)
  {
    connectivity.vertex(end).halfedge = endToMiddle;
  }
  connectivity.vertex(middle).halfedge = hasLeftFace ? backward : middleToEnd;

  GEO_ASSERT(mesh.source_vertex(forward) == start && mesh.source_vertex(backward) == middle);
  GEO_ASSERT(mesh.find_halfedge(middle, end) == middleToEnd);
  return middle;
}

/**
 * \brief Splits \p face into three at a new vertex placed at \p position, connected to its corners.
 *
 * Adds 1 vertex, 3 edges and 2 faces, preserving the Euler characteristic. Every existing handle stays
 * valid and new elements are only appended, so side arrays indexed by storage just grow. \p face keeps
 * its handle for the new triangle over its stored halfedge (the first of \c halfedges_around_face).
 * The new edges are not creases. Topology only: \p position is not checked, and a point outside the
 * face can fold the surface. O(1) amortized.
 *
 * \return The new vertex, or an invalid handle if \p face is not live (the mesh is left untouched).
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle
split_face(TriangleHalfedgeMesh<T, D, TIndex>& mesh,
           typename TriangleHalfedgeMesh<T, D, TIndex>::FaceHandle face,
           const typename TriangleHalfedgeMesh<T, D, TIndex>::vec_t& position)
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using VertexHandle = typename Mesh::VertexHandle;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;

  if (!mesh.is_live(face))
  {
    return VertexHandle{};
  }

  // Reserving everything up front means nothing below can throw, so the mesh is never half-split.
  const auto connectivity = mesh.connectivity();
  connectivity.reserve_vertices(1);
  connectivity.reserve_edges(3);
  connectivity.reserve_halfedges(6);
  connectivity.reserve_faces(2);

  const std::array<HalfedgeHandle, 3> sides = mesh.halfedges_around_face(face);
  const VertexHandle center = connectivity.new_vertex(position);

  // toCenter[i] runs from the target of sides[i] to the center.
  std::array<HalfedgeHandle, 3> toCenter{};
  for (std::size_t i = 0; i < 3; ++i)
  {
    toCenter[i] = detail::new_edge_between(mesh, connectivity.halfedge(sides[i]).targetVertex, center);
  }

  for (std::size_t i = 0; i < 3; ++i)
  {
    // The spoke back out to the source of sides[i] is the twin of the previous side's spoke in.
    const HalfedgeHandle fromCenter = connectivity.halfedge(toCenter[(i + 2) % 3]).twin;
    detail::link_triangle(mesh, i == 0 ? face : connectivity.new_face(), sides[i], toCenter[i], fromCenter);
  }

  connectivity.vertex(center).halfedge = connectivity.halfedge(toCenter[0]).twin;
  GEO_ASSERT(mesh.count_incident_faces(center) == 3);
  return center;
}

} // namespace Geometry

#endif // GEOMETRY_MESH_MESHSPLIT_HPP
