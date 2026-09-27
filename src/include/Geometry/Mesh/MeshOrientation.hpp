#ifndef GEOMETRY_MESH_MESHORIENTATION_HPP
#define GEOMETRY_MESH_MESHORIENTATION_HPP

#include "Geometry/Mesh/MeshVerify.hpp"
#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Utils/Compiler.hpp"

#include <cstdint>
#include <optional>

#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>

namespace Geometry
{

/**
 * \brief Verifies the surface is consistently wound: every shared edge is traversed in opposite
 * directions by the two faces meeting there (equivalently, the surface is orientable and coherently
 * oriented).
 *
 * Purely combinatorial and boundary-agnostic, so it applies to open and closed meshes alike.
 *
 * \return \c true if no two adjacent faces disagree on the direction of their shared edge.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD bool is_consistently_oriented(const TriangleHalfedgeMesh<T, D, TIndex>& mesh)
{
  using Mesh           = TriangleHalfedgeMesh<T, D, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;

  for (const HalfedgeHandle halfedge : mesh.halfedges())
  {
    const auto& record = mesh.get_halfedge(halfedge);
    if (record.is_boundary())
    {
      continue; // boundary edge: no opposing face to disagree with
    }

    const HalfedgeHandle twin = record.twin;
    // A consistently oriented interior edge has its two directions on two different faces. The twin
    // sharing this halfedge's face (or twinning back to a different halfedge) means the winding of the
    // two incident triangles clashes along the edge.
    if (mesh.get_halfedge(twin).twin != halfedge || mesh.get_halfedge(twin).face == record.face)
    {
      return false;
    }
  }
  return true;
}

/** \brief Global winding of a closed surface relative to its enclosed volume. */
enum class MeshOrientation : std::uint8_t {
  // Face normals point away from the enclosed volume (positive signed volume).
  Outward,
  // Face normals point into the enclosed volume; flip every winding to correct.
  Inward,
  // Not a closed volume (open or empty), so "outward" has no meaning.
  Undefined
};

/**
 * \brief Classifies whether a closed triangle mesh's faces wind outward or inward, from the sign of
 * the volume it encloses.
 *
 * The reliable answer to "do my normals point outwards?" -- computed globally from the signed volume
 * (sum of origin tetrahedra), so it needs no interior reference point. Returns \c Undefined for any
 * mesh with a boundary or no faces, where the question is ill-posed; check \c verify_closed yourself
 * only if you want to distinguish "open" from the other outcomes.
 *
 * Preconditions are ASSUMED, not verified: the mesh should be manifold and consistently oriented
 * (see \c verify_manifold, \c is_consistently_oriented) for the sign to describe the whole surface.
 * O(F).
 *
 * \return \c Outward or \c Inward for a closed mesh; \c Undefined when open, empty, or of zero volume.
 */
template <typename T, typename TIndex>
GEO_NODISCARD MeshOrientation mesh_orientation(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh)
{
  if (mesh.face_count() == 0 || !verify_closed(mesh))
  {
    return MeshOrientation::Undefined;
  }

  T signedVolume{0};
  for (const auto face : mesh.faces())
  {
    const auto handles = mesh.vertices_around_face(face);
    const linal::vec3<T>& first  = mesh.get_vertex(handles[0]).position;
    const linal::vec3<T>& second = mesh.get_vertex(handles[1]).position;
    const linal::vec3<T>& third  = mesh.get_vertex(handles[2]).position;
    signedVolume += linal::dot(first, linal::cross(second, third)) / T{6};
  }

  if (signedVolume > T{0})
  {
    return MeshOrientation::Outward;
  }
  if (signedVolume < T{0})
  {
    return MeshOrientation::Inward;
  }
  return MeshOrientation::Undefined; // zero volume: degenerate, no meaningful side
}

/**
 * \brief Boolean convenience over \c mesh_orientation for callers that only care about outward.
 *
 * \return \c true if outward, \c false if inward, \c std::nullopt when the orientation is undefined
 * (open, empty, or zero-volume mesh).
 */
template <typename T, typename TIndex>
GEO_NODISCARD std::optional<bool> is_outward_oriented(const TriangleHalfedgeMesh<T, 3, TIndex>& mesh)
{
  switch (mesh_orientation(mesh))
  {
  case MeshOrientation::Outward:
    return true;
  case MeshOrientation::Inward:
    return false;
  case MeshOrientation::Undefined:
    break;
  }
  return std::nullopt;
}

} // namespace Geometry

#endif // GEOMETRY_MESH_MESHORIENTATION_HPP
