#ifndef GEOMETRY_MESH_MESHEULER_HPP
#define GEOMETRY_MESH_MESHEULER_HPP

#include "Geometry/Mesh/MeshManifold.hpp"
#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace Geometry
{

// Returns the Euler characteristic chi = V - E + F. Signed because a mesh with boundary or several
// components can have chi larger than 2.
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD std::ptrdiff_t euler_characteristic(const TriangleHalfedgeMesh<T, D, TIndex>& mesh) noexcept
{
  const auto vertexCount = static_cast<std::ptrdiff_t>(mesh.vertex_count());
  const auto edgeCount = static_cast<std::ptrdiff_t>(mesh.edge_count());
  const auto faceCount = static_cast<std::ptrdiff_t>(mesh.face_count());
  return vertexCount - edgeCount + faceCount;
}

// Enumerates the boundary loops of the mesh. Each loop is the ordered list of boundary halfedges
// (halfedges with no incident face) forming one closed boundary cycle. A closed (watertight) mesh
// returns an empty result.
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD std::vector<std::vector<typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle>>
boundary_loops(const TriangleHalfedgeMesh<T, D, TIndex>& mesh)
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;

  std::vector<std::vector<HalfedgeHandle>> loops;
  std::vector<unsigned char> visited(mesh.halfedge_count(), 0U);

  for (const HalfedgeHandle start : mesh.halfedges())
  {
    if (visited[start.get_value()] != 0U || !mesh.is_boundary(start))
    {
      continue;
    }

    std::vector<HalfedgeHandle> loop;
    // A well-formed boundary loop chains boundary halfedges through .next and closes within the
    // halfedge count. On a mesh built through the raw connectivity view a boundary halfedge's .next
    // may leave the boundary or never return to `start`; the step budget and the boundary guard make
    // the walk terminate rather than hang (the loop contents on such corrupt input are unspecified).
    const std::size_t limit = mesh.halfedge_count();
    std::size_t steps = 0;
    HalfedgeHandle current = start;
    do
    {
      visited[current.get_value()] = 1U;
      loop.push_back(current);
      current = mesh.get_halfedge(current).next;
    } while (current != start && ++steps <= limit && mesh.is_boundary(current));

    loops.push_back(std::move(loop));
  }

  return loops;
}

// Returns the number of connected components (flood fill over faces across shared, non-boundary
// edges). Isolated vertices are ignored; a mesh with no faces has zero components.
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD std::size_t num_connected_components(const TriangleHalfedgeMesh<T, D, TIndex>& mesh)
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using FaceHandle = typename Mesh::FaceHandle;

  std::vector<unsigned char> visited(mesh.face_count(), 0U);
  std::size_t componentCount = 0;
  std::vector<FaceHandle> stack;

  for (const FaceHandle seed : mesh.faces())
  {
    if (visited[seed.get_value()] != 0U)
    {
      continue;
    }

    ++componentCount;
    stack.push_back(seed);
    visited[seed.get_value()] = 1U;

    while (!stack.empty())
    {
      const FaceHandle face = stack.back();
      stack.pop_back();

      for (auto circulator = mesh.adjacent_faces(face).circulator(); circulator.is_valid(); ++circulator)
      {
        const FaceHandle neighbour = circulator.get_facehandle();
        if (visited[neighbour.get_value()] == 0U)
        {
          visited[neighbour.get_value()] = 1U;
          stack.push_back(neighbour);
        }
      }
    }
  }

  return componentCount;
}

// Returns the genus of the surface, or std::nullopt when the formula does not yield a non-negative
// integer.
//
// For a connected, orientable, manifold surface with b boundary loops:
//   V - E + F = 2 - 2g - b   =>   g = (2 - b - chi) / 2
// Genus counts handles and is defined for surfaces with boundary as well as closed ones (a disc
// has genus 0). Preconditions (ASSUMED, not checked at runtime): the mesh is manifold and connected
// (exactly one component); the halfedge kernel is orientable by construction. Verify manifoldness with
// verify_manifold() and connectedness with num_connected_components() beforehand if unsure -- genus
// assumes both, like every other mesh algorithm, and only guards its own arithmetic.
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD std::optional<std::size_t> genus(const TriangleHalfedgeMesh<T, D, TIndex>& mesh)
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using VertexHandle = typename Mesh::VertexHandle;

  // Preconditions are assumed; the assertions catch a violated caller in debug builds and compile out
  // in release, so genus stays free of the manifold/connectivity scans it would otherwise pay for.
  GEO_ASSERT(num_connected_components(mesh) == 1);
  GEO_ASSERT(verify_manifold(mesh));

  // Count only vertices that belong to the surface. Isolated vertices are manifold-legal (add_vertex
  // creates them) but are ignored by num_connected_components and verify_manifold, so the Euler
  // characteristic must ignore them too or a stray isolated vertex would inflate chi by one and skew
  // the genus. The public euler_characteristic() stays the literal V - E + F.
  std::ptrdiff_t usedVertices = 0;
  for (const VertexHandle vertex : mesh.vertices())
  {
    if (mesh.get_vertex(vertex).halfedge.is_valid())
    {
      ++usedVertices;
    }
  }

  const std::ptrdiff_t chi = usedVertices
                           - static_cast<std::ptrdiff_t>(mesh.edge_count())
                           + static_cast<std::ptrdiff_t>(mesh.face_count());
  const auto boundaryLoopCount = static_cast<std::ptrdiff_t>(boundary_loops(mesh).size());
  const std::ptrdiff_t twiceGenus = 2 - boundaryLoopCount - chi;
  if (twiceGenus < 0 || (twiceGenus % 2) != 0)
  {
    return std::nullopt;
  }

  return static_cast<std::size_t>(twiceGenus / 2);
}

} // namespace Geometry

#endif // GEOMETRY_MESH_MESHEULER_HPP
