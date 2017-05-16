cmake_minimum_required(VERSION 3.0)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
add_definitions(-Wall -O2)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../)

set(COMMON_HDRS
        ../common.h
        ../socket_operations.h)

set(COMMON_SRCS
        ../socket_operations.c)

set(CLIENT_SRCS
        ../client_main.c)

set(SERVER_HDRS
        ../avl.h
        ../list.h
        ../queue.h
        ../stack.h
        ../db_file.h
        ../db_node.h
        ../db.h
        ../server.h)

set(SERVER_SRCS
        ../avl.c
        ../list.c
        ../queue.c
        ../stack.c
        ../db_file.c
        ../db_node.c
        ../db.c
        ../server.c
        ../server_main.c)

add_executable(server ${SERVER_SRCS} ${COMMON_SRCS})
target_link_libraries(server pthread)

add_executable(client ${CLIENT_SRCS} ${COMMON_SRCS})
