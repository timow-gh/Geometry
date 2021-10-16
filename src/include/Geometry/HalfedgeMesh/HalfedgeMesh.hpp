#ifndef GLFWTESTAPP_HALFEDGEMESH_H
#define GLFWTESTAPP_HALFEDGEMESH_H

#include "CrossGuid/Guid.hpp"
#include "Geometry/HalfedgeMesh/Facet.hpp"
#include "Geometry/HalfedgeMesh/FacetUtils.hpp"
#include "Geometry/HalfedgeMesh/Halfedge.hpp"
#include "Geometry/HalfedgeMesh/MeshPoints.hpp"
#include "Geometry/HalfedgeMesh/Vertex.hpp"
#include "Geometry/Triangle.hpp"
#include "LinAl/LinearAlgebra.hpp"
#include <Core/Types/TArray.hpp>
#include <Core/Types/TVector.hpp>

namespace Geometry
{

template <typename T>
class Vertex;
template <typename T>
class Halfedge;
template <typename T>
class Facet;

class HalfedgeMeshBuilder;

template <typename T>
class HalfedgeMesh
{
    Core::TVector<Vertex<T>> m_vertices;
    Core::TVector<Halfedge<T>> m_halfedges;
    Core::TVector<Facet<T>> m_facets;
    MeshPoints<T> meshPoints;
    xg::Guid m_guid;

  public:
    explicit HalfedgeMesh(const xg::Guid& guid);
    // TODO Build HeMesh from a vector of points.
    // explicit HalfedgeMesh(Core::TVector<LinAl::Vec3<T>>&& points);

    HalfedgeMesh() = delete;
    explicit HalfedgeMesh(const HalfedgeMesh& rhs) = delete;
    HalfedgeMesh& operator=(const HalfedgeMesh& rhs) = delete;

    explicit HalfedgeMesh(HalfedgeMesh&& rhs) noexcept = default;
    HalfedgeMesh& operator=(HalfedgeMesh&& rhs) noexcept = default;

    [[nodiscard]] LinAl::Vec3Vector<T>& getVertexPoints();
    [[nodiscard]] const LinAl::Vec3Vector<T>& getVertexPoints() const;

    [[nodiscard]] Core::TVector<Vertex<T>>& getVertices();
    [[nodiscard]] const Core::TVector<Vertex<T>>& getVertices() const;

    [[nodiscard]] Core::TVector<Halfedge<T>>& getHalfedges();
    [[nodiscard]] const Core::TVector<Halfedge<T>>& getHalfedges() const;

    [[nodiscard]] Core::TVector<Facet<T>>& getFacets();
    [[nodiscard]] const Core::TVector<Facet<T>>& getFacets() const;

    [[nodiscard]] MeshPoints<T>& getTriangleMesh();
    [[nodiscard]] const MeshPoints<T>& getTriangleMesh() const;

    [[nodiscard]] Vertex<T> getVertex(std::size_t index) const;
    [[nodiscard]] Halfedge<T> getHalfedge(std::size_t index) const;
    [[nodiscard]] Facet<T> getFacet(std::size_t index) const;

    [[nodiscard]] bool contains(const Vertex<T>& vertex) const;
    [[nodiscard]] bool contains(const Halfedge<T>& halfedge) const;
    [[nodiscard]] bool contains(const Facet<T>& facet) const;
    [[nodiscard]] bool contains(const LinAl::Vec3<T>& point) const;

    [[nodiscard]] const xg::Guid& getGuid() const;

    friend HalfedgeMeshBuilder;
};

template <typename T>
HalfedgeMesh<T>::HalfedgeMesh(const xg::Guid& guid) : m_guid(guid)
{
}

template <typename T>
LinAl::Vec3Vector<T>& HalfedgeMesh<T>::getVertexPoints()
{
    return meshPoints.getPoints();
}
template <typename T>
const LinAl::Vec3Vector<T>& HalfedgeMesh<T>::getVertexPoints() const
{
    return meshPoints.getPoints();
}

template <typename T>
Core::TVector<Geometry::Vertex<T>>& HalfedgeMesh<T>::getVertices()
{
    return m_vertices;
}

template <typename T>
const Core::TVector<Geometry::Vertex<T>>& HalfedgeMesh<T>::getVertices() const
{
    return m_vertices;
}

template <typename T>
const Core::TVector<Geometry::Halfedge<T>>&
HalfedgeMesh<T>::getHalfedges() const
{
    return m_halfedges;
}

template <typename T>
Core::TVector<Geometry::Halfedge<T>>& HalfedgeMesh<T>::getHalfedges()
{
    return m_halfedges;
}

template <typename T>
Core::TVector<Geometry::Facet<T>>& HalfedgeMesh<T>::getFacets()
{
    return m_facets;
}

template <typename T>
const Core::TVector<Geometry::Facet<T>>& HalfedgeMesh<T>::getFacets() const
{
    return m_facets;
}

template <typename T>
MeshPoints<T>& HalfedgeMesh<T>::getTriangleMesh()
{
    return meshPoints;
}

template <typename T>
const MeshPoints<T>& HalfedgeMesh<T>::getTriangleMesh() const
{
    return meshPoints;
}

template <typename T>
Vertex<T> HalfedgeMesh<T>::getVertex(std::size_t index) const
{
    return m_vertices[index];
}
template <typename T>
Halfedge<T> HalfedgeMesh<T>::getHalfedge(std::size_t index) const
{
    return m_halfedges[index];
}
template <typename T>
Facet<T> HalfedgeMesh<T>::getFacet(std::size_t index) const
{
    return m_facets[index];
}

template <typename T>
bool HalfedgeMesh<T>::contains(const Vertex<T>& vertex) const
{
    for (const Vertex<T>& v: m_vertices)
        if (vertex == v)
            return true;
    return false;
}

template <typename T>
bool HalfedgeMesh<T>::contains(const Halfedge<T>& halfedge) const
{
    for (const Halfedge<T>& he: m_halfedges)
        if (halfedge == he)
            return true;
    return false;
}

template <typename T>
bool HalfedgeMesh<T>::contains(const Facet<T>& facet) const
{
    for (const Facet<T>& f: m_facets)
        if (facet == f)
            return true;
    return false;
}

template <typename T>
bool HalfedgeMesh<T>::contains(const LinAl::Vec3<T>& vector) const
{
    return meshPoints.contains(vector);
}

template <typename T, typename U>
Core::TVector<U> calcTriangleIndices(const Core::TVector<Facet<T>>& facets)
{
    Core::TVector<U> result;
    for (const auto& facet: facets)
    {
        for (const Geometry::Halfedge<T>* halfedge:
             Geometry::calcHalfedges(facet))
        {
            result.push_back(static_cast<U>(halfedge->getVertexIndex()));
        }
    }
    return result;
}

template <typename T>
const xg::Guid& HalfedgeMesh<T>::getGuid() const
{
    return m_guid;
}
} // namespace Geometry

#endif // GLFWTESTAPP_HALFEDGEMESH_H
