#ifndef GEOMETRY_MESH_MESHQUALITY_HPP
#define GEOMETRY_MESH_MESHQUALITY_HPP

#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Mesh/detail/FaceGeometry.hpp"
#include "Geometry/Utils/Compiler.hpp"

#include <cstdint>

namespace Geometry
{

/**
 * \brief Geometric limits a mesh operation must respect, as angles in radians.
 *
 * Taken by the operations that reshape triangles (edge collapse, vertex removal). The defaults only
 * reject results that are broken rather than merely coarse: a triangle folded back almost completely
 * onto its neighbour, or one whose corners almost line up. Tighten them to trade decimation freedom
 * for surface quality, e.g. a smaller \c maxCornerAngle keeps thin slivers out.
 */
template <typename T>
struct MeshGeometryLimits
{
  // Largest angle between the normals of two triangles sharing an edge after the operation.
  T maxFoldAngle{detail::default_max_fold_angle<T>()};
  // Largest corner angle of any triangle the operation reshapes.
  T maxCornerAngle{detail::default_max_corner_angle<T>()};
};

/**
 * \brief Whether a single face is geometrically degenerate -- zero area / collinear vertices, so it
 * has no well-defined normal or orientation.
 *
 * \return \c true if \p face has zero area.
 */
template <typename T, typename TIndex>
GEO_NODISCARD bool is_degenerate(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh,
                                 typename TriangleHalfedgeMesh<T, 3, TIndex>::FaceHandle face)
{
  return detail::mesh_face_is_degenerate(mesh, face);
}

/**
 * \brief Whether any face of the mesh is degenerate.
 *
 * Run before normal- or orientation-dependent work: a single zero-area triangle leaves those
 * computations meaningless. O(F).
 *
 * \return \c true if at least one face has zero area.
 */
template <typename T, typename TIndex>
GEO_NODISCARD bool has_degenerate_faces(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh)
{
  for (const auto face : mesh.faces())
  {
    if (detail::mesh_face_is_degenerate(mesh, face))
    {
      return true;
    }
  }
  return false;
}

/**
 * \brief Whether the two faces on \p edge fold onto each other: their normals are more than
 * \p maxFoldAngle radians apart.
 *
 * A consistently wound, manifold mesh can still turn back on itself, e.g. after a mesh operation
 * moved a triangle over its neighbour; the topological checks cannot see that. This local test can,
 * for adjacent faces only -- it does not detect parts of the surface that are not adjacent passing
 * through each other. A boundary edge has nothing to fold onto, and a degenerate face has no normal,
 * so both report \c false (see \c has_degenerate_faces). O(1).
 *
 * \return \c true if \p edge is interior and its faces fold.
 */
template <typename T, typename TIndex>
GEO_NODISCARD bool is_folded_edge(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh,
                                  typename TriangleHalfedgeMesh<T, 3, TIndex>::EdgeHandle edge,
                                  T maxFoldAngle = detail::default_max_fold_angle<T>())
{
  const auto halfedge = mesh.get_edge(edge).halfedge;
  const auto face = mesh.get_halfedge(halfedge).face;
  const auto neighbour = mesh.get_halfedge(mesh.get_halfedge(halfedge).twin).face;
  if (!face.is_valid() || !neighbour.is_valid())
  {
    return false;
  }
  return detail::triangles_fold(detail::face_orientation(mesh, face), detail::face_orientation(mesh, neighbour),
                                detail::AngleLimit<T>{maxFoldAngle});
}

/**
 * \brief Whether any edge of the mesh is folded (see \c is_folded_edge).
 *
 * Run after operations that move or reconnect vertices to confirm the surface did not turn back on
 * itself. O(E).
 *
 * \return \c true if at least one edge is folded.
 */
template <typename T, typename TIndex>
GEO_NODISCARD bool has_folded_edges(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh,
                                    T maxFoldAngle = detail::default_max_fold_angle<T>())
{
  for (const auto edge : mesh.edges())
  {
    if (is_folded_edge(mesh, edge, maxFoldAngle))
    {
      return true;
    }
  }
  return false;
}

} // namespace Geometry

#endif // GEOMETRY_MESH_MESHQUALITY_HPP
