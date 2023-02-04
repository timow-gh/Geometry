CPMAddPackage(
        NAME CrossGuid
        GITHUB_REPOSITORY "timow-gh/CrossGuid"
        GIT_TAG main
        GIT_SHALLOW TRUE
        OPTIONS
        "CrossGuid_INSTALL ${${PROJECT_NAME}_INSTALL}"
)
