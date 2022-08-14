set(FETCHCONTENT_SOURCE_DIR_LINAL "/home/timo/dev/repos/LinAl//")
FetchContent_Declare(
        linal
        GIT_REPOSITORY https://github.com/timow-gh/LinAl.git
        GIT_TAG main)
FetchContent_MakeAvailable(linal)