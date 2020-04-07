






#include "Context.h"


#if !defined(__GNUC__)
#define __asm__ asm
#endif

#if defined(__sparc)
__attribute__((noinline))

static void coroutine_flush_register_windows() {
__asm__
#if defined(__GNUC__)
__volatile__
#endif
#if defined(__sparcv9) || defined(__sparc_v9__) || defined(__arch64__)
#if defined(__GNUC__)
("flushw" : : : "%o7")
#else
("flushw")
#endif
#else
("ta 0x03")
#endif
;
}
#else
static void coroutine_flush_register_windows() {}
#endif

int coroutine_save_stack(struct coroutine_context * context) {
void *stack_pointer = &stack_pointer;

assert(context->stack);
assert(context->base);


coroutine_flush_register_windows();


if (stack_pointer < context->base) {
size_t size = (char*)context->base - (char*)stack_pointer;
assert(size <= context->size);

memcpy(context->stack, stack_pointer, size);
context->used = size;
} else {
size_t size = (char*)stack_pointer - (char*)context->base;
assert(size <= context->size);

memcpy(context->stack, context->base, size);
context->used = size;
}


return _setjmp(context->state);
}

__attribute__((noreturn, noinline))
static void coroutine_restore_stack_padded(struct coroutine_context *context, void * buffer) {
void *stack_pointer = &stack_pointer;

assert(context->base);


if (stack_pointer < context->base) {
void * bottom = (char*)context->base - context->used;
assert(bottom > stack_pointer);

memcpy(bottom, context->stack, context->used);
} else {
void * top = (char*)context->base + context->used;
assert(top < stack_pointer);

memcpy(context->base, context->stack, context->used);
}



_longjmp(context->state, 1 | (int)buffer);
}

static const size_t GAP = 128;




__attribute__((noreturn))
void coroutine_restore_stack(struct coroutine_context *context) {
void *stack_pointer = &stack_pointer;
void *buffer = NULL;
ssize_t offset = 0;


if (stack_pointer < context->base) {
offset = (char*)stack_pointer - ((char*)context->base - context->used) + GAP;
if (offset > 0) buffer = alloca(offset);
} else {
offset = ((char*)context->base + context->used) - (char*)stack_pointer + GAP;
if (offset > 0) buffer = alloca(offset);
}

assert(context->used > 0);

coroutine_restore_stack_padded(context, buffer);
}

struct coroutine_context *coroutine_transfer(struct coroutine_context *current, struct coroutine_context *target)
{
struct coroutine_context *previous = target->from;


assert(current->base == target->base);



target->from = current;

assert(current != target);



if (current->stack) {
if (coroutine_save_stack(current) == 0) {
coroutine_restore_stack(target);
}
} else {
coroutine_restore_stack(target);
}

target->from = previous;

return target;
}
