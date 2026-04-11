cmake_minimum_required(VERSION 3.11)


include_directories(
	${PROJECT_SOURCE_DIR}/Source/Net
)
file(GLOB_RECURSE SRC_H_FILES ${CMAKE_CURRENT_LIST_DIR}/Net/*.h)
file(GLOB_RECURSE SRC_CPP_FILES ${CMAKE_CURRENT_LIST_DIR}/Net/*.cpp)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Server)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Client)
	