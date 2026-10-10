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
 * \brief Both meshes refined by \c corefine, with the intersection curve as edges of each, or the
 * reason corefinement failed.
 *
 * The meshes are handed back on success and on failure, since \c corefine consumes its operands.
 *
 * Edge i of A and edge i of B are the same piece of the curve: their endpoints have identical
 * positions. Where the surfaces overlap in a plane, every edge inside the overlap is listed, not only
 * its outline. A reported failure always carries empty lists.
 */
template <typename T, typename TIndex>
struct CorefineResult
{
  using Mesh = TriangleHalfedgeMesh<T, 3, TIndex>;
  using EdgeHandle = typename Mesh::EdgeHandle;

  Mesh meshA;
  Mesh meshB;
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
 * edge, so each refined face still lies within one original face and the shapes are unchanged up to
 * the rounding of the inserted points. Every handle of an operand stays valid in its refined mesh.
 * The meshes need not be closed. Intersections within one mesh are not looked for.
 *
 * The operands are consumed, so that no mesh is copied unless the caller asks for it: move a mesh in
 * when it is no longer needed, or pass an explicit copy (\c Mesh{mesh}) to keep it. Allocation
 * failures propagate as exceptions.
 * O(F log F + k log k + sum over faces of (points in the face)^2 + flips) for k intersection points.
 *
 * \pre \p meshA and \p meshB are distinct objects.
 * \return The refined meshes and their intersection edges, or the meshes and the reason for the
 * failure. \c NonFiniteCoordinates, \c DegenerateFace (a zero-area face anywhere in either mesh) and
 * \c DegenerateIntersection from two intersection points that round to one position are found before
 * any change, so the meshes come back unchanged. \c DegenerateIntersection from floating-point
 * predicates that contradict each other can stop the refinement midway; the meshes then come back
 * partly refined, with valid connectivity.
 */
template <typename T, typename TIndex>
GEO_NODISCARD CorefineResult<T, TIndex> corefine(TriangleHalfedgeMesh<T, 3, TIndex>&& meshA, TriangleHalfedgeMesh<T, 3, TIndex>&& meshB)
{
  using Mesh = TriangleHalfedgeMesh<T, 3, TIndex>;
  using Result = CorefineResult<T, TIndex>;

  GEO_ASSERT(&meshA != &meshB);
  // Moved from on every path, so the caller never has to guess whether an operand is still intact.
  Mesh refinedA = std::move(meshA);
  Mesh refinedB = std::move(meshB);
  detail::CorefinementResult<T, TIndex> corefinement = detail::corefine_in_place(refinedA, refinedB);
  if (!corefinement.has_value())
  {
    return Result{std::move(refinedA), std::move(refinedB), {}, {}, corefinement.error};
  }
  return Result{std::move(refinedA),
                std::move(refinedB),
                std::move(corefinement.corefinement.onMeshA.edgeOfSegment),
                std::move(corefinement.corefinement.onMeshB.edgeOfSegment),
                CorefineStatus::Ok};
}

} // namespace Geometry

#endif // GEOMETRY_MESH_MESHCOREFINE_HPP
