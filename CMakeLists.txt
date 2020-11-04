cmake_minimum_required(VERSION 3.16)
project(tx-bench)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wold-style-cast")

add_subdirectory(third_party/yaml-cpp)

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/database/notrack/CMakeLists.txt)
    add_subdirectory(database/notrack)
endif ()

add_executable(${PROJECT_NAME}
        src/main.cpp
        include/configuration.hpp
        src/configuration.cc
        src/template_engine.hpp
        src/template_engine.cc
        src/executor.hpp
        include/statistics.hpp
        database/stdout.hpp
        database/mysql.hpp
        include/properties.hpp
        include/database_creator.hpp
        include/database.hpp
        database/prestgresql.hpp)

target_link_libraries(${PROJECT_NAME} yaml-cpp pthread mysqlclient pq)
target_include_directories(${PROJECT_NAME} PRIVATE third_party/yaml-cpp/include)
target_include_directories(${PROJECT_NAME} PRIVATE third_party/argparse)
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_include_directories(${PROJECT_NAME} PRIVATE database)
if (TARGET notrack-database)
    target_link_libraries(${PROJECT_NAME} notrack-database)
endif ()
