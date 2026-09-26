#include <Geometry/Mesh/MakeTriangleMesh.hpp>
#include <Geometry/Mesh/MeshEuler.hpp>
#include <Geometry/Mesh/MeshTopology.hpp>

#include <gtest/gtest.h>

using namespace Geometry;

namespace {
template <typename T, typename TIndex>
void check_solid(const MeshCreationResult<T, TIndex>& result,
                 std::size_t vertices,
                 std::size_t faces,
                 const linal::vec3<T>& interior) {
    ASSERT_TRUE(result) << static_cast<int>(result.error);
    EXPECT_TRUE(result.has_value());
    const auto& mesh = result.mesh;
    EXPECT_EQ(mesh.vertex_count(), vertices);
    EXPECT_EQ(mesh.face_count(), faces);
    EXPECT_EQ(mesh.edge_count(), faces * 3 / 2);
    EXPECT_EQ(mesh.halfedge_count(), faces * 3);
    ASSERT_TRUE(mesh.has_valid_connectivity());
    EXPECT_TRUE(verify_manifold(mesh));
    EXPECT_TRUE(verify_closed(mesh));
    EXPECT_TRUE(boundary_loops(mesh).empty());
    EXPECT_EQ(num_connected_components(mesh), 1U);
    EXPECT_EQ(euler_characteristic(mesh), 2);
    T volume = 0;
    for (const auto face: mesh.faces()) {
        const auto handles = mesh.vertices_around_face(face);
        const linal::vec3<T> first{mesh.get_vertex(handles[0]).position - interior};
        const linal::vec3<T> second{mesh.get_vertex(handles[1]).position - interior};
        const linal::vec3<T> third{mesh.get_vertex(handles[2]).position - interior};
        const auto normal = linal::cross(linal::vec3<T>{second - first}, linal::vec3<T>{third - first});
        EXPECT_GT(linal::dot(normal, first), T{0});
        volume += linal::dot(first, linal::cross(second, third)) / T{6};
        std::size_t neighbors = 0;
        for (auto circ = mesh.adjacent_faces(face).circulator(); circ.is_valid() && neighbors < 4; ++circ) {
            ++neighbors;
        }
        EXPECT_EQ(neighbors, 3U);
    }
    EXPECT_GT(volume, T{0});
    for (const auto vertex: mesh.vertices()) {
        std::size_t count = 0;
        for (auto circ = mesh.outgoing_halfedges(vertex).circulator(); circ.is_valid() && count <= faces; ++circ) {
            ++count;
        }
        EXPECT_GE(count, 3U);
        EXPECT_LE(count, faces);
    }
}

template <typename T>
class PrimitiveMeshTest : public testing::Test {};
using Scalars = testing::Types<float, double>;
TYPED_TEST_SUITE(PrimitiveMeshTest, Scalars);

TYPED_TEST(PrimitiveMeshTest, Boxes) {
    using T = TypeParam;
    const linal::vec3<T> origin{2, 3, 4};
    for (const T sign: {T{1}, T{-1}}) {
        const std::array<linal::vec3<T>, 3> sides{{{2, 1, 0}, {0, 3, 1}, {0, 0, sign * T{4}}}};
        const Cuboid<T> shape{origin, sides};
        const auto result = make_triangle_mesh(shape);
        check_solid(result, 8, 12, linal::vec3<T>{origin + (sides[0] + sides[1] + sides[2]) / T{2}});
        ASSERT_TRUE(result);
        const auto corners = calc_cuboid_vertices(shape);
        for (const auto vertex: result.mesh.vertices()) {
            EXPECT_TRUE(linal::is_equal(result.mesh.get_vertex(vertex).position, corners[vertex.get_value()]));
        }
    }
    const auto result = make_triangle_mesh<T, std::uint8_t>(AABB<T, 3>{origin, linal::vec3<T>{4, 7, 10}});
    check_solid(result, 8, 12, linal::vec3<T>{3, 5, 7});
}

TYPED_TEST(PrimitiveMeshTest, RoundShapes) {
    using T = TypeParam;
    for (const linal::vec3<T> target: {linal::vec3<T>{2, 3, 9}, linal::vec3<T>{5, 7, 8}, linal::vec3<T>{2, 3, -1}}) {
        const linal::vec3<T> source{2, 3, 4};
        const Segment3<T> segment{source, target};
        for (const std::size_t count: {3U, 4U, 32U}) {
            const auto cone = make_triangle_mesh(Cone<T>{segment, T{2}}, count);
            const auto cylinder = make_triangle_mesh(Cylinder<T>{segment, T{2}}, count);
            check_solid(cone, count + 2, 2 * count, linal::vec3<T>{source + (target - source) / T{4}});
            check_solid(cylinder, 2 * count + 2, 4 * count, linal::vec3<T>{(source + target) / T{2}});
            ASSERT_TRUE(cone);
            ASSERT_TRUE(cylinder);
            const auto axis = segment.direction();
            for (const auto vertex: cylinder.mesh.vertices()) {
                const auto index = vertex.get_value();
                if (index >= 2 * count) {
                    continue;
                }
                const linal::vec3<T> radial{cylinder.mesh.get_vertex(vertex).position -
                                            (index < count ? source : target)};
                EXPECT_NEAR(linal::length(radial), T{2}, 1e-5);
                EXPECT_NEAR(linal::dot(radial, axis), T{0}, 1e-5);
                if (index < count) {
                    const auto coneVertex = typename TriangleHalfedgeMesh<T, 3>::VertexHandle{index};
                    EXPECT_TRUE(linal::is_equal(cone.mesh.get_vertex(coneVertex).position,
                                                cylinder.mesh.get_vertex(vertex).position));
                }
            }
        }
    }
    auto result = make_triangle_mesh(Cone<T>{{0, 0, 0}, {0, 0, 2}, T{1}});
    ASSERT_TRUE(result);
    auto mesh = std::move(result.mesh);
    EXPECT_EQ(mesh.face_count(), 64U);
    EXPECT_EQ(make_triangle_mesh(Cylinder<T>{Segment3<T>{{0, 0, 0}, {0, 0, 2}}, T{1}}).mesh.face_count(), 128U);
}

template <typename T, typename TIndex>
void check_error(const MeshCreationResult<T, TIndex>& result, MeshCreationStatus error) {
    EXPECT_FALSE(result);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error, error);
    EXPECT_TRUE(result.mesh.empty());
    EXPECT_EQ(result.mesh.edge_count(), 0U);
    EXPECT_EQ(result.mesh.halfedge_count(), 0U);
}

TEST(MakeTriangleMeshTest, Errors) {
    const Segment3d segment{{0, 0, 0}, {0, 0, 2}};
    const Cone<double> cone{segment, 1};
    check_error(make_triangle_mesh(cone, 2), MeshCreationStatus::InvalidSegmentCount);
    check_error(make_triangle_mesh(Cone<double>{segment, 0}), MeshCreationStatus::DegenerateGeometry);
    check_error(make_triangle_mesh(Cylinder<double>{Segment3d{{1, 2, 3}, {1, 2, 3}}, 1}),
                MeshCreationStatus::DegenerateGeometry);
    check_error(make_triangle_mesh(Cuboid<double>{{1, 2, 0}}), MeshCreationStatus::DegenerateGeometry);
    check_error(make_triangle_mesh(
                    Cuboid<double>{{0, 0, 0}, std::array<linal::vec3<double>, 3>{{{1, 0, 0}, {2, 0, 0}, {0, 0, 1}}}}),
                MeshCreationStatus::DegenerateGeometry);
    AABB3d bounds;
    check_error(make_triangle_mesh(bounds), MeshCreationStatus::InvalidBounds);
    bounds.set_min({0, 0, 0});
    bounds.set_max({0, 1, 1});
    check_error(make_triangle_mesh(bounds), MeshCreationStatus::DegenerateGeometry);
    bounds.set_max({std::numeric_limits<double>::quiet_NaN(), 1, 1});
    check_error(make_triangle_mesh(bounds), MeshCreationStatus::NonFiniteGeometry);
    check_error(make_triangle_mesh(Cone<double>{segment, std::numeric_limits<double>::infinity()}),
                MeshCreationStatus::NonFiniteGeometry);
    check_error(make_triangle_mesh<double, std::uint8_t>(cone, 43), MeshCreationStatus::IndexCapacityExceeded);
    EXPECT_TRUE((make_triangle_mesh<double, std::uint8_t>(cone, 42)));
    check_error(make_triangle_mesh<double, std::uint8_t>(Cylinder<double>{segment, 1}, 22),
                MeshCreationStatus::IndexCapacityExceeded);
    EXPECT_TRUE((make_triangle_mesh<double, std::uint8_t>(Cylinder<double>{segment, 1}, 21)));
    check_error(make_triangle_mesh(cone, std::numeric_limits<std::size_t>::max()),
                MeshCreationStatus::IndexCapacityExceeded);
}

TEST(MakeTriangleMeshTest, NumericLimits) {
    const double largest = std::numeric_limits<double>::max();
    check_error(make_triangle_mesh(Cone<double>{{-largest, 0, 0}, {largest, 0, 0}, 1}),
                MeshCreationStatus::NonFiniteGeometry);
    check_error(make_triangle_mesh(Cuboid<double>{{largest, 0, 0}, linal::vec3<double>{largest, 1, 1}}),
                MeshCreationStatus::NonFiniteGeometry);
    check_error(make_triangle_mesh(Cylinder<double>{Segment3d{{1e20, 1e20, 0}, {1e20, 1e20, 2}}, 1}),
                MeshCreationStatus::DegenerateGeometry);
    check_error(make_triangle_mesh(Cuboid<double>{{1e20, 1e20, 1e20}, linal::vec3<double>{1, 1, 1}}),
                MeshCreationStatus::DegenerateGeometry);
    EXPECT_TRUE(make_triangle_mesh(Cone<double>{{0, 0, 0}, {0, 0, 1e-100}, 1e-100}, 4));
}

TEST(MakeTriangleMeshTest, CheckedInsertionRejectsDuplicate) {
    TriangleHalfedgeMesh3d mesh;
    const auto first = mesh.add_vertex({0, 0, 0});
    const auto second = mesh.add_vertex({1, 0, 0});
    const auto third = mesh.add_vertex({0, 1, 0});
    ASSERT_EQ(detail::add_mesh_creation_triangle(mesh, first, second, third), MeshCreationStatus::Ok);
    EXPECT_EQ(detail::add_mesh_creation_triangle(mesh, first, second, third),
              MeshCreationStatus::TriangleInsertionFailed);
    EXPECT_EQ(mesh.face_count(), 1U);
    EXPECT_TRUE(mesh.has_valid_connectivity());
}
} // namespace
