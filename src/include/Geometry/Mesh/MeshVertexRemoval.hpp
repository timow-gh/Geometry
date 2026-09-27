#ifndef GEOMETRY_MESH_MESHVERTEXREMOVAL_HPP
#define GEOMETRY_MESH_MESHVERTEXREMOVAL_HPP

#include "Geometry/Mesh/MeshEdgeCollapseChecks.hpp"
#include "Geometry/Mesh/MeshEdgeCollapseStatus.hpp"
#include "Geometry/Mesh/MeshQuality.hpp"
#include "Geometry/Mesh/MeshTopology.hpp"
#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Mesh/detail/EdgeCollapse.hpp"
#include "Geometry/Mesh/detail/FaceGeometry.hpp"
#include "Geometry/Mesh/detail/VertexStar.hpp"
#include "Geometry/PolygonTriangulation.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <linal/vec.hpp>
#include <optional>
#include <vector>

namespace Geometry
{

/**
 * \brief Removes \p vertex by collapsing it into the nearest neighbour that it can legally be merged
 * into without flipping or flattening a face.
 *
 * Vertex removal whose hole is fan-triangulated from the chosen neighbour, i.e. a halfedge collapse
 * (see \c collapse_halfedge) with the target picked here. The shortest edge is the choice: it is
 * cheap, deterministic, and moves the surface least; callers needing another criterion (e.g. a
 * quadric error) should rank the outgoing halfedges themselves and call \c collapse_halfedge.
 * Candidates are rejected by \c check_collapse (the survivor keeps its position); ties go to the first
 * candidate in fan order. An isolated vertex is simply deleted. The removed elements are tombstoned.
 * O(valence(v)^2 * valence(neighbour)).
 *
 * \param limits Geometric limits every candidate must respect; the defaults only reject folded
 * triangles and nearly flat slivers.
 * \return The neighbour that absorbed \p vertex (invalid for a deleted isolated vertex), or the mesh
 * untouched and a status: \c InvalidHandle for a missing or deleted vertex; \c InvertsFaces if some
 * collapse was topologically legal but every legal one breaks the geometry; otherwise the
 * topological reason the last candidate in fan order was rejected.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD CollapseResult<typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle>
remove_vertex(TriangleHalfedgeMesh<T, D, TIndex>& mesh, typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle vertex,
              const MeshGeometryLimits<T>& limits = {})
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using VertexHandle = typename Mesh::VertexHandle;
  using vec_t = typename Mesh::vec_t;

  if (!mesh.is_live(vertex))
  {
    return {VertexHandle{}, CollapseStatus::InvalidHandle};
  }
  if (is_isolated(mesh, vertex))
  {
    mesh.connectivity().mark_deleted(vertex);
    return {VertexHandle{}, CollapseStatus::Ok};
  }

  HalfedgeHandle shortest{};
  T shortestLengthSquared{};
  CollapseStatus lastTopologicalRejection = CollapseStatus::Ok;
  bool foldsOver = false;
  const Mesh& constMesh = mesh;
  for (auto outgoing = constMesh.outgoing_halfedges(vertex).circulator(); outgoing.is_valid(); ++outgoing)
  {
    const HalfedgeHandle candidate = outgoing.get_halfedgehandle();
    const vec_t& targetPosition = mesh.get_position(mesh.target_vertex(candidate));
    const CollapseStatus status = check_collapse(mesh, candidate, targetPosition, limits);
    if (status == CollapseStatus::InvertsFaces)
    {
      foldsOver = true;
      continue;
    }
    if (status != CollapseStatus::Ok)
    {
      lastTopologicalRejection = status;
      continue;
    }
    const T lengthSquared = linal::length_squared(vec_t{targetPosition - mesh.get_position(vertex)});
    if (!shortest.is_valid() || lengthSquared < shortestLengthSquared)
    {
      shortest = candidate;
      shortestLengthSquared = lengthSquared;
    }
  }

  if (!shortest.is_valid())
  {
    return {VertexHandle{}, foldsOver ? CollapseStatus::InvertsFaces : lastTopologicalRejection};
  }
  const VertexHandle survivor = mesh.target_vertex(shortest);
  detail::collapse_halfedge_unchecked(mesh, shortest);
  return {survivor, CollapseStatus::Ok};
}

/**
 * \brief Outcome of \c remove_vertex_retriangulate; every value but \c Ok leaves the mesh untouched.
 */
enum class VertexRemovalStatus
{
  Ok,
  // The vertex is not in the mesh or already deleted.
  InvalidHandle,
  // The vertex's only face is a component on its own; removing it would leave a dangling edge.
  IsolatedTriangle,
  // The vertex is a corner of a closed tetrahedron; the hole's only triangle would duplicate the
  // opposite face.
  Tetrahedron,
  // Every triangulation of the hole needs an edge that already exists elsewhere in the mesh.
  DuplicateEdge,
  // Triangulations exist, but each contains a triangle that is degenerate, faces away from the
  // removed star, folds onto a face outside the hole, or is a sliver beyond the corner limit.
  InvertsFaces,
};

/**
 * \brief Removes \p vertex and its star, and fills the hole with the best-shaped triangulation of
 * the surrounding polygon.
 *
 * The general form of vertex removal: where \c remove_vertex is restricted to fan triangulations
 * (a collapse into one neighbour), this searches all triangulations of the k-gon by dynamic
 * programming and keeps the one minimizing the summed triangle aspect cost (squared edge lengths
 * over area, smallest for equilateral triangles). Triangles that are degenerate, or face away from
 * the removed star's summed area vector, and diagonals that already exist in the mesh are excluded,
 * so the hole is refilled without fold-overs or duplicate edges. \p limits additionally excludes
 * slivers and any triangle folding onto the face outside the hole across a polygon side; two new
 * triangles meeting at a diagonal are held together only by the shared reference direction, since
 * the cost is priced per triangle. A boundary vertex's hole is closed by a new boundary edge between
 * its two boundary neighbours; a boundary vertex with a single face just loses that face. Removes
 * 1 vertex, 3 edges and 2 faces (1, 2, 1 on the boundary), preserving the Euler characteristic, and
 * tombstones the removed elements. An isolated vertex is simply deleted. O(k^3 + k^2 * valence)
 * for valence k.
 *
 * \param limits Geometric limits the new triangles must respect; the defaults only reject folded
 * triangles and nearly flat slivers.
 * \return \c VertexRemovalStatus::Ok after removing, otherwise why the mesh was left untouched.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD VertexRemovalStatus remove_vertex_retriangulate(TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                                              typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle vertex,
                                                              const MeshGeometryLimits<T>& limits = {})
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using FaceHandle = typename Mesh::FaceHandle;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;

  if (!mesh.is_live(vertex))
  {
    return VertexRemovalStatus::InvalidHandle;
  }
  if (is_isolated(mesh, vertex))
  {
    mesh.connectivity().mark_deleted(vertex);
    return VertexRemovalStatus::Ok;
  }

  const detail::VertexStar<Mesh> star = detail::collect_vertex_star(mesh, vertex);
  const std::size_t count = star.ring.size();

  if (star.boundary && star.faces.size() == 1)
  {
    if (mesh.is_boundary(mesh.get_halfedge(star.ringHalfedges.front()).twin))
    {
      return VertexRemovalStatus::IsolatedTriangle;
    }
    detail::remove_ear_vertex_unchecked(mesh, vertex, star);
    return VertexRemovalStatus::Ok;
  }

  if (!star.boundary && count == 3)
  {
    // The hole's single triangle already exists, wound the other way, iff all three outer sides
    // belong to one face.
    const FaceHandle outerFace = mesh.get_halfedge(mesh.get_halfedge(star.ringHalfedges[0]).twin).face;
    const bool sharedOuterFace =
        outerFace.is_valid()
        && std::all_of(star.ringHalfedges.begin(), star.ringHalfedges.end(), [&](HalfedgeHandle ringHalfedge) {
             return mesh.get_halfedge(mesh.get_halfedge(ringHalfedge).twin).face == outerFace;
           });
    if (sharedOuterFace)
    {
      return VertexRemovalStatus::Tetrahedron;
    }
  }

  if (star.boundary && mesh.find_halfedge(star.ring.back(), star.ring.front()).is_valid())
  {
    return VertexRemovalStatus::DuplicateEdge;
  }

  const auto isDiagonalAllowed = [&](std::size_t from, std::size_t to) {
    return !mesh.find_halfedge(star.ring[from], star.ring[to]).is_valid();
  };

  // The removed star's summed orientation is the reference the new triangles must agree with.
  const auto& center = mesh.get_position(vertex);
  auto reference = detail::triangle_orientation(center, mesh.get_position(mesh.source_vertex(star.ringHalfedges[0])),
                                                mesh.get_position(mesh.target_vertex(star.ringHalfedges[0])));
  for (std::size_t i = 1; i < star.ringHalfedges.size(); ++i)
  {
    reference = reference + detail::triangle_orientation(center, mesh.get_position(mesh.source_vertex(star.ringHalfedges[i])),
                                                         mesh.get_position(mesh.target_vertex(star.ringHalfedges[i])));
  }

  // The face outside the hole across the polygon side (from, to), or an invalid handle for a
  // diagonal, the closing side of a boundary vertex, or a side on the mesh boundary.
  const auto outerFace = [&](std::size_t from, std::size_t to) {
    const bool isRingSide = to == from + 1 || (!star.boundary && from == 0 && to == count - 1);
    if (!isRingSide)
    {
      return FaceHandle{};
    }
    const HalfedgeHandle ringHalfedge = star.ringHalfedges[to == from + 1 ? from : count - 1];
    return mesh.get_halfedge(mesh.get_halfedge(ringHalfedge).twin).face;
  };

  const detail::AngleLimit<T> foldLimit{limits.maxFoldAngle};
  const detail::AngleLimit<T> cornerLimit{limits.maxCornerAngle};
  const auto aspectCost = [&](std::size_t first, std::size_t apex, std::size_t last) -> T {
    const auto& firstPosition = mesh.get_position(star.ring[first]);
    const auto& apexPosition = mesh.get_position(star.ring[apex]);
    const auto& lastPosition = mesh.get_position(star.ring[last]);
    const auto orientation = detail::triangle_orientation(firstPosition, apexPosition, lastPosition);
    if (!(detail::orientation_dot(orientation, reference) > T{0})
        || detail::has_corner_wider_than(firstPosition, apexPosition, lastPosition, cornerLimit))
    {
      return std::numeric_limits<T>::infinity();
    }
    for (const auto& [from, to] : {std::array{first, apex}, std::array{apex, last}, std::array{first, last}})
    {
      const FaceHandle outer = outerFace(from, to);
      if (!outer.is_valid())
      {
        continue;
      }
      const auto outerOrientation = detail::face_orientation(mesh, outer);
      if (detail::areas_too_disproportionate(orientation, outerOrientation)
          || detail::triangles_fold(orientation, outerOrientation, foldLimit))
      {
        return std::numeric_limits<T>::infinity();
      }
    }
    using vec_t = typename Mesh::vec_t;
    const T squaredEdges = linal::length_squared(vec_t{apexPosition - firstPosition})
                           + linal::length_squared(vec_t{lastPosition - apexPosition})
                           + linal::length_squared(vec_t{firstPosition - lastPosition});
    return squaredEdges / std::sqrt(detail::orientation_dot(orientation, orientation));
  };

  // The valence is unbounded, so the buffers live on the heap; the fallback search below reuses them.
  std::vector<TriangulationCell<T>> scratch(triangulation_scratch_size(count));
  std::vector<PolygonTriangle> triangles(triangulation_triangle_count(count));
  if (!minimum_cost_triangulation(count, aspectCost, scratch, triangles, isDiagonalAllowed))
  {
    const auto anyCost = [](std::size_t, std::size_t, std::size_t) noexcept { return T{0}; };
    const bool topologicallyPossible = minimum_cost_triangulation(count, anyCost, scratch, triangles, isDiagonalAllowed);
    return topologicallyPossible ? VertexRemovalStatus::InvertsFaces : VertexRemovalStatus::DuplicateEdge;
  }

  detail::retriangulate_star(mesh, vertex, star, triangles);
  return VertexRemovalStatus::Ok;
}

} // namespace Geometry

#endif // GEOMETRY_MESH_MESHVERTEXREMOVAL_HPP
