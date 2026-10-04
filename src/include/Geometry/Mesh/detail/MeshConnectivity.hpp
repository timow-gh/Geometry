#ifndef GEOMETRY_MESH_DETAIL_MESHCONNECTIVITY_HPP
#define GEOMETRY_MESH_DETAIL_MESHCONNECTIVITY_HPP

#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include "Geometry/Utils/Constness.hpp"
#include <algorithm>
#include <array>
#include <type_traits>
#include <vector>

namespace Geometry
{

namespace detail
{

/**
 * \internal
 * \brief Ensures \p storage can take \p additional appends without reallocating.
 *
 * Grows geometrically rather than to the exact size: the mesh operators reserve once per call, and
 * a decimation pass calls them once per element, so exact-size reserves would reallocate and copy
 * all storage on every call. Amortized O(additional).
 */
template <typename TElement>
void reserve_additional(std::vector<TElement>& storage, typename std::vector<TElement>::size_type additional)
{
  const auto required = storage.size() + additional;
  if (required > storage.capacity())
  {
    storage.reserve(std::max(required, 2 * storage.capacity()));
  }
}

} // namespace detail

/**
 * \brief Low-level connectivity kernel view for \c TriangleHalfedgeMesh -- the "unchecked" tier of
 * the mesh API.
 *
 * Exposes the raw element-creation and link-setting primitives that write algorithms (e.g.
 * \c add_triangle) need to build connectivity directly. None of these primitives validate mesh
 * invariants; the caller is responsible for leaving the mesh in a consistent state.
 *
 * The const specialization (\c C == \c Constness::Const) exposes only const element access and
 * read-only lookups; mutators live on the mutable specialization via \c requires clauses.
 */
template <typename Mesh, Constness C>
class MeshConnectivityView
{
    using MeshPtr = qualified_ptr_t<C, Mesh>;

    MeshPtr m_mesh{nullptr};

    template <typename, Constness>
    friend class MeshConnectivityView;

  public:
    using value_type = typename Mesh::value_type;
    using size_type = typename Mesh::size_type;
    using vec_t = typename Mesh::vec_t;

    using VertexHandle = typename Mesh::VertexHandle;
    using HalfedgeHandle = typename Mesh::HalfedgeHandle;
    using FaceHandle = typename Mesh::FaceHandle;
    using EdgeHandle = typename Mesh::EdgeHandle;

    using Vertex = typename Mesh::Vertex;
    using Halfedge = typename Mesh::Halfedge;
    using Face = typename Mesh::Face;
    using Edge = typename Mesh::Edge;

    using VertexRef = qualified_ref_t<C, Vertex>;
    using HalfedgeRef = qualified_ref_t<C, Halfedge>;
    using FaceRef = qualified_ref_t<C, Face>;
    using EdgeRef = qualified_ref_t<C, Edge>;

    constexpr MeshConnectivityView() noexcept = default;
    constexpr explicit MeshConnectivityView(MeshPtr mesh) noexcept
        : m_mesh(mesh) {}

    /** \brief Converts a mutable view to a const view; the reverse is disabled by the \c requires. */
    template <Constness Other>
    constexpr MeshConnectivityView(const MeshConnectivityView<Mesh, Other>& other) noexcept
        requires(is_const(C) && !is_const(Other))
        : m_mesh(other.m_mesh) {}

    // --- storage sizes --------------------------------------------------------------------
    // Raw storage sizes, tombstoned elements included: the next handle value new_* will hand out, and
    // the truncation point resize_* rolls back to.
    GEO_NODISCARD size_type vertex_storage_size() const noexcept { return m_mesh->m_vertices.size(); }
    GEO_NODISCARD size_type halfedge_storage_size() const noexcept { return m_mesh->m_halfedges.size(); }
    GEO_NODISCARD size_type face_storage_size() const noexcept { return m_mesh->m_faces.size(); }
    GEO_NODISCARD size_type edge_storage_size() const noexcept { return m_mesh->m_edges.size(); }

    // --- raw element access -------------------------------------------------------------
    GEO_NODISCARD VertexRef vertex(VertexHandle handle) const noexcept { return m_mesh->get_vertex(handle); }
    GEO_NODISCARD HalfedgeRef halfedge(HalfedgeHandle handle) const noexcept { return m_mesh->get_halfedge(handle); }
    GEO_NODISCARD FaceRef face(FaceHandle handle) const noexcept { return m_mesh->get_face(handle); }
    GEO_NODISCARD EdgeRef edge(EdgeHandle handle) const noexcept { return m_mesh->get_edge(handle); }

    GEO_NODISCARD bool contains(HalfedgeHandle handle) const noexcept { return m_mesh->contains(handle); }
    GEO_NODISCARD bool contains(VertexHandle handle) const noexcept { return m_mesh->contains(handle); }
    GEO_NODISCARD bool contains(FaceHandle handle) const noexcept { return m_mesh->contains(handle); }
    GEO_NODISCARD bool contains(EdgeHandle handle) const noexcept { return m_mesh->contains(handle); }

    GEO_NODISCARD bool is_deleted(VertexHandle handle) const noexcept { return m_mesh->is_deleted(handle); }
    GEO_NODISCARD bool is_deleted(HalfedgeHandle handle) const noexcept { return m_mesh->is_deleted(handle); }
    GEO_NODISCARD bool is_deleted(FaceHandle handle) const noexcept { return m_mesh->is_deleted(handle); }
    GEO_NODISCARD bool is_deleted(EdgeHandle handle) const noexcept { return m_mesh->is_deleted(handle); }

    GEO_NODISCARD bool is_live(VertexHandle handle) const noexcept { return m_mesh->is_live(handle); }
    GEO_NODISCARD bool is_live(HalfedgeHandle handle) const noexcept { return m_mesh->is_live(handle); }
    GEO_NODISCARD bool is_live(FaceHandle handle) const noexcept { return m_mesh->is_live(handle); }
    GEO_NODISCARD bool is_live(EdgeHandle handle) const noexcept { return m_mesh->is_live(handle); }

    // --- connectivity lookup ------------------------------------------------------------
    /**
     * \brief Locates the halfedge from \p from to \p to by walking the fan around \p from, or an
     * invalid handle if none exists.
     *
     * A fan walk rather than a lookup table: it replaces the former persistent directed-edge map, so
     * there is no side structure to keep in sync across mutations.
     */
    GEO_NODISCARD HalfedgeHandle find_halfedge(VertexHandle from, VertexHandle to) const noexcept
    {
      return m_mesh->find_halfedge(from, to);
    }

    // --- mutating primitives (mutable specialization only) ------------------------------
    /** \brief Appends a vertex at \p position and returns its handle; sets no connectivity. */
    GEO_NODISCARD VertexHandle new_vertex(const vec_t& position) const
      requires(!is_const(C))
    {
      return m_mesh->add_vertex(position);
    }

    /** \brief Appends a default (unlinked) halfedge and returns its handle. */
    GEO_NODISCARD HalfedgeHandle new_halfedge() const
      requires(!is_const(C))
    {
      const HalfedgeHandle handle = Mesh::template make_handle<HalfedgeHandle>(m_mesh->m_halfedges.size());
      m_mesh->m_halfedges.push_back(Halfedge{});
      return handle;
    }

    /** \brief Appends an edge referencing \p halfedge and returns its handle. */
    GEO_NODISCARD EdgeHandle new_edge(HalfedgeHandle halfedge) const
      requires(!is_const(C))
    {
      const EdgeHandle handle = Mesh::template make_handle<EdgeHandle>(m_mesh->m_edges.size());
      m_mesh->m_edges.push_back(Edge{halfedge});
      return handle;
    }

    /** \brief Appends a default (unlinked) face and returns its handle. */
    GEO_NODISCARD FaceHandle new_face() const
      requires(!is_const(C))
    {
      const FaceHandle handle = Mesh::template make_handle<FaceHandle>(m_mesh->m_faces.size());
      m_mesh->m_faces.push_back(Face{});
      return handle;
    }

    /**
     * \brief Tombstones an element: sets its deleted flag and updates the live counts.
     *
     * Nothing is relinked -- the caller must first detach the element so that no live element still
     * references it, or \c has_valid_connectivity() fails and \c garbage_collection() asserts.
     * Deleting an edge deletes both of its halfedges. Precondition: not already deleted.
     */
    void mark_deleted(VertexHandle handle) const noexcept requires(!is_const(C)) { m_mesh->mark_deleted(handle); }
    void mark_deleted(EdgeHandle handle) const noexcept requires(!is_const(C)) { m_mesh->mark_deleted(handle); }
    void mark_deleted(FaceHandle handle) const noexcept requires(!is_const(C)) { m_mesh->mark_deleted(handle); }

    /**
     * \brief Makes \p next follow \p prev in a halfedge cycle, setting both directions of the link.
     *
     * The one place the next/prev reciprocity invariant is written, so removal operators cannot
     * update one side and forget the other.
     */
    void link(HalfedgeHandle prev, HalfedgeHandle next) const noexcept requires(!is_const(C))
    {
      m_mesh->get_halfedge(prev).next = next;
      m_mesh->get_halfedge(next).prev = prev;
    }

    /**
     * \brief Re-establishes the boundary representative rule for \p vertex after its fan changed: a
     * boundary vertex must store a boundary outgoing halfedge.
     *
     * Removal operators call this for every vertex whose fan they edited. Precondition: the stored
     * halfedge is a live outgoing halfedge of \p vertex. O(valence).
     */
    void restore_boundary_representative(VertexHandle vertex) const noexcept requires(!is_const(C))
    {
      HalfedgeHandle& stored = m_mesh->get_vertex(vertex).halfedge;
      GEO_ASSERT(stored.is_valid() && !m_mesh->is_deleted(stored));
      const HalfedgeHandle boundary = m_mesh->find_outgoing_boundary(stored);
      if (boundary.is_valid())
      {
        stored = boundary;
      }
    }

    // --- reserve / resize support for transactional rollback ----------------------------
    // reserve_* pre-grows storage so a transaction's appends never reallocate mid-way; resize_*
    // truncates back to a prior element count to undo the appends on rollback.
    void reserve_vertices(size_type additional) const requires(!is_const(C)) { detail::reserve_additional(m_mesh->m_vertices, additional); }
    void reserve_faces(size_type additional) const requires(!is_const(C)) { detail::reserve_additional(m_mesh->m_faces, additional); }
    void reserve_halfedges(size_type additional) const requires(!is_const(C)) { detail::reserve_additional(m_mesh->m_halfedges, additional); }
    void reserve_edges(size_type additional) const requires(!is_const(C)) { detail::reserve_additional(m_mesh->m_edges, additional); }

    void resize_faces(size_type count) const requires(!is_const(C)) { m_mesh->m_faces.resize(count); }
    void resize_halfedges(size_type count) const requires(!is_const(C)) { m_mesh->m_halfedges.resize(count); }
    void resize_edges(size_type count) const requires(!is_const(C)) { m_mesh->m_edges.resize(count); }
};

} // namespace Geometry

#endif // GEOMETRY_MESH_DETAIL_MESHCONNECTIVITY_HPP
