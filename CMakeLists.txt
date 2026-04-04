cmake_minimum_required(VERSION 3.20)

project(ShaderCompiler LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build type configuration
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Debug options (only applied in Debug builds)
add_compile_options($<$<CONFIG:Debug>:-g>)
add_compile_options($<$<CONFIG:Debug>:-ggdb3>)
add_compile_options($<$<CONFIG:Debug>:-O0>)
add_compile_options($<$<CONFIG:Debug>:-fno-omit-frame-pointer>)

# OpenMP (parallelization)
find_package(OpenMP)

# ImGui library
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)
add_library(imgui STATIC
  ${IMGUI_DIR}/imgui.cpp
  ${IMGUI_DIR}/imgui_draw.cpp
  ${IMGUI_DIR}/imgui_tables.cpp
  ${IMGUI_DIR}/imgui_widgets.cpp
  ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
  ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
target_link_libraries(imgui PUBLIC glfw glad)

# Release optimization flags
set(RELEASE_OPTIMIZATION_FLAGS
  -O3
  -march=native
  -ffast-math
  -flto
)

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
target_include_directories(camera PUBLIC src/camera)

add_library(cliutils STATIC src/cliutils/cliutils.cpp)
target_include_directories(cliutils PUBLIC src/cliutils)

add_library(compiler STATIC
  src/compiler/compiler/compiler.cpp
  src/compiler/preprocessor/preprocessor.cpp
)
target_include_directories(compiler PUBLIC src/compiler)
target_link_libraries(compiler PRIVATE glad)

add_library(render STATIC src/render/render.cpp)
target_include_directories(render PUBLIC src/render src/camera src/compiler/compiler src/simulation)
target_link_libraries(render PRIVATE glfw glad compiler camera)

add_library(simulation STATIC src/simulation/simulation.cpp src/simulation/spatial_grid.cpp)
target_include_directories(simulation PUBLIC src/simulation)
target_compile_options(simulation PRIVATE $<$<CONFIG:Release>:${RELEASE_OPTIMIZATION_FLAGS}>)
target_link_options(simulation PRIVATE $<$<CONFIG:Release>:${RELEASE_OPTIMIZATION_FLAGS}>)
if(OpenMP_CXX_FOUND)
  target_link_libraries(simulation PUBLIC OpenMP::OpenMP_CXX)
endif()

# GUIManager library
add_library(gui STATIC src/gui/gui_manager.cpp)
target_include_directories(gui PUBLIC src/gui src/simulation src/render external/imgui)
target_link_libraries(gui PUBLIC imgui simulation render)

add_library(app STATIC src/app/app.cpp)
target_include_directories(app PUBLIC src/app src/camera src/cliutils src/gui src/render src/simulation)
target_link_libraries(app PRIVATE camera cliutils glfw glad render simulation gui)
target_compile_definitions(app PRIVATE SHADERS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders")
target_compile_options(app PRIVATE $<$<CONFIG:Release>:${RELEASE_OPTIMIZATION_FLAGS}>)
target_link_options(app PRIVATE $<$<CONFIG:Release>:${RELEASE_OPTIMIZATION_FLAGS}>)

# Main executable
add_executable(main src/main.cpp)
target_link_libraries(main
  PRIVATE
  app
  cliutils
)
if(OpenMP_CXX_FOUND)
  target_link_libraries(main PRIVATE OpenMP::OpenMP_CXX)
endif()

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
target_compile_options(cliutils PRIVATE ${PROJECT_PEDANTIC_WARNINGS})
target_compile_options(compiler PRIVATE ${PROJECT_PEDANTIC_WARNINGS})
target_compile_options(render PRIVATE ${PROJECT_PEDANTIC_WARNINGS})
target_compile_options(simulation PRIVATE ${PROJECT_PEDANTIC_WARNINGS})
target_compile_options(app PRIVATE ${PROJECT_PEDANTIC_WARNINGS})
target_compile_options(main PRIVATE ${PROJECT_PEDANTIC_WARNINGS})
