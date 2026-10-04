#ifndef GEOMETRY_MESH_MESHNORMALS_HPP
#define GEOMETRY_MESH_MESHNORMALS_HPP

#include "Geometry/Mesh/detail/FaceGeometry.hpp"
#include "Geometry/Mesh/detail/MeshResult.hpp"
#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <vector>

#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>

namespace Geometry
{

/** \brief Outcome reported by \c compute_halfedge_normals; Ok on success. */
enum class MeshNormalStatus {
    Ok,
    DegenerateGeometry
};

/**
 * \brief Per-corner shading normals indexed by interior-halfedge value; empty \c values when
 * \c error is set.
 *
 * A corner's normal is the angle-weighted average of the face normals in the crease-bounded sector
 * around that corner's vertex, so corners in one smooth sector share a normal (smooth shading) while
 * corners separated by a crease differ (sharp shading). Boundary halfedges carry no corner and stay
 * zero.
 */
template <std::floating_point T>
struct HalfedgeNormals {
    // One normal per halfedge; zero for boundary halfedges.
    std::vector<linal::vec3<T>> values;
    MeshNormalStatus error = MeshNormalStatus::Ok;

    GEO_NODISCARD bool has_value() const noexcept { return detail::mesh_result_ok(error); }
    GEO_NODISCARD explicit operator bool() const noexcept { return has_value(); }
};

namespace detail
{

/**
 * \internal
 * \brief Interior corner angle at \p vertex inside \p face -- the angle between the two triangle
 * edges meeting there.
 *
 * Used as the smooth-average weight so uneven tessellation contributes proportionally rather than
 * by raw face count.
 */
template <typename T, typename TIndex>
GEO_NODISCARD T mesh_corner_angle(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh,
                                  typename TriangleHalfedgeMesh<T, 3, TIndex>::FaceHandle face,
                                  typename TriangleHalfedgeMesh<T, 3, TIndex>::VertexHandle vertex) noexcept
{
  const auto handles = mesh.vertices_around_face(face);
  int corner = 0;
  for (int i = 0; i < 3; ++i)
    if (handles[static_cast<std::size_t>(i)] == vertex)
      corner = i;
  const auto& apex = mesh.get_vertex(vertex).position;
  const linal::vec3<T> lhs{mesh.get_vertex(handles[static_cast<std::size_t>((corner + 1) % 3)]).position - apex};
  const linal::vec3<T> rhs{mesh.get_vertex(handles[static_cast<std::size_t>((corner + 2) % 3)]).position - apex};
  const T lengths = linal::length(lhs) * linal::length(rhs);
  if (lengths == T{0})
    return T{0};
  const T cosine = std::clamp(linal::dot(lhs, rhs) / lengths, T{-1}, T{1});
  return std::acos(cosine);
}

/**
 * \internal
 * \brief Whether crossing \p outgoing's edge leaves the smooth sector around its source vertex.
 *
 * Crease and boundary edges bound sectors; a boundary outgoing halfedge lies on a boundary edge, so
 * it always counts as a break and never sits inside a sector.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD bool is_sector_break(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                   typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle outgoing) noexcept
{
  const auto edge = mesh.get_halfedge(outgoing).edge;
  return mesh.is_crease(edge) || mesh.is_boundary(edge);
}

/**
 * \internal
 * \brief An outgoing halfedge of \p vertex at which a smooth sector begins, or an invalid handle for
 * an isolated vertex.
 *
 * Starting every walk at a sector boundary lets one pass around the fan see each sector whole,
 * instead of wrapping one sector across the walk's start. A fan without breaks is a single closed
 * sector, and any outgoing halfedge starts it. O(valence).
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle
sector_walk_start(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                  typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle vertex) noexcept
{
  using HalfedgeHandle = typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle;

  const HalfedgeHandle stored = mesh.get_vertex(vertex).halfedge;
  if (!stored.is_valid())
  {
    return stored;
  }
  HalfedgeHandle outgoing = stored;
  do
  {
    if (is_sector_break(mesh, outgoing))
    {
      return mesh.next_in_outgoing_fan(outgoing);
    }
    outgoing = mesh.next_in_outgoing_fan(outgoing);
  } while (outgoing != stored);
  return stored;
}

/**
 * \internal
 * \brief Calls \p onSector(begin, end) for each crease-bounded smooth sector around \p vertex that
 * holds at least one face.
 *
 * A sector is the outgoing halfedges from \c begin up to (excluding) \c end in fan order;
 * \c begin == \c end denotes a whole closed fan. Visit its corners with \c for_each_sector_corner.
 * Every consumer of per-sector data walks sectors through this one definition, so they agree on
 * which corners share a sector. O(valence).
 */
template <typename T, std::uint8_t D, typename TIndex, typename TOnSector>
void for_each_smooth_sector(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                            typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle vertex,
                            TOnSector&& onSector)
{
  using HalfedgeHandle = typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle;

  const HalfedgeHandle start = sector_walk_start(mesh, vertex);
  if (!start.is_valid())
  {
    return;
  }
  HalfedgeHandle sectorBegin = start;
  HalfedgeHandle outgoing = start;
  do
  {
    const HalfedgeHandle following = mesh.next_in_outgoing_fan(outgoing);
    if (is_sector_break(mesh, outgoing) || following == start)
    {
      // A sector opening on a boundary halfedge is that halfedge alone: the gap of an open fan.
      if (!mesh.is_boundary(sectorBegin))
      {
        onSector(sectorBegin, following);
      }
      sectorBegin = following;
    }
    outgoing = following;
  } while (outgoing != start);
}

/**
 * \internal
 * \brief Calls \p onCorner(corner) for each corner of the sector [\p sectorBegin, \p sectorEnd)
 * reported by \c for_each_smooth_sector.
 *
 * A corner is the interior halfedge pointing into the sector's vertex, which is how per-corner
 * data (normals, render ids) is indexed.
 */
template <typename T, std::uint8_t D, typename TIndex, typename TOnCorner>
void for_each_sector_corner(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                            typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle sectorBegin,
                            typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle sectorEnd,
                            TOnCorner&& onCorner)
{
  auto outgoing = sectorBegin;
  do
  {
    GEO_ASSERT(!mesh.is_boundary(outgoing));
    onCorner(mesh.get_halfedge(outgoing).prev);
    outgoing = mesh.next_in_outgoing_fan(outgoing);
  } while (outgoing != sectorEnd);
}

} // namespace detail

/**
 * \brief Angle-weighted per-corner shading normals for a triangle mesh, respecting crease edges.
 *
 * Call before building render buffers or otherwise shading a surface; mark creases first (e.g. with
 * \c mark_creases_by_angle) so sharp features stay sharp. Each crease-bounded smooth sector gets one
 * normal, the corner-angle-weighted average of its face normals, computed once and shared by all of
 * its corners -- so corners in a sector are bitwise identical and a renderer can weld them. O(H).
 *
 * \return Populated \c HalfedgeNormals, or an empty result whose \c error is \c DegenerateGeometry
 * when any face has zero area.
 */
template <std::floating_point T, typename TIndex>
GEO_NODISCARD HalfedgeNormals<T> compute_halfedge_normals(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh)
{
  using Mesh = TriangleHalfedgeMesh<T, 3, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using VertexHandle = typename Mesh::VertexHandle;

  HalfedgeNormals<T> result;
  result.values.assign(mesh.halfedge_storage_size(), linal::vec3<T>{});

  // Precompute each face's flat normal once; every sector sum reuses them.
  std::vector<linal::vec3<T>> faceNormals(mesh.face_storage_size(), linal::vec3<T>{});
  for (const auto face: mesh.faces())
  {
    const auto normal = detail::mesh_face_normal(mesh, face);
    if (!normal)
      return {{}, MeshNormalStatus::DegenerateGeometry};
    faceNormals[static_cast<std::size_t>(face.get_value())] = *normal;
  }
  const auto face_normal_of = [&](HalfedgeHandle corner) -> const linal::vec3<T>& {
    return faceNormals[static_cast<std::size_t>(mesh.get_halfedge(corner).face.get_value())];
  };

  for (const VertexHandle vertex: mesh.vertices())
  {
    detail::for_each_smooth_sector(mesh, vertex, [&](HalfedgeHandle sectorBegin, HalfedgeHandle sectorEnd) {
      linal::vec3<T> accumulated{};
      detail::for_each_sector_corner(mesh, sectorBegin, sectorEnd, [&](HalfedgeHandle corner) {
        const T weight = detail::mesh_corner_angle(mesh, mesh.get_halfedge(corner).face, vertex);
        accumulated = linal::vec3<T>{accumulated + weight * face_normal_of(corner)};
      });

      // Opposing faces can cancel to zero; each corner then keeps its own flat normal.
      const T length = linal::length(accumulated);
      const linal::vec3<T> sectorNormal{accumulated / (length == T{0} ? T{1} : length)};
      detail::for_each_sector_corner(mesh, sectorBegin, sectorEnd, [&](HalfedgeHandle corner) {
        result.values[static_cast<std::size_t>(corner.get_value())] =
            length == T{0} ? face_normal_of(corner) : sectorNormal;
      });
    });
  }

  return result;
}

/**
 * \brief Marks every edge whose dihedral angle exceeds \p creaseAngle as a crease, so shading later
 * treats it as sharp.
 *
 * General-purpose and independent of any factory default crease set: run it to derive creases purely
 * from geometry. Boundary edges are always marked as creases; degenerate incident faces leave the
 * edge untouched.
 *
 * \param creaseAngle Threshold dihedral angle in radians.
 */
template <std::floating_point T, typename TIndex>
void mark_creases_by_angle(TriangleHalfedgeMesh<T, 3, TIndex>& mesh, T creaseAngle)
{
  const T cosThreshold = std::cos(creaseAngle);
  for (const auto edge: mesh.edges())
  {
    const auto halfedge = mesh.get_edge(edge).halfedge;
    const auto twin = mesh.get_halfedge(halfedge).twin;
    const auto faceA = mesh.get_halfedge(halfedge).face;
    const auto faceB = mesh.get_halfedge(twin).face;
    if (!faceA.is_valid() || !faceB.is_valid())
    {
      mesh.set_crease(edge, true); // boundary edge
      continue;
    }
    const auto normalA = detail::mesh_face_normal(mesh, faceA);
    const auto normalB = detail::mesh_face_normal(mesh, faceB);
    if (!normalA || !normalB)
      continue; // degenerate face: leave the edge as-is
    mesh.set_crease(edge, linal::dot(*normalA, *normalB) < cosThreshold);
  }
}

} // namespace Geometry

#endif // GEOMETRY_MESH_MESHNORMALS_HPP
