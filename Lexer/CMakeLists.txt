# Lexer/CMakeLists.txt

# Source files
set(SOURCES
    src/Lexer.cpp
    # Add other source files as needed
)

# Create the Lexer library
add_library(Lexer STATIC ${SOURCES})

# Set C++ standard for this target
set_target_properties(Lexer PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED YES)

# Set compiler options
target_compile_options(Lexer PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Wall -Wextra -Werror>
)

# Include directories for Lexer
target_include_directories(Lexer
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Testing
# option(BUILD_TESTS "Build the unit tests" ON) # REMOVE THIS LINE

# Use the project-wide BUILD_TESTING option defined in the root CMakeLists.txt
if(BUILD_TESTING) # UPDATE THIS LINE
    # Add test source file
    add_executable(LexerTest
        tests/LexerTest.cpp
    )

    # Link test executable against Google Test and the Lexer library
    # This remains the same - FetchContent makes these targets available
    target_link_libraries(LexerTest PRIVATE
        Lexer
        gtest      # Provided by FetchContent_MakeAvailable(googletest)
        gtest_main # Provided by FetchContent_MakeAvailable(googletest)
    )

    # Include directories for tests
    target_include_directories(LexerTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
        # Note: FetchContent automatically handles include directories for gtest
    )

    # Add the test to CTest
    # Consider using gtest_discover_tests for better integration later
    include(GoogleTest) # Include this to potentially use gtest_discover_tests
    # add_test(NAME LexerUnitTests COMMAND LexerTest) # Keep this OR...
    gtest_discover_tests(LexerTest) # Use this for automatic test discovery

endif()


# If Lexer depends on external libraries, link them here
# target_link_libraries(Lexer PUBLIC OtherLib)
