








#include "lv_gc.h"
#include "string.h"

#if defined(LV_GC_INCLUDE)
#include LV_GC_INCLUDE
#endif 
















#if(!defined(LV_ENABLE_GC)) || LV_ENABLE_GC == 0
LV_ROOTS
#endif 








void lv_gc_clear_roots(void)
{
#define LV_CLEAR_ROOT(root_type, root_name) memset(&LV_GC_ROOT(root_name), 0, sizeof(LV_GC_ROOT(root_name)));
LV_ITERATE_ROOTS(LV_CLEAR_ROOT)
}




