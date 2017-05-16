#define BOOST_TEST_MODULE db_node_test

#ifdef DB_TEST_STATIC
#include <boost/test/included/unit_test.hpp>
#else
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#endif

#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "db_node.h"

#define DB_NODE_NAME "db_test_file.txt"

BOOST_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(db_node_init_test)
{
        void *node = db_node_init(DB_NODE_NAME);
        FILE *file = NULL;

        BOOST_REQUIRE(node != NULL);

        file = fopen(DB_NODE_NAME, "r");
        BOOST_CHECK(file != NULL);
        if (file != NULL)
                fclose(file);

        db_node_release(node);
}

BOOST_AUTO_TEST_CASE(db_node_release_test)
{
        void *node = db_node_init(DB_NODE_NAME);
        FILE *file = NULL;

        BOOST_REQUIRE(node != NULL);
        db_node_release(node);

        file = fopen(DB_NODE_NAME, "r");
        BOOST_CHECK(file == NULL);
        if (file != NULL)
                fclose(file);
}

BOOST_AUTO_TEST_CASE(db_node_put_item_test)
{
        void *node = db_node_init(DB_NODE_NAME);
        struct s_db_item *db_item = NULL;
        const int size = 1024;
        uint8_t *buf = (uint8_t *)malloc(size);
        BOOST_REQUIRE(node != NULL);

        db_item = db_node_put_item(node, buf, size);
        BOOST_REQUIRE(db_item != NULL);
        BOOST_CHECK(db_item->data != NULL);
        BOOST_CHECK(db_item->size == size);
        BOOST_CHECK(db_item->ref_counter == 0);
        BOOST_CHECK(db_item->f_offset == 0);
        BOOST_CHECK(db_item->f_size == 0);
        BOOST_CHECK(db_item->ref_item == NULL);
        BOOST_CHECK(db_item->list_item.next == NULL);
        BOOST_CHECK(db_item->list_item.prev == NULL);

        db_node_release(node);
}

BOOST_AUTO_TEST_CASE(db_node_remove_item_test)
{
        void *node = db_node_init(DB_NODE_NAME);
        struct s_db_item *db_item = NULL;
        const int size = 1024;
        uint8_t *buf = (uint8_t *)malloc(size);
        BOOST_REQUIRE(node != NULL);

        db_item = db_node_put_item(node, buf, size);
        BOOST_REQUIRE(db_item != NULL);
        BOOST_CHECK(db_node_remove_item(node, db_item) == 0);

        db_node_release(node);
}

BOOST_AUTO_TEST_CASE(db_node_get_item_test)
{
        void *node = db_node_init(DB_NODE_NAME);
        struct s_db_item *db_item = NULL;
        const int size = 64;
        uint8_t *buf = (uint8_t *)malloc(size);
        uint8_t gbuf[size];
        BOOST_REQUIRE(node != NULL);

        BOOST_CHECK(db_node_get_item(node, NULL, 0) == NULL);
        BOOST_CHECK(db_node_get_item(node, NULL, size) == NULL);
        BOOST_CHECK(db_node_get_item(node, gbuf, 0) == NULL);
        BOOST_CHECK(db_node_get_item(node, gbuf, size) == NULL);

        buf[0] = '\0';
        gbuf[0] = '\0';
        strncpy((char *)buf, "magic value", size);
        strncpy((char *)gbuf, (char *)buf, size);

        db_item = db_node_put_item(node, buf, strlen((char *)buf));
        BOOST_REQUIRE(db_item != NULL);

        strncpy((char *)gbuf, (char *)buf, size);
        db_item = db_node_get_item(node, gbuf, strlen((char *)buf));
        BOOST_CHECK(db_node_get_item(node, gbuf, strlen((char *)buf)) != NULL);

        strncpy((char *)gbuf, "not exists value", size);
        BOOST_CHECK(db_node_get_item(node, gbuf, strlen((char *)buf)) == NULL);

        db_node_release(node);
}

BOOST_AUTO_TEST_CASE(db_node_iterator_test)
{
        void *node = db_node_init(DB_NODE_NAME);
        struct s_db_item *db_item = NULL;
        const int size = 1024;
        uint8_t *buf = (uint8_t *)malloc(size);
        BOOST_REQUIRE(node != NULL);

        buf[0] = 0xAA;

        db_item = db_node_put_item(node, buf, size);
        BOOST_REQUIRE(db_item != NULL);

        void *it = db_node_get_iterator(node);
        struct s_db_item *next = NULL;
        BOOST_CHECK(it != NULL);
        BOOST_CHECK(db_node_iterator_has_next(it) != 0);
        next = db_node_get_next(node, it);
        BOOST_CHECK(next != NULL);
        BOOST_CHECK(next->data[0] = 0xAA);

        BOOST_CHECK(db_node_remove_item(node, db_item) == 0);
        it = db_node_get_iterator(node);
        BOOST_CHECK(db_node_iterator_has_next(it) == 0);

        db_node_release(node);
}

BOOST_AUTO_TEST_CASE(db_node_save_test)
{
        int fd = -1;
        void *node = db_node_init(DB_NODE_NAME);
        struct s_db_item *db_item = NULL;
        const int size = 64;
        uint8_t *buf = (uint8_t *)malloc(size);
        uint8_t rbuf[size];
        BOOST_REQUIRE(node != NULL);

        memset(buf, 0xAB, size);
        memset(rbuf, 0, size);

        db_item = db_node_put_item(node, buf, size);
        BOOST_REQUIRE(db_item != NULL);

        db_node_save(node, db_item, 0);

        fd = open(DB_NODE_NAME, O_RDONLY);
        BOOST_REQUIRE(fd != -1);

        BOOST_CHECK(pread(fd, rbuf, size, sizeof(db_item->f_size)) == size);
        BOOST_CHECK(memcmp(db_item->data, rbuf, size) == 0);

        if (fd != -1)
                close(fd);

        db_node_release(node);
}

BOOST_AUTO_TEST_SUITE_END()
