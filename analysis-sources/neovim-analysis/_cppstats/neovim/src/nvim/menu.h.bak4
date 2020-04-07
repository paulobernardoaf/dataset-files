#if !defined(NVIM_MENU_H)
#define NVIM_MENU_H

#include <stdbool.h> 

#include "nvim/types.h" 
#include "nvim/ex_cmds_defs.h" 




#define MENU_INDEX_INVALID -1
#define MENU_INDEX_NORMAL 0
#define MENU_INDEX_VISUAL 1
#define MENU_INDEX_SELECT 2
#define MENU_INDEX_OP_PENDING 3
#define MENU_INDEX_INSERT 4
#define MENU_INDEX_CMDLINE 5
#define MENU_INDEX_TIP 6
#define MENU_MODES 7






#define MENU_NORMAL_MODE (1 << MENU_INDEX_NORMAL)
#define MENU_VISUAL_MODE (1 << MENU_INDEX_VISUAL)
#define MENU_SELECT_MODE (1 << MENU_INDEX_SELECT)
#define MENU_OP_PENDING_MODE (1 << MENU_INDEX_OP_PENDING)
#define MENU_INSERT_MODE (1 << MENU_INDEX_INSERT)
#define MENU_CMDLINE_MODE (1 << MENU_INDEX_CMDLINE)
#define MENU_TIP_MODE (1 << MENU_INDEX_TIP)
#define MENU_ALL_MODES ((1 << MENU_INDEX_TIP) - 1)



#define MNU_HIDDEN_CHAR ']'

typedef struct VimMenu vimmenu_T;

struct VimMenu {
int modes; 
int enabled; 
char_u *name; 
char_u *dname; 
char_u *en_name; 

char_u *en_dname; 
int mnemonic; 
char_u *actext; 
long priority; 
char_u *strings[MENU_MODES]; 
int noremap[MENU_MODES]; 
bool silent[MENU_MODES]; 
vimmenu_T *children; 
vimmenu_T *parent; 
vimmenu_T *next; 
};


#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "menu.h.generated.h"
#endif
#endif 
