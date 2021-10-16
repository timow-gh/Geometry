#ifndef GLFWTESTAPP_MESHBUILDER_H
#define GLFWTESTAPP_MESHBUILDER_H

#include "CuboidMeshBuilder.hpp"
#include "SphereMeshBuilder.hpp"

namespace Geometry
{
class HalfedgeMeshBuilder
{
  public:
    template <typename T>
    [[nodiscard]] std::unique_ptr<HalfedgeMesh<T>>
    buildMesh(const Cuboid<T>& cube)
    {
        Core::TArray<Triangle<T, 3>, 12> triangles = calcCuboidTriangles(cube);
        auto heMesh = std::make_unique<HalfedgeMesh<T>>(xg::newGuid());
        for (const auto& triangle: triangles)
            addTriangle<T>(heMesh.get(), triangle);
        return std::move(heMesh);
    }

    template <typename T>
    [[nodiscard]] std::unique_ptr<HalfedgeMesh<T>>
    buildMesh(const Sphere<T>& sphere)
    {
        Core::TVector<Triangle<T, 3>> triangles =
            calcSphereTriangles<T>(sphere);
        auto heMesh = std::make_unique<HalfedgeMesh<T>>(xg::newGuid());
        for (const auto& triangle: triangles)
            addTriangle<T>(heMesh.get(), triangle);
        return std::move(heMesh);
    }

  private:
    template <typename T>
    void addTriangle(HalfedgeMesh<T>* mesh, const Triangle<T, 3>& triangle)
    {
        // Create or find the Vertex of the LinAl::Vec3
        const LinAl::VecArray<T, 3, 3> trianglePoints =
            triangle.getTrianglePoints();
        Core::TArray<std::size_t, 3> vertexIndices;
        for (std::size_t i = 0; i < 3; ++i)
        {
            std::size_t vertexIndex;
            if (!mesh->meshPoints.contains(trianglePoints[i], vertexIndex))
            {
                vertexIndex = mesh->meshPoints.add(trianglePoints[i]);
                mesh->m_vertices.emplace_back(vertexIndex, mesh);
            }
            vertexIndices[i] = vertexIndex;
        }

        std::size_t halfedgeIndex =
            mesh->m_halfedges.size() == 0 ? 0 : mesh->m_halfedges.size();

        // Create the Halfedges and set the Halfedges for Vertices
        for (const std::size_t vIndex: vertexIndices)
        {
            mesh->m_halfedges.emplace_back(vIndex, mesh);
            if (mesh->m_vertices[vIndex].getHalfedgeIndex() == INVALID_INDEX)
                mesh->m_vertices[vIndex].setHalfedgeIndex(
                    mesh->m_halfedges.size() - 1);
        }

        mesh->m_facets.emplace_back(halfedgeIndex, mesh);

        // Fill the facet, the next and the previous pointer for each Halfedge
        // of the Facet
        const std::size_t facetIdx = mesh->m_facets.size() - 1;
        for (std::size_t i = halfedgeIndex; i < mesh->m_halfedges.size(); ++i)
        {
            mesh->m_halfedges[i].setFacet(facetIdx);
            auto nextHeIndex =
                i == mesh->m_halfedges.size() - 1 ? halfedgeIndex : i + 1;
            mesh->m_halfedges[i].setNextIndex(nextHeIndex);
            auto previousHeIndex =
                i == halfedgeIndex ? mesh->m_halfedges.size() - 1 : i - 1;
            mesh->m_halfedges[i].setPreviousIndex(previousHeIndex);
        }

        // Find and set the opposite Halfedges for each Halfedge of the Facet
        for (std::size_t i = halfedgeIndex; i < mesh->m_halfedges.size(); ++i)
        {
            Halfedge<T>& halfedge = mesh->m_halfedges[i];

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
};

} // namespace Geometry

#endif // GLFWTESTAPP_MESHBUILDER_H
