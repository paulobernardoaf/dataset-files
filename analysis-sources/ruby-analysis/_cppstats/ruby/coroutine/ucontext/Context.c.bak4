









#if defined(__sun) && !defined(__EXTENSIONS__)
#define __EXTENSIONS__
#endif
#include "Context.h"

void coroutine_trampoline(void * _start, void * _context)
{
coroutine_start start = (coroutine_start)_start;
struct coroutine_context * context = _context;

start(context->from, context);
}
