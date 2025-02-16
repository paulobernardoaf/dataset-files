#if !defined(NVIM_VERSION_H)
#define NVIM_VERSION_H

#include "nvim/ex_cmds_defs.h"


extern char* Version;
extern char* longVersion;




#define VIM_VERSION_MAJOR 8
#define VIM_VERSION_MINOR 0
#define VIM_VERSION_100 (VIM_VERSION_MAJOR * 100 + VIM_VERSION_MINOR)


#define VIM_VERSION_NODOT "vim80"

#define VIM_VERSION_SHORT "8.0"

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "version.h.generated.h"
#endif
#endif 
