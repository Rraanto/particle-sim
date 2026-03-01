cmake_minimum_required(VERSION 3.20)

project(ShaderCompiler LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-g -ggdb3 -O0 -fno-omit-frame-pointer)

# external dependencies 
# glfw's own CMakeLists.txt does the job 
# target name is glfw
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # avoid building glfw test exes
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

# Glad library
add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad
  PUBLIC 
  external/glad/include 
  external/glad/KHR${GLAD_COMPAT_INCLUDE_DIR}
  PRIVATE
  external/glad/include/glad
)

add_library(camera STATIC src/camera/camera.cpp)
target_include_directories(camera PRIVATE src/camera)

add_library(compiler STATIC
  src/compiler/compiler/compiler.cpp
  src/compiler/preprocessor/preprocessor.cpp
)
target_include_directories(compiler PUBLIC src/compiler)
target_link_libraries(compiler PRIVATE glad)

# Main executable 
add_executable(main src/main.cpp)
target_link_libraries(main 
  PRIVATE 
  camera
  glfw 
  glad
  compiler
)
target_compile_definitions(main PRIVATE SHADERS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders") # expose to executables to load shaders at runtime

# strict warnings on project code only 
set(PROJECT_PEDANTIC_WARNINGS
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wextra>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wpedantic>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Werror>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wshadow>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wconversion>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wno-float-conversion>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wsign-conversion>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wformat=2>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wundef>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wcast-qual>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wno-old-style-cast>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wnon-virtual-dtor>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wnull-dereference>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wno-double-promotion>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Woverloaded-virtual>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wimplicit-fallthrough>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wno-unused-parameter>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wmisleading-indentation>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wduplicated-cond>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wduplicated-branches>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wlogical-op>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wuseless-cast>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wzero-as-null-pointer-constant>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wextra-semi>
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
  $<$<CXX_COMPILER_ID:MSVC>:/WX>
  $<$<CXX_COMPILER_ID:MSVC>:/permissive->
)

target_compile_options(camera PRIVATE ${PROJECT_PEDANTIC_WARNINGS})
target_compile_options(compiler PRIVATE ${PROJECT_PEDANTIC_WARNINGS})
target_compile_options(main PRIVATE ${PROJECT_PEDANTIC_WARNINGS})
