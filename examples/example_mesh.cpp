#include "draw.hpp"
#include "geoqik_init.hpp"
#include "grid.hpp"
#include "origin.hpp"

#include <Geometry/Mesh/MakeTriangleMesh.hpp>

namespace {
template <typename Shape>
void draw_primitive(const Shape& shape, const example::color& color, const char* name) {
    const auto result = Geometry::make_triangle_mesh(shape);
    if (!result)
        example::fail_example(name, static_cast<int>(result.error));
    example::draw(result.mesh, color);
}
} // namespace

int main() {
    example::init_geoqik();
    example::draw_default_origin();
    example::draw_default_grid();

    // Circular primitives use the factories' default of 32 segments.
    draw_primitive(Geometry::Cone<double>{{-3, -3, 0}, {-3, -3, 3}, 1.25}, example::orange(), "Create cone");
    draw_primitive(Geometry::Cylinder<double>{Geometry::Segment3d{{3, -3, 0}, {3, -3, 3}}, 1.25},
                   example::blue(),
                   "Create cylinder");
    const std::array<linal::double3, 3> sides{{{2, 1, 0}, {-1, 2, 0}, {0, 0, 3}}};
    draw_primitive(Geometry::Cuboid<double>{{-3.5, 1.5, 0}, sides}, example::green(), "Create cuboid");
    draw_primitive(Geometry::AABB3d{{2, 2, 0}, {4, 4, 3}}, example::magenta(), "Create AABB");

    example::check_geoqik(geoqik_draw(), "Open visualization");
    example::check_geoqik(geoqik_wait_for_exit_and_cleanup(), "Close visualization");
    return EXIT_SUCCESS;
}
