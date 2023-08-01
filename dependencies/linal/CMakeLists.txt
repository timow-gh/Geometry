CPMAddPackage(
        NAME linal
        GITHUB_REPOSITORY "timow-gh/LinAl"
        GIT_TAG main
        GIT_SHALLOW TRUE
        OPTIONS
        "LinAl_INSTALL ${${PROJECT_NAME}_INSTALL}"
)
