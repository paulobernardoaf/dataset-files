#pragma once
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#define COROUTINE __declspec(noreturn) void
enum {
COROUTINE_REGISTERS = 8,
COROUTINE_XMM_REGISTERS = 1+10*2,
};
struct coroutine_context
{
void **stack_pointer;
};
typedef void(* coroutine_start)(struct coroutine_context *from, struct coroutine_context *self);
void coroutine_trampoline();
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
context->stack_pointer -= 4;
*--context->stack_pointer = 0;
*--context->stack_pointer = (void*)start;
*--context->stack_pointer = (void*)coroutine_trampoline;
*--context->stack_pointer = (void*)top; 
*--context->stack_pointer = (void*)stack; 
context->stack_pointer -= COROUTINE_REGISTERS;
memset(context->stack_pointer, 0, sizeof(void*) * COROUTINE_REGISTERS);
memset(context->stack_pointer - COROUTINE_XMM_REGISTERS, 0, sizeof(void*) * COROUTINE_XMM_REGISTERS);
}
struct coroutine_context * coroutine_transfer(struct coroutine_context * current, struct coroutine_context * target);
static inline void coroutine_destroy(struct coroutine_context * context)
{
}
