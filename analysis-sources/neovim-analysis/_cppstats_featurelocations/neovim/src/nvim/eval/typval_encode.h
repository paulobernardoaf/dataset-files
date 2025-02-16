



#if !defined(NVIM_EVAL_TYPVAL_ENCODE_H)
#define NVIM_EVAL_TYPVAL_ENCODE_H

#include <stddef.h>
#include <inttypes.h>
#include <string.h>
#include <assert.h>

#include "nvim/lib/kvec.h"
#include "nvim/eval/typval.h"
#include "nvim/func_attr.h"


typedef enum {
kMPConvDict, 
kMPConvList, 
kMPConvPairs, 
kMPConvPartial, 
kMPConvPartialList, 
} MPConvStackValType;


typedef enum {
kMPConvPartialArgs, 
kMPConvPartialSelf, 
kMPConvPartialEnd, 
} MPConvPartialStage;


typedef struct {
MPConvStackValType type; 
typval_T *tv; 
int saved_copyID; 
union {
struct {
dict_T *dict; 
dict_T **dictp; 


hashitem_T *hi; 
size_t todo; 
} d; 
struct {
list_T *list; 
listitem_T *li; 
} l; 
struct {
MPConvPartialStage stage; 
partial_T *pt; 
} p; 
struct {
typval_T *arg; 
typval_T *argv; 
size_t todo; 
} a; 
} data; 
} MPConvStackVal;


typedef kvec_withinit_t(MPConvStackVal, 8) MPConvStack;


#define _mp_size kv_size
#define _mp_init kvi_init
#define _mp_destroy kvi_destroy
#define _mp_push kvi_push
#define _mp_pop kv_pop
#define _mp_last kv_last

static inline size_t tv_strlen(const typval_T *const tv)
REAL_FATTR_ALWAYS_INLINE REAL_FATTR_PURE REAL_FATTR_WARN_UNUSED_RESULT
REAL_FATTR_NONNULL_ALL;








static inline size_t tv_strlen(const typval_T *const tv)
{
assert(tv->v_type == VAR_STRING);
return (tv->vval.v_string == NULL
? 0
: strlen((char *) tv->vval.v_string));
}









#define _TYPVAL_ENCODE_DO_CHECK_SELF_REFERENCE(val, copyID_attr, copyID, conv_type) do { const int te_csr_ret = _TYPVAL_ENCODE_CHECK_SELF_REFERENCE( TYPVAL_ENCODE_FIRST_ARG_NAME, (val), &(val)->copyID_attr, mpstack, copyID, conv_type, objname); if (te_csr_ret != NOTDONE) { return te_csr_ret; } } while (0)










#define _TYPVAL_ENCODE_FUNC_NAME_INNER_2(pref, name, suf) pref##name##suf

#define _TYPVAL_ENCODE_FUNC_NAME_INNER(pref, name, suf) _TYPVAL_ENCODE_FUNC_NAME_INNER_2(pref, name, suf)












#define _TYPVAL_ENCODE_FUNC_NAME(pref, suf) _TYPVAL_ENCODE_FUNC_NAME_INNER(pref, TYPVAL_ENCODE_NAME, suf)



#define _TYPVAL_ENCODE_CHECK_SELF_REFERENCE _TYPVAL_ENCODE_FUNC_NAME(_typval_encode_, _check_self_reference)



#define _TYPVAL_ENCODE_ENCODE _TYPVAL_ENCODE_FUNC_NAME(encode_vim_to_, )



#define _TYPVAL_ENCODE_CONVERT_ONE_VALUE _TYPVAL_ENCODE_FUNC_NAME(_typval_encode_, _convert_one_value)



#define TYPVAL_ENCODE_NODICT_VAR _TYPVAL_ENCODE_FUNC_NAME(_typval_encode_, _nodict_var)


#endif 
