








#include <stdint.h>
#include <string.h>

#include "lv_ll.h"
#include "lv_mem.h"




#define LL_NODE_META_SIZE (sizeof(lv_ll_node_t *) + sizeof(lv_ll_node_t *))
#define LL_PREV_P_OFFSET(ll_p) (ll_p->n_size)
#define LL_NEXT_P_OFFSET(ll_p) (ll_p->n_size + sizeof(lv_ll_node_t *))








static void node_set_prev(lv_ll_t * ll_p, lv_ll_node_t * act, lv_ll_node_t * prev);
static void node_set_next(lv_ll_t * ll_p, lv_ll_node_t * act, lv_ll_node_t * next);


















void lv_ll_init(lv_ll_t * ll_p, uint32_t node_size)
{
ll_p->head = NULL;
ll_p->tail = NULL;
#if defined(LV_MEM_ENV64)

if(node_size & 0x7) {
node_size = node_size & (~0x7);
node_size += 8;
}
#else

if(node_size & 0x3) {
node_size = node_size & (~0x3);
node_size += 4;
}
#endif

ll_p->n_size = node_size;
}






void * lv_ll_ins_head(lv_ll_t * ll_p)
{
lv_ll_node_t * n_new;

n_new = lv_mem_alloc(ll_p->n_size + LL_NODE_META_SIZE);

if(n_new != NULL) {
node_set_prev(ll_p, n_new, NULL); 
node_set_next(ll_p, n_new, ll_p->head); 

if(ll_p->head != NULL) { 
node_set_prev(ll_p, ll_p->head, n_new);
}

ll_p->head = n_new; 
if(ll_p->tail == NULL) { 
ll_p->tail = n_new;
}
}

return n_new;
}







void * lv_ll_ins_prev(lv_ll_t * ll_p, void * n_act)
{
lv_ll_node_t * n_new;
lv_ll_node_t * n_prev;

if(NULL == ll_p || NULL == n_act) return NULL;

if(lv_ll_get_head(ll_p) == n_act) {
n_new = lv_ll_ins_head(ll_p);
if(n_new == NULL) return NULL;
} else {
n_new = lv_mem_alloc(ll_p->n_size + LL_NODE_META_SIZE);
if(n_new == NULL) return NULL;

n_prev = lv_ll_get_prev(ll_p, n_act);
node_set_next(ll_p, n_prev, n_new);
node_set_prev(ll_p, n_new, n_prev);
node_set_prev(ll_p, n_act, n_new);
node_set_next(ll_p, n_new, n_act);
}

return n_new;
}






void * lv_ll_ins_tail(lv_ll_t * ll_p)
{
lv_ll_node_t * n_new;

n_new = lv_mem_alloc(ll_p->n_size + LL_NODE_META_SIZE);
if(n_new == NULL) return NULL;

if(n_new != NULL) {
node_set_next(ll_p, n_new, NULL); 
node_set_prev(ll_p, n_new, ll_p->tail); 
if(ll_p->tail != NULL) { 
node_set_next(ll_p, ll_p->tail, n_new);
}

ll_p->tail = n_new; 
if(ll_p->head == NULL) { 
ll_p->head = n_new;
}
}

return n_new;
}







void lv_ll_rem(lv_ll_t * ll_p, void * node_p)
{
if(lv_ll_get_head(ll_p) == node_p) {

ll_p->head = lv_ll_get_next(ll_p, node_p);
if(ll_p->head == NULL) {
ll_p->tail = NULL;
} else {
node_set_prev(ll_p, ll_p->head, NULL);
}
} else if(lv_ll_get_tail(ll_p) == node_p) {

ll_p->tail = lv_ll_get_prev(ll_p, node_p);
if(ll_p->tail == NULL) {
ll_p->head = NULL;
} else {
node_set_next(ll_p, ll_p->tail, NULL);
}
} else {
lv_ll_node_t * n_prev = lv_ll_get_prev(ll_p, node_p);
lv_ll_node_t * n_next = lv_ll_get_next(ll_p, node_p);

node_set_next(ll_p, n_prev, n_next);
node_set_prev(ll_p, n_next, n_prev);
}
}





void lv_ll_clear(lv_ll_t * ll_p)
{
void * i;
void * i_next;

i = lv_ll_get_head(ll_p);
i_next = NULL;

while(i != NULL) {
i_next = lv_ll_get_next(ll_p, i);

lv_ll_rem(ll_p, i);
lv_mem_free(i);

i = i_next;
}
}









void lv_ll_chg_list(lv_ll_t * ll_ori_p, lv_ll_t * ll_new_p, void * node, bool head)
{
lv_ll_rem(ll_ori_p, node);

if(head) {

node_set_prev(ll_new_p, node, NULL);
node_set_next(ll_new_p, node, ll_new_p->head);

if(ll_new_p->head != NULL) { 
node_set_prev(ll_new_p, ll_new_p->head, node);
}

ll_new_p->head = node; 
if(ll_new_p->tail == NULL) { 
ll_new_p->tail = node;
}
} else {

node_set_prev(ll_new_p, node, ll_new_p->tail);
node_set_next(ll_new_p, node, NULL);

if(ll_new_p->tail != NULL) { 
node_set_next(ll_new_p, ll_new_p->tail, node);
}

ll_new_p->tail = node; 
if(ll_new_p->head == NULL) { 
ll_new_p->head = node;
}
}
}






void * lv_ll_get_head(const lv_ll_t * ll_p)
{
void * head = NULL;

if(ll_p != NULL) {
head = ll_p->head;
}

return head;
}






void * lv_ll_get_tail(const lv_ll_t * ll_p)
{
void * tail = NULL;

if(ll_p != NULL) {
tail = ll_p->tail;
}

return tail;
}







void * lv_ll_get_next(const lv_ll_t * ll_p, const void * n_act)
{
void * next = NULL;

if(ll_p != NULL) {
const lv_ll_node_t * n_act_d = n_act;
memcpy(&next, n_act_d + LL_NEXT_P_OFFSET(ll_p), sizeof(void *));
}

return next;
}







void * lv_ll_get_prev(const lv_ll_t * ll_p, const void * n_act)
{
void * prev = NULL;

if(ll_p != NULL) {
const lv_ll_node_t * n_act_d = n_act;
memcpy(&prev, n_act_d + LL_PREV_P_OFFSET(ll_p), sizeof(void *));
}

return prev;
}






uint32_t lv_ll_get_len(const lv_ll_t * ll_p)
{
uint32_t len = 0;
void * node;

for(node = lv_ll_get_head(ll_p); node != NULL; node = lv_ll_get_next(ll_p, node)) {
len++;
}

return len;
}







void lv_ll_move_before(lv_ll_t * ll_p, void * n_act, void * n_after)
{
if(n_act == n_after) return; 

void * n_before;
if(n_after != NULL)
n_before = lv_ll_get_prev(ll_p, n_after);
else
n_before = lv_ll_get_tail(ll_p); 

if(n_act == n_before) return; 


lv_ll_rem(ll_p, n_act);


node_set_next(ll_p, n_before, n_act);
node_set_prev(ll_p, n_act, n_before);
node_set_prev(ll_p, n_after, n_act);
node_set_next(ll_p, n_act, n_after);


if(n_after == NULL) ll_p->tail = n_act;


if(n_before == NULL) ll_p->head = n_act;
}






bool lv_ll_is_empty(lv_ll_t * ll_p)
{
if(ll_p == NULL) return true;

if(ll_p->head == NULL && ll_p->tail == NULL) return true;

return false;
}











static void node_set_prev(lv_ll_t * ll_p, lv_ll_node_t * act, lv_ll_node_t * prev)
{
if(act == NULL) return; 

uint32_t node_p_size = sizeof(lv_ll_node_t *);
if(prev)
memcpy(act + LL_PREV_P_OFFSET(ll_p), &prev, node_p_size);
else
memset(act + LL_PREV_P_OFFSET(ll_p), 0, node_p_size);
}







static void node_set_next(lv_ll_t * ll_p, lv_ll_node_t * act, lv_ll_node_t * next)
{
if(act == NULL) return; 

uint32_t node_p_size = sizeof(lv_ll_node_t *);
if(next)
memcpy(act + LL_NEXT_P_OFFSET(ll_p), &next, node_p_size);
else
memset(act + LL_NEXT_P_OFFSET(ll_p), 0, node_p_size);
}
