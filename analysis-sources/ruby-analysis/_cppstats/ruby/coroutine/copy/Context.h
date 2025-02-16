#pragma once
#include <assert.h>
#include <stddef.h>
#include <setjmp.h>
#include <string.h>
#include <stdlib.h>
#if !defined(__OpenBSD__)
#include <alloca.h>
#endif
#define COROUTINE __attribute__((noreturn)) void
#if INTPTR_MAX <= INT32_MAX
#define COROUTINE_LIMITED_ADDRESS_SPACE
#endif
#define COROUTINE_PRIVATE_STACK
struct coroutine_context
{
void *stack;
size_t size, used;
void *base;
jmp_buf state;
struct coroutine_context *from;
};
typedef COROUTINE(*coroutine_start)(struct coroutine_context *from, struct coroutine_context *self);
int coroutine_save_stack(struct coroutine_context * context);
COROUTINE coroutine_restore_stack(struct coroutine_context *context);
static inline void coroutine_initialize_main(struct coroutine_context *context, void *stack, size_t size, void *base) {
assert(stack);
assert(size >= 1024);
context->stack = stack;
context->size = size;
context->used = 0;
assert(base);
context->base = base;
context->from = NULL;
}
static inline void coroutine_initialize(
struct coroutine_context *context,
coroutine_start start,
void *stack,
size_t size,
void *base
) {
assert(start);
coroutine_initialize_main(context, stack, size, base);
if (coroutine_save_stack(context)) {
start(context->from, context);
}
}
struct coroutine_context *coroutine_transfer(struct coroutine_context *current, register struct coroutine_context *target);
static inline void coroutine_destroy(struct coroutine_context *context)
{
context->stack = NULL;
context->size = 0;
context->from = NULL;
}
