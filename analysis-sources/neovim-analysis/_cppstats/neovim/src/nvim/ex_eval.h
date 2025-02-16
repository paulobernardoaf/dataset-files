#include "nvim/pos.h" 
#include "nvim/ex_cmds_defs.h" 
#define CSF_TRUE 0x0001 
#define CSF_ACTIVE 0x0002 
#define CSF_ELSE 0x0004 
#define CSF_WHILE 0x0008 
#define CSF_FOR 0x0010 
#define CSF_TRY 0x0100 
#define CSF_FINALLY 0x0200 
#define CSF_THROWN 0x0400 
#define CSF_CAUGHT 0x0800 
#define CSF_SILENT 0x1000 
#define CSTP_NONE 0 
#define CSTP_ERROR 1 
#define CSTP_INTERRUPT 2 
#define CSTP_THROW 4 
#define CSTP_BREAK 8 
#define CSTP_CONTINUE 16 
#define CSTP_RETURN 24 
#define CSTP_FINISH 32 
struct msglist {
char_u *msg; 
char_u *throw_msg; 
struct msglist *next; 
};
typedef enum
{
ET_USER, 
ET_ERROR, 
ET_INTERRUPT, 
} except_type_T;
typedef struct vim_exception except_T;
struct vim_exception {
except_type_T type; 
char_u *value; 
struct msglist *messages; 
char_u *throw_name; 
linenr_T throw_lnum; 
except_T *caught; 
};
typedef struct cleanup_stuff cleanup_T;
struct cleanup_stuff {
int pending; 
except_T *exception; 
};
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "ex_eval.h.generated.h"
#endif
