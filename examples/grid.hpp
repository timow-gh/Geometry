#pragma once

#include "draw.hpp"

#include <Geometry/Segment.hpp>

#include <linal/vec.hpp>

namespace example {

struct grid {
    linal::float3 origin = {0.0f, 0.0f, 0.0f};
    linal::float3 xAxis = {1.0f, 0.0f, 0.0f};
    linal::float3 yAxis = {0.0f, 1.0f, 0.0f};

    float xLength = 10.0f;
    float yLength = 10.0f;
    float xDivisions = 10;
    float yDivisions = 10;
};

inline void draw(const grid& grid, const color& color) {
    for (int i = 0; i <= grid.xDivisions; ++i) {
        float step = static_cast<float>(i) / grid.xDivisions;
        linal::float3 start = grid.origin + step * grid.xLength * grid.xAxis;
        linal::float3 end = start + grid.yLength * grid.yAxis;
        example::draw(Geometry::Segment3d{start, end}, color);
    }

    for (int i = 0; i <= grid.yDivisions; ++i) {
        float step = static_cast<float>(i) / grid.yDivisions;
        linal::float3 start = grid.origin + step * grid.yLength * grid.yAxis;
        linal::float3 end = start + grid.xLength * grid.xAxis;
        example::draw(Geometry::Segment3d{start, end}, color);
    }
}

inline void draw_default_grid() {
    static grid defaultGrid = grid{.origin = {-10.0f, -10.0f, 0.0f},
                                   .xAxis = {1.0f, 0.0f, 0.0f},
                                   .yAxis = {0.0f, 1.0f, 0.0f},
                                   .xLength = 20.0f,
                                   .yLength = 20.0f,
                                   .xDivisions = 10,
                                   .yDivisions = 10};
    draw(defaultGrid, example::grey());
}

} // namespace example