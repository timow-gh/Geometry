#ifndef GEOMETRY_MESH_MESHVERTEXREMOVAL_HPP
#define GEOMETRY_MESH_MESHVERTEXREMOVAL_HPP

#include "Geometry/Mesh/MeshCollapse.hpp"
#include "Geometry/Mesh/MeshTopology.hpp"
#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <cstdint>
#include <linal/vec.hpp>

namespace Geometry
{

/**
 * \brief Removes \p vertex by collapsing it into the nearest neighbour that it can legally be merged
 * into without flipping or flattening a face.
 *
 * Vertex removal whose hole is fan-triangulated from the chosen neighbour, i.e. a halfedge collapse
 * (see \c collapse_halfedge) with the target picked here. The shortest edge is the choice: it is
 * cheap, deterministic, and moves the surface least; callers needing another criterion (e.g. a
 * quadric error) should rank the outgoing halfedges themselves and call \c collapse_halfedge.
 * Candidates are rejected by \c is_collapse_ok and \c collapse_inverts_faces (the survivor keeps its
 * position); ties go to the first candidate in fan order. An isolated vertex is simply deleted. The
 * removed elements are tombstoned. O(valence(v)^2 * valence(neighbour)).
 *
 * \return The neighbour that absorbed \p vertex (invalid for a deleted isolated vertex), or the mesh
 * untouched and a status: \c InvalidHandle for a missing or deleted vertex; \c InvertsFaces if some
 * collapse was topologically legal but every legal one folds the surface; otherwise the topological
 * reason the last candidate in fan order was rejected.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD CollapseResult<typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle>
remove_vertex(TriangleHalfedgeMesh<T, D, TIndex>& mesh, typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle vertex)
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using VertexHandle = typename Mesh::VertexHandle;
  using vec_t = typename Mesh::vec_t;

  if (!mesh.contains(vertex) || mesh.is_deleted(vertex))
  {
    return {VertexHandle{}, CollapseStatus::InvalidHandle};
  }
  if (is_isolated(mesh, vertex))
  {
    mesh.connectivity().mark_deleted(vertex);
    return {VertexHandle{}, CollapseStatus::Ok};
  }

  HalfedgeHandle shortest{};
  T shortestLengthSquared{};
  CollapseStatus lastTopologicalRejection = CollapseStatus::Ok;
  bool foldsOver = false;
  const Mesh& constMesh = mesh;
  for (auto outgoing = constMesh.outgoing_halfedges(vertex).circulator(); outgoing.is_valid(); ++outgoing)
  {
    const HalfedgeHandle candidate = outgoing.get_halfedgehandle();
    const CollapseStatus status = is_collapse_ok(mesh, candidate);
    if (status != CollapseStatus::Ok)
    {
      lastTopologicalRejection = status;
      continue;
    }
    const vec_t& targetPosition = mesh.get_position(mesh.target_vertex(candidate));
    if (collapse_inverts_faces(mesh, candidate, targetPosition))
    {
      foldsOver = true;
      continue;
    }
    const T lengthSquared = linal::length_squared(vec_t{targetPosition - mesh.get_position(vertex)});
    if (!shortest.is_valid() || lengthSquared < shortestLengthSquared)
    {
      shortest = candidate;
      shortestLengthSquared = lengthSquared;
    }
  }

  if (!shortest.is_valid())
  {
    return {VertexHandle{}, foldsOver ? CollapseStatus::InvertsFaces : lastTopologicalRejection};
  }
  const VertexHandle survivor = mesh.target_vertex(shortest);
  detail::collapse_halfedge_unchecked(mesh, shortest);
  return {survivor, CollapseStatus::Ok};
}

} // namespace Geometry

#endif // GEOMETRY_MESH_MESHVERTEXREMOVAL_HPP
