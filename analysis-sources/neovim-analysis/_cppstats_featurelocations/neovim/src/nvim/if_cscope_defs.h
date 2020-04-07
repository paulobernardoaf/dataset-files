#if !defined(NVIM_IF_CSCOPE_DEFS_H)
#define NVIM_IF_CSCOPE_DEFS_H












#if defined(UNIX)
#include <sys/types.h> 
#endif

#include "nvim/os/os_defs.h"
#include "nvim/os/fs_defs.h"
#include "nvim/ex_cmds_defs.h"

#define CSCOPE_SUCCESS 0
#define CSCOPE_FAILURE -1

#define CSCOPE_DBFILE "cscope.out"
#define CSCOPE_PROMPT ">> "



typedef struct {
char * name;
int (*func)(exarg_T *eap);
char * help;
char * usage;
int cansplit; 
} cscmd_T;

typedef struct csi {
char * fname; 
char * ppath; 
char * flags; 
#if defined(UNIX)
pid_t pid; 
#else
DWORD pid; 
HANDLE hProc; 
DWORD nVolume; 
DWORD nIndexHigh; 
DWORD nIndexLow;
#endif
FileID file_id;

FILE * fr_fp; 
FILE * to_fp; 
} csinfo_T;

typedef enum { Add, Find, Help, Kill, Reset, Show } csid_e;

typedef enum {
Store,
Get,
Free,
Print
} mcmd_e;

#endif 
