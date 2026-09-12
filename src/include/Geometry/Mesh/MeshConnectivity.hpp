#ifndef GEOMETRY_MESH_MESHCONNECTIVITY_HPP
#define GEOMETRY_MESH_MESHCONNECTIVITY_HPP

#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include "Geometry/Utils/Constness.hpp"
#include <array>
#include <type_traits>
#include <vector>

namespace Geometry
{

// Low-level connectivity kernel handle for TriangleHalfedgeMesh.
//
// This is the "unchecked" tier of the mesh API: it exposes the raw element-creation and
// link-setting primitives that write algorithms need to build connectivity. None of
// these primitives validate mesh invariants -- callers are responsible for leaving the
// mesh in a consistent state.
//
// Const-correctness: the const specialization (C == Constness::Const) only exposes const
// element access and read-only map lookups. Mutators live on the mutable specialization.
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

    using DirectedEdgeKey = typename Mesh::DirectedEdgeKey;
    using FaceKey = typename Mesh::FaceKey;

    using VertexRef = qualified_ref_t<C, Vertex>;
    using HalfedgeRef = qualified_ref_t<C, Halfedge>;
    using FaceRef = qualified_ref_t<C, Face>;
    using EdgeRef = qualified_ref_t<C, Edge>;

    constexpr MeshConnectivityView() noexcept = default;
    constexpr explicit MeshConnectivityView(MeshPtr mesh) noexcept
        : m_mesh(mesh) {}

    // Non-const -> const conversion only.
    template <Constness Other>
    constexpr MeshConnectivityView(const MeshConnectivityView<Mesh, Other>& other) noexcept
        requires(is_const(C) && !is_const(Other))
        : m_mesh(other.m_mesh) {}

    // --- element counts -----------------------------------------------------------------
    GEO_NODISCARD size_type vertex_count() const noexcept { return m_mesh->m_vertices.size(); }
    GEO_NODISCARD size_type halfedge_count() const noexcept { return m_mesh->m_halfedges.size(); }
    GEO_NODISCARD size_type face_count() const noexcept { return m_mesh->m_faces.size(); }
    GEO_NODISCARD size_type edge_count() const noexcept { return m_mesh->m_edges.size(); }

    // --- raw element access -------------------------------------------------------------
    GEO_NODISCARD VertexRef vertex(VertexHandle handle) const noexcept { return m_mesh->get_vertex(handle); }
    GEO_NODISCARD HalfedgeRef halfedge(HalfedgeHandle handle) const noexcept { return m_mesh->get_halfedge(handle); }
    GEO_NODISCARD FaceRef face(FaceHandle handle) const noexcept { return m_mesh->get_face(handle); }
    GEO_NODISCARD EdgeRef edge(EdgeHandle handle) const noexcept { return m_mesh->get_edge(handle); }

    GEO_NODISCARD bool contains(HalfedgeHandle handle) const noexcept { return m_mesh->contains(handle); }
    GEO_NODISCARD bool contains(VertexHandle handle) const noexcept { return m_mesh->contains(handle); }

    // --- directed-edge / face-key maps --------------------------------------------------
    GEO_NODISCARD HalfedgeHandle find_directed_edge(const DirectedEdgeKey& key) const noexcept
    {
      auto const it = m_mesh->m_directedEdges.find(key);
      return it != m_mesh->m_directedEdges.end() ? it->second : HalfedgeHandle{};
    }

    GEO_NODISCARD bool contains_face_key(const FaceKey& key) const noexcept
    {
      return m_mesh->m_faceKeys.find(key) != m_mesh->m_faceKeys.end();
    }

    GEO_NODISCARD static FaceKey make_face_key(const std::array<VertexHandle, 3>& vertices) noexcept
    {
      return Mesh::make_face_key(vertices);
    }

    // --- mutating primitives (mutable specialization only) ------------------------------
    GEO_NODISCARD VertexHandle new_vertex(const vec_t& position) const
      requires(!is_const(C))
    {
      return m_mesh->add_vertex(position);
    }

    GEO_NODISCARD HalfedgeHandle new_halfedge() const
      requires(!is_const(C))
    {
      HalfedgeHandle const handle = Mesh::template make_handle<HalfedgeHandle>(m_mesh->m_halfedges.size());
      m_mesh->m_halfedges.push_back(Halfedge{});
      return handle;
    }

    GEO_NODISCARD EdgeHandle new_edge(HalfedgeHandle halfedge) const
      requires(!is_const(C))
    {
      EdgeHandle const handle = Mesh::template make_handle<EdgeHandle>(m_mesh->m_edges.size());
      m_mesh->m_edges.push_back(Edge{halfedge});
      return handle;
    }

    GEO_NODISCARD FaceHandle new_face() const
      requires(!is_const(C))
    {
      FaceHandle const handle = Mesh::template make_handle<FaceHandle>(m_mesh->m_faces.size());
      m_mesh->m_faces.push_back(Face{});
      return handle;
    }

    void insert_directed_edge(const DirectedEdgeKey& key, HalfedgeHandle halfedge) const
      requires(!is_const(C))
    {
      m_mesh->m_directedEdges.emplace(key, halfedge);
    }

    void erase_directed_edge(const DirectedEdgeKey& key) const
      requires(!is_const(C))
    {
      m_mesh->m_directedEdges.erase(key);
    }

    void insert_face_key(const FaceKey& key) const
      requires(!is_const(C))
    {
      m_mesh->m_faceKeys.insert(key);
    }

    void erase_face_key(const FaceKey& key) const
      requires(!is_const(C))
    {
      m_mesh->m_faceKeys.erase(key);
    }

    // --- reserve / resize support for transactional rollback ----------------------------
    void reserve_faces(size_type additional) const requires(!is_const(C)) { m_mesh->m_faces.reserve(m_mesh->m_faces.size() + additional); }
    void reserve_halfedges(size_type additional) const requires(!is_const(C)) { m_mesh->m_halfedges.reserve(m_mesh->m_halfedges.size() + additional); }
    void reserve_edges(size_type additional) const requires(!is_const(C)) { m_mesh->m_edges.reserve(m_mesh->m_edges.size() + additional); }
    void reserve_directed_edges(size_type additional) const requires(!is_const(C))
    {
      m_mesh->m_directedEdges.reserve(m_mesh->m_directedEdges.size() + additional);
    }
    void reserve_face_keys(size_type additional) const requires(!is_const(C))
    {
      m_mesh->m_faceKeys.reserve(m_mesh->m_faceKeys.size() + additional);
    }

    void resize_faces(size_type count) const requires(!is_const(C)) { m_mesh->m_faces.resize(count); }
    void resize_halfedges(size_type count) const requires(!is_const(C)) { m_mesh->m_halfedges.resize(count); }
    void resize_edges(size_type count) const requires(!is_const(C)) { m_mesh->m_edges.resize(count); }
};

} // namespace Geometry

#endif // GEOMETRY_MESH_MESHCONNECTIVITY_HPP
