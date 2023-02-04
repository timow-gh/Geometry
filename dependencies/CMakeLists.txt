include(FetchContent)

add_subdirectory(core)
add_subdirectory(linal)
add_subdirectory(crossguid)

if (${PROJECT_NAME}_TESTS)
    add_subdirectory(googletest)
endif ()