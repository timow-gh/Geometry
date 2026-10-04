#ifndef GEOMETRY_MESH_MESHGLOBALTOPOLOGY_HPP
#define GEOMETRY_MESH_MESHGLOBALTOPOLOGY_HPP

#include "Geometry/Mesh/MeshVerify.hpp"
#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace Geometry
{

/**
 * \brief Euler characteristic chi = V - E + F of the mesh.
 *
 * \return The signed characteristic; signed because a mesh with boundary or several components can
 * exceed 2.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD std::ptrdiff_t euler_characteristic(const TriangleHalfedgeMesh<T, D, TIndex>& mesh) noexcept
{
  const auto vertexCount = static_cast<std::ptrdiff_t>(mesh.vertex_count());
  const auto edgeCount = static_cast<std::ptrdiff_t>(mesh.edge_count());
  const auto faceCount = static_cast<std::ptrdiff_t>(mesh.face_count());
  return vertexCount - edgeCount + faceCount;
}

namespace detail
{

/**
 * \internal
 * \brief Walks every boundary loop once, calling \p onLoopStart before each loop and
 * \p onHalfedge for each of its halfedges in order.
 *
 * Shared by \c boundary_loops and \c boundary_loop_count so the corruption guard below exists once,
 * while the count needs no storage for the loops themselves. O(H).
 */
template <typename T, std::uint8_t D, typename TIndex, typename TOnLoopStart, typename TOnHalfedge>
void for_each_boundary_loop(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                            TOnLoopStart&& onLoopStart,
                            TOnHalfedge&& onHalfedge)
{
  using HalfedgeHandle = typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle;

  std::vector<unsigned char> visited(mesh.halfedge_storage_size(), 0U);
  for (const HalfedgeHandle start : mesh.halfedges())
  {
    if (visited[start.get_value()] != 0U || !mesh.is_boundary(start))
    {
      continue;
    }

    onLoopStart();
    // A well-formed boundary loop chains boundary halfedges through .next and closes within the
    // halfedge count. On a mesh built through the raw connectivity view a boundary halfedge's .next
    // may leave the boundary or never return to `start`; the step budget and the boundary guard make
    // the walk terminate rather than hang (the loop contents on such corrupt input are unspecified).
    const std::size_t limit = mesh.halfedge_storage_size();
    std::size_t steps = 0;
    HalfedgeHandle current = start;
    do
    {
      visited[current.get_value()] = 1U;
      onHalfedge(current);
      current = mesh.get_halfedge(current).next;
    } while (current != start && ++steps <= limit && mesh.is_boundary(current));
  }
}

} // namespace detail

/**
 * \brief Enumerates the boundary loops of the mesh, one ordered cycle of boundary halfedges each.
 *
 * Each loop lists the halfedges with no incident face forming one closed boundary cycle. Use
 * \c boundary_loop_count when only the number of loops is needed. O(H).
 *
 * \return One entry per boundary loop; empty for a closed (watertight) mesh.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD std::vector<std::vector<typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle>>
boundary_loops(const TriangleHalfedgeMesh<T, D, TIndex>& mesh)
{
  using HalfedgeHandle = typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle;

  std::vector<std::vector<HalfedgeHandle>> loops;
  detail::for_each_boundary_loop(
      mesh, [&loops]() { loops.emplace_back(); },
      [&loops](HalfedgeHandle halfedge) { loops.back().push_back(halfedge); });
  return loops;
}

/**
 * \brief Number of boundary loops, i.e. the \c b in \c V - E + F = 2 - 2g - b.
 *
 * Counts the loops \c boundary_loops would enumerate without storing their halfedges. O(H).
 *
 * \return The loop count; 0 for a closed (watertight) mesh.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD std::size_t boundary_loop_count(const TriangleHalfedgeMesh<T, D, TIndex>& mesh)
{
  using HalfedgeHandle = typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle;

  std::size_t loopCount = 0;
  detail::for_each_boundary_loop(
      mesh, [&loopCount]() noexcept { ++loopCount; }, [](HalfedgeHandle) noexcept {});
  return loopCount;
}

/**
 * \brief Number of connected components, by flood fill over faces across shared, non-boundary edges.
 *
 * Isolated vertices are ignored, so a mesh with no faces has zero components. O(H).
 *
 * \return The component count.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD std::size_t connected_component_count(const TriangleHalfedgeMesh<T, D, TIndex>& mesh)
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using FaceHandle = typename Mesh::FaceHandle;

  std::vector<unsigned char> visited(mesh.face_storage_size(), 0U);
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

/**
 * \brief Genus (handle count) of the surface.
 *
 * Defined for surfaces with boundary as well as closed ones (a disc has genus 0), from
 * \c V - E + F = 2 - 2g - b for a connected, orientable, manifold surface with \c b boundary loops,
 * so \c g = (2 - b - chi) / 2.
 *
 * Preconditions are ASSUMED, not checked at runtime: the mesh is manifold and connected (exactly one
 * component); the halfedge kernel is orientable by construction. Verify with \c verify_manifold() and
 * \c connected_component_count() beforehand if unsure -- genus guards only its own arithmetic.
 *
 * \return The genus, or \c std::nullopt when the formula does not yield a non-negative integer.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD std::optional<std::size_t> genus(const TriangleHalfedgeMesh<T, D, TIndex>& mesh)
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using VertexHandle = typename Mesh::VertexHandle;

  // Preconditions are assumed; the assertions catch a violated caller in debug builds and compile out
  // in release, so genus stays free of the manifold/connectivity scans it would otherwise pay for.
  GEO_ASSERT(connected_component_count(mesh) == 1);
  GEO_ASSERT(verify_manifold(mesh));

  // Count only vertices that belong to the surface. Isolated vertices are manifold-legal (add_vertex
  // creates them) but are ignored by connected_component_count and verify_manifold, so the Euler
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
  const auto boundaryLoopCount = static_cast<std::ptrdiff_t>(boundary_loop_count(mesh));
  const std::ptrdiff_t twiceGenus = 2 - boundaryLoopCount - chi;
  if (twiceGenus < 0 || (twiceGenus % 2) != 0)
  {
    return std::nullopt;
  }

  return static_cast<std::size_t>(twiceGenus / 2);
}

} // namespace Geometry

#endif // GEOMETRY_MESH_MESHGLOBALTOPOLOGY_HPP
