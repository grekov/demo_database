cmake_minimum_required(VERSION 3.0)
set(SRC_DIR ../../src)

include_directories(${CMAKE_CURRENT_BINARY_DIR}
                        ${CMAKE_CURRENT_SOURCE_DIR}
                        ${SRC_DIR})
add_definitions(-Wall -O2)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../)


set(SERVER_SRCS
        ${SRC_DIR}/avl.c
        ${SRC_DIR}/list.c
        ${SRC_DIR}/queue.c
        ${SRC_DIR}/stack.c
        ${SRC_DIR}/db_file.c
        ${SRC_DIR}/db_node.c
        ${SRC_DIR}/db.c
        ${SRC_DIR}/socket_operations.c)

set(TEST_SRCS
        ../list_test.cpp
        ../queue_test.cpp
        ../stack_test.cpp
        ../db_file_test.cpp
        ../db_node_test.cpp
        ../db_test.cpp
        ../socket_operations_test.cpp)

foreach(testsourcefile ${TEST_SRCS})
    string(REPLACE ".cpp" "" testname ${testsourcefile})
    string(REPLACE "../" "" testname ${testname})
    add_executable(${testname} ${testsourcefile} ${SERVER_SRCS})
    target_link_libraries(${testname} boost_unit_test_framework pthread)
endforeach(testsourcefile ${TEST_SRCS})
