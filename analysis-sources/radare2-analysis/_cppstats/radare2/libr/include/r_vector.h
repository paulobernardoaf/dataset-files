#include <r_types.h>
#include <r_util/r_assert.h>
#if defined(__cplusplus)
extern "C" {
#endif
typedef int (*RPVectorComparator)(const void *a, const void *b);
typedef void (*RVectorFree)(void *e, void *user);
typedef void (*RPVectorFree)(void *e);
typedef struct r_vector_t {
void *a;
size_t len;
size_t capacity;
size_t elem_size;
RVectorFree free;
void *free_user;
} RVector;
typedef struct r_pvector_t { RVector v; } RPVector;
R_API void r_vector_init(RVector *vec, size_t elem_size, RVectorFree free, void *free_user);
R_API RVector *r_vector_new(size_t elem_size, RVectorFree free, void *free_user);
R_API void r_vector_clear(RVector *vec);
R_API void r_vector_free(RVector *vec);
R_API RVector *r_vector_clone(RVector *vec);
static inline bool r_vector_empty(const RVector *vec) {
r_return_val_if_fail (vec, false);
return vec->len == 0;
}
R_API void *r_vector_index_ptr(RVector *vec, size_t index);
R_API void r_vector_assign(RVector *vec, void *p, void *elem);
R_API void *r_vector_assign_at(RVector *vec, size_t index, void *elem);
R_API void r_vector_remove_at(RVector *vec, size_t index, void *into);
R_API void *r_vector_insert(RVector *vec, size_t index, void *x);
R_API void *r_vector_insert_range(RVector *vec, size_t index, void *first, size_t count);
R_API void r_vector_pop(RVector *vec, void *into);
R_API void r_vector_pop_front(RVector *vec, void *into);
R_API void *r_vector_push(RVector *vec, void *x);
R_API void *r_vector_push_front(RVector *vec, void *x);
R_API void *r_vector_reserve(RVector *vec, size_t capacity);
R_API void *r_vector_shrink(RVector *vec);
#define r_vector_foreach(vec, it) if (!r_vector_empty (vec)) for (it = (void *)(vec)->a; (char *)it != (char *)(vec)->a + ((vec)->len * (vec)->elem_size); it = (void *)((char *)it + (vec)->elem_size))
#define r_vector_enumerate(vec, it, i) if (!r_vector_empty (vec)) for (it = (void *)(vec)->a, i = 0; i < (vec)->len; it = (void *)((char *)it + (vec)->elem_size), i++)
R_API void r_pvector_init(RPVector *vec, RPVectorFree free);
R_API RPVector *r_pvector_new(RPVectorFree free);
R_API void r_pvector_clear(RPVector *vec);
R_API void r_pvector_free(RPVector *vec);
static inline size_t r_pvector_len(const RPVector *vec) {
return vec->v.len;
}
static inline void *r_pvector_at(const RPVector *vec, size_t index) {
return ((void **)vec->v.a)[index];
}
static inline void r_pvector_set(RPVector *vec, size_t index, void *e) {
((void **)vec->v.a)[index] = e;
}
static inline bool r_pvector_empty(RPVector *vec) {
return r_pvector_len (vec) == 0;
}
static inline void **r_pvector_index_ptr(RPVector *vec, size_t index) {
return ((void **)vec->v.a) + index;
}
static inline void **r_pvector_data(RPVector *vec) {
return (void **)vec->v.a;
}
R_API void **r_pvector_contains(RPVector *vec, void *x);
R_API void *r_pvector_remove_at(RPVector *vec, size_t index);
R_API void r_pvector_remove_data(RPVector *vec, void *x);
static inline void **r_pvector_insert(RPVector *vec, size_t index, void *x) {
return (void **)r_vector_insert (&vec->v, index, &x);
}
static inline void **r_pvector_insert_range(RPVector *vec, size_t index, void **first, size_t count) {
return (void **)r_vector_insert_range (&vec->v, index, first, count);
}
R_API void *r_pvector_pop(RPVector *vec);
R_API void *r_pvector_pop_front(RPVector *vec);
static inline void **r_pvector_push(RPVector *vec, void *x) {
return (void **)r_vector_push (&vec->v, &x);
}
static inline void **r_pvector_push_front(RPVector *vec, void *x) {
return (void **)r_vector_push_front (&vec->v, &x);
}
R_API void r_pvector_sort(RPVector *vec, RPVectorComparator cmp);
static inline void **r_pvector_reserve(RPVector *vec, size_t capacity) {
return (void **)r_vector_reserve (&vec->v, capacity);
}
static inline void **r_pvector_shrink(RPVector *vec) {
return (void **)r_vector_shrink (&vec->v);
}
#define r_pvector_foreach(vec, it) for (it = (void **)(vec)->v.a; it != (void **)(vec)->v.a + (vec)->v.len; it++)
#define r_pvector_lower_bound(vec, x, i, cmp) do { int h = (vec)->v.len, m; for (i = 0; i < h; ) { m = i + ((h - i) >> 1); if ((cmp (x, ((void **)(vec)->v.a)[m])) > 0) { i = m + 1; } else { h = m; } } } while (0) 
#define r_pvector_upper_bound(vec, x, i, cmp) do { int h = (vec)->v.len, m; for (i = 0; i < h; ) { m = i + ((h - i) >> 1); if (!((cmp (x, ((void **)(vec)->v.a)[m])) < 0)) { i = m + 1; } else { h = m; } } } while (0) 
#if defined(__cplusplus)
}
#endif
