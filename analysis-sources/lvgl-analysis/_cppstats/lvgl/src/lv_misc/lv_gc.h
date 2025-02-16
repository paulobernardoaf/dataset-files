#if defined(__cplusplus)
extern "C" {
#endif
#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif
#include <stdint.h>
#include <stdbool.h>
#include "lv_mem.h"
#include "lv_ll.h"
#include "../lv_draw/lv_img_cache.h"
#define LV_ITERATE_ROOTS(f) f(lv_ll_t, _lv_task_ll) f(lv_ll_t, _lv_disp_ll) f(lv_ll_t, _lv_indev_ll) f(lv_ll_t, _lv_drv_ll) f(lv_ll_t, _lv_file_ll) f(lv_ll_t, _lv_anim_ll) f(lv_ll_t, _lv_group_ll) f(lv_ll_t, _lv_img_defoder_ll) f(lv_img_cache_entry_t*, _lv_img_cache_array) f(void*, _lv_task_act) f(void*, _lv_draw_buf)
#define LV_DEFINE_ROOT(root_type, root_name) root_type root_name;
#define LV_ROOTS LV_ITERATE_ROOTS(LV_DEFINE_ROOT)
#if LV_ENABLE_GC == 1
#if LV_MEM_CUSTOM != 1
#error "GC requires CUSTOM_MEM"
#endif 
#else 
#define LV_GC_ROOT(x) x
#define LV_EXTERN_ROOT(root_type, root_name) extern root_type root_name;
LV_ITERATE_ROOTS(LV_EXTERN_ROOT)
#endif 
void lv_gc_clear_roots(void);
#if defined(__cplusplus)
} 
#endif
