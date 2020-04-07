

#include <r_egg.h>

#define CTX egg->context

static inline int is_var(char *x) {
return x[0] == '.' || ((x[0] == '*' || x[0] == '&') && x[1] == '.');
}

static inline int is_space(char c) {
return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static const char *skipspaces(const char *s) {
while (is_space (*s)) {
s++;
}
return s;
}

static inline int is_op(char x) {
switch (x) {
case '-':
case '+':
return 1;
case '*':
case '/':
return 2;
case '^':
case '|':
case '&':
return 3;
default:
return 0;
}
}

static inline int get_op(char **pos){
while (**pos && !(is_op (**pos) && !is_var (*pos))) {
(*pos)++;
}
return (**pos)? ((is_op (**pos)) + 1): 0;
}



static char *trim(char *s) {
char *o;
for (o = s; *o; o++) {
if (is_space (*o)) {
*o = 0;
}
}
return s;
}

static void rcc_pushstr(REgg *egg, char *str, int filter);
static void rcc_context(REgg *egg, int delta);

enum {
NORMAL = 0,
ALIAS,
INCLUDE,
DATA,
INLINE,
NAKED,
SYSCALL,
SYSCALLBODY,
GOTO,
LAST
};

static char *find_include(const char *prefix, const char *file) {
char *pfx = NULL, *ret = NULL, *env = r_sys_getenv (R_EGG_INCDIR_ENV);
if (!prefix) {
prefix = "";
}
if (*prefix == '$') {
char *out = r_sys_getenv (prefix + 1);
pfx = out? out: strdup ("");
} else {
pfx = strdup (prefix);
if (!pfx) {
free (env);
return NULL;
}
}

if (env) {
char *str, *ptr = strchr (env, ':');

str = env;
while (str) {
if (ptr) {
*ptr = 0;
}
free (ret);
ret = r_str_appendf (NULL, "%s/%s", pfx, file);
{
char *filepath = r_str_appendf (NULL, "%s/%s/%s", str, pfx, file);

if (r_file_exists (filepath)) {
free (env);
free (pfx);
free (ret);
return filepath;
}
free (filepath);
}
if (!ptr) {
break;
}
str = ptr + 1;
ptr = strchr (str, ':');
}
free (env);
} else {
ret = r_str_appendf (NULL, "%s/%s", pfx, file);
}
free (pfx);
return ret;
}

R_API void r_egg_lang_init(REgg *egg) {
egg->lang.varsize = 'l';

egg->lang.docall = 1;
egg->lang.line = 1;
egg->lang.file = "stdin";
egg->lang.oc = '\n';
egg->lang.mode = NORMAL;
}

R_API void r_egg_lang_free(REgg *egg) {
int i, len;

for (i = 0; i < egg->lang.nsyscalls; i++) {
R_FREE (egg->lang.syscalls[i].name);
R_FREE (egg->lang.syscalls[i].arg);
}
len = sizeof (egg->lang.ctxpush) / sizeof (char *);
for (i = 0; i < len; i++) {
R_FREE (egg->lang.ctxpush[i]);
}
}

R_API void r_egg_lang_include_path(REgg *egg, const char *path) {
char *tmp_ptr = NULL;
char *env = r_sys_getenv (R_EGG_INCDIR_ENV);
if (!env || !*env) {
r_egg_lang_include_init (egg);
free (env);
env = r_sys_getenv (R_EGG_INCDIR_ENV);
tmp_ptr = env;
}
env = r_str_appendf (NULL, "%s:%s", path, env);
free (tmp_ptr);
r_sys_setenv (R_EGG_INCDIR_ENV, env);
free (env);
}

R_API void r_egg_lang_include_init(REgg *egg) {
char *s = r_str_newf (".:%s/%s", r_sys_prefix (NULL), R_EGG_INCDIR_PATH);
r_sys_setenv (R_EGG_INCDIR_ENV, s);
free (s);
}

static void rcc_set_callname(REgg *egg, const char *s) {
R_FREE (egg->lang.callname);
egg->lang.nargs = 0;
egg->lang.callname = trim (strdup (skipspaces (s)));
egg->lang.pushargs = !((!strcmp (s, "goto")) || (!strcmp (s, "break")));
}

static void rcc_reset_callname(REgg *egg) {
R_FREE (egg->lang.callname);
egg->lang.nargs = 0;
}

#define SYNTAX_ATT 0
#if SYNTAX_ATT
#define FRAME_FMT ".LC%d_%d_frame%d"
#define FRAME_END_FMT ".LC%d_%d_end_frame%d"
#else
#define FRAME_FMT "__%d_%d_frame%d"
#define FRAME_END_FMT "__%d_%d_end_frame%d"
#endif

#if 0
static char *get_frame_label(int type) {
static char label[128];
int nf = egg->lang.nfunctions;
int nb = egg->lang.nbrackets;
int ct = context;

#if 1
if (type == 1) {
nb--;
} else if (type == 2) {
ct--;
}
#endif

snprintf (label, sizeof (label), FRAME_FMT, nf, nb, ct);
return label;
}

static char *get_end_frame_label(REgg *egg) {
static char label[128];
snprintf (label, sizeof (label) - 1, FRAME_END_FMT,
egg->lang.nfunctions, egg->lang.nbrackets, context - 1);


return label;
}
#endif

static const char *find_alias(REgg *egg, const char *str) {

char *p = (char *) str;
int i;
if (*str == '"') {
return strdup (str);
}

while (*p && !is_space (*p)) {
p++;
}
*p = '\x00';
for (i = 0; i < egg->lang.nalias; i++) {
if (!strcmp (str, egg->lang.aliases[i].name)) {
return strdup (egg->lang.aliases[i].content);
}
}
return NULL;

}

static void rcc_internal_mathop(REgg *egg, const char *ptr, char *ep, char op) {
char *p, *q, *oldp; 
char type = ' ';
char buf[64]; 
oldp = p = q = strdup (ptr);
if (get_op (&q)) {
*q = '\x00';
}
REggEmit *e = egg->remit;
while (*p && is_space (*p)) {
p++;
}
if (is_var (p)) {
p = r_egg_mkvar (egg, buf, p, 0);
if (egg->lang.varxs == '*') {
e->load (egg, p, egg->lang.varsize);
R_FREE (oldp);
oldp = p = strdup (e->regs (egg, 0));


} else if (egg->lang.varxs == '&') {
e->load_ptr (egg, p);
R_FREE (oldp);
oldp = p = strdup (e->regs (egg, 0));
}
type = ' ';
} else {
type = '$';
}
if (*p) {
e->mathop (egg, op, egg->lang.varsize, type, p, ep);
}
if (p != oldp) {
R_FREE (p);
}
R_FREE (oldp);
R_FREE (ep);
}






static void rcc_mathop(REgg *egg, char **pos, int level) {
REggEmit *e = egg->remit;
int op_ret = level;
char op, *next_pos;

while (**pos && is_space (**pos)) {
(*pos)++;
}
next_pos = *pos + 1;

do {
op = (is_op (**pos) && !(is_var (*pos)))? **pos: '=';
*pos = (is_op (**pos) && !(is_var (*pos)))? *pos + 1: *pos;
op_ret = get_op (&next_pos);
if (op_ret > level) {
rcc_mathop (egg, pos, op_ret);
rcc_internal_mathop (egg, e->regs (egg, op_ret - 1),
strdup (e->regs (egg, level - 1)), op);
next_pos = *pos + 1;
} else {
rcc_internal_mathop (egg, *pos, strdup (e->regs (egg, level - 1)), op);
*pos = next_pos;
next_pos++;
}
} while (**pos && op_ret >= level);















}

static void rcc_pusharg(REgg *egg, char *str) {
REggEmit *e = egg->remit;
char buf[64], *p = r_egg_mkvar (egg, buf, str, 0);
if (!p) {
return;
}
R_FREE (egg->lang.ctxpush[CTX]);
egg->lang.ctxpush[CTX] = strdup (p); 
egg->lang.nargs++;
if (egg->lang.pushargs) {
e->push_arg (egg, egg->lang.varxs, egg->lang.nargs, p);
}

free (p);
}

static void rcc_element(REgg *egg, char *str) {
if (!egg || !str) {
return;
}
REggEmit *e = egg->remit;
char *ptr, *p = str + strlen (str);
int inside = 0;
int num, num2;
int i;

if (CTX) {
if (egg->lang.slurp == '"') {
if (egg->lang.mode == NORMAL) {
if (!egg->lang.dstvar) {
egg->lang.dstvar = strdup (".fix0");
}
rcc_pushstr (egg, str, 1);
}
} else {
if (egg->lang.callname) {
if (strstr (egg->lang.callname, "while") || strstr (egg->lang.callname, "if")) {
egg->lang.conditionstr = strdup (str);
}
}
egg->lang.nargs = 0;
if (egg->lang.mode == GOTO) {
egg->lang.mode = NORMAL; 
}
while (p-- != str) {
if (*p == '"') {
inside ^= 1;
} else if (*p == ',' && !inside) {
*p = '\0';
p = (char *) skipspaces (p + 1);
rcc_pusharg (egg, p);
}
}
rcc_pusharg (egg, str);
}
} else {
switch (egg->lang.mode) {
case ALIAS:
if (!egg->lang.dstvar) {
eprintf ("does not set name or content for alias\n");
break;
}
e->equ (egg, egg->lang.dstvar, str);
if (egg->lang.nalias > 255) {
eprintf ("global-buffer-overflow in aliases\n");
break;
}
for (i = 0; i < egg->lang.nalias; i++) {
if (!strcmp (egg->lang.dstvar, egg->lang.aliases[i].name)) {
R_FREE (egg->lang.aliases[i].name);
R_FREE (egg->lang.aliases[i].content);
break;
}
}
egg->lang.aliases[i].name = strdup (egg->lang.dstvar);
egg->lang.aliases[i].content = strdup (str);
egg->lang.nalias = (i == egg->lang.nalias)? (egg->lang.nalias + 1): egg->lang.nalias;

R_FREE (egg->lang.dstvar);
egg->lang.mode = NORMAL;
break;
case SYSCALL:
if (!egg->lang.dstvar) {
eprintf ("does not set name or arg for syscall\n");
break;
}
if (egg->lang.nsyscalls > 255) {
eprintf ("global-buffer-overflow in syscalls\n");
break;
}
{
bool found = false;
int idx = egg->lang.nsyscalls;
for (i = 0; i < egg->lang.nsyscalls; i++) {
if (!strcmp (egg->lang.dstvar, egg->lang.syscalls[i].name)) {
idx = i;
found = true;
break;
}
}

R_FREE (egg->lang.syscalls[idx].name);
R_FREE (egg->lang.syscalls[idx].arg);
egg->lang.syscalls[idx].name = strdup (egg->lang.dstvar);
egg->lang.syscalls[idx].arg = strdup (str);
if (!found) {
egg->lang.nsyscalls++;
}
R_FREE (egg->lang.dstvar);
}
break;
case GOTO:
egg->lang.elem[egg->lang.elem_n] = 0;
e->jmp (egg, egg->lang.elem, 0);
break;
case INCLUDE:
str = ptr = (char *) find_alias (egg, skipspaces (str));
if (ptr) {
if (strchr (ptr, '"')) {
ptr = strchr (ptr, '"') + 1;
if ((p = strchr (ptr, '"'))) {
*p = '\x00';
} else {
eprintf ("loss back quote in include directory\n");
}
egg->lang.includedir = strdup (ptr);
} else {
eprintf ("wrong include syntax\n");

egg->lang.includedir = NULL;
}
} else {
egg->lang.includedir = NULL;
}
R_FREE (str);
break;
default:
p = strchr (str, ',');
if (p) {
*p = '\0';
num2 = atoi (p + 1);
} else {
num2 = 0;
}
num = atoi (str) + num2;
egg->lang.stackframe = num;
egg->lang.stackfixed = num2;
if (egg->lang.mode != NAKED) {
e->frame (egg, egg->lang.stackframe + egg->lang.stackfixed);
}
}
egg->lang.elem[0] = 0;
egg->lang.elem_n = 0;
}
}

static void rcc_pushstr(REgg *egg, char *str, int filter) {
int dotrim;
int i, j, len, ch;
REggEmit *e = egg->remit;

e->comment (egg, "encode %s string (%s) (%s)",
filter? "filtered": "unfiltered", str, egg->lang.callname);


if (filter) {
for (i = 0; str[i]; i++) {
dotrim = 0;
if (str[i] == '\\') {
switch (str[i + 1]) {
case 't':
str[i] = '\t';
dotrim = 1;
break;
case 'n':
str[i] = '\n';
dotrim = 1;
break;
case 'e':
str[i] = '\x1b';
dotrim = 1;
break;
case 'x':
ch = r_hex_pair2bin (str + i + 2);
if (ch == -1) {
eprintf ("%s:%d Error string format\n",
egg->lang.file, egg->lang.line);
}
str[i] = (char) ch;
dotrim = 3;
break;
default:
break;
}
if (dotrim) {
memmove (str + i + 1, str + i + dotrim + 1,
strlen (str + i + dotrim + 1) + 1);
}

}
}
}

len = strlen (str);
j = (len - len % e->size) + e->size;
e->set_string (egg, egg->lang.dstvar, str, j);
R_FREE (egg->lang.dstvar);
}

R_API char *r_egg_mkvar(REgg *egg, char *out, const char *_str, int delta) {
int i, len, qi;
char *oldstr = NULL, *str = NULL, foo[32], *q, *ret = NULL;

delta += egg->lang.stackfixed; 
if (!_str) {
return NULL; 
}

ret = str = oldstr = strdup (skipspaces (_str));

if ((q = strchr (str, ':'))) {
*q = '\0';
qi = atoi (q + 1);
egg->lang.varsize = (qi == 1)? 'b': 'l';
} else {
egg->lang.varsize = 'l';
}
if (*str == '*' || *str == '&') {
egg->lang.varxs = *str;
str++;
} else {
egg->lang.varxs = 0;
}
if (str[0] == '.') {
REggEmit *e = egg->remit;
if (!strncmp (str + 1, "ret", 3)) {
strcpy (out, e->retvar);
} else if (!strncmp (str + 1, "fix", 3)) {
int idx = (int)r_num_math (NULL, str + 4) + delta + e->size;
e->get_var (egg, 0, out, idx - egg->lang.stackfixed);

} else if (!strncmp (str + 1, "var", 3)) {
int idx = (int)r_num_math (NULL, str + 4) + delta + e->size;
e->get_var (egg, 0, out, idx);

} else if (!strncmp (str + 1, "rarg", 4)) {
if (e->get_ar) {
int idx = (int)r_num_math (NULL, str + 5);
e->get_ar (egg, out, idx);
}
} else if (!strncmp (str + 1, "arg", 3)) {
if (str[4]) {
if (egg->lang.stackframe == 0) {
e->get_var (egg, 1, out, 4); 
} else {
int idx = (int)r_num_math (NULL, str + 4) + delta + e->size;
e->get_var (egg, 2, out, idx + 4);
}
} else {

if (egg->lang.callname) {
for (i = 0; i < egg->lang.nsyscalls; i++) {
if (!strcmp (egg->lang.syscalls[i].name, egg->lang.callname)) {
free (oldstr);
return strdup (egg->lang.syscalls[i].arg);
}
}
eprintf ("Unknown arg for syscall '%s'\n", egg->lang.callname);
} else {
eprintf ("NO CALLNAME '%s'\n", egg->lang.callname);
}
}
} else if (!strncmp (str + 1, "reg", 3)) {

if (egg->lang.attsyntax) {
snprintf (out, 32, "%%%s", e->regs (egg, atoi (str + 4)));
} else {
snprintf (out, 32, "%s", e->regs (egg, atoi (str + 4)));
}
} else {
out = str; 
eprintf ("Something is really wrong\n");
}
ret = strdup (out);
free (oldstr);
} else if (*str == '"' || *str == '\'') {
int mustfilter = *str == '"';

str++;
len = strlen (str) - 1;
if (!egg->lang.stackfixed || egg->lang.stackfixed < len) {
eprintf ("WARNING: No room in the static stackframe! (%d must be %d)\n",
egg->lang.stackfixed, len);
}
str[len] = '\0';
snprintf (foo, sizeof (foo) - 1, ".fix%d", egg->lang.nargs * 16); 
free (egg->lang.dstvar);
egg->lang.dstvar = strdup (skipspaces (foo));
rcc_pushstr (egg, str, mustfilter);
ret = r_egg_mkvar (egg, out, foo, 0);
free (oldstr);
}
return ret;
}

static void rcc_fun(REgg *egg, const char *str) {
char *ptr, *ptr2;
REggEmit *e = egg->remit;
str = skipspaces (str);
if (CTX) {
ptr = strchr (str, '=');
if (ptr) {
*ptr++ = '\0';
free (egg->lang.dstvar);
egg->lang.dstvar = strdup (skipspaces (str));
ptr2 = (char *) skipspaces (ptr);
if (*ptr2) {
rcc_set_callname (egg, skipspaces (ptr));
}
} else {
str = skipspaces (str);
rcc_set_callname (egg, skipspaces (str));
egg->remit->comment (egg, "rcc_fun %d (%s)",
CTX, egg->lang.callname);
}
} else {
ptr = strchr (str, '@');
if (ptr) {
*ptr++ = '\0';
egg->lang.mode = NORMAL;
if (strstr (ptr, "env")) {

free (egg->lang.setenviron);
egg->lang.setenviron = strdup (skipspaces (str));
egg->lang.slurp = 0;
} else if (strstr (ptr, "fastcall")) {

} else if (strstr (ptr, "syscall")) {
if (*str) {
egg->lang.mode = SYSCALL;
egg->lang.dstvar = strdup (skipspaces (str));
} else {
egg->lang.mode = INLINE;
free (egg->lang.syscallbody);
egg->lang.syscallbody = malloc (4096); 
egg->lang.dstval = egg->lang.syscallbody;
R_FREE (egg->lang.dstvar);
egg->lang.ndstval = 0;
*egg->lang.syscallbody = '\0';
}
} else if (strstr (ptr, "include")) {
egg->lang.mode = INCLUDE;
free (egg->lang.includefile);
egg->lang.includefile = strdup (skipspaces (str));


} else if (strstr (ptr, "alias")) {
egg->lang.mode = ALIAS;
ptr2 = egg->lang.dstvar = strdup (skipspaces (str));
while (*ptr2 && !is_space (*ptr2)) {
ptr2++;
}
*ptr2 = '\x00';

} else if (strstr (ptr, "data")) {
egg->lang.mode = DATA;
egg->lang.ndstval = 0;
egg->lang.dstvar = strdup (skipspaces (str));
egg->lang.dstval = malloc (4096);
} else if (strstr (ptr, "naked")) {
egg->lang.mode = NAKED;






r_egg_printf (egg, "%s:\n", str);
} else if (strstr (ptr, "inline")) {
egg->lang.mode = INLINE;
free (egg->lang.dstvar);
egg->lang.dstvar = strdup (skipspaces (str));
egg->lang.dstval = malloc (4096);
egg->lang.ndstval = 0;
} else {

if (*ptr) {
r_egg_printf (egg, "\n.%s %s\n", ptr, str);
}
r_egg_printf (egg, "%s:\n", str);
}
} else {

if (CTX > 0) {

eprintf ("LABEL %d\n", CTX);
r_egg_printf (egg, "\n%s:\n", str);
} else {
if (!strcmp (str, "goto")) {
egg->lang.mode = GOTO;
} else {

e->call (egg, str, 0);
}
}
}
}
}

#if 0
static void shownested() {
int i;
eprintf ("[[[NESTED %d]]] ", context);
for (i = 0; egg->lang.nested[i]; i++) {
eprintf ("%s ", egg->lang.nested[i]);
}
eprintf ("\n");
}
#endif

static void set_nested(REgg *egg, const char *s) {
int i = 0;
if (CTX < 1) {
return;
}
free (egg->lang.nested[CTX]);
egg->lang.nested[CTX] = strdup (s);



for (i = 1; i < 10; i++) {

R_FREE (egg->lang.nested[CTX + i]);
}
}

static void rcc_context(REgg *egg, int delta) {
REggEmit *emit = egg->remit;
char str[64];

if (CTX > 31 || CTX < 0) {
return;
}

if (delta > 0) {
egg->lang.nestedi[CTX]++;
R_FREE (egg->lang.nested_callname[CTX]);
if (egg->lang.callname) {
egg->lang.nested_callname[CTX] = strdup (egg->lang.callname);
}
}
if (egg->lang.callname && CTX > 0) { 



}
CTX += delta;
egg->lang.lastctxdelta = delta;

if (CTX == 0 && delta < 0) {
if (egg->lang.mode != NAKED) {
emit->frame_end (egg, egg->lang.stackframe + egg->lang.stackfixed, egg->lang.nbrackets);
}
if (egg->lang.mode == NORMAL) { 
egg->lang.stackframe = 0;
}
egg->lang.mode = NORMAL;
} else {


const char *elm = skipspaces (egg->lang.elem);



#if 0
if (delta < 0 && context > 0) {
eprintf ("close bracket foo!!!\n");
shownested ();
cn = strdup (egg->lang.nested[context - 1]);
eprintf ("STATEMENT cn=(%s) idx=%d (%s)\n", cn, context - 1, egg->lang.nested[context - 1]);
eprintf ("CNTXXXPUSH (%s)\n", egg->lang.ctxpush[context - 1]);
#if 0
if (!strcmp (cn, "while")) {
emit->while_end (egg, get_frame_label (context - 1));


return;
}
#endif
}
#endif



if (egg->lang.callname) {


char *b, *g, *e, *n;
emit->comment (egg, "cond frame %s (%s)", egg->lang.callname, elm);

if (egg->lang.conditionstr) {
b = strchr (egg->lang.conditionstr, '<'); 
g = strchr (egg->lang.conditionstr, '>'); 
e = strchr (egg->lang.conditionstr, '='); 
n = strchr (egg->lang.conditionstr, '!'); 
} else {
b = g = e = n = NULL;
}
if (!strcmp (egg->lang.callname, "while")) {
char lab[128];
sprintf (lab, "__begin_%d_%d_%d", egg->lang.nfunctions,
CTX - 1, egg->lang.nestedi[CTX - 1] - 1);


emit->get_while_end (egg, str, egg->lang.ctxpush[CTX - 1], lab); 




if (delta > 0) {
set_nested (egg, str);
}
rcc_set_callname (egg, "if");
}
if (!strcmp (egg->lang.callname, "if")) {






sprintf (str, "__end_%d_%d_%d", egg->lang.nfunctions, CTX - 1, egg->lang.nestedi[CTX - 1] - 1);
emit->branch (egg, b, g, e, n, egg->lang.varsize, str);
if (CTX > 0) {

}
rcc_reset_callname (egg);
R_FREE (egg->lang.conditionstr);
} 
} 
}
}

static int parsedatachar(REgg *egg, char c) {
char *str;
int i, j;

if (!egg->lang.dstval) {
return 0;
}

if (c == '{') { 
rcc_context (egg, 1);
if (++(egg->lang.inlinectr) == 1) {
return egg->lang.ndstval = 0;
}
} else if (egg->lang.inlinectr == 0) {

if (c == ')') {
egg->lang.stackframe = atoi (egg->lang.dstval);
egg->lang.ndstval = 0;
} else {
egg->lang.dstval[egg->lang.ndstval++] = c;
}
return 0;
}

if (c == '}') { 
if (CTX < 2) {
egg->lang.inlinectr = 0;
rcc_context (egg, -1);
egg->lang.slurp = 0;
egg->lang.mode = NORMAL;

if (egg->lang.dstval && egg->lang.dstvar) {
egg->lang.dstval[egg->lang.ndstval] = '\0';
egg->remit->comment (egg, "data (%s)(%s)size=(%d)\n",
egg->lang.dstvar, egg->lang.dstval, egg->lang.stackframe);
r_egg_printf (egg, ".data\n");
for (str = egg->lang.dstval; is_space (*str); str++) {
;
}
j = (egg->lang.stackframe)? egg->lang.stackframe: 1;

r_egg_printf (egg, "%s:\n", egg->lang.dstvar);
for (i = 1; i <= j; i++) {
if (*str == '"') {
r_egg_printf (egg, ".ascii %s%s\n", egg->lang.dstval, (i == j)? "\"\\x00\"": "");
} else {
r_egg_printf (egg, ".long %s\n", egg->lang.dstval);
}
}
r_egg_printf (egg, ".text\n");
R_FREE (egg->lang.dstvar);
R_FREE (egg->lang.dstval);
egg->lang.ndstval = 0;
CTX = 0;
return 1;
}
}
}
if (egg->lang.dstval) {
egg->lang.dstval[egg->lang.ndstval++] = c;
}
return 0;
}

static int parseinlinechar(REgg *egg, char c) {

if (c == '{') { 
rcc_context (egg, 1);
egg->lang.inlinectr++;
if (egg->lang.inlinectr == 1) {
return 0;
}
} else if (egg->lang.inlinectr == 0) {
return 0;
}


if (c == '}') { 
if (CTX < 2) {
rcc_context (egg, -1);
egg->lang.slurp = 0;
egg->lang.mode = NORMAL;
egg->lang.inlinectr = 0;
if (!egg->lang.dstvar && egg->lang.dstval == egg->lang.syscallbody) {
egg->lang.dstval = NULL;
return 1;
} else 
if (egg->lang.dstval && egg->lang.dstvar) {
egg->lang.dstval[egg->lang.ndstval] = '\0';

egg->lang.inlines[egg->lang.ninlines].name = strdup (skipspaces (egg->lang.dstvar));
egg->lang.inlines[egg->lang.ninlines].body = strdup (skipspaces (egg->lang.dstval));
egg->lang.ninlines++;
R_FREE (egg->lang.dstvar);
R_FREE (egg->lang.dstval);
return 1;
}
eprintf ("Parse error\n");
}
}
if (egg->lang.dstval) {
egg->lang.dstval[egg->lang.ndstval++] = c;
egg->lang.dstval[egg->lang.ndstval] = 0;
}
return 0;
}


static void rcc_next(REgg *egg) {
const char *ocn;
REggEmit *e = egg->remit;
char *str = NULL, *p, *ptr, buf[64];
int i;

if (egg->lang.setenviron) {
egg->lang.elem[egg->lang.elem_n - 1] = 0;
r_sys_setenv (egg->lang.setenviron, egg->lang.elem);
R_FREE (egg->lang.setenviron);
return;
}
if (egg->lang.includefile) {
char *p, *q, *path;
egg->lang.elem[egg->lang.elem_n - 1] = 0;
path = find_include (egg->lang.includedir, egg->lang.includefile);
if (!path) {
eprintf ("Cannot find include file '%s'\n", egg->lang.elem);
return;
}
R_FREE (egg->lang.includefile);
R_FREE (egg->lang.includedir);
rcc_reset_callname (egg);
p = q = r_file_slurp (path, NULL);
if (p) {
int oline = ++(egg->lang.line);
egg->lang.elem[0] = 0; 
egg->lang.elem_n = 0;
egg->lang.line = 0;
for (; *p; p++) {
r_egg_lang_parsechar (egg, *p);
}
free (q);
egg->lang.line = oline;
} else {
eprintf ("Cannot find '%s'\n", path);
}
free (path);
return;
}
egg->lang.docall = 1;
if (egg->lang.callname) {
if (!strcmp (egg->lang.callname, "goto")) {
if (egg->lang.nargs != 1) {
eprintf ("Invalid number of arguments for goto()\n");
return;
}
e->jmp (egg, egg->lang.ctxpush[CTX], 0);
rcc_reset_callname (egg);
return;
}
if (!strcmp (egg->lang.callname, "break")) {
e->trap (egg);
rcc_reset_callname (egg);
return;
}
ptr = strchr (egg->lang.callname, '=');
if (ptr) {
*ptr = '\0';

}
ocn = skipspaces (egg->lang.callname);
if (!ocn) {
return;
}
str = r_egg_mkvar (egg, buf, ocn, 0);
if (!str) {
eprintf ("Cannot mkvar\n");
return;
}
if (*ocn == '.') {
e->call (egg, str, 1);
}
if (!strcmp (str, "while")) {
char var[128];
if (egg->lang.lastctxdelta >= 0) {
exit (eprintf ("ERROR: Unsupported while syntax\n"));
}
sprintf (var, "__begin_%d_%d_%d\n", egg->lang.nfunctions, CTX, egg->lang.nestedi[CTX - 1]);
e->while_end (egg, var);
#if 0
eprintf ("------------------------------------------ lastctx: %d\n", egg->lang.lastctxdelta);



rcc_printf (" pop %%eax\n");
rcc_printf (" cmp $0, %%eax\n"); 



if (egg->lang.lastctxdelta < 0) {
rcc_printf (" jnz %s\n", get_frame_label (1));
} else {
rcc_printf (" jnz %s\n", get_frame_label (0));
}


#endif
egg->lang.nargs = 0;
} else {
for (i = 0; i < egg->lang.nsyscalls; i++) {
if (!strcmp (str, egg->lang.syscalls[i].name)) {
p = egg->lang.syscallbody;
e->comment (egg, "set syscall args");
e->syscall_args (egg, egg->lang.nargs);
egg->lang.docall = 0;
e->comment (egg, "syscall");
r_egg_lang_parsechar (egg, '\n'); 
if (p) {
for (; *p; p++) {
r_egg_lang_parsechar (egg, *p);
}
} else {
char *q, *s = e->syscall (egg, egg->lang.nargs);
if (s) {
for (q = s; *q; q++) {
r_egg_lang_parsechar (egg, *q);
}
free (s);
} else {
eprintf ("Cannot get @syscall payload\n");
}
}
egg->lang.docall = 0;
break;
}
}
if (egg->lang.docall) {
for (i = 0; i < egg->lang.ninlines; i++) {
if (!strcmp (str, egg->lang.inlines[i].name)) {
p = egg->lang.inlines[i].body;
egg->lang.docall = 0;
e->comment (egg, "inline");
r_egg_lang_parsechar (egg, '\n'); 
for (; *p; p++) {
r_egg_lang_parsechar (egg, *p);
}
egg->lang.docall = 0;
break;
}
}
}
if (egg->lang.docall) {
e->comment (egg, "call in egg->lang.mode %d", egg->lang.mode);
e->call (egg, str, 0);
}
}
if (egg->lang.nargs > 0) {
e->restore_stack (egg, egg->lang.nargs * e->size);
}
















if (egg->lang.dstvar) {

*buf = 0;
free (str);
str = r_egg_mkvar (egg, buf, egg->lang.dstvar, 0);
if (*buf == 0) {
eprintf ("Cannot resolve variable '%s'\n", egg->lang.dstvar);
} else {
e->get_result (egg, buf);
}

R_FREE (egg->lang.dstvar);
}
rcc_reset_callname (egg);
} else {
int vs = 'l';
char type, *eq, *ptr = egg->lang.elem, *tmp;
egg->lang.elem[egg->lang.elem_n] = '\0';
ptr = (char *) skipspaces (ptr);
if (*ptr) {
eq = strchr (ptr, '=');
if (eq) {
char *p = (char *) skipspaces (ptr);
vs = egg->lang.varsize;
*buf = *eq = '\x00';
e->mathop (egg, '=', vs, '$', "0", e->regs (egg, 1));

egg->lang.mathline = strdup ((char *) skipspaces (eq + 1));
tmp = egg->lang.mathline;
rcc_mathop (egg, &tmp, 2);
R_FREE (egg->lang.mathline);
tmp = NULL;

p = r_egg_mkvar (egg, buf, ptr, 0);
if (is_var (p)) {
char *q = r_egg_mkvar (egg, buf, p, 0);
if (q) {
free (p);
p = q;
}
if (egg->lang.varxs == '*' || egg->lang.varxs == '&') {
eprintf ("not support for *ptr in egg->lang.dstvar\n");
}

type = ' ';
} else {
type = '$';
}
e->mathop (egg, '=', vs, type, e->regs (egg, 1), p);
free (p);




























} else {
if (!strcmp (ptr, "break")) { 
e->trap (egg);
rcc_reset_callname (egg);
} else {
e->mathop (egg, '=', vs, '$', ptr, NULL);
}
}
}
}
free (str);
}

R_API int r_egg_lang_parsechar(REgg *egg, char c) {
REggEmit *e = egg->remit;
char *ptr, str[64], *tmp_ptr = NULL;
int i, j;
if (c == '\n') {
egg->lang.line++;
egg->lang.elem_n = 0;
}

if (egg->lang.skipline) {
if (c != '\n') {
egg->lang.oc = c;
return 0;
}
egg->lang.skipline = 0;
}
if (egg->lang.mode == DATA) {
return parsedatachar (egg, c);
}
if (egg->lang.mode == INLINE) {
return parseinlinechar (egg, c);
}

if (egg->lang.quoteline) {
if (c != egg->lang.quoteline) {
if (egg->lang.quotelinevar == 1) {
if (c == '`') {
egg->lang.elem[egg->lang.elem_n] = 0;
egg->lang.elem_n = 0;
tmp_ptr = r_egg_mkvar (egg, str, egg->lang.elem, 0);
r_egg_printf (egg, "%s", tmp_ptr);
free (tmp_ptr);
egg->lang.quotelinevar = 0;
} else {
egg->lang.elem[egg->lang.elem_n++] = c;
}
} else {
if (c == '`') {
egg->lang.elem_n = 0;
egg->lang.quotelinevar = 1;
} else {
r_egg_printf (egg, "%c", c);
}
}
egg->lang.oc = c;
return 0;
} else {
r_egg_printf (egg, "\n");
egg->lang.quoteline = 0;
}
}

if (egg->lang.commentmode) {
if (c == '/' && egg->lang.oc == '*') {
egg->lang.commentmode = 0;
}
egg->lang.oc = c;
return 0;
} else if (c == '*' && egg->lang.oc == '/') {
egg->lang.commentmode = 1;
}
if (egg->lang.slurp) {
if (egg->lang.slurp != '"' && c == egg->lang.slurpin) { 
exit (eprintf (
"%s:%d Nesting of expressions not yet supported\n",
egg->lang.file, egg->lang.line));
}
if (c == egg->lang.slurp && egg->lang.oc != '\\') { 
egg->lang.elem[egg->lang.elem_n] = '\0';
if (egg->lang.elem_n > 0) {
rcc_element (egg, egg->lang.elem);
} else {
e->frame (egg, 0);
}
egg->lang.elem_n = 0;
egg->lang.slurp = 0;
} else {
egg->lang.elem[egg->lang.elem_n++] = c;
}
egg->lang.elem[egg->lang.elem_n] = '\0';
} else {
switch (c) {
case ';':
rcc_next (egg);
break;
case '"':
egg->lang.slurp = '"';
break;
case '(':
egg->lang.slurpin = '(';
egg->lang.slurp = ')';
break;
case '{':
if (CTX > 0) {
if (CTX > 31 || CTX < 0) {
eprintf ("Sinking before overflow\n");
CTX = 0;
break;
}

if (egg->lang.nested_callname[CTX] && strstr (egg->lang.nested_callname[CTX], "if") &&
strstr (egg->lang.elem, "else")) {
*egg->lang.elem = '\x00';
egg->lang.elem_n = 0;
R_FREE (egg->lang.ifelse_table[CTX][egg->lang.nestedi[CTX] - 1])
egg->lang.ifelse_table[CTX][egg->lang.nestedi[CTX] - 1] =
r_str_newf (" __end_%d_%d_%d",
egg->lang.nfunctions, CTX, egg->lang.nestedi[CTX]);
}
r_egg_printf (egg, " __begin_%d_%d_%d:\n",
egg->lang.nfunctions, CTX, egg->lang.nestedi[CTX]); 
}
rcc_context (egg, 1);
break;
case '}':
egg->lang.endframe = egg->lang.nested[CTX];
if (egg->lang.endframe) {

r_egg_printf (egg, "%s", egg->lang.endframe);
R_FREE (egg->lang.nested[CTX]);

}
if (CTX > 1) {
if (egg->lang.nested_callname[CTX - 1] && strstr (egg->lang.nested_callname[CTX - 1], "if")) {
tmp_ptr = r_str_newf ("__ifelse_%d_%d", CTX - 1, egg->lang.nestedi[CTX - 1] - 1);
e->jmp (egg, tmp_ptr, 0);
R_FREE (tmp_ptr); 
egg->lang.ifelse_table[CTX - 1][egg->lang.nestedi[CTX - 1] - 1] =
r_str_newf ("__end_%d_%d_%d",
egg->lang.nfunctions, CTX - 1, egg->lang.nestedi[CTX - 1] - 1);
}




r_egg_printf (egg, " __end_%d_%d_%d:\n",
egg->lang.nfunctions, CTX - 1, egg->lang.nestedi[CTX - 1] - 1);


}
if (CTX > 0) {
egg->lang.nbrackets++;
}
rcc_context (egg, -1);
if (CTX == 0) {
r_egg_printf (egg, "\n");



for (i = 0; i < 32; i++) {
for (j = 0; j < egg->lang.nestedi[i] && j < 32; j++) {
if (egg->lang.ifelse_table[i][j]) {
r_egg_printf (egg, " __ifelse_%d_%d:\n", i, j);
e->jmp (egg, egg->lang.ifelse_table[i][j], 0);
R_FREE (egg->lang.ifelse_table[i][j]);
}
}
}


egg->lang.nbrackets = 0;
egg->lang.nfunctions++;
}
break;
case ':':
if (egg->lang.oc == '\n' || egg->lang.oc == '}') {
egg->lang.quoteline = '\n';
} else {
egg->lang.elem[egg->lang.elem_n++] = c;
}
break;
case '#':
if (egg->lang.oc == '\n') {
egg->lang.skipline = 1;
}
break;
case '/':
if (egg->lang.oc == '/') {
egg->lang.skipline = 1;
} else {
egg->lang.elem[egg->lang.elem_n++] = c;
}
break;
default:
egg->lang.elem[egg->lang.elem_n++] = c;
}
if (egg->lang.slurp) {
if (egg->lang.elem_n) {
ptr = egg->lang.elem;
egg->lang.elem[egg->lang.elem_n] = '\0';
while (is_space (*ptr)) {
ptr++;
}
rcc_fun (egg, ptr);
}
egg->lang.elem_n = 0;
}
}
if (c != '\t' && c != ' ') {
egg->lang.oc = c;
}
return 0;
}
