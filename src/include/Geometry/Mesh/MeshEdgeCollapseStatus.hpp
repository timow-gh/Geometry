#ifndef GEOMETRY_MESH_MESHEDGECOLLAPSESTATUS_HPP
#define GEOMETRY_MESH_MESHEDGECOLLAPSESTATUS_HPP

#include "Geometry/Mesh/detail/MeshResult.hpp"
#include "Geometry/Utils/Compiler.hpp"

namespace Geometry
{

/**
 * \brief Outcome of a (half)edge collapse validity check; every value but \c Ok names why the
 * collapse would break the mesh topology or geometry.
 */
enum class CollapseStatus
{
  Ok,
  // The halfedge is not in the mesh or already deleted.
  InvalidHandle,
  // An incident face has all three edges on the boundary; collapsing it would leave a dangling edge.
  IsolatedTriangle,
  // Both endpoints are boundary vertices but the edge is interior; the result would pinch the
  // surface into a non-manifold vertex.
  InteriorEdgeBetweenBoundaryVertices,
  // The endpoints share a neighbour that is not opposite the edge; the result would contain a
  // non-manifold edge and a fold-over face.
  LinkCondition,
  // The edge lies on a closed tetrahedron component; the result would be two coincident triangles.
  Tetrahedron,
  // Topologically legal, but the collapse would flip or flatten a face, fold it onto a neighbouring
  // face, or leave a nearly flat sliver (see collapse_inverts_faces, collapse_exceeds_geometry_limits).
  // Never returned by is_collapse_ok or the _topology_only collapses; reported by check_collapse and
  // the collapses that check geometry.
  InvertsFaces,
};

/**
 * \brief Result of an operation that removes one endpoint of an edge: the surviving vertex, or the
 * \c CollapseStatus explaining why the mesh was left untouched.
 */
template <typename TVertexHandle>
struct CollapseResult
{
  TVertexHandle survivor{};
  CollapseStatus status{CollapseStatus::Ok};

  GEO_NODISCARD bool has_value() const noexcept { return detail::mesh_result_ok(status); }
  GEO_NODISCARD explicit operator bool() const noexcept { return has_value(); }
};

} // namespace Geometry

#endif // GEOMETRY_MESH_MESHEDGECOLLAPSESTATUS_HPP
