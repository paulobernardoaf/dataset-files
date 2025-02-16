#if defined(__cplusplus)
extern "C" {
#endif
#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif
#include <stdint.h>
#include <stddef.h>
#include "lv_log.h"
#include "lv_types.h"
typedef struct
{
uint32_t total_size; 
uint32_t free_cnt;
uint32_t free_size; 
uint32_t free_biggest_size;
uint32_t used_cnt;
uint8_t used_pct; 
uint8_t frag_pct; 
} lv_mem_monitor_t;
void lv_mem_init(void);
void lv_mem_deinit(void);
void * lv_mem_alloc(size_t size);
void lv_mem_free(const void * data);
void * lv_mem_realloc(void * data_p, size_t new_size);
void lv_mem_defrag(void);
void lv_mem_monitor(lv_mem_monitor_t * mon_p);
uint32_t lv_mem_get_size(const void * data);
#if defined(__cplusplus)
} 
#endif
