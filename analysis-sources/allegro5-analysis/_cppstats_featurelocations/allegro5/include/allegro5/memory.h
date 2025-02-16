














#if !defined(__al_included_allegro5_memory_h)
#define __al_included_allegro5_memory_h

#include "allegro5/base.h"

#if defined(__cplusplus)
extern "C" {
#endif




typedef struct ALLEGRO_MEMORY_INTERFACE ALLEGRO_MEMORY_INTERFACE;

struct ALLEGRO_MEMORY_INTERFACE {
void *(*mi_malloc)(size_t n, int line, const char *file, const char *func);
void (*mi_free)(void *ptr, int line, const char *file, const char *func);
void *(*mi_realloc)(void *ptr, size_t n, int line, const char *file, const char *func);
void *(*mi_calloc)(size_t count, size_t n, int line, const char *file, const char *func);
};

AL_FUNC(void, al_set_memory_interface, (ALLEGRO_MEMORY_INTERFACE *iface));




#define al_malloc(n) (al_malloc_with_context((n), __LINE__, __FILE__, __func__))




#define al_free(p) (al_free_with_context((p), __LINE__, __FILE__, __func__))




#define al_realloc(p, n) (al_realloc_with_context((p), (n), __LINE__, __FILE__, __func__))




#define al_calloc(c, n) (al_calloc_with_context((c), (n), __LINE__, __FILE__, __func__))



AL_FUNC(void *, al_malloc_with_context, (size_t n,
int line, const char *file, const char *func));
AL_FUNC(void, al_free_with_context, (void *ptr,
int line, const char *file, const char *func));
AL_FUNC(void *, al_realloc_with_context, (void *ptr, size_t n,
int line, const char *file, const char *func));
AL_FUNC(void *, al_calloc_with_context, (size_t count, size_t n,
int line, const char *file, const char *func));


#if defined(__cplusplus)
}
#endif

#endif
