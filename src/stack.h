#ifndef STACK_H
#define STACK_H

/**
 * @file stack.h
 * @author Sviatoslav
 * @brief Simple stack with fixed capacity.
 *
 * Stack pop and push pointers to data structures.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize stack.
 * @param items_count   Items count.
 * @param item_size     Size of one item.
 * @return On success returns pointer to the stack,
 * otherwise returns NULL and set errno.
 */
void *stack_init(uint32_t items_count, uint32_t item_size);

/**
 * @brief Release stack memory.
 * @param stack Pointer to the stack.
 */
void stack_release(void *stack);

/**
 * @brief Removes the top item from the stack and return it.
 * @param stack Pointer to the stack.
 * @return Pointer to tho top item, if stack not empty, otherwise - NULL.
 */
void *stack_pop(void *stack);

/**
 * @brief Adds item to the top of the stack.
 * @param stack Pointer to the stack.
 * @param item Pointer to the item.
 */
void stack_push(void *stack, void *item);

#ifdef __cplusplus
}
#endif

#endif /* STACK_H */
