#ifndef SERVER_H
#define SERVER_H

/**
 * @file server.h
 * @author Sviatoslav
 * @brief DB server.
 *
 * For config detail see config.h.
 */


#include <stdint.h>

/**
 * @brief Initialize databse server.
 * @param max_server_connections Max listen connections.
 * @param db_nodes_count Max pair of DB nodes <key node, value node>.
 * @param readers_count Max thread count for execute read command (GET, LIST).
 * @param writers_count MAX thread count for execute write command (PUT, ERASE).
 * @return On success, return 0, otherwise -1 is returned.
 */
int server_init(uint32_t max_server_connections,
                uint32_t db_nodes_count,
                uint32_t readers_count,
                uint32_t writers_count);

/**
 * @brief Release all server resources.
 */
void server_release(void);

/**
 * @brief Run server loop.
 * @return Return -1 on fail.
 */
int server_run(void);

/**
 * @brief Stop server.
 */
void server_stop(void);

#endif // SERVER_H
