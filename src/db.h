#ifndef DB_H
#define DB_H

/**
 * @file db.h
 * @author Sviatoslav
 * @brief Main database.
 *
 * Database consists of several nodes.
 * Each key or value stores in independent DB node.
 *
 * It is provide multiple access for read and write.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct s_message;

/**
 * @brief Initialize databse.
 * Creates node_count pair nodes for key and value.
 * @param node_count Database node count.
 * @return On success, return zero.
 * On error, -1 is returned, and errno is set.
 */
int db_init(uint32_t node_count);

/**
 * @brief Release the database resources.
 */
void db_release(void);

/**
 * @brief Process incoming request.
 * @param msg Incoming request.
 */
void db_process_message(struct s_message *msg);

#ifdef __cplusplus
}
#endif

#endif /* DB_H*/
