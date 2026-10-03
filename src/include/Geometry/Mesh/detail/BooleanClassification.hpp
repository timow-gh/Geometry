#ifndef GEOMETRY_MESH_DETAIL_BOOLEANCLASSIFICATION_HPP
#define GEOMETRY_MESH_DETAIL_BOOLEANCLASSIFICATION_HPP

#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Mesh/detail/BooleanIntersection.hpp"
#include "Geometry/Mesh/detail/Corefine.hpp"
#include "Geometry/Predicates.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include "Geometry/detail/ImplicitPoint.hpp"
#include "Geometry/detail/TriangleOrientation.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <linal/vec.hpp>
#include <numbers>
#include <optional>
#include <vector>

namespace Geometry
{
namespace detail
{

/** \internal \brief Where a patch of one refined operand lies relative to the other operand's solid. */
enum class PatchClass : std::uint8_t
{
  Inside,
  Outside,
  // On the other operand's surface, facing the same way as the face it coincides with.
  OnSame,
  // On the other operand's surface, facing the opposite way.
  OnOpposite
};

/** \internal \brief The operand other than \p operand. O(1). */
GEO_NODISCARD constexpr Operand other_operand(const Operand operand) noexcept
{
  return operand == Operand::A ? Operand::B : Operand::A;
}

/** \internal \brief The refinement of \p operand within \p corefinement. O(1). */
template <typename T, typename TIndex>
GEO_NODISCARD const OperandRefinement<T, TIndex>& refinement_on(const Corefinement<T, TIndex>& corefinement, const Operand operand) noexcept
{
  return operand == Operand::A ? corefinement.onMeshA : corefinement.onMeshB;
}

/**
 * \internal
 * \brief \c classify_against_wedge for two coplanar faces of the other operand, where the face planes
 * cannot tell the two sides of the edge apart and a projection of the shared plane does.
 *
 * Coplanar faces on opposite sides of the edge form a flat wedge: one plane, one inner side. Coplanar
 * faces on the same side mean the other operand folds back onto itself, which is not a solid.
 *
 * \param forwardSide \c orient3d(start, end, forwardApex, apex).
 * \param backwardSide \c orient3d(end, start, backwardApex, apex).
 * O(1).
 */
template <typename T>
GEO_NODISCARD std::optional<PatchClass> classify_against_flat_wedge(const ImplicitPoint<T>& start,
                                                                    const ImplicitPoint<T>& end,
                                                                    const ImplicitPoint<T>& apex,
                                                                    const ImplicitPoint<T>& forwardApex,
                                                                    const ImplicitPoint<T>& backwardApex,
                                                                    const Orientation forwardSide,
                                                                    const Orientation backwardSide) noexcept
{
  using Vec3 = linal::vec3<T>;

  // Flat, the two planes are one with one orientation, so both sides must agree.
  if (forwardSide != backwardSide)
  {
    return std::nullopt;
  }
  if (forwardSide != Orientation::Zero)
  {
    return forwardSide == Orientation::Negative ? PatchClass::Inside : PatchClass::Outside;
  }

  // The projection only has to keep the plane non-degenerate; any face of it serves.
  const Vec3 normal = detail::triangle_orientation(start.position(), end.position(), forwardApex.position());
  if (normal[0] == T{0} && normal[1] == T{0} && normal[2] == T{0})
  {
    return std::nullopt;
  }
  const std::uint8_t axis = detail::dominant_axis(normal);
  // Patch classification runs on rounded intersection points, so it is only as exact as the
  // ImplicitPoint predicates (see detail::ImplicitPoint).
  const Orientation forwardTurn = orient2d(start, end, forwardApex, axis);
  const Orientation backwardTurn = orient2d(start, end, backwardApex, axis);
  if (detail::multiply_signs(forwardTurn, backwardTurn) != Orientation::Negative)
  {
    return std::nullopt;
  }
  const Orientation apexTurn = orient2d(start, end, apex, axis);
  if (apexTurn == forwardTurn)
  {
    return PatchClass::OnSame;
  }
  if (apexTurn == backwardTurn)
  {
    return PatchClass::OnOpposite;
  }
  return std::nullopt;
}

/**
 * \internal
 * \brief Classifies the face side running from \p start to \p end, whose face has its third corner at
 * \p apex, against the other operand's two faces at the same edge: (start, end, \p forwardApex) and
 * (end, start, \p backwardApex).
 *
 * Near the edge, the other operand's solid is the wedge on the inner side of its two faces, and the
 * face points from the edge towards \p apex, so the sides of \p apex against the two face planes decide.
 * A convex wedge is the intersection of the two inner half-spaces, a reflex one their union. A face
 * whose apex lies in the plane of one of the two faces, on that face's side of the edge, coincides with
 * it near the edge; it faces the same way iff both run from \p start to \p end.
 *
 * Both operands are outward-oriented, so "inner side" is \c orient3d \c Negative.
 *
 * \return \c std::nullopt when the side tells nothing: the apex lies on the edge's line (a sliver face,
 * which corefinement leaves where rounding moved a point off the edge it lies on), the predicate
 * results contradict each other, or the other operand folds back onto itself at the edge (its two
 * faces coincide), which leaves no wedge. O(1).
 */
template <typename T>
GEO_NODISCARD std::optional<PatchClass> classify_against_wedge(const ImplicitPoint<T>& start,
                                                               const ImplicitPoint<T>& end,
                                                               const ImplicitPoint<T>& apex,
                                                               const ImplicitPoint<T>& forwardApex,
                                                               const ImplicitPoint<T>& backwardApex) noexcept
{
  // Patch classification runs on rounded intersection points, so it is only as exact as the
  // ImplicitPoint predicates (see detail::ImplicitPoint).
  const Orientation forwardSide = orient3d(start, end, forwardApex, apex);
  const Orientation backwardSide = orient3d(end, start, backwardApex, apex);
  // Negative: the wedge is convex. It also equals the side of forwardApex against the backward face, so
  // an apex in the forward plane lies on forwardApex's side of the edge iff backwardSide equals it.
  const Orientation bend = orient3d(start, end, forwardApex, backwardApex);
  if (bend == Orientation::Zero)
  {
    return detail::classify_against_flat_wedge(start, end, apex, forwardApex, backwardApex, forwardSide, backwardSide);
  }
  if (forwardSide == Orientation::Zero && backwardSide == bend)
  {
    return PatchClass::OnSame;
  }
  if (backwardSide == Orientation::Zero && forwardSide == bend)
  {
    return PatchClass::OnOpposite;
  }
  if (forwardSide == Orientation::Zero && backwardSide == Orientation::Zero)
  {
    // The apex would lie on the edge's line.
    return std::nullopt;
  }
  const bool insideForward = forwardSide == Orientation::Negative;
  const bool insideBackward = backwardSide == Orientation::Negative;
  if (bend == Orientation::Negative)
  {
    return insideForward && insideBackward ? PatchClass::Inside : PatchClass::Outside;
  }
  return insideForward || insideBackward ? PatchClass::Inside : PatchClass::Outside;
}

/**
 * \internal
 * \brief Classifies the face of \p side, a halfedge of a refined operand on an intersection edge,
 * against the other refined operand's two faces at that edge.
 *
 * \pre The edge of \p side is an intersection edge, and the other operand is closed. O(valence) for
 * finding the edge in the other operand.
 */
template <typename T, typename TIndex>
GEO_NODISCARD std::optional<PatchClass> classify_at_intersection_edge(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh,
                                                                      const TriangleHalfedgeMesh<T, 3, TIndex>& otherMesh,
                                                                      const Corefinement<T, TIndex>& corefinement,
                                                                      const Operand operand,
                                                                      const typename TriangleHalfedgeMesh<T, 3, TIndex>::HalfedgeHandle side) noexcept
{
  using Mesh = TriangleHalfedgeMesh<T, 3, TIndex>;
  using VertexHandle = typename Mesh::VertexHandle;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using Refinement = OperandRefinement<T, TIndex>;

  const Refinement& refinement = detail::refinement_on(corefinement, operand);
  const Refinement& otherRefinement = detail::refinement_on(corefinement, detail::other_operand(operand));
  const std::size_t startPoint = refinement.pointOfVertex[mesh.source_vertex(side).get_value()];
  const std::size_t endPoint = refinement.pointOfVertex[mesh.target_vertex(side).get_value()];
  GEO_ASSERT(startPoint != no_intersection_point && endPoint != no_intersection_point);

  const HalfedgeHandle forward =
      otherMesh.find_halfedge(otherRefinement.vertexOfPoint[startPoint], otherRefinement.vertexOfPoint[endPoint]);
  GEO_ASSERT(forward.is_valid());
  const HalfedgeHandle backward = otherMesh.get_halfedge(forward).twin;
  GEO_ASSERT(!otherMesh.is_boundary(forward) && !otherMesh.is_boundary(backward));

  const VertexHandle apex = mesh.target_vertex(mesh.get_halfedge(side).next);
  const VertexHandle forwardApex = otherMesh.target_vertex(otherMesh.get_halfedge(forward).next);
  const VertexHandle backwardApex = otherMesh.target_vertex(otherMesh.get_halfedge(backward).next);
  return detail::classify_against_wedge(corefinement.graph.points[startPoint],
                                        corefinement.graph.points[endPoint],
                                        detail::implicit_point_of(mesh, refinement, corefinement.graph, apex),
                                        detail::implicit_point_of(otherMesh, otherRefinement, corefinement.graph, forwardApex),
                                        detail::implicit_point_of(otherMesh, otherRefinement, corefinement.graph, backwardApex));
}

/**
 * \internal
 * \brief Generalized winding number of the closed surface \p mesh at \p query: the solid angle the
 * surface subtends at \p query, divided by 4 pi (Jacobson, Kavan, Sorkine-Hornung, "Robust
 * inside-outside segmentation using generalized winding numbers", 2013).
 *
 * For an outward-oriented closed surface without self-intersections it is 1 inside and 0 outside, up
 * to rounding, so thresholding at 1/2 decides containment without casting a ray that could graze an
 * edge or a vertex. Each face contributes its signed solid angle (Van Oosterom and Strackee, 1983).
 * \pre \p query is not on the surface. O(F).
 */
template <typename T, typename TIndex>
GEO_NODISCARD T winding_number(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh, const linal::vec3<T>& query) noexcept
{
  using Mesh = TriangleHalfedgeMesh<T, 3, TIndex>;
  using Vec3 = linal::vec3<T>;
  using VertexHandle = typename Mesh::VertexHandle;
  using FaceHandle = typename Mesh::FaceHandle;

  T halfAngleSum{0};
  for (const FaceHandle face : mesh.faces())
  {
    const std::array<VertexHandle, 3> corners = mesh.vertices_around_face(face);
    const Vec3 first{mesh.get_position(corners[0]) - query};
    const Vec3 second{mesh.get_position(corners[1]) - query};
    const Vec3 third{mesh.get_position(corners[2]) - query};
    const T firstLength = linal::length(first);
    const T secondLength = linal::length(second);
    const T thirdLength = linal::length(third);
    const T numerator = linal::dot(first, linal::cross(second, third));
    const T denominator = firstLength * secondLength * thirdLength + linal::dot(first, second) * thirdLength
                          + linal::dot(second, third) * firstLength + linal::dot(third, first) * secondLength;
    halfAngleSum += std::atan2(numerator, denominator);
  }
  // Each atan2 is half the face's solid angle.
  return halfAngleSum / (T{2} * std::numbers::pi_v<T>);
}

/** \internal \brief Centroid of \p face, from its rounded corner positions. O(1). */
template <typename T, typename TIndex>
GEO_NODISCARD linal::vec3<T> face_centroid(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh,
                                           const typename TriangleHalfedgeMesh<T, 3, TIndex>::FaceHandle face) noexcept
{
  using Vec3 = linal::vec3<T>;

  const auto corners = mesh.vertices_around_face(face);
  const Vec3 sum{mesh.get_position(corners[0]) + mesh.get_position(corners[1]) + mesh.get_position(corners[2])};
  return Vec3{sum / T{3}};
}

/** \internal \brief Entry of \c OperandPatches::patchOfFace for a deleted face. */
inline constexpr std::size_t no_patch = std::numeric_limits<std::size_t>::max();

/**
 * \internal
 * \brief The patches of a refined operand: maximal sets of faces connected across edges that are not
 * intersection edges.
 *
 * The intersection curve separates the parts of a surface that lie inside the other operand from
 * those outside, so every patch lies entirely on one side, or on the other surface.
 */
template <typename TIndex>
struct OperandPatches
{
  // By face storage index.
  std::vector<std::size_t> patchOfFace;
  // By patch: one of its faces.
  std::vector<TIndex> seedFace;
};

/** \internal \brief Flood fills the faces of \p mesh into patches, never crossing an intersection edge. O(F). */
template <typename T, typename TIndex>
GEO_NODISCARD OperandPatches<TIndex> find_patches(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh, const std::vector<bool>& isIntersectionEdge)
{
  using Mesh = TriangleHalfedgeMesh<T, 3, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using FaceHandle = typename Mesh::FaceHandle;

  OperandPatches<TIndex> patches;
  patches.patchOfFace.assign(mesh.face_storage_size(), no_patch);
  std::vector<FaceHandle> stack;
  for (const FaceHandle seed : mesh.faces())
  {
    if (patches.patchOfFace[seed.get_value()] != no_patch)
    {
      continue;
    }
    const std::size_t patch = patches.seedFace.size();
    patches.seedFace.push_back(seed.get_value());
    patches.patchOfFace[seed.get_value()] = patch;
    stack.push_back(seed);
    while (!stack.empty())
    {
      const FaceHandle face = stack.back();
      stack.pop_back();
      for (const HalfedgeHandle side : mesh.halfedges_around_face(face))
      {
        const FaceHandle neighbor = mesh.get_halfedge(mesh.get_halfedge(side).twin).face;
        if (isIntersectionEdge[mesh.get_halfedge(side).edge.get_value()] || !neighbor.is_valid()
            || patches.patchOfFace[neighbor.get_value()] != no_patch)
        {
          continue;
        }
        patches.patchOfFace[neighbor.get_value()] = patch;
        stack.push_back(neighbor);
      }
    }
  }
  return patches;
}

/**
 * \internal
 * \brief The \c PatchClass of every face of the refined \p operand against the other refined operand
 * \p otherMesh, by face storage index.
 *
 * A patch bounded by intersection edges is classified at each of them by \c classify_against_wedge;
 * since a patch never crosses the other surface, all of them must agree. A side that tells nothing
 * (see there) abstains instead of failing the patch: with the floating-point placeholders,
 * corefinement can leave a sliver face whose apex lies on its intersection edge, while the patch's
 * other sides still decide. A patch without an intersection edge is a whole component that does not
 * meet the other surface along a curve (at most at isolated points), and the winding number at one of
 * its face centroids decides. A centroid rather than a vertex, because every vertex of such a
 * component may be a touch point on the other surface, while a face's interior never meets it.
 *
 * \pre Both operands are closed and outward-oriented, refined by \p corefinement.
 * \return \c std::nullopt if two sides of one patch disagree, or no side of a patch bounded by
 * intersection edges tells anything, both of which only contradicting predicate results produce.
 * O(F + I * valence + C * F_other) for I intersection edges and C components without one.
 */
template <typename T, typename TIndex>
GEO_NODISCARD std::optional<std::vector<PatchClass>> classify_faces(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh,
                                                                    const TriangleHalfedgeMesh<T, 3, TIndex>& otherMesh,
                                                                    const Corefinement<T, TIndex>& corefinement,
                                                                    const Operand operand)
{
  using Mesh = TriangleHalfedgeMesh<T, 3, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using FaceHandle = typename Mesh::FaceHandle;

  const std::vector<bool>& isIntersectionEdge = detail::refinement_on(corefinement, operand).isIntersectionEdge;
  const OperandPatches<TIndex> patches = detail::find_patches(mesh, isIntersectionEdge);
  std::vector<std::optional<PatchClass>> classOfPatch(patches.seedFace.size());
  std::vector<bool> boundedByCurve(patches.seedFace.size(), false);

  for (const FaceHandle face : mesh.faces())
  {
    const std::size_t patch = patches.patchOfFace[face.get_value()];
    for (const HalfedgeHandle side : mesh.halfedges_around_face(face))
    {
      if (!isIntersectionEdge[mesh.get_halfedge(side).edge.get_value()])
      {
        continue;
      }
      boundedByCurve[patch] = true;
      const std::optional<PatchClass> sideClass = detail::classify_at_intersection_edge(mesh, otherMesh, corefinement, operand, side);
      if (!sideClass)
      {
        continue;
      }
      if (classOfPatch[patch] && *classOfPatch[patch] != *sideClass)
      {
        return std::nullopt;
      }
      classOfPatch[patch] = sideClass;
    }
  }

  for (std::size_t patch = 0; patch < classOfPatch.size(); ++patch)
  {
    if (boundedByCurve[patch] && !classOfPatch[patch])
    {
      return std::nullopt;
    }
    if (!classOfPatch[patch])
    {
      const linal::vec3<T> query = detail::face_centroid(mesh, FaceHandle{patches.seedFace[patch]});
      classOfPatch[patch] = detail::winding_number(otherMesh, query) > T{0.5} ? PatchClass::Inside : PatchClass::Outside;
    }
  }

  std::vector<PatchClass> classOfFace(mesh.face_storage_size(), PatchClass::Outside);
  for (const FaceHandle face : mesh.faces())
  {
    classOfFace[face.get_value()] = *classOfPatch[patches.patchOfFace[face.get_value()]];
  }
  return classOfFace;
}

} // namespace detail
} // namespace Geometry

#endif // GEOMETRY_MESH_DETAIL_BOOLEANCLASSIFICATION_HPP
