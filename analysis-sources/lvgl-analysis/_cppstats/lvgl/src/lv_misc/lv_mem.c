#include "lv_mem.h"
#include "lv_math.h"
#include <string.h>
#if LV_MEM_CUSTOM != 0
#include LV_MEM_CUSTOM_INCLUDE
#endif
#if !defined(LV_MEM_ADD_JUNK)
#define LV_MEM_ADD_JUNK 0
#endif
#if defined(LV_ARCH_64)
#define MEM_UNIT uint64_t
#else
#define MEM_UNIT uint32_t
#endif
#if LV_ENABLE_GC == 0 
typedef union
{
struct
{
MEM_UNIT used : 1; 
MEM_UNIT d_size : 31; 
} s;
MEM_UNIT header; 
} lv_mem_header_t;
typedef struct
{
lv_mem_header_t header;
uint8_t first_data; 
} lv_mem_ent_t;
#endif 
#if LV_MEM_CUSTOM == 0
static lv_mem_ent_t * ent_get_next(lv_mem_ent_t * act_e);
static void * ent_alloc(lv_mem_ent_t * e, size_t size);
static void ent_trunc(lv_mem_ent_t * e, size_t size);
#endif
#if LV_MEM_CUSTOM == 0
static uint8_t * work_mem;
#endif
static uint32_t zero_mem; 
void lv_mem_init(void)
{
#if LV_MEM_CUSTOM == 0
#if LV_MEM_ADR == 0
static LV_MEM_ATTR MEM_UNIT work_mem_int[LV_MEM_SIZE / sizeof(MEM_UNIT)];
work_mem = (uint8_t *)work_mem_int;
#else
work_mem = (uint8_t *)LV_MEM_ADR;
#endif
lv_mem_ent_t * full = (lv_mem_ent_t *)work_mem;
full->header.s.used = 0;
full->header.s.d_size = LV_MEM_SIZE - sizeof(lv_mem_header_t);
#endif
}
void lv_mem_deinit(void)
{
#if LV_MEM_CUSTOM == 0
memset(work_mem, 0x00, (LV_MEM_SIZE / sizeof(MEM_UNIT)) * sizeof(MEM_UNIT));
lv_mem_ent_t * full = (lv_mem_ent_t *)work_mem;
full->header.s.used = 0;
full->header.s.d_size = LV_MEM_SIZE - sizeof(lv_mem_header_t);
#endif
}
void * lv_mem_alloc(size_t size)
{
if(size == 0) {
return &zero_mem;
}
#if defined(LV_ARCH_64)
if(size & 0x7) {
size = size & (~0x7);
size += 8;
}
#else
if(size & 0x3) {
size = size & (~0x3);
size += 4;
}
#endif
void * alloc = NULL;
#if LV_MEM_CUSTOM == 0
lv_mem_ent_t * e = NULL;
do {
e = ent_get_next(e);
if(e != NULL) {
alloc = ent_alloc(e, size);
}
} while(e != NULL && alloc == NULL);
#else
#if LV_ENABLE_GC == 1 
alloc = LV_MEM_CUSTOM_ALLOC(size);
#else 
alloc = LV_MEM_CUSTOM_ALLOC(size + sizeof(lv_mem_header_t));
if(alloc != NULL) {
((lv_mem_ent_t *)alloc)->header.s.d_size = size;
((lv_mem_ent_t *)alloc)->header.s.used = 1;
alloc = &((lv_mem_ent_t *)alloc)->first_data;
}
#endif 
#endif 
#if LV_MEM_ADD_JUNK
if(alloc != NULL) memset(alloc, 0xaa, size);
#endif
if(alloc == NULL) LV_LOG_WARN("Couldn't allocate memory");
return alloc;
}
void lv_mem_free(const void * data)
{
if(data == &zero_mem) return;
if(data == NULL) return;
#if LV_MEM_ADD_JUNK
memset((void *)data, 0xbb, lv_mem_get_size(data));
#endif
#if LV_ENABLE_GC == 0
lv_mem_ent_t * e = (lv_mem_ent_t *)((uint8_t *)data - sizeof(lv_mem_header_t));
e->header.s.used = 0;
#endif
#if LV_MEM_CUSTOM == 0
#if LV_MEM_AUTO_DEFRAG
lv_mem_ent_t * e_next;
e_next = ent_get_next(e);
while(e_next != NULL) {
if(e_next->header.s.used == 0) {
e->header.s.d_size += e_next->header.s.d_size + sizeof(e->header);
} else {
break;
}
e_next = ent_get_next(e_next);
}
#endif
#else 
#if LV_ENABLE_GC == 0
LV_MEM_CUSTOM_FREE(e);
#else
LV_MEM_CUSTOM_FREE((void *)data);
#endif 
#endif
}
#if LV_ENABLE_GC == 0
void * lv_mem_realloc(void * data_p, size_t new_size)
{
if(data_p != NULL) {
lv_mem_ent_t * e = (lv_mem_ent_t *)((uint8_t *)data_p - sizeof(lv_mem_header_t));
if(e->header.s.used == 0) {
data_p = NULL;
}
}
uint32_t old_size = lv_mem_get_size(data_p);
if(old_size == new_size) return data_p; 
#if LV_MEM_CUSTOM == 0
if(new_size < old_size) {
lv_mem_ent_t * e = (lv_mem_ent_t *)((uint8_t *)data_p - sizeof(lv_mem_header_t));
ent_trunc(e, new_size);
return &e->first_data;
}
#endif
void * new_p;
new_p = lv_mem_alloc(new_size);
if(new_p != NULL && data_p != NULL) {
if(old_size != 0) {
memcpy(new_p, data_p, LV_MATH_MIN(new_size, old_size));
lv_mem_free(data_p);
}
}
if(new_p == NULL) LV_LOG_WARN("Couldn't allocate memory");
return new_p;
}
#else 
void * lv_mem_realloc(void * data_p, size_t new_size)
{
void * new_p = LV_MEM_CUSTOM_REALLOC(data_p, new_size);
if(new_p == NULL) LV_LOG_WARN("Couldn't allocate memory");
return new_p;
}
#endif 
void lv_mem_defrag(void)
{
#if LV_MEM_CUSTOM == 0
lv_mem_ent_t * e_free;
lv_mem_ent_t * e_next;
e_free = ent_get_next(NULL);
while(1) {
while(e_free != NULL) {
if(e_free->header.s.used != 0) {
e_free = ent_get_next(e_free);
} else {
break;
}
}
if(e_free == NULL) return;
e_next = ent_get_next(e_free);
while(e_next != NULL) {
if(e_next->header.s.used == 0) {
e_free->header.s.d_size += e_next->header.s.d_size + sizeof(e_next->header);
} else {
break;
}
e_next = ent_get_next(e_next);
}
if(e_next == NULL) return;
e_free = e_next;
}
#endif
}
void lv_mem_monitor(lv_mem_monitor_t * mon_p)
{
memset(mon_p, 0, sizeof(lv_mem_monitor_t));
#if LV_MEM_CUSTOM == 0
lv_mem_ent_t * e;
e = NULL;
e = ent_get_next(e);
while(e != NULL) {
if(e->header.s.used == 0) {
mon_p->free_cnt++;
mon_p->free_size += e->header.s.d_size;
if(e->header.s.d_size > mon_p->free_biggest_size) {
mon_p->free_biggest_size = e->header.s.d_size;
}
} else {
mon_p->used_cnt++;
}
e = ent_get_next(e);
}
mon_p->total_size = LV_MEM_SIZE;
mon_p->used_pct = 100 - (100U * mon_p->free_size) / mon_p->total_size;
if(mon_p->free_size > 0) {
mon_p->frag_pct = (uint32_t)mon_p->free_biggest_size * 100U / mon_p->free_size;
mon_p->frag_pct = 100 - mon_p->frag_pct;
} else {
mon_p->frag_pct = 0; 
}
#endif
}
#if LV_ENABLE_GC == 0
uint32_t lv_mem_get_size(const void * data)
{
if(data == NULL) return 0;
if(data == &zero_mem) return 0;
lv_mem_ent_t * e = (lv_mem_ent_t *)((uint8_t *)data - sizeof(lv_mem_header_t));
return e->header.s.d_size;
}
#else 
uint32_t lv_mem_get_size(const void * data)
{
return LV_MEM_CUSTOM_GET_SIZE(data);
}
#endif 
#if LV_MEM_CUSTOM == 0
static lv_mem_ent_t * ent_get_next(lv_mem_ent_t * act_e)
{
lv_mem_ent_t * next_e = NULL;
if(act_e == NULL) { 
next_e = (lv_mem_ent_t *)work_mem;
} else { 
uint8_t * data = &act_e->first_data;
next_e = (lv_mem_ent_t *)&data[act_e->header.s.d_size];
if(&next_e->first_data >= &work_mem[LV_MEM_SIZE]) next_e = NULL;
}
return next_e;
}
static void * ent_alloc(lv_mem_ent_t * e, size_t size)
{
void * alloc = NULL;
if(e->header.s.used == 0 && e->header.s.d_size >= size) {
ent_trunc(e, size),
e->header.s.used = 1;
alloc = &e->first_data;
}
return alloc;
}
static void ent_trunc(lv_mem_ent_t * e, size_t size)
{
#if defined(LV_ARCH_64)
if(size & 0x7) {
size = size & (~0x7);
size += 8;
}
#else
if(size & 0x3) {
size = size & (~0x3);
size += 4;
}
#endif
if(e->header.s.d_size == size + sizeof(lv_mem_header_t)) {
size = e->header.s.d_size;
}
if(e->header.s.d_size != size) {
uint8_t * e_data = &e->first_data;
lv_mem_ent_t * after_new_e = (lv_mem_ent_t *)&e_data[size];
after_new_e->header.s.used = 0;
after_new_e->header.s.d_size = (uint32_t)e->header.s.d_size - size - sizeof(lv_mem_header_t);
}
e->header.s.d_size = (uint32_t)size;
}
#endif
