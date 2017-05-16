#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "queue.h"

struct s_queue {
        uint8_t *buf;  /**< Buffer              */
        uint32_t head; /**< Head position       */
        uint32_t tail; /**< Tail position       */
        uint32_t size; /**< Buffer size         */
};

static uint32_t queue_get_used(struct s_queue *queue);
static uint32_t queue_get_free(struct s_queue *queue);

void *queue_init(uint32_t size)
{
        uint32_t x;
        struct s_queue *ctx = NULL;

        if (size == 0) {
                errno = EINVAL;
                return NULL;
        }

        ctx = (struct s_queue *)malloc(sizeof(struct s_queue));

        if (ctx == NULL) {
                errno = ENOMEM;
                return NULL;
        }

        memset(ctx, 0, sizeof(struct s_queue));

        x = 1;
        while (x < size)
                x <<= 1;

        size = x;

        ctx->buf = (uint8_t *)malloc(size);
        ctx->head = 0;
        ctx->tail = 0;
        ctx->size = size;

        if (ctx->buf == NULL) {
                errno = ENOMEM;
                queue_release(ctx);
                return NULL;
        }

        return ctx;
}

void queue_release(void *queue)
{
        struct s_queue *q = (struct s_queue *)queue;
        if (queue == NULL)
                return;

        if (q->buf != NULL)
                free(q->buf);

        free(q);
}

int queue_write(void *queue, uint8_t *data, uint32_t size)
{
        struct s_queue *q = (struct s_queue *)queue;
        if (queue == NULL || data == NULL || size == 0) {
                errno = EINVAL;
                return -1;
        }

        if (queue_get_free(q) < size) {
                errno = EAGAIN;
                return -1;
        }

        if (q->head + size <= q->size) {
            memcpy(&q->buf[q->head], data, size);
        } else {
            uint32_t size1 = q->size - q->head;
            memcpy(&q->buf[q->head], data, size1);
            memcpy(q->buf, &data[size1], size - size1);
        }

        q->head = (q->head + size)&(q->size - 1);
        return (int)size;
}

int queue_read(void *queue, uint8_t *data, uint32_t size)
{
        struct s_queue *q = (struct s_queue *)queue;
        if (queue == NULL || data == NULL) {
                errno = EINVAL;
                return -1;
        }

        if (size == 0)
                return 0;

        if (queue_get_used(q) < size) {
                errno = EAGAIN;
                return -1;
        }

        if (q->tail + size <= q->size) {
            memcpy(data, &q->buf[q->tail], size);
        } else {
            uint32_t size1 = q->size - q->tail;
            memcpy(data, &q->buf[q->tail], size1);
            memcpy(&data[size1], q->buf, size - size1);
        }

        q->tail = (q->tail + size)&(q->size - 1);
        return (int)size;
}

static uint32_t queue_get_used(struct s_queue *queue)
{
        return (queue->size + queue->head - queue->tail)&(queue->size - 1);
}

static uint32_t queue_get_free(struct s_queue *queue)
{
        return queue->size - 1 - queue_get_used(queue);
}
