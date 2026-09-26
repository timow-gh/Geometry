#ifndef GEOMETRY_MESH_MESHTOPOLOGY_HPP
#define GEOMETRY_MESH_MESHTOPOLOGY_HPP

#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <cstddef>
#include <cstdint>

namespace Geometry
{

/**
 * \brief Whether \p vertex has no incident halfedge, i.e. it belongs to no edge or face.
 *
 * A per-element topological query, distinct from \c TriangleHalfedgeMesh::has_valid_connectivity(),
 * which checks internal reference consistency rather than topology.
 *
 * \return \c true if the vertex is isolated.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD bool is_isolated(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                               typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle vertex) noexcept
{
  return !mesh.get_vertex(vertex).halfedge.is_valid();
}

/**
 * \brief Whether \p vertex lies on the boundary -- one of its incident halfedges is a boundary
 * halfedge (has no incident face).
 *
 * Isolated vertices are not boundary vertices. O(1): by the mesh's boundary convention a boundary
 * vertex stores a boundary outgoing halfedge, so no fan walk is needed.
 *
 * \return \c true if the vertex is on the boundary.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD bool is_boundary(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                               typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle vertex)
{
  return mesh.is_boundary_outgoing(vertex);
}

/**
 * \brief Valence (degree) of \p vertex: the number of distinct incident edges.
 *
 * Derived from the incident face count rather than a separate edge walk: for a triangle-fan vertex
 * the incident-edge count equals the incident-face count when the vertex is interior (the fan
 * closes), and one more when it is on the boundary (the open fan has an extra bounding edge).
 *
 * \return The valence, or 0 for an isolated vertex.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD std::size_t valence(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                  typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle vertex)
{
  if (is_isolated(mesh, vertex))
  {
    return 0; // no incident edges
  }
  const std::size_t incidentFaceCount = mesh.count_incident_faces(vertex);
  // A non-isolated vertex with no incident face is a dangling edge, which is non-manifold and cannot
  // arise through the public mesh API (only the raw connectivity view). Algorithms assume a manifold
  // mesh, so assert rather than return a meaningless valence.
  GEO_ASSERT(incidentFaceCount > 0 && "valence assumes a manifold (non-dangling) vertex");
  return incidentFaceCount + (is_boundary(mesh, vertex) ? 1U : 0U);
}

} // namespace Geometry

#endif // GEOMETRY_MESH_MESHTOPOLOGY_HPP
