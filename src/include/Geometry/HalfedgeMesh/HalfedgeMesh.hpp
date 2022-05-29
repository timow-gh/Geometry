#ifndef GLFWTESTAPP_HALFEDGEMESH_H
#define GLFWTESTAPP_HALFEDGEMESH_H

#include <Core/Types/TArray.hpp>
#include <Core/Types/TVector.hpp>
#include <Core/Utils/Compiler.hpp>
#include <CrossGuid/Guid.hpp>
#include <Geometry/HalfedgeMesh/CalcFaceHalfedges.hpp>
#include <Geometry/HalfedgeMesh/Facet.hpp>
#include <Geometry/HalfedgeMesh/Halfedge.hpp>
#include <Geometry/HalfedgeMesh/MeshIndexTraits.hpp>
#include <Geometry/HalfedgeMesh/MeshPoints.hpp>
#include <Geometry/HalfedgeMesh/Vertex.hpp>
#include <Geometry/Triangle.hpp>
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename T>
class SphereMeshBuilder;

template <typename TFloatType = std::double_t, typename TIndexType = std::size_t>
class HalfedgeMesh {
  public:
    using Vertex_t = Vertex<TFloatType, TIndexType>;
    using Halfedge_t = Halfedge<TFloatType, TIndexType>;
    using Facet_t = Facet<TFloatType, TIndexType>;

    CORE_CONSTEXPR HalfedgeMesh() = default;
    CORE_CONSTEXPR explicit HalfedgeMesh(const HalfedgeMesh& rhs) = delete;
    CORE_CONSTEXPR HalfedgeMesh& operator=(const HalfedgeMesh& rhs) = delete;

    CORE_CONSTEXPR HalfedgeMesh(HalfedgeMesh&& rhs) CORE_NOEXCEPT = default;
    CORE_CONSTEXPR HalfedgeMesh& operator=(HalfedgeMesh&& rhs) CORE_NOEXCEPT = default;

//    CORE_NODISCARD CORE_CONSTEXPR LinAl::Vec3Vector<TFloatType>& getVertexPoints() { return m_meshPoints.getPoints(); }
//    CORE_NODISCARD CORE_CONSTEXPR const LinAl::Vec3Vector<TFloatType>& getVertexPoints() const
//    {
//        return m_meshPoints.getPoints();
//    }

    CORE_NODISCARD CORE_CONSTEXPR Core::TVector<Vertex_t>& getVertices() { return m_vertices; }
    CORE_NODISCARD CORE_CONSTEXPR const Core::TVector<Vertex_t>& getVertices() const { return m_vertices; }

    CORE_NODISCARD CORE_CONSTEXPR Core::TVector<Halfedge_t>& getHalfedges() { return m_halfedges; }
    CORE_NODISCARD CORE_CONSTEXPR const Core::TVector<Halfedge_t>& getHalfedges() const { return m_halfedges; }

    CORE_NODISCARD CORE_CONSTEXPR Core::TVector<Facet_t>& getFacets() { return m_facets; }
    CORE_NODISCARD CORE_CONSTEXPR const Core::TVector<Facet_t>& getFacets() const { return m_facets; }

    CORE_NODISCARD CORE_CONSTEXPR MeshPoints<TFloatType>& getMeshPoints() { return m_meshPoints; }
    CORE_NODISCARD CORE_CONSTEXPR const MeshPoints<TFloatType>& getMeshPoints() const { return m_meshPoints; }

    CORE_NODISCARD CORE_CONSTEXPR Vertex_t getVertex(std::size_t index) const { return m_vertices[index]; }
    CORE_NODISCARD CORE_CONSTEXPR Halfedge_t getHalfedge(std::size_t index) const { return m_halfedges[index]; }
    CORE_NODISCARD CORE_CONSTEXPR Facet_t getFacet(std::size_t index) const { return m_facets[index]; }

    CORE_NODISCARD CORE_CONSTEXPR bool contains(const Vertex_t& vertex) const
    {
        for (const Vertex_t& v: m_vertices)
            if (vertex == v)
                return true;
        return false;
    }

    CORE_NODISCARD CORE_CONSTEXPR bool contains(const Halfedge_t& halfedge) const
    {
        for (const Halfedge_t& he: m_halfedges)
            if (halfedge == he)
                return true;
        return false;
    }

    CORE_NODISCARD CORE_CONSTEXPR bool contains(const Facet_t& facet) const
    {
        for (const Facet_t& f: m_facets)
            if (facet == f)
                return true;
        return false;
    }

    CORE_NODISCARD CORE_CONSTEXPR bool contains(const LinAl::Vec3<TFloatType>& vector) const
    {
        return m_meshPoints.contains(vector);
    }

    friend SphereMeshBuilder<TFloatType>;

  private:
    friend Vertex_t;
    friend Halfedge_t;
    friend Facet_t;

    LinAl::Vec3<TFloatType> getVector(Vertex_t vertex) const { return m_meshPoints.getPoint(vertex.getIndex().getValue()); }

    Core::TVector<Vertex_t> m_vertices;
    Core::TVector<Halfedge_t> m_halfedges;
    Core::TVector<Facet_t> m_facets;
    MeshPoints<TFloatType> m_meshPoints;
};

} // namespace Geometry

#endif // GLFWTESTAPP_HALFEDGEMESH_H
