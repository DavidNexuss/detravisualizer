cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS_DEBUG "-fsanitize=address -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -ftree-vectorize -march=native -fopenmp")
set(CMAKE_COMPILE_WARNING_AS_ERROR OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(DOXYGEN_GENERATE_TREEVIEW YES)

project(detravisualizer)

find_package(OpenMP)
find_package(ZLIB REQUIRED)

add_subdirectory(lib/detragraphs)
add_subdirectory(lib/glfw)
include_directories(lib)
include_directories(lib/detragraphs/lib/detrarandom)
include_directories(src/external)
include_directories(src)

file(GLOB_RECURSE SRC 
  src/*.cpp src/*.c
)

include_directories(lib/octree)
add_executable(detravisualizer ${SRC})
target_link_libraries(detravisualizer detragraphs glfw ZLIB::ZLIB)
