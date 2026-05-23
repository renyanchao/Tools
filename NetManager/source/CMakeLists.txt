cmake_minimum_required(VERSION 3.11)

set(PROTOBUF_ROOT ${PROJECT_SOURCE_DIR}/Tool)

include_directories(
	${PROJECT_SOURCE_DIR}/Source/Net
	${PROJECT_SOURCE_DIR}/Packets
	${PROTOBUF_ROOT}/include
)
file(GLOB_RECURSE SRC_H_FILES ${CMAKE_CURRENT_LIST_DIR}/Net/*.h)
file(GLOB_RECURSE SRC_CPP_FILES ${CMAKE_CURRENT_LIST_DIR}/Net/*.cpp ${CMAKE_CURRENT_LIST_DIR}/Server/*.cpp)
file(GLOB_RECURSE SRC_PB_FILES ${PROJECT_SOURCE_DIR}/Packets/*.pb.cc)
file(GLOB_RECURSE SRC_PB_HEADERS ${PROJECT_SOURCE_DIR}/Packets/*.pb.h)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Server)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Client)
	
