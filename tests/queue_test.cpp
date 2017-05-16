#define BOOST_TEST_MODULE queue_test

#ifdef DB_TEST_STATIC
#include <boost/test/included/unit_test.hpp>
#else
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#endif

#include "queue.h"

BOOST_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(queue_init_test)
{
        void *queue = queue_init(1<<4);
        BOOST_REQUIRE(queue != NULL);
        queue_release(queue);
}

BOOST_AUTO_TEST_CASE(queue_init_zero_test)
{
        void *queue = queue_init(0);
        BOOST_CHECK(queue == NULL);
        queue_release(queue);
}

BOOST_AUTO_TEST_CASE(queue_non_init_read_write_test)
{
        BOOST_CHECK(queue_write(NULL, NULL, 0) == -1);
        BOOST_CHECK(queue_read(NULL, NULL, 0) == -1);
}

BOOST_AUTO_TEST_CASE(queue_write_test)
{
        void *queue = queue_init(1<<5);
        uint8_t buf[1<<4];

        BOOST_REQUIRE(queue != NULL);
        BOOST_CHECK(queue_write(queue, buf, sizeof(buf) == sizeof(buf)));

        queue_release(queue);
}

BOOST_AUTO_TEST_CASE(queue_write_overflow_test)
{
        void *queue = queue_init(1<<5);
        uint8_t buf[1<<6];

        BOOST_REQUIRE(queue != NULL);
        BOOST_CHECK(queue_write(queue, buf, sizeof(buf)  == -1));

        queue_release(queue);
}

BOOST_AUTO_TEST_CASE(queue_write_read_test)
{
        void *queue = queue_init(1<<5);
        const uint32_t buf_size = 1<<4;
        uint8_t wbuf[buf_size];
        uint8_t rbuf[buf_size];

        memset(wbuf, 0xAB, buf_size);
        memset(rbuf, 0x00, buf_size);

        BOOST_REQUIRE(queue != NULL);
        BOOST_CHECK(queue_write(queue, wbuf, buf_size) == (int)buf_size);
        BOOST_CHECK(queue_read(queue, rbuf, buf_size) == (int)buf_size);
        BOOST_CHECK(memcmp(wbuf, rbuf, buf_size) == 0);

        queue_release(queue);
}

BOOST_AUTO_TEST_CASE(queue_round_write_read_test)
{
        void *queue = queue_init(1<<5);
        const uint32_t buf_size = 24;
        uint8_t wbuf[buf_size];
        uint8_t rbuf[buf_size];

        memset(wbuf, 0xAB, buf_size);
        memset(rbuf, 0x00, buf_size);

        BOOST_REQUIRE(queue != NULL);

        BOOST_CHECK(queue_write(queue, wbuf, buf_size) == (int)buf_size);
        BOOST_CHECK(queue_read(queue, rbuf, buf_size) == (int)buf_size);
        BOOST_CHECK(memcmp(wbuf, rbuf, buf_size) == 0);

        memset(wbuf, 0xAC, buf_size);
        memset(rbuf, 0x00, buf_size);
        BOOST_CHECK(queue_write(queue, wbuf, buf_size) == (int)buf_size);
        BOOST_CHECK(queue_read(queue, rbuf, buf_size) == (int)buf_size);
        BOOST_CHECK(memcmp(wbuf, rbuf, buf_size) == 0);

        memset(wbuf, 0xBC, buf_size);
        memset(rbuf, 0x00, buf_size);
        BOOST_CHECK(queue_write(queue, wbuf, buf_size) == (int)buf_size);
        BOOST_CHECK(queue_read(queue, rbuf, buf_size) == (int)buf_size);
        BOOST_CHECK(memcmp(wbuf, rbuf, buf_size) == 0);

        queue_release(queue);
}

BOOST_AUTO_TEST_SUITE_END()
