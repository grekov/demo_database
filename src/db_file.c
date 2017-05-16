#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <errno.h>


#include "db_file.h"
#include "avl.h"
#include "list.h"

#define DB_FILE_MAX_NAME_LEN    64
#define DB_FILE_EMPTY_BLOCK_BIT 0x10000000

/**
 * @brief Structure for build index of free space by size.
 */
struct db_file_space {
        uint32_t size;  /**< Size of free space */
        struct s_list blocks; /**< List of blocks with same free space size */
};

/**
 * @brief Structre for build index of free space by offset.
 */
struct db_file_block {
        uint32_t offset;        /**< Start offset of free space */
        uint32_t size;          /**< Size of free space         */
        struct s_list_item blocks_item; /**< Item of db_file_space::blocks */
        struct db_file_space *f_space;  /**< Pointer to db_file_space */
};

struct db_file {
        struct avl_table *begin_block_table;/**< Block index by left edge  */
        struct avl_table *end_block_table;  /**< Block index by right edge */
        struct avl_table *free_space_table; /**< Block index by space      */

        char file_name[DB_FILE_MAX_NAME_LEN];
        int fd;                         /**< File decriptor             */
        uint32_t last_offset;           /**< Most of issued offset      */
};

static int avl_begin_block_cmp(const void *avl_a, const void *avl_b, void *avl_param)
{
        (void)avl_param;
        struct db_file_block *item1 = (struct db_file_block *)avl_a;
        struct db_file_block *item2 = (struct db_file_block *)avl_b;

        if (item1->offset < item2->offset) return -1;
        if (item1->offset > item2->offset) return  1;

        return 0;
}

static int avl_end_block_cmp(const void *avl_a, const void *avl_b, void *avl_param)
{
        (void)avl_param;
        struct db_file_block *item1 = (struct db_file_block *)avl_a;
        struct db_file_block *item2 = (struct db_file_block *)avl_b;
        uint32_t off1 = item1->offset + item1->size;
        uint32_t off2 = item2->offset + item2->size;

        if (off1 < off2) return -1;
        if (off1 > off2) return  1;

        return 0;
}

static int avl_space_cmp(const void *avl_a, const void *avl_b, void *avl_param)
{
        (void)avl_param;
        struct db_file_space *item1 = (struct db_file_space *)avl_a;
        struct db_file_space *item2 = (struct db_file_space *)avl_b;

        if (item1->size < item2->size) return -1;
        if (item1->size > item2->size) return  1;

        return 0;
}

static void avl_free_block_item(void *avl_item, void *avl_param)
{
        (void)avl_param;
        struct db_file_block *block = (struct db_file_block *)avl_item;

        if (block)
                free(block);
}

static void avl_free_space_item(void *avl_item, void *avl_param)
{
        (void)avl_param;
        struct db_file_space *space = (struct db_file_space *)avl_item;

        if (space)
                free(space);
}

void *db_file_init(const char *file_name)
{
        struct db_file *db_f = NULL;

        if (file_name == NULL) {
                errno = EINVAL;
                return NULL;
        }

        db_f = (struct db_file *)malloc(sizeof(struct db_file));
        if (db_f == NULL) {
                errno = ENOMEM;
                printf("%s: DB file allocate memory error\n", __FUNCTION__);
                return NULL;
        }

        memset(db_f, 0, sizeof(struct db_file));

        strncpy(db_f->file_name, file_name, DB_FILE_MAX_NAME_LEN);

        db_f->fd = open(db_f->file_name, O_RDWR | O_CREAT, 0640);
        if (db_f->fd == -1) {
                perror("DB open file error");
                goto exit_on_fail;
        }

        db_f->begin_block_table = avl_create(avl_begin_block_cmp, NULL, NULL);
        db_f->end_block_table   = avl_create(avl_end_block_cmp, NULL, NULL);
        db_f->free_space_table  = avl_create(avl_space_cmp, NULL, NULL);

        if (db_f->begin_block_table == NULL ||
                        db_f->end_block_table == NULL ||
                        db_f->free_space_table == NULL) {
                errno = ENOMEM;
                printf("%s: Cannot create AVL table\n", __FUNCTION__);
                goto exit_on_fail;
        }

        return db_f;

exit_on_fail:
        db_file_release(db_f);
        return NULL;
}

void db_file_release(void *db_file)
{
        struct db_file *db_f = (struct db_file *)db_file;
        if (db_f == NULL)
                return;

        if (db_f->fd >= 0) {
                close(db_f->fd);
                unlink(db_f->file_name);
        }

        if (db_f->begin_block_table != NULL)
                avl_destroy(db_f->begin_block_table, avl_free_block_item);

        if (db_f->end_block_table != NULL)
                avl_destroy(db_f->end_block_table, NULL);

        if (db_f->free_space_table != NULL)
                avl_destroy(db_f->free_space_table, avl_free_space_item);


        free(db_f);
}

static int db_file_add_block(struct db_file *db_f,
                             struct db_file_block *block)
{
        struct db_file_space space;
        struct db_file_space *f_space = NULL;
        uint32_t size = 0;

        space.size = block->size;

        f_space = (struct db_file_space *)
                        avl_find(db_f->free_space_table, &space);
        if (f_space == NULL) {
                f_space = (struct db_file_space *)
                                malloc(sizeof(struct db_file_space));
                if (f_space == NULL)
                        return -1;

                memset(f_space, 0, sizeof(struct db_file_space));

                f_space->size = block->size;
                if (avl_probe(db_f->free_space_table, f_space) == NULL) {
                        free(f_space);
                        return -1;
                }

        }

        list_append(&f_space->blocks, &block->blocks_item);
        block->f_space = f_space;

        avl_probe(db_f->begin_block_table, block);
        avl_probe(db_f->end_block_table, block);

        size = block->size;
        size |= DB_FILE_EMPTY_BLOCK_BIT;
        size = htonl(size);
        pwrite(db_f->fd, (uint8_t *)&size, sizeof(uint32_t), block->offset);

        return 0;
}

static void db_file_remove_block(struct db_file *db_f,
                                 struct db_file_block *block)
{
        struct db_file_space *f_space = block->f_space;

        avl_delete(db_f->begin_block_table, block);
        avl_delete(db_f->end_block_table, block);

        list_remove(&f_space->blocks, &block->blocks_item);
        if (list_get_item(f_space->blocks.first) == NULL) {
                avl_delete(db_f->free_space_table, f_space);
                free(f_space);
                f_space = NULL;
        }
}

static struct db_file_block * db_file_get_block(struct db_file *db_f,
                                                struct db_file_space *f_space)
{
        struct db_file_block *block = NULL;

        block = (struct db_file_block *)list_get_item(f_space->blocks.first);
        db_file_remove_block(db_f, block);

        return block;
}


uint32_t db_file_get_space(void *db_file, uint32_t size)
{
        struct avl_traverser trav;
        struct db_file_space space;
        struct db_file_space *f_space = NULL;
        struct db_file_block *block = NULL;
        struct db_file *db_f = (struct db_file *)db_file;
        uint32_t offset = 0;
        if (db_f == NULL)
                return -1;

        space.size = size;

        avl_t_init(&trav, db_f->free_space_table);
        f_space = (struct db_file_space *)
                        avl_t_find_near(&trav, db_f->free_space_table, &space);

        if (f_space && f_space->size < size)
                f_space = (struct db_file_space *)avl_t_next(&trav);

        if (f_space && f_space->size >= size) {
                block = db_file_get_block(db_f, f_space);
                f_space = NULL;

                offset = block->offset;
                if (block->size > size) {
                        block->offset += size;
                        block->size   -= size;

                        db_file_add_block(db_f, block);
                } else {
                        free(block);
                }

                return offset;
        }

        offset = db_f->last_offset;
        db_f->last_offset += size;

        return offset;
}

void db_file_put_space(void *db_file, uint32_t offset, uint32_t size)
{
        struct db_file_block tmp_block;
        struct db_file_block *block = NULL;
        struct db_file *db_f = (struct db_file *)db_file;
        if (db_f == NULL)
                return;

        tmp_block.size = 0;

        /*
         * 1. Search block with start_off = offset + size;
         * 2. Search block with end_off = offset.
         */
        tmp_block.offset = offset + size;
        block = (struct db_file_block *)
                        avl_find(db_f->begin_block_table, &tmp_block);
        if (block == NULL) {
                tmp_block.offset = offset;
                block = (struct db_file_block *)
                                avl_find(db_f->end_block_table, &tmp_block);
        }

        if (block != NULL) {
                db_file_remove_block(db_f, block);
                block->size += size;
                if (block->offset > offset)
                        block->offset = offset;
        } else {
                block = (struct db_file_block *)
                                malloc(sizeof(struct db_file_block));
                if (block == NULL)
                        return;

                memset(block, 0, sizeof(struct db_file_block));

                block->blocks_item.item = block;
                block->size   = size;
                block->offset = offset;
        }

        if (block->offset + block->size == db_f->last_offset) {
                db_f->last_offset -= block->size;
                ftruncate(db_f->fd, db_f->last_offset);
                free(block);
                return;
        } else {
                db_file_add_block(db_f, block);
        }
}


int db_file_write_data(void *db_file,
                       uint32_t offset,
                       uint8_t *data,
                       uint32_t size)
{
        struct db_file *db_f = (struct db_file *)db_file;
        if (db_file == NULL || data == NULL || size == 0)
                return -1;

        return (int)pwrite(db_f->fd, data, size, (off_t)offset);
}
