#if !defined(NVIM_EX_CMDS_H)
#define NVIM_EX_CMDS_H

#include <stdbool.h>

#include "nvim/os/time.h"
#include "nvim/pos.h"
#include "nvim/eval/typval.h"
#include "nvim/buffer_defs.h"
#include "nvim/ex_cmds_defs.h"


#define ECMD_HIDE 0x01 
#define ECMD_SET_HELP 0x02 

#define ECMD_OLDBUF 0x04 
#define ECMD_FORCEIT 0x08 
#define ECMD_ADDBUF 0x10 



#define ECMD_LASTL (linenr_T)0 
#define ECMD_LAST (linenr_T)-1 
#define ECMD_ONE (linenr_T)1 


typedef struct {
char *sub; 
Timestamp timestamp; 
list_T *additional_elements; 
} SubReplacementString;

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "ex_cmds.h.generated.h"
#endif
#endif 
