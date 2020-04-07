

















#include "allegro5/allegro.h"



static ALLEGRO_MEMORY_INTERFACE *mem = NULL;





void al_set_memory_interface(ALLEGRO_MEMORY_INTERFACE *memory_interface)
{
mem = memory_interface;
}





void *al_malloc_with_context(size_t n,
int line, const char *file, const char *func)
{
if (mem)
return mem->mi_malloc(n, line, file, func);
else
return malloc(n);
}





void al_free_with_context(void *ptr,
int line, const char *file, const char *func)
{
if (mem)
mem->mi_free(ptr, line, file, func);
else
free(ptr);
}





void *al_realloc_with_context(void *ptr, size_t n,
int line, const char *file, const char *func)
{
if (mem)
return mem->mi_realloc(ptr, n, line, file, func);
else
return realloc(ptr, n);
}





void *al_calloc_with_context(size_t count, size_t n,
int line, const char *file, const char *func)
{
if (mem)
return mem->mi_calloc(count, n, line, file, func);
else
return calloc(count, n);
}



