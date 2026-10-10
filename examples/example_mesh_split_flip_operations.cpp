#include "draw.hpp"
#include "geoqik_init.hpp"
#include "grid.hpp"
#include "origin.hpp"

#include <Geometry/Mesh/AddTriangle.hpp>
#include <Geometry/Mesh/MakeTriangleMesh.hpp>
#include <Geometry/Mesh/MeshDelete.hpp>
#include <Geometry/Mesh/MeshFlip.hpp>
#include <Geometry/Mesh/MeshFromTriangles.hpp>
#include <Geometry/Mesh/MeshGlobalTopology.hpp>
#include <Geometry/Mesh/MeshOrientation.hpp>
#include <Geometry/Mesh/MeshQuality.hpp>
#include <Geometry/Mesh/MeshSplit.hpp>
#include <Geometry/Mesh/MeshVerify.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <span>
#include <utility>
#include <vector>

// Every case the split and flip operators handle, one per cell: the mesh before the operation on the
// left, after it on the right. Rows, front to back:
//   split_edge: interior edge | boundary edge stored on its face side | boundary edge stored on its
//     boundary side, at a hole | edge of an isolated triangle | cube crease edge, both halves stay creases
//   split_face: interior face | face with one boundary side | corner face, two boundary sides |
//     isolated triangle | cube face
//   flip_edge: convex grid diagonal, its corner drops to valence 2 | interior edge between two boundary
//     vertices | flipped twice, the original returns | all six cube face diagonals | non-convex quad
//   rejected, one panel each: flip of a boundary edge | flip of a tetrahedron edge | flip of a split
//     face's spoke, at an interior vertex of valence 3 | flip of a pillow edge | split of a pillow edge
// Faces are light blue before and light green after. A thick red line marks the operated edge or face,
// orange lines the new edges, an orange point the new vertex, and a dark green line a flipped edge. The
// non-convex quad's result is light orange: the flip is legal topologically but folds the surface, the
// case a caller must catch. Rejected cases are light red, with the edge in red.
namespace {

using Mesh = Geometry::TriangleHalfedgeMesh3d;
using VertexHandle = Mesh::VertexHandle;
using HalfedgeHandle = Mesh::HalfedgeHandle;
using EdgeHandle = Mesh::EdgeHandle;
using FaceHandle = Mesh::FaceHandle;
using Vec3 = Mesh::vec_t;
using Index = Mesh::handle_value_type;
using Triangle = std::array<Index, 3>;
using Geometry::FlipStatus;

constexpr std::array<double, 5> columnX{-8.0, -4.0, 0.0, 4.0, 8.0};
constexpr double splitEdgeRowY = -7.5;
constexpr double splitFaceRowY = -2.5;
constexpr double flipRowY = 2.5;
constexpr double rejectedRowY = 7.5;
// Before and after sit this far left and right of their cell's center.
constexpr double panelOffset = 1.0;
// Lifts the flat meshes off the ground grid, which they would otherwise depth-fight.
constexpr double lift = 0.3;
constexpr float highlightLineWidth = 3.0f;
constexpr std::size_t smallGridSize = 3;

const example::color flippedEdgeColor{0.0f, 0.55f, 0.0f, 1.0f};

// A wrong operator result is reported here, naming the scenario and the broken property, instead of
// being left for the viewer to spot.
void require(bool holds, const char* scenario, const char* property) {
    if (holds)
        return;
    std::fprintf(stderr, "%s: %s\n", scenario, property);
    example::fail_example("Validate scenario", 0);
}

// The topological checks guard the halfedge structure.
void validate_structure(const Mesh& mesh, const char* scenario) {
    require(mesh.has_valid_connectivity(), scenario, "inconsistent connectivity");
    require(Geometry::verify_manifold(mesh), scenario, "not manifold");
    require(Geometry::is_consistently_oriented(mesh), scenario, "inconsistent winding");
}

// The geometric checks catch a surface that is structurally sound but turned back on itself. Splits
// and flips keep the topology, so a closed surface must stay closed and still face outward.
void validate(const Mesh& original, const Mesh& mesh, const char* scenario) {
    validate_structure(mesh, scenario);
    require(!Geometry::has_degenerate_faces(mesh), scenario, "degenerate face");
    require(!Geometry::has_folded_edges(mesh), scenario, "folded edge");
    require(Geometry::euler_characteristic(mesh) == Geometry::euler_characteristic(original), scenario,
            "Euler characteristic changed");
    const bool closed = Geometry::verify_closed(original);
    require(Geometry::verify_closed(mesh) == closed, scenario, "closedness changed");
    if (closed)
        require(Geometry::mesh_orientation(mesh) == Geometry::MeshOrientation::Outward, scenario, "not outward-facing");
}

// A rejected operation must leave the mesh exactly as it was, not merely valid.
void require_unchanged(const Mesh& original, const Mesh& mesh, const char* scenario) {
    bool unchanged = mesh.vertex_storage_size() == original.vertex_storage_size() &&
                     mesh.halfedge_storage_size() == original.halfedge_storage_size();
    for (Index i = 0; unchanged && i < original.halfedge_storage_size(); ++i) {
        const auto& before = original.get_halfedge(HalfedgeHandle{i});
        const auto& after = mesh.get_halfedge(HalfedgeHandle{i});
        unchanged = before.targetVertex == after.targetVertex && before.next == after.next && before.face == after.face;
    }
    require(unchanged, scenario, "the rejected operation changed the mesh");
}

// The vertex (column, row) of a size x size grid built by make_grid.
VertexHandle grid_vertex(std::size_t size, std::size_t column, std::size_t row) {
    return VertexHandle{static_cast<Index>(row * size + column)};
}

// size x size vertices spaced spacing apart, centered on the origin, each cell split along its
// (i, j)-(i+1, j+1) diagonal.
Mesh make_grid(std::size_t size, double spacing) {
    Mesh mesh;
    const double half = 0.5 * spacing * static_cast<double>(size - 1);
    for (std::size_t j = 0; j < size; ++j)
        for (std::size_t i = 0; i < size; ++i)
            if (mesh.add_vertex({spacing * static_cast<double>(i) - half, spacing * static_cast<double>(j) - half, 0.0}) !=
                grid_vertex(size, i, j))
                example::fail_example("Build grid", 0);
    // add_triangle refuses a triangle meeting a used corner through two new edges, so each one must
    // attach along an existing edge: row 0 grows through each cell's upper triangle first, later
    // rows through the lower one.
    for (std::size_t j = 0; j + 1 < size; ++j) {
        for (std::size_t i = 0; i + 1 < size; ++i) {
            const std::array<VertexHandle, 3> lower{grid_vertex(size, i, j), grid_vertex(size, i + 1, j),
                                                    grid_vertex(size, i + 1, j + 1)};
            const std::array<VertexHandle, 3> upper{grid_vertex(size, i, j), grid_vertex(size, i + 1, j + 1),
                                                    grid_vertex(size, i, j + 1)};
            for (const auto& triangle : j == 0 ? std::array{upper, lower} : std::array{lower, upper})
                if (!Geometry::add_triangle(mesh, triangle).is_valid())
                    example::fail_example("Build grid", 0);
        }
    }
    return mesh;
}

VertexHandle small_grid_vertex(std::size_t column, std::size_t row) {
    return grid_vertex(smallGridSize, column, row);
}

// The 3 x 3 grid most scenarios start from.
Mesh make_small_grid() {
    return make_grid(smallGridSize, 0.8);
}

// A 4 x 4 grid without its interior face (1,1), (2,1), (2,2). add_triangle stores the face side of a
// new boundary edge, so only a deletion leaves edges whose stored halfedge is on the boundary.
Mesh make_grid_with_hole() {
    constexpr std::size_t size = 4;
    Mesh mesh = make_grid(size, 0.55);
    const HalfedgeHandle halfedge = mesh.find_halfedge(grid_vertex(size, 1, 1), grid_vertex(size, 2, 1));
    if (!halfedge.is_valid() || Geometry::delete_face(mesh, mesh.get_halfedge(halfedge).face) != Geometry::DeleteStatus::Ok)
        example::fail_example("Punch hole", 0);
    return mesh;
}

Mesh make_isolated_triangle() {
    Mesh mesh;
    const VertexHandle vertex0 = mesh.add_vertex({-0.7, -0.6, 0.0});
    const VertexHandle vertex1 = mesh.add_vertex({0.8, -0.6, 0.0});
    const VertexHandle vertex2 = mesh.add_vertex({-0.1, 0.8, 0.0});
    if (!Geometry::add_triangle(mesh, vertex0, vertex1, vertex2).is_valid())
        example::fail_example("Build triangle", 0);
    return mesh;
}

Mesh make_cube() {
    auto result = Geometry::make_triangle_mesh(Geometry::AABB3d{Vec3{-0.7, -0.7, 0.0}, Vec3{0.7, 0.7, 1.4}});
    if (!result)
        example::fail_example("Create cube", static_cast<int>(result.error));
    return std::move(result.mesh);
}

// Every pair of its vertices is joined, so no edge can be flipped.
Mesh make_tetrahedron() {
    Mesh mesh;
    const VertexHandle vertex0 = mesh.add_vertex({-0.7, -0.6, 0.0});
    const VertexHandle vertex1 = mesh.add_vertex({0.8, -0.6, 0.0});
    const VertexHandle vertex2 = mesh.add_vertex({-0.1, 0.8, 0.0});
    const VertexHandle vertex3 = mesh.add_vertex({0.0, -0.1, 1.2});
    for (const auto& triangle : {std::array{vertex0, vertex2, vertex1}, std::array{vertex0, vertex1, vertex3},
                                 std::array{vertex1, vertex2, vertex3}, std::array{vertex2, vertex0, vertex3}})
        if (!Geometry::add_triangle(mesh, triangle).is_valid())
            example::fail_example("Build tetrahedron", 0);
    return mesh;
}

// Two triangles over the same three vertices, wound oppositely: a closed surface whose two faces share
// their apex across every edge. add_triangle refuses the second triangle; make_mesh_from_triangles
// accepts it.
Mesh make_pillow() {
    const std::array<Vec3, 3> positions{Vec3{-0.7, -0.6, 0.0}, Vec3{0.8, -0.6, 0.0}, Vec3{-0.1, 0.8, 0.0}};
    const std::array<Triangle, 2> triangles{Triangle{0, 1, 2}, Triangle{1, 0, 2}};
    auto result = Geometry::make_mesh_from_triangles(std::span<const Vec3>{positions}, std::span<const Triangle>{triangles});
    if (!result)
        example::fail_example("Create pillow", static_cast<int>(result.error));
    return std::move(result.mesh);
}

// Two triangles on the edge from vertex 0 to vertex 1 whose quad has a reflex corner at vertex 0, so
// the other diagonal runs outside it.
Mesh make_non_convex_quad() {
    Mesh mesh;
    const VertexHandle start = mesh.add_vertex({-0.3, -0.1, 0.0});
    const VertexHandle end = mesh.add_vertex({0.9, -0.1, 0.0});
    const VertexHandle leftApex = mesh.add_vertex({0.3, 0.5, 0.0});
    const VertexHandle rightApex = mesh.add_vertex({-0.9, -0.4, 0.0});
    if (!Geometry::add_triangle(mesh, start, end, leftApex).is_valid() ||
        !Geometry::add_triangle(mesh, end, start, rightApex).is_valid())
        example::fail_example("Build non-convex quad", 0);
    return mesh;
}

EdgeHandle edge_between(const Mesh& mesh, VertexHandle first, VertexHandle second) {
    const HalfedgeHandle halfedge = mesh.find_halfedge(first, second);
    if (!halfedge.is_valid())
        example::fail_example("Find edge", 0);
    return mesh.get_halfedge(halfedge).edge;
}

template <typename TPredicate>
EdgeHandle find_edge(const Mesh& mesh, TPredicate predicate) {
    for (const EdgeHandle edge : mesh.edges())
        if (predicate(edge))
            return edge;
    example::fail_example("Find edge", 0);
}

std::size_t boundary_side_count(const Mesh& mesh, FaceHandle face) {
    std::size_t count = 0;
    for (const HalfedgeHandle side : mesh.halfedges_around_face(face))
        count += mesh.is_boundary(mesh.get_halfedge(side).twin) ? 1U : 0U;
    return count;
}

FaceHandle face_with_boundary_sides(const Mesh& mesh, std::size_t boundarySides) {
    for (const FaceHandle face : mesh.faces())
        if (boundary_side_count(mesh, face) == boundarySides)
            return face;
    example::fail_example("Find face", 0);
}

std::vector<EdgeHandle> face_sides(const Mesh& mesh, FaceHandle face) {
    std::vector<EdgeHandle> sides;
    for (const HalfedgeHandle side : mesh.halfedges_around_face(face))
        sides.push_back(mesh.get_halfedge(side).edge);
    return sides;
}

Vec3 edge_midpoint(const Mesh& mesh, EdgeHandle edge) {
    const HalfedgeHandle halfedge = mesh.get_edge(edge).halfedge;
    return Vec3{0.5 * (mesh.get_position(mesh.source_vertex(halfedge)) + mesh.get_position(mesh.target_vertex(halfedge)))};
}

Vec3 face_centroid(const Mesh& mesh, FaceHandle face) {
    const auto corners = mesh.vertices_around_face(face);
    return Vec3{(mesh.get_position(corners[0]) + mesh.get_position(corners[1]) + mesh.get_position(corners[2])) / 3.0};
}

// Splits and flips only append, so the edges at or above the storage size before an operation are new.
std::vector<EdgeHandle> appended_edges(const Mesh& mesh, std::size_t edgeStorageBefore) {
    std::vector<EdgeHandle> edges;
    for (std::size_t i = edgeStorageBefore; i < mesh.edge_storage_size(); ++i)
        edges.emplace_back(static_cast<Index>(i));
    return edges;
}

// mesh moved to (x, y) and lifted, with every edge a crease. Each face is then shaded flat with its own
// normal: averaged vertex normals would cancel where faces turn back on each other (the pillow, the
// folded quad), and faces are what these operators change.
Mesh placed(const Mesh& mesh, double x, double y) {
    Mesh moved = mesh;
    const Vec3 offset{x, y, lift};
    for (const VertexHandle vertex : moved.vertices())
        moved.set_position(vertex, Vec3{moved.get_position(vertex) + offset});
    for (const EdgeHandle edge : moved.edges())
        moved.set_crease(edge, true);
    return moved;
}

// Thick lines along edges, over the thin triangle edges of the wireframe.
void draw_edges(const Mesh& mesh, std::span<const EdgeHandle> edges, const example::color& color) {
    example::draw_mesh_edges(mesh, edges, color, highlightLineWidth);
}

// What an operation changed, highlighted in the panel after it.
struct Changes {
    std::vector<EdgeHandle> newEdges;
    std::vector<EdgeHandle> flippedEdges;
    VertexHandle newVertex{};
};

void draw_before(const Mesh& mesh, std::span<const EdgeHandle> operated, double x, double y) {
    const Mesh moved = placed(mesh, x - panelOffset, y);
    example::draw_wireframe_mesh(moved, example::light_blue());
    draw_edges(moved, operated, example::red());
}

void draw_after(const Mesh& mesh, const Changes& changes, const example::color& faceColor, double x, double y) {
    const Mesh moved = placed(mesh, x + panelOffset, y);
    example::draw_wireframe_mesh(moved, faceColor);
    draw_edges(moved, changes.newEdges, example::orange());
    draw_edges(moved, changes.flippedEdges, flippedEdgeColor);
    if (changes.newVertex.is_valid()) {
        const Vec3& position = moved.get_position(changes.newVertex);
        example::draw(linal::float3{static_cast<float>(position[0]), static_cast<float>(position[1]),
                                    static_cast<float>(position[2])},
                      example::orange());
    }
}

// A rejected case gets a single panel at its cell's center.
void draw_rejected(const Mesh& mesh, EdgeHandle edge, std::size_t column) {
    const Mesh moved = placed(mesh, columnX[column], rejectedRowY);
    example::draw_wireframe_mesh(moved, example::light_red());
    draw_edges(moved, std::array{edge}, example::red());
}

void show_split_edge(const Mesh& original, EdgeHandle edge, std::size_t column, const char* scenario) {
    draw_before(original, std::array{edge}, columnX[column], splitEdgeRowY);
    Mesh mesh = original;
    const VertexHandle end = mesh.target_vertex(mesh.get_edge(edge).halfedge);

    const VertexHandle middle = Geometry::split_edge(mesh, edge, edge_midpoint(mesh, edge));

    require(middle.is_valid(), scenario, "split_edge rejected the edge");
    // A feature line stays marked along both halves.
    const bool crease = original.is_crease(edge);
    require(mesh.is_crease(edge) == crease && mesh.is_crease(edge_between(mesh, middle, end)) == crease, scenario,
            "crease flag not kept on both halves");
    validate(original, mesh, scenario);
    draw_after(mesh, Changes{appended_edges(mesh, original.edge_storage_size()), {}, middle}, example::light_green(),
               columnX[column], splitEdgeRowY);
}

void show_split_face(const Mesh& original, FaceHandle face, std::size_t column, const char* scenario) {
    draw_before(original, face_sides(original, face), columnX[column], splitFaceRowY);
    Mesh mesh = original;

    const VertexHandle center = Geometry::split_face(mesh, face, face_centroid(mesh, face));

    require(center.is_valid(), scenario, "split_face rejected the face");
    validate(original, mesh, scenario);
    draw_after(mesh, Changes{appended_edges(mesh, original.edge_storage_size()), {}, center}, example::light_green(),
               columnX[column], splitFaceRowY);
}

Mesh flipped(const Mesh& original, std::span<const EdgeHandle> edges, int flipsPerEdge, const char* scenario) {
    Mesh mesh = original;
    for (int round = 0; round < flipsPerEdge; ++round)
        for (const EdgeHandle edge : edges)
            require(Geometry::flip_edge(mesh, edge) == FlipStatus::Ok, scenario, "flip_edge rejected the edge");
    return mesh;
}

void show_flip(const Mesh& original, std::span<const EdgeHandle> edges, int flipsPerEdge, std::size_t column,
               const char* scenario) {
    draw_before(original, edges, columnX[column], flipRowY);
    const Mesh mesh = flipped(original, edges, flipsPerEdge, scenario);
    validate(original, mesh, scenario);
    draw_after(mesh, Changes{{}, {edges.begin(), edges.end()}, {}}, example::light_green(), columnX[column], flipRowY);
}

// is_flip_ok is purely topological, so it accepts this flip although the result folds; the fold check
// is the caller's, and here it confirms the fold instead.
void show_folding_flip(const Mesh& original, EdgeHandle edge, std::size_t column, const char* scenario) {
    draw_before(original, std::array{edge}, columnX[column], flipRowY);
    const Mesh mesh = flipped(original, std::array{edge}, 1, scenario);
    validate_structure(mesh, scenario);
    require(Geometry::has_folded_edges(mesh), scenario, "expected the flip to fold the surface");
    draw_after(mesh, Changes{{}, {edge}, {}}, example::light_orange(), columnX[column], flipRowY);
}

void show_rejected_flip(const Mesh& original, EdgeHandle edge, FlipStatus expected, std::size_t column,
                        const char* scenario) {
    Mesh mesh = original;
    require(Geometry::flip_edge(mesh, edge) == expected, scenario, "unexpected flip status");
    require_unchanged(original, mesh, scenario);
    validate_structure(mesh, scenario);
    draw_rejected(mesh, edge, column);
}

void show_rejected_split_edge(const Mesh& original, EdgeHandle edge, std::size_t column, const char* scenario) {
    Mesh mesh = original;
    require(!Geometry::split_edge(mesh, edge, edge_midpoint(mesh, edge)).is_valid(), scenario,
            "split_edge accepted the edge");
    require_unchanged(original, mesh, scenario);
    validate_structure(mesh, scenario);
    draw_rejected(mesh, edge, column);
}

void show_split_edge_row() {
    const Mesh grid = make_small_grid();
    show_split_edge(grid, edge_between(grid, small_grid_vertex(0, 0), small_grid_vertex(1, 1)), 0, "split_edge, interior edge");
    const EdgeHandle bottom = edge_between(grid, small_grid_vertex(0, 0), small_grid_vertex(1, 0));
    require(!grid.is_boundary(grid.get_edge(bottom).halfedge), "split_edge, boundary edge", "not stored on its face side");
    show_split_edge(grid, bottom, 1, "split_edge, boundary edge stored on its face side");

    const Mesh holed = make_grid_with_hole();
    const EdgeHandle holeEdge = find_edge(holed, [&](EdgeHandle edge) { return holed.is_boundary(holed.get_edge(edge).halfedge); });
    show_split_edge(holed, holeEdge, 2, "split_edge, boundary edge stored on its boundary side");

    const Mesh triangle = make_isolated_triangle();
    show_split_edge(triangle, *triangle.edges().begin(), 3, "split_edge, edge of an isolated triangle");

    const Mesh cube = make_cube();
    show_split_edge(cube, find_edge(cube, [&](EdgeHandle edge) { return cube.is_crease(edge); }), 4,
                    "split_edge, cube crease edge");
}

void show_split_face_row() {
    const Mesh grid = make_small_grid();
    show_split_face(grid, face_with_boundary_sides(grid, 0), 0, "split_face, interior face");
    show_split_face(grid, face_with_boundary_sides(grid, 1), 1, "split_face, face with one boundary side");
    show_split_face(grid, face_with_boundary_sides(grid, 2), 2, "split_face, corner face");

    const Mesh triangle = make_isolated_triangle();
    show_split_face(triangle, *triangle.faces().begin(), 3, "split_face, isolated triangle");

    const Mesh cube = make_cube();
    show_split_face(cube, *cube.faces().begin(), 4, "split_face, cube face");
}

void show_flip_row() {
    const Mesh grid = make_small_grid();
    show_flip(grid, std::array{edge_between(grid, small_grid_vertex(0, 0), small_grid_vertex(1, 1))}, 1, 0,
              "flip_edge, convex grid diagonal");
    show_flip(grid, std::array{edge_between(grid, small_grid_vertex(1, 0), small_grid_vertex(2, 1))}, 1, 1,
              "flip_edge, edge between two boundary vertices");
    show_flip(grid, std::array{edge_between(grid, small_grid_vertex(1, 1), small_grid_vertex(2, 2))}, 2, 2,
              "flip_edge, flipped twice");

    // The cube's non-crease edges are its face diagonals.
    const Mesh cube = make_cube();
    std::vector<EdgeHandle> diagonals;
    for (const EdgeHandle edge : cube.edges())
        if (!cube.is_crease(edge))
            diagonals.push_back(edge);
    require(diagonals.size() == 6, "flip_edge, cube face diagonals", "expected six face diagonals");
    show_flip(cube, diagonals, 1, 3, "flip_edge, cube face diagonals");

    const Mesh quad = make_non_convex_quad();
    show_folding_flip(quad, edge_between(quad, VertexHandle{0}, VertexHandle{1}), 4, "flip_edge, non-convex quad");
}

void show_rejected_row() {
    const Mesh grid = make_small_grid();
    show_rejected_flip(grid, edge_between(grid, small_grid_vertex(0, 0), small_grid_vertex(1, 0)), FlipStatus::BoundaryEdge, 0,
                       "flip_edge, boundary edge");

    const Mesh tetrahedron = make_tetrahedron();
    show_rejected_flip(tetrahedron, *tetrahedron.edges().begin(), FlipStatus::DiagonalExists, 1,
                       "flip_edge, tetrahedron edge");

    Mesh splitGrid = make_small_grid();
    const FaceHandle face = face_with_boundary_sides(splitGrid, 0);
    const VertexHandle corner = splitGrid.vertices_around_face(face)[0];
    const VertexHandle center = Geometry::split_face(splitGrid, face, face_centroid(splitGrid, face));
    require(center.is_valid(), "flip_edge, spoke of a split face", "split_face rejected the face");
    show_rejected_flip(splitGrid, edge_between(splitGrid, center, corner), FlipStatus::DiagonalExists, 2,
                       "flip_edge, spoke of a split face");

    const Mesh pillow = make_pillow();
    const EdgeHandle pillowEdge = *pillow.edges().begin();
    show_rejected_flip(pillow, pillowEdge, FlipStatus::DiagonalExists, 3, "flip_edge, pillow edge");
    show_rejected_split_edge(pillow, pillowEdge, 4, "split_edge, pillow edge");
}

} // namespace

int main() {
    example::init_geoqik();
    example::draw_default_origin();
    example::draw_default_grid();

    show_split_edge_row();
    show_split_face_row();
    show_flip_row();
    show_rejected_row();

    example::check_geoqik(geoqik_draw(), "Open visualization");
    example::check_geoqik(geoqik_wait_for_exit_and_cleanup(), "Close visualization");
    return EXIT_SUCCESS;
}
