#ifndef GEOMETRY_MESHTRIANGLEADDER_HPP
#define GEOMETRY_MESHTRIANGLEADDER_HPP

#include <Geometry/Triangle.hpp>

namespace Geometry
{

template <typename T>
class HalfedgeMesh;

template <typename T>
class MeshTriangleAdder {
    HalfedgeMesh<T>* m_halfedgeMesh;

  public:
    CORE_CONSTEXPR explicit MeshTriangleAdder(HalfedgeMesh<T>& halfedgeMesh) CORE_NOEXCEPT
        : m_halfedgeMesh(&halfedgeMesh)
    {
    }

    void operator()(const Triangle<T, 3>& triangle)
    {
        MeshPoints<T>& meshPoints = m_halfedgeMesh->getMeshPoints();
        Core::TVector<Vertex<T>>& vertices = m_halfedgeMesh->getVertices();
        Core::TVector<Halfedge<T>>& halfedges = m_halfedgeMesh->getHalfedges();
        Core::TVector<Facet<T>>& facets = m_halfedgeMesh->getFacets();

        std::size_t halfedgeIndex = halfedges.size() == 0 ? 0 : halfedges.size();

        Core::TArray<std::size_t, 3> vertexIndices;
        createOrFindVertex(triangle, meshPoints, vertices, vertexIndices);
        createHalfedgeAndSetVertex(vertexIndices, vertices, halfedges, m_halfedgeMesh);
        facets.emplace_back(halfedgeIndex, *m_halfedgeMesh);
        fillHalfedgesOfFacet(facets, halfedgeIndex, halfedges);
        setOppositeHalfedges(halfedgeIndex, halfedges);
    }

  private:
    void createOrFindVertex(const Triangle<T, 3>& triangle,
                            MeshPoints<T>& meshPoints,
                            Core::TVector<Vertex<T>>& vertices,
                            Core::TArray<std::size_t, 3>& vertexIndices) const
    {
        // Create or find the Vertex of the LinAl::Vec3
        const LinAl::VecArray<T, 3, 3> trianglePoints = triangle.getTrianglePoints();
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
                                    Core::TVector<Vertex<T>>& vertices,
                                    Core::TVector<Halfedge<T>>& halfedges,
                                    HalfedgeMesh<T>* halfedgeMesh) const
    {
        // Create the Halfedges and set the Halfedges for Vertices
        for (const std::size_t vIndex: vertexIndices)
        {
            halfedges.emplace_back(vIndex, halfedgeMesh);
            if (vertices[vIndex].getHalfedgeIndex() == INVALID_INDEX)
                vertices[vIndex].setHalfedgeIndex(halfedges.size() - 1);
        }
    }

    void fillHalfedgesOfFacet(Core::TVector<Facet<T>>& facets,
                              std::size_t halfedgeIndex,
                              Core::TVector<Halfedge<T>>& halfedges) const
    {
        // Fill the facet, the next and the previous pointer for each Halfedge
        // of the Facet
        const std::size_t facetIdx = facets.size() - 1;
        for (std::size_t i = halfedgeIndex; i < halfedges.size(); ++i)
        {
            halfedges[i].setFacet(facetIdx);
            auto nextHeIndex = i == halfedges.size() - 1 ? halfedgeIndex : i + 1;
            halfedges[i].setNextIndex(nextHeIndex);
            auto previousHeIndex = i == halfedgeIndex ? halfedges.size() - 1 : i - 1;
            halfedges[i].setPreviousIndex(previousHeIndex);
        }
    }

    void setOppositeHalfedges(std::size_t halfedgeIndex,
                              Core::TVector<Halfedge<T>>& halfedges) const
    {
        // Find and set the opposite Halfedges for each Halfedge of the Facet
        for (std::size_t i = halfedgeIndex; i < halfedges.size(); ++i)
        {
            Halfedge<T>& halfedge = halfedges[i];

            Vertex<T>& heVertex = halfedge.getVertex();
            Halfedge<T>& he = heVertex.getHalfedge();

            Halfedge<T>& oppHeCandidate = he.previous();
            std::size_t oppHeCandidateIndex = heVertex.halfedge().getPreviousIndex();

            auto nextVertex = halfedge.nextVertex();
            auto oppNextVertex = oppHeCandidate.nextVertex();

            if (!nextVertex || !oppNextVertex)
                continue;

            if (*heVertex == *oppNextVertex && *nextVertex == oppHeCandidate.vertex())
            {
                halfedge.setOppositeIndex(oppHeCandidateIndex);
                oppHeCandidate.setOppositeIndex(i);
            }
        }
    }
};

} // namespace Geometry

#endif // GEOMETRY_MESHTRIANGLEADDER_HPP
