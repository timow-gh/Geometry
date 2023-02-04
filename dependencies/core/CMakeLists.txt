CPMAddPackage(
        NAME Core
        GITHUB_REPOSITORY "timow-gh/Core"
        GIT_TAG main
        GIT_SHALLOW TRUE
        OPTIONS
        "Core_INSTALL ${${PROJECT_NAME}_INSTALL}"
)