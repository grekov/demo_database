#define BOOST_TEST_MODULE db_test

#ifdef DB_TEST_STATIC
#include <boost/test/included/unit_test.hpp>
#else
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#endif

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdint.h>

#include "common.h"
#include "db.h"

extern "C" void create_msg(struct s_message *msg, int cmd_type, int key_only)
{
        char key[] = "key string";
        char val[] = "value string";

        memset(msg, 0, sizeof(struct s_message));
        msg->sd = -1;

        msg->cmd.type = cmd_type;
        msg->cmd.key_size = strlen(key) + 1;
        if (!key_only)
                msg->cmd.val_size = strlen(val) + 1;
        msg->cmd.len = sizeof(msg->cmd) + msg->cmd.key_size + msg->cmd.val_size;

        msg->key = (uint8_t *)malloc(msg->cmd.key_size);
        BOOST_REQUIRE(msg->key != NULL);
        memcpy(msg->key, key, msg->cmd.key_size);

        if (!key_only) {
                msg->val = (uint8_t *)malloc(msg->cmd.val_size);
                BOOST_REQUIRE(msg->val != NULL);
                memcpy(msg->val, val, msg->cmd.val_size);
        }
}

BOOST_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(db_init_release_test)
{
        int rc = db_init(1);
        FILE *file = NULL;

        BOOST_REQUIRE(rc == 0);

        file = fopen("db_key_node_0.txt", "r");
        BOOST_CHECK(file != NULL);
        if (file != NULL)
                fclose(file);

        file = fopen("db_val_node_0.txt", "r");
        BOOST_CHECK(file != NULL);
        if (file != NULL)
                fclose(file);

        db_release();

        file = fopen("db_key_node_0.txt", "r");
        BOOST_CHECK(file == NULL);
        if (file != NULL)
                fclose(file);

        file = fopen("db_val_node_0.txt", "r");
        BOOST_CHECK(file == NULL);
        if (file != NULL)
                fclose(file);
}

BOOST_AUTO_TEST_CASE(db_put_test)
{
        int fd = -1;
        const int size = 64;
        char rbuf[size];
        struct s_message msg;
        int rc = db_init(1);
        BOOST_REQUIRE(rc == 0);

        create_msg(&msg, DB_CMD_PUT, 0);
        db_process_message(&msg);

        fd = open("db_key_node_0.txt", O_RDONLY);
        BOOST_REQUIRE(fd != -1);
        rc = pread(fd, rbuf, msg.cmd.key_size, 3*sizeof(uint32_t));
        BOOST_CHECK(rc == (int)msg.cmd.key_size);
        BOOST_CHECK(memcmp(msg.key, rbuf, msg.cmd.key_size) == 0);
        close(fd);

        fd = open("db_val_node_0.txt", O_RDONLY);
        BOOST_REQUIRE(fd != -1);
        rc = pread(fd, rbuf, msg.cmd.val_size, sizeof(uint32_t));
        BOOST_CHECK(rc == (int)msg.cmd.val_size);
        BOOST_CHECK(memcmp(msg.val, rbuf, msg.cmd.val_size) == 0);
        close(fd);

        db_release();
}

BOOST_AUTO_TEST_CASE(db_erase_test)
{
        int fd = -1;
        const int size = 64;
        char rbuf[size];
        struct s_message msg_put;
        struct s_message msg_erase;
        int rc = db_init(1);
        BOOST_REQUIRE(rc == 0);

        create_msg(&msg_put, DB_CMD_PUT, 0);
        create_msg(&msg_erase, DB_CMD_ERASE, 1);

        db_process_message(&msg_put);
        db_process_message(&msg_erase);

        fd = open("db_key_node_0.txt", O_RDONLY);
        BOOST_REQUIRE(fd != -1);
        rc = pread(fd, rbuf, msg_put.cmd.key_size, 3*sizeof(uint32_t));
        BOOST_CHECK(rc == 0);
        close(fd);

        fd = open("db_val_node_0.txt", O_RDONLY);
        BOOST_REQUIRE(fd != -1);
        rc = pread(fd, rbuf, msg_put.cmd.val_size, sizeof(uint32_t));
        BOOST_CHECK(rc == 0);
        close(fd);

        db_release();
}

BOOST_AUTO_TEST_SUITE_END()
