cmake_minimum_required(VERSION 3.11)
project(NETMANAGER)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Protobuf CONFIG REQUIRED)

if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    foreach(flag_var
        CMAKE_C_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS_RELWITHDEBINFO
        CMAKE_C_FLAGS_MINSIZEREL
        CMAKE_CXX_FLAGS_MINSIZEREL)
        if (DEFINED ${flag_var})
            string(REPLACE "/MTd" "/MDd" ${flag_var} "${${flag_var}}")
            string(REPLACE "/MT" "/MD" ${flag_var} "${${flag_var}}")
        endif()
    endforeach()
endif()

add_subdirectory(Source)
