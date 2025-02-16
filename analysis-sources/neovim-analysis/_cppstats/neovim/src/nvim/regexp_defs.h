#include <stdbool.h>
#include "nvim/pos.h"
#include "nvim/types.h"
#include "nvim/profile.h"
#define NSUBEXP 10
#define NFA_MAX_BRACES 20
#define NFA_MAX_STATES 100000
#define NFA_TOO_EXPENSIVE -1
#define AUTOMATIC_ENGINE 0
#define BACKTRACKING_ENGINE 1
#define NFA_ENGINE 2
typedef struct regengine regengine_T;
typedef struct regprog regprog_T;
typedef struct reg_extmatch reg_extmatch_T;
typedef struct {
regprog_T *regprog;
lpos_T startpos[NSUBEXP];
lpos_T endpos[NSUBEXP];
int rmm_ic;
colnr_T rmm_maxcol; 
} regmmatch_T;
#include "nvim/buffer_defs.h"
struct regprog {
regengine_T *engine;
unsigned regflags;
unsigned re_engine; 
unsigned re_flags; 
};
typedef struct {
regengine_T *engine;
unsigned regflags;
unsigned re_engine;
unsigned re_flags; 
int regstart;
char_u reganch;
char_u *regmust;
int regmlen;
char_u reghasz;
char_u program[1]; 
} bt_regprog_T;
typedef struct nfa_state nfa_state_T;
struct nfa_state {
int c;
nfa_state_T *out;
nfa_state_T *out1;
int id;
int lastlist[2]; 
int val;
};
typedef struct {
regengine_T *engine;
unsigned regflags;
unsigned re_engine;
unsigned re_flags; 
nfa_state_T *start; 
int reganch; 
int regstart; 
char_u *match_text; 
int has_zend; 
int has_backref; 
int reghasz;
char_u *pattern;
int nsubexp; 
int nstate;
nfa_state_T state[1]; 
} nfa_regprog_T;
typedef struct {
regprog_T *regprog;
char_u *startp[NSUBEXP];
char_u *endp[NSUBEXP];
bool rm_ic;
} regmatch_T;
struct reg_extmatch {
int16_t refcnt;
char_u *matches[NSUBEXP];
};
struct regengine {
regprog_T *(*regcomp)(char_u *, int);
void (*regfree)(regprog_T *);
int (*regexec_nl)(regmatch_T *, char_u *, colnr_T, bool);
long (*regexec_multi)(regmmatch_T *, win_T *, buf_T *, linenr_T, colnr_T,
proftime_T *, int *);
char_u *expr;
};
