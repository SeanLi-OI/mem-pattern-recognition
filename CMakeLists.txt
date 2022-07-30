project(mpr)
cmake_minimum_required(VERSION 3.0)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEFINED_IN_CMAKELISTS")
include_directories(${PROJECT_SOURCE_DIR}/include)

add_subdirectory(${PROJECT_SOURCE_DIR}/external/gflags)

set(glog_DIR ${PROJECT_SOURCE_DIR}/external/glog/build)
find_package(glog 0.6.0 REQUIRED)
find_package(OpenMP REQUIRED)

add_compile_options(-Wall -Wextra -pedantic -Werror -Wno-error=maybe-uninitialized -Wno-error=unused-parameter -Wno-error=switch -w -g -O3 -std=c++17 -march=native -pthread -lpthread -lglog)

add_executable(mpr
    ${PROJECT_SOURCE_DIR}/src/trace_list.cpp
    ${PROJECT_SOURCE_DIR}/src/tracereader.cpp
    ${PROJECT_SOURCE_DIR}/src/pattern_list.cpp
    ${PROJECT_SOURCE_DIR}/src/pc_meta.cpp
    ${PROJECT_SOURCE_DIR}/src/utils/macro.cpp
    ${PROJECT_SOURCE_DIR}/src/main.cpp
)
target_link_libraries(mpr glog::glog)
target_link_libraries(mpr gflags::gflags)

add_executable(pattern2line
    ${PROJECT_SOURCE_DIR}/src/utils/pattern2line.cpp
    ${PROJECT_SOURCE_DIR}/src/pc_meta.cpp
)
target_link_libraries(pattern2line glog::glog)

add_executable(perf-parse
    ${PROJECT_SOURCE_DIR}/src/utils/perf-parse.cpp
)
target_link_libraries(perf-parse gflags::gflags)
target_link_libraries(perf-parse glog::glog)

add_executable(analyze-other
    ${PROJECT_SOURCE_DIR}/src/utils/analyze-other.cpp
)

add_executable(analyze-gem5-prefetch
    ${PROJECT_SOURCE_DIR}/src/utils/analyze-gem5-prefetch.cpp
    ${PROJECT_SOURCE_DIR}/src/pc_meta.cpp
)
target_link_libraries(analyze-gem5-prefetch glog::glog)

add_executable(analyze-gem5-missinfo
    ${PROJECT_SOURCE_DIR}/src/utils/analyze-gem5-missinfo.cpp
    ${PROJECT_SOURCE_DIR}/src/pc_meta.cpp
)
target_link_libraries(analyze-gem5-missinfo glog::glog)
target_link_libraries(analyze-gem5-missinfo gflags::gflags)

add_executable(merge-result
    ${PROJECT_SOURCE_DIR}/src/utils/merge-result.cpp
    ${PROJECT_SOURCE_DIR}/src/pc_meta.cpp
    ${PROJECT_SOURCE_DIR}/src/trace_list.cpp
    ${PROJECT_SOURCE_DIR}/src/pattern_list.cpp
    ${PROJECT_SOURCE_DIR}/src/utils/macro.cpp
    ${PROJECT_SOURCE_DIR}/src/tracereader.cpp
)
target_link_libraries(merge-result glog::glog)
target_link_libraries(merge-result gflags::gflags)
target_link_libraries(merge-result OpenMP::OpenMP_CXX)