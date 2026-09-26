#ifndef GEOMETRY_MESH_MESHFACEGEOMETRY_HPP
#define GEOMETRY_MESH_MESHFACEGEOMETRY_HPP

#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Utils/Compiler.hpp"

#include <cstdint>
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>
#include <optional>
#include <type_traits>

namespace Geometry
{
namespace detail
{

/**
 * \internal
 * \brief Orientation of the triangle (first, apex, last): the area vector (cross product of two edges) in 3D,
 * the signed doubled area in 2D.
 *
 * One primitive for both dimensions, so orientation tests (flip checks, hole triangulation) are
 * written once: two triangles agree in orientation iff \c orientation_dot of theirs is positive, and
 * a triangle is degenerate iff \c orientation_dot with itself is zero.
 */
template <typename T, std::uint8_t D>
GEO_NODISCARD auto triangle_orientation(const linal::vec<T, D>& first, const linal::vec<T, D>& apex,
                                        const linal::vec<T, D>& last) noexcept
{
  static_assert(D == 2 || D == 3, "orientation is defined for planar and spatial triangles only");
  using Vec = linal::vec<T, D>;

  const Vec toApex{apex - first};
  const Vec toLast{last - first};
  if constexpr (D == 3)
  {
    return linal::vec3<T>{linal::cross(toApex, toLast)};
  }
  else
  {
    return toApex[0] * toLast[1] - toApex[1] * toLast[0];
  }
}

/**
 * \internal
 * \brief Inner product of two \c triangle_orientation values: positive when the triangles agree in
 * orientation; with itself, the squared doubled area.
 */
template <typename TOrientation>
GEO_NODISCARD auto orientation_dot(const TOrientation& lhs, const TOrientation& rhs) noexcept
{
  if constexpr (std::is_arithmetic_v<TOrientation>)
  {
    return lhs * rhs;
  }
  else
  {
    return linal::dot(lhs, rhs);
  }
}

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
 * \brief Normalized geometric (flat) normal of a triangular face.
 *
 * \return The unit normal, or \c std::nullopt when the triangle is degenerate (zero area); the
 * caller reports this as \c DegenerateGeometry.
 */
template <typename T, typename TIndex>
GEO_NODISCARD std::optional<linal::vec3<T>> mesh_face_normal(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh,
                                                             typename TriangleHalfedgeMesh<T, 3, TIndex>::FaceHandle face) noexcept
{
  const linal::vec3<T> areaVector = mesh_face_area_vector(mesh, face);
  const T length = linal::length(areaVector);
  if (length == T{0})
    return std::nullopt;
  return linal::vec3<T>{areaVector / length};
}

} // namespace detail
} // namespace Geometry

#endif // GEOMETRY_MESH_MESHFACEGEOMETRY_HPP
