

#include <Geometry/HalfedgeMesh/HalfedgeMesh.hpp>
#include <gtest/gtest.h>

using namespace Core;
using namespace LinAl;
using namespace Geometry;

TEST(HalfedgeMesh, IsDefaultConstructible)
{
    EXPECT_TRUE(std::is_default_constructible_v<HalfedgeMesh<double_t>>);
}

TEST(HalfedgeMesh, Not_CopyConstructible)
{
    EXPECT_FALSE(std::is_copy_constructible_v<HalfedgeMesh<double_t>>);
}

TEST(HalfedgeMesh, Not_CopyAssignable)
{
    EXPECT_FALSE(std::is_copy_assignable_v<HalfedgeMesh<double_t>>);
}

TEST(HalfedgeMesh, IsMoveConstructible)
{
    EXPECT_TRUE(std::is_move_constructible_v<HalfedgeMesh<double_t>>);
}

TEST(HalfedgeMesh, IsMoveAssignable)
{
    EXPECT_TRUE(std::is_move_assignable_v<HalfedgeMesh<double_t>>);
}

TEST(HalfedgeMesh, vertexPoints)
{
    HalfedgeMesh<float_t> heMesh{};
    Vec3Vector<float_t>& vertexPoints = heMesh.getVertexPoints();

    const HalfedgeMesh<float_t>& constMesh = heMesh;
    const Vec3Vector<float_t>& constVertexPoints = constMesh.getVertexPoints();
}

TEST(HalfedgeMesh, vertices)
{
    HalfedgeMesh<float_t> heMesh{};
    TVector<Vertex<float_t>>& vertices = heMesh.getVertices();

    const HalfedgeMesh<float_t>& constMesh = heMesh;
    const TVector<Vertex<float_t>>& constVertices = constMesh.getVertices();
}

TEST(HalfedgeMesh, halfedges)
{
    HalfedgeMesh<float_t> heMesh{};
    TVector<Halfedge<float_t>>& halfedges = heMesh.getHalfedges();

    const HalfedgeMesh<float_t>& constMesh = heMesh;
    const TVector<Halfedge<float_t>>& constHes = constMesh.getHalfedges();
}

TEST(HalfedgeMesh, facets)
{
    HalfedgeMesh<float_t> heMesh{};
    TVector<Facet<float_t>>& facets = heMesh.getFacets();

    const HalfedgeMesh<float_t>& constMesh = heMesh;
    const TVector<Facet<float_t>>& constFacet = constMesh.getFacets();
}

TEST(HalfedgeMesh, meshPoints)
{
    HalfedgeMesh<float_t> heMesh{};
    MeshPoints<float_t>& meshPoints = heMesh.getMeshPoints();

    const HalfedgeMesh<float_t>& constMesh = heMesh;
    const MeshPoints<float_t>& cMeshPoints = heMesh.getMeshPoints();
}
