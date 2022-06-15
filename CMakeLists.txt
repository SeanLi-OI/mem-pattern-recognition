project(mpr)
cmake_minimum_required (VERSION 3.0)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(${PROJECT_SOURCE_DIR}/include)
add_subdirectory(${PROJECT_SOURCE_DIR}/external/gflags)

add_compile_options(-Wall -Wextra -pedantic -Werror -Wno-error=maybe-uninitialized -Wno-error=unused-parameter -Wno-error=switch -w -g -O3 -std=c++17 -march=native)

add_executable(mpr
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/trace_list.cpp
    ${PROJECT_SOURCE_DIR}/src/tracereader.cpp
    ${PROJECT_SOURCE_DIR}/src/pc_meta.cpp
    ${PROJECT_SOURCE_DIR}/src/pattern_list.cpp
)
target_link_libraries(mpr gflags::gflags)

add_executable(pattern2line
    ${PROJECT_SOURCE_DIR}/src/utils/pattern2line.cpp
    ${PROJECT_SOURCE_DIR}/src/pc_meta.cpp
)

add_executable(perf-parse
    ${PROJECT_SOURCE_DIR}/src/utils/perf-parse.cpp
)
target_link_libraries(perf-parse gflags::gflags)

add_executable(analyze-other
    ${PROJECT_SOURCE_DIR}/src/utils/analyze-other.cpp
)

add_executable(analyze-gem5-prefetch
    ${PROJECT_SOURCE_DIR}/src/utils/analyze-gem5-prefetch.cpp
    ${PROJECT_SOURCE_DIR}/src/pc_meta.cpp
)