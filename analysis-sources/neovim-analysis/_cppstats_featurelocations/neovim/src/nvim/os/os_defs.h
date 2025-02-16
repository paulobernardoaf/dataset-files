#if !defined(NVIM_OS_OS_DEFS_H)
#define NVIM_OS_OS_DEFS_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(WIN32)
#include "nvim/os/win_defs.h"
#else
#include "nvim/os/unix_defs.h"
#endif

#define BASENAMELEN (NAME_MAX - 5)


#if defined(PATH_MAX) && (PATH_MAX > 1024)
#define MAXPATHL PATH_MAX
#else
#define MAXPATHL 1024
#endif


#define CMDBUFFSIZE 1024



#include <string.h>
#if defined(HAVE_STRINGS_H) && !defined(NO_STRINGS_WITH_STRING_H)
#include <strings.h>
#endif


#define os_strerror uv_strerror


#define os_translate_sys_error uv_translate_sys_error

#if defined(WIN32)
#define os_strtok strtok_s
#else
#define os_strtok strtok_r
#endif

#endif 
