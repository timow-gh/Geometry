#include <Geometry/Mesh/AddTriangle.hpp>
#include <Geometry/Mesh/TriangleHalfedgeMesh.hpp>

#include <linal/vec.hpp>

#include <gtest/gtest.h>
#include <algorithm>
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

        m_faceHandles.push_back(add_triangle(m_mesh,m_vertexHandles[0], m_vertexHandles[1], m_vertexHandles[2]));
        m_faceHandles.push_back(add_triangle(m_mesh,m_vertexHandles[2], m_vertexHandles[1], m_vertexHandles[3]));
    }

    Mesh m_mesh;

    std::vector<VertexHandle> m_vertexHandles;
    std::vector<FaceHandle> m_faceHandles;
};

TEST_F(TriangleHMeshFaceItertorTest, FaceHalfedgeIter) {
    std::vector<const Mesh::Halfedge*> faceHalfedges;
    for (const Mesh::Halfedge& halfedge : m_mesh.halfedges(m_faceHandles[0]))
    {
        faceHalfedges.emplace_back(&halfedge);
    }

    // Range-for over the face must yield exactly the 3 face halfedges (not empty, not infinite).
    ASSERT_EQ(faceHalfedges.size(), 3U);

    std::vector<Mesh::VertexHandle> vertexHandles;
    vertexHandles.reserve(faceHalfedges.size());
    for (const auto* halfedge : faceHalfedges)
    {
        vertexHandles.emplace_back(halfedge->targetVertex);
    }

    // Target vertices of the three halfedges around face 0, matching the ordering used in the
    // ConstHalfEdgeIterator test above.
    ASSERT_EQ(vertexHandles.size(), 3U);
    EXPECT_EQ(vertexHandles[0].get_value(), 1);
    EXPECT_EQ(vertexHandles[1].get_value(), 2);
    EXPECT_EQ(vertexHandles[2].get_value(), 0);
}

TEST_F(TriangleHMeshFaceItertorTest, FaceHalfedgeCirculator) {
    std::vector<Mesh::VertexHandle> vertexHandles;
    for (auto circ = m_mesh.halfedges(m_faceHandles[0]).circulator(); circ.is_valid(); ++circ)
    {
        vertexHandles.emplace_back(circ->targetVertex);
    }

    ASSERT_EQ(vertexHandles.size(), 3U);
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
    ASSERT_EQ(vertexHandles.size(), 3U);
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

    ASSERT_EQ(vertices.size(), 3U);
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
    ASSERT_TRUE(m_mesh.has_valid_connectivity());

    std::vector<FaceHandle> neighborsOfF0;
    for (auto circ = m_mesh.adjacent_faces(m_faceHandles[0]).circulator(); circ.is_valid(); ++circ)
    {
        neighborsOfF0.emplace_back(circ.get_facehandle());
    }
    ASSERT_EQ(neighborsOfF0.size(), 1U);
    EXPECT_EQ(neighborsOfF0[0], m_faceHandles[1]);

    // Range-for yields Face& to the neighbor's storage.
    std::vector<const Mesh::Face*> neighborRefs;
    for (const Mesh::Face& face : m_mesh.adjacent_faces(m_faceHandles[1]))
    {
        neighborRefs.emplace_back(&face);
    }
    ASSERT_EQ(neighborRefs.size(), 1U);
    EXPECT_EQ(neighborRefs[0], &m_mesh.get_face(m_faceHandles[0]));
}

TEST(TriangleHMeshFaceFaceCirculator, IsolatedTriangleHasNoNeighbors) {
    Mesh mesh;
    VertexHandle const vertex0 = mesh.add_vertex(linal::double3{0.0, 0.0, 0.0});
    VertexHandle const vertex1 = mesh.add_vertex(linal::double3{1.0, 0.0, 0.0});
    VertexHandle const vertex2 = mesh.add_vertex(linal::double3{0.0, 1.0, 0.0});
    FaceHandle const face0 = add_triangle(mesh, vertex0, vertex1, vertex2);
    ASSERT_TRUE(face0.is_valid());

    // All three edges are boundary -> the circulator is invalid immediately and yields nothing.
    EXPECT_FALSE(mesh.adjacent_faces(face0).circulator().is_valid());

    std::size_t count = 0;
    for (const Mesh::Face& face : mesh.adjacent_faces(face0))
    {
        (void)face;
        ++count;
    }
    EXPECT_EQ(count, 0U);
}

TEST_F(TriangleHMeshFaceItertorTest, VertexOutHalfedgeCirculatorInteriorVertex) {
    // v1 lies on the boundary of the two-triangle fixture. The fan orbit (twin.next) is uniform and
    // closes: it visits both interior outgoing halfedges (v1->v2 for Face0, v1->v3 for Face1) plus the
    // single boundary outgoing halfedge, i.e. three halfedges in total.
    std::vector<HalfedgeHandle> outgoing;
    for (auto circ = m_mesh.outgoing_halfedges(m_vertexHandles[1]).circulator(); circ.is_valid(); ++circ)
    {
        outgoing.emplace_back(circ.get_halfedgehandle());
        ASSERT_LE(outgoing.size(), 8U); // guard against a non-terminating walk
    }

    ASSERT_EQ(outgoing.size(), 3U);
    // Every yielded halfedge is outgoing from v1 (source == v1).
    for (HalfedgeHandle const halfedge : outgoing)
    {
        EXPECT_EQ(m_mesh.source_vertex(halfedge), m_vertexHandles[1]);
    }
    // The interior (face-bearing) outgoing halfedges target v2 and v3.
    std::vector<Mesh::handle_value_type> interiorTargets;
    for (HalfedgeHandle const halfedge : outgoing)
    {
        if (!m_mesh.is_boundary(halfedge))
        {
            interiorTargets.emplace_back(m_mesh.target_vertex(halfedge).get_value());
        }
    }
    std::sort(interiorTargets.begin(), interiorTargets.end());
    EXPECT_EQ(interiorTargets, (std::vector<Mesh::handle_value_type>{2U, 3U}));
}

TEST_F(TriangleHMeshFaceItertorTest, VertexOutHalfedgeCirculatorBoundaryVertexWrapsThroughBoundary) {
    // v0 belongs only to Face0 and lies on the boundary. Boundary halfedges are first-class and part of
    // the fan, so the uniform circulator wraps through the boundary halfedge and closes: it yields v0's
    // single interior outgoing halfedge (v0->v1) and its one boundary outgoing halfedge, then returns to
    // the start rather than stopping.
    std::vector<HalfedgeHandle> outgoing;
    for (auto circ = m_mesh.outgoing_halfedges(m_vertexHandles[0]).circulator(); circ.is_valid(); ++circ)
    {
        outgoing.emplace_back(circ.get_halfedgehandle());
        ASSERT_LE(outgoing.size(), 8U); // guard against a non-terminating walk
    }

    ASSERT_EQ(outgoing.size(), 2U);
    for (HalfedgeHandle const halfedge : outgoing)
    {
        EXPECT_EQ(m_mesh.source_vertex(halfedge), m_vertexHandles[0]);
    }
    std::size_t interiorCount = 0;
    std::size_t boundaryCount = 0;
    for (HalfedgeHandle const halfedge : outgoing)
    {
        if (m_mesh.is_boundary(halfedge))
        {
            ++boundaryCount;
        }
        else
        {
            ++interiorCount;
            EXPECT_EQ(m_mesh.target_vertex(halfedge).get_value(), 1U); // Face0: v0->v1
        }
    }
    EXPECT_EQ(interiorCount, 1U);
    EXPECT_EQ(boundaryCount, 1U);
}

TEST_F(TriangleHMeshFaceItertorTest, VertexOutHalfedgeMatchesEnumerationForInteriorVertex) {
    // halfedges_around_vertex() returns the interior (face-bearing) outgoing halfedges. Every interior
    // halfedge the circulator yields must appear in that enumeration.
    std::vector<HalfedgeHandle> const all = m_mesh.halfedges_around_vertex(m_vertexHandles[1]);

    for (auto circ = m_mesh.outgoing_halfedges(m_vertexHandles[1]).circulator(); circ.is_valid(); ++circ)
    {
        HalfedgeHandle const halfedge = circ.get_halfedgehandle();
        if (!m_mesh.is_boundary(halfedge))
        {
            EXPECT_NE(std::find(all.begin(), all.end(), halfedge), all.end());
        }
    }
}

TEST_F(TriangleHMeshFaceItertorTest, VertexInHalfedgeCirculatorInteriorVertex) {
    // v1 lies on the boundary; its uniform fan visits every incoming halfedge, one per incident edge.
    // v1 is incident to edges v1-v0, v1-v2, v1-v3, so there are three incoming halfedges, each with
    // target v1.
    std::vector<HalfedgeHandle> incoming;
    for (auto circ = m_mesh.incoming_halfedges(m_vertexHandles[1]).circulator(); circ.is_valid(); ++circ)
    {
        incoming.emplace_back(circ.get_halfedgehandle());
    }

    ASSERT_EQ(incoming.size(), 3U);
    for (HalfedgeHandle const halfedge : incoming)
    {
        EXPECT_EQ(m_mesh.target_vertex(halfedge), m_vertexHandles[1]);
    }
}

TEST_F(TriangleHMeshFaceItertorTest, VertexVertexCirculatorInteriorVertex) {
    // The full 1-ring of v1 is v0, v2 and v3 (v1 borders all three via its incident edges).
    std::vector<Mesh::handle_value_type> neighbors;
    for (auto circ = m_mesh.vertices(m_vertexHandles[1]).circulator(); circ.is_valid(); ++circ)
    {
        neighbors.emplace_back(circ.get_vertexhandle().get_value());
    }

    ASSERT_EQ(neighbors.size(), 3U);
    std::sort(neighbors.begin(), neighbors.end());
    EXPECT_EQ(neighbors, (std::vector<Mesh::handle_value_type>{0U, 2U, 3U}));
}

TEST_F(TriangleHMeshFaceItertorTest, VertexVertexRangeYieldsVertexReferences) {
    std::vector<const Mesh::Vertex*> vertices;
    for (const Mesh::Vertex& vertex : m_mesh.vertices(m_vertexHandles[1]))
    {
        vertices.emplace_back(&vertex);
    }

    ASSERT_EQ(vertices.size(), 3U);
    // Each dereferenced reference must alias one of v1's neighbour vertices (v0, v2 or v3).
    for (const Mesh::Vertex* vertex : vertices)
    {
        EXPECT_TRUE(vertex == &m_mesh.get_vertex(m_vertexHandles[0]) ||
                    vertex == &m_mesh.get_vertex(m_vertexHandles[2]) ||
                    vertex == &m_mesh.get_vertex(m_vertexHandles[3]));
    }
}

TEST_F(TriangleHMeshFaceItertorTest, VertexFaceCirculatorInteriorVertex) {
    // Faces incident to v1 are both fixture faces.
    std::vector<FaceHandle> faces;
    for (auto circ = m_mesh.faces(m_vertexHandles[1]).circulator(); circ.is_valid(); ++circ)
    {
        faces.emplace_back(circ.get_facehandle());
    }

    ASSERT_EQ(faces.size(), 2U);
    EXPECT_NE(std::find(faces.begin(), faces.end(), m_faceHandles[0]), faces.end());
    EXPECT_NE(std::find(faces.begin(), faces.end(), m_faceHandles[1]), faces.end());
}

TEST_F(TriangleHMeshFaceItertorTest, VertexFaceCirculatorBoundaryVertex) {
    // v0 is only in Face0.
    std::vector<FaceHandle> faces;
    for (auto circ = m_mesh.faces(m_vertexHandles[0]).circulator(); circ.is_valid(); ++circ)
    {
        faces.emplace_back(circ.get_facehandle());
    }

    ASSERT_EQ(faces.size(), 1U);
    EXPECT_EQ(faces[0], m_faceHandles[0]);
}

TEST_F(TriangleHMeshFaceItertorTest, VertexVertexCirculatorMutatesThroughReference) {
    // The non-const vertex circulator yields Vertex&, so writes through it are visible in the mesh.
    for (auto circ = m_mesh.vertices(m_vertexHandles[1]).circulator(); circ.is_valid(); ++circ)
    {
        (*circ).position = linal::double3{7.0, 8.0, 9.0};
    }

    for (auto circ = m_mesh.vertices(m_vertexHandles[1]).circulator(); circ.is_valid(); ++circ)
    {
        EXPECT_EQ(m_mesh.get_position(circ.get_vertexhandle()), (linal::double3{7.0, 8.0, 9.0}));
    }
}

TEST(TriangleHMeshVertexCirculator, IsolatedVertexYieldsNothing) {
    // A vertex with no incident face has an invalid Vertex.halfedge, so every vertex circulator is
    // invalid immediately and iterates nothing.
    Mesh mesh;
    VertexHandle const lone = mesh.add_vertex(linal::double3{0.0, 0.0, 0.0});

    EXPECT_FALSE(mesh.outgoing_halfedges(lone).circulator().is_valid());
    EXPECT_FALSE(mesh.incoming_halfedges(lone).circulator().is_valid());
    EXPECT_FALSE(mesh.vertices(lone).circulator().is_valid());
    EXPECT_FALSE(mesh.faces(lone).circulator().is_valid());

    std::size_t count = 0;
    for (const Mesh::Face& face : mesh.faces(lone))
    {
        (void)face;
        ++count;
    }
    EXPECT_EQ(count, 0U);
}

TEST_F(TriangleHMeshFaceItertorTest, EmptyRangeYieldsNothing) {
    // A default-constructed range holds an invalid start handle, so it must iterate nothing
    // (the circulator's is_valid() guards the invalid handle before any dereference).
    Mesh::ConstFaceHalfedgeRange emptyRange{};

    std::size_t count = 0;
    for (const Mesh::Halfedge& halfedge : emptyRange)
    {
        (void)halfedge;
        ++count;
    }
    EXPECT_EQ(count, 0U);

    EXPECT_FALSE(emptyRange.circulator().is_valid());
}

TEST_F(TriangleHMeshFaceItertorTest, WholeMeshRangesVisitEveryElement) {
    // Fixture: two triangles sharing edge v1->v2 => 4 vertices, 6 halfedges, 5 edges, 2 faces.
    EXPECT_EQ(m_mesh.vertices().size(), m_mesh.vertex_count());
    EXPECT_EQ(m_mesh.halfedges().size(), m_mesh.halfedge_count());
    EXPECT_EQ(m_mesh.edges().size(), m_mesh.edge_count());
    EXPECT_EQ(m_mesh.faces().size(), m_mesh.face_count());

    std::size_t vertexCount = 0;
    for (VertexHandle const vertex : m_mesh.vertices())
    {
        (void)vertex;
        ++vertexCount;
    }
    EXPECT_EQ(vertexCount, m_mesh.vertex_count());

    std::size_t halfedgeCount = 0;
    for (HalfedgeHandle const halfedge : m_mesh.halfedges())
    {
        (void)halfedge;
        ++halfedgeCount;
    }
    EXPECT_EQ(halfedgeCount, m_mesh.halfedge_count());

    std::size_t edgeCount = 0;
    for (Mesh::EdgeHandle const edge : m_mesh.edges())
    {
        (void)edge;
        ++edgeCount;
    }
    EXPECT_EQ(edgeCount, m_mesh.edge_count());

    std::size_t faceCount = 0;
    for (FaceHandle const face : m_mesh.faces())
    {
        (void)face;
        ++faceCount;
    }
    EXPECT_EQ(faceCount, m_mesh.face_count());
}

TEST_F(TriangleHMeshFaceItertorTest, WholeMeshRangesYieldHandlesInIndexOrder) {
    Mesh::handle_value_type expected = 0;
    for (VertexHandle const vertex : m_mesh.vertices())
    {
        EXPECT_EQ(vertex.get_value(), expected);
        ++expected;
    }

    expected = 0;
    for (FaceHandle const face : m_mesh.faces())
    {
        EXPECT_EQ(face.get_value(), expected);
        ++expected;
    }
}

TEST_F(TriangleHMeshFaceItertorTest, WholeMeshHandlesAreUsable) {
    // Every yielded handle must be a valid handle into the mesh's storage.
    for (VertexHandle const vertex : m_mesh.vertices())
    {
        ASSERT_TRUE(m_mesh.contains(vertex));
        (void)m_mesh.get_vertex(vertex);
    }
    for (HalfedgeHandle const halfedge : m_mesh.halfedges())
    {
        ASSERT_TRUE(m_mesh.contains(halfedge));
        (void)m_mesh.get_halfedge(halfedge);
    }
    for (Mesh::EdgeHandle const edge : m_mesh.edges())
    {
        ASSERT_TRUE(m_mesh.contains(edge));
        (void)m_mesh.get_edge(edge);
    }
    for (FaceHandle const face : m_mesh.faces())
    {
        ASSERT_TRUE(m_mesh.contains(face));
        (void)m_mesh.get_face(face);
    }
}

TEST_F(TriangleHMeshFaceItertorTest, WholeMeshFacesComposeWithLocalCirculator) {
    // The whole-mesh face range and the per-face vertex circulator compose: each face yields three
    // vertices.
    std::size_t faceCount = 0;
    for (FaceHandle const face : m_mesh.faces())
    {
        ++faceCount;
        std::size_t verticesInFace = 0;
        for (auto circ = m_mesh.vertices(face).circulator(); circ.is_valid(); ++circ)
        {
            ++verticesInFace;
        }
        EXPECT_EQ(verticesInFace, 3U);
    }
    EXPECT_EQ(faceCount, m_mesh.face_count());
}

TEST(TriangleHMeshWholeMeshIterator, EmptyMeshYieldsNothing) {
    Mesh mesh;

    EXPECT_TRUE(mesh.vertices().empty());
    EXPECT_TRUE(mesh.halfedges().empty());
    EXPECT_TRUE(mesh.edges().empty());
    EXPECT_TRUE(mesh.faces().empty());

    std::size_t count = 0;
    for (VertexHandle const vertex : mesh.vertices())
    {
        (void)vertex;
        ++count;
    }
    for (FaceHandle const face : mesh.faces())
    {
        (void)face;
        ++count;
    }
    EXPECT_EQ(count, 0U);
}
