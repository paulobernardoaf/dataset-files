#include <stdbool.h>
#include "nvim/macros.h"
#include "nvim/ascii.h"
#include "nvim/types.h"
#include "nvim/eval/typval.h"
#include "nvim/os/time.h"
#include "nvim/normal.h" 
#include "nvim/ex_cmds_defs.h" 
typedef int (*Indenter)(void);
#define PUT_FIXINDENT 1 
#define PUT_CURSEND 2 
#define PUT_CURSLINE 4 
#define PUT_LINE 8 
#define PUT_LINE_SPLIT 16 
#define PUT_LINE_FORWARD 32 
#define DELETION_REGISTER 36
#define NUM_SAVED_REGISTERS 37
#define STAR_REGISTER 37
#define PLUS_REGISTER 38
#define NUM_REGISTERS 39
#define OP_NOP 0 
#define OP_DELETE 1 
#define OP_YANK 2 
#define OP_CHANGE 3 
#define OP_LSHIFT 4 
#define OP_RSHIFT 5 
#define OP_FILTER 6 
#define OP_TILDE 7 
#define OP_INDENT 8 
#define OP_FORMAT 9 
#define OP_COLON 10 
#define OP_UPPER 11 
#define OP_LOWER 12 
#define OP_JOIN 13 
#define OP_JOIN_NS 14 
#define OP_ROT13 15 
#define OP_REPLACE 16 
#define OP_INSERT 17 
#define OP_APPEND 18 
#define OP_FOLD 19 
#define OP_FOLDOPEN 20 
#define OP_FOLDOPENREC 21 
#define OP_FOLDCLOSE 22 
#define OP_FOLDCLOSEREC 23 
#define OP_FOLDDEL 24 
#define OP_FOLDDELREC 25 
#define OP_FORMAT2 26 
#define OP_FUNCTION 27 
#define OP_NR_ADD 28 
#define OP_NR_SUB 29 
enum GRegFlags {
kGRegNoExpr = 1, 
kGRegExprSrc = 2, 
kGRegList = 4 
};
typedef struct yankreg {
char_u **y_array; 
size_t y_size; 
MotionType y_type; 
colnr_T y_width; 
Timestamp timestamp; 
dict_T *additional_data; 
} yankreg_T;
static inline int op_reg_index(const int regname)
FUNC_ATTR_CONST
{
if (ascii_isdigit(regname)) {
return regname - '0';
} else if (ASCII_ISLOWER(regname)) {
return CharOrdLow(regname) + 10;
} else if (ASCII_ISUPPER(regname)) {
return CharOrdUp(regname) + 10;
} else if (regname == '-') {
return DELETION_REGISTER;
} else if (regname == '*') {
return STAR_REGISTER;
} else if (regname == '+') {
return PLUS_REGISTER;
} else {
return -1;
}
}
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "ops.h.generated.h"
#endif
