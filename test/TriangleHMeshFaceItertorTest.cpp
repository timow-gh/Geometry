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

TEST_F(TriangleHMeshFaceItertorTest, FaceHalfedgeIter) {
    std::vector<const Mesh::Halfedge*> faceHalfedges;
    for (const Mesh::Halfedge& he : m_mesh.halfedges(m_faceHandles[0]))
    {
        faceHalfedges.emplace_back(&he);
    }

    // Range-for over the face must yield exactly the 3 face halfedges (not empty, not infinite).
    ASSERT_EQ(faceHalfedges.size(), 3u);

    std::vector<Mesh::VertexHandle> vertexHandles;
    for (const auto* he : faceHalfedges)
    {
        vertexHandles.emplace_back(he->vertex);
    }

    // Target vertices of the three halfedges around face 0, matching the ordering used in the
    // ConstHalfEdgeIterator test above.
    ASSERT_EQ(vertexHandles.size(), 3u);
    EXPECT_EQ(vertexHandles[0].get_value(), 1);
    EXPECT_EQ(vertexHandles[1].get_value(), 2);
    EXPECT_EQ(vertexHandles[2].get_value(), 0);
}

TEST_F(TriangleHMeshFaceItertorTest, FaceHalfedgeCirculator) {
    std::vector<Mesh::VertexHandle> vertexHandles;
    for (auto circ = m_mesh.halfedges(m_faceHandles[0]).circulator(); circ.is_valid(); ++circ)
    {
        vertexHandles.emplace_back(circ->vertex);
    }

    ASSERT_EQ(vertexHandles.size(), 3u);
    EXPECT_EQ(vertexHandles[0].get_value(), 1);
    EXPECT_EQ(vertexHandles[1].get_value(), 2);
    EXPECT_EQ(vertexHandles[2].get_value(), 0);
}

TEST_F(TriangleHMeshFaceItertorTest, EmptyRangeYieldsNothing) {
    // A default-constructed range holds an invalid start handle, so it must iterate nothing
    // (the circulator's is_valid() guards the invalid handle before any dereference).
    Mesh::FaceHalfedgeRange emptyRange{};

    std::size_t count = 0;
    for (const Mesh::Halfedge& he : emptyRange)
    {
        (void)he;
        ++count;
    }
    EXPECT_EQ(count, 0u);

    EXPECT_FALSE(emptyRange.circulator().is_valid());
}
