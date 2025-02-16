#if !defined(NVIM_GARRAY_H)
#define NVIM_GARRAY_H

#include <stddef.h> 

#include "nvim/types.h" 
#include "nvim/log.h"




typedef struct growarray {
int ga_len; 
int ga_maxlen; 
int ga_itemsize; 
int ga_growsize; 
void *ga_data; 
} garray_T;

#define GA_EMPTY_INIT_VALUE { 0, 0, 0, 1, NULL }
#define GA_INIT(itemsize, growsize) { 0, 0, (itemsize), (growsize), NULL }

#define GA_EMPTY(ga_ptr) ((ga_ptr)->ga_len <= 0)

#define GA_APPEND(item_type, gap, item) do { ga_grow(gap, 1); ((item_type *)(gap)->ga_data)[(gap)->ga_len++] = (item); } while (0)





#define GA_APPEND_VIA_PTR(item_type, gap) ga_append_via_ptr(gap, sizeof(item_type))


#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "garray.h.generated.h"
#endif

static inline void *ga_append_via_ptr(garray_T *gap, size_t item_size)
{
if ((int)item_size != gap->ga_itemsize) {
WLOG("wrong item size (%zu), should be %d", item_size, gap->ga_itemsize);
}
ga_grow(gap, 1);
return ((char *)gap->ga_data) + (item_size * (size_t)gap->ga_len++);
}







#define GA_DEEP_CLEAR(gap, item_type, free_item_fn) do { garray_T *_gap = (gap); if (_gap->ga_data != NULL) { for (int i = 0; i < _gap->ga_len; i++) { item_type *_item = &(((item_type *)_gap->ga_data)[i]); free_item_fn(_item); } } ga_clear(_gap); } while (false)











#define FREE_PTR_PTR(ptr) xfree(*(ptr))





#define GA_DEEP_CLEAR_PTR(gap) GA_DEEP_CLEAR(gap, void*, FREE_PTR_PTR)

#endif 
