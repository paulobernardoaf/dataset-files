#include <time.h> 
#include "nvim/pos.h"
#include "nvim/extmark_defs.h"
#include "nvim/mark_defs.h"
typedef struct u_header u_header_T;
typedef struct {
pos_T vi_start; 
pos_T vi_end; 
int vi_mode; 
colnr_T vi_curswant; 
} visualinfo_T;
#include "nvim/buffer_defs.h"
typedef struct u_entry u_entry_T;
struct u_entry {
u_entry_T *ue_next; 
linenr_T ue_top; 
linenr_T ue_bot; 
linenr_T ue_lcount; 
char_u **ue_array; 
long ue_size; 
#if defined(U_DEBUG)
int ue_magic; 
#endif
};
struct u_header {
union {
u_header_T *ptr; 
long seq;
} uh_next;
union {
u_header_T *ptr; 
long seq;
} uh_prev;
union {
u_header_T *ptr; 
long seq;
} uh_alt_next;
union {
u_header_T *ptr; 
long seq;
} uh_alt_prev;
long uh_seq; 
int uh_walk; 
u_entry_T *uh_entry; 
u_entry_T *uh_getbot_entry; 
pos_T uh_cursor; 
long uh_cursor_vcol;
int uh_flags; 
fmark_T uh_namedm[NMARKS]; 
extmark_undo_vec_t uh_extmark; 
visualinfo_T uh_visual; 
time_t uh_time; 
long uh_save_nr; 
#if defined(U_DEBUG)
int uh_magic; 
#endif
};
#define UH_CHANGED 0x01 
#define UH_EMPTYBUF 0x02 
typedef struct {
buf_T *bi_buf;
FILE *bi_fp;
} bufinfo_T;
