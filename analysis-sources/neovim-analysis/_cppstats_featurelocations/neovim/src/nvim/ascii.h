#if !defined(NVIM_ASCII_H)
#define NVIM_ASCII_H

#include <stdbool.h>

#include "nvim/macros.h"
#include "nvim/func_attr.h"
#include "nvim/os/os_defs.h"



#define CharOrd(x) ((uint8_t)(x) < 'a' ? (uint8_t)(x) - 'A': (uint8_t)(x) - 'a')


#define CharOrdLow(x) ((uint8_t)(x) - 'a')
#define CharOrdUp(x) ((uint8_t)(x) - 'A')
#define ROT13(c, a) (((((c) - (a)) + 13) % 26) + (a))

#define NUL '\000'
#define BELL '\007'
#define BS '\010'
#define TAB '\011'
#define NL '\012'
#define NL_STR "\012"
#define FF '\014'
#define CAR '\015' 
#define ESC '\033'
#define ESC_STR "\033"
#define DEL 0x7f
#define DEL_STR "\177"
#define CSI 0x9b 
#define CSI_STR "\233"
#define DCS 0x90 
#define STERM 0x9c 

#define POUND 0xA3

#define Ctrl_chr(x) (TOUPPER_ASC(x) ^ 0x40) 
#define Meta(x) ((x) | 0x80)

#define CTRL_F_STR "\006"
#define CTRL_H_STR "\010"
#define CTRL_V_STR "\026"

#define Ctrl_AT 0 
#define Ctrl_A 1
#define Ctrl_B 2
#define Ctrl_C 3
#define Ctrl_D 4
#define Ctrl_E 5
#define Ctrl_F 6
#define Ctrl_G 7
#define Ctrl_H 8
#define Ctrl_I 9
#define Ctrl_J 10
#define Ctrl_K 11
#define Ctrl_L 12
#define Ctrl_M 13
#define Ctrl_N 14
#define Ctrl_O 15
#define Ctrl_P 16
#define Ctrl_Q 17
#define Ctrl_R 18
#define Ctrl_S 19
#define Ctrl_T 20
#define Ctrl_U 21
#define Ctrl_V 22
#define Ctrl_W 23
#define Ctrl_X 24
#define Ctrl_Y 25
#define Ctrl_Z 26

#define Ctrl_BSL 28 
#define Ctrl_RSB 29 
#define Ctrl_HAT 30 
#define Ctrl__ 31





#if defined(BACKSLASH_IN_FILENAME)
#define PATHSEP psepc
#define PATHSEPSTR pseps
#else
#define PATHSEP '/'
#define PATHSEPSTR "/"
#endif

static inline bool ascii_iswhite(int)
REAL_FATTR_CONST
REAL_FATTR_ALWAYS_INLINE;

static inline bool ascii_isdigit(int)
REAL_FATTR_CONST
REAL_FATTR_ALWAYS_INLINE;

static inline bool ascii_isxdigit(int)
REAL_FATTR_CONST
REAL_FATTR_ALWAYS_INLINE;

static inline bool ascii_isident(int)
REAL_FATTR_CONST
REAL_FATTR_ALWAYS_INLINE;

static inline bool ascii_isbdigit(int)
REAL_FATTR_CONST
REAL_FATTR_ALWAYS_INLINE;

static inline bool ascii_isspace(int)
REAL_FATTR_CONST
REAL_FATTR_ALWAYS_INLINE;




static inline bool ascii_iswhite(int c)
{
return c == ' ' || c == '\t';
}










static inline bool ascii_isdigit(int c)
{
return c >= '0' && c <= '9';
}




static inline bool ascii_isxdigit(int c)
{
return (c >= '0' && c <= '9')
|| (c >= 'a' && c <= 'f')
|| (c >= 'A' && c <= 'F');
}




static inline bool ascii_isident(int c)
{
return ASCII_ISALNUM(c) || c == '_';
}




static inline bool ascii_isbdigit(int c)
{
return (c == '0' || c == '1');
}





static inline bool ascii_isspace(int c)
{
return (c >= 9 && c <= 13) || c == ' ';
}

#endif 
