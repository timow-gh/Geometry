#include "draw.hpp"
#include "geoqik_init.hpp"
#include "grid.hpp"
#include "origin.hpp"

#include <Geometry/Cylinder.hpp>
#include <Geometry/Mesh/MakeTriangleMesh.hpp>
#include <Geometry/Mesh/MeshBoolean.hpp>
#include <Geometry/Mesh/MeshFromTriangles.hpp>
#include <Geometry/Mesh/MeshOrientation.hpp>
#include <Geometry/Mesh/MeshVerify.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <map>
#include <span>
#include <utility>
#include <vector>

// Rows of mesh Booleans, each read from left to right:
//   front row (box A, sphere B): inputs | A u B | A n B | A - B | B - A
//   middle row (constructive solid geometry): cube and sphere | cube n sphere | minus a cylinder along x |
//     minus a cylinder along z | the same, cut open to show the tunnels
//   back row (coplanar contact): boxes side by side | their union, one box | block and flush bar |
//     block - bar, a through-hole | staircase union of three boxes
// Only crease edges are outlined: the operands' feature edges, and the curves where the Booleans cut.
namespace {

using Mesh = Geometry::TriangleHalfedgeMesh3d;
using Vec3 = Mesh::vec_t;
using Triangle = std::array<std::uint32_t, 3>;
using Geometry::BooleanOperation;

constexpr std::array<double, 5> columnX{-8.0, -4.0, 0.0, 4.0, 8.0};
constexpr double frontRowY = -6.5;
constexpr double middleRowY = 0.0;
constexpr double backRowY = 6.5;

// Outline the cut curves too: the operands' creases alone would leave each curve smooth.
const Geometry::MeshBooleanOptions<double> booleanOptions{.intersectionCreaseAngle = Geometry::default_crease_angle<double>};

const example::color inputColorA = example::light_blue();
const example::color inputColorB = example::light_orange();

Mesh make_box(const Vec3& min, const Vec3& max) {
    auto result = Geometry::make_triangle_mesh(Geometry::AABB3d{min, max});
    if (!result)
        example::fail_example("Create box", static_cast<int>(result.error));
    return std::move(result.mesh);
}

Mesh make_cylinder(const Vec3& source, const Vec3& target, double radius, std::size_t segments) {
    auto result = Geometry::make_triangle_mesh(Geometry::Cylinder<double>{Geometry::Segment3d{source, target}, radius}, segments);
    if (!result)
        example::fail_example("Create cylinder", static_cast<int>(result.error));
    return std::move(result.mesh);
}

// A geodesic sphere: an icosahedron whose triangles are split into four, subdivisions times, with
// every vertex pushed onto the sphere. There is no sphere factory. No edge is a crease, so it shades
// smoothly and only cut curves are outlined on it.
//
// Not a latitude-longitude sphere: its quads are planar only up to the rounding of sin and cos, so
// where one crosses a box face, three intersection points are collinear up to 1e-18 without being
// collinear. Telling them apart needs predicates on the points' definitions, which the library does
// not have yet. Neighboring triangles of a geodesic sphere are clearly not coplanar.
Mesh make_sphere(const Vec3& center, double radius, std::size_t subdivisions) {
    const double golden = 0.5 * (1.0 + std::sqrt(5.0));
    std::vector<Vec3> directions{{-1, golden, 0}, {1, golden, 0}, {-1, -golden, 0}, {1, -golden, 0},
                                 {0, -1, golden}, {0, 1, golden}, {0, -1, -golden}, {0, 1, -golden},
                                 {golden, 0, -1}, {golden, 0, 1}, {-golden, 0, -1}, {-golden, 0, 1}};
    std::vector<Triangle> triangles{{0, 11, 5}, {0, 5, 1}, {0, 1, 7}, {0, 7, 10}, {0, 10, 11}, {1, 5, 9}, {5, 11, 4},
                                    {11, 10, 2}, {10, 7, 6}, {7, 1, 8}, {3, 9, 4}, {3, 4, 2}, {3, 2, 6}, {3, 6, 8},
                                    {3, 8, 9}, {4, 9, 5}, {2, 4, 11}, {6, 2, 10}, {8, 6, 7}, {9, 8, 1}};

    for (std::size_t level = 0; level < subdivisions; ++level) {
        // Each edge is split once, shared by both of its triangles.
        std::map<std::pair<std::uint32_t, std::uint32_t>, std::uint32_t> midpoints;
        const auto midpoint = [&](std::uint32_t first, std::uint32_t second) {
            const auto [entry, inserted] = midpoints.try_emplace(std::minmax(first, second), static_cast<std::uint32_t>(directions.size()));
            if (inserted)
                directions.push_back(Vec3{0.5 * (directions[first] + directions[second])});
            return entry->second;
        };
        std::vector<Triangle> refined;
        refined.reserve(4 * triangles.size());
        for (const auto& [first, second, third] : triangles) {
            const std::uint32_t firstSecond = midpoint(first, second);
            const std::uint32_t secondThird = midpoint(second, third);
            const std::uint32_t thirdFirst = midpoint(third, first);
            refined.insert(refined.end(), {Triangle{first, firstSecond, thirdFirst}, Triangle{second, secondThird, firstSecond},
                                           Triangle{third, thirdFirst, secondThird}, Triangle{firstSecond, secondThird, thirdFirst}});
        }
        triangles = std::move(refined);
    }

    std::vector<Vec3> positions;
    positions.reserve(directions.size());
    for (const Vec3& direction : directions)
        positions.push_back(Vec3{center + (radius / linal::length(direction)) * direction});

    auto result = Geometry::make_mesh_from_triangles(std::span<const Vec3>{positions}, std::span<const Triangle>{triangles});
    if (!result)
        example::fail_example("Create sphere", static_cast<int>(result.error));
    if (Geometry::mesh_orientation(result.mesh) != Geometry::MeshOrientation::Outward)
        example::fail_example("Orient sphere", 0);
    return std::move(result.mesh);
}

// A failed Boolean is reported here, naming the operation, instead of leaving an empty spot in the
// scene. The result checks repeat what mesh_boolean guarantees, as a safeguard for the demo.
Mesh boolean(const Mesh& meshA, const Mesh& meshB, BooleanOperation operation, const char* name) {
    auto result = Geometry::mesh_boolean(meshA, meshB, operation, booleanOptions);
    if (!result) {
        std::fprintf(stderr, "%s: mesh Boolean failed\n", name);
        example::fail_example("Mesh Boolean", static_cast<int>(result.error));
    }
    if (!Geometry::verify_manifold(result.mesh) || !Geometry::verify_closed(result.mesh) ||
        Geometry::mesh_orientation(result.mesh) != Geometry::MeshOrientation::Outward) {
        std::fprintf(stderr, "%s: result is not a closed, outward-facing manifold\n", name);
        example::fail_example("Validate result", 0);
    }
    return std::move(result.mesh);
}

// Every scene is built around the origin and only moved into its cell for drawing, so each Boolean
// runs once, on the coordinates chosen for it.
void draw_at(const Mesh& mesh, double x, double y, const example::color& color) {
    constexpr float edgeLineWidth = 2.0f;
    // Lifts the outlines off the faces they lie on, which they would otherwise depth-fight.
    constexpr std::int32_t edgeDepthLayer = 1;
    Mesh moved = mesh;
    const Vec3 offset{x, y, 0.0};
    for (const auto vertex : moved.vertices())
        moved.set_position(vertex, Vec3{moved.get_position(vertex) + offset});
    example::draw(moved, color, edgeLineWidth, edgeDepthLayer);
}

// The four Booleans of an axis-aligned box and a sphere over one of its corners. The sphere's center
// is off the corner, so no sphere vertex lands exactly on a box face or edge.
void draw_box_and_sphere_row() {
    const Mesh box = make_box(Vec3{-1.0, -1.0, 0.0}, Vec3{1.0, 1.0, 2.0});
    const Mesh sphere = make_sphere(Vec3{0.85, 0.8, 1.9}, 1.0, 3);

    draw_at(box, columnX[0], frontRowY, inputColorA);
    draw_at(sphere, columnX[0], frontRowY, inputColorB);
    draw_at(boolean(box, sphere, BooleanOperation::Union, "box u sphere"), columnX[1], frontRowY, example::light_cyan());
    draw_at(boolean(box, sphere, BooleanOperation::Intersection, "box n sphere"), columnX[2], frontRowY, example::light_green());
    draw_at(boolean(box, sphere, BooleanOperation::Difference, "box - sphere"), columnX[3], frontRowY, inputColorA);
    draw_at(boolean(sphere, box, BooleanOperation::Difference, "sphere - box"), columnX[4], frontRowY, inputColorB);
}

// A rounded cube drilled along two axes, each step a Boolean on the previous result, then cut open.
//
// Booleans of Booleans are where the floating-point predicates are weakest: a result's vertices on
// its intersection curve are rounded, so the next Boolean meets nearly degenerate configurations that
// only predicates on the points' definitions decide reliably. The cylinders are chosen to stay clear
// of them: segment counts that put no edge at 45 degrees, over the cube's face diagonals, and a length
// of 4, a power of two, so each side quad is planar in floating point too.
void draw_constructive_solid_geometry_row() {
    const Vec3 center{0.0, 0.0, 1.0};
    const Mesh cube = make_box(Vec3{-1.0, -1.0, 0.0}, Vec3{1.0, 1.0, 2.0});
    const Mesh sphere = make_sphere(center, 1.35, 3);
    const Mesh rounded = boolean(cube, sphere, BooleanOperation::Intersection, "cube n sphere");

    const Mesh alongX = make_cylinder(Vec3{center - Vec3{2.0, 0.0, 0.0}}, Vec3{center + Vec3{2.0, 0.0, 0.0}}, 0.55, 20);
    const Mesh alongZ = make_cylinder(Vec3{center - Vec3{0.0, 0.0, 2.0}}, Vec3{center + Vec3{0.0, 0.0, 2.0}}, 0.45, 18);
    const Mesh drilledX = boolean(rounded, alongX, BooleanOperation::Difference, "rounded cube - cylinder x");
    const Mesh drilledXZ = boolean(drilledX, alongZ, BooleanOperation::Difference, "drilled - cylinder z");

    // Keeping the back half opens the part towards the front; the cut plane avoids the cylinder axes.
    const Mesh backHalf = make_box(Vec3{-2.0, 0.1, -1.0}, Vec3{2.0, 2.0, 3.0});
    const Mesh cutOpen = boolean(drilledXZ, backHalf, BooleanOperation::Intersection, "drilled n back half");

    draw_at(cube, columnX[0], middleRowY, inputColorA);
    draw_at(sphere, columnX[0], middleRowY, inputColorB);
    draw_at(rounded, columnX[1], middleRowY, example::light_green());
    draw_at(drilledX, columnX[2], middleRowY, example::light_yellow());
    draw_at(drilledXZ, columnX[3], middleRowY, example::light_yellow());
    draw_at(cutOpen, columnX[4], middleRowY, example::light_yellow());
}

// Where solids share part of a plane, the shared region is kept once or dropped, never doubled: flush
// sides merge into one face without a seam, and a flush cut opens instead of leaving a skin.
void draw_coplanar_contact_row() {
    const Mesh left = make_box(Vec3{-1.0, -1.0, 0.0}, Vec3{0.0, 1.0, 1.5});
    const Mesh right = make_box(Vec3{0.0, -1.0, 0.0}, Vec3{1.0, 1.0, 1.5});
    draw_at(left, columnX[0], backRowY, inputColorA);
    draw_at(right, columnX[0], backRowY, inputColorB);
    draw_at(boolean(left, right, BooleanOperation::Union, "left u right"), columnX[1], backRowY, example::light_cyan());

    const Mesh block = make_box(Vec3{-1.0, -1.0, 0.0}, Vec3{1.0, 1.0, 1.5});
    const Mesh bar = make_box(Vec3{-0.4, -0.6, 0.0}, Vec3{0.4, 0.6, 1.5});
    draw_at(block, columnX[2], backRowY, inputColorA);
    draw_at(bar, columnX[2], backRowY, inputColorB);
    draw_at(boolean(block, bar, BooleanOperation::Difference, "block - bar"), columnX[3], backRowY, inputColorA);

    const Mesh bottomStep = make_box(Vec3{-1.0, -1.0, 0.0}, Vec3{1.0, 1.0, 0.5});
    const Mesh middleStep = make_box(Vec3{-0.5, -1.0, 0.5}, Vec3{1.0, 1.0, 1.0});
    const Mesh topStep = make_box(Vec3{0.0, -1.0, 1.0}, Vec3{1.0, 1.0, 1.5});
    const Mesh staircase = boolean(boolean(bottomStep, middleStep, BooleanOperation::Union, "bottom u middle step"),
                                   topStep,
                                   BooleanOperation::Union,
                                   "steps u top step");
    draw_at(staircase, columnX[4], backRowY, example::light_cyan());
}

} // namespace

int main() {
    example::init_geoqik();
    example::draw_default_origin();
    example::draw_default_grid();

    draw_box_and_sphere_row();
    draw_constructive_solid_geometry_row();
    draw_coplanar_contact_row();

    example::check_geoqik(geoqik_draw(), "Open visualization");
    example::check_geoqik(geoqik_wait_for_exit_and_cleanup(), "Close visualization");
    return EXIT_SUCCESS;
}
