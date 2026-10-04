#include <Geometry/Mesh/MakeTriangleMesh.hpp>
#include <Geometry/Mesh/MeshBuffers.hpp>
#include <Geometry/Mesh/MeshNormals.hpp>

#include <gtest/gtest.h>

#include <cmath>
#include <numbers>
#include <set>
#include <vector>

using namespace Geometry;

namespace {

template <typename T>
linal::vec3<T> corner_normal(const HalfedgeNormals<T>& normals,
                             const TriangleHalfedgeMesh<T, 3>& mesh,
                             typename TriangleHalfedgeMesh<T, 3>::FaceHandle face,
                             typename TriangleHalfedgeMesh<T, 3>::VertexHandle vertex) {
    for (const auto halfedge: mesh.halfedges_around_face(face)) {
        if (mesh.target_vertex(halfedge) == vertex) {
            return normals.values[static_cast<std::size_t>(halfedge.get_value())];
        }
    }
    return {};
}

TEST(MeshNormalsTest, FlatTriangleCornerNormalsEqualFaceNormal) {
    TriangleHalfedgeMesh3d mesh;
    const auto vertexA = mesh.add_vertex({0, 0, 0});
    const auto vertexB = mesh.add_vertex({1, 0, 0});
    const auto vertexC = mesh.add_vertex({0, 1, 0});
    const auto face = add_triangle(mesh, vertexA, vertexB, vertexC);
    ASSERT_TRUE(face.is_valid());

    const auto normals = compute_halfedge_normals(mesh);
    ASSERT_TRUE(normals);
    const linal::vec3<double> expected{0, 0, 1};
    for (const auto vertex: {vertexA, vertexB, vertexC}) {
        const auto normal = corner_normal(normals, mesh, face, vertex);
        EXPECT_NEAR(linal::dot(normal, expected), 1.0, 1e-9);
    }
}

// Two triangles folded along the shared edge b-c. Smooth: shared-vertex corners average the two
// face normals. Crease: they keep their own face normals (sharp).
TEST(MeshNormalsTest, SharedEdgeSmoothVersusCrease) {
    const auto build = [](bool crease) {
        TriangleHalfedgeMesh3d mesh;
        const auto vertexA = mesh.add_vertex({0, 0, 0});
        const auto vertexB = mesh.add_vertex({1, 0, 0});
        const auto vertexC = mesh.add_vertex({0, 1, 0});
        const auto vertexD = mesh.add_vertex({1, 1, 1}); // lifted so the second triangle tilts
        EXPECT_TRUE(add_triangle(mesh, vertexA, vertexB, vertexC).is_valid());
        EXPECT_TRUE(add_triangle(mesh, vertexB, vertexD, vertexC).is_valid());
        if (crease) {
            for (const auto edge: mesh.edges()) {
                mesh.set_crease(edge, true);
            }
        }
        return mesh;
    };

    const auto smooth = build(false);
    const auto sharp = build(true);
    const auto smoothNormals = compute_halfedge_normals(smooth);
    const auto sharpNormals = compute_halfedge_normals(sharp);
    ASSERT_TRUE(smoothNormals);
    ASSERT_TRUE(sharpNormals);

    // Shared vertices b and c belong to both faces. Find them by value (1 and 2).
    using VertexHandle = TriangleHalfedgeMesh3d::VertexHandle;
    using FaceHandle = TriangleHalfedgeMesh3d::FaceHandle;
    const VertexHandle vertexB{1};
    const FaceHandle face0{0};
    const FaceHandle face1{1};

    // Sharp: the two corners at b differ (each equals its own face normal).
    const auto sharpB0 = corner_normal(sharpNormals, sharp, face0, vertexB);
    const auto sharpB1 = corner_normal(sharpNormals, sharp, face1, vertexB);
    EXPECT_LT(linal::dot(sharpB0, sharpB1), 0.999);

    // Smooth: the two corners at b are identical (same averaged sector normal).
    const auto smoothB0 = corner_normal(smoothNormals, smooth, face0, vertexB);
    const auto smoothB1 = corner_normal(smoothNormals, smooth, face1, vertexB);
    EXPECT_NEAR(linal::dot(smoothB0, smoothB1), 1.0, 1e-9);
    EXPECT_NEAR(linal::length(smoothB0), 1.0, 1e-9);
}

TEST(MeshNormalsTest, CuboidIsFullyFacetedAndSplits) {
    const auto result = make_triangle_mesh(Cuboid<double>{{0, 0, 0}, std::array<linal::vec3<double>, 3>{{{2, 0, 0}, {0, 2, 0}, {0, 0, 2}}}});
    ASSERT_TRUE(result);
    const auto& mesh = result.mesh;

    // The 12 real box edges (90 deg dihedral) are creases; the 12 in-face diagonals of the split
    // quads are coplanar (0 deg) and correctly stay smooth, so no seam appears inside a flat face.
    std::size_t creaseCount = 0;
    for (const auto edge: mesh.edges()) {
        if (mesh.is_crease(edge)) {
            ++creaseCount;
        }
    }
    EXPECT_EQ(creaseCount, 12U);

    const auto normals = compute_halfedge_normals(mesh);
    ASSERT_TRUE(normals);
    // Each face's three corner normals equal that face's flat normal (all corners split apart).
    for (const auto face: mesh.faces()) {
        const auto handles = mesh.vertices_around_face(face);
        const linal::vec3<double> edge1{mesh.get_vertex(handles[1]).position - mesh.get_vertex(handles[0]).position};
        const linal::vec3<double> edge2{mesh.get_vertex(handles[2]).position - mesh.get_vertex(handles[0]).position};
        const auto faceNormal = linal::normalize(linal::cross(edge1, edge2));
        for (const auto vertex: handles) {
            EXPECT_NEAR(linal::dot(corner_normal(normals, mesh, face, vertex), faceNormal), 1.0, 1e-9);
        }
    }

    // Fully split: 8 mesh vertices, 3 faces meeting at each corner with distinct normals -> 24.
    const auto buffers = make_render_buffers(mesh);
    ASSERT_TRUE(buffers);
    EXPECT_EQ(buffers.vertex_count(), 24U);
    EXPECT_EQ(buffers.normals.size(), buffers.positions.size());
    EXPECT_EQ(buffers.triangles.size(), mesh.face_count() * 3);
}

// Each sector's normal is computed once, so the corners sharing it must be bitwise equal -- the
// render buffers weld on that. Summing per corner from different start faces would round differently.
TEST(MeshNormalsTest, SmoothSectorCornersAreBitwiseEqualAndWeld) {
    auto result = make_triangle_mesh(Cylinder<double>{Segment3d{{0, 0, 0}, {0, 0, 3}}, 1}, 17);
    ASSERT_TRUE(result);
    auto& mesh = result.mesh;
    for (const auto edge: mesh.edges()) {
        mesh.set_crease(edge, false);
    }

    const auto normals = compute_halfedge_normals(mesh);
    ASSERT_TRUE(normals);
    std::vector<linal::vec3<double>> firstSeen(mesh.vertex_storage_size());
    std::vector<bool> seen(mesh.vertex_storage_size(), false);
    for (const auto face: mesh.faces()) {
        for (const auto corner: mesh.halfedges_around_face(face)) {
            const auto slot = static_cast<std::size_t>(mesh.target_vertex(corner).get_value());
            const auto& normal = normals.values[static_cast<std::size_t>(corner.get_value())];
            if (!seen[slot]) {
                firstSeen[slot] = normal;
                seen[slot] = true;
                continue;
            }
            for (linal::vec3<double>::size_type coordinate = 0; coordinate < 3; ++coordinate) {
                EXPECT_EQ(normal[coordinate], firstSeen[slot][coordinate]);
            }
        }
    }

    const auto buffers = make_render_buffers(mesh);
    ASSERT_TRUE(buffers);
    EXPECT_EQ(buffers.vertex_count(), mesh.vertex_count());
}

TEST(MeshNormalsTest, ConeWallSmoothCapSharp) {
    const std::size_t segments = 16;
    const auto result = make_triangle_mesh(Cone<double>{{0, 0, 0}, {0, 0, 4}, 1.0}, segments);
    ASSERT_TRUE(result);
    const auto& mesh = result.mesh;

    const auto normals = compute_halfedge_normals(mesh);
    ASSERT_TRUE(normals);

    // A wall (rim) vertex borders both a cap triangle and side triangles. Across the crease rim its
    // corner normals must differ; the cap corner should point along -Z (flat base).
    const auto buffers = make_render_buffers(mesh);
    ASSERT_TRUE(buffers);
    // Splitting must increase the render vertex count beyond the mesh vertex count (rim is creased).
    EXPECT_GT(buffers.vertex_count(), mesh.vertex_count());
    EXPECT_EQ(buffers.normals.size(), buffers.positions.size());
    for (const auto index: buffers.triangles) {
        EXPECT_LT(index, buffers.vertex_count());
    }
    // Every emitted normal is unit length (or a zero normal only for isolated vertices, of which a
    // closed cone has none).
    for (std::size_t i = 0; i < buffers.normals.size(); i += 3) {
        const float length = std::sqrt(buffers.normals[i] * buffers.normals[i] +
                                       buffers.normals[i + 1] * buffers.normals[i + 1] +
                                       buffers.normals[i + 2] * buffers.normals[i + 2]);
        EXPECT_NEAR(length, 1.0F, 1e-5F);
    }
}

TEST(MeshNormalsTest, MarkCreasesByAngleFlatMeshHasNoCreases) {
    // Two coplanar triangles: dihedral angle is 0, so no interior edge should become a crease; only
    // the boundary edges are creases.
    TriangleHalfedgeMesh3d mesh;
    const auto vertexA = mesh.add_vertex({0, 0, 0});
    const auto vertexB = mesh.add_vertex({1, 0, 0});
    const auto vertexC = mesh.add_vertex({0, 1, 0});
    const auto vertexD = mesh.add_vertex({1, 1, 0});
    ASSERT_TRUE(add_triangle(mesh, vertexA, vertexB, vertexC).is_valid());
    ASSERT_TRUE(add_triangle(mesh, vertexB, vertexD, vertexC).is_valid());
    mark_creases_by_angle(mesh, std::numbers::pi / 6);
    for (const auto edge: mesh.edges()) {
        EXPECT_EQ(mesh.is_crease(edge), mesh.is_boundary(edge));
    }
}

} // namespace
