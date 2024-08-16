#ifndef MESHFACE_HEADER
#define MESHFACE_HEADER

#include "Geometry/HalfedgeMesh/MeshTraits.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include "Geometry/HalfedgeMesh/MeshTraits.hpp"

namespace Geometry
{

/**
 * @brief Facet class
 *
 * The Facet class represents a face in the mesh. It is defined by a single halfedge.
 * The halfedge is the first halfedge of the face, following the face in counter-clockwise order.
 *
 * @tparam TFloat
 * @tparam TIndex
 */
template <typename TMeshTraits>
class Facet {
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

  constexpr Facet(HalfedgeIndex_t halfedgeIndex, HalfedgeMesh_t& mesh) noexcept
      : m_heIndex(halfedgeIndex)
      , m_mesh(&mesh)
  {
  }

  GEO_NODISCARD constexpr const Halfedge_t& getHalfedge() const { return m_mesh->getHalfedge(m_heIndex); }
  constexpr Halfedge_t& getHalfedge() { return m_mesh->getHalfedge(m_heIndex); }

  GEO_NODISCARD constexpr HalfedgeIndex_t getHalfedgeIndex() const { return m_heIndex; }

  constexpr bool operator==(const Facet& rhs) const { return m_heIndex == rhs.m_heIndex && m_mesh == rhs.m_mesh; }
  constexpr bool operator!=(const Facet& rhs) const { return !(rhs == *this); }

  GEO_NODISCARD constexpr bool is_valid() const
  {
    bool isValid = is_valid_impl();
    GEO_ASSERT(isValid);
    return isValid;
  }

private:
  GEO_NODISCARD bool is_valid_impl() const { return m_heIndex.is_valid() && m_mesh; }

  HalfedgeIndex_t m_heIndex{};
  HalfedgeMesh_t* m_mesh{nullptr};
};

} // namespace Geometry

#endif
