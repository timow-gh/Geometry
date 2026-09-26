#pragma once

#include "geoqik_init.hpp"

#include <Geometry/Mesh/MeshBuffers.hpp>
#include <Geometry/Segment.hpp>

#include <GeoQik/GeoQik.hpp>

#include <linal/vec.hpp>

#include <array>

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

template <std::floating_point T, typename TIndex>
inline geoqik_uuid_t draw(const Geometry::TriangleHalfedgeMesh<T, 3, TIndex>& mesh,
                          const color& surfaceColor,
                          float segmentLineWidth = 5.0f) {
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
    options.showSegments = 1;
    options.segmentLineWidth = segmentLineWidth;
    options.showVertices = 0;
    options.vertexPointSize = 2.0f;
    const auto result = geoqik_add_mesh_opts(buffers.positions.data(), buffers.vertex_count(),
                                             buffers.triangles.data(), buffers.triangles.size() / 3, &options);
    check_geoqik(result.err, "Draw mesh");
    return result.geometryId;
}

template <typename TGeomIter, typename TOutputIterator>
inline void draw(TGeomIter geometryBegin, TGeomIter geometryEnd, TOutputIterator inserter, const color& color) {
    for (auto it = geometryBegin; it != geometryEnd; ++it) {
        geoqik_uuid_t id = draw(*it, color);
        *inserter++ = id;
    }
}

} // namespace example
