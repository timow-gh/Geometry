include(FetchContent)

add_subdirectory(linal)

if (${PROJECT_NAME}_TESTS)
    add_subdirectory(googletest)
endif ()