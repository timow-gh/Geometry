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

namespace Geometry
{

template <typename T>
class Vertex;
template <typename T>
class Halfedge;
template <typename T>
class Facet;

template <typename T>
class HalfedgeMesh
{
    Core::TVector<Vertex<T>> m_vertices;
    Core::TVector<Halfedge<T>> m_halfedges;
    Core::TVector<Facet<T>> m_facets;
    MeshPoints<T> m_triangleMesh;
    xg::Guid m_guid;

  public:
    static std::unique_ptr<HalfedgeMesh<T>> create();

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

    void addTriangle(const Triangle<T, 3>& triangle);

  private:
    explicit HalfedgeMesh(const xg::Guid& guid);
};

template <typename T>
std::unique_ptr<HalfedgeMesh<T>> HalfedgeMesh<T>::create()
{
    auto guid = xg::newGuid();
    return std::make_unique<Geometry::HalfedgeMesh<T>>(
        Geometry::HalfedgeMesh<T>(guid));
}

template <typename T>
LinAl::Vec3Vector<T>& HalfedgeMesh<T>::getVertexPoints()
{
    return m_triangleMesh.getPoints();
}
template <typename T>
const LinAl::Vec3Vector<T>& HalfedgeMesh<T>::getVertexPoints() const
{
    return m_triangleMesh.getPoints();
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
    return m_triangleMesh;
}

template <typename T>
const MeshPoints<T>& HalfedgeMesh<T>::getTriangleMesh() const
{
    return m_triangleMesh;
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
    return m_triangleMesh.contains(vector);
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

template <typename T>
void HalfedgeMesh<T>::addTriangle(const Triangle<T, 3>& triangle)
{
    // Create or find the Vertex of the LinAl::Vec3
    const LinAl::VecArray<T, 3, 3> trianglePoints =
        triangle.getTrianglePoints();
    Core::TArray<std::size_t, 3> vertexIndices;
    for (std::size_t i = 0; i < 3; ++i)
    {
        std::size_t vertexIndex;
        if (!m_triangleMesh.contains(trianglePoints[i], vertexIndex))
        {
            vertexIndex = m_triangleMesh.add(trianglePoints[i]);
            m_vertices.emplace_back(vertexIndex, this);
        }
        vertexIndices[i] = vertexIndex;
    }

    std::size_t halfedgeIndex =
        m_halfedges.size() == 0 ? 0 : m_halfedges.size();

    // Create the Halfedges and set the Halfedges for Vertices
    for (const std::size_t vIndex: vertexIndices)
    {
        m_halfedges.emplace_back(vIndex, this);
        if (m_vertices[vIndex].getHalfedgeIndex() == INVALID_INDEX)
            m_vertices[vIndex].setHalfedgeIndex(m_halfedges.size() - 1);
    }

    m_facets.emplace_back(halfedgeIndex, this);

    // Fill the facet, the next and the previous pointer for each Halfedge of
    // the Facet
    const std::size_t facetIdx = m_facets.size() - 1;
    for (std::size_t i = halfedgeIndex; i < m_halfedges.size(); ++i)
    {
        m_halfedges[i].setFacet(facetIdx);
        auto nextHeIndex = i == m_halfedges.size() - 1 ? halfedgeIndex : i + 1;
        m_halfedges[i].setNextIndex(nextHeIndex);
        auto previousHeIndex =
            i == halfedgeIndex ? m_halfedges.size() - 1 : i - 1;
        m_halfedges[i].setPreviousIndex(previousHeIndex);
    }

    // Find and set the opposite Halfedges for each Halfedge of the Facet
    for (std::size_t i = halfedgeIndex; i < m_halfedges.size(); ++i)
    {
        Halfedge<T>& halfedge = m_halfedges[i];

        Halfedge<T>* he = halfedge.vertex().halfedge();
        if (!he)
            continue;

        Halfedge<T>* oppHeCandidate = he->previous();
        std::size_t oppHeCandidateIndex =
            halfedge.vertex().halfedge()->getPreviousIndex();

        if (!oppHeCandidate)
            continue;

        auto nextVertex = halfedge.nextVertex();
        auto oppNextVertex = oppHeCandidate->nextVertex();

        if (!nextVertex || !oppNextVertex)
            continue;

        if (halfedge.vertex() == *oppNextVertex &&
            *nextVertex == oppHeCandidate->vertex())
        {
            halfedge.setOppositeIndex(oppHeCandidateIndex);
            oppHeCandidate->setOppositeIndex(i);
        }
    }
}

template <typename T>
HalfedgeMesh<T>::HalfedgeMesh(const xg::Guid& guid) : m_guid(guid)
{
}

} // namespace Geometry

#endif // GLFWTESTAPP_HALFEDGEMESH_H
