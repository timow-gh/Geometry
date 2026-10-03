#ifndef GEOMETRY_MESH_MESHBOOLEAN_HPP
#define GEOMETRY_MESH_MESHBOOLEAN_HPP

#include "Geometry/Mesh/MeshCorefineStatus.hpp"
#include "Geometry/Mesh/MeshFromTriangles.hpp"
#include "Geometry/Mesh/MeshNormals.hpp"
#include "Geometry/Mesh/MeshOrientation.hpp"
#include "Geometry/Mesh/MeshVerify.hpp"
#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Mesh/detail/BooleanClassification.hpp"
#include "Geometry/Mesh/detail/BooleanIntersection.hpp"
#include "Geometry/Mesh/detail/Corefine.hpp"
#include "Geometry/Mesh/detail/MeshResult.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"

#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <linal/vec.hpp>
#include <optional>
#include <span>
#include <utility>
#include <vector>

namespace Geometry
{

/** \brief Which Boolean \c mesh_boolean computes. */
enum class BooleanOperation : std::uint8_t
{
  Union,
  Intersection,
  // A minus B.
  Difference
};

/** \brief Reason \c mesh_boolean could not compute a result; Ok on success. */
enum class BooleanStatus
{
  Ok,
  // An operand has an infinite or NaN vertex position.
  OperandNotFinite,
  // An operand has an edge with more than two faces, or a vertex where several fans meet.
  OperandNotManifold,
  // An operand has a boundary, so it bounds no solid.
  OperandNotClosed,
  // An operand has a zero-area face, which has no plane to intersect with.
  OperandDegenerateFace,
  // An operand's faces point into its volume, or it encloses no volume.
  OperandNotOutward,
  // Predicate results contradict each other, or two distinct intersection points round to one
  // position. Exact predicates on explicit points rule out the first; the second needs predicates on
  // the intersection points' definitions (see detail::ImplicitPoint).
  DegenerateIntersection,
  // The result is a solid the manifold mesh cannot represent: parts touching along an edge or at a
  // vertex.
  NonManifoldResult,
  // The result has more vertices or halfedges than the mesh's handle type can number.
  IndexCapacityExceeded
};

/** \brief How \c mesh_boolean marks crease edges on its result. */
template <std::floating_point T>
struct MeshBooleanOptions
{
  // Keep the crease flags of operand edges away from the intersection curve. Curve edges never
  // inherit a flag: a crease of one operand that ends up on the curve, e.g. the rim of a face two
  // operands share, no longer bounds the same two faces.
  bool transferCreases{true};
  // When set, also mark intersection curve edges whose dihedral angle in the result exceeds this
  // angle, in radians. When unset, no curve edge is a crease.
  std::optional<T> intersectionCreaseAngle;
};

/**
 * \brief Mesh built by \c mesh_boolean, or the reason it could not be built.
 *
 * A reported failure always carries an empty mesh. A successful result may be empty too, e.g. the
 * intersection of disjoint operands.
 */
template <typename T, typename TIndex>
struct MeshBooleanResult
{
  TriangleHalfedgeMesh<T, 3, TIndex> mesh;
  BooleanStatus error = BooleanStatus::Ok;

  GEO_NODISCARD bool has_value() const noexcept { return detail::mesh_result_ok(error); }
  GEO_NODISCARD explicit operator bool() const noexcept { return has_value(); }
};

namespace detail
{

/**
 * \internal
 * \brief Whether \p operation keeps a face of \p operand classified as \p patchClass.
 *
 * A region where the surfaces coincide exists in both refined operands, so at most A's copy is kept.
 * Copies facing the same way mean both solids lie on the same side of the region, which therefore
 * bounds their union and their intersection. Copies facing opposite ways mean the solids lie on
 * opposite sides, so the region bounds only A - B. Dropping it from the union merges
 * touching solids without an inner double wall; dropping a same-facing region from a difference opens
 * a flush cut without a zero-thickness skin. O(1).
 */
GEO_NODISCARD constexpr bool keeps_face(const BooleanOperation operation, const Operand operand, const PatchClass patchClass) noexcept
{
  switch (patchClass)
  {
  case PatchClass::Inside:
    return operation == BooleanOperation::Intersection || (operation == BooleanOperation::Difference && operand == Operand::B);
  case PatchClass::Outside:
    return operation == BooleanOperation::Union || (operation == BooleanOperation::Difference && operand == Operand::A);
  case PatchClass::OnSame:
    return operand == Operand::A && operation != BooleanOperation::Difference;
  case PatchClass::OnOpposite:
    return operand == Operand::A && operation == BooleanOperation::Difference;
  }
  GEO_ASSERT(false);
  return false;
}

/**
 * \internal
 * \brief Whether \p operation keeps the faces of \p operand reversed: the part of B inside A bounds
 * A - B from the other side. O(1).
 */
GEO_NODISCARD constexpr bool reverses_faces(const BooleanOperation operation, const Operand operand) noexcept
{
  return operation == BooleanOperation::Difference && operand == Operand::B;
}

/**
 * \internal
 * \brief Checks that \p mesh bounds a solid: finite, manifold, closed, without zero-area faces and
 * outward-oriented. A mesh without faces is the empty solid.
 *
 * Self-intersections are not detected. Outward orientation is judged by the total signed volume, so a
 * cavity (an inward shell inside an outward one) is accepted. O(V + H).
 */
template <typename T, typename TIndex>
GEO_NODISCARD BooleanStatus validate_operand(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh)
{
  if (!detail::has_finite_positions(mesh))
  {
    return BooleanStatus::OperandNotFinite;
  }
  if (!verify_manifold(mesh))
  {
    return BooleanStatus::OperandNotManifold;
  }
  if (!verify_closed(mesh))
  {
    return BooleanStatus::OperandNotClosed;
  }
  if (detail::has_degenerate_face(mesh))
  {
    return BooleanStatus::OperandDegenerateFace;
  }
  if (mesh.face_count() > 0 && is_outward_oriented(mesh) != std::optional<bool>{true})
  {
    return BooleanStatus::OperandNotOutward;
  }
  return BooleanStatus::Ok;
}

/** \internal \brief The \c BooleanStatus for a failed corefinement of two validated operands. O(1). */
GEO_NODISCARD constexpr BooleanStatus boolean_status_of(const CorefineStatus status) noexcept
{
  switch (status)
  {
  case CorefineStatus::Ok: return BooleanStatus::Ok;
  case CorefineStatus::NonFiniteCoordinates: return BooleanStatus::OperandNotFinite;
  case CorefineStatus::DegenerateFace: return BooleanStatus::OperandDegenerateFace;
  case CorefineStatus::DegenerateIntersection: return BooleanStatus::DegenerateIntersection;
  }
  GEO_ASSERT(false);
  return BooleanStatus::DegenerateIntersection;
}

/** \internal \brief Where one side of a result triangle came from, for transferring crease flags. */
struct SideOrigin
{
  bool crease{false};
  bool onIntersectionCurve{false};
};

/**
 * \internal
 * \brief The kept faces of both refined operands as one indexed triangle list.
 *
 * \c sides holds three entries per triangle, side j of triangle f at 3f + j, which is also the handle
 * value of the halfedge \c make_mesh_from_triangles builds for it; it stays empty unless requested.
 */
template <typename T, typename TIndex>
struct BooleanTriangles
{
  std::vector<linal::vec3<T>> positions;
  std::vector<std::array<TIndex, 3>> triangles;
  std::vector<SideOrigin> sides;
};

/**
 * \internal
 * \brief Collects the faces \p operation keeps from both refined operands, B's reversed for a
 * difference, with only the vertices they use.
 *
 * An intersection point is one vertex in each operand at the same position; both map to A's, so the
 * two parts meet along shared vertices.
 *
 * \return \c std::nullopt if the vertices do not fit \p TIndex. O(V + F).
 */
template <typename T, typename TIndex>
GEO_NODISCARD std::optional<BooleanTriangles<T, TIndex>> collect_kept_faces(const TriangleHalfedgeMesh<T, 3, TIndex>& meshA,
                                                                            const TriangleHalfedgeMesh<T, 3, TIndex>& meshB,
                                                                            const Corefinement<T, TIndex>& corefinement,
                                                                            const std::array<std::vector<PatchClass>, 2>& classOfFace,
                                                                            const BooleanOperation operation,
                                                                            const bool recordSides)
{
  using Mesh = TriangleHalfedgeMesh<T, 3, TIndex>;
  using VertexHandle = typename Mesh::VertexHandle;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using EdgeHandle = typename Mesh::EdgeHandle;
  using FaceHandle = typename Mesh::FaceHandle;
  using Triangles = BooleanTriangles<T, TIndex>;

  // The largest index value is the invalid handle, so every result vertex index must stay below it.
  constexpr TIndex unassigned = std::numeric_limits<TIndex>::max();
  std::array<std::vector<TIndex>, 2> resultVertex{std::vector<TIndex>(meshA.vertex_storage_size(), unassigned),
                                                  std::vector<TIndex>(meshB.vertex_storage_size(), unassigned)};
  Triangles result;
  result.positions.reserve(meshA.vertex_count() + meshB.vertex_count());
  result.triangles.reserve(meshA.face_count() + meshB.face_count());

  const auto resultVertexOf = [&](Operand operand, VertexHandle vertex) -> std::optional<TIndex> {
    if (operand == Operand::B)
    {
      const std::size_t point = corefinement.onMeshB.pointOfVertex[vertex.get_value()];
      if (point != no_intersection_point)
      {
        operand = Operand::A;
        vertex = corefinement.onMeshA.vertexOfPoint[point];
      }
    }
    TIndex& entry = resultVertex[static_cast<std::size_t>(operand)][vertex.get_value()];
    if (entry == unassigned)
    {
      if (result.positions.size() + 1 >= static_cast<std::size_t>(unassigned))
      {
        return std::nullopt;
      }
      entry = static_cast<TIndex>(result.positions.size());
      result.positions.push_back((operand == Operand::A ? meshA : meshB).get_position(vertex));
    }
    return entry;
  };

  for (const Operand operand : {Operand::A, Operand::B})
  {
    const Mesh& mesh = operand == Operand::A ? meshA : meshB;
    const std::vector<bool>& isIntersectionEdge = detail::refinement_on(corefinement, operand).isIntersectionEdge;
    const bool reversed = detail::reverses_faces(operation, operand);
    for (const FaceHandle face : mesh.faces())
    {
      if (!detail::keeps_face(operation, operand, classOfFace[static_cast<std::size_t>(operand)][face.get_value()]))
      {
        continue;
      }
      const std::array<HalfedgeHandle, 3> halfedges = mesh.halfedges_around_face(face);
      std::array<TIndex, 3> corners{};
      for (std::size_t j = 0; j < 3; ++j)
      {
        const std::optional<TIndex> corner = resultVertexOf(operand, mesh.source_vertex(halfedges[j]));
        if (!corner)
        {
          return std::nullopt;
        }
        corners[j] = *corner;
      }
      if (reversed)
      {
        std::swap(corners[1], corners[2]);
      }
      result.triangles.push_back(corners);
      if (!recordSides)
      {
        continue;
      }
      for (std::size_t j = 0; j < 3; ++j)
      {
        // Reversed, side j runs along the face's halfedge 2 - j, backwards.
        const EdgeHandle edge = mesh.get_halfedge(halfedges[reversed ? 2 - j : j]).edge;
        result.sides.push_back(SideOrigin{mesh.is_crease(edge), isIntersectionEdge[edge.get_value()]});
      }
    }
  }
  return result;
}

/** \internal \brief The \c BooleanStatus for a failed \c make_mesh_from_triangles on kept faces. O(1). */
GEO_NODISCARD constexpr BooleanStatus boolean_status_of(const MeshFromTrianglesStatus status) noexcept
{
  switch (status)
  {
  case MeshFromTrianglesStatus::Ok: return BooleanStatus::Ok;
  case MeshFromTrianglesStatus::IndexCapacityExceeded: return BooleanStatus::IndexCapacityExceeded;
  case MeshFromTrianglesStatus::NonManifoldEdge:
  case MeshFromTrianglesStatus::NonManifoldVertex: return BooleanStatus::NonManifoldResult;
  // Correctly classified patches never run along an edge in the same direction, so only contradicting
  // predicate results produce this.
  case MeshFromTrianglesStatus::InconsistentOrientation: return BooleanStatus::DegenerateIntersection;
  // Kept faces always name three distinct, collected vertices.
  case MeshFromTrianglesStatus::VertexIndexOutOfRange:
  case MeshFromTrianglesStatus::DegenerateTriangle: GEO_ASSERT(false); return BooleanStatus::DegenerateIntersection;
  }
  GEO_ASSERT(false);
  return BooleanStatus::DegenerateIntersection;
}

/**
 * \internal
 * \brief Sets the crease flags of the result built from \p sides as \p options ask.
 *
 * \pre \p mesh was built by \c make_mesh_from_triangles from the triangles \p sides describe, and is
 * closed. O(H).
 */
template <typename T, typename TIndex>
void mark_result_creases(TriangleHalfedgeMesh<T, 3, TIndex>& mesh, const std::span<const SideOrigin> sides, const MeshBooleanOptions<T>& options)
{
  using Mesh = TriangleHalfedgeMesh<T, 3, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using EdgeHandle = typename Mesh::EdgeHandle;

  const std::optional<T> cosThreshold =
      options.intersectionCreaseAngle ? std::optional<T>{std::cos(*options.intersectionCreaseAngle)} : std::nullopt;
  for (std::size_t i = 0; i < sides.size(); ++i)
  {
    const HalfedgeHandle halfedge{static_cast<TIndex>(i)};
    const EdgeHandle edge = mesh.get_halfedge(halfedge).edge;
    if (!sides[i].onIntersectionCurve)
    {
      // Both sides of such an edge come from the same operand edge, so they agree.
      if (options.transferCreases && sides[i].crease)
      {
        mesh.set_crease(edge, true);
      }
      continue;
    }
    // Once per edge: from its stored halfedge.
    if (cosThreshold && mesh.get_edge(edge).halfedge == halfedge)
    {
      const std::optional<bool> sharp = detail::exceeds_crease_angle(mesh, edge, *cosThreshold);
      mesh.set_crease(edge, sharp.value_or(false));
    }
  }
}

} // namespace detail

/**
 * \brief Union, intersection or difference (A - B) of the solids bounded by \p meshA and \p meshB, as
 * a new mesh.
 *
 * The operands are corefined along the curve where their surfaces meet, each refined surface is cut
 * along that curve into patches, every patch is classified as inside or outside the other solid (or as
 * lying on its surface), and the patches \p operation keeps are stitched into the result. Where the
 * surfaces overlap (coplanar contact), the shared region is kept once or dropped, never doubled:
 * solids that touch along a face merge, and a cut flush with a face opens instead of leaving a
 * zero-thickness skin. The result is closed and outward-oriented; its vertices are the operands'
 * vertices it uses plus the intersection points, at their rounded positions.
 *
 * Operands are validated, except for self-intersections, which they must not have. An operand
 * without faces is the empty solid.
 *
 * \p options controls crease flags: by default, operand creases away from the intersection curve are
 * kept and no curve edge is a crease.
 *
 * Allocation failures propagate as exceptions. O((V + F) log(V + F) + k log k + corefinement + C * F)
 * for k intersection points and C components that do not meet the other surface along a curve; see
 * \c corefine.
 *
 * \return The result, or an empty mesh and the reason: an operand status (\c OperandNotFinite,
 * \c OperandNotManifold, \c OperandNotClosed, \c OperandDegenerateFace, \c OperandNotOutward);
 * \c DegenerateIntersection when the floating-point predicates contradict each other or two
 * intersection points round to one position; \c NonManifoldResult when parts of the result touch along
 * an edge or at a vertex; \c IndexCapacityExceeded when the result does not fit \p TIndex.
 */
template <typename T, typename TIndex>
GEO_NODISCARD MeshBooleanResult<T, TIndex> mesh_boolean(const TriangleHalfedgeMesh<T, 3, TIndex>& meshA,
                                                        const TriangleHalfedgeMesh<T, 3, TIndex>& meshB,
                                                        const BooleanOperation operation,
                                                        const MeshBooleanOptions<T>& options = {})
{
  using Mesh = TriangleHalfedgeMesh<T, 3, TIndex>;
  using Result = MeshBooleanResult<T, TIndex>;
  using Classes = std::vector<detail::PatchClass>;

  const auto failure = [](const BooleanStatus error) { return Result{Mesh{}, error}; };
  for (const Mesh* operand : {&meshA, &meshB})
  {
    if (const BooleanStatus error = detail::validate_operand(*operand); error != BooleanStatus::Ok)
    {
      return failure(error);
    }
  }

  Mesh refinedA = meshA;
  Mesh refinedB = meshB;
  const detail::CorefinementResult<T, TIndex> corefined = detail::corefine_in_place(refinedA, refinedB);
  if (!corefined.has_value())
  {
    return failure(detail::boolean_status_of(corefined.error));
  }
  const detail::Corefinement<T, TIndex>& corefinement = corefined.corefinement;

  std::optional<Classes> classesA = detail::classify_faces(refinedA, refinedB, corefinement, detail::Operand::A);
  std::optional<Classes> classesB = detail::classify_faces(refinedB, refinedA, corefinement, detail::Operand::B);
  if (!classesA || !classesB)
  {
    return failure(BooleanStatus::DegenerateIntersection);
  }

  const bool recordSides = options.transferCreases || options.intersectionCreaseAngle.has_value();
  const std::optional<detail::BooleanTriangles<T, TIndex>> kept = detail::collect_kept_faces(
      refinedA, refinedB, corefinement, std::array<Classes, 2>{std::move(*classesA), std::move(*classesB)}, operation, recordSides);
  if (!kept)
  {
    return failure(BooleanStatus::IndexCapacityExceeded);
  }

  MeshFromTrianglesResult<T, 3, TIndex> built = make_mesh_from_triangles(std::span<const linal::vec3<T>>{kept->positions},
                                                                         std::span<const std::array<TIndex, 3>>{kept->triangles});
  if (!built.has_value())
  {
    return failure(detail::boolean_status_of(built.error));
  }
  // Correctly classified patches close up along the curve, so only contradicting predicate results
  // leave a boundary.
  if (!verify_closed(built.mesh))
  {
    return failure(BooleanStatus::DegenerateIntersection);
  }
  if (recordSides)
  {
    detail::mark_result_creases(built.mesh, std::span<const detail::SideOrigin>{kept->sides}, options);
  }
  return Result{std::move(built.mesh), BooleanStatus::Ok};
}

/** \brief \c mesh_boolean with \c BooleanOperation::Union. */
template <typename T, typename TIndex>
GEO_NODISCARD MeshBooleanResult<T, TIndex> mesh_union(const TriangleHalfedgeMesh<T, 3, TIndex>& meshA,
                                                      const TriangleHalfedgeMesh<T, 3, TIndex>& meshB,
                                                      const MeshBooleanOptions<T>& options = {})
{
  return mesh_boolean(meshA, meshB, BooleanOperation::Union, options);
}

/** \brief \c mesh_boolean with \c BooleanOperation::Intersection. */
template <typename T, typename TIndex>
GEO_NODISCARD MeshBooleanResult<T, TIndex> mesh_intersection(const TriangleHalfedgeMesh<T, 3, TIndex>& meshA,
                                                             const TriangleHalfedgeMesh<T, 3, TIndex>& meshB,
                                                             const MeshBooleanOptions<T>& options = {})
{
  return mesh_boolean(meshA, meshB, BooleanOperation::Intersection, options);
}

/** \brief \c mesh_boolean with \c BooleanOperation::Difference: A - B. */
template <typename T, typename TIndex>
GEO_NODISCARD MeshBooleanResult<T, TIndex> mesh_difference(const TriangleHalfedgeMesh<T, 3, TIndex>& meshA,
                                                           const TriangleHalfedgeMesh<T, 3, TIndex>& meshB,
                                                           const MeshBooleanOptions<T>& options = {})
{
  return mesh_boolean(meshA, meshB, BooleanOperation::Difference, options);
}

} // namespace Geometry

#endif // GEOMETRY_MESH_MESHBOOLEAN_HPP
