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
#include <LinAl/LinearAlgebra.hpp>

namespace Geometry
{

template <typename TFloatType = std::double_t, typename TIndexType = std::size_t>
struct HalfedgeMesh
{
    using VertexIndex_t = typename MeshIndexTraits<TIndexType>::VertexIndex_t;
    using HalfedgeIndex_t = typename MeshIndexTraits<TIndexType>::HalfedgeIndex_t;
    using FacetIndex_t = typename MeshIndexTraits<TIndexType>::FacetIndex_t;

    using Vertex_t = Vertex<TFloatType, TIndexType>;
    using Halfedge_t = Halfedge<TFloatType, TIndexType>;
    using Facet_t = Facet<TFloatType, TIndexType>;

    using MeshPoints_t = MeshPoints<TFloatType, TIndexType>;

    CORE_CONSTEXPR HalfedgeMesh() = default;
    CORE_CONSTEXPR explicit HalfedgeMesh(const HalfedgeMesh& rhs) = delete;
    CORE_CONSTEXPR HalfedgeMesh& operator=(const HalfedgeMesh& rhs) = delete;

    CORE_CONSTEXPR HalfedgeMesh(HalfedgeMesh&& rhs) CORE_NOEXCEPT = default;
    CORE_CONSTEXPR HalfedgeMesh& operator=(HalfedgeMesh&& rhs) CORE_NOEXCEPT = default;

    CORE_NODISCARD CORE_CONSTEXPR bool contains(const Vertex_t& vertex) const
    {
        for (const Vertex_t& v: vertices)
            if (vertex == v)
                return true;
        return false;
    }

    CORE_NODISCARD CORE_CONSTEXPR bool contains(const Halfedge_t& halfedge) const
    {
        for (const Halfedge_t& he: halfedges)
            if (halfedge == he)
                return true;
        return false;
    }

    CORE_NODISCARD CORE_CONSTEXPR bool contains(const Facet_t& facet) const
    {
        for (const Facet_t& f: facets)
            if (facet == f)
                return true;
        return false;
    }

    CORE_NODISCARD CORE_CONSTEXPR bool contains(const LinAl::Vec3<TFloatType>& vector) const
    {
        return meshPoints.contains(vector);
    }

    LinAl::Vec3<TFloatType> getVector(Vertex_t vertex) const { return meshPoints.getPoint(vertex.getIndex().getValue()); }
    // clang-format off
    CORE_NODISCARD CORE_CONSTEXPR Vertex_t getVertex(const VertexIndex_t vertexIndex) const { return vertices[vertexIndex.getValue()]; }
    CORE_NODISCARD CORE_CONSTEXPR Halfedge_t getHalfedge(const HalfedgeIndex_t halfedgeIndex) const { return halfedges[halfedgeIndex.getValue()]; }
    CORE_NODISCARD CORE_CONSTEXPR Facet_t getFacet(const FacetIndex_t facetIndex) const { return facets[facetIndex.getValue()]; }

    CORE_NODISCARD CORE_CONSTEXPR Vertex_t& getVertex(VertexIndex_t vertexIndex) { return vertices[vertexIndex.getValue()]; }
    CORE_NODISCARD CORE_CONSTEXPR Halfedge_t& getHalfedge(HalfedgeIndex_t halfedgeIndex) { return halfedges[halfedgeIndex.getValue()]; }
    CORE_NODISCARD CORE_CONSTEXPR Facet_t& getFacet(FacetIndex_t facetIndex) { return facets[facetIndex.getValue()]; }
    // clang-format on

    Core::TVector<Vertex_t> vertices;
    Core::TVector<Halfedge_t> halfedges;
    Core::TVector<Facet_t> facets;
    MeshPoints_t meshPoints;
};

} // namespace Geometry

#endif // GLFWTESTAPP_HALFEDGEMESH_H
