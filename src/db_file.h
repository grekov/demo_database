#ifndef DB_FILE_H
#define DB_FILE_H

/**
 * @file db_file.h
 * @author Sviatoslav
 * @brief Database file manager.
 *
 * Provide access to the one file.
 * Manage free space (lacune) after removing data.
 *
 * Each data in the file starts with 4 byte with total length.
 * If space not used, then one bit in lenght used for mark it.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize DB file.
 * @param file_name Unique file name.
 * @return On success returns not NULL pointer,
 * otherwise returns NULL and set errno.
 */
void *db_file_init(const char *file_name);

/**
 * @brief Release all resources.
 * @param db_file DB file.
 */
void db_file_release(void *db_file);

/**
 * @brief Get start offset of free space with given size.
 * @param db_file DB file.
 * @param size Requested size.
 * @return File offset.
 */
uint32_t db_file_get_space(void *db_file, uint32_t size);

/**
 * @brief Put unused space.
 * @param db_file DB file.
 * @param offset Offset of unused space.
 * @param size Size of unused space.
 */
void db_file_put_space(void *db_file, uint32_t offset, uint32_t size);

/**
 * @brief Write data to the file.
 * @param db_file DB file.
 * @param offset Write offset.
 * @param data Data.
 * @param size Size of data.
 * @return On success, the number of bytes written is returned.
 * On error, -1 is returned, and errno is set.
 */
int db_file_write_data(void *db_file,
                       uint32_t offset,
                       uint8_t *data,
                       uint32_t size);

#ifdef __cplusplus
}
#endif

#endif /* DB_FILE_H */
