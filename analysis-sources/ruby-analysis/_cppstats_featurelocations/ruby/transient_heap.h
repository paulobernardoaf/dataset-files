







#if !defined(RUBY_TRANSIENT_HEAP_H)
#define RUBY_TRANSIENT_HEAP_H

#include "internal.h"

#if USE_TRANSIENT_HEAP







void *rb_transient_heap_alloc(VALUE obj, size_t req_size);




void rb_transient_heap_mark(VALUE obj, const void *ptr);


void rb_transient_heap_promote(VALUE obj);
void rb_transient_heap_start_marking(int full_marking);
void rb_transient_heap_finish_marking(void);
void rb_transient_heap_update_references(void);


void rb_transient_heap_dump(void);
void rb_transient_heap_verify(void);
int rb_transient_heap_managed_ptr_p(const void *ptr);


void rb_ary_transient_heap_evacuate(VALUE ary, int promote);
void rb_obj_transient_heap_evacuate(VALUE obj, int promote);
void rb_hash_transient_heap_evacuate(VALUE hash, int promote);
void rb_struct_transient_heap_evacuate(VALUE st, int promote);

#else 

#define rb_transient_heap_alloc(o, s) NULL
#define rb_transient_heap_verify() ((void)0)
#define rb_transient_heap_promote(obj) ((void)0)
#define rb_transient_heap_start_marking(full_marking) ((void)0)
#define rb_transient_heap_update_references() ((void)0)
#define rb_transient_heap_finish_marking() ((void)0)
#define rb_transient_heap_mark(obj, ptr) ((void)0)

#define rb_ary_transient_heap_evacuate(x, y) ((void)0)
#define rb_obj_transient_heap_evacuate(x, y) ((void)0)
#define rb_hash_transient_heap_evacuate(x, y) ((void)0)
#define rb_struct_transient_heap_evacuate(x, y) ((void)0)

#endif 
#endif
