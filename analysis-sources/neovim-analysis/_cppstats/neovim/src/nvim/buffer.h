#include "nvim/vim.h"
#include "nvim/window.h"
#include "nvim/pos.h" 
#include "nvim/ex_cmds_defs.h" 
#include "nvim/screen.h" 
#include "nvim/func_attr.h"
#include "nvim/eval.h"
#include "nvim/macros.h"
enum getf_values {
GETF_SETMARK = 0x01, 
GETF_ALT = 0x02, 
GETF_SWITCH = 0x04, 
};
enum getf_retvalues {
GETFILE_ERROR = 1, 
GETFILE_NOT_WRITTEN = 2, 
GETFILE_SAME_FILE = 0, 
GETFILE_OPEN_OTHER = -1, 
GETFILE_UNUSED = 8
};
enum bln_values {
BLN_CURBUF = 1, 
BLN_LISTED = 2, 
BLN_DUMMY = 4, 
BLN_NEW = 8, 
BLN_NOOPT = 16, 
};
enum dobuf_action_values {
DOBUF_GOTO = 0, 
DOBUF_SPLIT = 1, 
DOBUF_UNLOAD = 2, 
DOBUF_DEL = 3, 
DOBUF_WIPE = 4, 
};
enum dobuf_start_values {
DOBUF_CURRENT = 0, 
DOBUF_FIRST = 1, 
DOBUF_LAST = 2, 
DOBUF_MOD = 3, 
};
enum bfa_values {
BFA_DEL = 1, 
BFA_WIPE = 2, 
BFA_KEEP_UNDO = 4, 
};
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "buffer.h.generated.h"
#endif
static inline void buf_set_changedtick(buf_T *const buf,
const varnumber_T changedtick)
REAL_FATTR_NONNULL_ALL REAL_FATTR_ALWAYS_INLINE;
static inline void buf_set_changedtick(buf_T *const buf,
const varnumber_T changedtick)
{
typval_T old_val = buf->changedtick_di.di_tv;
#if !defined(NDEBUG)
dictitem_T *const changedtick_di = tv_dict_find(
buf->b_vars, S_LEN("changedtick"));
assert(changedtick_di != NULL);
assert(changedtick_di->di_tv.v_type == VAR_NUMBER);
assert(changedtick_di->di_tv.v_lock == VAR_FIXED);
#if !defined(UNIT_TESTING_LUA_PREPROCESSING)
assert(changedtick_di->di_flags == (DI_FLAGS_RO|DI_FLAGS_FIX));
#endif
assert(changedtick_di == (dictitem_T *)&buf->changedtick_di);
#endif
buf->changedtick_di.di_tv.vval.v_number = changedtick;
if (tv_dict_is_watched(buf->b_vars)) {
tv_dict_watcher_notify(buf->b_vars,
(char *)buf->changedtick_di.di_key,
&buf->changedtick_di.di_tv,
&old_val);
}
}
static inline varnumber_T buf_get_changedtick(const buf_T *const buf)
REAL_FATTR_NONNULL_ALL REAL_FATTR_ALWAYS_INLINE REAL_FATTR_PURE
REAL_FATTR_WARN_UNUSED_RESULT;
static inline varnumber_T buf_get_changedtick(const buf_T *const buf)
{
return buf->changedtick_di.di_tv.vval.v_number;
}
static inline void buf_inc_changedtick(buf_T *const buf)
REAL_FATTR_NONNULL_ALL REAL_FATTR_ALWAYS_INLINE;
static inline void buf_inc_changedtick(buf_T *const buf)
{
buf_set_changedtick(buf, buf_get_changedtick(buf) + 1);
}
