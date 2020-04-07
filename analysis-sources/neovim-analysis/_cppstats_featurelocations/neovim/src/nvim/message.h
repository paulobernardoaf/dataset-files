#if !defined(NVIM_MESSAGE_H)
#define NVIM_MESSAGE_H

#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>

#include "nvim/macros.h"
#include "nvim/types.h"
#include "nvim/grid_defs.h"




#define VIM_GENERIC 0
#define VIM_ERROR 1
#define VIM_WARNING 2
#define VIM_INFO 3
#define VIM_QUESTION 4
#define VIM_LAST_TYPE 4 




#define VIM_YES 2
#define VIM_NO 3
#define VIM_CANCEL 4
#define VIM_ALL 5
#define VIM_DISCARDALL 6


#define MSG(s) msg((char_u *)(s))


#define MSG_ATTR(s, attr) msg_attr((const char *)(s), (attr))




#define EMSG(s) emsg((char_u *)(s))


#define EMSG2(s, p) emsgf((const char *) (s), (p))


#define EMSG3(s, p, q) emsgf((const char *) (s), (p), (q))


#define EMSGN(s, n) emsgf((const char *) (s), (int64_t)(n))


#define EMSGU(s, n) emsgf((const char *) (s), (uint64_t)(n))


#define IEMSG(s) iemsg((const char *)(s))


#define IEMSG2(s, p) iemsgf((const char *)(s), (p))


#define IEMSGN(s, n) iemsgf((const char *)(s), (int64_t)(n))


#define MSG_PUTS(s) msg_puts((const char *)(s))


#define MSG_PUTS_ATTR(s, a) msg_puts_attr((const char *)(s), (a))


#define MSG_PUTS_TITLE(s) msg_puts_title((const char *)(s))


#define MSG_PUTS_LONG(s) msg_puts_long_attr((char_u *)(s), 0)


#define MSG_PUTS_LONG_ATTR(s, a) msg_puts_long_attr((char_u *)(s), (a))


typedef struct msg_hist {
struct msg_hist *next; 
char_u *msg; 
const char *kind; 
int attr; 
bool multiline; 
} MessageHistoryEntry;


extern MessageHistoryEntry *first_msg_hist;

extern MessageHistoryEntry *last_msg_hist;

EXTERN bool msg_ext_need_clear INIT(= false);



EXTERN ScreenGrid msg_grid INIT(= SCREEN_GRID_INIT);
EXTERN int msg_grid_pos INIT(= 0);







EXTERN ScreenGrid msg_grid_adj INIT(= SCREEN_GRID_INIT);


EXTERN int msg_scrolled_at_flush INIT(= 0);


#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "message.h.generated.h"
#endif
#endif 
