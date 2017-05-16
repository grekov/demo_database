CC=gcc
CFLAGS= -c -Wall -O2

all: client server

list.o: list.c \
	list.h
	$(CC) $(CFLAGS) list.c

queue.o: queue.c \
	queue.h
	$(CC) $(CFLAGS) queue.c

stack.o: stack.c \
	stack.h
	$(CC) $(CFLAGS) stack.c

client_main.o: client_main.c \
	socket_operations.h \
	common.h
	$(CC) $(CFLAGS) client_main.c

server_main.o: server_main.c \
	server.h \
	common.h
	$(CC) $(CFLAGS) server_main.c

server.o: server.c \
	server.h \
	list.h \
	db.h \
	socket_operations.h
	$(CC) $(CFLAGS) server.c

db.o: db.c \
	db.h
	$(CC) $(CFLAGS) db.c

db_node.o: db_node.c \
	db_node.h
	$(CC) $(CFLAGS) db_node.c

db_file.o: db_file.c \
	db_file.h
	$(CC) $(CFLAGS) db_file.c

socket_operations.o: socket_operations.c \
	socket_operations.h
	$(CC) $(CFLAGS) socket_operations.c

avl.o: avl.c \
	avl.h
	$(CC) $(CFLAGS) avl.c



CLIENT_OBJECTS = client_main.o \
		socket_operations.o \
		stack.o

client: $(CLIENT_OBJECTS)
	$(CC) $(CLIENT_OBJECTS) -o $@

SERVER_OBJECTS = server_main.o \
		server.o \
		list.o \
		queue.o \
		db.o \
		db_node.o \
		db_file.o \
		avl.o \
		socket_operations.o \
		stack.o

server: $(SERVER_OBJECTS)
	$(CC) $(SERVER_OBJECTS) -o $@ -pthread

clean:
	rm -rf *.o client server
