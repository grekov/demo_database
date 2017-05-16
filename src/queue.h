#ifndef QUEUE_H
#define QUEUE_H

/**
 * @file queue.h
 * @author Sviatoslav
 * @brief Round buffer (FIFO).
 *
 * This queue without locks intended for using
 * with one writer thread and one reader thread.
 * Writer move head, reader move tail.
 *
 * For best performance queue size must be power of 2.
 * Max usufull space equal (size - 1)
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize queue.
 * @param size Queue size in bytes.
 * @return On success, pointer to the queue,
 * otherwise NULL is returned and set errno.
 */
void *queue_init(uint32_t size);

/**
 * @brief Release queue memory.
 * @param queue Pointer to the queue.
 */
void queue_release(void *queue);

/**
 * @brief Writes data to the queue.
 * @param queue Queue.
 * @param data  Pointer to the data.
 * @param size  Size of data.
 * @return On success, the number of bytes written is returned.
 * On error, -1 is returned, and errno is set.
 */
int queue_write(void *queue, uint8_t *data, uint32_t size);

/**
 * @brief Reads data from the queue.
 * @param queue Queue.
 * @param data Pointer to the data.
 * @param size Size to be read.
 * @return On success, the number of bytes read is returned.
 * On error, -1 is returned, and errno is set.
 */
int queue_read(void *queue, uint8_t *data, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif /* QUEUE_H */
