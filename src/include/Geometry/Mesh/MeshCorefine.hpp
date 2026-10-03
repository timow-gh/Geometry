#ifndef GEOMETRY_MESH_MESHCOREFINE_HPP
#define GEOMETRY_MESH_MESHCOREFINE_HPP

#include "Geometry/Mesh/MeshCorefineStatus.hpp"
#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Mesh/detail/Corefine.hpp"
#include "Geometry/Mesh/detail/MeshResult.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"

#include <utility>
#include <vector>

namespace Geometry
{

/**
 * \brief Intersection curve found by \c corefine, as edges of each refined mesh, or the reason
 * corefinement failed.
 *
 * Edge i of A and edge i of B are the same piece of the curve: their endpoints have identical
 * positions. A reported failure always carries empty lists.
 */
template <typename T, typename TIndex>
struct CorefineResult
{
  using EdgeHandle = typename TriangleHalfedgeMesh<T, 3, TIndex>::EdgeHandle;

  std::vector<EdgeHandle> intersectionEdgesA;
  std::vector<EdgeHandle> intersectionEdgesB;
  CorefineStatus error = CorefineStatus::Ok;

  GEO_NODISCARD bool has_value() const noexcept { return detail::mesh_result_ok(error); }
  GEO_NODISCARD explicit operator bool() const noexcept { return has_value(); }
};

/**
 * \brief Refines \p meshA and \p meshB so that the curve where their surfaces meet runs along edges
 * of both, e.g. to extract intersection curves or as the first stage of a mesh Boolean.
 *
 * Every point where the surfaces meet becomes a vertex of both meshes, at the same position:
 * crossings, touch points and the corners of coplanar overlaps. Faces and edges are split where those
 * points lie, and edges are flipped inside an original face until every piece of the curve is an
 * edge, so each refined face still lies within one original face and the shapes are unchanged. Every
 * existing handle stays valid. The meshes need not be closed.
 *
 * Works on copies and assigns them only on success, so a failure leaves both meshes untouched; this
 * costs O(V + F) extra. Allocation failures propagate as exceptions.
 * O(F log F + k log k + sum over faces of (points in the face)^2 + flips) for k intersection points.
 *
 * \pre \p meshA and \p meshB are distinct objects.
 * \return The intersection edges of each mesh, or the reason the meshes were left untouched:
 * \c NonFiniteCoordinates, \c DegenerateFace (zero area), or \c DegenerateIntersection when the
 * floating-point predicates contradict each other or two intersection points round to one position.
 */
template <typename T, typename TIndex>
GEO_NODISCARD CorefineResult<T, TIndex> corefine(TriangleHalfedgeMesh<T, 3, TIndex>& meshA, TriangleHalfedgeMesh<T, 3, TIndex>& meshB)
{
  using Mesh = TriangleHalfedgeMesh<T, 3, TIndex>;
  using Result = CorefineResult<T, TIndex>;

  GEO_ASSERT(&meshA != &meshB);
  Mesh refinedA = meshA;
  Mesh refinedB = meshB;
  detail::CorefinementResult<T, TIndex> corefinement = detail::corefine_in_place(refinedA, refinedB);
  if (!corefinement.has_value())
  {
    return Result{{}, {}, corefinement.error};
  }
  meshA = std::move(refinedA);
  meshB = std::move(refinedB);
  return Result{std::move(corefinement.corefinement.onMeshA.edgeOfSegment),
                std::move(corefinement.corefinement.onMeshB.edgeOfSegment),
                CorefineStatus::Ok};
}

} // namespace Geometry

#endif // GEOMETRY_MESH_MESHCOREFINE_HPP
