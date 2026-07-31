#pragma once

#include "draw.hpp"

#include <Geometry/Segment.hpp>

#include <geoqik/GeoQik.hpp>

#include <linal/vec.hpp>

#include <array>
#include <vector>
#include <iterator>


namespace example {

struct origin {
    linal::float3 position{0.0f, 0.0f, 0.0f};
    float axisLength{1.0f};
    std::array<Geometry::Segment3d, 3> axes = {
        Geometry::Segment3d{{position[0], position[1], position[2]},
                            {position[0] + axisLength, position[1], position[2]}},
        Geometry::Segment3d{{position[0], position[1], position[2]},
                            {position[0], position[1] + axisLength, position[2]}},
        Geometry::Segment3d{{position[0], position[1], position[2]},
                            {position[0], position[1], position[2] + axisLength}}};
    std::array<color, 3> axisColors = {light_red(), light_green(), light_blue()};

    std::array<linal::float3, 3> points = {linal::float3{position[0] + axisLength, position[1], position[2]},
                                           linal::float3{position[0], position[1] + axisLength, position[2]},
                                           linal::float3{position[0], position[1], position[2] + axisLength}};
    std::array<color, 3> pointColors = {light_red(), light_green(), light_blue()};
};

inline void draw(const origin& origin) {
    std::vector<geoqik_uuid_t> axisIds;
    for (std::size_t i = 0; i < origin.axes.size(); ++i) {
        geoqik_uuid_t id = example::draw(origin.axes[i], origin.axisColors[i]);
        axisIds.push_back(id);
    }

    for (std::size_t i = 0; i < origin.points.size(); ++i) {
        geoqik_uuid_t id = example::draw(origin.points[i], origin.pointColors[i]);
        axisIds.push_back(id);
    }
}

inline void draw_default_origin() {
    static origin defaultOrigin = origin{};
    draw(defaultOrigin);
}

} // namespace example