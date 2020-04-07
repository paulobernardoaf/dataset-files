#if !defined(NVIM_VIM_H)
#define NVIM_VIM_H

#include "nvim/types.h"
#include "nvim/pos.h" 


#define SESSION_FILE "Session.vim"
#define MAX_MSG_HIST_LEN 200
#define SYS_OPTWIN_FILE "$VIMRUNTIME/optwin.vim"
#define RUNTIME_DIRNAME "runtime"


#include "auto/config.h"
#define HAVE_PATHDEF




#if (SIZEOF_INT == 0)
#error Configure did not run properly.
#endif

#include "nvim/os/os_defs.h" 


enum { NUMBUFLEN = 65 };

#define MAX_TYPENR 65535

#define ROOT_UID 0

#include "nvim/keymap.h"
#include "nvim/macros.h"

#include "nvim/gettext.h"


#define MSG_HIST 0x1000









#define NORMAL 0x01 
#define VISUAL 0x02 
#define OP_PENDING 0x04 

#define CMDLINE 0x08 
#define INSERT 0x10 
#define LANGMAP 0x20 


#define REPLACE_FLAG 0x40 
#define REPLACE (REPLACE_FLAG + INSERT)
#define VREPLACE_FLAG 0x80 
#define VREPLACE (REPLACE_FLAG + VREPLACE_FLAG + INSERT)
#define LREPLACE (REPLACE_FLAG + LANGMAP)

#define NORMAL_BUSY (0x100 + NORMAL) 
#define HITRETURN (0x200 + NORMAL) 
#define ASKMORE 0x300 
#define SETWSIZE 0x400 
#define ABBREV 0x500 
#define EXTERNCMD 0x600 
#define SHOWMATCH (0x700 + INSERT) 
#define CONFIRM 0x800 
#define SELECTMODE 0x1000 
#define TERM_FOCUS 0x2000 
#define CMDPREVIEW 0x4000 


#define MAP_ALL_MODES (0x3f | SELECTMODE | TERM_FOCUS)


typedef enum {
kDirectionNotSet = 0,
FORWARD = 1,
BACKWARD = (-1),
FORWARD_FILE = 3,
BACKWARD_FILE = (-3),
} Direction;


#if !(defined(OK) && (OK == 1))

#define OK 1
#endif
#define FAIL 0
#define NOTDONE 2 


#define VAR_TYPE_NUMBER 0
#define VAR_TYPE_STRING 1
#define VAR_TYPE_FUNC 2
#define VAR_TYPE_LIST 3
#define VAR_TYPE_DICT 4
#define VAR_TYPE_FLOAT 5
#define VAR_TYPE_BOOL 6




enum {
EXPAND_UNSUCCESSFUL = -2,
EXPAND_OK = -1,
EXPAND_NOTHING = 0,
EXPAND_COMMANDS,
EXPAND_FILES,
EXPAND_DIRECTORIES,
EXPAND_SETTINGS,
EXPAND_BOOL_SETTINGS,
EXPAND_TAGS,
EXPAND_OLD_SETTING,
EXPAND_HELP,
EXPAND_BUFFERS,
EXPAND_EVENTS,
EXPAND_MENUS,
EXPAND_SYNTAX,
EXPAND_HIGHLIGHT,
EXPAND_AUGROUP,
EXPAND_USER_VARS,
EXPAND_MAPPINGS,
EXPAND_TAGS_LISTFILES,
EXPAND_FUNCTIONS,
EXPAND_USER_FUNC,
EXPAND_EXPRESSION,
EXPAND_MENUNAMES,
EXPAND_USER_COMMANDS,
EXPAND_USER_CMD_FLAGS,
EXPAND_USER_NARGS,
EXPAND_USER_COMPLETE,
EXPAND_ENV_VARS,
EXPAND_LANGUAGE,
EXPAND_COLORS,
EXPAND_COMPILER,
EXPAND_USER_DEFINED,
EXPAND_USER_LIST,
EXPAND_SHELLCMD,
EXPAND_CSCOPE,
EXPAND_SIGN,
EXPAND_PROFILE,
EXPAND_BEHAVE,
EXPAND_FILETYPE,
EXPAND_FILES_IN_PATH,
EXPAND_OWNSYNTAX,
EXPAND_LOCALES,
EXPAND_HISTORY,
EXPAND_USER,
EXPAND_SYNTIME,
EXPAND_USER_ADDR_TYPE,
EXPAND_PACKADD,
EXPAND_MESSAGES,
EXPAND_MAPCLEAR,
EXPAND_ARGLIST,
EXPAND_CHECKHEALTH,
};









#define MIN_SWAP_PAGE_SIZE 1048
#define MAX_SWAP_PAGE_SIZE 50000





#if !defined(TRUE)
#define FALSE 0 
#define TRUE 1
#endif

#define MAYBE 2 

#define STATUS_HEIGHT 1 
#define QF_WINHEIGHT 10 




#if !defined(CMDBUFFSIZE)
#define CMDBUFFSIZE 256 
#endif

#define LSIZE 512 

#define DIALOG_MSG_SIZE 1000 

enum { FOLD_TEXT_LEN = 51 }; 





#define MAXMAPLEN 50


#define UNDO_HASH_SIZE 32





#define STRLEN(s) strlen((char *)(s))
#define STRCPY(d, s) strcpy((char *)(d), (char *)(s))
#define STRNCPY(d, s, n) strncpy((char *)(d), (char *)(s), (size_t)(n))
#define STRLCPY(d, s, n) xstrlcpy((char *)(d), (char *)(s), (size_t)(n))
#define STRCMP(d, s) strcmp((char *)(d), (char *)(s))
#define STRNCMP(d, s, n) strncmp((char *)(d), (char *)(s), (size_t)(n))
#if defined(HAVE_STRCASECMP)
#define STRICMP(d, s) strcasecmp((char *)(d), (char *)(s))
#else
#if defined(HAVE_STRICMP)
#define STRICMP(d, s) stricmp((char *)(d), (char *)(s))
#else
#define STRICMP(d, s) vim_stricmp((char *)(d), (char *)(s))
#endif
#endif


#define STRMOVE(d, s) memmove((d), (s), STRLEN(s) + 1)

#if defined(HAVE_STRNCASECMP)
#define STRNICMP(d, s, n) strncasecmp((char *)(d), (char *)(s), (size_t)(n))
#else
#if defined(HAVE_STRNICMP)
#define STRNICMP(d, s, n) strnicmp((char *)(d), (char *)(s), (size_t)(n))
#else
#define STRNICMP(d, s, n) vim_strnicmp((char *)(d), (char *)(s), (size_t)(n))
#endif
#endif

#define STRRCHR(s, c) (char_u *)strrchr((const char *)(s), (c))

#define STRCAT(d, s) strcat((char *)(d), (char *)(s))
#define STRNCAT(d, s, n) strncat((char *)(d), (char *)(s), (size_t)(n))
#define STRLCAT(d, s, n) xstrlcat((char *)(d), (char *)(s), (size_t)(n))

#define vim_strpbrk(s, cs) (char_u *)strpbrk((char *)(s), (char *)(cs))

#include "nvim/message.h"



#define PERROR(msg) (void) emsgf("%s: %s", msg, strerror(errno))

#define SHOWCMD_COLS 10 
#define STL_MAX_ITEM 80 













#define fnamecmp(x, y) path_fnamecmp((const char *)(x), (const char *)(y))
#define fnamencmp(x, y, n) path_fnamencmp((const char *)(x), (const char *)(y), (size_t)(n))





#define HL_ATTR(n) highlight_attr[(int)(n)]




#define MB_MAXBYTES 21





#if !defined(WIN32)
#define mch_errmsg(str) fprintf(stderr, "%s", (str))
#define mch_msg(str) printf("%s", (str))
#endif

#include "nvim/globals.h" 
#include "nvim/buffer_defs.h" 
#include "nvim/ex_cmds_defs.h" 


#define DIP_ALL 0x01 
#define DIP_DIR 0x02 
#define DIP_ERR 0x04 
#define DIP_START 0x08 
#define DIP_OPT 0x10 
#define DIP_NORTP 0x20 
#define DIP_NOAFTER 0x40 
#define DIP_AFTER 0x80 


#define LOWEST_WIN_ID 1000


#if (defined(BSD) || defined(__FreeBSD_kernel__)) && defined(S_ISCHR)
#define OPEN_CHR_FILES
#endif


#define REPLACE_CR_NCHAR -1
#define REPLACE_NL_NCHAR -2

#endif 
