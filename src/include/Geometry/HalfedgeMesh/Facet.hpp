#ifndef MESHFACE_HEADER
#define MESHFACE_HEADER

#include "Geometry/HalfedgeMesh/MeshIndexTraits.hpp"
#include "Geometry/HalfedgeMesh/MeshTraits.hpp"
#include "Geometry/Utils/Compiler.hpp"

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
template <typename TFloat, typename TIndex>
class Facet {
public:
  using HalfedgeIndex_t = typename MeshIndexTraits<TIndex>::HalfedgeIndex_t;

  using Halfedge_t = typename MeshTraits<TFloat, TIndex>::Halfedge_t;
  using HalfedgeMesh_t = HalfedgeMesh<TFloat, TIndex>;

  constexpr Facet(HalfedgeIndex_t halfedgeIndex, HalfedgeMesh_t& mesh) noexcept
      : m_heIndex(halfedgeIndex)
      , m_mesh(&mesh)
  {
  }

  GEO_NODISCARD constexpr const Halfedge_t& getHalfedge() const { return m_mesh->getHalfedge(m_heIndex); }
  constexpr Halfedge_t& getHalfedge() { return m_mesh->getHalfedge(m_heIndex); }

  GEO_NODISCARD constexpr HalfedgeIndex_t getHalfedgeIndex() const { return m_heIndex; }

  GEO_NODISCARD constexpr bool is_valid() const { return m_heIndex.is_valid() && m_mesh && m_mesh->contains(*this); }

  constexpr bool operator==(const Facet& rhs) const { return m_heIndex == rhs.m_heIndex && m_mesh == rhs.m_mesh; }
  constexpr bool operator!=(const Facet& rhs) const { return !(rhs == *this); }

private:
  HalfedgeIndex_t m_heIndex{};
  HalfedgeMesh_t* m_mesh{nullptr};
};

} // namespace Geometry

#endif
