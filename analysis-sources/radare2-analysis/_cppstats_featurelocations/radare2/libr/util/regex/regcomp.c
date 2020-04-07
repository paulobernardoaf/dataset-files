



































#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include "r_regex.h"

#include "utils.h"
#include "regex2.h"

#include "cclass.h"
#include "cname.h"





struct parse {
char *next; 
char *end; 
int error; 
sop *strip; 
sopno ssize; 
sopno slen; 
int ncsalloc; 
struct re_guts *g;
#define NPAREN 10 
sopno pbegin[NPAREN]; 
sopno pend[NPAREN]; 
};

static void p_ere(struct parse *, int);
static void p_ere_exp(struct parse *);
static void p_str(struct parse *);
static void p_bre(struct parse *, int, int);
static int p_simp_re(struct parse *, int);
static int p_count(struct parse *);
static void p_bracket(struct parse *);
static void p_b_term(struct parse *, cset *);
static void p_b_cclass(struct parse *, cset *);
static void p_b_eclass(struct parse *, cset *);
static char p_b_symbol(struct parse *);
static char p_b_coll_elem(struct parse *, int);
static char othercase(int);
static void bothcases(struct parse *, int);
static void ordinary(struct parse *, int);
static void special(struct parse *, int);
static void nonnewline(struct parse *);
static void repeat(struct parse *, sopno, int, int);
static int seterr(struct parse *, int);
static cset *allocset(struct parse *);
static void freeset(struct parse *, cset *);
static int freezeset(struct parse *, cset *);
static int firstch(struct parse *, cset *);
static int nch(struct parse *, cset *);
static void mcadd(struct parse *, cset *, char *);
static void mcinvert(struct parse *, cset *);
static void mccase(struct parse *, cset *);
static int isinsets(struct re_guts *, int);
static int samesets(struct re_guts *, int, int);
static void categorize(struct parse *, struct re_guts *);
static sopno dupl(struct parse *, sopno, sopno);
static void doemit(struct parse *, sop, size_t);
static void doinsert(struct parse *, sop, size_t, sopno);
static void dofwd(struct parse *, sopno, sop);
static void enlarge(struct parse *, sopno);
static void stripsnug(struct parse *, struct re_guts *);
static void findmust(struct parse *, struct re_guts *);
static sopno pluscount(struct parse *, struct re_guts *);

static char nuls[10]; 





#define PEEK() (*p->next)
#define PEEK2() (*(p->next+1))
#define MORE() (p->next < p->end)
#define MORE2() (p->next+1 < p->end)
#define SEE(c) (MORE() && PEEK() == (c))
#define SEETWO(a, b) (MORE() && MORE2() && PEEK() == (a) && PEEK2() == (b))
#define EAT(c) ((SEE(c)) ? (NEXT(), 1) : 0)
#define EATTWO(a, b) ((SEETWO(a, b)) ? (NEXT2(), 1) : 0)
#define NEXT() (p->next++)
#define NEXT2() (p->next += 2)
#define NEXTn(n) (p->next += (n))
#define GETNEXT() (*p->next++)
#define SETERROR(e) seterr(p, (e))
#define REQUIRE(co, e) (void)((co) || SETERROR(e))
#define MUSTSEE(c, e) (REQUIRE(MORE() && PEEK() == (c), e))
#define MUSTEAT(c, e) (REQUIRE(MORE() && GETNEXT() == (c), e))
#define MUSTNOTSEE(c, e) (REQUIRE(!MORE() || PEEK() != (c), e))
#define EMIT(op, sopnd) doemit(p, (sop)(op), (size_t)(sopnd))
#define INSERT(op, pos) doinsert(p, (sop)(op), HERE()-(pos)+1, pos)
#define AHEAD(pos) dofwd(p, pos, HERE()-(pos))
#define ASTERN(sop, pos) EMIT(sop, HERE()-(pos))
#define HERE() (p->slen)
#define THERE() (p->slen - 1)
#define THERETHERE() (p->slen - 2)
#define DROP(n) (p->slen -= (n))


R_API int r_regex_match (const char *pattern, const char *flags, const char *text) {
int ret;
RRegex rx;
int re_flags = r_regex_flags (flags);
if (r_regex_comp (&rx, pattern, re_flags)) {
eprintf ("FAIL TO COMPILE %s\n", pattern);
return 0;
}
ret = r_regex_exec (&rx, text, 0, 0, re_flags);
r_regex_fini (&rx);
return ret? 0: 1;
#if 0
regex_t preg;
regmatch_t pmatch[NUM_MATCHES];
if (regcomp(&preg, reg, REG_EXTENDED))
return -1;
return (regexec (&preg, str, NUM_MATCHES, pmatch, 0))?1:0;
#endif
}

R_API RRegex *r_regex_new (const char *pattern, const char *flags) {
RRegex rx, *r;
memset(&rx, 0, sizeof(RRegex));
if (r_regex_comp (&rx, pattern, r_regex_flags (flags))) {
return NULL;
}
r = R_NEW (RRegex);
if (!r) {
return NULL;
}
memcpy (r, &rx, sizeof (RRegex));
return r;
}

R_API int r_regex_flags(const char *f) {
int flags = 0;
if (!f || !*f) {
return 0;
}
if (strchr (f, 'e')) {
flags |= R_REGEX_EXTENDED;
}
if (strchr (f, 'i')) {
flags |= R_REGEX_ICASE;
}
if (strchr (f, 's')) {
flags |= R_REGEX_NOSUB;
}
if (strchr (f, 'n')) {
flags |= R_REGEX_NEWLINE;
}
if (strchr (f, 'N')) {
flags |= R_REGEX_NOSPEC;
}
if (strchr (f, 'p')) {
flags |= R_REGEX_PEND;
}
if (strchr (f, 'd')) {
flags |= R_REGEX_DUMP;
}
return flags;
}

R_API void r_regex_fini(RRegex *preg) {
struct re_guts *g;
if (!preg) {
return;
}
if (preg->re_magic != MAGIC1) { 
return; 
}

g = preg->re_g;
if (!g || g->magic != MAGIC2) { 
return;
}
preg->re_magic = 0; 
g->magic = 0; 

free (g->strip);
free (g->sets);
free (g->setbits);
free (g->must);
free (g);
}

R_API void r_regex_free(RRegex *preg) {
r_regex_fini (preg);
free (preg);
}





R_API int r_regex_comp(RRegex *preg, const char *pattern, int cflags) {
struct parse pa;
struct re_guts *g;
struct parse *p = &pa;
int i;
size_t len;
#if defined(REDEBUG)
#define GOODFLAGS(f) (f)
#else
#define GOODFLAGS(f) ((f)&~R_REGEX_DUMP)
#endif
cflags = GOODFLAGS (cflags);
if (!preg || ((cflags & R_REGEX_EXTENDED) && (cflags & R_REGEX_NOSPEC))) {
return R_REGEX_INVARG;
}
if (cflags & R_REGEX_PEND) {
if (preg->re_endp < pattern) {
return R_REGEX_INVARG;
}
len = preg->re_endp - pattern;
} else {
len = strlen ((char *)pattern);
}

g = (struct re_guts *)calloc (sizeof (struct re_guts) + (NC - 1), sizeof (cat_t));
if (!g) {
return R_REGEX_ESPACE;
}









size_t maxlen = ((size_t) - 1 >> 1) / sizeof (sop) * 2 / 3;
if (len >= maxlen) {
free (g);
return R_REGEX_ESPACE;
}
preg->re_flags = cflags;
p->ssize = len / (size_t)2 * (size_t)3 + (size_t)1; 
if (p->ssize < len) {
free (g);
return R_REGEX_ESPACE;
}

p->strip = (sop *)calloc (p->ssize, sizeof (sop));
if (!p->strip) {
free (g);
return R_REGEX_ESPACE;
}
p->slen = 0;
if (!p->strip) {
free (g);
return R_REGEX_ESPACE;
}


p->g = g;
p->next = (char *)pattern; 
p->end = p->next + len;
p->error = 0;
p->ncsalloc = 0;
for (i = 0; i < NPAREN; i++) {
p->pbegin[i] = 0;
p->pend[i] = 0;
}
g->csetsize = NC;
g->sets = NULL;
g->setbits = NULL;
g->ncsets = 0;
g->cflags = cflags;
g->iflags = 0;
g->nbol = 0;
g->neol = 0;
g->must = NULL;
g->mlen = 0;
g->nsub = 0;
g->ncategories = 1; 
g->categories = &g->catspace[-(CHAR_MIN)]; 
(void) memset ((char *)g->catspace, 0, NC*sizeof(cat_t));
g->backrefs = 0;


EMIT (OEND, 0);
g->firststate = THERE ();
if (cflags & R_REGEX_EXTENDED) {
p_ere(p, OUT);
} else if (cflags & R_REGEX_NOSPEC) {
p_str (p);
} else {
p_bre (p, OUT, OUT);
}
EMIT (OEND, 0);
g->laststate = THERE ();


categorize (p, g);
stripsnug (p, g);
findmust (p, g);
g->nplus = pluscount (p, g);
g->magic = MAGIC2;
preg->re_nsub = g->nsub;
preg->re_g = g;
preg->re_magic = MAGIC1;
#if !defined(REDEBUG)

if (g->iflags & BAD) {
SETERROR (R_REGEX_ASSERT);
}
#endif
if (p->error) {
r_regex_fini (preg);
}
return p->error;
}




static void p_ere(struct parse *p, int stop) { 
bool isFirst = true;
sopno prevback = 0;
sopno prevfwd = 0;
sopno conc = 0;
char c;

for (;;) {

conc = HERE();
while (MORE() && (c = PEEK()) != '|' && c != stop) {
p_ere_exp (p);
}
REQUIRE (HERE () != conc, R_REGEX_EMPTY); 

if (!EAT('|')) {
break; 
}
if (isFirst) {
INSERT (OCH_, conc); 
prevfwd = conc;
prevback = conc;
isFirst = false;
}
ASTERN(OOR1, prevback);
prevback = THERE();
AHEAD(prevfwd); 
prevfwd = HERE();
EMIT(OOR2, 0); 
}

if (!isFirst) { 
AHEAD(prevfwd);
ASTERN(O_CH, prevback);
}

}




static void p_ere_exp(struct parse *p) {
char c;
sopno pos;
int count;
int count2;
sopno subno;
int wascaret = 0;

if (!MORE()) { 
return;
}
c = GETNEXT();

pos = HERE();
switch (c) {
case '(':
REQUIRE(MORE(), R_REGEX_EPAREN);
p->g->nsub++;
subno = p->g->nsub;
if (subno < NPAREN) {
p->pbegin[subno] = HERE ();
}
EMIT(OLPAREN, subno);
if (!SEE (')')) {
p_ere (p, ')');
}
if (subno < NPAREN) {
p->pend[subno] = HERE();
if (!p->pend[subno]) {
break;
}
}
EMIT(ORPAREN, subno);
MUSTEAT(')', R_REGEX_EPAREN);
break;
case '^':
EMIT(OBOL, 0);
p->g->iflags |= USEBOL;
p->g->nbol++;
wascaret = 1;
break;
case '$':
EMIT(OEOL, 0);
p->g->iflags |= USEEOL;
p->g->neol++;
break;
case '|':
SETERROR(R_REGEX_EMPTY);
break;
case '*':
case '+':
case '?':
SETERROR(R_REGEX_BADRPT);
break;
case '.':
if (p->g->cflags & R_REGEX_NEWLINE) {
nonnewline(p);
} else {
EMIT (OANY, 0);
}
break;
case '[':
p_bracket(p);
break;
case '\\':
REQUIRE(MORE(), R_REGEX_EESCAPE);
c = GETNEXT();
if (!isalpha(c)) {
ordinary(p, c);
} else {
special(p, c);
}
break;
case '{': 
REQUIRE(!MORE() || !isdigit((ut8)PEEK()), R_REGEX_BADRPT);

default:
ordinary(p, c);
break;
}

if (!MORE ()) {
return;
}
c = PEEK();

if (!(c == '*' || c == '+' || c == '?' ||
(c == '{' && MORE2 () && isdigit ((ut8)PEEK2 ())))) {
return; 
}
NEXT();

REQUIRE(!wascaret, R_REGEX_BADRPT);
switch (c) {
case '*': 

INSERT(OPLUS_, pos);
ASTERN(O_PLUS, pos);
INSERT(OQUEST_, pos);
ASTERN(O_QUEST, pos);
break;
case '+':
INSERT(OPLUS_, pos);
ASTERN(O_PLUS, pos);
break;
case '?':

INSERT(OCH_, pos); 
ASTERN(OOR1, pos); 
AHEAD(pos); 
EMIT(OOR2, 0); 
AHEAD(THERE()); 
ASTERN(O_CH, THERETHERE());
break;
case '{':
count = p_count(p);
if (EAT(',')) {
if (isdigit((ut8)PEEK())) {
count2 = p_count(p);
REQUIRE(count <= count2, R_REGEX_BADBR);
} else { 
count2 = INTFINITY;
}
} else { 
count2 = count;
}
repeat(p, pos, count, count2);
if (!EAT('}')) { 
while (MORE () && PEEK () != '}') {
NEXT ();
}
REQUIRE(MORE(), R_REGEX_EBRACE);
SETERROR(R_REGEX_BADBR);
}
break;
}

if (!MORE ()) {
return;
}
c = PEEK();
if (!(c == '*' || c == '+' || c == '?' ||
(c == '{' && MORE2 () && isdigit ((ut8)PEEK2 ())))) {
return;
}
SETERROR(R_REGEX_BADRPT);
}




static void p_str(struct parse *p) {
REQUIRE(MORE(), R_REGEX_EMPTY);
while (MORE()) {
ordinary(p, GETNEXT());
}
}











static void p_bre(struct parse *p,
int end1, 
int end2) 
{
sopno start = HERE();
int first = 1; 
int wasdollar = 0;

if (EAT('^')) {
EMIT(OBOL, 0);
p->g->iflags |= USEBOL;
p->g->nbol++;
}
while (MORE() && !SEETWO(end1, end2)) {
wasdollar = p_simp_re(p, first);
first = 0;
}
if (wasdollar) { 
DROP(1);
EMIT(OEOL, 0);
p->g->iflags |= USEEOL;
p->g->neol++;
}

REQUIRE(HERE() != start, R_REGEX_EMPTY); 
}




static int 
p_simp_re(struct parse *p,
int starordinary) 
{
int c;
int count;
int count2;
sopno pos;
int i;
sopno subno;
#define BACKSL (1<<CHAR_BIT)

pos = HERE(); 

if (!MORE()) { 
return 0;
}
c = GETNEXT();
if (c == '\\') {
REQUIRE(MORE(), R_REGEX_EESCAPE);
c = BACKSL | GETNEXT();
}
switch (c) {
case '.':
if (p->g->cflags & R_REGEX_NEWLINE) {
nonnewline(p);
} else {
EMIT (OANY, 0);
}
break;
case '[':
p_bracket(p);
break;
case BACKSL|'{':
SETERROR(R_REGEX_BADRPT);
break;
case BACKSL|'(':
p->g->nsub++;
subno = p->g->nsub;
if (subno < NPAREN) {
p->pbegin[subno] = HERE ();
}
EMIT(OLPAREN, subno);

if (MORE () && !SEETWO ('\\', ')')) {
p_bre (p, '\\', ')');
}
if (subno < NPAREN) {
p->pend[subno] = HERE();
if (!p->pend[subno]) {
break;
}
}
EMIT(ORPAREN, subno);
REQUIRE(EATTWO('\\', ')'), R_REGEX_EPAREN);
break;
case BACKSL|')': 
case BACKSL|'}':
SETERROR(R_REGEX_EPAREN);
break;
case BACKSL|'1':
case BACKSL|'2':
case BACKSL|'3':
case BACKSL|'4':
case BACKSL|'5':
case BACKSL|'6':
case BACKSL|'7':
case BACKSL|'8':
case BACKSL|'9':
i = (c&~BACKSL) - '0';
if (p->pend[i] != 0) {
if (i <= p->g->nsub) {
EMIT(OBACK_, i);
if (p->pbegin[i] != 0 && OP(p->strip[p->pbegin[i]]) == OLPAREN &&
OP(p->strip[p->pend[i]]) == ORPAREN) {
(void) dupl(p, p->pbegin[i]+1, p->pend[i]);
EMIT(O_BACK, i);
}
}
} else {
SETERROR(R_REGEX_ESUBREG);
}
p->g->backrefs = 1;
break;
case '*':
REQUIRE(starordinary, R_REGEX_BADRPT);

default:
ordinary(p, (char)c);
break;
}

if (EAT('*')) { 

INSERT(OPLUS_, pos);
ASTERN(O_PLUS, pos);
INSERT(OQUEST_, pos);
ASTERN(O_QUEST, pos);
} else if (EATTWO('\\', '{')) {
count = p_count(p);
if (EAT(',')) {
if (MORE() && isdigit((ut8)PEEK())) {
count2 = p_count(p);
REQUIRE(count <= count2, R_REGEX_BADBR);
} else { 
count2 = INTFINITY;
}
} else { 
count2 = count;
}
repeat(p, pos, count, count2);
if (!EATTWO('\\', '}')) { 
while (MORE () && !SEETWO ('\\', '}')) {
NEXT ();
}
REQUIRE(MORE(), R_REGEX_EBRACE);
SETERROR(R_REGEX_BADBR);
}
} else if (c == '$') { 
return (1);
}

return(0);
}




static int 
p_count(struct parse *p)
{
int count = 0;
int ndigits = 0;

while (MORE() && isdigit((ut8)PEEK()) && count <= DUPMAX) {
count = count*10 + (GETNEXT() - '0');
ndigits++;
}

REQUIRE(ndigits > 0 && count <= DUPMAX, R_REGEX_BADBR);
return(count);
}







static void p_bracket(struct parse *p) {
cset *cs;
int invert = 0;


if (p->next + 5 < p->end && strncmp(p->next, "[:<:]]", 6) == 0) {
EMIT(OBOW, 0);
NEXTn(6);
return;
}
if (p->next + 5 < p->end && strncmp(p->next, "[:>:]]", 6) == 0) {
EMIT(OEOW, 0);
NEXTn(6);
return;
}

if (!(cs = allocset(p))) {

return;
}

if (EAT ('^')) {
invert++; 
}
if (EAT (']')) {
CHadd(cs, ']');
} else if (EAT ('-')) {
CHadd (cs, '-');
}
while (MORE () && PEEK () != ']' && !SEETWO ('-', ']')) {
p_b_term (p, cs);
}
if (EAT ('-')) {
CHadd (cs, '-');
}
MUSTEAT(']', R_REGEX_EBRACK);

if (p->error != 0) { 
freeset(p, cs);
return;
}

if (p->g->cflags&R_REGEX_ICASE) {
int i;
int ci;

for (i = p->g->csetsize - 1; i >= 0; i--) {
if (CHIN(cs, i) && isalpha(i)) {
ci = othercase(i);
if (ci != i) {
CHadd (cs, ci);
}
}
}
if (cs->multis != NULL) {
mccase (p, cs);
}
}
if (invert) {
int i;

for (i = p->g->csetsize - 1; i >= 0; i--) {
if (CHIN (cs, i)) {
CHsub(cs, i);
} else {
CHadd (cs, i);
}
}
if (p->g->cflags & R_REGEX_NEWLINE) {
CHsub (cs, '\n');
}
if (cs->multis != NULL) {
mcinvert (p, cs);
}
}

if (cs->multis) { 
return;
}

if (nch(p, cs) == 1) { 
ordinary(p, firstch(p, cs));
freeset(p, cs);
} else {
EMIT(OANYOF, freezeset(p, cs));
}
}




static void p_b_term(struct parse *p, cset *cs) {
char c;
char start = 0, finish;
int i;


switch ((MORE()) ? PEEK() : '\0') {
case '[':
c = (MORE2()) ? PEEK2() : '\0';
break;
case '-':
SETERROR(R_REGEX_ERANGE);
return; 
break;
default:
c = '\0';
break;
}

switch (c) {
case ':': 
NEXT2();
REQUIRE(MORE(), R_REGEX_EBRACK);
c = PEEK();
REQUIRE(c != '-' && c != ']', R_REGEX_ECTYPE);
p_b_cclass(p, cs);
REQUIRE(MORE(), R_REGEX_EBRACK);
REQUIRE(EATTWO(':', ']'), R_REGEX_ECTYPE);
break;
case '=': 
NEXT2();
REQUIRE(MORE(), R_REGEX_EBRACK);
c = PEEK();
REQUIRE(c != '-' && c != ']', R_REGEX_ECOLLATE);
p_b_eclass(p, cs);
REQUIRE(MORE(), R_REGEX_EBRACK);
REQUIRE(EATTWO('=', ']'), R_REGEX_ECOLLATE);
break;
default: 

start = p_b_symbol(p);
if (SEE('-') && MORE2() && PEEK2() != ']') {

NEXT();
if (EAT ('-')) {
finish = '-';
} else {
finish = p_b_symbol (p);
}
} else {
finish = start;
}

REQUIRE(start <= finish, R_REGEX_ERANGE);
for (i = start; i <= finish; i++) {
CHadd (cs, i);
}
break;
}
}




static void p_b_cclass(struct parse *p, cset *cs) {
char *sp = p->next;
struct cclass *cp;
size_t len;
char *u;
char c;

while (MORE () && isalpha ((unsigned char)PEEK ())) {
NEXT ();
}
len = p->next - sp;
for (cp = cclasses; cp->name != NULL; cp++) {
if (strncmp (cp->name, sp, len) == 0 && cp->name[len] == '\0') {
break;
}
}
if (!cp->name) {

SETERROR(R_REGEX_ECTYPE);
return;
}

u = cp->chars;
while ((c = *u++) != '\0') {
CHadd (cs, c);
}
for (u = cp->multis; *u != '\0'; u += strlen (u) + 1) {
MCadd (p, cs, u);
}
}






static void p_b_eclass(struct parse *p, cset *cs) {
char c;

c = p_b_coll_elem(p, '=');
CHadd(cs, c);
}




static char 
p_b_symbol(struct parse *p)
{
char value;

REQUIRE(MORE(), R_REGEX_EBRACK);
if (!EATTWO ('[', '.')) {
return (GETNEXT ());
}


value = p_b_coll_elem(p, '.');
REQUIRE(EATTWO('.', ']'), R_REGEX_ECOLLATE);
return(value);
}




static char 
p_b_coll_elem(struct parse *p,
int endc) 
{
char *sp = p->next;
struct cname *cp;
int len;

while (MORE () && !SEETWO (endc, ']')) {
NEXT ();
}
if (!MORE()) {
SETERROR(R_REGEX_EBRACK);
return(0);
}
len = p->next - sp;
for (cp = cnames; cp->name != NULL; cp++) {
if (strncmp (cp->name, sp, len) == 0 && cp->name[len] == '\0') {
return (cp->code); 
}
}
if (len == 1) {
return (*sp); 
}
SETERROR(R_REGEX_ECOLLATE); 
return(0);
}




static char 
othercase(int ch)
{
ch = (ut8)ch;
if (isalpha(ch)) {
if (isupper (ch)) {
return ((ut8)tolower(ch));
} else if (islower (ch)) {
return ((ut8)toupper(ch));
} else { 
return (ch);
}
}
return ch;
}






static void bothcases(struct parse *p, int ch) {
char *oldnext = p->next;
char *oldend = p->end;
char bracket[3];

ch = (ut8)ch;
if (othercase(ch) != ch) { 
p->next = bracket;
p->end = bracket+2;
bracket[0] = ch;
bracket[1] = ']';
bracket[2] = '\0';
p_bracket(p);
if (p->next == bracket+2) {
p->next = oldnext;
p->end = oldend;
}
}
}




static void
ordinary(struct parse *p, int ch)
{
cat_t *cap = p->g->categories;

if ((p->g->cflags & R_REGEX_ICASE) && isalpha ((ut8)ch) && othercase (ch) != ch) {
bothcases(p, ch);
} else {
EMIT(OCHAR, (ut8)ch);
if (cap[ch] == 0) {
cap[ch] = p->g->ncategories++;
}
}
}

static void
special(struct parse *p, int ch) {
char *oldnext = p->next;
char *oldend = p->end;
char bracket[16] = {0};
char digits[3] = {0};
char c;
int num = 0;
switch (ch) {
case 'x':
digits[0] = GETNEXT ();
digits[1] = GETNEXT ();
c = (char)strtol (digits, NULL, 16);
ordinary (p, c);
return;
case 'n':
ordinary (p, '\n');
return;
case 't':
ordinary (p, '\t');
return;
case 'r':
ordinary (p, '\r');
return;
case 's':
num = 5;
memcpy (bracket, "\t\r\n ]", num);
break;
case 'd':
num = 4;
memcpy (bracket, "0-9]", num);
break;
case 'w':
num = 4;
memcpy (bracket, "a-z]", num);
break;
default:
SETERROR (R_REGEX_INVARG);
return;
}

p->next = bracket;
p->end = bracket + num;

p_bracket (p);

if (p->next == bracket + num) {
p->next = oldnext;
p->end = oldend;
}
}






static void
nonnewline(struct parse *p)
{
char *oldnext = p->next;
char *oldend = p->end;
char bracket[4];

p->next = bracket;
p->end = bracket+3;
bracket[0] = '^';
bracket[1] = '\n';
bracket[2] = ']';
bracket[3] = '\0';
p_bracket(p);
if (p->next == bracket+3) {
p->next = oldnext;
p->end = oldend;
}
}




static void
repeat(struct parse *p,
sopno start, 
int from, 
int to) 
{
sopno finish = HERE();
#define N 2
#define INF 3
#define REP(f, t) ((f)*8 + (t))
#define MAP(n) (((n) <= 1) ? (n) : ((n) == INTFINITY) ? INF : N)
sopno copy;

if (p->error != 0) { 
return;
}

if (from > to) {
return;
}

switch (REP(MAP(from), MAP(to))) {
case REP(0, 0): 
DROP(finish-start); 
break;
case REP(0, 1): 
case REP(0, N): 
case REP(0, INF): 

INSERT(OCH_, start); 
repeat(p, start+1, 1, to);
ASTERN(OOR1, start);
AHEAD(start); 
EMIT(OOR2, 0);
AHEAD(THERE());
ASTERN(O_CH, THERETHERE());
break;
case REP(1, 1): 

break;
case REP(1, N): 

INSERT(OCH_, start);
ASTERN(OOR1, start);
AHEAD(start);
EMIT(OOR2, 0); 
AHEAD(THERE()); 
ASTERN(O_CH, THERETHERE());
copy = dupl(p, start+1, finish+1);
if (copy == finish+4) {
repeat(p, copy, 1, to-1);
}
break;
case REP(1, INF): 
INSERT(OPLUS_, start);
ASTERN(O_PLUS, start);
break;
case REP(N, N): 
copy = dupl(p, start, finish);
repeat(p, copy, from-1, to-1);
break;
case REP(N, INF): 
copy = dupl(p, start, finish);
repeat(p, copy, from-1, to);
break;
default: 
SETERROR(R_REGEX_ASSERT); 
break;
}
}




static int 
seterr(struct parse *p, int e)
{
if (p->error == 0) { 
p->error = e;
}
p->next = nuls; 
p->end = nuls;
return(0); 
}




static cset * allocset(struct parse *p) {
int no = p->g->ncsets++;
size_t nc;
size_t nbytes;
cset *cs;
size_t css = (size_t)p->g->csetsize;
int i;

if (no >= p->ncsalloc) { 
void *ptr;

p->ncsalloc += CHAR_BIT;
nc = p->ncsalloc;
if (nc % CHAR_BIT) {
goto nomem;
}
nbytes = nc / CHAR_BIT * css;

ptr = (cset *)realloc((char *)p->g->sets, nc * sizeof(cset));
if (!ptr) {
goto nomem;
}
p->g->sets = ptr;

ptr = (ut8 *)realloc((char *)p->g->setbits, nbytes);
if (!ptr) {
goto nomem;
}
p->g->setbits = ptr;

for (i = 0; i < no; i++) {
p->g->sets[i].ptr = p->g->setbits + css * (i / CHAR_BIT);
}

(void) memset((char *)p->g->setbits + (nbytes - css), 0, css);
}

if (!p->g->sets || !p->g->setbits) {
goto nomem;
}

cs = &p->g->sets[no];
cs->ptr = p->g->setbits + css*((no)/CHAR_BIT);
cs->mask = 1 << ((no) % CHAR_BIT);
cs->hash = 0;
cs->smultis = 0;
cs->multis = NULL;

return(cs);
nomem:
R_FREE(p->g->sets);
R_FREE(p->g->setbits);

SETERROR(R_REGEX_ESPACE);

return(NULL);
}




static void freeset(struct parse *p, cset *cs) {
int i;
cset *top = &p->g->sets[p->g->ncsets];
size_t css = (size_t)p->g->csetsize;

for (i = 0; i < css; i++) {
CHsub (cs, i);
}
if (cs == top - 1) { 
p->g->ncsets--;
}
}










static int 
freezeset(struct parse *p, cset *cs)
{
ut8 h = cs->hash;
int i;
cset *top = &p->g->sets[p->g->ncsets];
cset *cs2;
size_t css = (size_t)p->g->csetsize;


for (cs2 = &p->g->sets[0]; cs2 < top; cs2++) {
if (cs2->hash == h && cs2 != cs) {

for (i = 0; i < css; i++) {
if (!!CHIN (cs2, i) != !!CHIN (cs, i)) {
break; 
}
}
if (i == css) {
break; 
}
}
}

if (cs2 < top) { 
freeset(p, cs);
cs = cs2;
}

return((int)(cs - p->g->sets));
}




static int 
firstch(struct parse *p, cset *cs)
{
int i;
size_t css = (size_t)p->g->csetsize;

for (i = 0; i < css; i++) {
if (CHIN (cs, i)) {
return ((char)i);
}
}
return(0); 
}




static int nch(struct parse *p, cset *cs) {
int i;
size_t css = (size_t)p->g->csetsize;
int n = 0;

for (i = 0; i < css; i++) {
if (CHIN (cs, i)) {
n++;
}
}
return(n);
}




static void mcadd( struct parse *p, cset *cs, char *cp) {
size_t oldend = cs->smultis;
void *np;

cs->smultis += strlen(cp) + 1;
np = realloc(cs->multis, cs->smultis);
if (!np) {
if (cs->multis) {
free (cs->multis);
}
cs->multis = NULL;
SETERROR(R_REGEX_ESPACE);
return;
}
cs->multis = np;

STRLCPY(cs->multis + oldend - 1, cp, cs->smultis - oldend + 1);
}








static void mcinvert(struct parse *p, cset *cs) {

return;
}








static void mccase(struct parse *p, cset *cs) {

return;
}




static int 
isinsets(struct re_guts *g, int c)
{
ut8 *col;
int i;
int ncols = (g->ncsets+(CHAR_BIT-1)) / CHAR_BIT;
unsigned uc = (ut8)c;

for (i = 0, col = g->setbits; i < ncols; i++, col += g->csetsize) {
if (col[uc] != 0) {
return (1);
}
}
return(0);
}




static int 
samesets(struct re_guts *g, int c1, int c2)
{
ut8 *col;
int i;
int ncols = (g->ncsets+(CHAR_BIT-1)) / CHAR_BIT;
unsigned uc1 = (ut8)c1;
unsigned uc2 = (ut8)c2;

for (i = 0, col = g->setbits; i < ncols; i++, col += g->csetsize) {
if (col[uc1] != col[uc2]) {
return (0);
}
}
return(1);
}




static void
categorize(struct parse *p, struct re_guts *g)
{
cat_t *cats = g? g->categories : NULL;
unsigned int c;
unsigned int c2;
cat_t cat;


if (!p || p->error != 0 || !cats) {
return;
}

for (c = CHAR_MIN; c <= CHAR_MAX; c++) {
if ( *(cats+c) && isinsets(g, c)) {
cat = g->ncategories++;
cats[c] = cat;
for (c2 = c + 1; c2 <= CHAR_MAX; c2++) {
if (cats[c2] == 0 && samesets (g, c, c2)) {
cats[c2] = cat;
}
}
}
}
}




static sopno 
dupl(struct parse *p,
sopno start, 
sopno finish) 
{
sopno ret = HERE();
sopno len = finish - start;

if (finish >= start) {
if (len == 0) {
return (ret);
}
enlarge(p, p->ssize + len); 
if (p->ssize >= p->slen + len) {
(void) memcpy((char *)(p->strip + p->slen),
(char *)(p->strip + start), (size_t)len*sizeof(sop));
p->slen += len;
return(ret);
}
}
return ret;
}








static void
doemit(struct parse *p, sop op, size_t opnd)
{

if (p->error != 0) {
return;
}


if (opnd < 1<<OPSHIFT) {


if (p->slen >= p->ssize) {
enlarge (p, (p->ssize + 1) / 2 * 3); 
}
if (p->slen < p->ssize) {

p->strip[p->slen++] = SOP(op, opnd);
}
}
}




static void
doinsert(struct parse *p, sop op, size_t opnd, sopno pos)
{
sopno sn;
sop s;
int i;


if (p->error != 0) {
return;
}

sn = HERE();
EMIT(op, opnd); 
if (HERE() != sn+1) {
return;
}
s = p->strip[sn];


if (pos > 0) {
for (i = 1; i < NPAREN; i++) {
if (p->pbegin[i] >= pos) {
p->pbegin[i]++;
}
if (p->pend[i] >= pos) {
p->pend[i]++;
}
}
} 

memmove((char *)&p->strip[pos+1], (char *)&p->strip[pos],
(HERE()-pos-1)*sizeof(sop));
p->strip[pos] = s;
}




static void
dofwd(struct parse *p, sopno pos, sop value)
{

if (p->error != 0) {
return;
}

if (value < 1<<OPSHIFT) {
p->strip[pos] = OP(p->strip[pos]) | value;
}
}




static void
enlarge(struct parse *p, sopno size)
{
sop *sp;

if (p->ssize >= size) {
return;
}

sp = (sop *)realloc(p->strip, size*sizeof(sop));
if (!sp) {
SETERROR(R_REGEX_ESPACE);
return;
}
p->strip = sp;
p->ssize = size;
}




static void
stripsnug(struct parse *p, struct re_guts *g)
{
g->nstates = p->slen;
g->strip = (sop *)realloc((char *)p->strip, p->slen * sizeof(sop));
if (!g->strip) {
SETERROR(R_REGEX_ESPACE);
g->strip = p->strip;
}
}










static void
findmust(struct parse *p, struct re_guts *g)
{
sop *scan;
sop *start = NULL; 
sop *newstart = NULL; 
sopno newlen;
sop s;
char *cp;
sopno i;


if (p->error != 0) {
return;
}


newlen = 0;
start = scan = g->strip + 1;
do {
s = *scan++;
switch (OP(s)) {
case OCHAR: 
if (newlen == 0) { 
newstart = scan - 1;
}
newlen++;
break;
case OPLUS_: 
case OLPAREN:
case ORPAREN:
break;
case OQUEST_: 
case OCH_:
scan--;
do {
scan += OPND(s);
s = *scan;

if (OP(s) != O_QUEST && OP(s) != O_CH &&
OP(s) != OOR2) {
g->iflags |= BAD;
return;
}
} while (OP(s) != O_QUEST && OP(s) != O_CH);

default: 
if (newlen > g->mlen) { 
start = newstart;
g->mlen = newlen;
}
newlen = 0;
break;
}
} while (OP(s) != OEND);

if (g->mlen == 0) { 
return;
}


g->must = malloc((size_t)g->mlen + 1);
if (!g->must) { 
g->mlen = 0;
return;
}
cp = g->must;
scan = start;
for (i = g->mlen; i > 0; i--) {
while (OP (s = *scan++) != OCHAR) {
continue;
}
if (cp < g->must + g->mlen) {
*cp++ = (char)OPND(s);
}
}
if (cp == g->must + g->mlen) {
*cp++ = '\0'; 
}
}




static sopno 
pluscount(struct parse *p, struct re_guts *g)
{
sop *scan;
sop s;
sopno plusnest = 0;
sopno maxnest = 0;

if (p->error != 0) {
return (0); 
}

scan = g->strip + 1;
do {
s = *scan++;
switch (OP(s)) {
case OPLUS_:
plusnest++;
break;
case O_PLUS:
if (plusnest > maxnest) {
maxnest = plusnest;
}
plusnest--;
break;
}
} while (OP(s) != OEND);
if (plusnest != 0) {
g->iflags |= BAD;
}
return(maxnest);
}
