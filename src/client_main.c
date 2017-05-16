#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>
#include <fcntl.h>

#include "common.h"
#include "socket_operations.h"

#define CLIENT_WAIT_TIMEOUT_SEC    (5*1000)

static int init_message(int argc, char *argv[],struct s_message *msg)
{
        struct s_command *cmd = &msg->cmd;

        memset(msg, 0, sizeof(struct s_message));

        if (argc < 2) {
                errno = EINVAL;
                perror("Too few arguments");
                return -1;
        }

        if (strcmp(argv[1], "put") == 0)
                cmd->type = DB_CMD_PUT;
        else if (strcmp(argv[1], "get") == 0)
                cmd->type = DB_CMD_GET;
        else if (strcmp(argv[1], "erase") == 0)
                cmd->type = DB_CMD_ERASE;
        else if (strcmp(argv[1], "list") == 0)
                cmd->type = DB_CMD_LIST;

        if (cmd->type == -1) {
                errno = EINVAL;
                perror("unkown command");
                return -1;
        }

        switch(cmd->type) {
        case DB_CMD_PUT:
                if (argc < 4) {
                        errno = EINVAL;
                        perror("Too few arguments");
                        return -1;
                }
                cmd->key_size = strlen(argv[2]) + 1;
                cmd->val_size = strlen(argv[3]) + 1;
                break;
        case DB_CMD_GET:
        case DB_CMD_ERASE:
                if (argc < 3)
                {
                        errno = EINVAL;
                        perror("Too few arguments");
                        return -1;
                }
                cmd->key_size = strlen(argv[2]) + 1;
                break;
        case DB_CMD_LIST:
                break;
        default:
                break;
        }

        cmd->len = sizeof(struct s_command) + cmd->key_size + cmd->val_size;

        if (cmd->key_size != 0) {
                msg->key = malloc(cmd->key_size);
                if (msg->key == NULL)
                        goto alloc_error;

                memcpy(msg->key, argv[2], cmd->key_size);
        }

        if (cmd->val_size != 0) {
                msg->val = malloc(cmd->val_size);
                if (msg->val == NULL)
                        goto alloc_error;

                memcpy(msg->val, argv[3], cmd->val_size);
        }

        return 0;

alloc_error:
        printf("Memory allocation error\n");
        if (msg->key != NULL)
                free(msg->key);
        if (msg->val != NULL)
                free(msg->val);
        memset(msg, 0, sizeof(struct s_message));
        return -1;
}

static void release_message(struct s_message *msg)
{
        if (msg->key != NULL)
                free(msg->key);

        if (msg->val != NULL)
                free(msg->val);

        memset(msg, 0, sizeof(struct s_message));
}

static void process_response(struct s_message *resp, void *arg)
{
        int * wait_response = (int *)arg;
        if (resp == NULL)
                return;

        if (resp->cmd.val_size && resp->val) {
                printf("%s\n", resp->val);
                free(resp->val);
                resp->val = NULL;
        } else {
                *wait_response = 0;
        }
}

int main(int argc, char *argv[])
{
        int rc = EXIT_SUCCESS;
        int iwrite = 0;
        int wait_response = 1;

        struct s_message msg;
        struct sockaddr_un server_addr_un;
        struct sockaddr *server_addr = (struct sockaddr *)&server_addr_un;

        if (init_message(argc, argv, &msg) != 0)
                exit(EXIT_FAILURE);

        msg.sd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (msg.sd == -1) {
                perror("Opening stream socket");
                rc = EXIT_FAILURE;
                goto socket_create_err;
        }

        memset(&server_addr_un, 0, sizeof(struct sockaddr_un));
        server_addr_un.sun_family = AF_UNIX;
        strcpy(server_addr_un.sun_path, DB_SOCKET_NAME);

        if (connect(msg.sd, server_addr, sizeof(struct sockaddr_un)) == -1) {
                perror("Connecting stream socket");
                rc = EXIT_FAILURE;
                goto socket_connect_err;
        }

        iwrite = socket_write(&msg);

        if (iwrite == -1) {
                perror("Writing to the stream socket");
        } else {
                struct s_message resp;
                struct pollfd fds;
                int rv = 0;

                memset(&resp, 0, sizeof(struct s_message));
                memset(&fds, 0, sizeof(fds));

                resp.sd = msg.sd;
                fds.fd = resp.sd;
                fds.events = POLLIN;

                while (wait_response) {

                        rv = poll(&fds, 1, CLIENT_WAIT_TIMEOUT_SEC);
                        if (rv < 0) {
                                break;
                        } else if (rv == 0) {
                                printf("Timeout expired.Cmd type [%d]. Exit.\n",
                                       msg.cmd.type);
                                break;
                        } else {
                                socket_read(&resp,
                                            process_response,
                                            &wait_response);
                        }
                }

                if (resp.val != NULL)
                        free(resp.val);
        }

socket_connect_err:
        close(msg.sd);
socket_create_err:
        release_message(&msg);
        exit(rc);
}
