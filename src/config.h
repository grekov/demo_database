#ifndef CONFIG_H
#define CONFIG_H

#define DB_SERVER_MAX_CONNECTIONS 100

/**
  * Count of readers thread.
  */
#define DB_SERVER_READERS_COUNT 4

/**
  * Count of writers thread.
  */
#define DB_SERVER_WRITERS_COUNT 2

/**
  * Count of database nodes.
  */
#define DB_SERVER_NODES_COUNT   4

#endif /* CONFIG_H */
