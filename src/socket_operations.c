#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include "socket_operations.h"
#include "common.h"


#define SOCKET_READ_SIZE        2048     /**< Socket read size           */

static int cmd_is_valid(struct s_command *cmd)
{
        int cmd_size = sizeof(struct s_command);

        if (cmd->len != cmd_size + cmd->key_size + cmd->val_size) {
                printf("Bad len %d; key_size %d, val_size %d\n",
                       cmd->len, cmd->key_size, cmd->val_size);
                return 0;
        }

        switch (cmd->type) {
        case DB_CMD_PUT:
                if (cmd->key_size == 0 || cmd->val_size == 0)
                        return 0;
                break;
        case DB_CMD_GET:
        case DB_CMD_ERASE:
                if (cmd->key_size == 0)
                        return 0;
                break;
        case DB_CMD_LIST:
        case DB_CMD_RESP:
                return 1;
        default:
                printf("Unkown command %d\n", cmd->type);
                return 0;
        }

        return 1;
}

static int alloc_key_val(struct s_message *msg)
{
        msg->key = NULL;
        msg->val = NULL;

        if (msg->cmd.key_size) {
                msg->key = (uint8_t *)malloc(msg->cmd.key_size);
                if (msg->key == NULL)
                        return -1;
        }

        if (msg->cmd.val_size) {

                msg->val = (uint8_t *)malloc(msg->cmd.val_size);
                if (msg->val == NULL) {
                        if (msg->key != NULL) {
                                free(msg->key);
                                msg->key = NULL;
                        }
                        return -1;
                }
        }

        return 0;
}

void socket_read(struct s_message *msg,
                 f_msg_handler msg_handler,
                 void *handler_arg)
{
        struct s_command *cmd = &msg->cmd;
        int iread = 0, offset = 0;
        uint8_t buf[SOCKET_READ_SIZE];
        uint32_t cmd_size = sizeof(struct s_command);
        uint32_t cp = 0;
        int sd = -1;

        if (msg->sd < 0 || msg_handler == NULL)
                return;

        sd = msg->sd;

        do {
                while (iread > 0) {
                        if (msg->cmd_len != cmd_size) {
                                uint8_t *cmd_data = (uint8_t *)&msg->cmd;
                                cp = cmd_size - msg->cmd_len;
                                if (cmd_size > (uint32_t)iread)
                                        cp = iread;

                                memcpy(&cmd_data[msg->cmd_len],
                                       &buf[offset],
                                       cp);

                                msg->cmd_len += cp;
                                offset += cp;
                                iread  -= cp;

                                if (msg->cmd_len == cmd_size) {
                                        if (!cmd_is_valid(cmd))
                                                goto close_socket;

                                        if (alloc_key_val(msg) != 0)
                                                goto close_socket;
                                }
                        }

                        if (msg->cmd_len != cmd_size)
                                continue;

                        if (iread > 0 && cmd->key_size != msg->key_len) {
                                cp = cmd->key_size - msg->key_len;
                                if (cp > (uint32_t)iread)
                                        cp = iread;

                                memcpy(&msg->key[msg->key_len],
                                                &buf[offset],
                                                cp);

                                msg->key_len += cp;
                                offset  += cp;
                                iread   -= cp;
                        }

                        if (iread > 0 && cmd->val_size != msg->val_len) {
                                cp = cmd->val_size - msg->val_len;
                                if (cp > (uint32_t)iread)
                                        cp = iread;

                                memcpy(&msg->val[msg->val_len],
                                                &buf[offset],
                                                cp);

                                msg->val_len += cp;
                                offset  += cp;
                                iread   -= cp;
                        }

                        if (msg->cmd_len == cmd_size &&
                                        cmd->key_size == msg->key_len &&
                                        cmd->val_size == msg->val_len) {

                                (*msg_handler)(msg, handler_arg);

                                memset(msg, 0, sizeof(struct s_message));
                                msg->sd = sd;
                        }
                }

                offset = 0;
                iread = read(sd, buf, SOCKET_READ_SIZE);

                if (iread < 0) {
                        return;
                } else if (iread == 0) {
                        goto close_socket;
                }
        } while (iread > 0);

        return;

close_socket:
        close(sd);
        sd = -1;
        msg->sd = -1;
}


int socket_write(struct s_message *msg)
{
        ssize_t iwrite = 0;
        int size = 0;

        if (msg == NULL) {
                errno = EINVAL;
                return -1;
        }

        if (msg->sd < 0) {
                errno = EBADF;
                return -1;
        }

        iwrite = write(msg->sd, &msg->cmd, sizeof(struct s_command));
        if (iwrite != sizeof(struct s_command))
                return -1;

        size += iwrite;

        if (msg->cmd.key_size != 0 && msg->key != NULL) {
                iwrite = write(msg->sd, msg->key, msg->cmd.key_size);
                if (iwrite != msg->cmd.key_size)
                        return -1;
                size += iwrite;
        }

        if (msg->cmd.val_size != 0 && msg->val != NULL) {
                iwrite = write(msg->sd, msg->val, msg->cmd.val_size);
                if (iwrite != msg->cmd.val_size)
                        return -1;
                size += iwrite;
        }

        return size;
}
