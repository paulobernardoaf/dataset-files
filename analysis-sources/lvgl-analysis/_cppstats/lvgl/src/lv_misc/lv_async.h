#if defined(__cplusplus)
extern "C" {
#endif
#include "lv_task.h"
#include "lv_types.h"
typedef void (*lv_async_cb_t)(void *);
typedef struct _lv_async_info_t {
lv_async_cb_t cb;
void *user_data;
} lv_async_info_t;
struct _lv_obj_t;
lv_res_t lv_async_call(lv_async_cb_t async_xcb, void * user_data);
#if defined(__cplusplus)
} 
#endif
