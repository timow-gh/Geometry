#ifndef GEOMETRY_MESH_DETAIL_FACEGEOMETRY_HPP
#define GEOMETRY_MESH_DETAIL_FACEGEOMETRY_HPP

#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include "Geometry/detail/TriangleOrientation.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>
#include <numbers>
#include <optional>
#include <type_traits>

namespace Geometry
{
namespace detail
{

/**
 * \internal
 * \brief Default largest angle between the normals of two triangles sharing an edge, in radians.
 *
 * 179 degrees: only a triangle turned back almost completely onto its neighbour counts as folded, so
 * the limit never interferes with creases or other sharp features a surface legitimately has, yet a
 * fold still gets caught before it is exact. Callers wanting a smoother result pass a smaller angle.
 */
template <typename T>
GEO_NODISCARD constexpr T default_max_fold_angle() noexcept
{
  return std::numbers::pi_v<T> * T{179} / T{180};
}

/**
 * \internal
 * \brief Default largest corner angle of a triangle, in radians.
 *
 * 179 degrees: only triangles whose corners almost line up are rejected. Their normals are dominated
 * by rounding and can point anywhere, so they show up as stray faces standing across the surface.
 * Thinner-but-sound slivers stay allowed; how much triangle quality to demand is the caller's choice.
 */
template <typename T>
GEO_NODISCARD constexpr T default_max_corner_angle() noexcept
{
  return std::numbers::pi_v<T> * T{179} / T{180};
}

/**
 * \internal
 * \brief An upper bound on the angle between two vectors, tested without \c sqrt or \c acos.
 *
 * The angle exceeds the limit iff its cosine falls below cos(limit). Both sides are compared as
 * signed squares, x * |x|, which preserve order, so the test needs only the inner product and the
 * product of squared lengths -- no normalisation, and no loss of the sign that tells an acute from an
 * obtuse angle. Works for any limit in [0, pi].
 */
template <typename T>
class AngleLimit
{
public:
  explicit AngleLimit(T maxAngle) noexcept
      : m_signedSquaredCosine{signed_square(std::cos(maxAngle))}
  {
    GEO_ASSERT(maxAngle >= T{0} && maxAngle <= std::numbers::pi_v<T>);
  }

  /**
   * \brief Whether the angle between two vectors exceeds the limit.
   *
   * \param dot Inner product of the two vectors.
   * \param squaredLengthProduct Product of their squared lengths; must be positive.
   */
  GEO_NODISCARD bool is_exceeded(T dot, T squaredLengthProduct) const noexcept
  {
    GEO_ASSERT(squaredLengthProduct > T{0});
    return signed_square(dot) < m_signedSquaredCosine * squaredLengthProduct;
  }

private:
  GEO_NODISCARD static T signed_square(T value) noexcept { return value * std::abs(value); }

  T m_signedSquaredCosine;
};

/**
 * \internal
 * \brief Whether two triangles sharing an edge, given by their \c triangle_orientation, fold onto each
 * other: their normals are further apart than \p limit.
 *
 * A degenerate triangle has no normal to compare, so it never counts as folded here; degeneracy is
 * reported by the dedicated checks. In 2D the orientations are signed areas and the angle is 0 or
 * 180 degrees, so this reduces to "wound opposite ways".
 */
template <typename TOrientation, typename T>
GEO_NODISCARD bool triangles_fold(const TOrientation& orientation, const TOrientation& neighbourOrientation,
                                  const AngleLimit<T>& limit) noexcept
{
  const T squaredLengthProduct = orientation_dot(orientation, orientation) * orientation_dot(neighbourOrientation, neighbourOrientation);
  if (squaredLengthProduct == T{0})
  {
    return false;
  }
  return limit.is_exceeded(orientation_dot(orientation, neighbourOrientation), squaredLengthProduct);
}

/**
 * \internal
 * \brief Whether two triangles differ so much in area that comparing their normals is unreliable.
 *
 * A tiny triangle's normal is dominated by rounding error, so its angle to a large neighbour means
 * little. Operators that must not fold the surface treat such a pair as folded -- rejecting one
 * operation is cheap, trusting a meaningless normal is not. Compares squared doubled areas, so the
 * bound of 1e8 is an area ratio of 1e4.
 */
template <typename TOrientation>
GEO_NODISCARD bool areas_too_disproportionate(const TOrientation& orientation, const TOrientation& neighbourOrientation) noexcept
{
  using T = decltype(orientation_dot(orientation, orientation));
  constexpr auto maxSquaredAreaRatio = static_cast<T>(1e8);

  const T squaredArea = orientation_dot(orientation, orientation);
  const T neighbourSquaredArea = orientation_dot(neighbourOrientation, neighbourOrientation);
  return std::max(squaredArea, neighbourSquaredArea) >= maxSquaredAreaRatio * std::min(squaredArea, neighbourSquaredArea);
}

/**
 * \internal
 * \brief Whether some corner angle of the triangle (first, apex, last) exceeds \p limit.
 *
 * Catches slivers whose corners almost line up. Two coincident corners make an angle undefined; such
 * a triangle is degenerate and counts as exceeding. Works in 2D and 3D.
 */
template <typename T, std::uint8_t D>
GEO_NODISCARD bool has_corner_wider_than(const linal::vec<T, D>& first, const linal::vec<T, D>& apex,
                                         const linal::vec<T, D>& last, const AngleLimit<T>& limit) noexcept
{
  using Vec = linal::vec<T, D>;

  const std::array<Vec, 3> corners{first, apex, last};
  for (std::size_t i = 0; i < 3; ++i)
  {
    const Vec toNext{corners[(i + 1) % 3] - corners[i]};
    const Vec toPrevious{corners[(i + 2) % 3] - corners[i]};
    const T squaredLengthProduct = linal::length_squared(toNext) * linal::length_squared(toPrevious);
    if (squaredLengthProduct == T{0} || limit.is_exceeded(linal::dot(toNext, toPrevious), squaredLengthProduct))
    {
      return true;
    }
  }
  return false;
}

/**
 * \internal
 * \brief \c triangle_orientation of a mesh face at its current vertex positions, in 2D or 3D.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD auto face_orientation(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                    typename TriangleHalfedgeMesh<T, D, TIndex>::FaceHandle face) noexcept
{
  const auto corners = mesh.vertices_around_face(face);
  return triangle_orientation(mesh.get_position(corners[0]), mesh.get_position(corners[1]), mesh.get_position(corners[2]));
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

#endif // GEOMETRY_MESH_DETAIL_FACEGEOMETRY_HPP
