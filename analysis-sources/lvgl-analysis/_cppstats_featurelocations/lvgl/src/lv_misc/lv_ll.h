




#if !defined(LV_LL_H)
#define LV_LL_H

#if defined(__cplusplus)
extern "C" {
#endif




#include "lv_mem.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>










typedef uint8_t lv_ll_node_t;


typedef struct
{
uint32_t n_size;
lv_ll_node_t * head;
lv_ll_node_t * tail;
} lv_ll_t;










void lv_ll_init(lv_ll_t * ll_p, uint32_t node_size);






void * lv_ll_ins_head(lv_ll_t * ll_p);







void * lv_ll_ins_prev(lv_ll_t * ll_p, void * n_act);






void * lv_ll_ins_tail(lv_ll_t * ll_p);







void lv_ll_rem(lv_ll_t * ll_p, void * node_p);





void lv_ll_clear(lv_ll_t * ll_p);









void lv_ll_chg_list(lv_ll_t * ll_ori_p, lv_ll_t * ll_new_p, void * node, bool head);






void * lv_ll_get_head(const lv_ll_t * ll_p);






void * lv_ll_get_tail(const lv_ll_t * ll_p);







void * lv_ll_get_next(const lv_ll_t * ll_p, const void * n_act);







void * lv_ll_get_prev(const lv_ll_t * ll_p, const void * n_act);






uint32_t lv_ll_get_len(const lv_ll_t * ll_p);







void lv_ll_move_before(lv_ll_t * ll_p, void * n_act, void * n_after);






bool lv_ll_is_empty(lv_ll_t * ll_p);




#define LV_LL_READ(list, i) for(i = lv_ll_get_head(&list); i != NULL; i = lv_ll_get_next(&list, i))

#define LV_LL_READ_BACK(list, i) for(i = lv_ll_get_tail(&list); i != NULL; i = lv_ll_get_prev(&list, i))

#if defined(__cplusplus)
} 
#endif

#endif
