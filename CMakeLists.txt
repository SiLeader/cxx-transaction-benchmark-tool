cmake_minimum_required(VERSION 3.16)
project(tx-bench)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

add_subdirectory(third_party/yaml-cpp)

add_executable(${PROJECT_NAME} src/main.cpp src/configuration.hpp src/configuration.cc src/template_engine.hpp src/template_engine.cc src/executor.hpp src/statistics.hpp database/stdout.hpp)
target_link_libraries(${PROJECT_NAME} yaml-cpp pthread)
target_include_directories(${PROJECT_NAME} PRIVATE third_party/yaml-cpp/include)
