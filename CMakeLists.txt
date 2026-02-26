cmake_minimum_required(VERSION 3.16...4.2.3)

project(codex-sockettapd
        VERSION 0.1
        LANGUAGES CXX
        DESCRIPTION "Daemon that creates and listens on a UNIX socket for messages from the Codex socket-tap.")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Begin of gitache configuration.
include(FetchContent)

set(GITACHE_PACKAGES libcwd_r)  # List of requested packages (example).
FetchContent_Declare(
  gitache
  GIT_REPOSITORY "https://github.com/CarloWood/gitache.git"
  GIT_TAG        "stable"
)
FetchContent_MakeAvailable(gitache)
# End of gitache configuration.

# This project uses aicxx modules.
include(cwm4/cmake/AICxxProject NO_POLICY_SCOPE)

# Exit if someone tries to contaminates the source directory with an in-source build.
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "Please do out-of-source builds.\nCleanup: \"rm -rf CMake CMakeCache.txt CMakeFiles/\"")
endif()

include(AICxxSubmodules)

configure_file(version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h @ONLY)

add_subdirectory(enchantum)
add_subdirectory(src)
