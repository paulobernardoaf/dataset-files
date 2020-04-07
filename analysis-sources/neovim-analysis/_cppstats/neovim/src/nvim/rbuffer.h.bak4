












#if !defined(NVIM_RBUFFER_H)
#define NVIM_RBUFFER_H

#include <stddef.h>
#include <stdint.h>




















#define RBUFFER_UNTIL_EMPTY(buf, rptr, rcnt) for (size_t rcnt = 0, _r = 1; _r; _r = 0) for ( char *rptr = rbuffer_read_ptr(buf, &rcnt); buf->size; rptr = rbuffer_read_ptr(buf, &rcnt))






#define RBUFFER_UNTIL_FULL(buf, wptr, wcnt) for (size_t wcnt = 0, _r = 1; _r; _r = 0) for ( char *wptr = rbuffer_write_ptr(buf, &wcnt); rbuffer_space(buf); wptr = rbuffer_write_ptr(buf, &wcnt))








#define RBUFFER_EACH(buf, c, i) for (size_t i = 0; i < buf->size; i = buf->size) for (char c = 0; i < buf->size ? ((int)(c = *rbuffer_get(buf, i))) || 1 : 0; i++)







#define RBUFFER_EACH_REVERSE(buf, c, i) for (size_t i = buf->size; i != SIZE_MAX; i = SIZE_MAX) for (char c = 0; i-- > 0 ? ((int)(c = *rbuffer_get(buf, i))) || 1 : 0; )







typedef struct rbuffer RBuffer;



typedef void(*rbuffer_callback)(RBuffer *buf, void *data);

struct rbuffer {
rbuffer_callback full_cb, nonfull_cb;
void *data;
size_t size;

char *temp;
char *end_ptr, *read_ptr, *write_ptr;
char start_ptr[];
};

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "rbuffer.h.generated.h"
#endif

#endif 
