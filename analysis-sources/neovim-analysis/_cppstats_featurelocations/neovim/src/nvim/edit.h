#if !defined(NVIM_EDIT_H)
#define NVIM_EDIT_H

#include "nvim/vim.h"




#define CPT_ABBR 0 
#define CPT_MENU 1 
#define CPT_KIND 2 
#define CPT_INFO 3 
#define CPT_USER_DATA 4 
#define CPT_COUNT 5 


typedef enum {
CP_ORIGINAL_TEXT = 1, 
CP_FREE_FNAME = 2, 
CP_CONT_S_IPOS = 4, 
CP_EQUAL = 8, 
CP_ICASE = 16, 
} cp_flags_T;

typedef int (*IndentGetter)(void);


#define KEY_OPEN_FORW 0x101
#define KEY_OPEN_BACK 0x102
#define KEY_COMPLETE 0x103 


#define INDENT_SET 1 
#define INDENT_INC 2 
#define INDENT_DEC 3 


#define BL_WHITE 1 
#define BL_SOL 2 
#define BL_FIX 4 


#define INSCHAR_FORMAT 1 
#define INSCHAR_DO_COM 2 
#define INSCHAR_CTRLV 4 
#define INSCHAR_NO_FEX 8 
#define INSCHAR_COM_LIST 16 

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "edit.h.generated.h"
#endif
#endif 
