CPMAddPackage(
        NAME LinAl
        GITHUB_REPOSITORY timow-gh/LinAl
        GIT_TAG v0.2.0-dev
        GIT_SHALLOW TRUE
        OPTIONS
        "LinAl_INSTALL ${${PROJECT_NAME}_INSTALL}"
)
