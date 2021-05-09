FetchContent_Declare(
        crossguid
        GIT_REPOSITORY  git@github.com:timow-gh/CrossGuid.git
        GIT_TAG main)
set(CrossGuid_INSTALL ON)
FetchContent_MakeAvailable(crossguid)