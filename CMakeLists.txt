cmake_minimum_required(VERSION 3.3)
project(CST-CN)

set(CMAKE_BUILD_TYPE "Release")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(/home/rcanovas/include)
link_directories(/home/rcanovas/lib)
link_libraries(sdsl divsufsort divsufsort64)

set(SOURCE_FILES test/createCST.cpp include/cst_cn.h)
set(SOURCE_FILES3 test/testOperations.cpp include/cst_cn.h)

add_executable(createCST ${SOURCE_FILES})
add_executable(testOps ${SOURCE_FILES3})
