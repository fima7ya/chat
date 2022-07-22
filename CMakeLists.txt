cmake_minimum_required(VERSION 3.14)
project(search_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_DEBUG_POSTFIX d)


add_executable(chatserver proto.cpp server.cpp)
add_executable(chatclient proto.cpp client.cpp)

set_target_properties(chatserver PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
set_target_properties(chatclient PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})



