#include <stdio.h>
#include "nvim/types.h"
typedef enum {
kShellOptFilter = 1, 
kShellOptExpand = 2, 
kShellOptDoOut = 4, 
kShellOptSilent = 8, 
kShellOptRead = 16, 
kShellOptWrite = 32, 
kShellOptHideMess = 64, 
} ShellOpts;
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "os/shell.h.generated.h"
#endif
