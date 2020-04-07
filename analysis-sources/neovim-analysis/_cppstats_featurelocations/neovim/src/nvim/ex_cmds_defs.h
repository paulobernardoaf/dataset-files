#if !defined(NVIM_EX_CMDS_DEFS_H)
#define NVIM_EX_CMDS_DEFS_H

#include <stdbool.h>
#include <stdint.h>

#include "nvim/pos.h" 
#include "nvim/normal.h"
#include "nvim/regexp_defs.h"

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "ex_cmds_enum.generated.h"
#endif

























#define RANGE 0x001 
#define BANG 0x002 
#define EXTRA 0x004 
#define XFILE 0x008 
#define NOSPC 0x010 
#define DFLALL 0x020 
#define WHOLEFOLD 0x040 

#define NEEDARG 0x080 
#define TRLBAR 0x100 
#define REGSTR 0x200 
#define COUNT 0x400 
#define NOTRLCOM 0x800 
#define ZEROR 0x1000 
#define USECTRLV 0x2000 
#define NOTADR 0x4000 
#define EDITCMD 0x8000 
#define BUFNAME 0x10000 
#define BUFUNL 0x20000 
#define ARGOPT 0x40000 
#define SBOXOK 0x80000 
#define CMDWIN 0x100000 


#define MODIFY 0x200000 
#define EXFLAGS 0x400000 
#define RESTRICT 0x800000L 
#define FILES (XFILE | EXTRA) 
#define WORD1 (EXTRA | NOSPC) 
#define FILE1 (FILES | NOSPC) 


#define ADDR_LINES 0
#define ADDR_WINDOWS 1
#define ADDR_ARGUMENTS 2
#define ADDR_LOADED_BUFFERS 3
#define ADDR_BUFFERS 4
#define ADDR_TABS 5
#define ADDR_TABS_RELATIVE 6 
#define ADDR_QUICKFIX 7
#define ADDR_OTHER 99

typedef struct exarg exarg_T;


#define BAD_REPLACE '?' 
#define BAD_KEEP -1 
#define BAD_DROP -2 

typedef void (*ex_func_T)(exarg_T *eap);

typedef char_u *(*LineGetter)(int, void *, int, bool);


typedef struct cmdname {
char_u *cmd_name; 
ex_func_T cmd_func; 
uint32_t cmd_argt; 
int cmd_addr_type; 
} CommandDefinition;




typedef struct eslist_elem eslist_T;
struct eslist_elem {
int saved_emsg_silent; 
eslist_T *next; 
};



enum {
CSTACK_LEN = 50,
};

typedef struct {
int cs_flags[CSTACK_LEN]; 
char cs_pending[CSTACK_LEN]; 
union {
void *csp_rv[CSTACK_LEN]; 
void *csp_ex[CSTACK_LEN]; 
} cs_pend;
void *cs_forinfo[CSTACK_LEN]; 
int cs_line[CSTACK_LEN]; 
int cs_idx; 
int cs_looplevel; 
int cs_trylevel; 
eslist_T *cs_emsg_silent_list; 
int cs_lflags; 
} cstack_T;
#define cs_rettv cs_pend.csp_rv
#define cs_exception cs_pend.csp_ex


enum {
CSL_HAD_LOOP = 1, 
CSL_HAD_ENDLOOP = 2, 
CSL_HAD_CONT = 4, 
CSL_HAD_FINA = 8, 
};


struct exarg {
char_u *arg; 
char_u *nextcmd; 
char_u *cmd; 
char_u **cmdlinep; 
cmdidx_T cmdidx; 
uint32_t argt; 
int skip; 
int forceit; 
int addr_count; 
linenr_T line1; 
linenr_T line2; 
int addr_type; 
int flags; 
char_u *do_ecmd_cmd; 
linenr_T do_ecmd_lnum; 
int append; 
int usefilter; 
int amount; 
int regname; 
int force_bin; 
int read_edit; 
int force_ff; 
int force_enc; 
int bad_char; 
int useridx; 
char_u *errmsg; 
LineGetter getline; 
void *cookie; 
cstack_T *cstack; 
};

#define FORCE_BIN 1 
#define FORCE_NOBIN 2 


#define EXFLAG_LIST 0x01 
#define EXFLAG_NR 0x02 
#define EXFLAG_PRINT 0x04 


struct expand {
int xp_context; 
char_u *xp_pattern; 
size_t xp_pattern_len; 
char_u *xp_arg; 
sctx_T xp_script_ctx; 
int xp_backslash; 
#if !defined(BACKSLASH_IN_FILENAME)
int xp_shell; 

#endif
int xp_numfiles; 
char_u **xp_files; 
char_u *xp_line; 
int xp_col; 
};


#define XP_BS_NONE 0 
#define XP_BS_ONE 1 
#define XP_BS_THREE 2 




typedef struct {
int split; 
int tab; 
bool browse; 
bool confirm; 
bool hide; 
bool keepalt; 
bool keepjumps; 
bool keepmarks; 
bool keeppatterns; 
bool lockmarks; 
bool noswapfile; 
char_u *save_ei; 
regmatch_T filter_regmatch; 
bool filter_force; 
} cmdmod_T;

#endif 
