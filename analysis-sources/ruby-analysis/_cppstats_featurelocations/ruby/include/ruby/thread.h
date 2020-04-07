










#if !defined(RUBY_THREAD_H)
#define RUBY_THREAD_H 1

#if defined(__cplusplus)
extern "C" {
#if 0
} 
#endif
#endif

#include "ruby/intern.h"


#define RB_NOGVL_INTR_FAIL (0x1)
#define RB_NOGVL_UBF_ASYNC_SAFE (0x2)

RUBY_SYMBOL_EXPORT_BEGIN

void *rb_thread_call_with_gvl(void *(*func)(void *), void *data1);

void *rb_thread_call_without_gvl(void *(*func)(void *), void *data1,
rb_unblock_function_t *ubf, void *data2);
void *rb_thread_call_without_gvl2(void *(*func)(void *), void *data1,
rb_unblock_function_t *ubf, void *data2);





void *rb_nogvl(void *(*func)(void *), void *data1,
rb_unblock_function_t *ubf, void *data2,
int flags);

#define RUBY_CALL_WO_GVL_FLAG_SKIP_CHECK_INTS_AFTER 0x01
#define RUBY_CALL_WO_GVL_FLAG_SKIP_CHECK_INTS_

RUBY_SYMBOL_EXPORT_END

#if defined(__cplusplus)
#if 0
{ 
#endif
} 
#endif

#endif 
