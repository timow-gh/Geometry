#ifndef GEOMETRY_VERTEX_H
#define GEOMETRY_VERTEX_H

#include "Geometry/HalfedgeMesh/HalfedgeMesh.hpp"
#include "Geometry/HalfedgeMesh/MeshIndexTraits.hpp"
#include "Geometry/HalfedgeMesh/MeshTraits.hpp"
#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <algorithm>
#include <linal/vec.hpp>
#include <vector>

namespace Geometry
{

template <typename TFloat, typename TIndex>
class Vertex {
public:
  using VertexIndex_t = typename MeshIndexTraits<TIndex>::VertexIndex_t;
  using HalfedgeIndex_t = typename MeshIndexTraits<TIndex>::HalfedgeIndex_t;

  using Halfedge_t = typename MeshTraits<TFloat, TIndex>::Halfedge_t;
  using HalfedgeMesh_t = HalfedgeMesh<TFloat, TIndex>;

  constexpr Vertex(VertexIndex_t vertexIndex, HalfedgeMesh_t* mesh) noexcept
      : m_vIndex(vertexIndex)
      , m_mesh(mesh)
  {
  }
  constexpr Vertex(VertexIndex_t meshPointIndex, HalfedgeIndex_t halfedgeIndex, HalfedgeMesh_t* mesh) noexcept
      : m_vIndex(meshPointIndex)
      , m_heIndices(std::vector<HalfedgeIndex_t>{halfedgeIndex})
      , m_mesh(mesh)
  {
  }

  GEO_NODISCARD constexpr linal::vec3<TFloat> getVector() const { return m_mesh->getVector(*this); }
  GEO_NODISCARD constexpr VertexIndex_t getIndex() const { return m_vIndex; }
  GEO_NODISCARD constexpr const std::vector<HalfedgeIndex_t>& getHalfedgeIndices() const { return m_heIndices; }

  constexpr void addHalfedgeIndex(HalfedgeIndex_t halfedgeIndex)
  {
    m_heIndices.push_back(halfedgeIndex);
    std::sort(m_heIndices.begin(), m_heIndices.end());
  }

  GEO_NODISCARD std::vector<Halfedge_t> calcHalfedges() const
  {
    std::vector<Halfedge_t> halfedges;
    for (const auto& heIndex: m_heIndices)
    {
      halfedges.push_back(m_mesh->getHalfedge(heIndex));
    }
    return halfedges;
  }

  GEO_NODISCARD bool is_valid() const
  {
    bool isValid = is_valid_impl();
    GEO_ASSERT(isValid);
    return isValid;
  }

  constexpr bool operator==(const Vertex& rhs) const { return is_equal_impl(rhs); }
  constexpr bool operator!=(const Vertex& rhs) const { return !(rhs == *this); }

private:
  GEO_NODISCARD bool is_valid_impl() const
  {
    if (!m_mesh)
    {
      return false;
    }

    if (!m_vIndex.is_valid())
    {
      return false;
    }

    if (!std::is_sorted(m_heIndices.begin(), m_heIndices.end()))
    {
      return false;
    }

    for (const auto& heIndex: m_heIndices)
    {
      if (!heIndex.is_valid())
      {
        return false;
      }
    }

    return true;
  }

  [[nodiscard]] bool is_equal_impl(const Vertex& rhs) const
  {
    if (m_mesh != rhs.m_mesh)
    {
      return false;
    }

    if (m_vIndex != rhs.m_vIndex)
    {
      return false;
    }

    for (std::size_t i = 0; i < m_heIndices.size(); ++i)
    {
      if (m_heIndices[i] != rhs.m_heIndices[i])
      {
        return false;
      }
    }

    return true;
  }

  VertexIndex_t m_vIndex{};
  std::vector<HalfedgeIndex_t> m_heIndices{};
  HalfedgeMesh_t* m_mesh{nullptr};
};

} // namespace Geometry

#endif // GEOMETRY_VERTEX_H
