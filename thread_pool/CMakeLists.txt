cmake_minimum_required(VERSION 3.7.0)
project(thread_pool VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 11)
include(CTest)
enable_testing()

add_executable(thread_pool main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
