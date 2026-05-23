cmake_minimum_required(VERSION 3.11)


add_executable(client Client.cpp)
target_link_libraries(client PRIVATE ${PROTOBUF_ROOT}/libprotobufd.lib)
	
