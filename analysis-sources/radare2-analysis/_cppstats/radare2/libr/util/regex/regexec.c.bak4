










































#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <r_regex.h>

#include "utils.h"
#include "regex2.h"


#define states long
#define states1 states 
#define CLEAR(v) ((v) = 0)
#define SET0(v, n) ((v) &= ~((unsigned long)1 << (n)))
#define SET1(v, n) ((v) |= (unsigned long)1 << (n))
#define ISSET(v, n) (((v) & ((unsigned long)1 << (n))) != 0)
#define ASSIGN(d, s) ((d) = (s))
#define EQ(a, b) ((a) == (b))
#define STATEVARS long dummy 
#define STATESETUP(m, n) 
#define STATETEARDOWN(m) 
#define SETUP(v) ((v) = 0)
#define onestate long
#define INIT(o, n) ((o) = (unsigned long)1 << (n))
#define INC(o) ((o) <<= 1)
#define ISSTATEIN(v, o) (((v) & (o)) != 0)


#define FWD(dst, src, n) ((dst) |= ((unsigned long)(src)&(here)) << (n))
#define BACK(dst, src, n) ((dst) |= ((unsigned long)(src)&(here)) >> (n))
#define ISSETBACK(v, n) (((v) & ((unsigned long)here >> (n))) != 0)

#define SNAMES 

#include "engine.c"


#undef states
#undef CLEAR
#undef SET0
#undef SET1
#undef ISSET
#undef ASSIGN
#undef EQ
#undef STATEVARS
#undef STATESETUP
#undef STATETEARDOWN
#undef SETUP
#undef onestate
#undef INIT
#undef INC
#undef ISSTATEIN
#undef FWD
#undef BACK
#undef ISSETBACK
#undef SNAMES


#define states char *
#define CLEAR(v) memset(v, 0, m->g->nstates)
#define SET0(v, n) ((v)[n] = 0)
#define SET1(v, n) ((v)[n] = 1)
#define ISSET(v, n) ((v)[n])
#define ASSIGN(d, s) memcpy(d, s, m->g->nstates)
#define EQ(a, b) (memcmp(a, b, m->g->nstates) == 0)
#define STATEVARS long vn; char *space
#define STATESETUP(m, nv) { (m)->space = malloc((nv)*(m)->g->nstates); if (!(m)->space) return R_REGEX_ESPACE; (m)->vn = 0; }


#define STATETEARDOWN(m) { free((m)->space); }
#define SETUP(v) ((v) = &m->space[m->vn++ * m->g->nstates])
#define onestate long
#define INIT(o, n) ((o) = (n))
#define INC(o) ((o)++)
#define ISSTATEIN(v, o) ((v)[o])


#define FWD(dst, src, n) ((dst)[here+(n)] |= (src)[here])
#define BACK(dst, src, n) ((dst)[here-(n)] |= (src)[here])
#define ISSETBACK(v, n) ((v)[here - (n)])

#define LNAMES 

#include "engine.c"


R_API bool r_regex_check(const RRegex *rr, const char *str) {
return r_regex_exec (rr, str, 0, NULL, rr->re_flags);
}







int 
r_regex_exec(const RRegex *preg, const char *string, size_t nmatch,
RRegexMatch pmatch[], int eflags)
{
struct re_guts *g;
#if defined(REDEBUG)
#define GOODFLAGS(f) (f)
#else
#define GOODFLAGS(f) ((f)&(R_REGEX_NOTBOL|R_REGEX_NOTEOL|R_REGEX_STARTEND))
#endif
if (!preg || !string) {
return R_REGEX_ASSERT;
}

g = preg->re_g;
if (preg->re_magic != MAGIC1 || g->magic != MAGIC2) {
return (R_REGEX_BADPAT);
}
if (g->iflags & BAD) { 
return (R_REGEX_BADPAT);
}
eflags = GOODFLAGS(eflags);
if (g->nstates <= CHAR_BIT * sizeof (states1) && !(eflags & R_REGEX_LARGE)) {
return(smatcher(g, (char *)string, nmatch, pmatch, eflags));
} else {
return (lmatcher (g, (char *)string, nmatch, pmatch, eflags));
}
}
