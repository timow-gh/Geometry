#ifndef GEOMETRY_MESH_MESHNORMALS_HPP
#define GEOMETRY_MESH_MESHNORMALS_HPP

#include "Geometry/Mesh/MeshFaceGeometry.hpp"
#include "Geometry/Mesh/MeshResult.hpp"
#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Utils/Compiler.hpp"

#include <algorithm>
#include <cmath>
#include <concepts>
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
    std::vector<linal::vec3<T>> values; ///< One normal per halfedge; zero for boundary halfedges
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

} // namespace detail

/**
 * \brief Angle-weighted per-corner shading normals for a triangle mesh, respecting crease edges.
 *
 * Call before building render buffers or otherwise shading a surface; mark creases first (e.g. with
 * \c mark_creases_by_angle) so sharp features stay sharp. Each corner averages the face normals of
 * its crease-bounded smooth sector, weighted by corner angle. O(H) with a bounded fan walk per
 * corner.
 *
 * \return Populated \c HalfedgeNormals, or an empty result whose \c error is \c DegenerateGeometry
 * when any face has zero area.
 */
template <std::floating_point T, typename TIndex>
GEO_NODISCARD HalfedgeNormals<T> compute_halfedge_normals(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh)
{
  using Mesh = TriangleHalfedgeMesh<T, 3, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using FaceHandle = typename Mesh::FaceHandle;
  using size_type = typename Mesh::size_type;

  HalfedgeNormals<T> result;
  result.values.assign(mesh.halfedge_count(), linal::vec3<T>{});

  // Precompute each face's flat normal once; corner accumulation reuses them.
  std::vector<linal::vec3<T>> faceNormals(mesh.face_count(), linal::vec3<T>{});
  for (const auto face: mesh.faces())
    if (!detail::mesh_face_normal(mesh, face, faceNormals[static_cast<std::size_t>(face.get_value())]))
      return {{}, MeshNormalStatus::DegenerateGeometry};

  // For each corner (interior halfedge pointing into its vertex), sum angle-weighted face normals
  // over the crease-bounded sector of faces around that vertex. The sector is walked through the
  // outgoing fan around the vertex, stopping at crease edges and boundaries.
  const size_type faceLimit = mesh.face_count();
  for (const auto face: mesh.faces())
  {
    const auto corners = mesh.halfedges_around_face(face);
    for (const HalfedgeHandle corner: corners)
    {
      const auto vertex = mesh.target_vertex(corner);
      linal::vec3<T> accumulated{};

      // Accumulate one face's weighted contribution.
      const auto add_face = [&](FaceHandle here) {
        const T weight = detail::mesh_corner_angle(mesh, here, vertex);
        accumulated =
            linal::vec3<T>{accumulated + weight * faceNormals[static_cast<std::size_t>(here.get_value())]};
      };

      add_face(face);

      // Walk one direction around `vertex` until a crease/boundary stops us or we loop back to the
      // start face (closed smooth fan). Forward crosses the outgoing edge next(current); backward
      // crosses the incoming edge current. The neighbour's interior halfedge pointing into `vertex`
      // becomes the new `current`. Returns true if the walk closed the fan back to the start.
      const auto walk = [&](bool forward) {
        HalfedgeHandle current = corner;
        for (size_type step = 0; step < faceLimit; ++step)
        {
          const HalfedgeHandle bridge = forward ? mesh.get_halfedge(current).next // outgoing edge
                                                : current;                        // incoming edge
          if (mesh.is_crease(mesh.get_halfedge(bridge).edge))
            return false;
          const HalfedgeHandle twin = mesh.get_halfedge(bridge).twin;
          const FaceHandle neighbor = mesh.get_halfedge(twin).face;
          if (!neighbor.is_valid())
            return false; // boundary
          current = (mesh.target_vertex(twin) == vertex) ? twin : mesh.get_halfedge(twin).prev;
          if (current == corner)
            return true; // closed fan, every face already counted
          add_face(neighbor);
        }
        return false;
      };

      // Backward only when the forward sweep did not already close the fan, else faces double-count.
      if (!walk(true))
        walk(false);

      const T length = linal::length(accumulated);
      const auto index = static_cast<std::size_t>(corner.get_value());
      result.values[index] = length == T{0}
                                 ? faceNormals[static_cast<std::size_t>(face.get_value())]
                                 : linal::vec3<T>{accumulated / length};
    }
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
    linal::vec3<T> normalA{}, normalB{};
    if (!detail::mesh_face_normal(mesh, faceA, normalA) || !detail::mesh_face_normal(mesh, faceB, normalB))
      continue; // degenerate face: leave the edge as-is
    mesh.set_crease(edge, linal::dot(normalA, normalB) < cosThreshold);
  }
}

} // namespace Geometry

#endif // GEOMETRY_MESH_MESHNORMALS_HPP
