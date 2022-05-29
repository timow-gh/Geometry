#ifndef GEOMETRY_MESHTRIANGLEADDER_HPP
#define GEOMETRY_MESHTRIANGLEADDER_HPP

#include <Geometry/Triangle.hpp>

namespace Geometry
{

template <typename TFloatType, typename TIndexType>
class MeshTriangleAdder {
    HalfedgeMesh<TFloatType, TIndexType>* m_halfedgeMesh;

  public:
    using Vertex_t = Vertex<TFloatType, TIndexType>;
    using Halfedge_t = Halfedge<TFloatType, TIndexType>;
    using Facet_t = Facet<TFloatType, TIndexType>;

    CORE_CONSTEXPR explicit MeshTriangleAdder(HalfedgeMesh<TFloatType, TIndexType>& halfedgeMesh) CORE_NOEXCEPT
        : m_halfedgeMesh(&halfedgeMesh)
    {
    }

    void operator()(const Triangle<TFloatType, 3>& triangle)
    {
        MeshPoints<TFloatType>& meshPoints = m_halfedgeMesh->getMeshPoints();
        Core::TVector<Vertex_t>& vertices = m_halfedgeMesh->getVertices();
        Core::TVector<Halfedge_t>& halfedges = m_halfedgeMesh->getHalfedges();
        Core::TVector<Facet_t>& facets = m_halfedgeMesh->getFacets();

        std::size_t halfedgeIndex = halfedges.size() == 0 ? 0 : halfedges.size();

        Core::TArray<std::size_t, 3> vertexIndices;
        createOrFindVertex(triangle, meshPoints, vertices, vertexIndices);
        createHalfedgeAndSetVertex(vertexIndices, vertices, halfedges, m_halfedgeMesh);
        facets.emplace_back(halfedgeIndex, *m_halfedgeMesh);
        fillHalfedgesOfFacet(facets, halfedgeIndex, halfedges);
        setOppositeHalfedges(halfedgeIndex, halfedges);
    }

  private:
    void createOrFindVertex(const Triangle<TFloatType, 3>& triangle,
                            MeshPoints<TFloatType>& meshPoints,
                            Core::TVector<Vertex_t>& vertices,
                            Core::TArray<std::size_t, 3>& vertexIndices) const
    {
        // Create or find the Vertex of the LinAl::Vec3
        const LinAl::VecArray<TFloatType, 3, 3> trianglePoints = triangle.getTrianglePoints();
        for (std::size_t i = 0; i < 3; ++i)
        {
            std::size_t vertexIndex;
            if (!meshPoints.contains(trianglePoints[i], vertexIndex))
            {
                vertexIndex = meshPoints.add(trianglePoints[i]);
                vertices.emplace_back(vertexIndex, m_halfedgeMesh);
            }
            vertexIndices[i] = vertexIndex;
        }
    }

    void createHalfedgeAndSetVertex(const Core::TArray<std::size_t, 3>& vertexIndices,
                                    Core::TVector<Vertex_t>& vertices,
                                    Core::TVector<Halfedge_t>& halfedges,
                                    HalfedgeMesh<TFloatType, TIndexType>* halfedgeMesh) const
    {
        // Create the Halfedges and set the Halfedges for Vertices
        for (const std::size_t vIndex: vertexIndices)
        {
            halfedges.emplace_back(vIndex, halfedgeMesh);
            if (vertices[vIndex].getHalfedgeIndex() == INVALID_INDEX)
                vertices[vIndex].setHalfedgeIndex(halfedges.size() - 1);
        }
    }

    void
    fillHalfedgesOfFacet(Core::TVector<Facet_t>& facets, std::size_t halfedgeIndex, Core::TVector<Halfedge_t>& halfedges) const
    {
        // Fill the facet, the next and the previous pointer for each Halfedge
        // of the Facet
        const std::size_t facetIdx = facets.size() - 1;
        for (std::size_t i = halfedgeIndex; i < halfedges.size(); ++i)
        {
            halfedges[i].setFacetIndex(facetIdx);
            auto nextHeIndex = i == halfedges.size() - 1 ? halfedgeIndex : i + 1;
            halfedges[i].setNextIndex(nextHeIndex);
            auto previousHeIndex = i == halfedgeIndex ? halfedges.size() - 1 : i - 1;
            halfedges[i].setPreviousIndex(previousHeIndex);
        }
    }

    void setOppositeHalfedges(std::size_t halfedgeIndex, Core::TVector<Halfedge_t>& halfedges) const
    {
        // Find and set the opposite Halfedges for each Halfedge of the Facet
        for (std::size_t i = halfedgeIndex; i < halfedges.size(); ++i)
        {
            Halfedge_t& halfedge = halfedges[i];

            Vertex_t& heVertex = halfedge.getVertex();
            Halfedge_t& he = heVertex.getHalfedge();

            Halfedge_t& oppHeCandidate = he.getPrevious();
            std::size_t oppHeCandidateIndex = heVertex.getHalfedge().getPreviousIndex();

            Vertex_t& nextVertex = halfedge.getNextVertex();
            Vertex_t& oppNextVertex = oppHeCandidate.getNextVertex();

            if (heVertex == oppNextVertex && nextVertex == oppHeCandidate.getVertex())
            {
                halfedge.setOppositeIndex(oppHeCandidateIndex);
                oppHeCandidate.setOppositeIndex(i);
            }
        }
    }
};

} // namespace Geometry

#endif // GEOMETRY_MESHTRIANGLEADDER_HPP
