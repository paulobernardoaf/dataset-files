#if !defined(NVIM_EVAL_H)
#define NVIM_EVAL_H

#include "nvim/buffer_defs.h"
#include "nvim/channel.h"
#include "nvim/eval/funcs.h" 
#include "nvim/event/time.h" 
#include "nvim/ex_cmds_defs.h" 
#include "nvim/os/fileio.h" 
#include "nvim/os/stdpaths_defs.h" 

#define COPYID_INC 2
#define COPYID_MASK (~0x1)


extern hashtab_T func_hashtab;


typedef struct {
dict_T *fd_dict; 
char_u *fd_newkey; 
dictitem_T *fd_di; 
} funcdict_T;


EXTERN ufunc_T dumuf;
#define UF2HIKEY(fp) ((fp)->uf_name)
#define HIKEY2UF(p) ((ufunc_T *)(p - offsetof(ufunc_T, uf_name)))
#define HI2UF(hi) HIKEY2UF((hi)->hi_key)




























typedef struct lval_S {
const char *ll_name; 
size_t ll_name_len; 
char *ll_exp_name; 
typval_T *ll_tv; 

listitem_T *ll_li; 
list_T *ll_list; 
int ll_range; 
long ll_n1; 
long ll_n2; 
int ll_empty2; 
dict_T *ll_dict; 
dictitem_T *ll_di; 
char_u *ll_newkey; 
} lval_T;


typedef enum {
VAR_FLAVOUR_DEFAULT = 1, 
VAR_FLAVOUR_SESSION = 2, 
VAR_FLAVOUR_SHADA = 4 
} var_flavour_T;


typedef enum {
VV_COUNT,
VV_COUNT1,
VV_PREVCOUNT,
VV_ERRMSG,
VV_WARNINGMSG,
VV_STATUSMSG,
VV_SHELL_ERROR,
VV_THIS_SESSION,
VV_VERSION,
VV_LNUM,
VV_TERMRESPONSE,
VV_FNAME,
VV_LANG,
VV_LC_TIME,
VV_CTYPE,
VV_CC_FROM,
VV_CC_TO,
VV_FNAME_IN,
VV_FNAME_OUT,
VV_FNAME_NEW,
VV_FNAME_DIFF,
VV_CMDARG,
VV_FOLDSTART,
VV_FOLDEND,
VV_FOLDDASHES,
VV_FOLDLEVEL,
VV_PROGNAME,
VV_SEND_SERVER,
VV_DYING,
VV_EXCEPTION,
VV_THROWPOINT,
VV_STDERR,
VV_REG,
VV_CMDBANG,
VV_INSERTMODE,
VV_VAL,
VV_KEY,
VV_PROFILING,
VV_FCS_REASON,
VV_FCS_CHOICE,
VV_BEVAL_BUFNR,
VV_BEVAL_WINNR,
VV_BEVAL_WINID,
VV_BEVAL_LNUM,
VV_BEVAL_COL,
VV_BEVAL_TEXT,
VV_SCROLLSTART,
VV_SWAPNAME,
VV_SWAPCHOICE,
VV_SWAPCOMMAND,
VV_CHAR,
VV_MOUSE_WIN,
VV_MOUSE_WINID,
VV_MOUSE_LNUM,
VV_MOUSE_COL,
VV_OP,
VV_SEARCHFORWARD,
VV_HLSEARCH,
VV_OLDFILES,
VV_WINDOWID,
VV_PROGPATH,
VV_COMPLETED_ITEM,
VV_OPTION_NEW,
VV_OPTION_OLD,
VV_OPTION_TYPE,
VV_ERRORS,
VV_MSGPACK_TYPES,
VV_EVENT,
VV_FALSE,
VV_TRUE,
VV_NULL,
VV__NULL_LIST, 
VV__NULL_DICT, 
VV_VIM_DID_ENTER,
VV_TESTING,
VV_TYPE_NUMBER,
VV_TYPE_STRING,
VV_TYPE_FUNC,
VV_TYPE_LIST,
VV_TYPE_DICT,
VV_TYPE_FLOAT,
VV_TYPE_BOOL,
VV_ECHOSPACE,
VV_EXITING,
VV_LUA,
} VimVarIndex;


typedef enum {
kMPNil,
kMPBoolean,
kMPInteger,
kMPFloat,
kMPString,
kMPBinary,
kMPArray,
kMPMap,
kMPExt,
#define LAST_MSGPACK_TYPE kMPExt
} MessagePackType;


extern const list_T *eval_msgpack_type_lists[LAST_MSGPACK_TYPE + 1];

#undef LAST_MSGPACK_TYPE

typedef int (*ArgvFunc)(int current_argcount, typval_T *argv,
int called_func_argcount);


typedef enum {
TFN_INT = 1, 
TFN_QUIET = 2, 
TFN_NO_AUTOLOAD = 4, 
TFN_NO_DEREF = 8, 
TFN_READ_ONLY = 16, 
} TransFunctionNameFlags;


typedef enum {
GLV_QUIET = TFN_QUIET, 
GLV_NO_AUTOLOAD = TFN_NO_AUTOLOAD, 
GLV_READ_ONLY = TFN_READ_ONLY, 

} GetLvalFlags;


#define FNE_INCL_BR 1 
#define FNE_CHECK_START 2 


typedef struct {
TimeWatcher tw;
int timer_id;
int repeat_count;
int refcount;
int emsg_count; 
long timeout;
bool stopped;
bool paused;
Callback callback;
} timer_T;


typedef enum
{
ASSERT_EQUAL,
ASSERT_NOTEQUAL,
ASSERT_MATCH,
ASSERT_NOTMATCH,
ASSERT_INRANGE,
ASSERT_OTHER,
} assert_type_T;


typedef enum {
kDictListKeys, 
kDictListValues, 
kDictListItems, 
} DictListType;

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "eval.h.generated.h"
#endif
#endif 
