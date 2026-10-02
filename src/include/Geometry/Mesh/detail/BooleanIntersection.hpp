#ifndef GEOMETRY_MESH_DETAIL_BOOLEANINTERSECTION_HPP
#define GEOMETRY_MESH_DETAIL_BOOLEANINTERSECTION_HPP

#include "Geometry/AABB.hpp"
#include "Geometry/AABBTree.hpp"
#include "Geometry/Mesh/MakeTriangleMesh.hpp"
#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Predicates.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include "Geometry/detail/FixedVector.hpp"
#include "Geometry/detail/ImplicitPoint.hpp"
#include "Geometry/detail/TriangleOrientation.hpp"

#include <algorithm>
#include <array>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <linal/vec.hpp>
#include <optional>
#include <span>
#include <tuple>
#include <utility>
#include <vector>

namespace Geometry
{
namespace detail
{

/** \internal \brief Dimension of a \c MeshSimplex. */
enum class SimplexKind : std::uint8_t
{
  Vertex,
  Edge,
  Face
};

/**
 * \internal
 * \brief A vertex, edge or face of one Boolean operand, named by its handle value in the original,
 * unrefined mesh.
 *
 * Corefinement splits faces and edges but never renames what the input contained, so these names
 * stay valid while both meshes are refined.
 */
template <typename TIndex>
struct MeshSimplex
{
  SimplexKind kind{SimplexKind::Vertex};
  TIndex index{};

  GEO_NODISCARD constexpr auto operator<=>(const MeshSimplex&) const noexcept = default;
};

/**
 * \internal
 * \brief Symbolic identity of an intersection point: the lowest-dimensional simplex of each operand
 * that contains it.
 *
 * Every face pair that finds a point finds the same key for it, whatever its rounded position, so
 * the point becomes one shared vertex of both refined meshes and the intersection curve cannot
 * crack. Ordered lexicographically, so keys can be sorted and searched.
 */
template <typename TIndex>
struct IntersectionKey
{
  MeshSimplex<TIndex> simplexA;
  MeshSimplex<TIndex> simplexB;

  GEO_NODISCARD constexpr auto operator<=>(const IntersectionKey&) const noexcept = default;
};

/** \internal \brief An intersection point: its key and its definition. */
template <typename T, typename TIndex>
struct KeyedPoint
{
  IntersectionKey<TIndex> key;
  ImplicitPoint<T> point;
};

/**
 * \internal
 * \brief Intersection of one face of operand A with one face of operand B: distinct keyed points
 * and the segments between them.
 *
 * Faces in different planes meet in at most one segment. A lone point without a segment is a touch
 * (e.g. a vertex resting on the other face); it is kept, because a contact at a single point decides
 * whether a result is manifold. Coplanar faces overlap in a convex polygon of at most six corners.
 * Its boundary is reported as the edges of each face clipped to the other, which are the constraints
 * both refined faces need: at most six distinct segments.
 *
 * Fixed capacity, so computing it never allocates.
 */
template <typename T, typename TIndex>
struct FaceIntersection
{
  using Key = IntersectionKey<TIndex>;
  // Indices into points; the smaller first.
  using Segment = std::array<std::uint8_t, 2>;

  static constexpr std::size_t max_points = 6;
  static constexpr std::size_t max_segments = 6;

  FixedVector<KeyedPoint<T, TIndex>, max_points> points;
  FixedVector<Segment, max_segments> segments;
  bool coplanar{false};
  // False when predicate results contradicted each other, e.g. more than two points on one line.
  // Exact predicates rule this out, the floating-point placeholders do not; points and segments are
  // then incomplete.
  bool consistent{true};

  /** \brief Index of the point with \p key, or \c points.size() if there is none. O(points). */
  GEO_NODISCARD std::size_t find_point(const Key& key) const noexcept
  {
    const auto found = std::ranges::find(points, key, &KeyedPoint<T, TIndex>::key);
    return static_cast<std::size_t>(std::distance(points.begin(), found));
  }

  /**
   * \brief Adds the point unless its key is already present; marks the result inconsistent instead of
   * exceeding the capacity. O(points).
   */
  void add_point(const Key& key, const ImplicitPoint<T>& point) noexcept
  {
    if (find_point(key) < points.size())
    {
      return;
    }
    if (points.full())
    {
      consistent = false;
      return;
    }
    points.push_back(KeyedPoint<T, TIndex>{key, point});
  }

  /**
   * \brief Adds the segment between two distinct points unless it is already present, in either
   * direction; marks the result inconsistent instead of exceeding the capacity. O(segments).
   */
  void add_segment(const std::size_t first, const std::size_t second) noexcept
  {
    GEO_ASSERT(first != second);
    GEO_ASSERT(first < points.size() && second < points.size());
    const Segment segment{static_cast<std::uint8_t>(std::min(first, second)), static_cast<std::uint8_t>(std::max(first, second))};
    if (std::ranges::find(segments, segment) != segments.end())
    {
      return;
    }
    if (segments.full())
    {
      consistent = false;
      return;
    }
    segments.push_back(segment);
  }
};

/** \internal \brief A vertex (corner), edge or the face of one triangle, by local index. */
struct LocalSimplex
{
  SimplexKind kind{SimplexKind::Face};
  // Corner for a vertex, edge index for an edge, 0 for the face.
  std::uint8_t index{0};
};

/**
 * \internal
 * \brief Corners of one operand face together with the handle values of its vertices and edges.
 *
 * Edge j joins corner j and corner (j + 1) % 3, as the face's halfedges do.
 */
template <typename T, typename TIndex>
struct FaceSimplices
{
  using Vec3 = linal::vec3<T>;

  std::array<Vec3, 3> corners{};
  std::array<TIndex, 3> vertices{};
  std::array<TIndex, 3> edges{};
  // Whether edge j's stored halfedge runs from corner (j + 1) % 3 to corner j.
  std::array<bool, 3> edgeAgainstFace{};
  TIndex face{};

  GEO_NODISCARD constexpr MeshSimplex<TIndex> simplex(const LocalSimplex local) const noexcept
  {
    GEO_ASSERT(local.index < 3);
    switch (local.kind)
    {
    case SimplexKind::Vertex: return MeshSimplex<TIndex>{SimplexKind::Vertex, vertices[local.index]};
    case SimplexKind::Edge: return MeshSimplex<TIndex>{SimplexKind::Edge, edges[local.index]};
    case SimplexKind::Face: return MeshSimplex<TIndex>{SimplexKind::Face, face};
    }
    GEO_ASSERT(false);
    return {};
  }

  /**
   * \brief Endpoints of edge \p edge in the direction of its stored halfedge.
   *
   * Both faces of the edge see the same direction, so a point defined on the edge rounds the same
   * way whichever face pair finds it.
   */
  GEO_NODISCARD constexpr std::array<Vec3, 2> canonical_edge(const std::uint8_t edge) const noexcept
  {
    GEO_ASSERT(edge < 3);
    const auto next = static_cast<std::uint8_t>((edge + 1) % 3);
    if (edgeAgainstFace[edge])
    {
      return {corners[next], corners[edge]};
    }
    return {corners[edge], corners[next]};
  }
};

/** \internal \brief Corners and simplex handles of \p face. \pre \p face is live. O(1). */
template <typename T, typename TIndex>
GEO_NODISCARD FaceSimplices<T, TIndex> make_face_simplices(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh,
                                                           const typename TriangleHalfedgeMesh<T, 3, TIndex>::FaceHandle face) noexcept
{
  using Mesh = TriangleHalfedgeMesh<T, 3, TIndex>;
  using VertexHandle = typename Mesh::VertexHandle;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using EdgeHandle = typename Mesh::EdgeHandle;

  GEO_ASSERT(mesh.is_live(face));
  FaceSimplices<T, TIndex> simplices;
  simplices.face = face.get_value();
  const std::array<HalfedgeHandle, 3> halfedges = mesh.halfedges_around_face(face);
  for (std::size_t j = 0; j < 3; ++j)
  {
    const VertexHandle vertex = mesh.source_vertex(halfedges[j]);
    const EdgeHandle edge = mesh.get_halfedge(halfedges[j]).edge;
    simplices.corners[j] = mesh.get_position(vertex);
    simplices.vertices[j] = vertex.get_value();
    simplices.edges[j] = edge.get_value();
    simplices.edgeAgainstFace[j] = mesh.get_edge(edge).halfedge != halfedges[j];
  }
  return simplices;
}

/**
 * \internal
 * \brief The simplex of a triangle that contains a point, from the point's side of each edge line;
 * \c std::nullopt if the point is outside.
 *
 * \p sides[j] is \c Positive when the point is strictly on the triangle's side of edge j, \c Zero
 * when it is on the edge's line. One \c Zero means the edge, two mean the corner both edges share.
 *
 * \pre Not all three sides are \c Zero: no point is on all three lines of a non-degenerate triangle.
 * O(1).
 */
GEO_NODISCARD constexpr std::optional<LocalSimplex> locate_by_sides(const std::array<Orientation, 3>& sides) noexcept
{
  if (std::ranges::find(sides, Orientation::Negative) != sides.end())
  {
    return std::nullopt;
  }
  const auto zeroCount = std::ranges::count(sides, Orientation::Zero);
  GEO_ASSERT(zeroCount < 3);
  if (zeroCount == 0)
  {
    return LocalSimplex{SimplexKind::Face, 0};
  }
  if (zeroCount == 1)
  {
    const auto edge = std::distance(sides.begin(), std::ranges::find(sides, Orientation::Zero));
    return LocalSimplex{SimplexKind::Edge, static_cast<std::uint8_t>(edge)};
  }
  // Edges j and j + 1 share corner j + 1, so the corner follows the one edge the point is off by two.
  const auto offEdge = std::distance(sides.begin(), std::ranges::find(sides, Orientation::Positive));
  return LocalSimplex{SimplexKind::Vertex, static_cast<std::uint8_t>((offEdge + 2) % 3)};
}

/**
 * \internal
 * \brief A triangle projected by dropping one axis, with the orientation of the projection, so that
 * side tests can be normalized to "Positive = inside" however the projection turned.
 */
template <typename T>
struct ProjectedTriangle
{
  std::array<linal::vec2<T>, 3> corners{};
  std::uint8_t axis{0};
  // Zero when the triangle's plane is parallel to the dropped axis.
  Orientation orientation{Orientation::Zero};
};

/** \internal \brief \p corners projected by dropping \p axis. O(1). */
template <typename T>
GEO_NODISCARD ProjectedTriangle<T> make_projected_triangle(const std::array<linal::vec3<T>, 3>& corners, const std::uint8_t axis) noexcept
{
  ProjectedTriangle<T> projected;
  projected.axis = axis;
  for (std::size_t i = 0; i < 3; ++i)
  {
    projected.corners[i] = detail::project_dropping_axis(corners[i], axis);
  }
  projected.orientation = orient2d(projected.corners[0], projected.corners[1], projected.corners[2]);
  return projected;
}

/**
 * \internal
 * \brief The simplex of \p triangle that contains \p point, which lies in the triangle's plane;
 * \c std::nullopt if it is outside.
 *
 * \pre \p point lies in the triangle's plane, and the projection is not degenerate. O(1).
 */
template <typename T>
GEO_NODISCARD std::optional<LocalSimplex> locate_in_triangle(const ProjectedTriangle<T>& triangle, const linal::vec3<T>& point) noexcept
{
  using Vec2 = linal::vec2<T>;

  GEO_ASSERT(triangle.orientation != Orientation::Zero);
  const Vec2 projected = detail::project_dropping_axis(point, triangle.axis);
  std::array<Orientation, 3> sides{};
  for (std::size_t j = 0; j < 3; ++j)
  {
    const Orientation side = orient2d(triangle.corners[j], triangle.corners[(j + 1) % 3], projected);
    sides[j] = detail::multiply_signs(side, triangle.orientation);
  }
  return detail::locate_by_sides(sides);
}

/**
 * \internal
 * \brief The simplex of the triangle \p corners where the edge (\p source, \p target) crosses its
 * plane; \c std::nullopt if the crossing is outside the triangle.
 *
 * Uses \c orient3d of the edge against each triangle edge rather than a constructed crossing point,
 * so the answer is exact once the predicate is. When \p source is below the plane, the crossing is on
 * the inner side of triangle edge j iff \c orient3d(source, target, corner j, corner j + 1) is
 * \c Positive; \p targetSide flips that for an edge running downwards.
 *
 * \pre \p source and \p target lie strictly on opposite sides of the plane; \p targetSide is the side
 * of \p target. O(1).
 */
template <typename T>
GEO_NODISCARD std::optional<LocalSimplex> locate_edge_crossing(const linal::vec3<T>& source,
                                                               const linal::vec3<T>& target,
                                                               const Orientation targetSide,
                                                               const std::array<linal::vec3<T>, 3>& corners) noexcept
{
  GEO_ASSERT(targetSide != Orientation::Zero);
  std::array<Orientation, 3> sides{};
  for (std::size_t j = 0; j < 3; ++j)
  {
    sides[j] = detail::multiply_signs(orient3d(source, target, corners[j], corners[(j + 1) % 3]), targetSide);
  }
  return detail::locate_by_sides(sides);
}

/** \internal \brief Side of the plane through \p planeCorners for each of \p corners. O(1). */
template <typename T>
GEO_NODISCARD std::array<Orientation, 3> sides_of_plane(const std::array<linal::vec3<T>, 3>& planeCorners,
                                                        const std::array<linal::vec3<T>, 3>& corners) noexcept
{
  std::array<Orientation, 3> sides{};
  for (std::size_t i = 0; i < 3; ++i)
  {
    sides[i] = orient3d(planeCorners[0], planeCorners[1], planeCorners[2], corners[i]);
  }
  return sides;
}

/** \internal \brief Whether all three sides are \c Positive or all are \c Negative. O(1). */
GEO_NODISCARD constexpr bool is_strictly_one_side(const std::array<Orientation, 3>& sides) noexcept
{
  return sides[0] != Orientation::Zero && sides[0] == sides[1] && sides[1] == sides[2];
}

/** \internal \brief Whether all three sides are \c Zero. O(1). */
GEO_NODISCARD constexpr bool is_all_zero(const std::array<Orientation, 3>& sides) noexcept
{
  return std::ranges::all_of(sides, [](const Orientation side) { return side == Orientation::Zero; });
}

/**
 * \internal
 * \brief A point where one face (own) meets another face's plane inside that face, named from own's
 * side: \p own is a simplex of own, \p other one of the other face.
 */
template <typename T, typename TIndex>
struct PlaneContact
{
  MeshSimplex<TIndex> own;
  MeshSimplex<TIndex> other;
  ImplicitPoint<T> point;
};

/**
 * \internal
 * \brief The points where face \p own meets face \p other, found from own's side: own's vertices on
 * other's plane, and own's edges strictly crossing it, each located in \p other.
 *
 * Every vertex and every edge yields at most one contact, so there are at most six. A point on an
 * edge of both faces is found from both sides under the same key.
 *
 * \param ownSides Side of other's plane for each corner of own.
 * \pre The faces are not coplanar, and \p other is not degenerate. O(1).
 */
template <typename T, typename TIndex>
GEO_NODISCARD FixedVector<PlaneContact<T, TIndex>, 6> plane_contacts(const FaceSimplices<T, TIndex>& own,
                                                                     const std::array<Orientation, 3>& ownSides,
                                                                     const FaceSimplices<T, TIndex>& other) noexcept
{
  using Vec3 = linal::vec3<T>;
  using Contact = PlaneContact<T, TIndex>;
  using Point = ImplicitPoint<T>;

  const std::uint8_t otherAxis = detail::dominant_axis(detail::triangle_orientation(other.corners[0], other.corners[1], other.corners[2]));
  const ProjectedTriangle<T> otherProjected = detail::make_projected_triangle(other.corners, otherAxis);

  FixedVector<Contact, 6> contacts;
  for (std::uint8_t corner = 0; corner < 3; ++corner)
  {
    if (ownSides[corner] != Orientation::Zero)
    {
      continue;
    }
    if (const std::optional<LocalSimplex> located = detail::locate_in_triangle(otherProjected, own.corners[corner]))
    {
      const MeshSimplex<TIndex> vertex = own.simplex(LocalSimplex{SimplexKind::Vertex, corner});
      contacts.push_back(Contact{vertex, other.simplex(*located), Point::create_explicit(own.corners[corner])});
    }
  }
  for (std::uint8_t edge = 0; edge < 3; ++edge)
  {
    const auto next = static_cast<std::uint8_t>((edge + 1) % 3);
    if (detail::multiply_signs(ownSides[edge], ownSides[next]) != Orientation::Negative)
    {
      continue;
    }
    const std::optional<LocalSimplex> located =
        detail::locate_edge_crossing(own.corners[edge], own.corners[next], ownSides[next], other.corners);
    if (!located)
    {
      continue;
    }
    // A crossing at a corner of other is that input vertex; anywhere else it is defined by the edge
    // and the plane.
    const std::array<Vec3, 2> endpoints = own.canonical_edge(edge);
    const Point point = located->kind == SimplexKind::Vertex
                            ? Point::create_explicit(other.corners[located->index])
                            : Point::create_edge_plane(endpoints[0], endpoints[1], other.corners[0], other.corners[1], other.corners[2]);
    contacts.push_back(Contact{own.simplex(LocalSimplex{SimplexKind::Edge, edge}), other.simplex(*located), point});
  }
  return contacts;
}

/**
 * \internal
 * \brief Intersection of two faces in different planes: at most one segment, or a single touch point.
 *
 * Every point found lies on the line where the planes meet and is an endpoint of one face's
 * stretch of that line, so with exact predicates at most two distinct points exist.
 *
 * \param sidesOfA Side of B's plane for each corner of A; \p sidesOfB likewise. O(1).
 */
template <typename T, typename TIndex>
GEO_NODISCARD FaceIntersection<T, TIndex> intersect_crossing_faces(const FaceSimplices<T, TIndex>& faceA,
                                                                   const std::array<Orientation, 3>& sidesOfA,
                                                                   const FaceSimplices<T, TIndex>& faceB,
                                                                   const std::array<Orientation, 3>& sidesOfB) noexcept
{
  using Key = IntersectionKey<TIndex>;

  FaceIntersection<T, TIndex> intersection;
  for (const PlaneContact<T, TIndex>& contact : detail::plane_contacts(faceA, sidesOfA, faceB))
  {
    intersection.add_point(Key{contact.own, contact.other}, contact.point);
  }
  for (const PlaneContact<T, TIndex>& contact : detail::plane_contacts(faceB, sidesOfB, faceA))
  {
    intersection.add_point(Key{contact.other, contact.own}, contact.point);
  }
  if (intersection.points.size() > 2)
  {
    intersection.consistent = false;
  }
  else if (intersection.points.size() == 2)
  {
    intersection.add_segment(0, 1);
  }
  return intersection;
}

/**
 * \internal
 * \brief Whether the projected segments (\p first, \p second) and (\p third, \p fourth) cross at a
 * point interior to both. Touching and collinear overlaps are not crossings. O(1).
 */
template <typename T>
GEO_NODISCARD bool is_proper_crossing(const linal::vec2<T>& first,
                                      const linal::vec2<T>& second,
                                      const linal::vec2<T>& third,
                                      const linal::vec2<T>& fourth) noexcept
{
  return detail::multiply_signs(orient2d(first, second, third), orient2d(first, second, fourth)) == Orientation::Negative
         && detail::multiply_signs(orient2d(third, fourth, first), orient2d(third, fourth, second)) == Orientation::Negative;
}

/**
 * \internal
 * \brief Indices of the points of \p intersection that lie on edge \p edge of \p face: its two
 * corners and its interior. \p onA selects which simplex of each key names \p face's simplices.
 * O(points).
 */
template <typename T, typename TIndex>
GEO_NODISCARD FixedVector<std::size_t, FaceIntersection<T, TIndex>::max_points>
points_on_edge(const FaceIntersection<T, TIndex>& intersection, const FaceSimplices<T, TIndex>& face, const bool onA, const std::uint8_t edge) noexcept
{
  const std::array<MeshSimplex<TIndex>, 3> edgeSimplices{face.simplex(LocalSimplex{SimplexKind::Vertex, edge}),
                                                         face.simplex(LocalSimplex{SimplexKind::Vertex, static_cast<std::uint8_t>((edge + 1) % 3)}),
                                                         face.simplex(LocalSimplex{SimplexKind::Edge, edge})};
  FixedVector<std::size_t, FaceIntersection<T, TIndex>::max_points> onEdge;
  for (std::size_t i = 0; i < intersection.points.size(); ++i)
  {
    const IntersectionKey<TIndex>& key = intersection.points[i].key;
    if (std::ranges::find(edgeSimplices, onA ? key.simplexA : key.simplexB) != edgeSimplices.end())
    {
      onEdge.push_back(i);
    }
  }
  return onEdge;
}

/**
 * \internal
 * \brief Intersection of two coplanar faces: the corners of their overlap polygon, and its boundary
 * as each face's edges clipped to the other.
 *
 * Both faces are projected by dropping the dominant axis of A's normal. The polygon's corners are
 * the corners of either face inside the other and the proper crossings of their edges; with exact
 * predicates every corner gets exactly one key. An edge clipped to the other face is the stretch
 * between the (at most two) corners on it.
 *
 * \pre The faces are coplanar and not degenerate. O(1).
 */
template <typename T, typename TIndex>
GEO_NODISCARD FaceIntersection<T, TIndex> intersect_coplanar_faces(const FaceSimplices<T, TIndex>& faceA,
                                                                   const FaceSimplices<T, TIndex>& faceB) noexcept
{
  using Vec3 = linal::vec3<T>;
  using Key = IntersectionKey<TIndex>;
  using Point = ImplicitPoint<T>;
  using Face = FaceSimplices<T, TIndex>;

  FaceIntersection<T, TIndex> intersection;
  intersection.coplanar = true;
  const std::uint8_t axis = detail::dominant_axis(detail::triangle_orientation(faceA.corners[0], faceA.corners[1], faceA.corners[2]));
  const ProjectedTriangle<T> projectedA = detail::make_projected_triangle(faceA.corners, axis);
  const ProjectedTriangle<T> projectedB = detail::make_projected_triangle(faceB.corners, axis);
  GEO_ASSERT(projectedA.orientation != Orientation::Zero);
  // B is coplanar with A, so its projection along A's dominant axis can only be degenerate if
  // inexact predicates called nearly coplanar faces coplanar.
  if (projectedB.orientation == Orientation::Zero)
  {
    intersection.consistent = false;
    return intersection;
  }

  for (std::uint8_t corner = 0; corner < 3; ++corner)
  {
    if (const std::optional<LocalSimplex> located = detail::locate_in_triangle(projectedB, faceA.corners[corner]))
    {
      const Key key{faceA.simplex(LocalSimplex{SimplexKind::Vertex, corner}), faceB.simplex(*located)};
      intersection.add_point(key, Point::create_explicit(faceA.corners[corner]));
    }
  }
  for (std::uint8_t corner = 0; corner < 3; ++corner)
  {
    if (const std::optional<LocalSimplex> located = detail::locate_in_triangle(projectedA, faceB.corners[corner]))
    {
      const Key key{faceA.simplex(*located), faceB.simplex(LocalSimplex{SimplexKind::Vertex, corner})};
      intersection.add_point(key, Point::create_explicit(faceB.corners[corner]));
    }
  }
  for (std::uint8_t edgeA = 0; edgeA < 3; ++edgeA)
  {
    for (std::uint8_t edgeB = 0; edgeB < 3; ++edgeB)
    {
      if (!detail::is_proper_crossing(projectedA.corners[edgeA],
                                      projectedA.corners[(edgeA + 1) % 3],
                                      projectedB.corners[edgeB],
                                      projectedB.corners[(edgeB + 1) % 3]))
      {
        continue;
      }
      const std::array<Vec3, 2> endpointsA = faceA.canonical_edge(edgeA);
      const std::array<Vec3, 2> endpointsB = faceB.canonical_edge(edgeB);
      const Key key{faceA.simplex(LocalSimplex{SimplexKind::Edge, edgeA}), faceB.simplex(LocalSimplex{SimplexKind::Edge, edgeB})};
      intersection.add_point(key, Point::create_edge_edge(endpointsA[0], endpointsA[1], endpointsB[0], endpointsB[1], axis));
    }
  }

  const auto addClippedEdges = [&intersection](const Face& face, const bool onA) {
    for (std::uint8_t edge = 0; edge < 3; ++edge)
    {
      const auto onEdge = detail::points_on_edge(intersection, face, onA, edge);
      if (onEdge.size() > 2)
      {
        intersection.consistent = false;
      }
      else if (onEdge.size() == 2)
      {
        intersection.add_segment(onEdge[0], onEdge[1]);
      }
    }
  };
  addClippedEdges(faceA, true);
  addClippedEdges(faceB, false);
  return intersection;
}

/**
 * \internal
 * \brief Intersection of face \p faceA of \p meshA with face \p faceB of \p meshB, with every point
 * named by its \c IntersectionKey.
 *
 * Each face's corners are tested against the other's plane with \c orient3d: all strictly on one side
 * means disjoint, all on the plane means coplanar (solved in 2D with \c orient2d), anything else
 * means the faces cross or touch. Every decision here (which side, which simplex) is taken on input
 * points only, never on a constructed point, so it is exact once the predicates are; only the cached
 * positions of the returned points are rounded.
 *
 * With inexact predicates, A on B's plane and B on A's plane can disagree; either counts as coplanar.
 * Does not allocate. O(1).
 *
 * \pre Both faces are live and have non-zero area.
 */
template <typename T, typename TIndex>
GEO_NODISCARD FaceIntersection<T, TIndex> intersect_faces(const TriangleHalfedgeMesh<T, 3, TIndex>& meshA,
                                                          const typename TriangleHalfedgeMesh<T, 3, TIndex>::FaceHandle faceA,
                                                          const TriangleHalfedgeMesh<T, 3, TIndex>& meshB,
                                                          const typename TriangleHalfedgeMesh<T, 3, TIndex>::FaceHandle faceB) noexcept
{
  using Face = FaceSimplices<T, TIndex>;

  const Face simplicesA = detail::make_face_simplices(meshA, faceA);
  const Face simplicesB = detail::make_face_simplices(meshB, faceB);
  const std::array<Orientation, 3> sidesOfA = detail::sides_of_plane(simplicesB.corners, simplicesA.corners);
  const std::array<Orientation, 3> sidesOfB = detail::sides_of_plane(simplicesA.corners, simplicesB.corners);
  if (detail::is_strictly_one_side(sidesOfA) || detail::is_strictly_one_side(sidesOfB))
  {
    return {};
  }
  if (detail::is_all_zero(sidesOfA) || detail::is_all_zero(sidesOfB))
  {
    return detail::intersect_coplanar_faces(simplicesA, simplicesB);
  }
  return detail::intersect_crossing_faces(simplicesA, sidesOfA, simplicesB, sidesOfB);
}

/**
 * \internal
 * \brief Items grouped by element handle value, e.g. the segments of each face.
 *
 * Two flat arrays sorted by element instead of one container per element: building costs two
 * allocations, and memory grows with the number of items, not with the size of the mesh.
 */
template <typename TIndex>
class IndexGroups {
public:
  struct Entry
  {
    TIndex element{};
    std::size_t item{};

    GEO_NODISCARD constexpr auto operator<=>(const Entry&) const noexcept = default;
  };

  /** \brief Groups \p entries by element; duplicate entries collapse. O(n log n). */
  GEO_NODISCARD static IndexGroups create_from_entries(std::vector<Entry> entries)
  {
    std::ranges::sort(entries);
    const auto duplicates = std::ranges::unique(entries);
    entries.erase(duplicates.begin(), duplicates.end());

    IndexGroups groups;
    groups.m_elements.reserve(entries.size());
    groups.m_items.reserve(entries.size());
    for (const Entry& entry : entries)
    {
      groups.m_elements.push_back(entry.element);
      groups.m_items.push_back(entry.item);
    }
    return groups;
  }

  /** \brief Items of \p element in increasing order; empty if it has none. O(log n). */
  GEO_NODISCARD std::span<const std::size_t> items_of(const TIndex element) const noexcept
  {
    const auto [first, last] = std::ranges::equal_range(m_elements, element);
    const auto offset = static_cast<std::size_t>(std::distance(m_elements.begin(), first));
    const auto count = static_cast<std::size_t>(std::distance(first, last));
    return std::span<const std::size_t>{m_items}.subspan(offset, count);
  }

  /** \brief Number of (element, item) entries. */
  GEO_NODISCARD std::size_t size() const noexcept { return m_items.size(); }

private:
  std::vector<TIndex> m_elements;
  std::vector<std::size_t> m_items;
};

/** \internal \brief Where the intersection points and segments lie on one operand's original mesh. */
template <typename TIndex>
struct OperandIntersections
{
  // Segments in each original face, including those along its boundary edges.
  IndexGroups<TIndex> segmentsByFace;
  // Points whose simplex on this operand is the edge, i.e. strictly inside it.
  IndexGroups<TIndex> pointsByEdge;
  // Points whose simplex on this operand is the face, i.e. strictly inside it.
  IndexGroups<TIndex> pointsByFace;
};

/**
 * \internal
 * \brief All intersection points and segments of two meshes, keyed symbolically, with the lookups
 * corefinement needs.
 *
 * A point found by several face pairs appears once. Its definition comes from the face pair with the
 * smallest (face of A, face of B); different face pairs can define one point differently (e.g. an
 * edge–edge crossing as either edge against the other's face), and fixing the choice keeps the
 * rounded position independent of the traversal order.
 */
template <typename T, typename TIndex>
struct IntersectionGraph
{
  using Key = IntersectionKey<TIndex>;
  // Point indices, the smaller first.
  using Segment = std::array<std::size_t, 2>;

  // Sorted and unique.
  std::vector<Key> keys;
  // points[i] is the point of keys[i].
  std::vector<ImplicitPoint<T>> points;
  // Sorted and unique.
  std::vector<Segment> segments;
  OperandIntersections<TIndex> onMeshA;
  OperandIntersections<TIndex> onMeshB;

  /** \brief Index of the point with \p key, if any. O(log n). */
  GEO_NODISCARD std::optional<std::size_t> find_point(const Key& key) const noexcept
  {
    const auto found = std::ranges::lower_bound(keys, key);
    if (found == keys.end() || *found != key)
    {
      return std::nullopt;
    }
    return static_cast<std::size_t>(std::distance(keys.begin(), found));
  }
};

/** \internal \brief Reason \c compute_intersection_graph could not build a graph; Ok on success. */
enum class IntersectionGraphStatus
{
  Ok,
  // A vertex position is infinite or NaN.
  NonFiniteCoordinates,
  // Predicate results contradicted each other (see FaceIntersection::consistent). Exact predicates
  // rule this out.
  ContradictoryPredicates
};

/**
 * \internal
 * \brief Graph built by \c compute_intersection_graph, or the reason it could not be built.
 *
 * A reported failure always carries an empty graph, never a partial one.
 */
template <typename T, typename TIndex>
struct IntersectionGraphResult
{
  IntersectionGraph<T, TIndex> graph;
  IntersectionGraphStatus error = IntersectionGraphStatus::Ok;

  GEO_NODISCARD bool has_value() const noexcept { return error == IntersectionGraphStatus::Ok; }
  GEO_NODISCARD explicit operator bool() const noexcept { return has_value(); }
};

/** \internal \brief The face pair that found a point or segment, the tie-breaker for its definition. */
template <typename TIndex>
struct FacePair
{
  TIndex faceA{};
  TIndex faceB{};

  GEO_NODISCARD constexpr auto operator<=>(const FacePair&) const noexcept = default;
};

/** \internal \brief A point as found by one face pair, before duplicates are merged. */
template <typename T, typename TIndex>
struct FoundPoint
{
  IntersectionKey<TIndex> key;
  FacePair<TIndex> facePair;
  ImplicitPoint<T> point;
};

/** \internal \brief A segment as found by one face pair, before duplicates are merged. */
template <typename TIndex>
struct FoundSegment
{
  IntersectionKey<TIndex> start;
  IntersectionKey<TIndex> end;
  FacePair<TIndex> facePair;
};

/** \internal \brief Bounding boxes of the live faces of a mesh, and the face of each box. */
template <typename T, typename TIndex>
struct FaceBoxes
{
  std::vector<AABB<T, 3>> boxes;
  std::vector<typename TriangleHalfedgeMesh<T, 3, TIndex>::FaceHandle> faces;
};

/** \internal \brief Closed bounding box of every live face. \pre All positions are finite. O(F). */
template <typename T, typename TIndex>
GEO_NODISCARD FaceBoxes<T, TIndex> make_face_boxes(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh)
{
  using Mesh = TriangleHalfedgeMesh<T, 3, TIndex>;
  using Vec3 = linal::vec3<T>;
  using VertexHandle = typename Mesh::VertexHandle;
  using FaceHandle = typename Mesh::FaceHandle;

  FaceBoxes<T, TIndex> faceBoxes;
  faceBoxes.boxes.reserve(mesh.face_count());
  faceBoxes.faces.reserve(mesh.face_count());
  for (const FaceHandle face : mesh.faces())
  {
    const std::array<VertexHandle, 3> corners = mesh.vertices_around_face(face);
    Vec3 min = mesh.get_position(corners[0]);
    Vec3 max = min;
    for (std::size_t i = 1; i < 3; ++i)
    {
      const Vec3& position = mesh.get_position(corners[i]);
      for (std::uint8_t axis = 0; axis < 3; ++axis)
      {
        min[axis] = std::min(min[axis], position[axis]);
        max[axis] = std::max(max[axis], position[axis]);
      }
    }
    faceBoxes.boxes.push_back(detail::make_closed_box(min, max));
    faceBoxes.faces.push_back(face);
  }
  return faceBoxes;
}

/** \internal \brief Whether every live vertex has finite coordinates. O(V). */
template <typename T, typename TIndex>
GEO_NODISCARD bool has_finite_positions(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh) noexcept
{
  using VertexHandle = typename TriangleHalfedgeMesh<T, 3, TIndex>::VertexHandle;

  return std::ranges::all_of(mesh.vertices(), [&mesh](const VertexHandle vertex) {
    return detail::mesh_position_is_finite(mesh.get_position(vertex));
  });
}

/**
 * \internal
 * \brief Groups the points whose simplex (picked from each key by \p select) has kind \p kind by
 * that simplex's handle value. O(n log n).
 */
template <typename TIndex, typename TSelect>
GEO_NODISCARD IndexGroups<TIndex> group_points(const std::span<const IntersectionKey<TIndex>> keys, const SimplexKind kind, TSelect select)
{
  using Entry = typename IndexGroups<TIndex>::Entry;

  std::vector<Entry> entries;
  for (std::size_t i = 0; i < keys.size(); ++i)
  {
    const MeshSimplex<TIndex> simplex = select(keys[i]);
    if (simplex.kind == kind)
    {
      entries.push_back(Entry{simplex.index, i});
    }
  }
  return IndexGroups<TIndex>::create_from_entries(std::move(entries));
}

/**
 * \internal
 * \brief Merges what the face pairs found into the graph: one point per key, one segment per pair of
 * keys, and the per-operand lookups. O(k log k) for k found items.
 *
 * \pre Every segment's keys are among \p foundPoints.
 */
template <typename T, typename TIndex>
GEO_NODISCARD IntersectionGraph<T, TIndex> assemble_intersection_graph(std::vector<FoundPoint<T, TIndex>> foundPoints,
                                                                       const std::span<const FoundSegment<TIndex>> foundSegments)
{
  using Graph = IntersectionGraph<T, TIndex>;
  using Key = IntersectionKey<TIndex>;
  using Segment = typename Graph::Segment;
  using Entry = typename IndexGroups<TIndex>::Entry;
  using Found = FoundPoint<T, TIndex>;

  struct LocatedSegment
  {
    Segment segment;
    FacePair<TIndex> facePair;
  };

  // The face pair breaks ties so that the kept definition does not depend on the traversal order.
  std::ranges::sort(foundPoints, [](const Found& lhs, const Found& rhs) {
    return std::tie(lhs.key, lhs.facePair) < std::tie(rhs.key, rhs.facePair);
  });
  Graph graph;
  for (const Found& found : foundPoints)
  {
    if (graph.keys.empty() || graph.keys.back() != found.key)
    {
      graph.keys.push_back(found.key);
      graph.points.push_back(found.point);
    }
  }

  std::vector<LocatedSegment> located;
  located.reserve(foundSegments.size());
  for (const FoundSegment<TIndex>& found : foundSegments)
  {
    const std::optional<std::size_t> start = graph.find_point(found.start);
    const std::optional<std::size_t> end = graph.find_point(found.end);
    GEO_ASSERT(start && end && *start != *end);
    located.push_back(LocatedSegment{Segment{std::min(*start, *end), std::max(*start, *end)}, found.facePair});
  }
  std::ranges::sort(located, [](const LocatedSegment& lhs, const LocatedSegment& rhs) {
    return std::tie(lhs.segment, lhs.facePair) < std::tie(rhs.segment, rhs.facePair);
  });

  std::vector<Entry> facesA;
  std::vector<Entry> facesB;
  facesA.reserve(located.size());
  facesB.reserve(located.size());
  for (const LocatedSegment& segment : located)
  {
    if (graph.segments.empty() || graph.segments.back() != segment.segment)
    {
      graph.segments.push_back(segment.segment);
    }
    const std::size_t index = graph.segments.size() - 1;
    facesA.push_back(Entry{segment.facePair.faceA, index});
    facesB.push_back(Entry{segment.facePair.faceB, index});
  }

  const std::span<const Key> keys{graph.keys};
  const auto onA = [](const Key& key) { return key.simplexA; };
  const auto onB = [](const Key& key) { return key.simplexB; };
  graph.onMeshA.segmentsByFace = IndexGroups<TIndex>::create_from_entries(std::move(facesA));
  graph.onMeshA.pointsByEdge = detail::group_points(keys, SimplexKind::Edge, onA);
  graph.onMeshA.pointsByFace = detail::group_points(keys, SimplexKind::Face, onA);
  graph.onMeshB.segmentsByFace = IndexGroups<TIndex>::create_from_entries(std::move(facesB));
  graph.onMeshB.pointsByEdge = detail::group_points(keys, SimplexKind::Edge, onB);
  graph.onMeshB.pointsByFace = detail::group_points(keys, SimplexKind::Face, onB);
  return graph;
}

/**
 * \internal
 * \brief Every intersection point and segment between the faces of \p meshA and \p meshB.
 *
 * The broad phase pairs faces whose closed bounding boxes overlap (\c for_each_overlapping_pair), the
 * narrow phase runs \c intersect_faces on each pair, and the results are merged by key. Isolated
 * touch points are kept: they belong to no segment, but tell later steps where the operands touch.
 *
 * Allocation failures propagate as exceptions. O((F_A + F_B) log(F_A + F_B) + P + k log k) for P
 * candidate face pairs and k found items.
 *
 * \pre Every face of both meshes has non-zero area.
 * \return The graph, or an empty graph and the reason it could not be built.
 */
template <typename T, typename TIndex>
GEO_NODISCARD IntersectionGraphResult<T, TIndex> compute_intersection_graph(const TriangleHalfedgeMesh<T, 3, TIndex>& meshA,
                                                                            const TriangleHalfedgeMesh<T, 3, TIndex>& meshB)
{
  using Mesh = TriangleHalfedgeMesh<T, 3, TIndex>;
  using FaceHandle = typename Mesh::FaceHandle;
  using Result = IntersectionGraphResult<T, TIndex>;
  using Status = IntersectionGraphStatus;
  using Tree = AABBTree<T, TIndex>;
  using Box = AABB<T, 3>;

  if (!detail::has_finite_positions(meshA) || !detail::has_finite_positions(meshB))
  {
    return Result{{}, Status::NonFiniteCoordinates};
  }

  const FaceBoxes<T, TIndex> boxesA = detail::make_face_boxes(meshA);
  const FaceBoxes<T, TIndex> boxesB = detail::make_face_boxes(meshB);
  // Neither can fail: positions are finite, and a live face count always fits the handle type.
  auto treeA = Tree::create_from_boxes(std::span<const Box>{boxesA.boxes});
  auto treeB = Tree::create_from_boxes(std::span<const Box>{boxesB.boxes});
  GEO_ASSERT(treeA.has_value() && treeB.has_value());

  std::vector<FoundPoint<T, TIndex>> foundPoints;
  std::vector<FoundSegment<TIndex>> foundSegments;
  bool consistent = true;
  for_each_overlapping_pair(treeA.tree, treeB.tree, [&](const TIndex boxA, const TIndex boxB) {
    const FaceHandle faceA = boxesA.faces[boxA];
    const FaceHandle faceB = boxesB.faces[boxB];
    const FaceIntersection<T, TIndex> intersection = detail::intersect_faces(meshA, faceA, meshB, faceB);
    consistent = consistent && intersection.consistent;
    const FacePair<TIndex> facePair{faceA.get_value(), faceB.get_value()};
    for (const KeyedPoint<T, TIndex>& point : intersection.points)
    {
      foundPoints.push_back(FoundPoint<T, TIndex>{point.key, facePair, point.point});
    }
    for (const auto& segment : intersection.segments)
    {
      foundSegments.push_back(
          FoundSegment<TIndex>{intersection.points[segment[0]].key, intersection.points[segment[1]].key, facePair});
    }
  });
  if (!consistent)
  {
    return Result{{}, Status::ContradictoryPredicates};
  }
  return Result{detail::assemble_intersection_graph(std::move(foundPoints), std::span<const FoundSegment<TIndex>>{foundSegments}),
                Status::Ok};
}

} // namespace detail
} // namespace Geometry

#endif // GEOMETRY_MESH_DETAIL_BOOLEANINTERSECTION_HPP
