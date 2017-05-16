#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

#include <signal.h>

#include "list.h"
#include "stack.h"
#include "queue.h"

#include "common.h"
#include "server.h"
#include "db.h"
#include "socket_operations.h"

#define THREAD_QUEUE_SIZE       (1<<20) /**< Thread interaction queue   */

struct s_connection {
        struct s_message msg;
        int sd;
        struct s_list_item conn_list_item;
};

enum s_thread_flags {
        THREAD_INIT_SEMA  = 0x01,
        THREAD_INIT_OK    = 0x02
};

struct s_thread {
        int  id;
        int  init_flags;
        pthread_t thread;
        sem_t     sem;
        void     *queue;
};

struct s_server {
        int sd;
        uint32_t max_connection;

        void         *conn_stack;
        struct s_list conn_list;

        struct s_thread *readers;
        struct s_thread *writers;
        int readers_count;
        int writers_count;
        int last_reader;
        int last_writer;
};

static struct s_server *serv = NULL;
static int stop = 0;

static struct s_connection *server_accept_conn(struct s_server *server);
static void server_process_conn(struct s_server *server,
                                struct s_connection *conn);
static void put_msg_to_queue(struct s_message *msg, void * arg);

static void *thread_run(void *arg);

static int server_init_threads(struct s_thread *threads, uint32_t count)
{
        int i;
        for (i = 0; i < count; i++) {
                struct s_thread *th = &threads[i];
                th->id = i;

                th->queue = queue_init(THREAD_QUEUE_SIZE);
                if (th->queue == NULL) {
                        perror("Thread allocation memory error");
                        return -1;
                }

                if (sem_init(&th->sem, 0, 0) != 0) {
                        perror("Sem init error");
                        return -1;
                }
                th->init_flags |= THREAD_INIT_SEMA;

                if (pthread_create(&th->thread, NULL, thread_run, th) != 0) {
                        perror("Thread create error");
                        return -1;
                }
                th->init_flags |= THREAD_INIT_OK;
        }

        return 0;
}


static int server_release_threads(struct s_thread *threads, uint32_t count)
{
        int i;
        for (i = 0; i < count; i++) {
                struct s_thread *th = &threads[i];
                if (th->queue != NULL)
                        queue_release(th->queue);

                if (th->init_flags&THREAD_INIT_SEMA)
                        sem_destroy(&th->sem);

                if (th->init_flags&THREAD_INIT_OK) {
                        int *rv = 0;
                        int rc = 0;
                        rc = pthread_cancel(th->thread);

                        if (rc != 0) {
                                printf("pthread canceled error %d: %s\n",
                                       rc, strerror(rc));
                        }

                        rc = pthread_join(th->thread, (void **)&rv);

                        if (rc != 0 || rv != PTHREAD_CANCELED) {
                                printf("pthread join error %d: rc %d. rv %d\n",
                                       i, rc, *rv);
                        } else {
                                /*printf("pthread %d canceled OK\n", th->id);*/
                        }
                }
        }

        return 0;
}

int server_init(uint32_t max_server_connections,
                uint32_t db_nodes_count,
                uint32_t readers_count,
                uint32_t writers_count)
{
        sigset_t sigset, oldset;
        struct sockaddr_un addr;
        serv = malloc(sizeof(struct s_server));

        if (serv == NULL) {
                printf("Server allocation memory error.\n");
                return -1;
        }

        memset(serv, 0, sizeof(struct s_server));
        memset(&addr, 0, sizeof(addr));

        serv->max_connection = max_server_connections;

        if (db_init(db_nodes_count) != 0) {
                perror("DB init error");
                goto exit_on_fail;
        }

        serv->conn_stack = stack_init(serv->max_connection,
                                      sizeof(struct s_connection));

        if (serv->conn_stack == NULL) {
                perror("Connection stack init error");
                goto exit_on_fail;
        }

        serv->sd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (serv->sd  == -1)  {
                perror("Create server socket error");
                goto exit_on_fail;
        }

        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, DB_SOCKET_NAME, sizeof(addr.sun_path)-1);
        if (bind(serv->sd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
                perror("Bind server socket error");
                goto exit_on_fail;
        }

        if (listen(serv->sd, serv->max_connection) != 0) {
                perror("Listen server socket error");
                goto exit_on_fail;
        }

        serv->readers_count = readers_count;
        serv->writers_count = writers_count;
        serv->readers = malloc(sizeof(struct s_thread) * serv->readers_count);
        serv->writers = malloc(sizeof(struct s_thread) * serv->writers_count);
        if (serv->readers == NULL || serv->writers == NULL) {
                printf("Threads allocation memory error.\n");
                goto exit_on_fail;
        }

        memset(serv->readers, 0, sizeof(struct s_thread) * serv->readers_count);
        memset(serv->writers, 0, sizeof(struct s_thread) * serv->writers_count);

        sigemptyset(&sigset);
        sigaddset(&sigset, SIGINT);
        sigaddset(&sigset, SIGTERM);
        /*sigaddset(&sigset, SIGSEGV);*/
        pthread_sigmask(SIG_BLOCK, &sigset, &oldset);

        if (server_init_threads(serv->readers, serv->readers_count) != 0)
                goto exit_on_fail;

        if (server_init_threads(serv->writers, serv->writers_count) != 0)
                goto exit_on_fail;

        pthread_sigmask(SIG_SETMASK, &oldset, NULL);

        return 0;

exit_on_fail:
        printf("Server init failed.\n");
        server_release();
        return -1;
}

void server_release(void)
{
        struct s_connection *conn = NULL;
        if (serv == NULL)
                return;

        server_release_threads(serv->readers, serv->readers_count);
        server_release_threads(serv->writers, serv->writers_count);

        conn = list_get_item(serv->conn_list.first);
        while (conn != NULL) {
                close(conn->sd);
                conn = list_get_item(conn->conn_list_item.next);
        }

        if (serv->readers != NULL)
                free(serv->readers);

        if (serv->writers != NULL)
                free(serv->writers);

        if (serv->sd != -1) {
                close(serv->sd);
                unlink(DB_SOCKET_NAME);
        }

        db_release();

        if (serv->conn_stack != NULL)
                stack_release(serv->conn_stack);

        free(serv);
        serv = NULL;
}

int server_run(void)
{
        int max_sd = -1;
        fd_set readfds;

        struct s_connection *next = NULL;
        struct s_connection *conn = NULL;
        struct timeval timeout;

        if (serv == NULL) {
                errno = EINVAL;
                return -1;
        }

        while (!stop) {
                FD_ZERO(&readfds);
                FD_SET(serv->sd, &readfds);
                max_sd = serv->sd;

                conn = list_get_item(serv->conn_list.first);
                while (conn != NULL) {
                        FD_SET(conn->sd, &readfds);
                        if (conn->sd > max_sd)
                                max_sd = conn->sd;

                        conn = list_get_item(conn->conn_list_item.next);
                }

                timeout.tv_sec = 1;
                timeout.tv_usec = 0;
                if (select(max_sd + 1 , &readfds , NULL , NULL , &timeout) == -1) {
                        if (errno == EINTR) {
                                stop = 1;
                                break;
                        } else {
                                continue;
                        }
                }

                if (FD_ISSET(serv->sd, &readfds)) {
                        conn = server_accept_conn(serv);
                        if (conn)
                                server_process_conn(serv, conn);
                }

                conn = list_get_item(serv->conn_list.first);
                while (conn != NULL) {
                        next = list_get_item(conn->conn_list_item.next);

                        if (FD_ISSET(conn->sd, &readfds))
                                server_process_conn(serv, conn);

                        conn = next;
                }
        }
        return 0;
}

void server_stop(void)
{
        stop = 1;
}

static struct s_connection *server_accept_conn(struct s_server *server)
{
        struct s_connection *conn = NULL;
        int new_sd = accept(server->sd, NULL, NULL);
        int flags = 0;

        if (new_sd < 0)
                return NULL;

        conn = stack_pop(server->conn_stack);
        if (conn == NULL)
                return NULL;

        conn->sd = new_sd;
        conn->conn_list_item.item = conn;
        memset(&conn->msg, 0, sizeof(conn->msg));
        flags = fcntl(conn->sd, F_GETFL);
        fcntl(conn->sd, F_SETFL, flags | O_NONBLOCK);

        list_append(&server->conn_list, &conn->conn_list_item);
        return conn;
}

static void server_process_conn(struct s_server *server,
                                struct s_connection *conn)
{
        conn->msg.sd = conn->sd;
        socket_read(&conn->msg, put_msg_to_queue, server);
        if (conn->msg.sd < 0) {
                list_remove(&server->conn_list, &conn->conn_list_item);
                stack_push(server->conn_stack, conn);
        }
}

static void put_msg_to_queue(struct s_message *msg, void * arg)
{
        uint32_t msg_size = sizeof(struct s_message);
        struct s_server *server = (struct s_server *)arg;
        struct s_thread *th = NULL;

        if (msg->cmd.type == DB_CMD_PUT || msg->cmd.type == DB_CMD_ERASE) {
                server->last_writer++;
                server->last_writer %= server->writers_count;
                th = &server->writers[server->last_writer];
        } else {
                server->last_reader++;
                server->last_reader %= server->readers_count;
                th = &server->readers[server->last_reader];
        }

        if (queue_write(th->queue, (uint8_t *)msg, msg_size) == (int)msg_size)
                sem_post(&th->sem);
        else
                printf("th%d queue overflow\n", th->id);
}

static void *thread_run(void *arg)
{
        struct s_thread *th = (struct s_thread *)arg;
        struct s_message msg;
        uint32_t msg_size = sizeof(struct s_message);
        int size = 0;

        pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL);

        while (1) {
                if (sem_wait(&th->sem) != 0)
                        continue;

                do {
                        if (size > 0)
                                db_process_message(&msg);

                        size = queue_read(th->queue, (uint8_t *)&msg, msg_size);
                } while (size > 0);
        }

        pthread_exit(NULL);
}

