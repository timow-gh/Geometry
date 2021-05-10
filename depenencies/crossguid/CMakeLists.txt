FetchContent_Declare(
        crossguid
        GIT_REPOSITORY  https://github.com/timow-gh/CrossGuid.git
        GIT_TAG main)
set(CrossGuid_INSTALL ON)
set(BUILD_EXAMPLES OFF)
set(CrossGuid_BUILD_TESTS OFF)
FetchContent_MakeAvailable(crossguid)