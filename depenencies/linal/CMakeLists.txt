FetchContent_Declare(
        linal
        GIT_REPOSITORY  https://github.com/timow-gh/LinAl.git
        GIT_TAG main)
set(LinAl_INSTALL ON)
FetchContent_MakeAvailable(linal)