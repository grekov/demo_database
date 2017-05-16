#define BOOST_TEST_MODULE socket_operations_test

#ifdef DB_TEST_STATIC
#include <boost/test/included/unit_test.hpp>
#else
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#endif

#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <fcntl.h>

#include "socket_operations.h"
#include "common.h"

void handler(struct s_message *msg, void *arg)
{
     struct s_message *server_msg = (struct s_message *)arg;
     BOOST_REQUIRE(msg != NULL);
     BOOST_REQUIRE(arg != NULL);
     memcpy(server_msg, msg, sizeof(struct s_message));
}

void *server_thread(void *args)
{
        int sd = -1;
        struct s_message msg;
        struct sockaddr_un addr;

        sd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sd == -1) {
                BOOST_MESSAGE("Create socket error");
                goto exit_on_fail;
        }

        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, DB_SOCKET_NAME, sizeof(addr.sun_path)-1);
        if (bind(sd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
                BOOST_MESSAGE("Bind server socket error");
                goto exit_on_fail;
        }

        if (listen(sd, 1) != 0) {
                BOOST_MESSAGE("Listen server socket error");
                goto exit_on_fail;
        }

        memset(&msg, 0, sizeof(msg));
        msg.sd = accept(sd, NULL, NULL);
        if (msg.sd == -1) {
                BOOST_MESSAGE("Accept socket error");
                goto exit_on_fail;
        }

        socket_read(&msg, handler, args);
        BOOST_CHECK(msg.sd == -1);

exit_on_fail:
        if (sd != -1) {
                close(sd);
                unlink(DB_SOCKET_NAME);
        }
        return NULL;
}

void *client_thread(void *args)
{
        struct s_message *msg = (struct s_message *)args;
        struct sockaddr_un server_addr_un;
        struct sockaddr *server_addr = (struct sockaddr *)&server_addr_un;

        msg->sd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (msg->sd == -1) {
                BOOST_MESSAGE("Opening stream socket error");
                return NULL;
        }

        memset(&server_addr_un, 0, sizeof(struct sockaddr_un));
        server_addr_un.sun_family = AF_UNIX;
        strcpy(server_addr_un.sun_path, DB_SOCKET_NAME);

        if (connect(msg->sd, server_addr, sizeof(struct sockaddr_un)) == -1) {
                BOOST_MESSAGE("Connecting stream socket error");
                close(msg->sd);
                return NULL;
        }

        BOOST_CHECK(socket_write(msg) == (int)(sizeof(msg->cmd) +
                    msg->cmd.key_size +
                    msg->cmd.val_size));

        close(msg->sd);

        return NULL;
}

BOOST_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(socket_small_data_test)
{
        pthread_t client;
        pthread_t server;

        char key[] = "key string";
        char val[] = "value string";

        struct s_message client_msg;
        struct s_message server_msg;

        memset(&client_msg, 0, sizeof(client_msg));
        memset(&server_msg, 0, sizeof(server_msg));

        client_msg.cmd.type = DB_CMD_PUT;
        client_msg.cmd.key_size = strlen(key) + 1;
        client_msg.cmd.val_size = strlen(val) + 1;
        client_msg.cmd.len = sizeof(client_msg.cmd) +
                        client_msg.cmd.key_size +
                        client_msg.cmd.val_size;


        client_msg.key = (uint8_t *)malloc(client_msg.cmd.key_size);
        client_msg.val = (uint8_t *)malloc(client_msg.cmd.val_size);

        BOOST_REQUIRE(client_msg.key != NULL);
        BOOST_REQUIRE(client_msg.val != NULL);

        memcpy(client_msg.key, key, client_msg.cmd.key_size);
        memcpy(client_msg.val, val, client_msg.cmd.val_size);

        server_msg.cmd.type = 0xFFFFFFFF;
        pthread_create(&server, NULL, server_thread, &server_msg);
        sleep(1);
        pthread_create(&client, NULL, client_thread, &client_msg);
        sleep(1);

        pthread_join(client, NULL);
        pthread_join(server, NULL);

        BOOST_MESSAGE("All thread joined.");
        BOOST_CHECK(client_msg.cmd.type == server_msg.cmd.type);
        BOOST_CHECK(client_msg.cmd.key_size == server_msg.cmd.key_size);
        BOOST_CHECK(client_msg.cmd.val_size == server_msg.cmd.val_size);

        BOOST_REQUIRE(server_msg.key != NULL);
        BOOST_REQUIRE(server_msg.val != NULL);

        BOOST_CHECK(memcmp(client_msg.key,
                           server_msg.key,
                           client_msg.cmd.key_size) == 0);
        BOOST_CHECK(memcmp(client_msg.val,
                           server_msg.val,
                           client_msg.cmd.val_size) == 0);

        free(client_msg.key);
        free(client_msg.val);

        if (server_msg.key != NULL)
                free(server_msg.key);
        if (server_msg.val != NULL)
                free(server_msg.val);
}

BOOST_AUTO_TEST_CASE(socket_big_data_test)
{
        pthread_t client;
        pthread_t server;

        char key[4096];
        char val[4096];

        struct s_message client_msg;
        struct s_message server_msg;

        for (int i = 0; i < 4095; i++) {
                key[i] = (i % 128) + 1;
                val[i] = (i % 128) + 2;
        }

        key[4095] = '\0';
        val[4095] = '\0';

        memset(&client_msg, 0, sizeof(client_msg));
        memset(&server_msg, 0, sizeof(server_msg));

        client_msg.cmd.type = DB_CMD_PUT;
        client_msg.cmd.key_size = strlen(key) + 1;
        client_msg.cmd.val_size = strlen(val) + 1;
        client_msg.cmd.len = sizeof(client_msg.cmd) +
                        client_msg.cmd.key_size +
                        client_msg.cmd.val_size;

        client_msg.key = (uint8_t *)malloc(client_msg.cmd.key_size);
        client_msg.val = (uint8_t *)malloc(client_msg.cmd.val_size);

        BOOST_REQUIRE(client_msg.key != NULL);
        BOOST_REQUIRE(client_msg.val != NULL);

        memcpy(client_msg.key, key, client_msg.cmd.key_size);
        memcpy(client_msg.val, val, client_msg.cmd.val_size);

        server_msg.cmd.type = 0xFFFFFFFF;
        pthread_create(&server, NULL, server_thread, &server_msg);
        sleep(1);
        pthread_create(&client, NULL, client_thread, &client_msg);

        pthread_join(client, NULL);
        pthread_join(server, NULL);

        BOOST_MESSAGE("All thread joined.");

        BOOST_CHECK(client_msg.cmd.type == server_msg.cmd.type);
        BOOST_CHECK(client_msg.cmd.key_size == server_msg.cmd.key_size);
        BOOST_CHECK(client_msg.cmd.val_size == server_msg.cmd.val_size);

        BOOST_REQUIRE(server_msg.key != NULL);
        BOOST_REQUIRE(server_msg.val != NULL);

        BOOST_CHECK(memcmp(client_msg.key,
                           server_msg.key,
                           client_msg.cmd.key_size) == 0);
        BOOST_CHECK(memcmp(client_msg.val,
                           server_msg.val,
                           client_msg.cmd.val_size) == 0);

        free(client_msg.key);
        free(client_msg.val);
}


BOOST_AUTO_TEST_SUITE_END()
