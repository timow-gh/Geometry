CPMAddPackage(
        NAME LinAl
        GITHUB_REPOSITORY timow-gh/LinAl
        GIT_TAG only_blaze
        GIT_SHALLOW TRUE
        OPTIONS
        "LinAl_INSTALL ${${PROJECT_NAME}_INSTALL}"
)
