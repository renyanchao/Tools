cmake_minimum_required(VERSION 3.11)


add_executable(client Client.cpp ${SRC_PB_FILES} ${SRC_PB_HEADERS})
target_link_libraries(client PRIVATE ${PROTOBUF_ROOT}/libprotobufd.lib)
	
