#include <stdlib.h>
#include <errno.h>

#include "list.h"

void list_init(struct s_list *list)
{
    list->first = NULL;
    list->last  = NULL;
}

int list_append(struct s_list *list, struct s_list_item *item)
{
    if (list == NULL || item == NULL) {
        errno = EINVAL;
        return -1;
    }

    item->next = NULL;
    item->prev = NULL;

    if (list->first == NULL) {
        list->first = item;
        list->last  = item;
        return 0;
    }

    item->prev = list->last;

    list->last->next = item;
    list->last = item;

    return 0;
}

int list_remove(struct s_list *list, struct s_list_item *item)
{
    if (list == NULL || item == NULL) {
        errno = EINVAL;
        return -1;
    }

    struct s_list_item *prev = item->prev;
    struct s_list_item *next = item->next;

    if (prev != NULL) prev->next = next;
    if (next != NULL) next->prev = prev;

    if (list->first == item) list->first = next;
    if (list->last  == item) list->last  = prev;

    item->prev = NULL;
    item->next = NULL;

    return 0;
}

void *list_get_item(struct s_list_item *item)
{
    if (item == NULL)
        return NULL;

    return item->item;
}



