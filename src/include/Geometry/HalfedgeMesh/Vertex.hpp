#ifndef GLFWTESTAPP_VERTEX_H
#define GLFWTESTAPP_VERTEX_H

#include <Core/Utils/Compiler.hpp>
#include <Geometry/HalfedgeMesh/Halfedge.hpp>
#include <LinAl/LinearAlgebra.hpp>
#include <cstdio>

namespace Geometry
{

template <typename T>
class Halfedge;

template <typename T>
class HalfedgeMesh;

template <typename T>
class Vertex {
  public:
    CORE_CONSTEXPR Vertex(std::size_t meshPointIndex, HalfedgeMesh<T>* mesh) CORE_NOEXCEPT
        : m_index(meshPointIndex)
        , m_halfedgeIndex(INVALID_INDEX)
        , halfedgeMesh(mesh)
    {
    }

    CORE_CONSTEXPR Vertex(std::size_t meshPointIndex,
                          std::size_t halfedgeIndex,
                          HalfedgeMesh<T>* mesh) CORE_NOEXCEPT
        : m_index(meshPointIndex)
        , m_halfedgeIndex(halfedgeIndex)
        , halfedgeMesh(mesh)
    {
    }

    CORE_NODISCARD CORE_CONSTEXPR LinAl::Vec3<T> getPoint() const
    {
        return halfedgeMesh->getVertexPoints()[m_index];
    }

    CORE_NODISCARD CORE_CONSTEXPR std::size_t getIndex() const
    {
        return m_index;
    }

    CORE_NODISCARD CORE_CONSTEXPR const Halfedge<T>& getHalfedge() const
    {
        return halfedgeMesh->getHalfedges()[m_halfedgeIndex];
    }

    CORE_CONSTEXPR Halfedge<T>& getHalfedge()
    {
        return halfedgeMesh->getHalfedges()[m_halfedgeIndex];
    }

    CORE_NODISCARD CORE_CONSTEXPR std::size_t getHalfedgeIndex() const
    {
        return m_halfedgeIndex;
    }

    CORE_CONSTEXPR void setHalfedgeIndex(std::size_t halfedgeIndex)
    {
        m_halfedgeIndex = halfedgeIndex;
    }

    CORE_CONSTEXPR bool operator==(const Vertex& rhs) const
    {
        return m_index == rhs.m_index && m_halfedgeIndex == rhs.m_halfedgeIndex &&
               halfedgeMesh == rhs.halfedgeMesh;
    }

    CORE_CONSTEXPR bool operator!=(const Vertex& rhs) const
    {
        return !(rhs == *this);
    }

    CORE_NODISCARD CORE_CONSTEXPR bool isValid() const
    {
        if (m_index != INVALID_INDEX && m_halfedgeIndex != INVALID_INDEX &&
            halfedgeMesh->contains(*this))
            return true;
        return false;
    }

  private:
    std::size_t m_index;
    std::size_t m_halfedgeIndex;
    HalfedgeMesh<T>* halfedgeMesh;
};

} // namespace Geometry

#endif // GLFWTESTAPP_VERTEX_H
