#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "stack.h"

struct s_stack {
        uint8_t * list;
        void **   stack;
        uint32_t  listSize;  /**< Full size             */
        uint32_t  stackSize; /**< Curent stack size     */
};

void *stack_init(uint32_t items_count, uint32_t item_size)
{
        struct s_stack *ctx = NULL;
        uint32_t i;

        if (items_count == 0 || item_size == 0) {
                errno = EINVAL;
                return NULL;
        }

        ctx = (struct s_stack *)malloc(sizeof(struct s_stack));

        if (ctx == NULL) {
                errno = ENOMEM;
                return NULL;
        }

        memset(ctx, 0, sizeof(struct s_stack));

        ctx->list  = (uint8_t *)malloc(item_size * items_count);
        ctx->stack = (void **)malloc(sizeof(void *) * items_count);
        ctx->listSize  = items_count;
        ctx->stackSize = items_count;

        if (ctx->list == NULL || ctx->stack == NULL) {
                errno = ENOMEM;
                stack_release (ctx);
                return NULL;
        }

        memset(ctx->list,  0, (item_size * items_count));
        memset(ctx->stack, 0, sizeof(void *) * items_count);

        for (i = 0; i < items_count; i++)
                ctx->stack[i] = &ctx->list[(i * item_size)];

        return ctx;
}

void stack_release(void *stack)
{
        struct s_stack *ctx = (struct s_stack *)stack;

        if (ctx == NULL)
                return;

        if (ctx->list != NULL)
                free(ctx->list);

        if (ctx->stack != NULL)
                free(ctx->stack);

        free(ctx);
}

void *stack_pop(void *stack)
{
        struct s_stack *ctx = (struct s_stack *)stack;

        if (ctx == NULL)
                return NULL;

        if (ctx->stackSize > 0) {
                ctx->stackSize--;
                return ctx->stack[ctx->stackSize];
        }

        return NULL;
}

void stack_push(void *stack, void *item)
{
        struct s_stack *ctx = (struct s_stack *)stack;

        if (ctx == NULL || item == NULL)
                return;

        if (ctx->stackSize < ctx->listSize) {
                ctx->stack[ctx->stackSize] = item;
                ctx->stackSize++;
        }
}


