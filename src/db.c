#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "db.h"
#include "common.h"
#include "db_node.h"
#include "socket_operations.h"

struct s_db {
        void **key_nodes; /**< List of nodes for storing keys   */
        void **val_nodes; /**< List of nodes for storing values */
        uint32_t node_count;
};

static struct s_db *db = NULL;

int db_init(uint32_t node_count)
{
        int i = 0;
        if (node_count == 0) {
                errno = EINVAL;
                return -1;
        }

        db = (struct s_db *)malloc(sizeof(struct s_db));
        if (db == NULL) {
                errno = ENOMEM;
                goto exit_on_fail;
        }

        memset(db, 0, sizeof(struct s_db));

        db->node_count = node_count;
        db->key_nodes = (void **)malloc(sizeof(void *) * db->node_count);
        db->val_nodes = (void **)malloc(sizeof(void *) * db->node_count);

        if (db->key_nodes == NULL || db->val_nodes == NULL) {
                errno = ENOMEM;
                goto exit_on_fail;
        }

        memset(db->key_nodes, 0, sizeof(void *) * db->node_count);
        memset(db->val_nodes, 0, sizeof(void *) * db->node_count);

        for (i = 0; i < (int)db->node_count; i++) {
                char name[64];

                sprintf(name, "db_key_node_%d.txt", i);
                db->key_nodes[i] = db_node_init(name);

                sprintf(name, "db_val_node_%d.txt", i);
                db->val_nodes[i] = db_node_init(name);

                if (db->key_nodes[i] == NULL || db->val_nodes[i] == NULL) {
                        errno = ENOMEM;
                        goto exit_on_fail;
                }
        }
        return 0;

exit_on_fail:
        db_release();
        return -1;
}


void db_release(void)
{
        uint32_t i;

        if (db == NULL)
                return;
        if (db->key_nodes != NULL && db->val_nodes != NULL) {
                for (i = 0; i < db->node_count; i++) {
                        db_node_release(db->key_nodes[i]);
                        db_node_release(db->val_nodes[i]);
                }
        }

        if (db->key_nodes != NULL)
                free(db->key_nodes);

        if (db->val_nodes != NULL)
                free(db->val_nodes);

        free(db);
        db = NULL;
}

static uint32_t db_get_node_id(int max, uint8_t *data, uint32_t size)
{
        uint8_t first = data[0];
        uint8_t last  = (size > 2) ? data[size - 2] : first;

        return (first^last) % max;
}

static void db_send_response(struct s_message *msg, struct s_db_item *val_item)
{
        struct s_message resp;
        memset(&resp, 0, sizeof(resp));

        if (msg->sd < 0)
                return;

        resp.cmd.type = DB_CMD_RESP;
        resp.sd = msg->sd;
        resp.val = (val_item) ? val_item->data : NULL;
        resp.cmd.val_size = (val_item) ? val_item->size : 0;

        resp.cmd.len  = sizeof(resp.cmd);
        resp.cmd.len += resp.cmd.val_size;

        if (socket_write(&resp) != (int)resp.cmd.len)
                perror("Send response error");
}

static void db_get_value(struct s_message *msg, void *key_node)
{
        struct s_db_item *key_item = NULL;
        struct s_db_item *val_item = NULL;

        db_node_rdlock(key_node);

        key_item = db_node_get_item(key_node, msg->key, msg->cmd.key_size);

        if (key_item != NULL) {
                val_item = key_item->ref_item;
                db_send_response(msg, val_item);
        }

        db_node_unlock(key_node);
        free(msg->key);
        msg->key = NULL;

        db_send_response(msg, NULL);
}

static void db_get_all_values(struct s_db *db, struct s_message *msg)
{
        uint32_t i = 0;
        void *val_node = NULL;
        void *it = NULL;
        struct s_db_item *val_item = NULL;

        for (i = 0; i < db->node_count; i++) {
                val_node = db->val_nodes[i];

                db_node_rdlock(val_node);
                it = db_node_get_iterator(val_node);
                while (db_node_iterator_has_next(it)) {
                        val_item = db_node_get_next(val_node, it);
                        db_send_response(msg, val_item);
                }
                db_node_unlock(val_node);
        }
        db_send_response(msg, NULL);
}

static void db_put_value(struct s_db *db,
                         struct s_message *msg,
                         void *key_node,
                         void *val_node,
                         uint32_t val_node_id)
{
        struct s_db_item *key_item = NULL;
        struct s_db_item *val_item = NULL;
        struct s_command *cmd = &msg->cmd;

        int free_msg_key = 0;
        int free_msg_val = 0;

        db_node_wrlock(key_node);
        db_node_wrlock(val_node);

        key_item = db_node_get_item(key_node, msg->key, cmd->key_size);
        val_item = db_node_get_item(val_node, msg->val, cmd->val_size);

        /*
         * Three cases:
         * 1. Key and value exist.
         * 2. Key does NOT exist AND value does NOT exist.
         * 3. Key exists AND value does NOT exist. Key refer to old value.
         * 4. Key does NOT exist AND value exists.
         */

        if (key_item != NULL && val_item != NULL) {
                free_msg_key = 1;
                free_msg_val = 1;
        } else  if (key_item == NULL && val_item == NULL) {
                key_item = db_node_put_item(key_node, msg->key, cmd->key_size);
                val_item = db_node_put_item(val_node, msg->val, cmd->val_size);

                if (key_item != NULL && val_item != NULL) {
                        key_item->ref_item = val_item;
                        val_item->ref_counter = 1;

                        db_node_save(val_node, val_item, 0);
                        db_node_save(key_node, key_item, val_node_id);
                } else {

                        if (key_item != NULL)
                                db_node_remove_item(key_node, key_item);
                        else
                                free_msg_key = 1;

                        if (val_item != NULL)
                                db_node_remove_item(val_node, val_item);
                        else
                                free_msg_val = 1;
                }
        } else if (key_item != NULL && val_item == NULL) {
                struct s_db_item *cur_val_item = key_item->ref_item;
                uint32_t node_id = db_get_node_id(db->node_count,
                                                  cur_val_item->data,
                                                  cur_val_item->size);

                void *cur_val_node = db->val_nodes[node_id];
                int need_lock = (cur_val_node != val_node);

                val_item = db_node_put_item(val_node, msg->val, cmd->val_size);
                if (val_item) {
                        if (need_lock)
                                db_node_wrlock(cur_val_node);

                        if (cur_val_item->ref_counter > 1)
                                cur_val_item->ref_counter--;
                        else
                                db_node_remove_item(cur_val_node, cur_val_item);

                        if (need_lock)
                                db_node_unlock(cur_val_node);

                        key_item->ref_item = val_item;
                        val_item->ref_counter = 1;

                        db_node_save(val_node, val_item, 0);
                        db_node_update_ref(key_node, key_item, val_node_id);
                } else {
                        free_msg_val = 1;
                }
                free_msg_key = 1;
        } else if (key_item == NULL && val_item != NULL) {
                key_item = db_node_put_item(key_node, msg->key, cmd->key_size);
                if (key_item != NULL) {
                        key_item->ref_item = val_item;
                        val_item->ref_counter++;
                        db_node_save(key_node, key_item, val_node_id);
                } else {
                        free_msg_key = 1;
                }
                free_msg_val = 1;
        }

        db_node_unlock(val_node);
        db_node_unlock(key_node);

        db_send_response(msg, NULL);

        if (free_msg_key) {
                free(msg->key);
                msg->key = NULL;
        }

        if (free_msg_val) {
                free(msg->val);
                msg->val = NULL;
        }
}

static void db_erase_value(struct s_db *db,
                           struct s_message *msg,
                           void *key_node)
{
        struct s_command *cmd = &msg->cmd;
        struct s_db_item *key_item = NULL;
        struct s_db_item *val_item = NULL;
        void * val_node = NULL;

        db_node_wrlock(key_node);

        key_item = db_node_get_item(key_node, msg->key, cmd->key_size);
        if (key_item != NULL) {
                uint32_t node_id = 0;
                val_item = key_item->ref_item;
                if (val_item != NULL) {
                        node_id = db_get_node_id(db->node_count,
                                                 val_item->data,
                                                 val_item->size);
                        val_node = db->val_nodes[node_id];
                }
        }

        if (val_node != NULL) {
                db_node_wrlock(val_node);

                db_node_remove_item(key_node, key_item);
                if (val_item->ref_counter > 1)
                        val_item->ref_counter--;
                else
                        db_node_remove_item(val_node, val_item);

                db_node_unlock(val_node);
        }

        db_node_unlock(key_node);

        db_send_response(msg, NULL);

        free(msg->key);
        msg->key = NULL;
}

void db_process_message(struct s_message *msg)
{
        struct s_command *cmd = NULL;
        void *key_node = NULL;
        void *val_node = NULL;
        uint32_t val_node_id = 0;

        if (msg == NULL)
                return;

        cmd = &msg->cmd;

        if (cmd->type != DB_CMD_LIST) {
                uint32_t node_id = db_get_node_id(db->node_count,
                                                  msg->key,
                                                  cmd->key_size);
                key_node = db->key_nodes[node_id];

                if (cmd->type == DB_CMD_PUT) {
                        node_id = db_get_node_id(db->node_count,
                                                 msg->val,
                                                 cmd->val_size);
                        val_node = db->val_nodes[node_id];
                        val_node_id = node_id;
                }
        }

        switch(cmd->type) {
        case DB_CMD_GET:
                db_get_value(msg, key_node);
                break;
        case DB_CMD_PUT:
                db_put_value(db, msg, key_node, val_node, val_node_id);
                break;
        case DB_CMD_ERASE:
                db_erase_value(db, msg, key_node);
                break;
        case DB_CMD_LIST:
                db_get_all_values(db, msg);
                break;
        }
}
