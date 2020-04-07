#include <stdbool.h>
#include <stdint.h>
#include "nvim/vim.h"
#include "nvim/buffer_defs.h"
#include "nvim/eval/typval.h"
#include "nvim/normal.h"
#include "nvim/os/time.h"
#define FIND_ANY 1
#define FIND_DEFINE 2
#define CHECK_PATH 3
#define ACTION_SHOW 1
#define ACTION_GOTO 2
#define ACTION_SPLIT 3
#define ACTION_SHOW_ALL 4
#define ACTION_EXPAND 5
#define SEARCH_REV 0x01 
#define SEARCH_ECHO 0x02 
#define SEARCH_MSG 0x0c 
#define SEARCH_NFMSG 0x08 
#define SEARCH_OPT 0x10 
#define SEARCH_HIS 0x20 
#define SEARCH_END 0x40 
#define SEARCH_NOOF 0x80 
#define SEARCH_START 0x100 
#define SEARCH_MARK 0x200 
#define SEARCH_KEEP 0x400 
#define SEARCH_PEEK 0x800 
#define SEARCH_COL 0x1000 
#define FM_BACKWARD 0x01 
#define FM_FORWARD 0x02 
#define FM_BLOCKSTOP 0x04 
#define FM_SKIPCOMM 0x08 
#define RE_SEARCH 0 
#define RE_SUBST 1 
#define RE_BOTH 2 
#define RE_LAST 2 
typedef struct soffset {
char dir; 
bool line; 
bool end; 
int64_t off; 
} SearchOffset;
typedef struct spat {
char_u *pat; 
bool magic; 
bool no_scs; 
Timestamp timestamp; 
SearchOffset off; 
dict_T *additional_data; 
} SearchPattern;
typedef struct {
linenr_T sa_stop_lnum; 
proftime_T *sa_tm; 
int sa_timed_out; 
int sa_wrapped; 
} searchit_arg_T;
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "search.h.generated.h"
#endif
