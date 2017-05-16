#define BOOST_TEST_MODULE list_test

#ifdef DB_TEST_STATIC
#include <boost/test/included/unit_test.hpp>
#else
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#endif

#include "list.h"

struct s_test_data {
        struct s_list_item list_item;
        int val;
};

BOOST_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE(list_init_test)
{
        struct s_list list;
        struct s_list_item item;

        list.first = &item;
        list.last = &item;

        list_init(&list);

        BOOST_CHECK(list.first == NULL);
        BOOST_CHECK(list.last == NULL);
}

BOOST_AUTO_TEST_CASE(list_append_one_item_test)
{
        struct s_list list;
        struct s_test_data data;

        list_init(&list);

        data.list_item.item = (void *)&data;
        list_append(&list, &data.list_item);

        BOOST_CHECK(list.first == &data.list_item);
        BOOST_CHECK(list.last == &data.list_item);
}

BOOST_AUTO_TEST_CASE(list_append_two_item_test)
{
        struct s_list list;
        struct s_test_data data1;
        struct s_test_data data2;

        list_init(&list);

        data1.list_item.item = (void *)&data1;
        list_append(&list, &data1.list_item);

        data2.list_item.item = (void *)&data2;
        list_append(&list, &data2.list_item);

        BOOST_CHECK(list.first == &data1.list_item);
        BOOST_CHECK(list.last == &data2.list_item);
}

BOOST_AUTO_TEST_CASE(list_remove_one_last_item_test)
{
        struct s_list list;
        struct s_test_data data;

        list_init(&list);

        data.list_item.item = (void *)&data;
        list_append(&list, &data.list_item);

        BOOST_CHECK(list.first == &data.list_item);
        BOOST_CHECK(list.last == &data.list_item);

        list_remove(&list, &data.list_item);

        BOOST_CHECK(list.first == NULL);
        BOOST_CHECK(list.last == NULL);
}

BOOST_AUTO_TEST_CASE(list_remove_first_item_test)
{
        struct s_list list;
        const int count = 5;
        struct s_test_data data[count];

        list_init(&list);

        for (int i = 0; i < count; i++) {
             data[i].val = i;
             data[i].list_item.item = &data[i];
             list_append(&list, &data[i].list_item);
        }

        list_remove(&list, &data[0].list_item);

        BOOST_CHECK(list.first == &data[1].list_item);
        BOOST_CHECK(list.last == &data[count-1].list_item);
}

BOOST_AUTO_TEST_CASE(list_remove_middle_item_test)
{
        struct s_list list;
        const int count = 5;
        struct s_test_data data[count];

        list_init(&list);

        for (int i = 0; i < count; i++) {
             data[i].val = i;
             data[i].list_item.item = &data[i];
             list_append(&list, &data[i].list_item);
        }

        list_remove(&list, &data[2].list_item);

        BOOST_CHECK(list.first == &data[0].list_item);
        BOOST_CHECK(list.last == &data[count-1].list_item);

        int size = 0;
        struct s_list_item * list_item = list.first;
        while (list_item != NULL) {
                BOOST_CHECK(((struct s_test_data *)list_item->item)->val != 2);
                list_item = list_item->next;
                size++;
        }

        BOOST_CHECK(size == count - 1);
}

BOOST_AUTO_TEST_CASE(list_remove_last_item_test)
{
        struct s_list list;
        const int count = 5;
        struct s_test_data data[count];

        list_init(&list);

        for (int i = 0; i < count; i++) {
             data[i].val = i;
             data[i].list_item.item = &data[i];
             list_append(&list, &data[i].list_item);
        }

        list_remove(&list, &data[count-1].list_item);

        BOOST_CHECK(list.first == &data[0].list_item);
        BOOST_CHECK(list.last == &data[count-2].list_item);

        int size = 0;
        struct s_list_item * list_item = list.first;
        while (list_item != NULL) {
                BOOST_CHECK(((struct s_test_data *)list_item->item)->val != 4);
                list_item = list_item->next;
                size++;
        }

        BOOST_CHECK(size == count - 1);
}

BOOST_AUTO_TEST_SUITE_END()
