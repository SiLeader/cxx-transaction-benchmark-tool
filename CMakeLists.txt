cmake_minimum_required(VERSION 3.16)
project(tx-bench)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

add_subdirectory(third_party/yaml-cpp)

add_executable(${PROJECT_NAME} main.cpp configuration.hpp configuration.cc template_engine.hpp template_engine.cc executor.hpp statistics.hpp database/stdout.hpp)
target_link_libraries(${PROJECT_NAME} yaml-cpp pthread)
target_include_directories(${PROJECT_NAME} PRIVATE third_party/yaml-cpp/include)
