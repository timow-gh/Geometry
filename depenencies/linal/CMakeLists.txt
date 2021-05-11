FetchContent_Declare(
        linal
        GIT_REPOSITORY git@github.com:timow-gh/LinAl.git
        GIT_TAG main)
set(LinAl_INSTALL ON)
FetchContent_MakeAvailable(linal)