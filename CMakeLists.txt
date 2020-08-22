cmake_minimum_required(VERSION 3.0.0)
project(push VERSION 0.1.0)

add_compile_options("-std=c++20")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" )
  add_compile_options("-fcoroutines")
endif()

include_directories(include)
include_directories(conduit/include)

set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(Threads REQUIRED)

add_executable(server server.cpp)
add_executable(client client.cpp)
add_executable(server_reference server_reference.cpp)
add_executable(client_reference client_reference.cpp)

target_link_libraries(server PRIVATE Threads::Threads)
target_link_libraries(client PRIVATE Threads::Threads)
target_link_libraries(server_reference PRIVATE Threads::Threads)
target_link_libraries(client_reference PRIVATE Threads::Threads)