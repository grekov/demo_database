#define BOOST_TEST_MODULE stack_test

#ifdef DB_TEST_STATIC
#include <boost/test/included/unit_test.hpp>
#else
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#endif

#include "stack.h"

BOOST_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(stack_init_test)
{
        void *stack = stack_init(10, sizeof(int));
        BOOST_REQUIRE(stack != NULL);
        stack_release(stack);
}

BOOST_AUTO_TEST_CASE(stack_init_zero_test)
{
        void *stack = stack_init(0, 0);
        BOOST_CHECK(stack == NULL);
        stack_release(stack);
}

BOOST_AUTO_TEST_CASE(stack_pop_null_test)
{
        BOOST_CHECK(stack_pop(NULL) == NULL);
}

BOOST_AUTO_TEST_CASE(stack_pop_test)
{
        void *stack = stack_init(10, sizeof(int));
        BOOST_REQUIRE(stack != NULL);

        int *item = (int *)stack_pop(stack);
        BOOST_REQUIRE(item != NULL);

        stack_release(stack);
}

BOOST_AUTO_TEST_CASE(stack_pop_push_pop_test)
{
        void *stack = stack_init(10, sizeof(int));
        BOOST_REQUIRE(stack != NULL);

        int *item = (int *)stack_pop(stack);
        BOOST_REQUIRE(item != NULL);

        *item = 5;

        stack_push(stack, item);
        BOOST_CHECK(*(int *)stack_pop(stack) == 5);

        stack_release(stack);
}

BOOST_AUTO_TEST_CASE(stack_pop_all_test)
{
        void *stack = stack_init(3, sizeof(int));
        BOOST_REQUIRE(stack != NULL);

        int *item = (int *)stack_pop(stack);
        BOOST_REQUIRE(item != NULL);

        stack_pop(stack);
        stack_pop(stack);
        stack_pop(stack);

        BOOST_CHECK(stack_pop(stack) == NULL);

        stack_release(stack);
}

BOOST_AUTO_TEST_SUITE_END()
