# About
demo_databse is a simple database server (like a key-value storage) for code demonstration purpose: multithread application written in the C-language for Linux OS. Client-server interaction occurs over Unix-socket.

# Common task
Server performs operations:
 - put <key, value> pair
 - get value by key
 - show list of all values
 - erase key

# Restrictions
 - Key and value length not limited.
 - Values must be deduplicated. If the same value already exsists, then only key must be added to database.
 This way, two different keys must refer to the one value. If one of such keys removing, value still exists with other key,
 - All keys and values must be stored to files on put operation and removed on erase respectively. Lacunes to files must be reused.

# Implementation
Server is a multithread application that try to execute incoming requests in parallel mode.
For this goal uses data distribution between database nodes.
The server consists of several nodes (4 for keys and 4 for values). Number of nodes can be changed.
Sometimes, in the best case, we will have simultaneous write or simultaneous read and write operations.
In common case we have one writer or many readers execution provided by rw_lock semantic.

# Building
Change dir to src and call make.

# Running
```sh
$ ./server
```
or
```sh
$ ./server &
$ killall server
```

### Client usage example from command line:
```sh
 ./client put key value
 ./client get key
 value
 ./client put key1 another_value
 ./client list
 another_value
 value
 ./client erase key
 ./client erase key1
 ./client list
  ```

### Scripts:
Go to scripts directory and run simultaneously from different consoles _client_w.sh_ and _client_r.sh_ scripts.
