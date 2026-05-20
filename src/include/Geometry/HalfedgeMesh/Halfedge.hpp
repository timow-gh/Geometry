#ifndef GEOMETRY_HALFEDGE_H
#define GEOMETRY_HALFEDGE_H

#include "Geometry/HalfedgeMesh/MeshTraits.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include "Geometry/HalfedgeMesh/MeshTraits.hpp"
#include <algorithm>

namespace Geometry
{

template <typename TMeshTraits>
class Halfedge {
public:
  using value_type = typename TMeshTraits::value_type;
  using index_type = typename TMeshTraits::index_type;

  using VertexIndex_t = typename TMeshTraits::VertexIndex_t;
  using HalfedgeIndex_t = typename TMeshTraits::HalfedgeIndex_t;
  using FacetIndex_t = typename TMeshTraits::FacetIndex_t;

  using Vertex_t = typename TMeshTraits::Vertex_t;
  using Halfedge_t = typename TMeshTraits::Halfedge_t;
  using Facet_t = typename TMeshTraits::Facet_t;

  using HalfedgeMesh_t = typename TMeshTraits::HalfedgeMesh_t;

  /** @brief Creates a halfedge that must be completed later.
   *
   * Intended for use during the building of the mesh, where the complete of a halfedge is not yet known.
   *
   * @param halfedgeIndex
   * @param vertexIndex
   * @param mesh
   */
  constexpr Halfedge(HalfedgeIndex_t halfedgeIndex, VertexIndex_t vertexIndex, HalfedgeMesh_t* mesh) noexcept
      : m_halfedgeIdx(halfedgeIndex)
      , m_vertexIdx(vertexIndex)
      , m_mesh(mesh)
  {
  }

  GEO_NODISCARD
  constexpr const Facet_t& get_facet() const { return m_mesh->get_facets()[m_facetIdx.get_value()]; }

  GEO_NODISCARD constexpr const Vertex_t& get_vertex() const { return m_mesh->get_vertices()[m_vertexIdx.get_value()]; }
  GEO_NODISCARD constexpr Vertex_t& get_vertex() { return m_mesh->get_vertices()[m_vertexIdx.get_value()]; }
  GEO_NODISCARD constexpr VertexIndex_t get_vertex_index() const { return m_vertexIdx; }
  GEO_NODISCARD constexpr const Vertex_t& get_next_vertex() const { return get_next().get_vertex(); }
  GEO_NODISCARD constexpr Vertex_t& get_next_vertex() { return get_next().get_vertex(); }

  GEO_NODISCARD constexpr HalfedgeIndex_t get_halfedge_index() const { return m_halfedgeIdx; }
  GEO_NODISCARD constexpr HalfedgeIndex_t get_next_index() const { return m_nextIdx; }
  GEO_NODISCARD constexpr HalfedgeIndex_t get_previous_index() const { return m_previousIdx; }
  GEO_NODISCARD constexpr HalfedgeIndex_t get_opposite_index() const { return m_oppositeIdx; }
  GEO_NODISCARD constexpr FacetIndex_t get_facet_index() const { return m_facetIdx; }

  GEO_NODISCARD constexpr const Halfedge_t& get_next() const { return m_mesh->get_halfedges()[m_nextIdx.get_value()]; }
  GEO_NODISCARD constexpr Halfedge_t& get_next() { return m_mesh->get_halfedges()[m_nextIdx.get_value()]; }
  GEO_NODISCARD constexpr const Halfedge_t& get_previous() const { return m_mesh->get_halfedges()[m_previousIdx.get_value()]; }
  GEO_NODISCARD constexpr Halfedge_t& get_previous() { return m_mesh->get_halfedges()[m_previousIdx.get_value()]; }
  GEO_NODISCARD constexpr const Halfedge_t& get_opposite() const { return m_mesh->get_halfedges()[m_oppositeIdx.get_value()]; }

  constexpr void set_next_index(HalfedgeIndex_t index) { m_nextIdx = index; }
  constexpr void set_previous_index(HalfedgeIndex_t index) { m_previousIdx = index; }
  constexpr void set_opposite_index(HalfedgeIndex_t index) { m_oppositeIdx = index; }
  constexpr void set_facet_index(FacetIndex_t facetIndex) { m_facetIdx = facetIndex; }
  constexpr void set_vertex_index(VertexIndex_t index) { m_vertexIdx = index; }

  constexpr bool operator==(const Halfedge& rhs) const { return m_halfedgeIdx == rhs.m_halfedgeIdx; }
  constexpr bool operator!=(const Halfedge& rhs) const { return !(rhs == *this); }

  GEO_NODISCARD constexpr bool is_valid() const
  {
    bool isValid = is_valid_halfedge_impl();
    GEO_ASSERT(isValid);
    return isValid;
  }

private:
  GEO_NODISCARD constexpr bool is_valid_halfedge_impl() const
  {
    if (!(m_halfedgeIdx.is_valid() && m_vertexIdx.is_valid() && m_facetIdx.is_valid() && m_nextIdx.is_valid() && m_previousIdx.is_valid() &&
          m_oppositeIdx.is_valid() && m_mesh != nullptr))
    {
      return false;
    }

    const Vertex_t& vertex = get_vertex();
    const auto& heIndices = vertex.get_halfedge_indices();
    if (std::find(heIndices.begin(), heIndices.end(), m_halfedgeIdx) == heIndices.end())
    {
      return false;
    }

    Halfedge_t oppHe = get_opposite();
    if (oppHe.m_oppositeIdx != m_halfedgeIdx)
    {
      return false;
    }

    Halfedge_t nextHe = get_next();
    if (nextHe.m_previousIdx != m_halfedgeIdx)
    {
      return false;
    }

    Halfedge_t prevHe = get_previous();
    return static_cast<bool>(prevHe.m_nextIdx == m_halfedgeIdx);
  }

  HalfedgeIndex_t m_halfedgeIdx{};
  FacetIndex_t m_facetIdx{};
  VertexIndex_t m_vertexIdx{};
  HalfedgeIndex_t m_nextIdx{};
  HalfedgeIndex_t m_previousIdx{};
  HalfedgeIndex_t m_oppositeIdx{};
  HalfedgeMesh_t* m_mesh{nullptr};
};

} // namespace Geometry

#endif // GEOMETRY_HALFEDGE_H
