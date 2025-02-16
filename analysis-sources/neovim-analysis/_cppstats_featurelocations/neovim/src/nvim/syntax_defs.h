#if !defined(NVIM_SYNTAX_DEFS_H)
#define NVIM_SYNTAX_DEFS_H

#include "nvim/highlight_defs.h"

#define SST_MIN_ENTRIES 150 
#define SST_MAX_ENTRIES 1000 
#define SST_FIX_STATES 7 
#define SST_DIST 16 
#define SST_INVALID (synstate_T *)-1 

typedef struct syn_state synstate_T;

#include "nvim/buffer_defs.h"
#include "nvim/regexp_defs.h"


struct sp_syn {
int inc_tag; 
int16_t id; 
int16_t *cont_in_list; 
};




typedef struct keyentry keyentry_T;

struct keyentry {
keyentry_T *ke_next; 
struct sp_syn k_syn; 
int16_t *next_list; 
int flags;
int k_char; 
char_u keyword[1]; 
};




typedef struct buf_state {
int bs_idx; 
int bs_flags; 
int bs_seqnr; 
int bs_cchar; 
reg_extmatch_T *bs_extmatch; 
} bufstate_T;





struct syn_state {
synstate_T *sst_next; 
linenr_T sst_lnum; 
union {
bufstate_T sst_stack[SST_FIX_STATES]; 
garray_T sst_ga; 
} sst_union;
int sst_next_flags; 
int sst_stacksize; 
int16_t *sst_next_list; 

disptick_T sst_tick; 
linenr_T sst_change_lnum; 

};

#endif 
