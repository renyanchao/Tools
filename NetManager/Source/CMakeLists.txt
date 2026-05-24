cmake_minimum_required(VERSION 3.11)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(PROTO_SOURCE_DIR ${PROJECT_SOURCE_DIR}/Proto)
set(PROTO_GENERATED_DIR ${CMAKE_BINARY_DIR}/generated/Proto)

file(MAKE_DIRECTORY ${PROTO_GENERATED_DIR})

set(PROTO_FILES
    ${PROTO_SOURCE_DIR}/Chat.proto
    ${PROTO_SOURCE_DIR}/Login.proto
)

set(PROTO_GENERATED_FILES
    ${PROTO_GENERATED_DIR}/Chat.pb.cc
    ${PROTO_GENERATED_DIR}/Chat.pb.h
    ${PROTO_GENERATED_DIR}/Login.pb.cc
    ${PROTO_GENERATED_DIR}/Login.pb.h
)

if (TARGET protobuf::protoc)
    set(PROTOC_COMMAND protobuf::protoc)
elseif (DEFINED Protobuf_PROTOC_EXECUTABLE)
    set(PROTOC_COMMAND ${Protobuf_PROTOC_EXECUTABLE})
else()
    find_program(PROTOC_COMMAND protoc REQUIRED)
endif()

add_custom_command(
    OUTPUT ${PROTO_GENERATED_FILES}
    COMMAND ${PROTOC_COMMAND}
    ARGS
        --proto_path=${PROTO_SOURCE_DIR}
        --cpp_out=${PROTO_GENERATED_DIR}
        ${PROTO_FILES}
    DEPENDS ${PROTO_FILES}
    COMMENT "Generating protobuf C++ files"
    VERBATIM
)

add_library(Proto STATIC ${PROTO_GENERATED_FILES})
target_include_directories(Proto PUBLIC ${PROTO_GENERATED_DIR})
target_link_libraries(Proto PUBLIC protobuf::libprotobuf)
source_group("Generated Files" FILES ${PROTO_GENERATED_FILES})

file(GLOB_RECURSE NETMANAGER_BASE_HEADERS ${CMAKE_CURRENT_LIST_DIR}/Net/*.h)
file(GLOB_RECURSE NETMANAGER_BASE_SOURCES ${CMAKE_CURRENT_LIST_DIR}/Net/*.cpp)

add_library(NetManagerBase STATIC
    ${NETMANAGER_BASE_HEADERS}
    ${NETMANAGER_BASE_SOURCES}
)
target_include_directories(NetManagerBase PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Net)
target_link_libraries(NetManagerBase PUBLIC Proto)

source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/Net PREFIX "NetManagerBase" FILES
    ${NETMANAGER_BASE_HEADERS}
    ${NETMANAGER_BASE_SOURCES}
)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Server)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Client)
