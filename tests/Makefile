CC = g++
CFLAGS = -c -Wall -O2
CFLAGS += -I../src

# Uncomment to make static test build
#MAKE_STATIC=DB_TEST_STATIC

ifeq ($(MAKE_STATIC),DB_TEST_STATIC)
    CFLAGS += -DDB_TEST_STATIC
else
    LIBS += -lboost_unit_test_framework
endif

SRC_DIR = ../src

all: stack_test \
	queue_test \
	list_test \
	db_file_test \
	db_node_test \
	socket_operations_test \
	db_test

stack.o: $(SRC_DIR)/stack.c \
	$(SRC_DIR)/stack.h
	$(CC) $(CFLAGS) $^

stack_test.o: stack_test.cpp
	$(CC) $(CFLAGS) $^

stack_test: stack.o stack_test.o
	$(CC) $^ $(LIBS) -o $@

queue.o: $(SRC_DIR)/queue.c \
	$(SRC_DIR)/queue.h
	$(CC) $(CFLAGS) $^

queue_test.o: queue_test.cpp
	$(CC) $(CFLAGS) $^

queue_test: queue.o queue_test.o
	$(CC) $^ $(LIBS) -o $@

list.o: $(SRC_DIR)/list.c \
	$(SRC_DIR)/list.h
	$(CC) $(CFLAGS) $^

list_test.o: list_test.cpp
	$(CC) $(CFLAGS) $^

list_test: list.o list_test.o
	$(CC) $^ $(LIBS) -o $@

avl.o: $(SRC_DIR)/avl.c \
	$(SRC_DIR)/avl.h
	$(CC) $(CFLAGS) $^

db_file.o: $(SRC_DIR)/db_file.c \
	$(SRC_DIR)/db_file.h
	$(CC) $(CFLAGS) $^

db_file_test.o: db_file_test.cpp
	$(CC) $(CFLAGS) $^

db_file_test: db_file.o db_file_test.o avl.o list.o
	$(CC) $^ $(LIBS) -o $@

db_node.o: $(SRC_DIR)/db_node.c \
	$(SRC_DIR)/db_node.h
	$(CC) $(CFLAGS) $^

db_node_test.o: db_node_test.cpp
	$(CC) $(CFLAGS) $^

db_node_test: db_node.o db_file.o db_node_test.o avl.o list.o
	$(CC) $^ $(LIBS) -lpthread -o $@

socket_operations.o: $(SRC_DIR)/socket_operations.c \
	$(SRC_DIR)/socket_operations.h
	$(CC) $(CFLAGS) $^

socket_operations_test.o: socket_operations_test.cpp
	$(CC) $(CFLAGS) $^

socket_operations_test: socket_operations.o socket_operations_test.o
	$(CC) $^ $(LIBS) -lpthread -o $@

db.o: $(SRC_DIR)/db.c \
	$(SRC_DIR)/db.h
	$(CC) $(CFLAGS) $^

db_test.o: db_test.cpp
	$(CC) $(CFLAGS) $^

db_test: db.o db_node.o db_file.o db_test.o avl.o list.o socket_operations.o
	$(CC) $^ $(LIBS) -lpthread -o $@


clean:
	rm -f *.o *_test ../src/*.h.gch
