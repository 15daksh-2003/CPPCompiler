# CMakeLists.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.

cmake_minimum_required(VERSION 3.8)
project(CPPCompilerSolution)

# Set the C++ standard globally (optional if set per target)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Testing Setup ---
# Add a project-wide option to control building of tests
option(BUILD_TESTING "Build project tests" ON)

if(BUILD_TESTING)
    # Enable CTest support
    enable_testing()

    # Include the FetchContent module to manage external dependencies
    include(FetchContent)

    # Declare the Google Test dependency
    # We specify the Git repository and a specific tag for reproducibility.
    # Check the googletest repo for the latest stable release tag.
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.16.0  # Example: Use a specific release tag!
        # Or use main branch (less stable): GIT_TAG origin/main
    )

    # Configure googletest: Prevent GTest from installing itself globally
    # and disable building gmock tests (usually not needed just to use gtest)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE) # Needed on Windows sometimes
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE) # Optional: disable gmock if only using gtest

    # Make Google Test available
    # This command triggers the download (if needed) and adds GTest's targets
    # (like gtest, gtest_main) to the build.
    FetchContent_MakeAvailable(googletest)

    # Note: FetchContent downloads sources typically into your build directory
    # (e.g., out/build/x64-debug/_deps/googletest-src), which should already
    # be covered by your .gitignore rule for /out/.
endif()

# Add subdirectories for modules
add_subdirectory(Lexer)
add_subdirectory(CPPCompiler)

# Optionally, set common compile options or flags here
# but prefer to set properties per target in subdirectories
