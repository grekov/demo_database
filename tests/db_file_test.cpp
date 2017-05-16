#define BOOST_TEST_MODULE db_file_test

#ifdef DB_TEST_STATIC
#include <boost/test/included/unit_test.hpp>
#else
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#endif

#include <stdio.h>

#include "db_file.h"

#define DB_FILE_NAME "db_test_file.txt"

BOOST_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(db_file_init_test)
{
        void *db_file = db_file_init(DB_FILE_NAME);
        FILE *file = NULL;

        BOOST_REQUIRE(db_file != NULL);

        file = fopen(DB_FILE_NAME, "r");
        BOOST_CHECK(file != NULL);
        if (file != NULL)
                fclose(file);

        db_file_release(db_file);
}

BOOST_AUTO_TEST_CASE(db_file_release_test)
{
        void *db_file = db_file_init(DB_FILE_NAME);
        FILE *file = NULL;

        BOOST_REQUIRE(db_file != NULL);
        db_file_release(db_file);

        file = fopen(DB_FILE_NAME, "r");
        BOOST_CHECK(file == NULL);

        if (file != NULL )
                fclose(file);
}

BOOST_AUTO_TEST_CASE(db_file_get_space_test)
{
        void *db_file = db_file_init(DB_FILE_NAME);
        BOOST_REQUIRE(db_file != NULL);

        BOOST_CHECK(db_file_get_space(db_file, 1024) == 0);
        BOOST_CHECK(db_file_get_space(db_file, 1024) == 1024);
        BOOST_CHECK(db_file_get_space(db_file, 1024) == 2048);

        db_file_release(db_file);
}

BOOST_AUTO_TEST_CASE(db_file_put_space_test)
{
        void *db_file = db_file_init(DB_FILE_NAME);
        BOOST_REQUIRE(db_file != NULL);

        BOOST_CHECK(db_file_get_space(db_file, 512) == 0);
        BOOST_CHECK(db_file_get_space(db_file, 512) == 512);
        BOOST_CHECK(db_file_get_space(db_file, 512) == 1024);

        db_file_put_space(db_file, 512, 512);
        BOOST_CHECK(db_file_get_space(db_file, 128) == 512);
        BOOST_CHECK(db_file_get_space(db_file, 128) == 512 + 128);
        BOOST_CHECK(db_file_get_space(db_file, 128) == 512 + 2*128);
        BOOST_CHECK(db_file_get_space(db_file, 128) == 512 + 3*128);
        BOOST_CHECK(db_file_get_space(db_file, 128) == 1024 + 512);

        db_file_release(db_file);
}

BOOST_AUTO_TEST_CASE(db_file_put_near_spaces_and_get_equal_test)
{
        void *db_file = db_file_init(DB_FILE_NAME);
        BOOST_REQUIRE(db_file != NULL);

        BOOST_CHECK(db_file_get_space(db_file, 64) == 0);
        BOOST_CHECK(db_file_get_space(db_file, 128) == 64);
        BOOST_CHECK(db_file_get_space(db_file, 128) == 64 + 128);
        BOOST_CHECK(db_file_get_space(db_file, 256) == 64 + 256);

        db_file_put_space(db_file, 0, 64);
        db_file_put_space(db_file, 64, 128);

        BOOST_CHECK(db_file_get_space(db_file, 64 + 128) == 0);

        db_file_release(db_file);
}

BOOST_AUTO_TEST_CASE(db_file_put_near_spaces_and_get_bigger_test)
{
        void *db_file = db_file_init(DB_FILE_NAME);
        BOOST_REQUIRE(db_file != NULL);

        BOOST_CHECK(db_file_get_space(db_file, 64) == 0);
        BOOST_CHECK(db_file_get_space(db_file, 128) == 64);
        BOOST_CHECK(db_file_get_space(db_file, 128) == 64 + 128);
        BOOST_CHECK(db_file_get_space(db_file, 256) == 64 + 256);

        db_file_put_space(db_file, 0, 64);
        db_file_put_space(db_file, 64, 128);

        BOOST_CHECK(db_file_get_space(db_file, 256) == 512 + 64);

        db_file_release(db_file);
}

BOOST_AUTO_TEST_CASE(db_file_put_two_spaces_and_get_more_suitable_test)
{
        void *db_file = db_file_init(DB_FILE_NAME);
        BOOST_REQUIRE(db_file != NULL);

        BOOST_CHECK(db_file_get_space(db_file, 64) == 0);
        BOOST_CHECK(db_file_get_space(db_file, 128) == 64);
        BOOST_CHECK(db_file_get_space(db_file, 128) == 64 + 128);
        BOOST_CHECK(db_file_get_space(db_file, 256) == 64 + 256);

        db_file_put_space(db_file, 0, 64);
        db_file_put_space(db_file, 64 + 128, 128);

        BOOST_CHECK(db_file_get_space(db_file, 64) == 0);
        BOOST_CHECK(db_file_get_space(db_file, 64) == 64 + 128);

        db_file_release(db_file);
}

BOOST_AUTO_TEST_SUITE_END()
