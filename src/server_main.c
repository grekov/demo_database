#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>

#include "server.h"
#include "config.h"

static void signal_handler(int signo)
{
        server_stop();
}

int main(void)
{
        struct sigaction sa;
        int rc = EXIT_SUCCESS;

        memset(&sa, 0, sizeof(sa));

        sa.sa_handler = &signal_handler;
        sa.sa_flags = SA_RESETHAND;
        sigfillset(&sa.sa_mask);

        if (sigaction(SIGINT, &sa, NULL) == -1)
                perror("Warning: cannot hanle SIGINT");

        if (sigaction(SIGTERM, &sa, NULL) == -1)
                perror("Warning: cannot hanle SIGTERM");

        if (sigaction(SIGSEGV, &sa, NULL) == -1)
                perror("Warning: cannot hanle SIGSEGV");

        if (server_init(DB_SERVER_MAX_CONNECTIONS,
                        DB_SERVER_NODES_COUNT,
                        DB_SERVER_READERS_COUNT,
                        DB_SERVER_WRITERS_COUNT) != 0) {
                rc = EXIT_FAILURE;
                goto server_init_err;
        }

        server_run();
        server_release();

server_init_err:
        exit(rc);
}
