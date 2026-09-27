#ifndef GEOMETRY_MESH_DETAIL_VERTEXSTAR_HPP
#define GEOMETRY_MESH_DETAIL_VERTEXSTAR_HPP

#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Utils/Assert.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace Geometry
{
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
} // namespace Geometry

#endif // GEOMETRY_MESH_DETAIL_VERTEXSTAR_HPP
