#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include "Core/Math/Eps.hpp"
#include "Geometry/HalfedgeMesh/Vertex.hpp"
#include "Geometry/HalfedgeMeshBuilder/SphereMeshBuilder.hpp"
#include "Geometry/Sphere.hpp"
#include "LinAl/LinearAlgebra.hpp"
#include "MeshTestHelper.h"
#include "SphereTestHelper.h"
#include "gtest/gtest.h"

using namespace MeshTestHelper;
using namespace Geometry;
using namespace LinAl;

class Test_MeshBuilder_Sphere
    : public ::testing::Test
{
  protected:
    Sphere<double_t> m_sphere;
    std::unique_ptr<HalfedgeMesh<double_t>> m_sphereMesh;

    Test_MeshBuilder_Sphere() : m_sphere({ZERO_VEC3D, 3.0})
    {
        SphereMeshBuilder<double_t>().setSphere(m_sphere).build();
    }
};

TEST_F(Test_MeshBuilder_Sphere, VertexIndices)
{
    for (const auto& vertex: m_sphereMesh->getVertices())
        EXPECT_TRUE(isValidIndex(vertex.getIndex()));
}

TEST_F(Test_MeshBuilder_Sphere, MeshPoints)
{
    for (const auto& vertexPoint: m_sphereMesh->getVertexPoints())
        EXPECT_TRUE(isPointOnSphere(m_sphere, vertexPoint));
}

#pragma clang diagnostic pop