#ifndef GEOMETRY_MESH_MESHFACEGEOMETRY_HPP
#define GEOMETRY_MESH_MESHFACEGEOMETRY_HPP

#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Utils/Compiler.hpp"

#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>

namespace Geometry
{
namespace detail
{

/**
 * \internal
 * \brief Un-normalized area vector of a triangular face -- the cross product of two edges,
 * whose length equals twice the triangle area.
 *
 * The single geometric primitive both \c mesh_face_normal and \c mesh_face_is_degenerate build on,
 * so neither has to re-derive the other: degeneracy is "this vector is zero", the flat normal is
 * "this vector, normalized". Sharing it keeps one definition of the face's geometry.
 */
template <typename T, typename TIndex>
GEO_NODISCARD linal::vec3<T> mesh_face_area_vector(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh,
                                                   typename TriangleHalfedgeMesh<T, 3, TIndex>::FaceHandle face) noexcept
{
  const auto handles = mesh.vertices_around_face(face);
  const linal::vec3<T> first{mesh.get_vertex(handles[1]).position - mesh.get_vertex(handles[0]).position};
  const linal::vec3<T> second{mesh.get_vertex(handles[2]).position - mesh.get_vertex(handles[0]).position};
  return linal::vec3<T>{linal::cross(first, second)};
}

/**
 * \internal
 * \brief Whether a triangular face is geometrically degenerate -- zero area / collinear vertices.
 *
 * Expressed directly as "the area vector vanishes" so the flatness test reads as the area check it
 * is, without computing or dividing by a normal.
 */
template <typename T, typename TIndex>
GEO_NODISCARD bool mesh_face_is_degenerate(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh,
                                           typename TriangleHalfedgeMesh<T, 3, TIndex>::FaceHandle face) noexcept
{
  return linal::length(mesh_face_area_vector(mesh, face)) == T{0};
}

/**
 * \internal
 * \brief Normalized geometric (flat) normal of a triangular face, written to \p normal.
 *
 * \return \c false with \p normal left unset when the triangle is degenerate (zero area); the
 * caller reports this as \c DegenerateGeometry.
 */
template <typename T, typename TIndex>
GEO_NODISCARD bool mesh_face_normal(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh,
                                    typename TriangleHalfedgeMesh<T, 3, TIndex>::FaceHandle face,
                                    linal::vec3<T>& normal) noexcept
{
  const linal::vec3<T> areaVector = mesh_face_area_vector(mesh, face);
  const T length = linal::length(areaVector);
  if (length == T{0})
    return false;
  normal = linal::vec3<T>{areaVector / length};
  return true;
}

} // namespace detail
} // namespace Geometry

#endif // GEOMETRY_MESH_MESHFACEGEOMETRY_HPP
