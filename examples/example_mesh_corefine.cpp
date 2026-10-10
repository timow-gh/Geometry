#include "draw.hpp"
#include "geoqik_init.hpp"
#include "grid.hpp"
#include "origin.hpp"

#include <Geometry/Cylinder.hpp>
#include <Geometry/Mesh/MakeTriangleMesh.hpp>
#include <Geometry/Mesh/MeshCorefine.hpp>
#include <Geometry/Mesh/MeshFromTriangles.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <span>
#include <utility>
#include <vector>

namespace {

using Mesh = Geometry::TriangleHalfedgeMesh3d;
using VertexHandle = Mesh::VertexHandle;
using HalfedgeHandle = Mesh::HalfedgeHandle;
using EdgeHandle = Mesh::EdgeHandle;
using Vec3 = Mesh::vec_t;
using Triangle = std::array<std::uint32_t, 3>;

// Wider than the scenes, so that neighboring cells do not touch.
constexpr std::array<double, 4> columnX{-6.75, -2.25, 2.25, 6.75};
constexpr std::array<double, 5> rowY{9.0, 4.5, 0.0, -4.5, -9.0};

constexpr float curveLineWidth = 3.5f;
constexpr float curveVertexDiameter = 9.0f;

const example::color colorA = example::light_blue();
const example::color colorB = example::light_orange();
const example::color curveColor = example::red();
const example::color curveVertexColor = example::magenta();

Mesh make_box(const Vec3& min, const Vec3& max) {
    auto result = Geometry::make_triangle_mesh(Geometry::AABB3d{min, max});
    if (!result)
        example::fail_example("Create box", static_cast<int>(result.error));
    return std::move(result.mesh);
}

Mesh make_mesh(const std::vector<Vec3>& positions, const std::vector<Triangle>& triangles) {
    auto result = Geometry::make_mesh_from_triangles(std::span<const Vec3>{positions}, std::span<const Triangle>{triangles});
    if (!result)
        example::fail_example("Create mesh", static_cast<int>(result.error));
    return std::move(result.mesh);
}

Mesh translated(const Mesh& mesh, const Vec3& offset) {
    Mesh moved = mesh;
    for (const VertexHandle vertex : moved.vertices())
        moved.set_position(vertex, Vec3{moved.get_position(vertex) + offset});
    return moved;
}

// The coordinates of every vertex on the curve, once each, as draw_points expects them.
std::vector<double> curve_vertex_coordinates(const Mesh& mesh, std::span<const EdgeHandle> curve) {
    std::vector<VertexHandle> vertices;
    vertices.reserve(2 * curve.size());
    for (const EdgeHandle edge : curve) {
        const HalfedgeHandle halfedge = mesh.get_edge(edge).halfedge;
        vertices.push_back(mesh.source_vertex(halfedge));
        vertices.push_back(mesh.target_vertex(halfedge));
    }
    std::ranges::sort(vertices);
    const auto duplicates = std::ranges::unique(vertices);
    vertices.erase(duplicates.begin(), duplicates.end());

    std::vector<double> coordinates;
    coordinates.reserve(3 * vertices.size());
    for (const VertexHandle vertex : vertices) {
        const Vec3& position = mesh.get_position(vertex);
        coordinates.insert(coordinates.end(), {position[0], position[1], position[2]});
    }
    return coordinates;
}

// The refined triangles as a wireframe, and the curve on top: its edges as thick lines and its
// vertices as points, so that inserted vertices stand out from the operand's own.
void draw_refined(const Mesh& mesh, std::span<const EdgeHandle> curve, const Vec3& offset, const example::color& color) {
    const Mesh moved = translated(mesh, offset);
    example::draw_wireframe_mesh(moved, color);
    example::draw_mesh_edges(moved, curve, curveColor, curveLineWidth);
    example::draw_points(curve_vertex_coordinates(moved, curve), curveVertexColor, curveVertexDiameter);
}

// One row: the operands as given | A refined | B refined | both refined. The operands are built
// around center, which is moved into each cell; drawing comes first, so they can then be moved into
// corefine without a copy.
void show_scene(Mesh meshA, Mesh meshB, const Vec3& center, std::size_t row, const char* name) {
    const auto offset_of = [&center, row](std::size_t column) {
        return Vec3{Vec3{columnX[column], rowY[row], 0.0} - center};
    };
    example::draw_wireframe_mesh(translated(meshA, offset_of(0)), colorA);
    example::draw_wireframe_mesh(translated(meshB, offset_of(0)), colorB);

    const auto result = Geometry::corefine(std::move(meshA), std::move(meshB));
    if (!result) {
        std::fprintf(stderr, "%s: corefinement failed\n", name);
        example::fail_example("Corefine", static_cast<int>(result.error));
    }
    draw_refined(result.meshA, result.intersectionEdgesA, offset_of(1), colorA);
    draw_refined(result.meshB, result.intersectionEdgesB, offset_of(2), colorB);
    // Both curves lie on the same positions, so drawing A's suffices.
    draw_refined(result.meshA, result.intersectionEdgesA, offset_of(3), colorA);
    example::draw_wireframe_mesh(translated(result.meshB, offset_of(3)), colorB);
}

// The curve is one closed loop on each box. Dyadic offsets keep every intersection point exact and
// avoid coplanar faces.
void show_shifted_boxes(std::size_t row) {
    show_scene(make_box(Vec3{0.0, 0.0, 0.0}, Vec3{2.0, 2.0, 2.0}),
               make_box(Vec3{1.0, 0.5, 0.25}, Vec3{3.0, 2.5, 2.25}),
               Vec3{1.5, 1.25, 0.0},
               row,
               "shifted boxes");
}

// Two loops, where the cylinder enters the bottom face and leaves the top face. The axis avoids the box
// diagonals, and a length of 4, a power of two, keeps each side quad planar in floating point.
void show_cylinder_through_box(std::size_t row) {
    auto cylinder = Geometry::make_triangle_mesh(
        Geometry::Cylinder<double>{Geometry::Segment3d{Vec3{0.7, 1.2, -1.0}, Vec3{0.7, 1.2, 3.0}}, 0.45}, 16);
    if (!cylinder)
        example::fail_example("Create cylinder", static_cast<int>(cylinder.error));
    show_scene(make_box(Vec3{0.0, 0.0, 0.0}, Vec3{2.0, 2.0, 2.0}), std::move(cylinder.mesh), Vec3{1.0, 1.0, -1.0}, row,
               "cylinder through box");
}

// Each side face of the tetrahedron crosses the box's top face along a line through three points:
// where it enters the face, where it crosses the face's diagonal, and where it leaves. The middle one
// splits the sub-edge joining the other two instead of leaving a sliver beside it.
void show_tetrahedron_through_box_top(std::size_t row) {
    show_scene(make_box(Vec3{0.0, 0.0, 0.0}, Vec3{2.0, 2.0, 2.0}),
               make_mesh({Vec3{1.34, 1.24, 1.65}, Vec3{1.61, 1.35, 2.48}, Vec3{0.42, 1.39, 2.30}, Vec3{1.03, 0.30, 2.82}},
                         {Triangle{0, 2, 1}, Triangle{0, 3, 2}, Triangle{0, 1, 3}, Triangle{1, 2, 3}}),
               Vec3{1.0, 1.0, 0.0},
               row,
               "tetrahedron through box top");
}

// B's min-x face lies in A's max-x face, shifted, so the shared rectangle is refined in both faces and
// every edge inside it is part of the curve, not only its outline.
void show_coplanar_contact(std::size_t row) {
    show_scene(make_box(Vec3{0.0, 0.0, 0.0}, Vec3{1.0, 1.0, 1.0}),
               make_box(Vec3{1.0, 0.5, 0.25}, Vec3{2.0, 1.5, 1.25}),
               Vec3{1.0, 0.75, 0.0},
               row,
               "coplanar contact");
}

// An open mesh: the box pierces the single triangle's interior, so eight points inside one face are
// joined into an octagon, which needs edge flips because the points are inserted one at a time.
void show_triangle_pierced_by_box(std::size_t row) {
    show_scene(make_mesh({Vec3{-1.25, -1.25, 0.0}, Vec3{2.75, -1.25, 0.0}, Vec3{-1.25, 2.75, 0.0}}, {Triangle{0, 1, 2}}),
               make_box(Vec3{-0.5, -0.5, -1.0}, Vec3{0.5, 0.5, 1.0}),
               Vec3{0.75, 0.75, -1.0},
               row,
               "triangle pierced by box");
}

} // namespace

int main() {
    example::init_geoqik();
    example::draw_default_origin();
    example::draw_default_grid();

    show_shifted_boxes(0);
    show_cylinder_through_box(1);
    show_tetrahedron_through_box_top(2);
    show_coplanar_contact(3);
    show_triangle_pierced_by_box(4);

    example::check_geoqik(geoqik_draw(), "Open visualization");
    example::check_geoqik(geoqik_wait_for_exit_and_cleanup(), "Close visualization");
    return EXIT_SUCCESS;
}
