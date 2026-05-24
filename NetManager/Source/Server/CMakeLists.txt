cmake_minimum_required(VERSION 3.11)

add_executable(Server Server.cpp)
target_link_libraries(Server PRIVATE NetManagerBase)
