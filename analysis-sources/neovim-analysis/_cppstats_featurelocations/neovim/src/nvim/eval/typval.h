#if !defined(NVIM_EVAL_TYPVAL_H)
#define NVIM_EVAL_TYPVAL_H

#include <inttypes.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "nvim/types.h"
#include "nvim/hashtab.h"
#include "nvim/garray.h"
#include "nvim/mbyte.h"
#include "nvim/func_attr.h"
#include "nvim/lib/queue.h"
#include "nvim/profile.h" 
#include "nvim/pos.h" 
#include "nvim/gettext.h"
#include "nvim/message.h"
#include "nvim/macros.h"
#if defined(LOG_LIST_ACTIONS)
#include "nvim/memory.h"
#endif


typedef int64_t varnumber_T;
typedef uint64_t uvarnumber_T;


typedef double float_T;


enum { DO_NOT_FREE_CNT = (INT_MAX / 2) };


enum ListLenSpecials {




kListLenUnknown = -1,





kListLenShouldKnow = -2,



kListLenMayKnow = -3,
};


#define VARNUMBER_MAX INT64_MAX
#define UVARNUMBER_MAX UINT64_MAX


#define VARNUMBER_MIN INT64_MIN


#define PRIdVARNUMBER PRId64

typedef struct listvar_S list_T;
typedef struct dictvar_S dict_T;
typedef struct partial_S partial_T;

typedef struct ufunc ufunc_T;

typedef enum {
kCallbackNone = 0,
kCallbackFuncref,
kCallbackPartial,
} CallbackType;

typedef struct {
union {
char_u *funcref;
partial_T *partial;
} data;
CallbackType type;
} Callback;
#define CALLBACK_NONE ((Callback){ .type = kCallbackNone })


typedef struct dict_watcher {
Callback callback;
char *key_pattern;
size_t key_pattern_len;
QUEUE node;
bool busy; 
} DictWatcher;


typedef enum {
kSpecialVarFalse, 
kSpecialVarTrue, 
kSpecialVarNull, 
} SpecialVarValue;


typedef enum {
VAR_UNLOCKED = 0, 
VAR_LOCKED = 1, 
VAR_FIXED = 2, 
} VarLockStatus;


typedef enum {
VAR_UNKNOWN = 0, 
VAR_NUMBER, 
VAR_STRING, 
VAR_FUNC, 
VAR_LIST, 
VAR_DICT, 
VAR_FLOAT, 
VAR_SPECIAL, 

VAR_PARTIAL, 
} VarType;


typedef struct {
VarType v_type; 
VarLockStatus v_lock; 
union typval_vval_union {
varnumber_T v_number; 
SpecialVarValue v_special; 
float_T v_float; 
char_u *v_string; 
list_T *v_list; 
dict_T *v_dict; 
partial_T *v_partial; 
} vval; 
} typval_T;


typedef enum {
VAR_NO_SCOPE = 0, 
VAR_SCOPE = 1, 
VAR_DEF_SCOPE = 2, 

} ScopeType;


typedef struct listitem_S listitem_T;

struct listitem_S {
listitem_T *li_next; 
listitem_T *li_prev; 
typval_T li_tv; 
};


typedef struct listwatch_S listwatch_T;

struct listwatch_S {
listitem_T *lw_item; 
listwatch_T *lw_next; 
};



struct listvar_S {
listitem_T *lv_first; 
listitem_T *lv_last; 
listwatch_T *lv_watch; 
listitem_T *lv_idx_item; 
list_T *lv_copylist; 
list_T *lv_used_next; 
list_T *lv_used_prev; 
int lv_refcount; 
int lv_len; 
int lv_idx; 
int lv_copyID; 
VarLockStatus lv_lock; 
};


typedef struct {
list_T sl_list; 
listitem_T sl_items[10];
} staticList10_T;

#define TV_LIST_STATIC10_INIT { .sl_list = { .lv_first = NULL, .lv_last = NULL, .lv_refcount = 0, .lv_len = 0, .lv_watch = NULL, .lv_idx_item = NULL, .lv_lock = VAR_FIXED, .lv_used_next = NULL, .lv_used_prev = NULL, }, }













#define TV_DICTITEM_STRUCT(...) struct { typval_T di_tv; uint8_t di_flags; char_u di_key[__VA_ARGS__]; }












typedef TV_DICTITEM_STRUCT(1) ScopeDictDictItem;






typedef TV_DICTITEM_STRUCT() dictitem_T;


typedef enum {
DI_FLAGS_RO = 1, 
DI_FLAGS_RO_SBX = 2, 
DI_FLAGS_FIX = 4, 
DI_FLAGS_LOCK = 8, 
DI_FLAGS_ALLOC = 16, 
} DictItemFlags;


struct dictvar_S {
VarLockStatus dv_lock; 
ScopeType dv_scope; 

int dv_refcount; 
int dv_copyID; 
hashtab_T dv_hashtab; 
dict_T *dv_copydict; 
dict_T *dv_used_next; 
dict_T *dv_used_prev; 
QUEUE watchers; 
};


typedef int scid_T;

#define PRIdSCID "d"







typedef struct {
scid_T sc_sid; 
int sc_seq; 
linenr_T sc_lnum; 
} sctx_T;


typedef struct funccall_S funccall_T;


struct ufunc {
int uf_varargs; 
int uf_flags;
int uf_calls; 
bool uf_cleared; 
garray_T uf_args; 
garray_T uf_lines; 
int uf_profiling; 
int uf_prof_initialized;

int uf_tm_count; 
proftime_T uf_tm_total; 
proftime_T uf_tm_self; 
proftime_T uf_tm_children; 

int *uf_tml_count; 
proftime_T *uf_tml_total; 
proftime_T *uf_tml_self; 
proftime_T uf_tml_start; 
proftime_T uf_tml_children; 
proftime_T uf_tml_wait; 
int uf_tml_idx; 
int uf_tml_execed; 
sctx_T uf_script_ctx; 

int uf_refcount; 
funccall_T *uf_scoped; 
char_u uf_name[]; 

};


#define MAX_FUNC_ARGS 20

struct partial_S {
int pt_refcount; 
char_u *pt_name; 
ufunc_T *pt_func; 

bool pt_auto; 

int pt_argc; 
typval_T *pt_argv; 
dict_T *pt_dict; 
};


typedef struct ht_stack_S {
hashtab_T *ht;
struct ht_stack_S *prev;
} ht_stack_T;


typedef struct list_stack_S {
list_T *list;
struct list_stack_S *prev;
} list_stack_T;


typedef struct {
listitem_T *item; 
int idx; 
} ListSortItem;

typedef int (*ListSorter)(const void *, const void *);

#if defined(LOG_LIST_ACTIONS)


typedef struct {
uintptr_t l; 
uintptr_t li1; 
uintptr_t li2; 
int len; 
const char *action; 
} ListLogEntry;

typedef struct list_log ListLog;


struct list_log {
ListLog *next; 
size_t capacity; 
size_t size; 
ListLogEntry entries[]; 
};

extern ListLog *list_log_first; 
extern ListLog *list_log_last; 

static inline ListLog *list_log_alloc(const size_t size)
REAL_FATTR_ALWAYS_INLINE REAL_FATTR_WARN_UNUSED_RESULT;






static inline ListLog *list_log_new(const size_t size)
{
ListLog *ret = xmalloc(offsetof(ListLog, entries)
+ size * sizeof(ret->entries[0]));
ret->size = 0;
ret->capacity = size;
ret->next = NULL;
if (list_log_first == NULL) {
list_log_first = ret;
} else {
list_log_last->next = ret;
}
list_log_last = ret;
return ret;
}

static inline void list_log(const list_T *const l,
const listitem_T *const li1,
const listitem_T *const li2,
const char *const action)
REAL_FATTR_ALWAYS_INLINE;










static inline void list_log(const list_T *const l,
const listitem_T *const li1,
const listitem_T *const li2,
const char *const action)
{
ListLog *tgt;
if (list_log_first == NULL) {
tgt = list_log_new(128);
} else if (list_log_last->size == list_log_last->capacity) {
tgt = list_log_new(list_log_last->capacity * 2);
} else {
tgt = list_log_last;
}
tgt->entries[tgt->size++] = (ListLogEntry) {
.l = (uintptr_t)l,
.li1 = (uintptr_t)li1,
.li2 = (uintptr_t)li2,
.len = (l == NULL ? 0 : l->lv_len),
.action = action,
};
}
#else
#define list_log(...)
#define list_write_log(...)
#define list_free_log()
#endif





#define TV_DICT_HI2DI(hi) ((dictitem_T *)((hi)->hi_key - offsetof(dictitem_T, di_key)))


static inline void tv_list_ref(list_T *const l)
REAL_FATTR_ALWAYS_INLINE;






static inline void tv_list_ref(list_T *const l)
{
if (l == NULL) {
return;
}
l->lv_refcount++;
}

static inline void tv_list_set_ret(typval_T *const tv, list_T *const l)
REAL_FATTR_ALWAYS_INLINE REAL_FATTR_NONNULL_ARG(1);





static inline void tv_list_set_ret(typval_T *const tv, list_T *const l)
{
tv->v_type = VAR_LIST;
tv->vval.v_list = l;
tv_list_ref(l);
}

static inline VarLockStatus tv_list_locked(const list_T *const l)
REAL_FATTR_PURE REAL_FATTR_WARN_UNUSED_RESULT;






static inline VarLockStatus tv_list_locked(const list_T *const l)
{
if (l == NULL) {
return VAR_FIXED;
}
return l->lv_lock;
}







static inline void tv_list_set_lock(list_T *const l,
const VarLockStatus lock)
{
if (l == NULL) {
assert(lock == VAR_FIXED);
return;
}
l->lv_lock = lock;
}







static inline void tv_list_set_copyid(list_T *const l,
const int copyid)
FUNC_ATTR_NONNULL_ALL
{
l->lv_copyID = copyid;
}

static inline int tv_list_len(const list_T *const l)
REAL_FATTR_PURE REAL_FATTR_WARN_UNUSED_RESULT;




static inline int tv_list_len(const list_T *const l)
{
list_log(l, NULL, NULL, "len");
if (l == NULL) {
return 0;
}
return l->lv_len;
}

static inline int tv_list_copyid(const list_T *const l)
REAL_FATTR_PURE REAL_FATTR_WARN_UNUSED_RESULT REAL_FATTR_NONNULL_ALL;






static inline int tv_list_copyid(const list_T *const l)
{
return l->lv_copyID;
}

static inline list_T *tv_list_latest_copy(const list_T *const l)
REAL_FATTR_PURE REAL_FATTR_WARN_UNUSED_RESULT REAL_FATTR_NONNULL_ALL;








static inline list_T *tv_list_latest_copy(const list_T *const l)
{
return l->lv_copylist;
}

static inline int tv_list_uidx(const list_T *const l, int n)
REAL_FATTR_PURE REAL_FATTR_WARN_UNUSED_RESULT;







static inline int tv_list_uidx(const list_T *const l, int n)
{

if (n < 0) {
n += tv_list_len(l);
}


if (n < 0 || n >= tv_list_len(l)) {
return -1;
}
return n;
}

static inline bool tv_list_has_watchers(const list_T *const l)
REAL_FATTR_PURE REAL_FATTR_WARN_UNUSED_RESULT;








static inline bool tv_list_has_watchers(const list_T *const l)
{
return l && l->lv_watch;
}

static inline listitem_T *tv_list_first(const list_T *const l)
REAL_FATTR_PURE REAL_FATTR_WARN_UNUSED_RESULT;






static inline listitem_T *tv_list_first(const list_T *const l)
{
if (l == NULL) {
list_log(l, NULL, NULL, "first");
return NULL;
}
list_log(l, l->lv_first, NULL, "first");
return l->lv_first;
}

static inline listitem_T *tv_list_last(const list_T *const l)
REAL_FATTR_PURE REAL_FATTR_WARN_UNUSED_RESULT;






static inline listitem_T *tv_list_last(const list_T *const l)
{
if (l == NULL) {
list_log(l, NULL, NULL, "last");
return NULL;
}
list_log(l, l->lv_last, NULL, "last");
return l->lv_last;
}

static inline void tv_dict_set_ret(typval_T *const tv, dict_T *const d)
REAL_FATTR_ALWAYS_INLINE REAL_FATTR_NONNULL_ARG(1);





static inline void tv_dict_set_ret(typval_T *const tv, dict_T *const d)
{
tv->v_type = VAR_DICT;
tv->vval.v_dict = d;
if (d != NULL) {
d->dv_refcount++;
}
}

static inline long tv_dict_len(const dict_T *const d)
REAL_FATTR_PURE REAL_FATTR_WARN_UNUSED_RESULT;




static inline long tv_dict_len(const dict_T *const d)
{
if (d == NULL) {
return 0L;
}
return (long)d->dv_hashtab.ht_used;
}

static inline bool tv_dict_is_watched(const dict_T *const d)
REAL_FATTR_PURE REAL_FATTR_WARN_UNUSED_RESULT;






static inline bool tv_dict_is_watched(const dict_T *const d)
{
return d && !QUEUE_EMPTY(&d->watchers);
}






static inline void tv_init(typval_T *const tv)
{
if (tv != NULL) {
memset(tv, 0, sizeof(*tv));
}
}

#define TV_INITIAL_VALUE ((typval_T) { .v_type = VAR_UNKNOWN, .v_lock = VAR_UNLOCKED, })









extern const char *const tv_empty_string;


extern bool tv_in_free_unref_items;








#define _TV_LIST_ITER_MOD(modifier, l, li, code) do { modifier list_T *const l_ = (l); list_log(l_, NULL, NULL, "iter" #modifier); if (l_ != NULL) { for (modifier listitem_T *li = l_->lv_first; li != NULL; li = li->li_next) { code } } } while (0)



















#define TV_LIST_ITER(l, li, code) _TV_LIST_ITER_MOD(, l, li, code)










#define TV_LIST_ITER_CONST(l, li, code) _TV_LIST_ITER_MOD(const, l, li, code)










#define TV_LIST_ITEM_TV(li) (&(li)->li_tv)







#define TV_LIST_ITEM_NEXT(l, li) ((li)->li_next)







#define TV_LIST_ITEM_PREV(l, li) ((li)->li_prev)









#define TV_DICT_ITER(d, di, code) HASHTAB_ITER(&(d)->dv_hashtab, di##hi_, { { dictitem_T *const di = TV_DICT_HI2DI(di##hi_); { code } } })









static inline bool tv_get_float_chk(const typval_T *const tv,
float_T *const ret_f)
REAL_FATTR_NONNULL_ALL REAL_FATTR_WARN_UNUSED_RESULT;


bool emsgf(const char *const fmt, ...);









static inline bool tv_get_float_chk(const typval_T *const tv,
float_T *const ret_f)
{
if (tv->v_type == VAR_FLOAT) {
*ret_f = tv->vval.v_float;
return true;
}
if (tv->v_type == VAR_NUMBER) {
*ret_f = (float_T)tv->vval.v_number;
return true;
}
emsgf("%s", _("E808: Number or Float required"));
return false;
}

static inline DictWatcher *tv_dict_watcher_node_data(QUEUE *q)
REAL_FATTR_NONNULL_ALL REAL_FATTR_NONNULL_RET REAL_FATTR_PURE
REAL_FATTR_WARN_UNUSED_RESULT REAL_FATTR_ALWAYS_INLINE;





static inline DictWatcher *tv_dict_watcher_node_data(QUEUE *q)
{
return QUEUE_DATA(q, DictWatcher, node);
}

static inline bool tv_is_func(const typval_T tv)
FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_ALWAYS_INLINE FUNC_ATTR_CONST;








static inline bool tv_is_func(const typval_T tv)
{
return tv.v_type == VAR_FUNC || tv.v_type == VAR_PARTIAL;
}





#define TV_TRANSLATE (SIZE_MAX)




#define TV_CSTRING (SIZE_MAX - 1)

#if defined(UNIT_TESTING)

EXTERN const size_t kTVCstring INIT(= TV_CSTRING);
EXTERN const size_t kTVTranslate INIT(= TV_TRANSLATE);
#endif

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "eval/typval.h.generated.h"
#endif
#endif 
