#include "nvim/ex_cmds_defs.h" 
#define BCO_ENTER 1 
#define BCO_ALWAYS 2 
#define BCO_NOHELP 4 
typedef enum {
OPT_FREE = 1, 
OPT_GLOBAL = 2, 
OPT_LOCAL = 4, 
OPT_MODELINE = 8, 
OPT_WINONLY = 16, 
OPT_NOWIN = 32, 
} OptionFlags;
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "option.h.generated.h"
#endif
