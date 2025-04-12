# CPPCompiler/CMakeLists.txt

# Add main executable
add_executable(CPPCompiler  "include/CPPCompiler.h" "src/main.cpp")

# Set C++ standard for this target
set_target_properties(CPPCompiler PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED YES)

# Set compiler options
target_compile_options(CPPCompiler PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Wall -Wextra -Werror>
)

# Include directories for CPPCompiler
target_include_directories(CPPCompiler
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link the Lexer library
target_link_libraries(CPPCompiler
    PRIVATE
        Lexer
)
