#include "draw.hpp"
#include "geoqik_init.hpp"
#include "grid.hpp"
#include "origin.hpp"

#include <Geometry/Cylinder.hpp>
#include <Geometry/Mesh/AddTriangle.hpp>
#include <Geometry/Mesh/MakeTriangleMesh.hpp>
#include <Geometry/Mesh/MeshEdgeCollapse.hpp>
#include <Geometry/Mesh/MeshEdgeCollapseChecks.hpp>
#include <Geometry/Mesh/MeshDelete.hpp>
#include <Geometry/Mesh/MeshManifold.hpp>
#include <Geometry/Mesh/MeshOrientation.hpp>
#include <Geometry/Mesh/MeshQuality.hpp>
#include <Geometry/Mesh/MeshTopology.hpp>
#include <Geometry/Mesh/MeshVertexRemoval.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <numbers>
#include <optional>
#include <random>
#include <thread>
#include <utility>
#include <vector>

// Side-by-side panels of the same inputs, first shown unmodified, then after each removal operator:
//   front row (wavy open surface): original | edge collapse | halfedge collapse | retriangulation
//   back row: closed cylinder original | halfedge collapse | retriangulation | surface with holes
namespace {

using Mesh = Geometry::TriangleHalfedgeMesh3d;
using VertexHandle = Mesh::VertexHandle;
using EdgeHandle = Mesh::EdgeHandle;
using FaceHandle = Mesh::FaceHandle;

constexpr std::size_t surfaceResolution = 11;
constexpr double surfaceExtent = 4.0;
constexpr std::size_t decimatedVertexCount = 45;

// The default corner limit only rejects nearly flat triangles. On the wavy surface that still lets
// the removal operators leave thin slivers standing edge-on across it, so ask for sounder triangles.
const Geometry::MeshGeometryLimits<double> decimationLimits{
    .maxCornerAngle = std::numbers::pi * 150.0 / 180.0};

// A wavy height field over [originX, originX + extent] x [originY, originY + extent], lifted above
// the ground grid. The waves make the fold-over checks work on a genuinely curved surface.
Mesh make_wavy_surface(double originX, double originY) {
    Mesh mesh;
    std::vector<VertexHandle> vertices;
    const double step = surfaceExtent / static_cast<double>(surfaceResolution - 1);
    for (std::size_t j = 0; j < surfaceResolution; ++j) {
        for (std::size_t i = 0; i < surfaceResolution; ++i) {
            const double u = static_cast<double>(i) * step;
            const double v = static_cast<double>(j) * step;
            const double height = 1.0 + 0.35 * std::sin(1.6 * u) * std::cos(1.3 * v);
            vertices.push_back(mesh.add_vertex({originX + u, originY + v, height}));
        }
    }
    const auto at = [&](std::size_t i, std::size_t j) { return vertices[j * surfaceResolution + i]; };
    // add_triangle refuses a triangle meeting a used corner through two new edges, so each one must
    // attach along an existing edge: row 0 grows through each cell's upper triangle first, later
    // rows through the lower one.
    for (std::size_t j = 0; j + 1 < surfaceResolution; ++j) {
        for (std::size_t i = 0; i + 1 < surfaceResolution; ++i) {
            const std::array<VertexHandle, 3> lower{at(i, j), at(i + 1, j), at(i + 1, j + 1)};
            const std::array<VertexHandle, 3> upper{at(i, j), at(i + 1, j + 1), at(i, j + 1)};
            for (const auto& triangle : j == 0 ? std::array{upper, lower} : std::array{lower, upper})
                if (!Geometry::add_triangle(mesh, triangle).is_valid())
                    example::fail_example("Build wavy surface", 0);
        }
    }
    return mesh;
}

Mesh make_cylinder(double centerX, double centerY) {
    const Geometry::Cylinder<double> cylinder{
        Geometry::Segment3d{{centerX, centerY, 0.5}, {centerX, centerY, 3.5}}, 1.5};
    auto result = Geometry::make_triangle_mesh(cylinder, 24);
    if (!result)
        example::fail_example("Create cylinder", static_cast<int>(result.error));
    return std::move(result.mesh);
}

// Where an edge collapse puts the merged vertex: on the boundary endpoint when only one endpoint is
// on the boundary (so the outline keeps its shape), otherwise at the midpoint.
Mesh::vec_t merged_position(const Mesh& mesh, EdgeHandle edge) {
    const auto halfedge = mesh.get_edge(edge).halfedge;
    const VertexHandle source = mesh.source_vertex(halfedge);
    const VertexHandle target = mesh.target_vertex(halfedge);
    const bool sourceOnBoundary = Geometry::is_boundary(mesh, source);
    const bool targetOnBoundary = Geometry::is_boundary(mesh, target);
    if (sourceOnBoundary != targetOnBoundary)
        return mesh.get_position(sourceOnBoundary ? source : target);
    return Mesh::vec_t{0.5 * (mesh.get_position(source) + mesh.get_position(target))};
}

// Greedy shortest-edge decimation with the edge collapse operator: each step collapses the shortest
// edge that is topologically legal and folds no face at its merged position.
void decimate_by_edge_collapse(Mesh& mesh, std::size_t targetVertexCount) {
    while (mesh.vertex_count() > targetVertexCount) {
        std::optional<EdgeHandle> shortest;
        double shortestLength = std::numeric_limits<double>::max();
        for (const EdgeHandle edge : mesh.edges()) {
            const auto halfedge = mesh.get_edge(edge).halfedge;
            const double length = linal::length(
                Mesh::vec_t{mesh.get_position(mesh.target_vertex(halfedge)) - mesh.get_position(mesh.source_vertex(halfedge))});
            if (length >= shortestLength ||
                Geometry::check_collapse(mesh, halfedge, merged_position(mesh, edge), decimationLimits) != Geometry::CollapseStatus::Ok)
                continue;
            shortest = edge;
            shortestLength = length;
        }
        if (!shortest)
            return;
        const auto result = Geometry::collapse_edge(mesh, *shortest, merged_position(mesh, *shortest), decimationLimits);
        if (!result)
            example::fail_example("Collapse edge", static_cast<int>(result.status));
    }
}

// Removes vertices in a fixed pseudo-random order with the given removal operator until the target
// count is reached or no remaining vertex can be removed.
template <typename TRemove>
void decimate_by_vertex_removal(Mesh& mesh, std::size_t targetVertexCount, TRemove remove) {
    std::mt19937 generator(7);
    bool progress = true;
    while (progress && mesh.vertex_count() > targetVertexCount) {
        std::vector<VertexHandle> candidates(mesh.vertices().begin(), mesh.vertices().end());
        std::shuffle(candidates.begin(), candidates.end(), generator);
        progress = false;
        for (const VertexHandle vertex : candidates) {
            if (mesh.vertex_count() <= targetVertexCount)
                break;
            if (!mesh.is_deleted(vertex) && remove(mesh, vertex))
                progress = true;
        }
    }
}

bool remove_by_halfedge_collapse(Mesh& mesh, VertexHandle vertex) {
    return Geometry::remove_vertex(mesh, vertex, decimationLimits).has_value();
}

bool remove_by_retriangulation(Mesh& mesh, VertexHandle vertex) {
    return Geometry::remove_vertex_retriangulate(mesh, vertex, decimationLimits) == Geometry::VertexRemovalStatus::Ok;
}

// A bad operator result is reported here, naming the broken property, instead of being left for the
// viewer to reveal. The topological checks guard the halfedge structure; the geometric ones catch a
// surface that is structurally sound but turned back on itself.
void validate(const Mesh& mesh, const char* operation) {
    const auto require = [operation](bool holds, const char* property) {
        if (holds) return;
        std::fprintf(stderr, "%s: %s\n", operation, property);
        example::fail_example("Validate mesh", 0);
    };
    require(mesh.has_valid_connectivity(), "inconsistent connectivity");
    require(Geometry::verify_manifold(mesh), "not manifold");
    require(Geometry::is_consistently_oriented(mesh), "inconsistent winding");
    require(!Geometry::has_degenerate_faces(mesh), "degenerate face");
    require(!Geometry::has_folded_edges(mesh), "folded edge");
}

// A closed surface must additionally still enclose its volume with outward-facing normals.
void validate_closed(const Mesh& mesh, const char* operation) {
    validate(mesh, operation);
    if (Geometry::mesh_orientation(mesh) != Geometry::MeshOrientation::Outward) {
        std::fprintf(stderr, "%s: not a closed, outward-facing surface\n", operation);
        example::fail_example("Validate mesh", 0);
    }
}

// Punches holes: deletes a few interior vertices (each leaves a polygonal hole) and a few single
// faces. delete_face / delete_vertex refuse deletions that would pinch a vertex, so every hole
// here stays a clean boundary loop.
void punch_holes(Mesh& mesh) {
    const auto grid_vertex = [](std::size_t i, std::size_t j) {
        return VertexHandle{static_cast<std::uint32_t>(j * surfaceResolution + i)};
    };
    for (const auto& [i, j] : std::array<std::array<std::size_t, 2>, 3>{{{2, 3}, {7, 2}, {5, 7}}})
        if (Geometry::delete_vertex(mesh, grid_vertex(i, j)) != Geometry::MeshDeleteStatus::Ok)
            example::fail_example("Delete vertex", 0);

    for (const auto& [i, j] : std::array<std::array<std::size_t, 2>, 2>{{{2, 7}, {7, 6}}}) {
        const auto halfedge = mesh.find_halfedge(grid_vertex(i, j), grid_vertex(i + 1, j));
        const FaceHandle face = mesh.get_halfedge(halfedge).face;
        if (Geometry::delete_face(mesh, face) != Geometry::MeshDeleteStatus::Ok)
            example::fail_example("Delete face", 0);
    }
}

// Removal operators only tombstone elements, so compact before drawing. The renderer outlines crease
// edges only; tagging every edge as a crease shows the full triangulation, the thing these operators
// change, with flat per-face shading. Thin lines keep the dense wireframe from hiding the faces.
geoqik_uuid_t finish_and_draw(Mesh& mesh, const example::color& color) {
    constexpr float wireframeLineWidth = 1.0f;
    mesh.garbage_collection();
    for (const EdgeHandle edge : mesh.edges())
        mesh.set_crease(edge, true);
    return example::draw(mesh, color, wireframeLineWidth);
}

// geoqik can only move the vertices of a drawn mesh, not change its topology, so an operated mesh
// replaces its earlier drawing.
void redraw(Mesh& mesh, const geoqik_uuid_t& drawing, const example::color& color) {
    example::check_geoqik(geoqik_remove_mesh(&drawing), "Remove mesh");
    finish_and_draw(mesh, color);
}

} // namespace

int main() {
    example::init_geoqik();
    example::draw_default_origin();
    example::draw_default_grid();

    constexpr std::array<double, 4> columnX{-9.25, -4.5, 0.25, 5.0};
    constexpr double frontRowY = -9.0;
    constexpr double backRowY = 3.0;
    constexpr double cylinderOffset = 0.5 * surfaceExtent;
    // Long enough for the viewer to take in the unmodified meshes before the operators change them.
    constexpr auto operationDelay = std::chrono::seconds(2);

    Mesh original = make_wavy_surface(columnX[0], frontRowY);
    finish_and_draw(original, example::light_blue());

    Mesh edgeCollapsed = make_wavy_surface(columnX[1], frontRowY);
    const geoqik_uuid_t edgeCollapsedDrawing = finish_and_draw(edgeCollapsed, example::orange());

    Mesh halfedgeCollapsed = make_wavy_surface(columnX[2], frontRowY);
    const geoqik_uuid_t halfedgeCollapsedDrawing = finish_and_draw(halfedgeCollapsed, example::green());

    Mesh retriangulated = make_wavy_surface(columnX[3], frontRowY);
    const geoqik_uuid_t retriangulatedDrawing = finish_and_draw(retriangulated, example::magenta());

    Mesh cylinder = make_cylinder(columnX[0] + cylinderOffset, backRowY + cylinderOffset);
    const std::size_t cylinderTarget = cylinder.vertex_count() / 2;
    finish_and_draw(cylinder, example::light_blue());

    Mesh collapsedCylinder = make_cylinder(columnX[1] + cylinderOffset, backRowY + cylinderOffset);
    const geoqik_uuid_t collapsedCylinderDrawing = finish_and_draw(collapsedCylinder, example::green());

    Mesh retriangulatedCylinder = make_cylinder(columnX[2] + cylinderOffset, backRowY + cylinderOffset);
    const geoqik_uuid_t retriangulatedCylinderDrawing = finish_and_draw(retriangulatedCylinder, example::magenta());

    Mesh holes = make_wavy_surface(columnX[3], backRowY);
    const geoqik_uuid_t holesDrawing = finish_and_draw(holes, example::cyan());

    example::check_geoqik(geoqik_draw(), "Open visualization");
    std::this_thread::sleep_for(operationDelay);

    decimate_by_edge_collapse(edgeCollapsed, decimatedVertexCount);
    validate(edgeCollapsed, "Edge collapse");
    redraw(edgeCollapsed, edgeCollapsedDrawing, example::orange());

    decimate_by_vertex_removal(halfedgeCollapsed, decimatedVertexCount, remove_by_halfedge_collapse);
    validate(halfedgeCollapsed, "Halfedge collapse");
    redraw(halfedgeCollapsed, halfedgeCollapsedDrawing, example::green());

    decimate_by_vertex_removal(retriangulated, decimatedVertexCount, remove_by_retriangulation);
    validate(retriangulated, "Retriangulation");
    redraw(retriangulated, retriangulatedDrawing, example::magenta());

    decimate_by_vertex_removal(collapsedCylinder, cylinderTarget, remove_by_halfedge_collapse);
    validate_closed(collapsedCylinder, "Cylinder halfedge collapse");
    redraw(collapsedCylinder, collapsedCylinderDrawing, example::green());

    decimate_by_vertex_removal(retriangulatedCylinder, cylinderTarget, remove_by_retriangulation);
    validate_closed(retriangulatedCylinder, "Cylinder retriangulation");
    redraw(retriangulatedCylinder, retriangulatedCylinderDrawing, example::magenta());

    punch_holes(holes);
    validate(holes, "Punch holes");
    redraw(holes, holesDrawing, example::cyan());

    example::check_geoqik(geoqik_wait_for_exit_and_cleanup(), "Close visualization");
    return EXIT_SUCCESS;
}
