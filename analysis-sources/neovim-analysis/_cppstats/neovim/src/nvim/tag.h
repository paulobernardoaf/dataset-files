#include "nvim/types.h"
#include "nvim/ex_cmds_defs.h"
#define DT_TAG 1 
#define DT_POP 2 
#define DT_NEXT 3 
#define DT_PREV 4 
#define DT_FIRST 5 
#define DT_LAST 6 
#define DT_SELECT 7 
#define DT_HELP 8 
#define DT_JUMP 9 
#define DT_CSCOPE 10 
#define DT_LTAG 11 
#define DT_FREE 99 
#define TAG_HELP 1 
#define TAG_NAMES 2 
#define TAG_REGEXP 4 
#define TAG_NOIC 8 
#define TAG_CSCOPE 16 
#define TAG_VERBOSE 32 
#define TAG_INS_COMP 64 
#define TAG_KEEP_LANG 128 
#define TAG_NO_TAGFUNC 256 
#define TAG_MANY 300 
typedef struct {
char_u *tn_tags; 
char_u *tn_np; 
int tn_did_filefind_init;
int tn_hf_idx;
void *tn_search_ctx;
} tagname_T;
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "tag.h.generated.h"
#endif
