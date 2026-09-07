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
        // Four vertices forming two triangles that share the edge (v1 -> v2), so the two faces are
        // topologically adjacent (their shared halfedges are twinned).
        m_vertexHandles.push_back(m_mesh.add_vertex(linal::double3{0.0, 0.0, 0.0}));
        m_vertexHandles.push_back(m_mesh.add_vertex(linal::double3{1.0, 0.0, 0.0}));
        m_vertexHandles.push_back(m_mesh.add_vertex(linal::double3{0.0, 1.0, 0.0}));
        m_vertexHandles.push_back(m_mesh.add_vertex(linal::double3{1.0, 1.0, 0.0}));

        m_faceHandles.push_back(m_mesh.add_triangle(m_vertexHandles[0], m_vertexHandles[1], m_vertexHandles[2]));
        m_faceHandles.push_back(m_mesh.add_triangle(m_vertexHandles[2], m_vertexHandles[1], m_vertexHandles[3]));
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

TEST_F(TriangleHMeshFaceItertorTest, FaceVertexCirculator) {
    std::vector<Mesh::VertexHandle> vertexHandles;
    for (auto circ = m_mesh.vertices(m_faceHandles[0]).circulator(); circ.is_valid(); ++circ)
    {
        vertexHandles.emplace_back(circ.get_vertexhandle());
    }

    // Target vertices around face 0, in next order (matches vertices_around_face's 2nd/3rd/1st, i.e.
    // the same target ordering as the halfedge circulator: 1, 2, 0).
    ASSERT_EQ(vertexHandles.size(), 3u);
    EXPECT_EQ(vertexHandles[0].get_value(), 1);
    EXPECT_EQ(vertexHandles[1].get_value(), 2);
    EXPECT_EQ(vertexHandles[2].get_value(), 0);
}

TEST_F(TriangleHMeshFaceItertorTest, FaceVertexRangeYieldsVertexReferences) {
    std::vector<const Mesh::Vertex*> vertices;
    for (const Mesh::Vertex& vertex : m_mesh.vertices(m_faceHandles[0]))
    {
        vertices.emplace_back(&vertex);
    }

    ASSERT_EQ(vertices.size(), 3u);
    // Dereferenced references must be the mesh's own vertex storage (target vertices 1, 2, 0).
    EXPECT_EQ(vertices[0], &m_mesh.get_vertex(m_vertexHandles[1]));
    EXPECT_EQ(vertices[1], &m_mesh.get_vertex(m_vertexHandles[2]));
    EXPECT_EQ(vertices[2], &m_mesh.get_vertex(m_vertexHandles[0]));
}

TEST_F(TriangleHMeshFaceItertorTest, FaceVertexCirculatorMutatesThroughReference) {
    // The non-const circulator yields Vertex&, so a write through it must be visible in the mesh.
    for (auto circ = m_mesh.vertices(m_faceHandles[0]).circulator(); circ.is_valid(); ++circ)
    {
        (*circ).position = linal::double3{7.0, 8.0, 9.0};
    }

    for (auto circ = m_mesh.vertices(m_faceHandles[0]).circulator(); circ.is_valid(); ++circ)
    {
        EXPECT_EQ(m_mesh.get_position(circ.get_vertexhandle()), (linal::double3{7.0, 8.0, 9.0}));
    }
}

TEST_F(TriangleHMeshFaceItertorTest, FaceFaceCirculatorSkipsBoundary) {
    // The two fixture faces share edge (v1 -> v2): neighbors across exactly one edge, each with two
    // boundary edges the circulator must skip.
    ASSERT_TRUE(m_mesh.is_valid());

    std::vector<FaceHandle> neighborsOfF0;
    for (auto circ = m_mesh.adjacent_faces(m_faceHandles[0]).circulator(); circ.is_valid(); ++circ)
    {
        neighborsOfF0.emplace_back(circ.get_facehandle());
    }
    ASSERT_EQ(neighborsOfF0.size(), 1u);
    EXPECT_EQ(neighborsOfF0[0], m_faceHandles[1]);

    // Range-for yields Face& to the neighbor's storage.
    std::vector<const Mesh::Face*> neighborRefs;
    for (const Mesh::Face& face : m_mesh.adjacent_faces(m_faceHandles[1]))
    {
        neighborRefs.emplace_back(&face);
    }
    ASSERT_EQ(neighborRefs.size(), 1u);
    EXPECT_EQ(neighborRefs[0], &m_mesh.get_face(m_faceHandles[0]));
}

TEST(TriangleHMeshFaceFaceCirculator, IsolatedTriangleHasNoNeighbors) {
    Mesh mesh;
    VertexHandle const v0 = mesh.add_vertex(linal::double3{0.0, 0.0, 0.0});
    VertexHandle const v1 = mesh.add_vertex(linal::double3{1.0, 0.0, 0.0});
    VertexHandle const v2 = mesh.add_vertex(linal::double3{0.0, 1.0, 0.0});
    FaceHandle const f0 = mesh.add_triangle(v0, v1, v2);
    ASSERT_TRUE(f0.is_valid());

    // All three edges are boundary -> the circulator is invalid immediately and yields nothing.
    EXPECT_FALSE(mesh.adjacent_faces(f0).circulator().is_valid());

    std::size_t count = 0;
    for (const Mesh::Face& face : mesh.adjacent_faces(f0))
    {
        (void)face;
        ++count;
    }
    EXPECT_EQ(count, 0u);
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
