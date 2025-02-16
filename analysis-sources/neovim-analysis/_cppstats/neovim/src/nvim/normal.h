#include <stdbool.h>
#include "nvim/pos.h"
#include "nvim/buffer_defs.h" 
#define FIND_IDENT 1 
#define FIND_STRING 2 
#define FIND_EVAL 4 
typedef enum {
kMTCharWise = 0, 
kMTLineWise = 1, 
kMTBlockWise = 2, 
kMTUnknown = -1 
} MotionType;
typedef struct oparg_S {
int op_type; 
int regname; 
MotionType motion_type; 
int motion_force; 
bool use_reg_one; 
bool inclusive; 
bool end_adjusted; 
pos_T start; 
pos_T end; 
pos_T cursor_start; 
long line_count; 
bool empty; 
bool is_VIsual; 
colnr_T start_vcol; 
colnr_T end_vcol; 
long prev_opcount; 
long prev_count0; 
} oparg_T;
typedef struct cmdarg_S {
oparg_T *oap; 
int prechar; 
int cmdchar; 
int nchar; 
int ncharC1; 
int ncharC2; 
int extra_char; 
long opcount; 
long count0; 
long count1; 
int arg; 
int retval; 
char_u *searchbuf; 
} cmdarg_T;
#define CA_COMMAND_BUSY 1 
#define CA_NO_ADJ_OP_END 2 
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "normal.h.generated.h"
#endif
