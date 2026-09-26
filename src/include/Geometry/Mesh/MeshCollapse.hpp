#ifndef GEOMETRY_MESH_MESHCOLLAPSE_HPP
#define GEOMETRY_MESH_MESHCOLLAPSE_HPP

#include "Geometry/Mesh/MeshResult.hpp"
#include "Geometry/Mesh/MeshTopology.hpp"
#include "Geometry/Mesh/TriangleHalfedgeMesh.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <linal/vec.hpp>

namespace Geometry
{

/**
 * \brief Outcome of a (half)edge collapse validity check; every value but \c Ok names why the
 * collapse would break the mesh topology.
 */
enum class CollapseStatus
{
  Ok,
  // The halfedge is not in the mesh or already deleted.
  InvalidHandle,
  // An incident face has all three edges on the boundary; collapsing it would leave a dangling edge.
  IsolatedTriangle,
  // Both endpoints are boundary vertices but the edge is interior; the result would pinch the
  // surface into a non-manifold vertex.
  InteriorEdgeBetweenBoundaryVertices,
  // The endpoints share a neighbour that is not opposite the edge; the result would contain a
  // non-manifold edge and a fold-over face.
  LinkCondition,
  // The edge lies on a closed tetrahedron component; the result would be two coincident triangles.
  Tetrahedron,
  // Topologically legal, but the collapse would flip or flatten a face (see collapse_inverts_faces).
  // Never returned by is_collapse_ok; reported by operations that also check geometry.
  InvertsFaces,
};

/**
 * \brief Result of an operation that removes one endpoint of an edge: the surviving vertex, or the
 * \c CollapseStatus explaining why the mesh was left untouched.
 */
template <typename TVertexHandle>
struct CollapseResult
{
  TVertexHandle survivor{};
  CollapseStatus status{CollapseStatus::Ok};

  GEO_NODISCARD bool has_value() const noexcept { return detail::mesh_result_ok(status); }
  GEO_NODISCARD explicit operator bool() const noexcept { return has_value(); }
};

namespace detail
{

/**
 * \internal
 * \brief The vertex opposite \p halfedge in its face, or an invalid handle for a boundary halfedge.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle
opposite_vertex(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle halfedge) noexcept
{
  using VertexHandle = typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle;

  if (mesh.is_boundary(halfedge))
  {
    return VertexHandle{};
  }
  return mesh.target_vertex(mesh.get_halfedge(halfedge).next);
}

/**
 * \internal
 * \brief Whether every edge of \p face is a boundary edge, i.e. the face is a component on its own.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD bool is_isolated_face(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                    typename TriangleHalfedgeMesh<T, D, TIndex>::FaceHandle face) noexcept
{
  for (const auto halfedge : mesh.halfedges_around_face(face))
  {
    if (!mesh.is_boundary(mesh.get_halfedge(halfedge).twin))
    {
      return false;
    }
  }
  return true;
}

/**
 * \internal
 * \brief Link condition of the edge (p, q) under \p halfedge (p -> q): every common neighbour of
 * p and q is a vertex opposite the edge.
 *
 * Probes q's fan for each neighbour of p instead of tagging p's one-ring in a side buffer: valence
 * is small in practice, and this keeps the check allocation-free. O(valence(p) * valence(q)).
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD bool satisfies_link_condition(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                            typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle halfedge) noexcept
{
  using VertexHandle = typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle;

  const VertexHandle removed = mesh.source_vertex(halfedge);
  const VertexHandle survivor = mesh.target_vertex(halfedge);
  const VertexHandle leftApex = opposite_vertex(mesh, halfedge);
  const VertexHandle rightApex = opposite_vertex(mesh, mesh.get_halfedge(halfedge).twin);

  // Both faces sharing one apex means they are the same triangle glued along all three edges.
  if (leftApex.is_valid() && leftApex == rightApex)
  {
    return false;
  }

  for (auto neighbour = mesh.vertices(removed).circulator(); neighbour.is_valid(); ++neighbour)
  {
    const VertexHandle candidate = neighbour.get_vertexhandle();
    if (candidate == survivor || candidate == leftApex || candidate == rightApex)
    {
      continue;
    }
    if (mesh.find_halfedge(survivor, candidate).is_valid())
    {
      return false;
    }
  }
  return true;
}

/**
 * \internal
 * \brief Whether \p apex, a vertex opposite a collapsing edge, is an interior vertex of valence 3.
 *
 * Once the link condition holds, such an apex is adjacent only to the edge's endpoints and the
 * other apex, so its three faces plus the other face on the edge close into a tetrahedron. The
 * collapse would fold that component into two coincident triangles.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD bool is_tetrahedron_apex(const TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                       typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle apex)
{
  return apex.is_valid() && !is_boundary(mesh, apex) && valence(mesh, apex) == 3;
}

} // namespace detail

/**
 * \brief Checks whether collapsing \p halfedge (p -> q, removing p) keeps the mesh a manifold of
 * unchanged topology.
 *
 * Purely topological, following Hoppe et al. 93: the boundary rule, the link condition, and the two
 * degenerate small components the link condition alone misses (an isolated triangle and a closed
 * tetrahedron). Geometric validity (fold-overs) is a separate question, see
 * \c collapse_inverts_faces. Symmetric: p -> q and q -> p give the same answer, since both yield the
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

  if (!mesh.contains(halfedge) || mesh.is_deleted(halfedge))
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

namespace detail
{

/**
 * \internal
 * \brief Removes a face that a collapse has squashed into a 2-gon, gluing its two outer neighbours
 * into one edge.
 *
 * The 2-gon runs \p keptEdgeLoopHalfedge (s -> t) then \p removedEdgeLoopHalfedge (t -> s). Their
 * twins become twins of each other under the edge of \p keptEdgeLoopHalfedge; the edge of
 * \p removedEdgeLoopHalfedge is deleted and takes both loop halfedges with it. A crease on either
 * glued edge survives on the merged edge, so a feature line is not lost.
 */
template <typename T, std::uint8_t D, typename TIndex>
void remove_collapsed_loop(TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                           typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle keptEdgeLoopHalfedge,
                           typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle removedEdgeLoopHalfedge) noexcept
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using VertexHandle = typename Mesh::VertexHandle;
  using EdgeHandle = typename Mesh::EdgeHandle;
  using FaceHandle = typename Mesh::FaceHandle;

  const auto connectivity = mesh.connectivity();
  GEO_ASSERT(connectivity.halfedge(keptEdgeLoopHalfedge).next == removedEdgeLoopHalfedge);
  GEO_ASSERT(connectivity.halfedge(removedEdgeLoopHalfedge).next == keptEdgeLoopHalfedge);

  const HalfedgeHandle outerFromTarget = connectivity.halfedge(keptEdgeLoopHalfedge).twin;   // t -> s
  const HalfedgeHandle outerFromSource = connectivity.halfedge(removedEdgeLoopHalfedge).twin; // s -> t
  const EdgeHandle keptEdge = connectivity.halfedge(keptEdgeLoopHalfedge).edge;
  const EdgeHandle removedEdge = connectivity.halfedge(removedEdgeLoopHalfedge).edge;
  const FaceHandle face = connectivity.halfedge(keptEdgeLoopHalfedge).face;
  const VertexHandle source = connectivity.halfedge(removedEdgeLoopHalfedge).targetVertex;
  const VertexHandle target = connectivity.halfedge(keptEdgeLoopHalfedge).targetVertex;
  // Both glued halfedges on the boundary would be an edge without faces; is_collapse_ok rules it out.
  GEO_ASSERT(!connectivity.halfedge(outerFromTarget).is_boundary() || !connectivity.halfedge(outerFromSource).is_boundary());

  connectivity.halfedge(outerFromTarget).twin = outerFromSource;
  connectivity.halfedge(outerFromSource).twin = outerFromTarget;
  connectivity.halfedge(outerFromTarget).edge = keptEdge;
  connectivity.halfedge(outerFromSource).edge = keptEdge;
  connectivity.edge(keptEdge).halfedge = outerFromSource;
  connectivity.edge(keptEdge).crease = connectivity.edge(keptEdge).crease || connectivity.edge(removedEdge).crease;

  // Halfedge liveness follows the edge, so both loop halfedges move onto the deleted edge.
  connectivity.halfedge(keptEdgeLoopHalfedge).edge = removedEdge;

  if (connectivity.vertex(source).halfedge == keptEdgeLoopHalfedge)
  {
    connectivity.vertex(source).halfedge = outerFromSource;
  }
  if (connectivity.vertex(target).halfedge == removedEdgeLoopHalfedge)
  {
    connectivity.vertex(target).halfedge = outerFromTarget;
  }

  connectivity.mark_deleted(face);
  connectivity.mark_deleted(removedEdge);
}

/**
 * \internal
 * \brief Re-establishes the boundary representative rule for \p vertex: a boundary vertex must
 * store a boundary outgoing halfedge. Precondition: the stored halfedge is live.
 */
template <typename T, std::uint8_t D, typename TIndex>
void restore_boundary_representative(TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                     typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle vertex) noexcept
{
  if (!vertex.is_valid())
  {
    return;
  }
  auto& stored = mesh.connectivity().vertex(vertex).halfedge;
  GEO_ASSERT(stored.is_valid() && !mesh.is_deleted(stored));
  const auto boundary = mesh.find_outgoing_boundary(stored);
  if (boundary.is_valid())
  {
    stored = boundary;
  }
}

/**
 * \internal
 * \brief Collapses \p halfedge (p -> q) without checking legality: p is deleted and its edges are
 * re-attached to q; the one or two faces on the edge are deleted and each of them merges its two
 * remaining edges into one.
 *
 * Removes 1 vertex, 3 edges and 2 faces (1 vertex, 2 edges, 1 face for a boundary edge), preserving
 * the Euler characteristic. Positions are untouched. Precondition: \c is_collapse_ok returns \c Ok.
 * O(valence(p)).
 */
template <typename T, std::uint8_t D, typename TIndex>
void collapse_halfedge_unchecked(TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                 typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle halfedge) noexcept
{
  using Mesh = TriangleHalfedgeMesh<T, D, TIndex>;
  using HalfedgeHandle = typename Mesh::HalfedgeHandle;
  using VertexHandle = typename Mesh::VertexHandle;

  GEO_ASSERT(is_collapse_ok(mesh, halfedge) == CollapseStatus::Ok);
  const auto connectivity = mesh.connectivity();

  const HalfedgeHandle opposite = connectivity.halfedge(halfedge).twin;
  const VertexHandle removed = mesh.source_vertex(halfedge);
  const VertexHandle survivor = mesh.target_vertex(halfedge);
  const VertexHandle leftApex = opposite_vertex(mesh, halfedge);
  const VertexHandle rightApex = opposite_vertex(mesh, opposite);
  const bool hasLeftFace = !mesh.is_boundary(halfedge);
  const bool hasRightFace = !mesh.is_boundary(opposite);

  const HalfedgeHandle leftNext = connectivity.halfedge(halfedge).next;  // q -> vl, or along the boundary
  const HalfedgeHandle leftPrev = connectivity.halfedge(halfedge).prev;  // vl -> p, or along the boundary
  const HalfedgeHandle rightNext = connectivity.halfedge(opposite).next; // p -> vr, or along the boundary
  const HalfedgeHandle rightPrev = connectivity.halfedge(opposite).prev; // vr -> q, or along the boundary

  // An outgoing halfedge of q that survives: the glued twin of vl -> p on the left, or the boundary
  // halfedge that continues past the removed edge.
  const HalfedgeHandle survivorOutgoing = hasLeftFace ? connectivity.halfedge(leftPrev).twin : leftNext;

  // Re-target every halfedge arriving at p. The fan walk reads only twin/next links, so rewriting
  // target vertices while walking is safe.
  HalfedgeHandle outgoing = halfedge;
  do
  {
    connectivity.halfedge(connectivity.halfedge(outgoing).twin).targetVertex = survivor;
    outgoing = mesh.next_in_outgoing_fan(outgoing);
  } while (outgoing != halfedge);

  const auto link = [&](HalfedgeHandle prev, HalfedgeHandle next) {
    connectivity.halfedge(prev).next = next;
    connectivity.halfedge(next).prev = prev;
  };
  link(leftPrev, leftNext);
  link(rightPrev, rightNext);

  // Each squashed face keeps the edge that already joined q to its apex.
  if (hasLeftFace)
  {
    remove_collapsed_loop(mesh, leftNext, leftPrev);
  }
  if (hasRightFace)
  {
    remove_collapsed_loop(mesh, rightPrev, rightNext);
  }

  connectivity.vertex(survivor).halfedge = survivorOutgoing;
  connectivity.mark_deleted(connectivity.halfedge(halfedge).edge);
  connectivity.mark_deleted(removed);

  restore_boundary_representative(mesh, survivor);
  restore_boundary_representative(mesh, leftApex);
  restore_boundary_representative(mesh, rightApex);
}

/**
 * \internal
 * \brief Whether moving a triangle's corners from \p before to \p after flips or flattens it.
 *
 * A triangle that was already degenerate has no orientation to lose, so only its becoming
 * non-degenerate counts, never as an inversion. In 3D the orientation is the area vector and a
 * flip is a non-positive dot product; in 2D it is the sign of the signed area.
 */
template <typename T, std::uint8_t D>
GEO_NODISCARD bool triangle_inverts(const std::array<linal::vec<T, D>, 3>& before,
                                    const std::array<linal::vec<T, D>, 3>& after) noexcept
{
  static_assert(D == 2 || D == 3, "orientation is defined for planar and spatial triangles only");
  using Vec = linal::vec<T, D>;

  if constexpr (D == 3)
  {
    const auto area_vector = [](const std::array<Vec, 3>& corners) {
      return linal::cross(Vec{corners[1] - corners[0]}, Vec{corners[2] - corners[0]});
    };
    const Vec areaBefore = area_vector(before);
    const Vec areaAfter = area_vector(after);
    if (linal::length_squared(areaAfter) == T{0})
    {
      return true;
    }
    return linal::length_squared(areaBefore) != T{0} && linal::dot(areaBefore, areaAfter) <= T{0};
  }
  else
  {
    const auto signed_area = [](const std::array<Vec, 3>& corners) {
      const Vec first{corners[1] - corners[0]};
      const Vec second{corners[2] - corners[0]};
      return first[0] * second[1] - first[1] * second[0];
    };
    const T areaBefore = signed_area(before);
    const T areaAfter = signed_area(after);
    if (areaAfter == T{0})
    {
      return true;
    }
    return areaBefore != T{0} && (areaBefore > T{0}) != (areaAfter > T{0});
  }
}

} // namespace detail

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

  GEO_ASSERT(mesh.contains(halfedge) && !mesh.is_deleted(halfedge));
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
 * \brief Halfedge collapse: removes the source p of \p halfedge by merging it into the target q,
 * which keeps its position.
 *
 * The degree-of-freedom-free Euler operator of incremental decimation (Kobbelt et al. 98): it is
 * both an edge collapse with the merged vertex placed at q and a vertex removal whose hole is
 * fan-triangulated from q. Removes 1 vertex, 3 edges and 2 faces (1, 2, 1 on the boundary), so the
 * Euler characteristic is preserved. Removed elements are tombstoned, so every other handle stays
 * valid until \c garbage_collection(). Topology only: check \c collapse_inverts_faces first when
 * fold-overs matter. O(valence(p) * valence(q)).
 *
 * \return \c CollapseStatus::Ok after collapsing, otherwise the reason the mesh was left untouched.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD CollapseStatus collapse_halfedge(TriangleHalfedgeMesh<T, D, TIndex>& mesh,
                                               typename TriangleHalfedgeMesh<T, D, TIndex>::HalfedgeHandle halfedge)
{
  const CollapseStatus status = is_collapse_ok(mesh, halfedge);
  if (status == CollapseStatus::Ok)
  {
    detail::collapse_halfedge_unchecked(mesh, halfedge);
  }
  return status;
}

/**
 * \brief Edge collapse: merges both endpoints of \p edge into one vertex placed at \p position.
 *
 * A halfedge collapse followed by moving the survivor: the connectivity after collapsing (p, q) is
 * the same whichever endpoint survives, so the merged position is the collapse's only degree of
 * freedom. Which endpoint's handle survives is unspecified; use the returned one. Same topological
 * rules and cost as \c collapse_halfedge.
 *
 * \return The surviving vertex, or an invalid one with the reason the mesh was left untouched.
 */
template <typename T, std::uint8_t D, typename TIndex>
GEO_NODISCARD CollapseResult<typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle>
collapse_edge(TriangleHalfedgeMesh<T, D, TIndex>& mesh,
              typename TriangleHalfedgeMesh<T, D, TIndex>::EdgeHandle edge,
              const typename TriangleHalfedgeMesh<T, D, TIndex>::vec_t& position)
{
  using VertexHandle = typename TriangleHalfedgeMesh<T, D, TIndex>::VertexHandle;

  if (!mesh.contains(edge) || mesh.is_deleted(edge))
  {
    return {VertexHandle{}, CollapseStatus::InvalidHandle};
  }

  const auto halfedge = mesh.get_edge(edge).halfedge;
  const VertexHandle survivor = mesh.target_vertex(halfedge);
  const CollapseStatus status = collapse_halfedge(mesh, halfedge);
  if (status != CollapseStatus::Ok)
  {
    return {VertexHandle{}, status};
  }
  mesh.set_position(survivor, position);
  return {survivor, CollapseStatus::Ok};
}

} // namespace Geometry

#endif // GEOMETRY_MESH_MESHCOLLAPSE_HPP
