#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <errno.h>

#include <arpa/inet.h>

#include "db_node.h"
#include "db_file.h"
#include "list.h"
#include "avl.h"

struct s_db_node_iterator {
        struct s_db_item *current;
};

struct s_db_node {
        void * db_file; /**< Pointer to DB file */
        struct s_db_node_iterator iterator; /**< Items iterator */
        struct avl_table * table; /**< Table contains all items */
        struct s_list      list;  /**< List for itarate all items */
        pthread_rwlock_t   rw_lock;
};

int avl_compare(const void *avl_a, const void *avl_b, void *avl_param)
{
        (void)avl_param;
        int i = 0;
        struct s_db_item *item1 = (struct s_db_item *)avl_a;
        struct s_db_item *item2 = (struct s_db_item *)avl_b;

        if (item1->size < item2->size) return -1;
        if (item1->size > item2->size) return  1;

        while (i < item1->size) {

                if (i + (int)sizeof(uint64_t) <= item1->size){
                        uint64_t v1 = *((uint64_t *)&item1->data[i]);
                        uint64_t v2 = *((uint64_t *)&item2->data[i]);

                        if (v1 < v2) return -1;
                        if (v1 > v2) return  1;
                } else {
                        int count = item1->size - i;
                        return memcmp(&item1->data[i], &item2->data[i], count);
                }

                i += sizeof(uint64_t);
        }

        return 0;
}

static void avl_free_item(void *avl_item, void *avl_param)
{
        (void)avl_param;
        struct s_db_item *item = (struct s_db_item *)avl_item;

        if (item && item->data != NULL) {
                free(item->data);
                item->data = NULL;
        }

        if (item)
                free(item);
}

void *db_node_init(const char *node_name)
{
        struct s_db_node *db_node = (struct s_db_node *)
                        malloc(sizeof(struct s_db_node));
        if (db_node == NULL) {
                printf("%s: DB node allocation memory error.\n", __FUNCTION__);
                return NULL;
        }

        memset(db_node, 0, sizeof(struct s_db_node));

        db_node->db_file = db_file_init(node_name);
        if (db_node->db_file == NULL)
                goto exit_on_fail;


        db_node->table = avl_create(avl_compare, NULL, NULL);

        if (db_node->table == NULL) {
                errno = ENOMEM;
                printf("%s:Cannot create AVL table\n", __FUNCTION__);
                goto exit_on_fail;
        }

        pthread_rwlock_init(&db_node->rw_lock, NULL);

        return db_node;

exit_on_fail:
        db_node_release(db_node);
        return NULL;
}

void db_node_release(void *node)
{
        struct s_db_node *db_node = (struct s_db_node *)node;
        if (db_node == NULL)
                return;

        if (db_node->db_file != NULL)
                db_file_release(db_node->db_file);


        if (db_node->table != NULL)
                avl_destroy(db_node->table, avl_free_item);

        free(db_node);
}

void db_node_rdlock(void *node)
{
        struct s_db_node *db_node = (struct s_db_node *)node;
        if (db_node == NULL)
                return;

        pthread_rwlock_rdlock(&db_node->rw_lock);
}

void db_node_wrlock(void *node)
{
        struct s_db_node *db_node = (struct s_db_node *)node;
        if (db_node == NULL)
                return;

        pthread_rwlock_wrlock(&db_node->rw_lock);
}


void db_node_unlock(void *node)
{
        struct s_db_node *db_node = (struct s_db_node *)node;
        if (db_node == NULL)
                return;

        pthread_rwlock_unlock(&db_node->rw_lock);
}

struct s_db_item *db_node_get_item(void *node, uint8_t *data, int size)
{
        struct s_db_item item;
        struct s_db_node *db_node = (struct s_db_node *)node;
        if (db_node == NULL || data == NULL || size == 0)
                return NULL;

        item.data = data;
        item.size = size;
        return (struct s_db_item *)avl_find(db_node->table, &item);
}

struct s_db_item *db_node_put_item(void *node, uint8_t *data, int size)
{
        struct s_db_item *db_item = NULL;
        struct s_db_node *db_node = (struct s_db_node *)node;
        if (db_node == NULL || data == NULL || size == 0)
                return NULL;

        db_item = (struct s_db_item *)malloc(sizeof(struct s_db_item));
        if (db_item == NULL)
                return NULL;

        memset(db_item, 0, sizeof(struct s_db_item));
        db_item->data = data;
        db_item->size = size;
        db_item->list_item.item = db_item;

        if (avl_probe(db_node->table, db_item) != NULL) {
                list_append(&db_node->list, &db_item->list_item);
                return db_item;
        }

        return NULL;
}

int db_node_remove_item(void *node, struct s_db_item *item)
{
        struct s_db_node *db_node = (struct s_db_node *)node;
        if (db_node == NULL || item == NULL)
                return -1;

        item = (struct s_db_item *)avl_delete(db_node->table, item);
        if (item != NULL) {
                if (item->f_size) {
                        void *db_f = db_node->db_file;
                        db_file_put_space(db_f, item->f_offset, item->f_size);
                }
                list_remove(&db_node->list, &item->list_item);
                free(item->data);
                free(item);
                return 0;
        }

        return -1;
}

void *db_node_get_iterator(void *node)
{
        struct s_db_node *db_node = (struct s_db_node *)node;
        struct s_db_node_iterator *it = NULL;

        if (db_node == NULL)
                return NULL;

        it = &db_node->iterator;
        it->current = (struct s_db_item *)list_get_item(db_node->list.first);
        return it;
}


int db_node_iterator_has_next(void *iterator)
{
        struct s_db_node_iterator *it = (struct s_db_node_iterator *)iterator;

        if (it == NULL)
                return 0;

        return (it->current != NULL) ? 1 : 0;
}

struct s_db_item *db_node_get_next(void *node, void *iterator)
{
        struct s_db_item *item = NULL;
        struct s_db_node *db_node = (struct s_db_node *)node;
        struct s_db_node_iterator *it = (struct s_db_node_iterator *)iterator;

        if (db_node == NULL || it == NULL)
                return NULL;

        item = it->current;
        it->current = (struct s_db_item *)list_get_item(item->list_item.next);
        return item;
}


void db_node_update_ref(void *node,
                        struct s_db_item *item,
                        uint32_t ref_node_id)
{
        void *db_f = NULL;
        struct s_db_node *db_node = (struct s_db_node *)node;
        struct s_db_item *val_item = NULL;
        uint32_t offset = 0;
        uint32_t val_n = 0;
        if (node == NULL || item == NULL)
                return;

        db_f = db_node->db_file;
        val_item = item->ref_item;

        if (val_item == NULL || item->f_size == 0)
                return;

        /* Skip len field */
        offset = item->f_offset + sizeof(uint32_t);

        val_n = htonl(ref_node_id);
        db_file_write_data(db_f, offset,
                           (uint8_t *)&val_n,
                           sizeof(uint32_t));
        offset += sizeof(uint32_t);

        val_n = htonl(val_item->f_offset);
        db_file_write_data(db_f, offset,
                           (uint8_t *)&val_n,
                           sizeof(uint32_t));
}

void db_node_save(void *node, struct s_db_item *item, uint32_t ref_node_id)
{
        void *db_f = NULL;
        struct s_db_node *db_node = (struct s_db_node *)node;
        struct s_db_item *val_item = NULL;
        uint32_t offset = 0;
        uint32_t val_n;
        if (node == NULL || item == NULL)
                return;

        db_f = db_node->db_file;
        val_item = item->ref_item;

        item->f_size  = sizeof(uint32_t);       /* total length */
        if (val_item) {
                item->f_size += sizeof(uint32_t);/* value node id */
                item->f_size += sizeof(uint32_t);/* value offset for this key */
        }
        item->f_size += item->size;             /* key length   */
        item->f_offset = db_file_get_space(db_f, item->f_size);

        offset = item->f_offset;

        val_n = htonl(item->f_size);
        db_file_write_data(db_f, offset, (uint8_t *)&val_n, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        if (val_item) {
                val_n = htonl(ref_node_id);
                db_file_write_data(db_f, offset,
                                   (uint8_t *)&val_n,
                                   sizeof(uint32_t));
                offset += sizeof(uint32_t);

                val_n = htonl(val_item->f_offset);
                db_file_write_data(db_f, offset,
                                   (uint8_t *)&val_n,
                                   sizeof(uint32_t));
                offset += sizeof(uint32_t);
        }

        db_file_write_data(db_f, offset, item->data, item->size);
}

