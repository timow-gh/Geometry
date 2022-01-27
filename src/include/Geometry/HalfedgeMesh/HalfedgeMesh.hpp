#ifndef GLFWTESTAPP_HALFEDGEMESH_H
#define GLFWTESTAPP_HALFEDGEMESH_H

#include <Core/Types/TArray.hpp>
#include <Core/Types/TVector.hpp>
#include <Core/Utils/Compiler.hpp>
#include <CrossGuid/Guid.hpp>
#include <Geometry/HalfedgeMesh/Facet.hpp>
#include <Geometry/HalfedgeMesh/FacetUtils.hpp>
#include <Geometry/HalfedgeMesh/Halfedge.hpp>
#include <Geometry/HalfedgeMesh/MeshPoints.hpp>
#include <Geometry/HalfedgeMesh/Vertex.hpp>
#include <Geometry/Triangle.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename T>
class Vertex;
template <typename T>
class Halfedge;
template <typename T>
class Facet;

template <typename T>
class SphereMeshBuilder;

template <typename T>
class HalfedgeMesh {
  public:
    using IndexType = std::size_t;

    CORE_CONSTEXPR HalfedgeMesh() = default;
    CORE_CONSTEXPR explicit HalfedgeMesh(const HalfedgeMesh& rhs) = delete;
    CORE_CONSTEXPR HalfedgeMesh& operator=(const HalfedgeMesh& rhs) = delete;

    CORE_CONSTEXPR HalfedgeMesh(HalfedgeMesh&& rhs) CORE_NOEXCEPT = default;
    CORE_CONSTEXPR HalfedgeMesh& operator=(HalfedgeMesh&& rhs) CORE_NOEXCEPT = default;

    CORE_NODISCARD CORE_CONSTEXPR LinAl::Vec3Vector<T>& getVertexPoints()
    {
        return m_meshPoints.getPoints();
    }

    CORE_NODISCARD CORE_CONSTEXPR const LinAl::Vec3Vector<T>& getVertexPoints() const
    {
        return m_vertices.getPoints();
    }

    CORE_NODISCARD CORE_CONSTEXPR Core::TVector<Vertex<T>>& getVertices()
    {
        return m_vertices;
    }

    CORE_NODISCARD CORE_CONSTEXPR const Core::TVector<Vertex<T>>& getVertices() const
    {
        return m_vertices;
    }

    CORE_NODISCARD CORE_CONSTEXPR Core::TVector<Halfedge<T>>& getHalfedges()
    {
        return m_halfedges;
    }

    CORE_NODISCARD CORE_CONSTEXPR const Core::TVector<Halfedge<T>>& getHalfedges() const
    {
        return m_halfedges;
    }

    CORE_NODISCARD CORE_CONSTEXPR Core::TVector<Facet<T>>& getFacets()
    {
        return m_facets;
    }

    CORE_NODISCARD CORE_CONSTEXPR const Core::TVector<Facet<T>>& getFacets() const
    {
        return m_facets;
    }

    CORE_NODISCARD CORE_CONSTEXPR MeshPoints<T>& getMeshPoints()
    {
        return m_meshPoints;
    }

    CORE_NODISCARD CORE_CONSTEXPR const MeshPoints<T>& getMeshPoings() const
    {
        return m_meshPoints;
    }

    CORE_NODISCARD CORE_CONSTEXPR Vertex<T> getVertex(std::size_t index) const
    {
        return m_vertices[index];
    }

    CORE_NODISCARD CORE_CONSTEXPR Halfedge<T> getHalfedge(std::size_t index) const
    {
        return m_halfedges[index];
    }

    CORE_NODISCARD CORE_CONSTEXPR Facet<T> getFacet(std::size_t index) const
    {
        return m_facets[index];
    }

    CORE_NODISCARD CORE_CONSTEXPR bool contains(const Vertex<T>& vertex) const
    {
        for (const Vertex<T>& v: m_vertices)
            if (vertex == v)
                return true;
        return false;
    }

    CORE_NODISCARD CORE_CONSTEXPR bool contains(const Halfedge<T>& halfedge) const
    {
        for (const Halfedge<T>& he: m_halfedges)
            if (halfedge == he)
                return true;
        return false;
    }

    CORE_NODISCARD CORE_CONSTEXPR bool contains(const Facet<T>& facet) const
    {
        for (const Facet<T>& f: m_facets)
            if (facet == f)
                return true;
        return false;
    }

    CORE_NODISCARD CORE_CONSTEXPR bool contains(const LinAl::Vec3<T>& vector) const
    {
        return m_meshPoints.contains(vector);
    }

    friend SphereMeshBuilder<T>;

  private:
    Core::TVector<Vertex<T>> m_vertices;
    Core::TVector<Halfedge<T>> m_halfedges;
    Core::TVector<Facet<T>> m_facets;
    MeshPoints<T> m_meshPoints;
};

template <typename T, typename U>
CORE_CONSTEXPR Core::TVector<U> calcTriangleIndices(const Core::TVector<Facet<T>>& facets)
{
    Core::TVector<U> result;
    for (const auto& facet: facets)
    {
        for (const Geometry::Halfedge<T>* halfedge: Geometry::calcHalfedges(facet))
        {
            result.push_back(static_cast<U>(halfedge->getVertexIndex()));
        }
    }
    return result;
}

} // namespace Geometry

#endif // GLFWTESTAPP_HALFEDGEMESH_H
