CPMAddPackage(
        NAME geoqik
        GITHUB_REPOSITORY "timow-gh/GeoQik"
        # Pinned to a commit rather than main: the examples need the line depthLayer, which no
        # release tag contains yet, and CPM's source cache would keep serving a stale main.
        # Commits cannot be fetched shallowly.
        GIT_TAG e61738ff57d5d784f070c670f7fbae6a054538a7
        GIT_SHALLOW FALSE
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
