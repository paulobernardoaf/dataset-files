






#pragma once

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define COROUTINE __attribute__((noreturn)) void
#define COROUTINE_LIMITED_ADDRESS_SPACE

enum {COROUTINE_REGISTERS = 8};

struct coroutine_context
{
void **stack_pointer;
};

typedef COROUTINE(* coroutine_start)(struct coroutine_context *from, struct coroutine_context *self);

static inline void coroutine_initialize_main(struct coroutine_context * context) {
context->stack_pointer = NULL;
}

static inline void coroutine_initialize(
struct coroutine_context *context,
coroutine_start start,
void *stack,
size_t size
) {
assert(start && stack && size >= 1024);


char * top = (char*)stack + size;
context->stack_pointer = (void**)((uintptr_t)top & ~0xF);

*--context->stack_pointer = (void*)start;

context->stack_pointer -= COROUTINE_REGISTERS;
memset(context->stack_pointer, 0, sizeof(void*) * COROUTINE_REGISTERS);
}

struct coroutine_context * coroutine_transfer(struct coroutine_context * current, struct coroutine_context * target);

static inline void coroutine_destroy(struct coroutine_context * context)
{
}
