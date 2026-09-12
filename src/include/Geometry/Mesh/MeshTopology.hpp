#ifndef GEOMETRY_MESH_MESHTOPOLOGY_HPP
#define GEOMETRY_MESH_MESHTOPOLOGY_HPP

#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <cstddef>
#include <cstdint>

namespace Geometry
{

// Per-element topological queries on a TriangleHalfedgeMesh. They complement (but do not overlap) 
// the connectivity-consistency check TriangleHalfedgeMesh::has_valid_connectivity(), which validates
// internal reference consistency rather than topology.

// Returns true if the vertex has no incident halfedge (it belongs to no edge or face).
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD bool is_isolated(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                               typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle vertex) noexcept
{
  return !mesh.get_vertex(vertex).halfedge.is_valid();
}

// Returns true if the vertex lies on the boundary, i.e. one of its incident halfedges is a boundary
// halfedge (has no incident face). Isolated vertices are not considered boundary vertices. By the
// mesh's boundary convention a boundary vertex stores a boundary outgoing halfedge, so this is O(1).
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD bool is_boundary(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                               typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle vertex)
{
  return mesh.is_boundary_outgoing(vertex);
}

// Returns the valence (degree) of a vertex: the number of distinct incident edges.
//
// halfedges_around_vertex returns the interior (face-bearing) outgoing halfedges, one per incident
// face. For a triangle-fan vertex the number of incident edges equals the number of incident faces
// when the vertex is interior (the fan closes), and one more than the number of faces when the vertex
// is on the boundary (the open fan has an extra bounding edge).
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD std::size_t valence(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                  typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle vertex)
{
  const std::size_t incidentFaceCount = mesh.count_incident_faces(vertex);
  if (incidentFaceCount == 0)
  {
    return 0;
  }
  return incidentFaceCount + (is_boundary(mesh, vertex) ? 1U : 0U);
}

} // namespace Geometry

#endif // GEOMETRY_MESH_MESHTOPOLOGY_HPP
