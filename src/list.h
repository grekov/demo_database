#ifndef LIST_H
#define LIST_H

/**
 * @file list.h
 * @author Sviatoslav
 * @brief Linked list.
 *
 * Helpful structs and functions for bi-directional linked list.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Item of linked list.
 */
struct s_list_item {
    void *item;               /**< Pointer to the user data item     */
    struct s_list_item *prev; /**< Pointer to the previous list item */
    struct s_list_item *next; /**< Pointer to the next list item     */
};

/**
 * @brief Linked list struct.
 */
struct s_list {
    struct s_list_item *first; /**< First list item */
    struct s_list_item *last;  /**< Last list item */
};

/**
 * @brief Initialize list. Set first and last items to NULL.
 * @param list Pointer to the list.
 */
void list_init(struct s_list *list);

/**
 * @brief Insert item at the end of the list.
 * @param list List.
 * @param item Item for insert.
 * @return On success, return zero.
 * On error, -1 is returned, and errno is set.
 */
int list_append(struct s_list *list, struct s_list_item *item);

/**
 * @brief Remove item from the list.
 * @param list List.
 * @param item Item for remove.
 * @return On success, return zero.
 * On error, -1 is returned, and errno is set.
 */
int list_remove(struct s_list *list, struct s_list_item *item);

/**
 * @brief Extract pointer to the user data item.
 * @param item List item.
 * @return Pointer to the user data item.
 */
void *list_get_item(struct s_list_item *item);

#ifdef __cplusplus
}
#endif

#endif /* LIST_H */
