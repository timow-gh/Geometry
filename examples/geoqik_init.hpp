#pragma once

#include <GeoQik/GeoQik.hpp>
#include <cstdio>
#include <cstdlib>

namespace example {

constexpr float default_line_width = 3.0f;
constexpr float default_point_size = 2.0f * default_line_width;

[[noreturn]] inline void fail_example(const char* operation, int error) {
    std::fprintf(stderr, "%s failed (error %d)\n", operation, error);
    geoqik_cleanup();
    std::exit(EXIT_FAILURE);
}

inline void check_geoqik(geoqik_error_code_t error, const char* operation) {
    if (error != GEOQIK_SUCCESS) fail_example(operation, static_cast<int>(error));
}

inline void init_geoqik() {
    check_geoqik(geoqik_init(), "Initialize geoqik");

    geoqik_error_code_t err = geoqik_set_line_width(default_line_width);
    check_geoqik(err, "Set line width");
    err = geoqik_set_point_size(default_point_size);
    check_geoqik(err, "Set point size");
}

} // namespace example
