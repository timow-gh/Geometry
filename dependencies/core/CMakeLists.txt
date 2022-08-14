set(FETCHCONTENT_SOURCE_DIR_CORE "/home/timo/dev/repos/Core/")

FetchContent_Declare(
        core
        GIT_REPOSITORY https://github.com/timow-gh/Core.git
        GIT_TAG main)
set(Core_INSTALL ON)
FetchContent_MakeAvailable(core)