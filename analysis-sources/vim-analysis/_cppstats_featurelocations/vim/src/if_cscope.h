










#if defined(FEAT_CSCOPE) || defined(PROTO)

#if defined (MSWIN)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

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
dev_t st_dev; 
ino_t st_ino; 
#else
#if defined(MSWIN)
DWORD pid; 
HANDLE hProc; 
DWORD nVolume; 
DWORD nIndexHigh; 
DWORD nIndexLow;
#endif
#endif

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
