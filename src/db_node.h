#ifndef DB_NODE_H
#define DB_NODE_H

/**
 * @file db_node.h
 * @author Sviatoslav
 * @brief Database node.
 *
 * Provide access to the one table.
 */

#include <stdint.h>
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Table item struct.
 * ref_counter is a count reference to this item.
 * ref_item is a reference to another s_db_item.
 *
 * For example, key item refer to the value item.
 * Value item ref_counter not zero, ref_item of key item not NULL.
 *
 */
struct s_db_item {
        uint8_t *data;  /**< Item data          */
        int size;       /**< Size of item data  */
        int ref_counter;/**< Reference counter  */
        uint32_t f_offset; /**< Offset in file  */
        uint32_t f_size;   /**< Used space size in file */
        struct s_db_item  *ref_item;    /**< Reference to another item  */
        struct s_list_item list_item;
};

/**
 * @brief Initialize DB node.
 * @param node_name Unique node name.
 * @return On success returns not NULL pointer,
 * otherwise returns NULL and set errno.
 */
void * db_node_init(const char * node_name);

/**
 * @brief Release node resources.
 * @param node DB node.
 */
void db_node_release(void *node);

/**
 * @brief Lock db node with for read.
 * @param node DB node.
 */
void db_node_rdlock(void *node);

/**
 * @brief Lock DB node for write.
 * @param node DB node.
 */
void db_node_wrlock(void *node);

/**
 * @brief Unlock DB node.
 * @param node DB node.
 */
void db_node_unlock(void *node);

/**
 * @brief Get node item with the same data and size.
 * @param node DB node.
 * @param data Data.
 * @param size Size of data.
 * @return Pointer to the item, if it exists in node, otherwise - NULL.
 */
struct s_db_item *db_node_get_item(void *node, uint8_t *data, int size);

/**
 * @brief Put data to the node.
 * Data must be allocated by malloc. Will be free() on release.
 * @param node DB node.
 * @param data Data.
 * @param size Data size.
 * @return On success, return pointer to the item, otherwise - NULL.
 */
struct s_db_item *db_node_put_item(void *node, uint8_t *data, int size);

/**
 * @brief Remove item from node and free item and data memory.
 * @param node DB node.
 * @param item Item for remove.
 * @return On success, return zero, otherwise -1 is returned.
 */
int db_node_remove_item(void *node, struct s_db_item *item);

/**
 * @brief Get DB node iterator.
 * Provides iteration of all items in node.
 * @param node DB node.
 * @return Pointer to the DB node iterator.
 */
void *db_node_get_iterator(void *node);

/**
 * @brief Check, if iterator has next item.
 * @param iterator Pointer to the iterator.
 * @return Non-zero value is returned if has next, otherwize return 0.
 */
int db_node_iterator_has_next(void *iterator);

/**
 * @brief Get next item.
 * @param node DB node.
 * @param iterator Iterator.
 * @return Next DB item.
 */
struct s_db_item *db_node_get_next(void *node, void *iterator);

/**
 * @brief Update in the file only reference info for given item.
 * @param node DB node.
 * @param item Item.
 * @param ref_node_id Node id of the ref_item, if it not NULL.
 */
void db_node_update_ref(void *node,
                        struct s_db_item *item,
                        uint32_t ref_node_id);

/**
 * @brief Save item to the file.
 * @param node DB node.
 * @param item Item.
 * @param ref_node_id Node id of the ref_item, if it not NULL.
 */
void db_node_save(void *node, struct s_db_item *item, uint32_t ref_node_id);

#ifdef __cplusplus
}
#endif

#endif /* DB_NODE_H */
