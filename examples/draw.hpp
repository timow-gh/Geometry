#pragma once

#include "geoqik_init.hpp"

#include <Geometry/Mesh/MeshBuffers.hpp>
#include <Geometry/Segment.hpp>

#include <GeoQik/GeoQik.hpp>

#include <linal/vec.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace example {

struct color {
    std::array<float, 4> rgba = {1.0f, 1.0f, 1.0f, 1.0f};
    color(float r, float g, float b, float a)
        : rgba{r, g, b, a} {}
};

inline color white() {
    return color(1.0f, 1.0f, 1.0f, 1.0f);
}
inline color grey() {
    return color(0.5f, 0.5f, 0.5f, 1.0f);
}
inline color black() {
    return color(0.0f, 0.0f, 0.0f, 1.0f);
}
inline color red() {
    return color(1.0f, 0.0f, 0.0f, 1.0f);
}
inline color light_red() {
    return color(1.0f, 0.5f, 0.5f, 1.0f);
}
inline color green() {
    return color(0.0f, 1.0f, 0.0f, 1.0f);
}
inline color light_green() {
    return color(0.5f, 1.0f, 0.5f, 1.0f);
}
inline color blue() {
    return color(0.0f, 0.0f, 1.0f, 1.0f);
}
inline color light_blue() {
    return color(0.5f, 0.5f, 1.0f, 1.0f);
}
inline color yellow() {
    return color(1.0f, 1.0f, 0.0f, 1.0f);
}
inline color light_yellow() {
    return color(1.0f, 1.0f, 0.5f, 1.0f);
}
inline color cyan() {
    return color(0.0f, 1.0f, 1.0f, 1.0f);
}
inline color light_cyan() {
    return color(0.5f, 1.0f, 1.0f, 1.0f);
}
inline color magenta() {
    return color(1.0f, 0.0f, 1.0f, 1.0f);
}
inline color light_magenta() {
    return color(1.0f, 0.5f, 1.0f, 1.0f);
}
inline color orange() {
    return color(1.0f, 0.5f, 0.0f, 1.0f);
}
inline color light_orange() {
    return color(1.0f, 0.75f, 0.5f, 1.0f);
}

inline geoqik_uuid_t draw(const Geometry::Segment3d& line, const color& color) {
    geoqik_add_line_opts_t opts{};
    opts.color = color.rgba.data();
    opts.colorCount = color.rgba.size();
    geoqik_result_t result = geoqik_add_line_opts(line.get_source()[0],
                                                  line.get_source()[1],
                                                  line.get_source()[2],
                                                  line.get_target()[0],
                                                  line.get_target()[1],
                                                  line.get_target()[2],
                                                  &opts);
    check_geoqik(result.err, "Draw segment");
    return result.geometryId;
}

inline geoqik_uuid_t draw(const Geometry::Segment2d& line, const color& color) {
    geoqik_add_line_opts_t opts{};
    opts.color = color.rgba.data();
    opts.colorCount = color.rgba.size();
    geoqik_result_t result = geoqik_add_line_opts(line.get_source()[0],
                                                  line.get_source()[1],
                                                  0.0f,
                                                  line.get_target()[0],
                                                  line.get_target()[1],
                                                  0.0f,
                                                  &opts);
    check_geoqik(result.err, "Draw segment");
    return result.geometryId;
}

inline geoqik_uuid_t draw(const linal::float3& source, const color& color) {
    geoqik_add_points_options_t opts{};
    opts.color = color.rgba.data();
    opts.colorCount = color.rgba.size();
    geoqik_result_t result = geoqik_add_point_opts(source[0], source[1], source[2], &opts);
    check_geoqik(result.err, "Draw point");
    return result.geometryId;
}

// A positive segmentDepthLayer lifts the edges off the surface they lie on, which they otherwise
// depth-fight; 0 keeps geoqik's legacy unbiased edges.
template <std::floating_point T, typename TIndex>
inline geoqik_uuid_t draw(const Geometry::TriangleHalfedgeMesh<T, 3, TIndex>& mesh,
                          const color& surfaceColor,
                          float segmentLineWidth = 5.0f,
                          std::int32_t segmentDepthLayer = 0) {
    // Split vertices along crease edges so each render vertex carries one normal, matching geoqik's
    // one-normal-per-vertex contract. Smooth sectors stay shared; creases produce sharp shading.
    const auto buffers = Geometry::make_render_buffers(mesh);
    if (!buffers) fail_example("Create render buffers", static_cast<int>(buffers.error));

    const auto edgeColor = black();
    geoqik_add_mesh_opts_t options{};
    options.color = surfaceColor.rgba.data();
    options.colorCount = surfaceColor.rgba.size();
    options.normals = buffers.normals.data();
    options.normalsCount = buffers.normals.size();
    options.segmentIndices = buffers.segments.data();
    // Unlike vertexCount and triangleCount, this field counts scalar indices.
    options.segmentIndexCount = buffers.segments.size();
    options.segmentColor = edgeColor.rgba.data();
    // Without segments geoqik would outline every triangle; a mesh without creases shows none.
    options.showSegments = buffers.segments.empty() ? 0 : 1;
    options.segmentLineWidth = segmentLineWidth;
    if (segmentDepthLayer != 0) {
        options.segmentStyleSet = 1;
        options.segmentStyle.lineWidth = segmentLineWidth;
        options.segmentStyle.depthLayer = segmentDepthLayer;
        options.segmentLineType = GEOQIK_LINE_TYPE_LINES;
    }
    options.showVertices = 0;
    options.vertexPointSize = 2.0f;
    const auto result = geoqik_add_mesh_opts(buffers.positions.data(), buffers.vertex_count(),
                                             buffers.triangles.data(), buffers.triangles.size() / 3, &options);
    check_geoqik(result.err, "Draw mesh");
    return result.geometryId;
}

// The positions of the render vertices index pairs name, flattened as geoqik's line arrays expect:
// six coordinates per line.
inline std::vector<double> line_endpoints(const Geometry::MeshRenderBuffers& buffers,
                                          std::span<const Geometry::BufferIndex> pairs) {
    std::vector<double> endpoints;
    endpoints.reserve(3 * pairs.size());
    for (const Geometry::BufferIndex index : pairs)
        for (std::size_t axis = 0; axis < 3; ++axis)
            endpoints.push_back(buffers.positions[3 * index + axis]);
    return endpoints;
}

// Every edge of the triangles in buffers, once, as index pairs. Along a crease the two sides use
// different render vertices, so such an edge appears twice, drawn on top of itself.
inline std::vector<Geometry::BufferIndex> triangle_edges(const Geometry::MeshRenderBuffers& buffers) {
    using Edge = std::array<Geometry::BufferIndex, 2>;
    std::vector<Edge> edges;
    edges.reserve(buffers.triangles.size());
    for (std::size_t first = 0; first + 2 < buffers.triangles.size(); first += 3) {
        for (std::size_t side = 0; side < 3; ++side) {
            const Geometry::BufferIndex start = buffers.triangles[first + side];
            const Geometry::BufferIndex end = buffers.triangles[first + (side + 1) % 3];
            edges.push_back(Edge{std::min(start, end), std::max(start, end)});
        }
    }
    std::ranges::sort(edges);
    const auto duplicates = std::ranges::unique(edges);
    edges.erase(duplicates.begin(), duplicates.end());

    std::vector<Geometry::BufferIndex> indices;
    indices.reserve(2 * edges.size());
    for (const Edge& edge : edges)
        indices.insert(indices.end(), edge.begin(), edge.end());
    return indices;
}

// Lines lifted off the faces they lie on, which they would otherwise depth-fight; a higher depth
// layer draws over a lower one.
inline geoqik_result_t draw_lines(const std::vector<double>& endpoints,
                                  const color& color,
                                  float lineWidth,
                                  std::int32_t depthLayer) {
    geoqik_add_line_opts_t options{};
    options.color = color.rgba.data();
    options.colorCount = color.rgba.size();
    options.styleSet = 1;
    options.style.lineWidth = lineWidth;
    options.style.depthLayer = depthLayer;
    options.lineType = GEOQIK_LINE_TYPE_LINES;
    return geoqik_add_lines_opts(endpoints.data(), endpoints.size(), &options);
}

// Draws mesh's faces in surfaceColor and every triangle edge as a thin black line on top. Separate
// lines rather than the mesh's own segment overlay, because geoqik draws that overlay in white
// whatever color is asked for. Returns the render buffers, so that a caller can outline more edges.
template <std::floating_point T, typename TIndex>
inline Geometry::MeshRenderBuffers draw_wireframe_mesh(const Geometry::TriangleHalfedgeMesh<T, 3, TIndex>& mesh,
                                                       const color& surfaceColor) {
    // Thin, so that a densely triangulated body keeps its color between the lines.
    constexpr float wireframeLineWidth = 0.75f;
    auto buffers = Geometry::make_render_buffers(mesh);
    if (!buffers) fail_example("Create render buffers", static_cast<int>(buffers.error));

    geoqik_add_mesh_opts_t options{};
    options.color = surfaceColor.rgba.data();
    options.colorCount = surfaceColor.rgba.size();
    options.normals = buffers.normals.data();
    options.normalsCount = buffers.normals.size();
    const auto result = geoqik_add_mesh_opts(buffers.positions.data(), buffers.vertex_count(),
                                             buffers.triangles.data(), buffers.triangles.size() / 3, &options);
    check_geoqik(result.err, "Draw mesh");
    check_geoqik(draw_lines(line_endpoints(buffers, triangle_edges(buffers)), black(), wireframeLineWidth, 1).err,
                 "Draw triangle edges");
    return buffers;
}

template <typename TGeomIter, typename TOutputIterator>
inline void draw(TGeomIter geometryBegin, TGeomIter geometryEnd, TOutputIterator inserter, const color& color) {
    for (auto it = geometryBegin; it != geometryEnd; ++it) {
        geoqik_uuid_t id = draw(*it, color);
        *inserter++ = id;
    }
}

} // namespace example
