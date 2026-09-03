#include <Geometry/TriangleHalfedgeMesh.hpp>

#include <linal/vec.hpp>

#include <gtest/gtest.h>
#include <vector>

using namespace Geometry;

namespace {

using Mesh = TriangleHalfedgeMesh3d;
using HalfedgeHandle = Mesh::HalfedgeHandle;
using VertexHandle = Mesh::VertexHandle;
using FaceHandle = Mesh::FaceHandle;

} // namespace

class TriangleHMeshFaceItertorTest : public ::testing::Test {
  protected:
    void SetUp() override {
        m_vertexHandles.push_back(m_mesh.add_vertex(linal::double3{0.0, 0.0, 0.0}));
        m_vertexHandles.push_back(m_mesh.add_vertex(linal::double3{1.0, 0.0, 0.0}));
        m_vertexHandles.push_back(m_mesh.add_vertex(linal::double3{0.0, 1.0, 0.0}));

        m_vertexHandles.push_back(m_mesh.add_vertex(linal::double3{0.0, 1.0, 0.0}));
        m_vertexHandles.push_back(m_mesh.add_vertex(linal::double3{1.0, 0.0, 0.0}));
        m_vertexHandles.push_back(m_mesh.add_vertex(linal::double3{1.0, 1.0, 0.0}));

        m_faceHandles.push_back(m_mesh.add_triangle(m_vertexHandles[0], m_vertexHandles[1], m_vertexHandles[2]));
        m_faceHandles.push_back(m_mesh.add_triangle(m_vertexHandles[3], m_vertexHandles[4], m_vertexHandles[5]));
    }

    Mesh m_mesh;

    std::vector<VertexHandle> m_vertexHandles;
    std::vector<FaceHandle> m_faceHandles;
};

TEST_F(TriangleHMeshFaceItertorTest, ConstHalfEdgeIterator) {
    auto faceHandle = m_mesh.get_face(m_faceHandles[0]);
    Mesh::ConstHalfEdgeIterator iter = Mesh::ConstHalfEdgeIterator(faceHandle.halfedge, &m_mesh);

    std::array<HalfedgeHandle, 3> heHandles;

    heHandles[0] = iter.get_halfedgehandle();
    EXPECT_EQ(heHandles[0], iter.get_halfedgehandle());
    EXPECT_EQ(heHandles[0].get_value(), 0);
    VertexHandle vHandle = iter->vertex;
    EXPECT_EQ(vHandle.get_value(), 1);

    iter++;
    heHandles[1] = iter.get_halfedgehandle();
    EXPECT_EQ(heHandles[1].get_value(), 1);
    vHandle = (*iter).vertex;
    EXPECT_EQ(vHandle.get_value(), 2);

    ++iter;
    heHandles[2] = iter.get_halfedgehandle();
    EXPECT_EQ(heHandles[2].get_value(), 2);

    iter++;
    EXPECT_EQ(heHandles[0], iter.get_halfedgehandle());
}
