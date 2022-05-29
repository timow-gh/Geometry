#ifndef MESHFACE_HEADER
#define MESHFACE_HEADER

#include <Core/Utils/Compiler.hpp>
#include <Geometry/HalfedgeMesh/MeshIndexTraits.hpp>
#include <Geometry/HalfedgeMesh/MeshTraits.hpp>

namespace Geometry
{

template <typename TFloatType, typename TIndexType>
class Facet {
  public:
    using HalfedgeIndex_t = typename MeshIndexTraits<TIndexType>::HalfedgeIndex_t;

    using Halfedge_t = typename MeshTraits<TFloatType, TIndexType>::Halfedge_t;
    using HalfedgeMesh_t = HalfedgeMesh<TFloatType, TIndexType>;

    CORE_CONSTEXPR Facet(HalfedgeIndex_t halfedgeIndex, HalfedgeMesh_t& mesh) CORE_NOEXCEPT
        : m_heIndex(halfedgeIndex)
        , m_mesh(&mesh)
    {
    }

    CORE_NODISCARD CORE_CONSTEXPR const Halfedge_t& getHalfedge() const { return m_mesh->getHalfedges()[m_heIndex]; }
    CORE_CONSTEXPR Halfedge_t& getHalfedge() { return m_mesh->getHalfedges()[m_heIndex]; }

    CORE_NODISCARD CORE_CONSTEXPR HalfedgeIndex_t getHalfedgeIndex() const { return m_heIndex; }

    CORE_NODISCARD CORE_CONSTEXPR bool isValid() const { return m_heIndex.isValid() && m_mesh && m_mesh->contains(*this); }

    CORE_CONSTEXPR bool operator==(const Facet& rhs) const { return m_heIndex == rhs.m_heIndex && m_mesh == rhs.m_mesh; }
    CORE_CONSTEXPR bool operator!=(const Facet& rhs) const { return !(rhs == *this); }

  private:
    HalfedgeIndex_t m_heIndex{};
    HalfedgeMesh_t* m_mesh{nullptr};
};

} // namespace Geometry

#endif