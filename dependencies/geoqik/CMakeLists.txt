CPMAddPackage(
        NAME geoqik
        GITHUB_REPOSITORY "timow-gh/GeoQik"
        GIT_TAG main
        GIT_SHALLOW TRUE
        geoqik_USE_WARNINGS "OFF"
        geoqik_WARNINGS_AS_ERRORS "OFF"
        geoqik_STATIC_ANALYSIS "OFF"
        geoqik_USE_CPPCHECK "OFF"
        geoqik_TESTS "OFF"
        geoqik_EXAMPLES "OFF"
        geoqik_INSTALL "OFF"
        geoqik_PACK "OFF"
        geoqik_BUILD_DOCUMENTATION "OFF"
)
