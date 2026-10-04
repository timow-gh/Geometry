#ifndef GEOMETRY_DETAIL_IMPLICITPOINT_HPP
#define GEOMETRY_DETAIL_IMPLICITPOINT_HPP

#include "Geometry/Predicates.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include "Geometry/detail/TriangleOrientation.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <linal/vec.hpp>
#include <span>
#include <type_traits>

namespace Geometry
{
namespace detail
{

/** \internal \brief Which input points define an \c ImplicitPoint, and how. */
enum class ImplicitPointKind : std::uint8_t
{
  // An input vertex; the position is exact.
  Explicit,
  // Where the edge (source, target) crosses the plane through (first, second, third).
  EdgePlane,
  // Where the coplanar edges (firstSource, firstTarget) and (secondSource, secondTarget) cross,
  // evaluated in the projection that drops one axis.
  EdgeEdge
};

/**
 * \internal
 * \brief Point that divides the segment (\p source, \p target) in the ratio of the unsigned
 * distances \p sourceDistance : \p targetDistance of its endpoints from a crossing line or plane.
 *
 * Taking magnitudes keeps the parameter in [0, 1] whatever the signs of the floating-point
 * determinants: the exact predicate that established the crossing can disagree with them near a
 * degeneracy. If both vanish for the same reason, the midpoint is as good as any point.
 * O(1).
 */
template <typename T, std::uint8_t D>
GEO_NODISCARD linal::vec<T, D>
point_between(const linal::vec<T, D>& source, const linal::vec<T, D>& target, const T sourceDistance, const T targetDistance) noexcept
{
  using Vec = linal::vec<T, D>;

  const T total = std::abs(sourceDistance) + std::abs(targetDistance);
  const T parameter = total > T{0} ? std::abs(sourceDistance) / total : T{0.5};
  const Vec direction{target - source};
  return Vec{source + Vec{direction * parameter}};
}

/**
 * \internal
 * \brief A point of the mesh Booleans, kept as its definition by input points together with a
 * cached rounded position.
 *
 * <b>The rounding gap.</b> Exact predicates decide correctly for the coordinates they are given,
 * which is enough only while every point they see is an input vertex. Corefinement also creates
 * points: an intersection point P, e.g. where edge (a0, a1) crosses the plane of (b0, b1, b2), is a
 * rational function of input points. P is in general not representable in \p T, so only the rounded
 * P' = round(P) can be stored, off by a few ulps per coordinate. Every later decision involving an
 * intersection point evaluates a predicate on P', not on P:
 * 1. ordering several intersection points along one original edge;
 * 2. locating a point in a partly refined face (\c orient2d against sub-edges whose endpoints are
 *    themselves intersection points);
 * 3. the convexity check before flipping an edge during constraint recovery;
 * 4. classifying a patch at an intersection edge (p, q): \c orient3d(p, q, u, r) with rounded p and q.
 *
 * Whenever the true configuration is near-degenerate (two intersection points very close together on
 * one edge, a point very close to a sub-edge, a nearly coplanar patch), even an exact predicate on P'
 * can return a different sign than the same predicate on P. A flipped sign means:
 * - two points swap order along an edge, so sub-edges overlap and triangles fold over;
 * - a point lands in the wrong sub-triangle or on the wrong side of an intersection segment, so the
 *   refinements of the two meshes disagree about where the intersection curve runs, and constraint
 *   recovery fails or produces crossing edges;
 * - a patch is classified inside instead of outside, so the wrong patch is kept. This produces
 *   exactly the holes and double walls (gaps and slivers) the Booleans must never produce;
 * - rounded points lie slightly off both original planes, so a result can contain tiny
 *   self-intersections, and a later Boolean on that result violates its own precondition.
 *
 * Rounding does not open cracks between the two meshes along the intersection curve: a point is
 * identified by its key (the pair of input simplices that contain it) and gets one rounded position,
 * which both meshes share.
 *
 * <b>Why keeping the definition closes the gap.</b> An indirect predicate never builds P'. It
 * substitutes the exact expression of P in its defining input points into the determinant and
 * evaluates the sign exactly: a floating-point filter first, exact expansion arithmetic only when the
 * filter is inconclusive (Attene, "Indirect predicates for geometric constructions", 2020). Every
 * decision is then exact for the true point P, and rounding happens once, when the final coordinates
 * are written after the topology is fixed. What remains is the unavoidable output rounding: the
 * combinatorially correct result can still contain tiny self-intersections, because it is stored in
 * \p T. Neither the floating-point placeholders nor plain exact predicates can do this, since both
 * only ever see P'.
 *
 * Therefore every predicate that may receive an intersection point takes \c ImplicitPoint arguments
 * (the \c orient2d and \c orient3d overloads below). For now they evaluate on the cached position;
 * once indirect predicates exist, they evaluate the definition instead, without changes at any call
 * site. Call sites of the four decisions above point here rather than repeating this.
 *
 * The defining points are copies of input coordinates, never computed ones, so the definition is
 * exact. A default-constructed point is the explicit origin; it exists so that fixed-capacity
 * containers can hold the type.
 */
template <typename T>
class ImplicitPoint {
  static_assert(std::is_floating_point_v<T>, "implicit points require floating-point coordinates");

public:
  using Vec3 = linal::vec3<T>;

  static constexpr std::size_t max_definition_size = 5;

  constexpr ImplicitPoint() noexcept = default;

  /** \brief An input vertex at \p point. O(1). */
  GEO_NODISCARD static constexpr ImplicitPoint create_explicit(const Vec3& point) noexcept
  {
    return ImplicitPoint{ImplicitPointKind::Explicit, {point}, point, 0};
  }

  /**
   * \brief Where the edge (\p source, \p target) crosses the plane through (\p planeFirst,
   * \p planeSecond, \p planeThird).
   *
   * The cached position interpolates the edge by the magnitudes of the two \c orient3d determinants
   * of its endpoints. Swapping \p source and \p target defines the same point but may round
   * differently, so callers that meet one edge several times pass it in one canonical direction.
   *
   * \pre \p source and \p target lie strictly on opposite sides of the plane. O(1).
   */
  GEO_NODISCARD static ImplicitPoint create_edge_plane(const Vec3& source,
                                                       const Vec3& target,
                                                       const Vec3& planeFirst,
                                                       const Vec3& planeSecond,
                                                       const Vec3& planeThird) noexcept
  {
    GEO_ASSERT(detail::multiply_signs(orient3d(planeFirst, planeSecond, planeThird, source),
                                      orient3d(planeFirst, planeSecond, planeThird, target)) == Orientation::Negative);

    const Vec3 normal = detail::triangle_orientation(planeFirst, planeSecond, planeThird);
    const Vec3 sourceOffset{source - planeFirst};
    const Vec3 targetOffset{target - planeFirst};
    const Vec3 position =
        detail::point_between(source, target, linal::dot(normal, sourceOffset), linal::dot(normal, targetOffset));
    return ImplicitPoint{ImplicitPointKind::EdgePlane, {source, target, planeFirst, planeSecond, planeThird}, position, 0};
  }

  /**
   * \brief Where the coplanar edges (\p firstSource, \p firstTarget) and (\p secondSource,
   * \p secondTarget) cross, both projected by dropping \p axis.
   *
   * The cached position is the crossing of the projected lines, lifted to 3D along the first edge.
   * As for \c create_edge_plane, the edge directions affect the rounding, not the point.
   *
   * \pre The four points are coplanar, the plane is not parallel to \p axis, and the projected edges
   * cross at a point interior to both. O(1).
   */
  GEO_NODISCARD static ImplicitPoint create_edge_edge(const Vec3& firstSource,
                                                      const Vec3& firstTarget,
                                                      const Vec3& secondSource,
                                                      const Vec3& secondTarget,
                                                      const std::uint8_t axis) noexcept
  {
    using Vec2 = linal::vec2<T>;

    const Vec2 projectedFirstSource = detail::project_dropping_axis(firstSource, axis);
    const Vec2 projectedFirstTarget = detail::project_dropping_axis(firstTarget, axis);
    const Vec2 projectedSecondSource = detail::project_dropping_axis(secondSource, axis);
    const Vec2 projectedSecondTarget = detail::project_dropping_axis(secondTarget, axis);
    GEO_ASSERT(detail::multiply_signs(orient2d(projectedSecondSource, projectedSecondTarget, projectedFirstSource),
                                      orient2d(projectedSecondSource, projectedSecondTarget, projectedFirstTarget))
               == Orientation::Negative);
    GEO_ASSERT(detail::multiply_signs(orient2d(projectedFirstSource, projectedFirstTarget, projectedSecondSource),
                                      orient2d(projectedFirstSource, projectedFirstTarget, projectedSecondTarget))
               == Orientation::Negative);

    const T sourceDistance = detail::triangle_orientation(projectedSecondSource, projectedSecondTarget, projectedFirstSource);
    const T targetDistance = detail::triangle_orientation(projectedSecondSource, projectedSecondTarget, projectedFirstTarget);
    const Vec3 position = detail::point_between(firstSource, firstTarget, sourceDistance, targetDistance);
    return ImplicitPoint{ImplicitPointKind::EdgeEdge, {firstSource, firstTarget, secondSource, secondTarget}, position, axis};
  }

  GEO_NODISCARD constexpr ImplicitPointKind kind() const noexcept { return m_kind; }

  /**
   * \brief The rounded position. Use it for output and for inexact work such as bounding boxes;
   * decisions go through the \c ImplicitPoint predicates instead.
   */
  GEO_NODISCARD constexpr const Vec3& position() const noexcept { return m_position; }

  /**
   * \brief The defining input points, in the order of the \c create_* parameters: one for
   * \c Explicit, five for \c EdgePlane, four for \c EdgeEdge.
   */
  GEO_NODISCARD constexpr std::span<const Vec3> definition() const noexcept
  {
    switch (m_kind)
    {
    case ImplicitPointKind::Explicit: return {m_definition.data(), 1};
    case ImplicitPointKind::EdgePlane: return {m_definition.data(), 5};
    case ImplicitPointKind::EdgeEdge: return {m_definition.data(), 4};
    }
    GEO_ASSERT(false);
    return {};
  }

  /** \brief The axis an \c EdgeEdge point's projection drops. \pre kind() == EdgeEdge. */
  GEO_NODISCARD constexpr std::uint8_t axis() const noexcept
  {
    GEO_ASSERT(m_kind == ImplicitPointKind::EdgeEdge);
    return m_axis;
  }

private:
  constexpr ImplicitPoint(const ImplicitPointKind kind,
                          const std::array<Vec3, max_definition_size>& definition,
                          const Vec3& position,
                          const std::uint8_t axis) noexcept
      : m_definition(definition)
      , m_position(position)
      , m_kind(kind)
      , m_axis(axis)
  {
    GEO_ASSERT(axis < 3);
  }

  std::array<Vec3, max_definition_size> m_definition{};
  Vec3 m_position{};
  ImplicitPointKind m_kind{ImplicitPointKind::Explicit};
  // EdgeEdge only: the axis its projection drops.
  std::uint8_t m_axis{0};
};

} // namespace detail

/**
 * \brief \c orient3d for points that may be constructed intersection points.
 *
 * The overload every rounding-sensitive 3D decision of the mesh Booleans calls; see
 * \c detail::ImplicitPoint. For now it evaluates the cached rounded positions, so a near-degenerate
 * configuration can get the sign of the rounded points rather than the true ones. Once indirect
 * predicates exist it evaluates the definitions, without changes at the call sites. With explicit
 * points only, it equals \c orient3d on their coordinates. O(1).
 */
template <typename T>
GEO_NODISCARD Orientation orient3d(const detail::ImplicitPoint<T>& first,
                                   const detail::ImplicitPoint<T>& second,
                                   const detail::ImplicitPoint<T>& third,
                                   const detail::ImplicitPoint<T>& query) noexcept
{
  return orient3d(first.position(), second.position(), third.position(), query.position());
}

/**
 * \brief \c orient2d of the projections that drop \p axis, for points that may be constructed
 * intersection points.
 *
 * The overload every rounding-sensitive 2D decision of the mesh Booleans calls; the same limits as
 * the \c orient3d overload apply. \p axis is chosen per face (\c detail::dominant_axis), so the
 * projection itself is exact. \pre \p axis < 3. O(1).
 */
template <typename T>
GEO_NODISCARD Orientation orient2d(const detail::ImplicitPoint<T>& first,
                                   const detail::ImplicitPoint<T>& second,
                                   const detail::ImplicitPoint<T>& query,
                                   const std::uint8_t axis) noexcept
{
  return orient2d(detail::project_dropping_axis(first.position(), axis),
                  detail::project_dropping_axis(second.position(), axis),
                  detail::project_dropping_axis(query.position(), axis));
}

} // namespace Geometry

#endif // GEOMETRY_DETAIL_IMPLICITPOINT_HPP
