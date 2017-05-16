#ifndef SOCKET_OPERATIONS_H
#define SOCKET_OPERATIONS_H

/**
 * @socket_operations.h
 * @author Sviatoslav
 * @brief Functions for read and write struct msg to socket.
 *
 */

#include <stdint.h>

#define DB_SOCKET_NAME    "db_socket"

#ifdef __cplusplus
extern "C" {
#endif


struct s_message;

/**
 * socket_read call this function, when msg ready.
 */
typedef void (*f_msg_handler)(struct s_message *msg, void *arg);

/**
 * @brief Reads data from socket and call msg_handler.
 * msg::sd field must be set to correct descriptor.
 * @param msg Message.
 * @param msg_handler Handler.
 * @param handler_arg Handler arg.
 */
void socket_read(struct s_message *msg,
                 f_msg_handler msg_handler,
                 void *handler_arg);

/**
 * @brief Writes data to socket from message.
 * msg::sd field must be set to correct descriptor.
 * Send cmd and key and/or value if they not NULL.
 * @param msg Message.
 * @return On success, the number of bytes written is returned.
 * On error, -1 is returned, and errno is set.
 */
int socket_write(struct s_message *msg);

#ifdef __cplusplus
}
#endif

#endif /* SOCKET_OPERATIONS_H */
