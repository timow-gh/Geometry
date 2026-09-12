#ifndef GEOMETRY_MESH_ADDTRIANGLE_HPP
#define GEOMETRY_MESH_ADDTRIANGLE_HPP

#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <array>
#include <cstdint>
#include <utility>

namespace Geometry
{

// -----------------------------------------------------------------------------------------------
// add_triangle inserts one triangular face into a TriangleHalfedgeMesh, maintaining this mesh's
// halfedge representation:
//
//   * The interior halfedge carries the incident face; the opposite halfedge is either the 
//     interior halfedge of a neighbouring face or a BOUNDARY halfedge whose face is 
//     invalid (FaceHandle{}).
//   * Every halfedge therefore always has a valid twin, and boundary halfedges are chained through
//     next/prev into closed boundary loops (all with invalid face).
//   * The mesh stays manifold: a vertex's incident faces form a single fan. add_triangle rejects any
//     triangle that would create a non-manifold edge or a non-manifold vertex.
//
// The directed-edge map is kept a bijection halfedge <-> (source,target): interior a->b keyed (a,b);
// its opposite b->a keyed (b,a).
// -----------------------------------------------------------------------------------------------

namespace detail
{

// Precondition check for add_triangle. Uses only the read-only (const) connectivity view. On success
// `existingInner` is filled with the forward halfedge located for each edge i (vertices[i] ->
// vertices[i+1]), or an invalid handle where that edge is new; add_triangle reuses these so the
// forward fan walks are not repeated. On failure `existingInner` is left unspecified.
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD bool can_add_triangle(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                    const std::array<typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle, 3>& vertices,
                                    std::array<typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle, 3>& existingInner)
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using size_type = typename Mesh::size_type;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using FaceHandle = typename Mesh::FaceHandle;

  const auto connectivity = mesh.connectivity();

  if (!mesh.contains(vertices[0]) || !mesh.contains(vertices[1]) || !mesh.contains(vertices[2]))
  {
    return false;
  }
  if (vertices[0] == vertices[1] || vertices[1] == vertices[2] || vertices[2] == vertices[0])
  {
    return false;
  }

  // Resolve each edge's forward (from->to) and reversed (to->from) halfedge exactly once; every later
  // check reads these three cached pairs instead of re-walking the fans.
  std::array<HalfedgeHandle, 3> forward{};
  std::array<HalfedgeHandle, 3> reversed{};
  for (size_type i = 0; i < 3; ++i)
  {
    forward[i] = connectivity.find_halfedge(vertices[i], vertices[(i + 1) % 3]);
    reversed[i] = connectivity.find_halfedge(vertices[(i + 1) % 3], vertices[i]);
  }
  existingInner = forward;

  // A duplicate face with the SAME winding has all three directed edges already interior, caught by
  // the boundary test below. A duplicate with the OPPOSITE winding instead has all three *reversed*
  // directed edges interior and belonging to one existing face; its own edges are still boundary, so
  // the boundary test would wrongly accept it. Reject when every reversed edge is the interior
  // halfedge of the same face (this replaces the former sorted-vertex face-key set).
  FaceHandle oppositeFace{};
  bool reversedDuplicate = true;
  for (size_type i = 0; i < 3; ++i)
  {
    // A halfedge already running from->to must be a boundary halfedge we will consume; an interior
    // one means this directed edge already bounds a face -> duplicate/non-manifold edge.
    if (forward[i].is_valid() && !connectivity.halfedge(forward[i]).is_boundary())
    {
      return false;
    }

    const FaceHandle face = reversed[i].is_valid() ? connectivity.halfedge(reversed[i]).face : FaceHandle{};
    if (!face.is_valid() || (oppositeFace.is_valid() && face != oppositeFace))
    {
      reversedDuplicate = false;
    }
    else
    {
      oppositeFace = face;
    }
  }
  if (reversedDuplicate)
  {
    return false;
  }

  // Edge i (between vertices[i] and vertices[i+1]) is present in either direction.
  const std::array<bool, 3> edgePresent{forward[0].is_valid() || reversed[0].is_valid(),
                                        forward[1].is_valid() || reversed[1].is_valid(),
                                        forward[2].is_valid() || reversed[2].is_valid()};

  for (size_type i = 0; i < 3; ++i)
  {
    const auto vertex = vertices[i];

    // A used vertex must lie on the boundary; one already surrounded by faces would gain a second fan.
    if (mesh.get_vertex(vertex).halfedge.is_valid() && !mesh.is_boundary_outgoing(vertex))
    {
      return false;
    }

    // A used vertex whose two incident triangle edges are BOTH new starts a fan disconnected from the
    // existing one at that vertex (e.g. two triangles sharing only a corner) -> non-manifold vertex.
    // The incident edges at vertices[i] are edge (i+2)%3 (prev->vertex) and edge i (vertex->next).
    if (mesh.get_vertex(vertex).halfedge.is_valid())
    {
      if (!edgePresent[(i + 2) % 3] && !edgePresent[i])
      {
        return false;
      }
    }
  }

  return true;
}

} // namespace detail

// Adds a triangle face spanned by the three vertices (in order). Returns an invalid FaceHandle if the
// triangle cannot be added. Strongly exception-safe: on failure the mesh is rolled back.
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD typename TriangleHalfedgeMesh<T, D, TIndex>::FaceHandle
add_triangle(TriangleHalfedgeMesh<T, D, TIndex>& mesh,
             const std::array<typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle, 3>& triangleVertices)
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using FaceHandle = typename Mesh::FaceHandle;
  using EdgeHandle = typename Mesh::EdgeHandle;
  using VertexHandle = typename Mesh::VertexHandle;
  using Halfedge = typename Mesh::Halfedge;
  using size_type = typename Mesh::size_type;

  // can_add_triangle locates each edge's forward halfedge as part of its checks; reuse those handles
  // here instead of re-walking the fans in Phase A.
  std::array<HalfedgeHandle, 3> existingInner{};
  if (!detail::can_add_triangle(mesh, triangleVertices, existingInner))
  {
    return FaceHandle{};
  }

  auto connectivity = mesh.connectivity();

  auto halfedge_at = [&](HalfedgeHandle handle) -> Halfedge& { return connectivity.halfedge(handle); };

  // --- Phase A: classify which edges are new from the already-located halfedges. -------------------
  std::array<bool, 3> isNew{};
  size_type newEdgeCount = 0;

  for (size_type i = 0; i < 3; ++i)
  {
    isNew[i] = !existingInner[i].is_valid();
    if (isNew[i])
    {
      ++newEdgeCount;
    }
  }

  connectivity.reserve_faces(1);
  connectivity.reserve_halfedges(2 * newEdgeCount);
  connectivity.reserve_edges(newEdgeCount);

  const size_type faceCount = connectivity.face_count();
  const size_type halfedgeCount = connectivity.halfedge_count();
  const size_type edgeCount = connectivity.edge_count();

  // Undo log for links overwritten on pre-existing halfedges/vertices (strong exception safety).
  // nextUndo records overwritten .next (and, via set_next, the paired .prev of the old successor);
  // prevUndo records an overwritten .prev on a pre-existing successor when only that side is
  // pre-existing (the .next side being a new, roll-back-truncated halfedge).
  //
  // These logs are stack-resident fixed-capacity arrays: a single triangle has exactly 3 edges /
  // 3 corners, so the number of entries each log can accumulate is bounded by a small constant
  // (see the per-log capacities below). This keeps add_triangle free of per-call heap allocations.
  std::array<std::pair<HalfedgeHandle, HalfedgeHandle>, 9> nextUndo{};
  std::array<std::pair<HalfedgeHandle, HalfedgeHandle>, 6> prevUndo{};
  std::array<std::pair<HalfedgeHandle, FaceHandle>, 3> faceUndo{};
  std::array<std::pair<VertexHandle, HalfedgeHandle>, 3> vertexUndo{};
  size_type nextUndoCount = 0;
  size_type prevUndoCount = 0;
  size_type faceUndoCount = 0;
  size_type vertexUndoCount = 0;

  auto set_next = [&](HalfedgeHandle prev, HalfedgeHandle next) {
    halfedge_at(prev).next = next;
    halfedge_at(next).prev = prev;
  };
  // set_next that records the old next of `prev` for rollback (prev is a pre-existing halfedge).
  auto set_next_logged = [&](HalfedgeHandle prev, HalfedgeHandle next) {
    nextUndo[nextUndoCount++] = {prev, halfedge_at(prev).next};
    set_next(prev, next);
  };
  // set_next that records the old prev of `next` for rollback. Use when `next` is the pre-existing
  // halfedge and `prev` is a new (roll-back-truncated) halfedge, so only `next.prev` must survive.
  auto set_next_logged_succ = [&](HalfedgeHandle prev, HalfedgeHandle next) {
    prevUndo[prevUndoCount++] = {next, halfedge_at(next).prev};
    set_next(prev, next);
  };
  auto set_face_logged = [&](HalfedgeHandle handle, FaceHandle face) {
    faceUndo[faceUndoCount++] = {handle, halfedge_at(handle).face};
    halfedge_at(handle).face = face;
  };
  auto set_vertex_logged = [&](VertexHandle vertex, HalfedgeHandle handle) {
    vertexUndo[vertexUndoCount++] = {vertex, connectivity.vertex(vertex).halfedge};
    connectivity.vertex(vertex).halfedge = handle;
  };

  const FaceHandle face = connectivity.new_face();

  auto rollback = [&]() noexcept {
    for (size_type i = nextUndoCount; i-- > 0;)
    {
      halfedge_at(nextUndo[i].first).next = nextUndo[i].second;
      halfedge_at(nextUndo[i].second).prev = nextUndo[i].first;
    }
    for (size_type i = prevUndoCount; i-- > 0;) halfedge_at(prevUndo[i].first).prev = prevUndo[i].second;
    for (size_type i = faceUndoCount; i-- > 0;) halfedge_at(faceUndo[i].first).face = faceUndo[i].second;
    for (size_type i = vertexUndoCount; i-- > 0;) connectivity.vertex(vertexUndo[i].first).halfedge = vertexUndo[i].second;
    // No directed-edge/face-key map to undo: the resize_* truncations below drop every new element,
    // and the fan walk that answers edge queries reads only the (now-restored) connectivity.
    connectivity.resize_edges(edgeCount);
    connectivity.resize_halfedges(halfedgeCount);
    connectivity.resize_faces(faceCount);
  };

  try
  {
    // --- Phase B: for corners where both edges already exist, ensure the two inner halfedges are
    //     adjacent in the boundary loop, patching (rotating) the loop if not. Patches are collected
    //     here and applied below rather than immediately. For a triangle there are at most 3 such corners.
    // Deferred set_next(prev,next): at most 3 patches per patched corner over at most 3 corners.
    std::array<std::pair<HalfedgeHandle, HalfedgeHandle>, 9> nextCache{};
    size_type nextCacheCount = 0;

    for (size_type i = 0; i < 3; ++i)
    {
      const size_type ii = (i + 1) % 3;
      if (isNew[i] || isNew[ii])
      {
        continue; // corner between v_{i+1}; only patch when both incident edges pre-exist
      }
      const HalfedgeHandle innerPrevHe = existingInner[i];  // ... -> corner
      const HalfedgeHandle innerNextHe = existingInner[ii]; // corner -> ...

      if (halfedge_at(innerPrevHe).next != innerNextHe)
      {
        // Need to make innerPrevHe.next == innerNextHe. Find a free (boundary) gap to rotate.
        // outerPrev = twin(innerNextHe) is boundary; rotate around the corner via twin/next to find
        // an incoming boundary halfedge whose next is a boundary outgoing at the corner.
        const HalfedgeHandle outerPrev = halfedge_at(innerNextHe).twin;
        const HalfedgeHandle outerNext = halfedge_at(innerPrevHe).twin;

        // boundaryPrev: an incoming boundary halfedge at the corner, distinct from the wedge edge.
        // Rotate around the corner until we land on a boundary (no-face) halfedge that is not the
        // wedge's own incoming interior halfedge.
        HalfedgeHandle boundaryPrev = outerPrev;
        do
        {
          boundaryPrev = halfedge_at(halfedge_at(boundaryPrev).next).twin;
        } while (!halfedge_at(boundaryPrev).is_boundary() || boundaryPrev == innerNextHe);
        // boundaryPrev is now an incoming boundary halfedge at the corner distinct from the wedge.
        const HalfedgeHandle boundaryNext = halfedge_at(boundaryPrev).next;

        const HalfedgeHandle patchStart = halfedge_at(innerPrevHe).next;
        const HalfedgeHandle patchEnd = halfedge_at(innerNextHe).prev;

        nextCache[nextCacheCount++] = {boundaryPrev, patchStart};
        nextCache[nextCacheCount++] = {patchEnd, boundaryNext};
        nextCache[nextCacheCount++] = {outerPrev, outerNext};
      }
    }

    // --- Phase C: create the interior + boundary halfedge pairs for new edges. -----------------------
    std::array<HalfedgeHandle, 3> inner{};
    std::array<HalfedgeHandle, 3> outer{};

    for (size_type i = 0; i < 3; ++i)
    {
      const VertexHandle from = triangleVertices[i];
      const VertexHandle to = triangleVertices[(i + 1) % 3];
      if (isNew[i])
      {
        const HalfedgeHandle innerHe = connectivity.new_halfedge();
        const HalfedgeHandle outerHe = connectivity.new_halfedge();
        halfedge_at(innerHe).targetVertex = to;
        halfedge_at(outerHe).targetVertex = from;
        halfedge_at(innerHe).twin = outerHe;
        halfedge_at(outerHe).twin = innerHe;
        const EdgeHandle edge = connectivity.new_edge(innerHe);
        halfedge_at(innerHe).edge = edge;
        halfedge_at(outerHe).edge = edge;
        inner[i] = innerHe;
        outer[i] = outerHe;
      }
      else
      {
        inner[i] = existingInner[i];
        outer[i] = halfedge_at(inner[i]).twin;
      }
    }

    // Apply Phase B patches first (open the boundary gaps before we relink corners).
    for (size_type i = 0; i < nextCacheCount; ++i)
    {
      set_next_logged(nextCache[i].first, nextCache[i].second);
    }

    // --- Phase D: relink the boundary loop at each corner. -------------------------------------------
    // At corner c = v_{ii} between incoming interior inner[i] (-> c) and outgoing interior inner[ii]
    // (c ->). The boundary-side halfedges run opposite to the interior:
    //   outerIntoC   = twin(inner[ii]) = outer[ii]   (a boundary halfedge arriving at c)
    //   outerOutOfC  = twin(inner[i])  = outer[i]    (a boundary halfedge leaving c)
    // Only the boundary halfedges of NEW edges actually stay on the boundary. We connect, in boundary
    // order, [boundary arriving at c] -> outerIntoC (if edge ii new) ... outerOutOfC (if edge i new)
    // -> [boundary leaving c], collapsing to a direct link when an edge is reused.
    for (size_type i = 0; i < 3; ++i)
    {
      const size_type ii = (i + 1) % 3;
      const bool prevNew = isNew[i];  // edge inner[i]
      const bool nextNew = isNew[ii]; // edge inner[ii]

      const HalfedgeHandle outerIntoC = outer[ii];
      const HalfedgeHandle outerOutOfC = outer[i];

      if (prevNew && nextNew)
      {
        // Both new. Find where the wedge splices into any existing boundary loop at the corner.
        // outerOutOfC leaves c; the boundary that used to leave c (if any) becomes its next.
        // outerIntoC arrives at c; the boundary that used to arrive at c becomes its prev.
        const VertexHandle corner = triangleVertices[ii];
        const HalfedgeHandle oldBoundaryOut = connectivity.vertex(corner).halfedge; // boundary leaving c
        if (oldBoundaryOut.is_valid() && halfedge_at(oldBoundaryOut).is_boundary())
        {
          const HalfedgeHandle oldBoundaryIn = halfedge_at(oldBoundaryOut).prev; // boundary arriving at c
          set_next_logged(oldBoundaryIn, outerIntoC);
          set_next(outerIntoC, outerOutOfC);
          set_next_logged(outerOutOfC, oldBoundaryOut);
        }
        else
        {
          // brand-new corner: the two new boundary halfedges chain directly.
          set_next(outerIntoC, outerOutOfC);
        }
      }
      else if (!prevNew && nextNew)
      {
        // inner[i] reused (was boundary, becoming interior); its old boundary continuation now
        // continues from outerIntoC. outerOutOfC is not on the boundary (edge i reused).
        const HalfedgeHandle oldContinuation = halfedge_at(inner[i]).next; // where the old boundary went after c
        // oldContinuation is pre-existing; outerIntoC is new (edge ii is new). Only oldContinuation.prev
        // must survive a rollback, so log that side.
        set_next_logged_succ(outerIntoC, oldContinuation);
      }
      else if (prevNew && !nextNew)
      {
        // inner[ii] reused; the old boundary that arrived at c now flows into outerOutOfC.
        const HalfedgeHandle oldPredecessor = halfedge_at(inner[ii]).prev; // what arrived before c on old boundary
        // oldPredecessor is pre-existing; outerOutOfC is new (edge i is new). Only oldPredecessor.next
        // must survive a rollback, so log that side.
        set_next_logged(oldPredecessor, outerOutOfC);
      }
      // both reused: handled by Phase B (loop already made consistent); nothing to link here.
    }

    // --- Phase E: interior triangle cycle, faces, and vertex halfedge adjustment. --------------------
    for (size_type i = 0; i < 3; ++i)
    {
      const size_type ii = (i + 1) % 3;
      if (!isNew[i])
      {
        // inner[i] was a boundary halfedge; its next is being repointed into the face cycle.
        nextUndo[nextUndoCount++] = {inner[i], halfedge_at(inner[i]).next};
      }
      // set_next also overwrites inner[ii].prev. When inner[ii] is pre-existing and inner[i] is new
      // (so inner[i].next is truncated on rollback and cannot carry the paired restore), inner[ii].prev
      // must be logged on its own side. In the both-reused case Phase B already made this link
      // consistent, so the write is a no-op and logging it is harmless.
      if (!isNew[ii] && isNew[i])
      {
        prevUndo[prevUndoCount++] = {inner[ii], halfedge_at(inner[ii]).prev};
      }
      set_next(inner[i], inner[ii]);
    }
    for (size_type i = 0; i < 3; ++i)
    {
      if (!isNew[i])
      {
        set_face_logged(inner[i], face);
      }
      else
      {
        halfedge_at(inner[i]).face = face;
      }
    }

    for (size_type i = 0; i < 3; ++i)
    {
      const VertexHandle vertex = triangleVertices[i];
      // Keep the vertex referencing a boundary outgoing halfedge if one remains; otherwise (now
      // interior) any outgoing interior halfedge is fine.
      const HalfedgeHandle boundary = mesh.find_outgoing_boundary(inner[i]);
      if (boundary.is_valid())
      {
        set_vertex_logged(vertex, boundary);
      }
      else if (!connectivity.vertex(vertex).halfedge.is_valid())
      {
        set_vertex_logged(vertex, inner[i]);
      }
      else
      {
        set_vertex_logged(vertex, connectivity.vertex(vertex).halfedge); // record only, keep value
      }
    }

    connectivity.face(face).set_halfedgehandle(inner.front());
  }
  catch (...)
  {
    rollback();
    throw;
  }

  return face;
}

template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD typename TriangleHalfedgeMesh<T, D, TIndex>::FaceHandle
add_triangle(TriangleHalfedgeMesh<T, D, TIndex>& mesh,
             typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle first,
             typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle second,
             typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle third)
{
  return add_triangle(mesh, {first, second, third});
}

} // namespace Geometry

#endif // GEOMETRY_MESH_ADDTRIANGLE_HPP
