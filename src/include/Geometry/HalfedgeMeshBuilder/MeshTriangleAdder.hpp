#ifndef GEOMETRY_MESHTRIANGLEADDER_HPP
#define GEOMETRY_MESHTRIANGLEADDER_HPP

#include <Geometry/HalfedgeMesh/MeshIndexTraits.hpp>
#include <Geometry/Triangle.hpp>

namespace Geometry
{

template <typename TFloatType, typename TIndexType>
class MeshTriangleAdder {
    HalfedgeMesh<TFloatType, TIndexType>* m_halfedgeMesh;

  public:
    using VertexIndex_t = typename MeshIndexTraits<TIndexType>::VertexIndex_t;
    using HalfedgeIndex_t = typename MeshIndexTraits<TIndexType>::HalfedgeIndex_t;
    using FacetIndex_t = typename MeshIndexTraits<TIndexType>::FacetIndex_t;

    using Vertex_t = Vertex<TFloatType, TIndexType>;
    using Halfedge_t = Halfedge<TFloatType, TIndexType>;
    using Facet_t = Facet<TFloatType, TIndexType>;

    using MeshPoints_t = MeshPoints<TFloatType, TIndexType>;

    CORE_CONSTEXPR explicit MeshTriangleAdder(HalfedgeMesh<TFloatType, TIndexType>& halfedgeMesh) CORE_NOEXCEPT
        : m_halfedgeMesh(&halfedgeMesh)
    {
    }

    void operator()(const Triangle<TFloatType, 3>& triangle)
    {
        MeshPoints_t& meshPoints = m_halfedgeMesh->meshPoints;
        Core::TVector<Vertex_t>& vertices = m_halfedgeMesh->vertices;
        Core::TVector<Halfedge_t>& halfedges = m_halfedgeMesh->halfedges;
        Core::TVector<Facet_t>& facets = m_halfedgeMesh->facets;

        HalfedgeIndex_t halfedgeIndex{halfedges.size() == 0 ? 0 : halfedges.size()};

        Core::TArray<VertexIndex_t, 3> vertexIndices;
        createOrFindVertex(triangle, meshPoints, vertices, vertexIndices);
        createHalfedgeAndSetVertex(vertexIndices, vertices, halfedges, m_halfedgeMesh);
        facets.emplace_back(halfedgeIndex, *m_halfedgeMesh);
        fillHalfedgesOfFacet(facets, halfedgeIndex, halfedges);
        setOppositeHalfedges(halfedgeIndex, halfedges);
    }

  private:
    void createOrFindVertex(const Triangle<TFloatType, 3>& triangle,
                            MeshPoints_t& meshPoints,
                            Core::TVector<Vertex_t>& vertices,
                            Core::TArray<VertexIndex_t, 3>& vertexIndices) const
    {
        // Create or find the Vertex of the LinAl::Vec3
        const LinAl::VecArray<TFloatType, 3, 3> trianglePoints = triangle.getTrianglePoints();
        for (std::size_t i = 0; i < 3; ++i)
        {
            VertexIndex_t vertexIndex;
            if (!meshPoints.contains(trianglePoints[i], vertexIndex))
            {
                vertexIndex = meshPoints.add(trianglePoints[i]);
                vertices.emplace_back(vertexIndex, m_halfedgeMesh);
            }
            vertexIndices[i] = vertexIndex;
        }
    }

    void createHalfedgeAndSetVertex(const Core::TArray<VertexIndex_t, 3>& vertexIndices,
                                    Core::TVector<Vertex_t>& vertices,
                                    Core::TVector<Halfedge_t>& halfedges,
                                    HalfedgeMesh<TFloatType, TIndexType>* halfedgeMesh) const
    {
        // Create the Halfedges and set the Halfedges for Vertices
        for (const VertexIndex_t vIndex: vertexIndices)
        {
            halfedges.emplace_back(vIndex, halfedgeMesh);
            if (!vertices[vIndex.getValue()].getHalfedgeIndex().isValid())
                vertices[vIndex.getValue()].setHalfedgeIndex(HalfedgeIndex_t{halfedges.size() - 1});
        }
    }

    void fillHalfedgesOfFacet(Core::TVector<Facet_t>& facets,
                              HalfedgeIndex_t halfedgeIndex,
                              Core::TVector<Halfedge_t>& halfedges) const
    {
        // Fill the facet, the next and the previous pointer for each Halfedge
        // of the Facet
        const FacetIndex_t facetIdx = FacetIndex_t(facets.size() - 1);
        for (TIndexType i = halfedgeIndex.getValue(); i < halfedges.size(); ++i)
        {
            halfedges[i].setFacetIndex(facetIdx);
            TIndexType nextHeIndex = i == halfedges.size() - 1 ? halfedgeIndex.getValue() : i + 1;
            halfedges[i].setNextIndex(HalfedgeIndex_t{nextHeIndex});
            TIndexType previousHeIndex = i == halfedgeIndex.getValue() ? halfedges.size() - 1 : i - 1;
            halfedges[i].setPreviousIndex(HalfedgeIndex_t{previousHeIndex});
        }
    }

    void setOppositeHalfedges(HalfedgeIndex_t halfedgeIndex, Core::TVector<Halfedge_t>& halfedges) const
    {
        // Find and set the opposite Halfedges for each Halfedge of the Facet
        for (std::size_t i = halfedgeIndex.getValue(); i < halfedges.size(); ++i)
        {
            Halfedge_t& halfedge = halfedges[i];

            Vertex_t& heVertex = halfedge.getVertex();
            Halfedge_t& he = heVertex.getHalfedge();

            Halfedge_t& oppHeCandidate = he.getPrevious();
            HalfedgeIndex_t oppHeCandidateIndex = heVertex.getHalfedge().getPreviousIndex();

            Vertex_t& nextVertex = halfedge.getNextVertex();
            Vertex_t& oppNextVertex = oppHeCandidate.getNextVertex();

            if (heVertex == oppNextVertex && nextVertex == oppHeCandidate.getVertex())
            {
                halfedge.setOppositeIndex(oppHeCandidateIndex);
                oppHeCandidate.setOppositeIndex(HalfedgeIndex_t{i});
            }
        }
    }
};

} // namespace Geometry

#endif // GEOMETRY_MESHTRIANGLEADDER_HPP
