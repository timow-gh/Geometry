#ifndef MESHFACE_HEADER
#define MESHFACE_HEADER

#include <Geometry/HalfedgeMesh/MeshIndexTraits.hpp>
#include <Geometry/HalfedgeMesh/MeshTraits.hpp>
#include <Geometry/Utils/Compiler.hpp>

namespace Geometry
{

template <typename TFloat, typename TIndex>
class Facet {
public:
  using HalfedgeIndex_t = typename MeshIndexTraits<TIndex>::HalfedgeIndex_t;

  using Halfedge_t = typename MeshTraits<TFloat, TIndex>::Halfedge_t;
  using HalfedgeMesh_t = HalfedgeMesh<TFloat, TIndex>;

  GEO_CONSTEXPR Facet(HalfedgeIndex_t halfedgeIndex, HalfedgeMesh_t& mesh) GEO_NOEXCEPT
      : m_heIndex(halfedgeIndex)
      , m_mesh(&mesh)
  {
  }

  GEO_NODISCARD GEO_CONSTEXPR const Halfedge_t& getHalfedge() const { return m_mesh->getHalfedge(m_heIndex); }
  GEO_CONSTEXPR Halfedge_t& getHalfedge() { return m_mesh->getHalfedge(m_heIndex); }

  GEO_NODISCARD GEO_CONSTEXPR HalfedgeIndex_t getHalfedgeIndex() const { return m_heIndex; }

  GEO_NODISCARD GEO_CONSTEXPR bool isValid() const { return m_heIndex.isValid() && m_mesh && m_mesh->contains(*this); }

  GEO_CONSTEXPR bool operator==(const Facet& rhs) const { return m_heIndex == rhs.m_heIndex && m_mesh == rhs.m_mesh; }
  GEO_CONSTEXPR bool operator!=(const Facet& rhs) const { return !(rhs == *this); }

private:
  HalfedgeIndex_t m_heIndex{};
  HalfedgeMesh_t* m_mesh{nullptr};
};

} // namespace Geometry

#endif