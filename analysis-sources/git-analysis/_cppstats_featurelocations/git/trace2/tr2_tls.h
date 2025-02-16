#if !defined(TR2_TLS_H)
#define TR2_TLS_H

#include "strbuf.h"




#define TR2_MAX_THREAD_NAME (24)

struct tr2tls_thread_ctx {
struct strbuf thread_name;
uint64_t *array_us_start;
int alloc;
int nr_open_regions; 
int thread_id;
};
















struct tr2tls_thread_ctx *tr2tls_create_self(const char *thread_name,
uint64_t us_thread_start);




struct tr2tls_thread_ctx *tr2tls_get_self(void);




int tr2tls_is_main_thread(void);




void tr2tls_unset_self(void);




void tr2tls_push_self(uint64_t us_now);




void tr2tls_pop_self(void);







void tr2tls_pop_unwind_self(void);





uint64_t tr2tls_region_elasped_self(uint64_t us);






uint64_t tr2tls_absolute_elapsed(uint64_t us);




void tr2tls_init(void);




void tr2tls_release(void);




int tr2tls_locked_increment(int *p);




void tr2tls_start_process_clock(void);

#endif 
