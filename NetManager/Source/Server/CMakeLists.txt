cmake_minimum_required(VERSION 3.11)

add_executable(server Server.cpp)
target_link_libraries(server PRIVATE NetManagerBase)
