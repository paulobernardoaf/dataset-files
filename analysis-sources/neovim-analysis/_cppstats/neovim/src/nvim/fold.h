#include <stdio.h>
#include "nvim/pos.h"
#include "nvim/garray.h"
#include "nvim/types.h"
#include "nvim/buffer_defs.h"
typedef struct foldinfo {
linenr_T fi_lnum; 
int fi_level; 
int fi_low_level; 
} foldinfo_T;
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "fold.h.generated.h"
#endif
