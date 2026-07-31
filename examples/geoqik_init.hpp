#pragma once

#include <geoqik/GeoQik.hpp>

namespace example {

constexpr float default_line_width = 3.0f;
constexpr float default_point_size = 2.0f * default_line_width;

void init_geoqik() {
    geoqik_init();

    geoqik_error_code_t err = geoqik_set_line_width(default_line_width);
    assert(err == GEOQIK_SUCCESS);
    err = geoqik_set_point_size(default_point_size);
    assert(err == GEOQIK_SUCCESS);
}

} // namespace example