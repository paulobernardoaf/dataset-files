#if !defined(NVIM_SIGN_DEFS_H)
#define NVIM_SIGN_DEFS_H

#include "nvim/pos.h"
#include "nvim/types.h"




typedef struct signgroup_S
{
uint16_t refcount; 
int next_sign_id; 
char_u sg_name[1]; 
} signgroup_T;


#define SGN_KEY_OFF offsetof(signgroup_T, sg_name)
#define HI2SG(hi) ((signgroup_T *)((hi)->hi_key - SGN_KEY_OFF))

typedef struct signlist signlist_T;

struct signlist
{
int id; 
linenr_T lnum; 
int typenr; 
signgroup_T *group; 
int priority; 
signlist_T *next; 
signlist_T *prev; 
};


#define SIGN_DEF_PRIO 10


typedef enum {
SIGN_ANY,
SIGN_LINEHL,
SIGN_ICON,
SIGN_TEXT,
SIGN_NUMHL,
} SignType;



#endif 
