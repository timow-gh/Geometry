include(FetchContent)

add_subdirectory(linal)

if (${PROJECT_NAME}_TESTS)
    add_subdirectory(googletest)
endif ()

if (${PROJECT_NAME}_EXAMPLES)
    add_subdirectory(geoqik)
endif ()