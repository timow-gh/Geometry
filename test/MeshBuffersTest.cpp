#include <Geometry/Mesh/MakeTriangleMesh.hpp>
#include <Geometry/Mesh/MeshBuffers.hpp>
#include <Geometry/Mesh/MeshDelete.hpp>

#include <gtest/gtest.h>
#include <set>

using namespace Geometry;

namespace {
template <typename T>
class MeshBuffersTest : public testing::Test {};
using Scalars = testing::Types<float, double>;
TYPED_TEST_SUITE(MeshBuffersTest, Scalars);

TYPED_TEST(MeshBuffersTest, EmptyAndIsolatedVertices) {
    TriangleHalfedgeMesh<TypeParam, 3, std::uint8_t> mesh;
    EXPECT_TRUE(make_vertex_buffer(mesh));
    EXPECT_TRUE(make_vertex_buffer(mesh).values.empty());
    EXPECT_TRUE(make_triangle_index_buffer(mesh));
    EXPECT_TRUE(make_triangle_index_buffer(mesh).values.empty());
    EXPECT_TRUE(make_edge_index_buffer(mesh));
    EXPECT_TRUE(make_edge_index_buffer(mesh).values.empty());
    const auto vertex = mesh.add_vertex({1, 2, 3});
    ASSERT_TRUE(vertex.is_valid());
    const auto result = make_vertex_buffer(mesh);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.error, MeshBufferStatus::Ok);
    EXPECT_EQ(result.values, (std::vector<float>{1, 2, 3}));
    EXPECT_TRUE(make_triangle_index_buffer(mesh).values.empty());
    EXPECT_TRUE(make_edge_index_buffer(mesh).values.empty());
}

TYPED_TEST(MeshBuffersTest, OpenPatchPreservesNumberingWindingAndUniqueEdges) {
    TriangleHalfedgeMesh<TypeParam, 3, std::uint64_t> mesh;
    const auto vertex0 = mesh.add_vertex({0, 0, 0});
    const auto vertex1 = mesh.add_vertex({2, 0, 0});
    const auto vertex2 = mesh.add_vertex({2, 3, 0});
    const auto vertex3 = mesh.add_vertex({0, 3, 0});
    const auto isolated = mesh.add_vertex({7, 8, 9});
    ASSERT_TRUE(isolated.is_valid());
    ASSERT_TRUE(add_triangle(mesh, vertex0, vertex1, vertex2).is_valid());
    ASSERT_TRUE(add_triangle(mesh, vertex0, vertex2, vertex3).is_valid());
    const auto positions = make_vertex_buffer(mesh);
    const auto triangles = make_triangle_index_buffer(mesh);
    const auto edges = make_edge_index_buffer(mesh);
    ASSERT_TRUE(positions);
    ASSERT_TRUE(triangles);
    ASSERT_TRUE(edges);
    EXPECT_EQ(positions.values, (std::vector<float>{0, 0, 0, 2, 0, 0, 2, 3, 0, 0, 3, 0, 7, 8, 9}));
    ASSERT_EQ(triangles.values.size(), 6U);
    // A circulator may cyclically rotate a triangle, but must preserve its winding.
    for (std::size_t face = 0; face < 2; ++face) {
        const std::array<std::uint32_t, 3> expected =
            face == 0 ? std::array<std::uint32_t, 3>{0, 1, 2} : std::array<std::uint32_t, 3>{0, 2, 3};
        bool matches = false;
        for (std::size_t rotation = 0; rotation < 3; ++rotation) {
            matches = matches || (triangles.values[3 * face] == expected[rotation] &&
                                  triangles.values[3 * face + 1] == expected[(rotation + 1) % 3] &&
                                  triangles.values[3 * face + 2] == expected[(rotation + 2) % 3]);
        }
        EXPECT_TRUE(matches);
    }
    ASSERT_EQ(edges.values.size(), 10U);
    std::set<std::pair<std::uint32_t, std::uint32_t>> unique;
    for (std::size_t i = 0; i < edges.values.size(); i += 2) {
        EXPECT_TRUE(
            unique
                .emplace(std::min(edges.values[i], edges.values[i + 1]), std::max(edges.values[i], edges.values[i + 1]))
                .second);
    }
    EXPECT_EQ(unique, (std::set<std::pair<std::uint32_t, std::uint32_t>>{{0, 1}, {1, 2}, {0, 2}, {2, 3}, {0, 3}}));
    EXPECT_EQ(mesh.vertex_count(), 5U);
    EXPECT_EQ(mesh.face_count(), 2U);
    EXPECT_TRUE(mesh.has_valid_connectivity());
}

TEST(MeshBufferErrorsTest, InvalidPositionsDiscardAllValues) {
    for (const double value: {std::numeric_limits<double>::infinity(),
                              -std::numeric_limits<double>::infinity(),
                              std::numeric_limits<double>::quiet_NaN(),
                              std::numeric_limits<double>::max(),
                              -std::numeric_limits<double>::max()}) {
        TriangleHalfedgeMesh3d mesh;
        const auto first = mesh.add_vertex({1, 2, 3});
        const auto second = mesh.add_vertex({4, value, 6});
        ASSERT_TRUE(first.is_valid() && second.is_valid());
        const auto result = make_vertex_buffer(mesh);
        EXPECT_FALSE(result);
        EXPECT_FALSE(result.has_value());
        EXPECT_TRUE(result.values.empty());
        EXPECT_EQ(result.error,
                  std::isfinite(value) ? MeshBufferStatus::PositionOutOfRange : MeshBufferStatus::NonFinitePosition);
    }
    TriangleHalfedgeMesh3d mesh;
    const auto limit = static_cast<double>(std::numeric_limits<float>::max());
    const auto vertex = mesh.add_vertex({limit, -limit, 1.0 / 3});
    ASSERT_TRUE(vertex.is_valid());
    const auto result = make_vertex_buffer(mesh);
    ASSERT_TRUE(result);
    EXPECT_EQ(result.values,
              (std::vector<float>{std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), 1.0F / 3}));
}

TEST(MeshBufferErrorsTest, CapacityBoundariesWithoutAllocation) {
    EXPECT_TRUE(detail::mesh_buffer_count_fits(3, 3, 10));
    EXPECT_FALSE(detail::mesh_buffer_count_fits(4, 3, 10));
    const auto limit = std::numeric_limits<std::size_t>::max();
    EXPECT_TRUE(detail::mesh_buffer_count_fits(limit / 3, 3, limit));
    EXPECT_FALSE(detail::mesh_buffer_count_fits(limit / 3 + 1, 3, limit));
    EXPECT_TRUE(detail::mesh_buffer_index_fits<std::uint32_t>(std::numeric_limits<std::uint32_t>::max()));
    EXPECT_FALSE(
        detail::mesh_buffer_index_fits<std::uint32_t>(std::uint64_t{std::numeric_limits<std::uint32_t>::max()} + 1));
}

TEST(MeshBufferPrimitivesTest, EveryFactoryProducesConsistentBuffers) {
    const auto check = [](const auto& result) {
        ASSERT_TRUE(result);
        const auto& mesh = result.mesh;
        const auto positions = make_vertex_buffer(mesh);
        const auto triangles = make_triangle_index_buffer(mesh);
        const auto edges = make_edge_index_buffer(mesh);
        ASSERT_TRUE(positions && triangles && edges);
        EXPECT_EQ(positions.values.size(), 3 * mesh.vertex_count());
        EXPECT_EQ(triangles.values.size(), 3 * mesh.face_count());
        EXPECT_EQ(edges.values.size(), 2 * mesh.edge_count());
        for (const auto index: triangles.values) {
            EXPECT_LT(index, mesh.vertex_count());
        }
        std::set<std::pair<std::uint32_t, std::uint32_t>> unique;
        for (std::size_t i = 0; i < edges.values.size(); i += 2) {
            EXPECT_LT(edges.values[i], mesh.vertex_count());
            EXPECT_LT(edges.values[i + 1], mesh.vertex_count());
            EXPECT_TRUE(unique
                            .emplace(std::min(edges.values[i], edges.values[i + 1]),
                                     std::max(edges.values[i], edges.values[i + 1]))
                            .second);
        }
    };
    check(make_triangle_mesh(Cone<double>{{0, 0, 0}, {0, 0, 3}, 1}));
    check(make_triangle_mesh(Cylinder<double>{Segment3d{{0, 0, 0}, {0, 0, 3}}, 1}));
    check(make_triangle_mesh(Cuboid<double>{{1, 2, 3}}));
    check(make_triangle_mesh(AABB3d{{0, 0, 0}, {1, 2, 3}}));
}

TEST(MeshRenderBuffersTest, ParallelNormalsRemappedIndicesAndUniqueEdges) {
    const auto check = [](const auto& result) {
        ASSERT_TRUE(result);
        const auto buffers = make_render_buffers(result.mesh);
        ASSERT_TRUE(buffers);
        // Positions and normals are parallel (geoqik requires one normal per vertex).
        EXPECT_EQ(buffers.normals.size(), buffers.positions.size());
        EXPECT_EQ(buffers.triangles.size(), 3 * result.mesh.face_count());
        // The overlay carries only crease edges, so it never exceeds the full edge set and is even.
        std::size_t creaseEdges = 0;
        for (const auto edge: result.mesh.edges()) {
            if (result.mesh.is_crease(edge)) {
                ++creaseEdges;
            }
        }
        EXPECT_EQ(buffers.segments.size(), 2 * creaseEdges);
        // Splitting only ever adds render vertices relative to the mesh's shared vertices.
        EXPECT_GE(buffers.vertex_count(), result.mesh.vertex_count());
        for (const auto index: buffers.triangles) {
            EXPECT_LT(index, buffers.vertex_count());
        }
        std::set<std::pair<std::uint32_t, std::uint32_t>> unique;
        for (std::size_t i = 0; i < buffers.segments.size(); i += 2) {
            EXPECT_LT(buffers.segments[i], buffers.vertex_count());
            EXPECT_LT(buffers.segments[i + 1], buffers.vertex_count());
            EXPECT_TRUE(unique
                            .emplace(std::min(buffers.segments[i], buffers.segments[i + 1]),
                                     std::max(buffers.segments[i], buffers.segments[i + 1]))
                            .second);
        }
    };
    check(make_triangle_mesh(Cone<double>{{0, 0, 0}, {0, 0, 3}, 1}));
    check(make_triangle_mesh(Cylinder<double>{Segment3d{{0, 0, 0}, {0, 0, 3}}, 1}));
    check(make_triangle_mesh(Cuboid<double>{{1, 2, 3}}));
    check(make_triangle_mesh(AABB3d{{0, 0, 0}, {1, 2, 3}}));
}

TEST(MeshRenderBuffersTest, DegenerateFaceIsReportedAsDegenerate) {
    TriangleHalfedgeMesh3d mesh;
    const auto first = mesh.add_vertex({0, 0, 0});
    const auto second = mesh.add_vertex({1, 0, 0});
    const auto third = mesh.add_vertex({2, 0, 0});
    ASSERT_TRUE(add_triangle(mesh, first, second, third).is_valid());

    EXPECT_EQ(make_render_buffers(mesh).error, MeshBufferStatus::DegenerateGeometry);
}

// Tombstoned vertices shift the live vertices' buffer slots below their handle values, so index
// buffers built before garbage_collection() must translate handles, in release builds too.
TEST(MeshBufferGarbageTest, IndexBuffersAddressCompactedVertexBuffer) {
    TriangleHalfedgeMesh3d mesh;
    const auto corner = mesh.add_vertex({0, 0, 0});
    const auto right = mesh.add_vertex({1, 0, 0});
    const auto top = mesh.add_vertex({1, 1, 0});
    const auto left = mesh.add_vertex({0, 1, 0});
    const auto farRight = mesh.add_vertex({2, 0, 0});
    ASSERT_TRUE(add_triangle(mesh, corner, right, top).is_valid());
    ASSERT_TRUE(add_triangle(mesh, corner, top, left).is_valid());
    ASSERT_TRUE(add_triangle(mesh, right, farRight, top).is_valid());
    // Removes both faces at the corner; the left vertex is left isolated and dropped with it.
    ASSERT_EQ(delete_vertex(mesh, corner), DeleteStatus::Ok);
    ASSERT_TRUE(mesh.has_garbage());

    const auto positions = make_vertex_buffer(mesh);
    const auto triangles = make_triangle_index_buffer(mesh);
    const auto edges = make_edge_index_buffer(mesh);
    ASSERT_TRUE(positions);
    ASSERT_TRUE(triangles);
    ASSERT_TRUE(edges);
    ASSERT_EQ(positions.values.size(), 3 * mesh.vertex_count());
    ASSERT_EQ(triangles.values.size(), 3u);
    ASSERT_EQ(edges.values.size(), 6u);

    const auto expect_position = [&](BufferIndex index, TriangleHalfedgeMesh3d::VertexHandle vertex) {
        ASSERT_LT(index, mesh.vertex_count());
        const auto& expected = mesh.get_vertex(vertex).position;
        for (linal::vec3<double>::size_type coordinate = 0; coordinate < 3; ++coordinate)
            EXPECT_EQ(positions.values[3 * index + coordinate], static_cast<float>(expected[coordinate]));
    };
    std::size_t slot = 0;
    for (const auto face: mesh.faces())
        for (auto vertex = mesh.vertices(face).circulator(); vertex.is_valid(); ++vertex)
            expect_position(triangles.values[slot++], vertex.get_vertexhandle());
    slot = 0;
    for (const auto edge: mesh.edges()) {
        const auto halfedge = mesh.get_edge(edge).halfedge;
        expect_position(edges.values[slot++], mesh.source_vertex(halfedge));
        expect_position(edges.values[slot++], mesh.target_vertex(halfedge));
    }
}
} // namespace
