




#if !defined(LV_UTILS_H)
#define LV_UTILS_H

#if defined(__cplusplus)
extern "C" {
#endif




#include <stdint.h>
#include <stddef.h>


















char * lv_utils_num_to_str(int32_t num, char * buf);


















void * lv_utils_bsearch(const void * key, const void * base, uint32_t n, uint32_t size,
int32_t (*cmp)(const void * pRef, const void * pElement));





#if defined(__cplusplus)
} 
#endif

#endif
