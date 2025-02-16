#if !defined(NVIM_DIFF_H)
#define NVIM_DIFF_H

#include "nvim/pos.h"
#include "nvim/ex_cmds_defs.h"


EXTERN int diff_context INIT(= 6); 
EXTERN int diff_foldcolumn INIT(= 2); 
EXTERN bool diff_need_scrollbind INIT(= false);

EXTERN bool need_diff_redraw INIT(= false); 

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "diff.h.generated.h"
#endif
#endif 
