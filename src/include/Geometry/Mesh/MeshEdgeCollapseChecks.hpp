#ifndef GEOMETRY_MESH_MESHEDGECOLLAPSECHECKS_HPP
#define GEOMETRY_MESH_MESHEDGECOLLAPSECHECKS_HPP

#include "Geometry/Mesh/MeshEdgeCollapseStatus.hpp"
#include "Geometry/Mesh/MeshQuality.hpp"
#include "Geometry/Mesh/MeshTopology.hpp"
#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Mesh/detail/EdgeCollapseChecks.hpp"
#include "Geometry/Mesh/detail/FaceGeometry.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>

namespace Geometry
{

/**
 * \brief Checks whether collapsing \p halfedge (p -> q, removing p) keeps the mesh a manifold of
 * unchanged topology.
 *
 * Purely topological, following Hoppe et al. 93: the boundary rule, the link condition, and the two
 * degenerate small components the link condition alone misses (an isolated triangle and a closed
 * tetrahedron). Geometric validity (fold-overs) is a separate question, see
 * \c check_collapse. Symmetric: p -> q and q -> p give the same answer, since both yield the
 * same connectivity. O(valence(p) * valence(q)).
 *
 * \return \c CollapseStatus::Ok if the collapse is legal, otherwise the first violated rule.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD CollapseStatus is_collapse_ok(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                            typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle halfedge)
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using VertexHandle = typename Mesh::VertexHandle;

  if (!mesh.is_live(halfedge))
  {
    return CollapseStatus::InvalidHandle;
  }

  const HalfedgeHandle opposite = mesh.get_halfedge(halfedge).twin;
  for (const HalfedgeHandle side : {halfedge, opposite})
  {
    if (!mesh.is_boundary(side) && detail::is_isolated_face(mesh, mesh.get_halfedge(side).face))
    {
      return CollapseStatus::IsolatedTriangle;
    }
  }

  const VertexHandle removed = mesh.source_vertex(halfedge);
  const VertexHandle survivor = mesh.target_vertex(halfedge);
  if (is_boundary(mesh, removed) && is_boundary(mesh, survivor) && !mesh.is_boundary(mesh.get_halfedge(halfedge).edge))
  {
    return CollapseStatus::InteriorEdgeBetweenBoundaryVertices;
  }

  if (!detail::satisfies_link_condition(mesh, halfedge))
  {
    return CollapseStatus::LinkCondition;
  }

  if (detail::is_tetrahedron_apex(mesh, detail::opposite_vertex(mesh, halfedge))
      || detail::is_tetrahedron_apex(mesh, detail::opposite_vertex(mesh, opposite)))
  {
    return CollapseStatus::Tetrahedron;
  }

  return CollapseStatus::Ok;
}

/**
 * \brief Whether collapsing \p halfedge (p -> q) and placing the merged vertex at \p position would
 * flip or flatten any surviving face.
 *
 * The geometric companion to the purely topological \c is_collapse_ok: a legal collapse can still
 * fold the surface over itself. Checks every face around p and q except the one or two removed with
 * the edge. Pass q's current position for a halfedge collapse. Exact comparisons, no tolerance.
 * O(valence(p) + valence(q)).
 *
 * \return \c true if some surviving face would flip or become degenerate.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD bool collapse_inverts_faces(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                          typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle halfedge,
                                          const typename TriangleHalfedgeMesh<T, D, TIndex>::vec_t& position)
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using VertexHandle = typename Mesh::VertexHandle;
  using FaceHandle = typename Mesh::FaceHandle;
  using vec_t = typename Mesh::vec_t;

  GEO_ASSERT(mesh.is_live(halfedge));
  const VertexHandle removed = mesh.source_vertex(halfedge);
  const VertexHandle survivor = mesh.target_vertex(halfedge);
  const FaceHandle leftFace = mesh.get_halfedge(halfedge).face;
  const FaceHandle rightFace = mesh.get_halfedge(mesh.get_halfedge(halfedge).twin).face;

  const auto inverts = [&](FaceHandle face) {
    if (face == leftFace || face == rightFace)
    {
      return false;
    }
    const auto corners = mesh.vertices_around_face(face);
    std::array<vec_t, 3> before{};
    std::array<vec_t, 3> after{};
    for (std::size_t i = 0; i < 3; ++i)
    {
      before[i] = mesh.get_position(corners[i]);
      after[i] = (corners[i] == removed || corners[i] == survivor) ? position : before[i];
    }
    return detail::triangle_inverts(before, after);
  };

  for (const VertexHandle endpoint : {removed, survivor})
  {
    for (auto face = mesh.faces(endpoint).circulator(); face.is_valid(); ++face)
    {
      if (inverts(face.get_facehandle()))
      {
        return true;
      }
    }
  }
  return false;
}

/**
 * \brief Whether collapsing \p halfedge (p -> q) with the merged vertex at \p position would fold a
 * triangle onto a neighbour or leave a nearly flat sliver, per \p limits.
 *
 * Complements \c collapse_inverts_faces, which compares each reshaped face only with its own former
 * self and so accepts any turn below 90 degrees. On a curved surface such turns let a triangle end up
 * lying back on the face next to it, or standing edge-on across the surface. This check looks at the
 * result instead: every reshaped triangle is compared with each triangle it will share an edge with,
 * and its corner angles are bounded. Pairs whose areas differ too much for their normals to be
 * compared reliably are rejected as well. Local by nature: it cannot see parts of the surface that
 * are not adjacent, so it does not rule out self-intersection. No allocation.
 * O(valence(p) + valence(q)).
 *
 * \return \c true if some reshaped triangle would exceed \p limits.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD bool collapse_exceeds_geometry_limits(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                                    typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle halfedge,
                                                    const typename TriangleHalfedgeMesh<T, D, TIndex>::vec_t& position,
                                                    const MeshGeometryLimits<T>& limits = {})
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using VertexHandle = typename Mesh::VertexHandle;
  using FaceHandle = typename Mesh::FaceHandle;
  using vec_t = typename Mesh::vec_t;

  GEO_ASSERT(mesh.is_live(halfedge));
  const VertexHandle removed = mesh.source_vertex(halfedge);
  const VertexHandle survivor = mesh.target_vertex(halfedge);
  const FaceHandle leftFace = mesh.get_halfedge(halfedge).face;
  const FaceHandle rightFace = mesh.get_halfedge(mesh.get_halfedge(halfedge).twin).face;
  const detail::AngleLimit<T> foldLimit{limits.maxFoldAngle};
  const detail::AngleLimit<T> cornerLimit{limits.maxCornerAngle};

  const auto corners_after = [&](FaceHandle face) {
    const auto corners = mesh.vertices_around_face(face);
    std::array<vec_t, 3> positions{};
    for (std::size_t i = 0; i < 3; ++i)
    {
      positions[i] = (corners[i] == removed || corners[i] == survivor) ? position : mesh.get_position(corners[i]);
    }
    return positions;
  };
  const auto orientation_after = [&](FaceHandle face) {
    const auto positions = corners_after(face);
    return detail::triangle_orientation(positions[0], positions[1], positions[2]);
  };

  const auto exceeds = [&](FaceHandle face) {
    if (face == leftFace || face == rightFace)
    {
      return false;
    }
    const auto positions = corners_after(face);
    if (detail::has_corner_wider_than(positions[0], positions[1], positions[2], cornerLimit))
    {
      return true;
    }
    const auto orientation = detail::triangle_orientation(positions[0], positions[1], positions[2]);
    for (const auto side : mesh.halfedges_around_face(face))
    {
      const FaceHandle neighbour = detail::face_across_after_collapse(mesh, halfedge, side);
      if (!neighbour.is_valid())
      {
        continue;
      }
      const auto neighbourOrientation = orientation_after(neighbour);
      if (detail::areas_too_disproportionate(orientation, neighbourOrientation)
          || detail::triangles_fold(orientation, neighbourOrientation, foldLimit))
      {
        return true;
      }
    }
    return false;
  };

  for (const VertexHandle endpoint : {removed, survivor})
  {
    for (auto face = mesh.faces(endpoint).circulator(); face.is_valid(); ++face)
    {
      if (exceeds(face.get_facehandle()))
      {
        return true;
      }
    }
  }
  return false;
}

/**
 * \brief Checks whether collapsing \p halfedge (p -> q) with the merged vertex at \p position keeps
 * the mesh a manifold of unchanged topology and respects the geometry.
 *
 * The one check the safe collapses run, exposed so a decimation driver can rank candidates by its own
 * cost and only consider the valid ones: \c is_collapse_ok first, then \c collapse_inverts_faces and
 * \c collapse_exceeds_geometry_limits. Pass q's current position for a halfedge collapse.
 * O(valence(p) * valence(q)).
 *
 * \return \c CollapseStatus::Ok if the collapse is valid, the violated topological rule, or
 * \c CollapseStatus::InvertsFaces if only the geometry forbids it.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD CollapseStatus check_collapse(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                            typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle halfedge,
                                            const typename TriangleHalfedgeMesh<T, D, TIndex>::vec_t& position,
                                            const MeshGeometryLimits<T>& limits = {})
{
  const CollapseStatus status = is_collapse_ok(mesh, halfedge);
  if (status != CollapseStatus::Ok)
  {
    return status;
  }
  if (collapse_inverts_faces(mesh, halfedge, position) || collapse_exceeds_geometry_limits(mesh, halfedge, position, limits))
  {
    return CollapseStatus::InvertsFaces;
  }
  return CollapseStatus::Ok;
}

} // namespace Geometry

#endif // GEOMETRY_MESH_MESHEDGECOLLAPSECHECKS_HPP
