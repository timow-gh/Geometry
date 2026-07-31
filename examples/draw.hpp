#pragma once

#include <Geometry/Segment.hpp>

#include <geoqik/GeoQik.hpp>

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
inline color light_red() {
    return color(1.0f, 0.5f, 0.5f, 1.0f);
}
inline color light_green() {
    return color(0.5f, 1.0f, 0.5f, 1.0f);
}
inline color green() {
    return color(0.0f, 1.0f, 0.0f, 1.0f);
}
inline color light_blue() {
    return color(0.5f, 0.5f, 1.0f, 1.0f);
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
    assert(result.err == GEOQIK_SUCCESS);
    return result.geometryId;
}

inline geoqik_uuid_t draw(const linal::float3& source, const color& color) {
    geoqik_add_points_options_t opts{};
    opts.color = color.rgba.data();
    opts.colorCount = color.rgba.size();
    geoqik_result_t result = geoqik_add_point_opts(source[0], source[1], source[2], &opts);
    assert(result.err == GEOQIK_SUCCESS);
    return result.geometryId;
}

template <typename TGeomIter, typename TOutputIterator>
inline void
draw(TGeomIter geometryBegin, TGeomIter geometryEnd, TOutputIterator inserter, const color& color) {
    for (auto it = geometryBegin; it != geometryEnd; ++it) {
        geoqik_uuid_t id = draw(*it, color);
        *inserter++ = id;
    }
}

} // namespace example