











#if !defined(LZMA_COMMON_H)
#define LZMA_COMMON_H

#include "sysdefs.h"
#include "mythread.h"
#include "tuklib_integer.h"

#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(DLL_EXPORT)
#define LZMA_API_EXPORT __declspec(dllexport)
#else
#define LZMA_API_EXPORT
#endif

#elif HAVE_VISIBILITY
#define LZMA_API_EXPORT __attribute__((__visibility__("default")))
#else
#define LZMA_API_EXPORT
#endif

#define LZMA_API(type) LZMA_API_EXPORT type LZMA_API_CALL

#include "lzma.h"



#if defined(__GNUC__)
#define likely(expr) __builtin_expect(expr, true)
#define unlikely(expr) __builtin_expect(expr, false)
#else
#define likely(expr) (expr)
#define unlikely(expr) (expr)
#endif



#define LZMA_BUFFER_SIZE 4096






#define LZMA_MEMUSAGE_BASE (UINT64_C(1) << 15)



#define LZMA_FILTER_RESERVED_START (LZMA_VLI_C(1) << 62)




#define LZMA_SUPPORTED_FLAGS ( LZMA_TELL_NO_CHECK | LZMA_TELL_UNSUPPORTED_CHECK | LZMA_TELL_ANY_CHECK | LZMA_CONCATENATED )








typedef struct lzma_coder_s lzma_coder;

typedef struct lzma_next_coder_s lzma_next_coder;

typedef struct lzma_filter_info_s lzma_filter_info;



typedef lzma_ret (*lzma_init_function)(
lzma_next_coder *next, lzma_allocator *allocator,
const lzma_filter_info *filters);





typedef lzma_ret (*lzma_code_function)(
lzma_coder *coder, lzma_allocator *allocator,
const uint8_t *restrict in, size_t *restrict in_pos,
size_t in_size, uint8_t *restrict out,
size_t *restrict out_pos, size_t out_size,
lzma_action action);


typedef void (*lzma_end_function)(
lzma_coder *coder, lzma_allocator *allocator);





struct lzma_filter_info_s {


lzma_vli id;



lzma_init_function init;


void *options;
};



struct lzma_next_coder_s {

lzma_coder *coder;



lzma_vli id;





uintptr_t init;


lzma_code_function code;




lzma_end_function end;



lzma_check (*get_check)(const lzma_coder *coder);



lzma_ret (*memconfig)(lzma_coder *coder, uint64_t *memusage,
uint64_t *old_memlimit, uint64_t new_memlimit);



lzma_ret (*update)(lzma_coder *coder, lzma_allocator *allocator,
const lzma_filter *filters,
const lzma_filter *reversed_filters);
};



#define LZMA_NEXT_CODER_INIT (lzma_next_coder){ .coder = NULL, .init = (uintptr_t)(NULL), .id = LZMA_VLI_UNKNOWN, .code = NULL, .end = NULL, .get_check = NULL, .memconfig = NULL, .update = NULL, }














struct lzma_internal_s {

lzma_next_coder next;





enum {
ISEQ_RUN,
ISEQ_SYNC_FLUSH,
ISEQ_FULL_FLUSH,
ISEQ_FINISH,
ISEQ_END,
ISEQ_ERROR,
} sequence;




size_t avail_in;


bool supported_actions[4];



bool allow_buf_error;
};



extern void *lzma_alloc(size_t size, lzma_allocator *allocator)
lzma_attribute((__malloc__)) lzma_attr_alloc_size(1);


extern void lzma_free(void *ptr, lzma_allocator *allocator);




extern lzma_ret lzma_strm_init(lzma_stream *strm);





extern lzma_ret lzma_next_filter_init(lzma_next_coder *next,
lzma_allocator *allocator, const lzma_filter_info *filters);



extern lzma_ret lzma_next_filter_update(
lzma_next_coder *next, lzma_allocator *allocator,
const lzma_filter *reversed_filters);



extern void lzma_next_end(lzma_next_coder *next, lzma_allocator *allocator);




extern size_t lzma_bufcpy(const uint8_t *restrict in, size_t *restrict in_pos,
size_t in_size, uint8_t *restrict out,
size_t *restrict out_pos, size_t out_size);







#define return_if_error(expr) do { const lzma_ret ret_ = (expr); if (ret_ != LZMA_OK) return ret_; } while (0)












#define lzma_next_coder_init(func, next, allocator) do { if ((uintptr_t)(func) != (next)->init) lzma_next_end(next, allocator); (next)->init = (uintptr_t)(func); } while (0)











#define lzma_next_strm_init(func, strm, ...) do { return_if_error(lzma_strm_init(strm)); const lzma_ret ret_ = func(&(strm)->internal->next, (strm)->allocator, __VA_ARGS__); if (ret_ != LZMA_OK) { lzma_end(strm); return ret_; } } while (0)










#endif
