cmake_minimum_required(VERSION 3.11)


add_executable(server ${SRC_H_FILES} ${SRC_CPP_FILES} ${SRC_PB_FILES} ${SRC_PB_HEADERS})
target_link_libraries(server PRIVATE ${PROTOBUF_ROOT}/libprotobufd.lib)
	
