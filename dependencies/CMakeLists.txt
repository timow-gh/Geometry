include(FetchContent)

add_subdirectory(core)
add_subdirectory(linal)
add_subdirectory(crossguid)

if (Geometry_BUILD_TESTS)
    add_subdirectory(googletest)
endif ()