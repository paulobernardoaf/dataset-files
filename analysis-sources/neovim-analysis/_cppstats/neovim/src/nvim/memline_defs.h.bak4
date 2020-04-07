#if !defined(NVIM_MEMLINE_DEFS_H)
#define NVIM_MEMLINE_DEFS_H

#include "nvim/memfile_defs.h"






typedef struct info_pointer {
blocknr_T ip_bnum; 
linenr_T ip_low; 
linenr_T ip_high; 
int ip_index; 
} infoptr_T; 

typedef struct ml_chunksize {
int mlcs_numlines;
long mlcs_totalsize;
} chunksize_T;


#define ML_CHNK_ADDLINE 1
#define ML_CHNK_DELLINE 2
#define ML_CHNK_UPDLINE 3
















typedef struct memline {
linenr_T ml_line_count; 

memfile_T *ml_mfp; 

#define ML_EMPTY 1 
#define ML_LINE_DIRTY 2 
#define ML_LOCKED_DIRTY 4 
#define ML_LOCKED_POS 8 
int ml_flags;

infoptr_T *ml_stack; 
int ml_stack_top; 
int ml_stack_size; 

linenr_T ml_line_lnum; 
char_u *ml_line_ptr; 

bhdr_T *ml_locked; 
linenr_T ml_locked_low; 
linenr_T ml_locked_high; 
int ml_locked_lineadd; 
chunksize_T *ml_chunksize;
int ml_numchunks;
int ml_usedchunks;
} memline_T;

#endif 
