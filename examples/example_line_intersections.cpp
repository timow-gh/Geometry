#include "draw.hpp"
#include "geoqik_init.hpp"
#include "origin.hpp"
#include "grid.hpp"

#include <Geometry/Intersect/IntersectSegment.hpp>
#include <Geometry/Segment.hpp>

#include <geoqik/GeoQik.hpp>

int main() {
    example::init_geoqik();
    example::draw_default_origin();
    example::draw_default_grid();
    geoqik_draw();

    Geometry::Segment3d seg1{{0.0, 0.0, 0.0}, {2.0, 2.0, 2.0}};
    linal::double3 seg2Source{0.0, 1.0, 1.0};
    linal::double3 seg2Target{1.0, 1.0, 1.0};
    Geometry::Segment3d seg2{seg2Source, seg2Source + (seg2Target - seg2Source) * 1.5};
    example::draw(seg1, example::green());
    example::draw(seg2, example::green());

    std::optional<linal::vec3<double>> intersection = Geometry::intersect(seg1, seg2);
    if (intersection.has_value()) {
        geoqik_set_point_color(1.0f, 0.0f, 0.0f, 1.0f); // Red color for intersection point
        linal::vec3<double>& intersectionPoint = intersection.value();
        geoqik_add_point(intersectionPoint[0], intersectionPoint[1], intersectionPoint[2]);
    }

    geoqik_wait_for_exit_and_cleanup();

    return 0;
}
