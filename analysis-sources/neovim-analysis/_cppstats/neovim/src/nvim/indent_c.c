#include <assert.h>
#include <inttypes.h>
#include "nvim/vim.h"
#include "nvim/ascii.h"
#include "nvim/misc1.h"
#include "nvim/charset.h"
#include "nvim/cursor.h"
#include "nvim/edit.h"
#include "nvim/indent.h"
#include "nvim/indent_c.h"
#include "nvim/mark.h"
#include "nvim/memline.h"
#include "nvim/memory.h"
#include "nvim/option.h"
#include "nvim/search.h"
#include "nvim/strings.h"
typedef struct {
int found;
lpos_T lpos;
} cpp_baseclass_cache_T;
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "indent_c.c.generated.h"
#endif
static pos_T *ind_find_start_comment(void)
{ 
return find_start_comment(curbuf->b_ind_maxcomment);
}
pos_T *
find_start_comment ( 
int ind_maxcomment
)
{
pos_T *pos;
char_u *line;
char_u *p;
int64_t cur_maxcomment = ind_maxcomment;
for (;; ) {
pos = findmatchlimit(NULL, '*', FM_BACKWARD, cur_maxcomment);
if (pos == NULL)
break;
line = ml_get(pos->lnum);
for (p = line; *p && (colnr_T)(p - line) < pos->col; ++p)
p = skip_string(p);
if ((colnr_T)(p - line) <= pos->col)
break;
cur_maxcomment = curwin->w_cursor.lnum - pos->lnum - 1;
if (cur_maxcomment <= 0) {
pos = NULL;
break;
}
}
return pos;
}
static pos_T *ind_find_start_CORS(linenr_T *is_raw)
{
static pos_T comment_pos_copy;
pos_T *comment_pos = find_start_comment(curbuf->b_ind_maxcomment);
if (comment_pos != NULL) {
comment_pos_copy = *comment_pos;
comment_pos = &comment_pos_copy;
}
pos_T *rs_pos = find_start_rawstring(curbuf->b_ind_maxcomment);
if (comment_pos == NULL || (rs_pos != NULL && lt(*rs_pos, *comment_pos))) {
if (is_raw != NULL && rs_pos != NULL) {
*is_raw = rs_pos->lnum;
}
return rs_pos;
}
return comment_pos;
}
static pos_T *find_start_rawstring(int ind_maxcomment)
{ 
pos_T *pos;
char_u *line;
char_u *p;
long cur_maxcomment = ind_maxcomment;
for (;;)
{
pos = findmatchlimit(NULL, 'R', FM_BACKWARD, cur_maxcomment);
if (pos == NULL)
break;
line = ml_get(pos->lnum);
for (p = line; *p && (colnr_T)(p - line) < pos->col; ++p)
p = skip_string(p);
if ((colnr_T)(p - line) <= pos->col)
break;
cur_maxcomment = curwin->w_cursor.lnum - pos->lnum - 1;
if (cur_maxcomment <= 0)
{
pos = NULL;
break;
}
}
return pos;
}
static char_u *skip_string(char_u *p)
{
int i;
for (;; ++p) {
if (p[0] == '\'') { 
if (!p[1]) 
break;
i = 2;
if (p[1] == '\\') { 
++i;
while (ascii_isdigit(p[i - 1])) 
++i;
}
if (p[i] == '\'') { 
p += i;
continue;
}
} else if (p[0] == '"') { 
for (++p; p[0]; ++p) {
if (p[0] == '\\' && p[1] != NUL)
++p;
else if (p[0] == '"') 
break;
}
if (p[0] == '"')
continue; 
} else if (p[0] == 'R' && p[1] == '"') {
char_u *delim = p + 2;
char_u *paren = vim_strchr(delim, '(');
if (paren != NULL) {
const ptrdiff_t delim_len = paren - delim;
for (p += 3; *p; ++p)
if (p[0] == ')' && STRNCMP(p + 1, delim, delim_len) == 0
&& p[delim_len + 1] == '"')
{
p += delim_len + 1;
break;
}
if (p[0] == '"')
continue; 
}
}
break; 
}
if (!*p)
--p; 
return p;
}
bool cin_is_cinword(char_u *line)
{
bool retval = false;
size_t cinw_len = STRLEN(curbuf->b_p_cinw) + 1;
char_u *cinw_buf = xmalloc(cinw_len);
line = skipwhite(line);
for (char_u *cinw = curbuf->b_p_cinw; *cinw; ) {
size_t len = copy_option_part(&cinw, cinw_buf, cinw_len, ",");
if (STRNCMP(line, cinw_buf, len) == 0
&& (!vim_iswordc(line[len]) || !vim_iswordc(line[len - 1]))) {
retval = true;
break;
}
}
xfree(cinw_buf);
return retval;
}
static char_u *cin_skipcomment(char_u *s)
{
while (*s) {
char_u *prev_s = s;
s = skipwhite(s);
if (curbuf->b_ind_hash_comment != 0 && s != prev_s && *s == '#') {
s += STRLEN(s);
break;
}
if (*s != '/')
break;
++s;
if (*s == '/') { 
s += STRLEN(s);
break;
}
if (*s != '*')
break;
for (++s; *s; ++s) 
if (s[0] == '*' && s[1] == '/') {
s += 2;
break;
}
}
return s;
}
static int cin_nocode(char_u *s)
{
return *cin_skipcomment(s) == NUL;
}
static pos_T *find_line_comment(void) 
{
static pos_T pos;
char_u *line;
char_u *p;
pos = curwin->w_cursor;
while (--pos.lnum > 0) {
line = ml_get(pos.lnum);
p = skipwhite(line);
if (cin_islinecomment(p)) {
pos.col = (int)(p - line);
return &pos;
}
if (*p != NUL)
break;
}
return NULL;
}
static bool cin_has_js_key(char_u *text)
{
char_u *s = skipwhite(text);
char_u quote = 0;
if (*s == '\'' || *s == '"') {
quote = *s;
++s;
}
if (!vim_isIDc(*s)) { 
return FALSE;
}
while (vim_isIDc(*s)) {
++s;
}
if (*s && *s == quote) {
++s;
}
s = cin_skipcomment(s);
return (*s == ':' && s[1] != ':');
}
static int cin_islabel_skip(char_u **s)
{
if (!vim_isIDc(**s)) 
return FALSE;
while (vim_isIDc(**s))
(*s)++;
*s = cin_skipcomment(*s);
return **s == ':' && *++*s != ':';
}
int cin_islabel(void)
{ 
char_u *s = cin_skipcomment(get_cursor_line_ptr());
if (cin_isdefault(s))
return FALSE;
if (cin_isscopedecl(s))
return FALSE;
if (!cin_islabel_skip(&s)) {
return FALSE;
}
pos_T cursor_save;
pos_T *trypos;
char_u *line;
cursor_save = curwin->w_cursor;
while (curwin->w_cursor.lnum > 1) {
--curwin->w_cursor.lnum;
curwin->w_cursor.col = 0;
if ((trypos = ind_find_start_CORS(NULL)) != NULL) { 
curwin->w_cursor = *trypos;
}
line = get_cursor_line_ptr();
if (cin_ispreproc(line)) 
continue;
if (*(line = cin_skipcomment(line)) == NUL)
continue;
curwin->w_cursor = cursor_save;
if (cin_isterminated(line, TRUE, FALSE)
|| cin_isscopedecl(line)
|| cin_iscase(line, TRUE)
|| (cin_islabel_skip(&line) && cin_nocode(line)))
return TRUE;
return FALSE;
}
curwin->w_cursor = cursor_save;
return TRUE; 
}
static int cin_isinit(void)
{
char_u *s;
static char *skip[] = {"static", "public", "protected", "private"};
s = cin_skipcomment(get_cursor_line_ptr());
if (cin_starts_with(s, "typedef"))
s = cin_skipcomment(s + 7);
for (;; ) {
int i, l;
for (i = 0; i < (int)ARRAY_SIZE(skip); ++i) {
l = (int)strlen(skip[i]);
if (cin_starts_with(s, skip[i])) {
s = cin_skipcomment(s + l);
l = 0;
break;
}
}
if (l != 0)
break;
}
if (cin_starts_with(s, "enum"))
return TRUE;
if (cin_ends_in(s, (char_u *)"=", (char_u *)"{"))
return TRUE;
return FALSE;
}
int 
cin_iscase (
char_u *s,
int strict 
)
{
s = cin_skipcomment(s);
if (cin_starts_with(s, "case")) {
for (s += 4; *s; ++s) {
s = cin_skipcomment(s);
if (*s == NUL) {
break;
}
if (*s == ':') {
if (s[1] == ':') 
++s;
else
return TRUE;
}
if (*s == '\'' && s[1] && s[2] == '\'')
s += 2; 
else if (*s == '/' && (s[1] == '*' || s[1] == '/'))
return FALSE; 
else if (*s == '"') {
if (strict)
return FALSE; 
else
return TRUE;
}
}
return FALSE;
}
if (cin_isdefault(s))
return TRUE;
return FALSE;
}
static int cin_isdefault(char_u *s)
{
return STRNCMP(s, "default", 7) == 0
&& *(s = cin_skipcomment(s + 7)) == ':'
&& s[1] != ':';
}
int cin_isscopedecl(char_u *s)
{
int i;
s = cin_skipcomment(s);
if (STRNCMP(s, "public", 6) == 0)
i = 6;
else if (STRNCMP(s, "protected", 9) == 0)
i = 9;
else if (STRNCMP(s, "private", 7) == 0)
i = 7;
else
return FALSE;
return *(s = cin_skipcomment(s + i)) == ':' && s[1] != ':';
}
#define FIND_NAMESPACE_LIM 20
static bool cin_is_cpp_namespace(char_u *s)
{
char_u *p;
bool has_name = false;
bool has_name_start = false;
s = cin_skipcomment(s);
if (STRNCMP(s, "namespace", 9) == 0 && (s[9] == NUL || !vim_iswordc(s[9]))) {
p = cin_skipcomment(skipwhite(s + 9));
while (*p != NUL) {
if (ascii_iswhite(*p)) {
has_name = true; 
p = cin_skipcomment(skipwhite(p));
} else if (*p == '{') {
break;
} else if (vim_iswordc(*p)) {
has_name_start = true;
if (has_name) {
return false; 
}
p++;
} else if (p[0] == ':' && p[1] == ':' && vim_iswordc(p[2])) {
if (!has_name_start || has_name) {
return false;
}
p += 3;
} else {
return false;
}
}
return true;
}
return false;
}
static char_u *after_label(char_u *l)
{
for (; *l; ++l) {
if (*l == ':') {
if (l[1] == ':') 
++l;
else if (!cin_iscase(l + 1, FALSE))
break;
} else if (*l == '\'' && l[1] && l[2] == '\'')
l += 2; 
}
if (*l == NUL)
return NULL;
l = cin_skipcomment(l + 1);
if (*l == NUL)
return NULL;
return l;
}
static int 
get_indent_nolabel ( 
linenr_T lnum
)
{
char_u *l;
pos_T fp;
colnr_T col;
char_u *p;
l = ml_get(lnum);
p = after_label(l);
if (p == NULL)
return 0;
fp.col = (colnr_T)(p - l);
fp.lnum = lnum;
getvcol(curwin, &fp, &col, NULL, NULL);
return (int)col;
}
static int skip_label(linenr_T lnum, char_u **pp)
{
char_u *l;
int amount;
pos_T cursor_save;
cursor_save = curwin->w_cursor;
curwin->w_cursor.lnum = lnum;
l = get_cursor_line_ptr();
if (cin_iscase(l, FALSE) || cin_isscopedecl(l) || cin_islabel()) {
amount = get_indent_nolabel(lnum);
l = after_label(get_cursor_line_ptr());
if (l == NULL) 
l = get_cursor_line_ptr();
} else {
amount = get_indent();
l = get_cursor_line_ptr();
}
*pp = l;
curwin->w_cursor = cursor_save;
return amount;
}
static int cin_first_id_amount(void)
{
char_u *line, *p, *s;
int len;
pos_T fp;
colnr_T col;
line = get_cursor_line_ptr();
p = skipwhite(line);
len = (int)(skiptowhite(p) - p);
if (len == 6 && STRNCMP(p, "static", 6) == 0) {
p = skipwhite(p + 6);
len = (int)(skiptowhite(p) - p);
}
if (len == 6 && STRNCMP(p, "struct", 6) == 0)
p = skipwhite(p + 6);
else if (len == 4 && STRNCMP(p, "enum", 4) == 0)
p = skipwhite(p + 4);
else if ((len == 8 && STRNCMP(p, "unsigned", 8) == 0)
|| (len == 6 && STRNCMP(p, "signed", 6) == 0)) {
s = skipwhite(p + len);
if ((STRNCMP(s, "int", 3) == 0 && ascii_iswhite(s[3]))
|| (STRNCMP(s, "long", 4) == 0 && ascii_iswhite(s[4]))
|| (STRNCMP(s, "short", 5) == 0 && ascii_iswhite(s[5]))
|| (STRNCMP(s, "char", 4) == 0 && ascii_iswhite(s[4])))
p = s;
}
for (len = 0; vim_isIDc(p[len]); ++len)
;
if (len == 0 || !ascii_iswhite(p[len]) || cin_nocode(p))
return 0;
p = skipwhite(p + len);
fp.lnum = curwin->w_cursor.lnum;
fp.col = (colnr_T)(p - line);
getvcol(curwin, &fp, &col, NULL, NULL);
return (int)col;
}
static int cin_get_equal_amount(linenr_T lnum)
{
char_u *line;
char_u *s;
colnr_T col;
pos_T fp;
if (lnum > 1) {
line = ml_get(lnum - 1);
if (*line != NUL && line[STRLEN(line) - 1] == '\\')
return -1;
}
line = s = ml_get(lnum);
while (*s != NUL && vim_strchr((char_u *)"=;{}\"'", *s) == NULL) {
if (cin_iscomment(s)) 
s = cin_skipcomment(s);
else
++s;
}
if (*s != '=')
return 0;
s = skipwhite(s + 1);
if (cin_nocode(s))
return 0;
if (*s == '"') 
++s;
fp.lnum = lnum;
fp.col = (colnr_T)(s - line);
getvcol(curwin, &fp, &col, NULL, NULL);
return (int)col;
}
static int cin_ispreproc(char_u *s)
{
if (*skipwhite(s) == '#')
return TRUE;
return FALSE;
}
static int cin_ispreproc_cont(char_u **pp, linenr_T *lnump, int *amount)
{
char_u *line = *pp;
linenr_T lnum = *lnump;
int retval = false;
int candidate_amount = *amount;
if (*line != NUL && line[STRLEN(line) - 1] == '\\') {
candidate_amount = get_indent_lnum(lnum);
}
for (;; ) {
if (cin_ispreproc(line)) {
retval = TRUE;
*lnump = lnum;
break;
}
if (lnum == 1)
break;
line = ml_get(--lnum);
if (*line == NUL || line[STRLEN(line) - 1] != '\\')
break;
}
if (lnum != *lnump) {
*pp = ml_get(*lnump);
}
if (retval) {
*amount = candidate_amount;
}
return retval;
}
static int cin_iscomment(char_u *p)
{
return p[0] == '/' && (p[1] == '*' || p[1] == '/');
}
static int cin_islinecomment(char_u *p)
{
return p[0] == '/' && p[1] == '/';
}
static char_u
cin_isterminated (
char_u *s,
int incl_open, 
int incl_comma 
)
{
char_u found_start = 0;
unsigned n_open = 0;
int is_else = FALSE;
s = cin_skipcomment(s);
if (*s == '{' || (*s == '}' && !cin_iselse(s)))
found_start = *s;
if (!found_start)
is_else = cin_iselse(s);
while (*s) {
s = skip_string(cin_skipcomment(s));
if (*s == '}' && n_open > 0)
--n_open;
if ((!is_else || n_open == 0)
&& (*s == ';' || *s == '}' || (incl_comma && *s == ','))
&& cin_nocode(s + 1))
return *s;
else if (*s == '{') {
if (incl_open && cin_nocode(s + 1))
return *s;
else
++n_open;
}
if (*s)
s++;
}
return found_start;
}
static int cin_isfuncdecl(char_u **sp, linenr_T first_lnum, linenr_T min_lnum)
{
char_u *s;
linenr_T lnum = first_lnum;
linenr_T save_lnum = curwin->w_cursor.lnum;
int retval = false;
pos_T *trypos;
int just_started = TRUE;
if (sp == NULL)
s = ml_get(lnum);
else
s = *sp;
curwin->w_cursor.lnum = lnum;
if (find_last_paren(s, '(', ')')
&& (trypos = find_match_paren(curbuf->b_ind_maxparen)) != NULL) {
lnum = trypos->lnum;
if (lnum < min_lnum) {
curwin->w_cursor.lnum = save_lnum;
return false;
}
s = ml_get(lnum);
}
curwin->w_cursor.lnum = save_lnum;
if (cin_ispreproc(s)) {
return false;
}
while (*s && *s != '(' && *s != ';' && *s != '\'' && *s != '"') {
if (cin_iscomment(s)) {
s = cin_skipcomment(s);
} else if (*s == ':') {
if (*(s + 1) == ':') {
s += 2;
} else {
return false;
}
} else {
s++;
}
}
if (*s != '(') {
return false; 
}
while (*s && *s != ';' && *s != '\'' && *s != '"') {
if (*s == ')' && cin_nocode(s + 1)) {
lnum = first_lnum - 1;
s = ml_get(lnum);
if (*s == NUL || s[STRLEN(s) - 1] != '\\')
retval = TRUE;
goto done;
}
if ((*s == ',' && cin_nocode(s + 1)) || s[1] == NUL || cin_nocode(s)) {
int comma = (*s == ',');
for (;; ) {
if (lnum >= curbuf->b_ml.ml_line_count)
break;
s = ml_get(++lnum);
if (!cin_ispreproc(s))
break;
}
if (lnum >= curbuf->b_ml.ml_line_count)
break;
s = skipwhite(s);
if (!just_started && (!comma && *s != ',' && *s != ')'))
break;
just_started = FALSE;
} else if (cin_iscomment(s)) 
s = cin_skipcomment(s);
else {
++s;
just_started = FALSE;
}
}
done:
if (lnum != first_lnum && sp != NULL)
*sp = ml_get(first_lnum);
return retval;
}
static int cin_isif(char_u *p)
{
return STRNCMP(p, "if", 2) == 0 && !vim_isIDc(p[2]);
}
static int cin_iselse(char_u *p)
{
if (*p == '}') 
p = cin_skipcomment(p + 1);
return STRNCMP(p, "else", 4) == 0 && !vim_isIDc(p[4]);
}
static int cin_isdo(char_u *p)
{
return STRNCMP(p, "do", 2) == 0 && !vim_isIDc(p[2]);
}
static int 
cin_iswhileofdo ( 
char_u *p,
linenr_T lnum
)
{
pos_T cursor_save;
pos_T *trypos;
int retval = FALSE;
p = cin_skipcomment(p);
if (*p == '}') 
p = cin_skipcomment(p + 1);
if (cin_starts_with(p, "while")) {
cursor_save = curwin->w_cursor;
curwin->w_cursor.lnum = lnum;
curwin->w_cursor.col = 0;
p = get_cursor_line_ptr();
while (*p && *p != 'w') { 
++p;
++curwin->w_cursor.col;
}
if ((trypos = findmatchlimit(NULL, 0, 0,
curbuf->b_ind_maxparen)) != NULL
&& *cin_skipcomment(ml_get_pos(trypos) + 1) == ';')
retval = TRUE;
curwin->w_cursor = cursor_save;
}
return retval;
}
static int cin_is_if_for_while_before_offset(char_u *line, int *poffset)
{
int offset = *poffset;
if (offset-- < 2)
return 0;
while (offset > 2 && ascii_iswhite(line[offset]))
--offset;
offset -= 1;
if (!STRNCMP(line + offset, "if", 2))
goto probablyFound;
if (offset >= 1) {
offset -= 1;
if (!STRNCMP(line + offset, "for", 3))
goto probablyFound;
if (offset >= 2) {
offset -= 2;
if (!STRNCMP(line + offset, "while", 5))
goto probablyFound;
}
}
return 0;
probablyFound:
if (!offset || !vim_isIDc(line[offset - 1])) {
*poffset = offset;
return 1;
}
return 0;
}
static int cin_iswhileofdo_end(int terminated)
{
char_u *line;
char_u *p;
char_u *s;
pos_T *trypos;
int i;
if (terminated != ';') 
return FALSE;
p = line = get_cursor_line_ptr();
while (*p != NUL) {
p = cin_skipcomment(p);
if (*p == ')') {
s = skipwhite(p + 1);
if (*s == ';' && cin_nocode(s + 1)) {
i = (int)(p - line);
curwin->w_cursor.col = i;
trypos = find_match_paren(curbuf->b_ind_maxparen);
if (trypos != NULL) {
s = cin_skipcomment(ml_get(trypos->lnum));
if (*s == '}') 
s = cin_skipcomment(s + 1);
if (cin_starts_with(s, "while")) {
curwin->w_cursor.lnum = trypos->lnum;
return TRUE;
}
}
line = get_cursor_line_ptr();
p = line + i;
}
}
if (*p != NUL)
++p;
}
return FALSE;
}
static int cin_isbreak(char_u *p)
{
return STRNCMP(p, "break", 5) == 0 && !vim_isIDc(p[5]);
}
static int cin_is_cpp_baseclass(cpp_baseclass_cache_T *cached) {
lpos_T *pos = &cached->lpos; 
char_u *s;
int class_or_struct, lookfor_ctor_init, cpp_base_class;
linenr_T lnum = curwin->w_cursor.lnum;
char_u *line = get_cursor_line_ptr();
if (pos->lnum <= lnum) {
return cached->found; 
}
pos->col = 0;
s = skipwhite(line);
if (*s == '#') 
return FALSE;
s = cin_skipcomment(s);
if (*s == NUL)
return FALSE;
cpp_base_class = lookfor_ctor_init = class_or_struct = FALSE;
while (lnum > 1) {
line = ml_get(lnum - 1);
s = skipwhite(line);
if (*s == '#' || *s == NUL)
break;
while (*s != NUL) {
s = cin_skipcomment(s);
if (*s == '{' || *s == '}'
|| (*s == ';' && cin_nocode(s + 1)))
break;
if (*s != NUL)
++s;
}
if (*s != NUL)
break;
--lnum;
}
pos->lnum = lnum;
line = ml_get(lnum);
s = line;
for (;; ) {
if (*s == NUL) {
if (lnum == curwin->w_cursor.lnum) {
break;
}
line = ml_get(++lnum);
s = line;
}
if (s == line) {
if (cin_iscase(s, false)) {
break;
}
s = cin_skipcomment(line);
if (*s == NUL)
continue;
}
if (s[0] == '"' || (s[0] == 'R' && s[1] == '"'))
s = skip_string(s) + 1;
else if (s[0] == ':') {
if (s[1] == ':') {
lookfor_ctor_init = FALSE;
s = cin_skipcomment(s + 2);
} else if (lookfor_ctor_init || class_or_struct) {
cpp_base_class = true;
lookfor_ctor_init = class_or_struct = false;
pos->col = 0;
s = cin_skipcomment(s + 1);
} else
s = cin_skipcomment(s + 1);
} else if ((STRNCMP(s, "class", 5) == 0 && !vim_isIDc(s[5]))
|| (STRNCMP(s, "struct", 6) == 0 && !vim_isIDc(s[6]))) {
class_or_struct = TRUE;
lookfor_ctor_init = FALSE;
if (*s == 'c')
s = cin_skipcomment(s + 5);
else
s = cin_skipcomment(s + 6);
} else {
if (s[0] == '{' || s[0] == '}' || s[0] == ';') {
cpp_base_class = lookfor_ctor_init = class_or_struct = FALSE;
} else if (s[0] == ')') {
class_or_struct = FALSE;
lookfor_ctor_init = TRUE;
} else if (s[0] == '?') {
return FALSE;
} else if (!vim_isIDc(s[0])) {
class_or_struct = false;
lookfor_ctor_init = false;
} else if (pos->col == 0) {
lookfor_ctor_init = FALSE;
if (cpp_base_class) {
pos->col = (colnr_T)(s - line);
}
}
if (lnum == curwin->w_cursor.lnum && *s == ',' && cin_nocode(s + 1)) {
pos->col = 0;
}
s = cin_skipcomment(s + 1);
}
}
cached->found = cpp_base_class;
if (cpp_base_class) {
pos->lnum = lnum;
}
return cpp_base_class;
}
static int get_baseclass_amount(int col)
{
int amount;
colnr_T vcol;
pos_T *trypos;
if (col == 0) {
amount = get_indent();
if (find_last_paren(get_cursor_line_ptr(), '(', ')')
&& (trypos = find_match_paren(curbuf->b_ind_maxparen)) != NULL)
amount = get_indent_lnum(trypos->lnum); 
if (!cin_ends_in(get_cursor_line_ptr(), (char_u *)",", NULL))
amount += curbuf->b_ind_cpp_baseclass;
} else {
curwin->w_cursor.col = col;
getvcol(curwin, &curwin->w_cursor, &vcol, NULL, NULL);
amount = (int)vcol;
}
if (amount < curbuf->b_ind_cpp_baseclass)
amount = curbuf->b_ind_cpp_baseclass;
return amount;
}
static int cin_ends_in(char_u *s, char_u *find, char_u *ignore)
{
char_u *p = s;
char_u *r;
int len = (int)STRLEN(find);
while (*p != NUL) {
p = cin_skipcomment(p);
if (STRNCMP(p, find, len) == 0) {
r = skipwhite(p + len);
if (ignore != NULL && STRNCMP(r, ignore, STRLEN(ignore)) == 0)
r = skipwhite(r + STRLEN(ignore));
if (cin_nocode(r))
return TRUE;
}
if (*p != NUL)
++p;
}
return FALSE;
}
static int cin_starts_with(char_u *s, char *word)
{
int l = (int)STRLEN(word);
return STRNCMP(s, word, l) == 0 && !vim_isIDc(s[l]);
}
static int cin_is_cpp_extern_c(char_u *s)
{
char_u *p;
int has_string_literal = false;
s = cin_skipcomment(s);
if (STRNCMP(s, "extern", 6) == 0 && (s[6] == NUL || !vim_iswordc(s[6]))) {
p = cin_skipcomment(skipwhite(s + 6));
while (*p != NUL) {
if (ascii_iswhite(*p)) {
p = cin_skipcomment(skipwhite(p));
} else if (*p == '{') {
break;
} else if (p[0] == '"' && p[1] == 'C' && p[2] == '"') {
if (has_string_literal) {
return false;
}
has_string_literal = true;
p += 3;
} else if (p[0] == '"' && p[1] == 'C' && p[2] == '+' && p[3] == '+'
&& p[4] == '"') {
if (has_string_literal) {
return false;
}
has_string_literal = true;
p += 5;
} else {
return false;
}
}
return has_string_literal ? true : false;
}
return false;
}
static int cin_skip2pos(pos_T *trypos)
{
char_u *line;
char_u *p;
char_u *new_p;
p = line = ml_get(trypos->lnum);
while (*p && (colnr_T)(p - line) < trypos->col) {
if (cin_iscomment(p)) {
p = cin_skipcomment(p);
} else {
new_p = skip_string(p);
if (new_p == p) {
p++;
} else {
p = new_p;
}
}
}
return (int)(p - line);
}
static pos_T *find_start_brace(void)
{ 
pos_T cursor_save;
pos_T *trypos;
pos_T *pos;
static pos_T pos_copy;
cursor_save = curwin->w_cursor;
while ((trypos = findmatchlimit(NULL, '{', FM_BLOCKSTOP, 0)) != NULL) {
pos_copy = *trypos; 
trypos = &pos_copy;
curwin->w_cursor = *trypos;
pos = NULL;
if ((colnr_T)cin_skip2pos(trypos) == trypos->col
&& (pos = ind_find_start_CORS(NULL)) == NULL) { 
break;
}
if (pos != NULL) {
curwin->w_cursor.lnum = pos->lnum;
}
}
curwin->w_cursor = cursor_save;
return trypos;
}
static pos_T *find_match_paren(int ind_maxparen)
{
return find_match_char('(', ind_maxparen);
}
static pos_T * find_match_char(char_u c, int ind_maxparen)
{
pos_T cursor_save;
pos_T *trypos;
static pos_T pos_copy;
int ind_maxp_wk;
cursor_save = curwin->w_cursor;
ind_maxp_wk = ind_maxparen;
retry:
if ((trypos = findmatchlimit(NULL, c, 0, ind_maxp_wk)) != NULL) {
if ((colnr_T)cin_skip2pos(trypos) > trypos->col) {
ind_maxp_wk = ind_maxparen - (int)(cursor_save.lnum - trypos->lnum);
if (ind_maxp_wk > 0) {
curwin->w_cursor = *trypos;
curwin->w_cursor.col = 0; 
goto retry;
}
trypos = NULL;
} else {
pos_T *trypos_wk;
pos_copy = *trypos; 
trypos = &pos_copy;
curwin->w_cursor = *trypos;
if ((trypos_wk = ind_find_start_CORS(NULL)) != NULL) { 
ind_maxp_wk = ind_maxparen - (int)(cursor_save.lnum
- trypos_wk->lnum);
if (ind_maxp_wk > 0) {
curwin->w_cursor = *trypos_wk;
goto retry;
}
trypos = NULL;
}
}
}
curwin->w_cursor = cursor_save;
return trypos;
}
static pos_T *find_match_paren_after_brace(int ind_maxparen)
{
pos_T *trypos = find_match_paren(ind_maxparen);
if (trypos == NULL) {
return NULL;
}
pos_T *tryposBrace = find_start_brace();
if (tryposBrace != NULL
&& (trypos->lnum != tryposBrace->lnum
? trypos->lnum < tryposBrace->lnum
: trypos->col < tryposBrace->col)) {
trypos = NULL;
}
return trypos;
}
static int corr_ind_maxparen(pos_T *startpos)
{
long n = (long)startpos->lnum - (long)curwin->w_cursor.lnum;
if (n > 0 && n < curbuf->b_ind_maxparen / 2)
return curbuf->b_ind_maxparen - (int)n;
return curbuf->b_ind_maxparen;
}
static int find_last_paren(char_u *l, int start, int end)
{
int i;
int retval = FALSE;
int open_count = 0;
curwin->w_cursor.col = 0; 
for (i = 0; l[i] != NUL; i++) {
i = (int)(cin_skipcomment(l + i) - l); 
i = (int)(skip_string(l + i) - l); 
if (l[i] == start)
++open_count;
else if (l[i] == end) {
if (open_count > 0)
--open_count;
else {
curwin->w_cursor.col = i;
retval = TRUE;
}
}
}
return retval;
}
void parse_cino(buf_T *buf)
{
char_u *p;
char_u *l;
int divider;
int fraction = 0;
int sw = get_sw_value(buf);
buf->b_ind_level = sw;
buf->b_ind_open_imag = 0;
buf->b_ind_no_brace = 0;
buf->b_ind_first_open = 0;
buf->b_ind_open_extra = 0;
buf->b_ind_close_extra = 0;
buf->b_ind_open_left_imag = 0;
buf->b_ind_jump_label = -1;
buf->b_ind_case = sw;
buf->b_ind_case_code = sw;
buf->b_ind_case_break = 0;
buf->b_ind_scopedecl = sw;
buf->b_ind_scopedecl_code = sw;
buf->b_ind_param = sw;
buf->b_ind_func_type = sw;
buf->b_ind_cpp_baseclass = sw;
buf->b_ind_continuation = sw;
buf->b_ind_unclosed = sw * 2;
buf->b_ind_unclosed2 = sw;
buf->b_ind_unclosed_noignore = 0;
buf->b_ind_unclosed_wrapped = 0;
buf->b_ind_unclosed_whiteok = 0;
buf->b_ind_matching_paren = 0;
buf->b_ind_paren_prev = 0;
buf->b_ind_comment = 0;
buf->b_ind_in_comment = 3;
buf->b_ind_in_comment2 = 0;
buf->b_ind_maxparen = 20;
buf->b_ind_maxcomment = 70;
buf->b_ind_java = 0;
buf->b_ind_js = 0;
buf->b_ind_keep_case_label = 0;
buf->b_ind_cpp_namespace = 0;
buf->b_ind_if_for_while = 0;
buf->b_ind_hash_comment = 0;
buf->b_ind_cpp_extern_c = 0;
for (p = buf->b_p_cino; *p; ) {
l = p++;
if (*p == '-') {
p++;
}
char_u *digits_start = p; 
int n = getdigits_int(&p, true, 0);
divider = 0;
if (*p == '.') { 
fraction = atoi((char *)++p);
while (ascii_isdigit(*p)) {
p++;
if (divider) {
divider *= 10;
} else {
divider = 10;
}
}
}
if (*p == 's') { 
if (p == digits_start) {
n = sw; 
} else {
n *= sw;
if (divider)
n += (sw * fraction + divider / 2) / divider;
}
++p;
}
if (l[1] == '-')
n = -n;
switch (*l) {
case '>': buf->b_ind_level = n; break;
case 'e': buf->b_ind_open_imag = n; break;
case 'n': buf->b_ind_no_brace = n; break;
case 'f': buf->b_ind_first_open = n; break;
case '{': buf->b_ind_open_extra = n; break;
case '}': buf->b_ind_close_extra = n; break;
case '^': buf->b_ind_open_left_imag = n; break;
case 'L': buf->b_ind_jump_label = n; break;
case ':': buf->b_ind_case = n; break;
case '=': buf->b_ind_case_code = n; break;
case 'b': buf->b_ind_case_break = n; break;
case 'p': buf->b_ind_param = n; break;
case 't': buf->b_ind_func_type = n; break;
case '/': buf->b_ind_comment = n; break;
case 'c': buf->b_ind_in_comment = n; break;
case 'C': buf->b_ind_in_comment2 = n; break;
case 'i': buf->b_ind_cpp_baseclass = n; break;
case '+': buf->b_ind_continuation = n; break;
case '(': buf->b_ind_unclosed = n; break;
case 'u': buf->b_ind_unclosed2 = n; break;
case 'U': buf->b_ind_unclosed_noignore = n; break;
case 'W': buf->b_ind_unclosed_wrapped = n; break;
case 'w': buf->b_ind_unclosed_whiteok = n; break;
case 'm': buf->b_ind_matching_paren = n; break;
case 'M': buf->b_ind_paren_prev = n; break;
case ')': buf->b_ind_maxparen = n; break;
case '*': buf->b_ind_maxcomment = n; break;
case 'g': buf->b_ind_scopedecl = n; break;
case 'h': buf->b_ind_scopedecl_code = n; break;
case 'j': buf->b_ind_java = n; break;
case 'J': buf->b_ind_js = n; break;
case 'l': buf->b_ind_keep_case_label = n; break;
case '#': buf->b_ind_hash_comment = n; break;
case 'N': buf->b_ind_cpp_namespace = n; break;
case 'k': buf->b_ind_if_for_while = n; break;
case 'E': buf->b_ind_cpp_extern_c = n; break;
}
if (*p == ',')
++p;
}
}
int get_c_indent(void)
{
pos_T cur_curpos;
int amount;
int scope_amount;
int cur_amount = MAXCOL;
colnr_T col;
char_u *theline;
char_u *linecopy;
pos_T *trypos;
pos_T *comment_pos;
pos_T *tryposBrace = NULL;
pos_T tryposCopy;
pos_T our_paren_pos;
char_u *start;
int start_brace;
#define BRACE_IN_COL0 1 
#define BRACE_AT_START 2 
#define BRACE_AT_END 3 
linenr_T ourscope;
char_u *l;
char_u *look;
char_u terminated;
int lookfor;
#define LOOKFOR_INITIAL 0
#define LOOKFOR_IF 1
#define LOOKFOR_DO 2
#define LOOKFOR_CASE 3
#define LOOKFOR_ANY 4
#define LOOKFOR_TERM 5
#define LOOKFOR_UNTERM 6
#define LOOKFOR_SCOPEDECL 7
#define LOOKFOR_NOBREAK 8
#define LOOKFOR_CPP_BASECLASS 9
#define LOOKFOR_ENUM_OR_INIT 10
#define LOOKFOR_JS_KEY 11
#define LOOKFOR_COMMA 12
int whilelevel;
linenr_T lnum;
int n;
int iscase;
int lookfor_break;
int lookfor_cpp_namespace = FALSE;
int cont_amount = 0; 
int original_line_islabel;
int added_to_amount = 0;
linenr_T raw_string_start = 0;
cpp_baseclass_cache_T cache_cpp_baseclass = { false, { MAXLNUM, 0 } };
int ind_continuation = curbuf->b_ind_continuation;
cur_curpos = curwin->w_cursor;
if (cur_curpos.lnum == 1)
return 0;
linecopy = vim_strsave(ml_get(cur_curpos.lnum));
if ((State & INSERT)
&& curwin->w_cursor.col < (colnr_T)STRLEN(linecopy)
&& linecopy[curwin->w_cursor.col] == ')')
linecopy[curwin->w_cursor.col] = NUL;
theline = skipwhite(linecopy);
curwin->w_cursor.col = 0;
original_line_islabel = cin_islabel(); 
comment_pos = ind_find_start_comment();
if (comment_pos != NULL) {
tryposCopy = *comment_pos;
comment_pos = &tryposCopy;
}
trypos = find_start_rawstring(curbuf->b_ind_maxcomment);
if (trypos != NULL && (comment_pos == NULL || lt(*trypos, *comment_pos))) {
amount = -1;
goto laterend;
}
if (*theline == '#' && (*linecopy == '#' || in_cinkeys('#', ' ', true))) {
amount = curbuf->b_ind_hash_comment;
goto theend;
}
if (original_line_islabel && !curbuf->b_ind_js
&& curbuf->b_ind_jump_label < 0) {
amount = 0;
goto theend;
}
if (cin_islinecomment(theline)
&& (trypos = find_line_comment()) != NULL) { 
getvcol(curwin, trypos, &col, NULL, NULL);
amount = col;
goto theend;
}
if (!cin_iscomment(theline) && comment_pos != NULL) { 
int lead_start_len = 2;
int lead_middle_len = 1;
char_u lead_start[COM_MAX_LEN]; 
char_u lead_middle[COM_MAX_LEN]; 
char_u lead_end[COM_MAX_LEN]; 
char_u *p;
int start_align = 0;
int start_off = 0;
int done = FALSE;
getvcol(curwin, comment_pos, &col, NULL, NULL);
amount = col;
*lead_start = NUL;
*lead_middle = NUL;
p = curbuf->b_p_com;
while (*p != NUL) {
int align = 0;
int off = 0;
int what = 0;
while (*p != NUL && *p != ':') {
if (*p == COM_START || *p == COM_END || *p == COM_MIDDLE) {
what = *p++;
} else if (*p == COM_LEFT || *p == COM_RIGHT) {
align = *p++;
} else if (ascii_isdigit(*p) || *p == '-') {
off = getdigits_int(&p, true, 0);
} else {
p++;
}
}
if (*p == ':')
++p;
(void)copy_option_part(&p, lead_end, COM_MAX_LEN, ",");
if (what == COM_START) {
STRCPY(lead_start, lead_end);
lead_start_len = (int)STRLEN(lead_start);
start_off = off;
start_align = align;
} else if (what == COM_MIDDLE) {
STRCPY(lead_middle, lead_end);
lead_middle_len = (int)STRLEN(lead_middle);
} else if (what == COM_END) {
if (STRNCMP(theline, lead_middle, lead_middle_len) == 0
&& STRNCMP(theline, lead_end, STRLEN(lead_end)) != 0) {
done = TRUE;
if (curwin->w_cursor.lnum > 1) {
look = skipwhite(ml_get(curwin->w_cursor.lnum - 1));
if (STRNCMP(look, lead_start, lead_start_len) == 0)
amount = get_indent_lnum(curwin->w_cursor.lnum - 1);
else if (STRNCMP(look, lead_middle,
lead_middle_len) == 0) {
amount = get_indent_lnum(curwin->w_cursor.lnum - 1);
break;
} else if (STRNCMP(ml_get(comment_pos->lnum) + comment_pos->col,
lead_start, lead_start_len) != 0) {
continue;
}
}
if (start_off != 0)
amount += start_off;
else if (start_align == COM_RIGHT)
amount += vim_strsize(lead_start)
- vim_strsize(lead_middle);
break;
}
if (STRNCMP(theline, lead_middle, lead_middle_len) != 0
&& STRNCMP(theline, lead_end, STRLEN(lead_end)) == 0) {
amount = get_indent_lnum(curwin->w_cursor.lnum - 1);
if (off != 0)
amount += off;
else if (align == COM_RIGHT)
amount += vim_strsize(lead_start)
- vim_strsize(lead_middle);
done = TRUE;
break;
}
}
}
if (done)
;
else if (theline[0] == '*')
amount += 1;
else {
amount = -1;
for (lnum = cur_curpos.lnum - 1; lnum > comment_pos->lnum; --lnum) {
if (linewhite(lnum)) 
continue;
amount = get_indent_lnum(lnum); 
break;
}
if (amount == -1) { 
if (!curbuf->b_ind_in_comment2) {
start = ml_get(comment_pos->lnum);
look = start + comment_pos->col + 2; 
if (*look != NUL) 
comment_pos->col = (colnr_T)(skipwhite(look) - start);
}
getvcol(curwin, comment_pos, &col, NULL, NULL);
amount = col;
if (curbuf->b_ind_in_comment2 || *look == NUL)
amount += curbuf->b_ind_in_comment;
}
}
goto theend;
}
if (*skipwhite(theline) == ']'
&& (trypos = find_match_char('[', curbuf->b_ind_maxparen)) != NULL) {
amount = get_indent_lnum(trypos->lnum);
goto theend;
}
if (((trypos = find_match_paren(curbuf->b_ind_maxparen)) != NULL
&& curbuf->b_ind_java == 0)
|| (tryposBrace = find_start_brace()) != NULL
|| trypos != NULL) {
if (trypos != NULL && tryposBrace != NULL) {
if (trypos->lnum != tryposBrace->lnum
? trypos->lnum < tryposBrace->lnum
: trypos->col < tryposBrace->col)
trypos = NULL;
else
tryposBrace = NULL;
}
if (trypos != NULL) {
our_paren_pos = *trypos;
if (theline[0] == ')' && curbuf->b_ind_paren_prev) {
amount = get_indent_lnum(curwin->w_cursor.lnum - 1); 
} else {
amount = -1;
for (lnum = cur_curpos.lnum - 1; lnum > our_paren_pos.lnum; --lnum) {
l = skipwhite(ml_get(lnum));
if (cin_nocode(l)) { 
continue;
}
if (cin_ispreproc_cont(&l, &lnum, &amount)) {
continue; 
}
curwin->w_cursor.lnum = lnum;
if ((trypos = ind_find_start_CORS(NULL)) != NULL) {
lnum = trypos->lnum + 1;
continue;
}
if ((trypos = find_match_paren(
corr_ind_maxparen(&cur_curpos))) != NULL
&& trypos->lnum == our_paren_pos.lnum
&& trypos->col == our_paren_pos.col) {
amount = get_indent_lnum(lnum); 
if (theline[0] == ')') {
if (our_paren_pos.lnum != lnum
&& cur_amount > amount)
cur_amount = amount;
amount = -1;
}
break;
}
}
}
if (amount == -1) {
int ignore_paren_col = 0;
int is_if_for_while = 0;
if (curbuf->b_ind_if_for_while) {
pos_T cursor_save = curwin->w_cursor;
pos_T outermost;
char_u *line;
trypos = &our_paren_pos;
do {
outermost = *trypos;
curwin->w_cursor.lnum = outermost.lnum;
curwin->w_cursor.col = outermost.col;
trypos = find_match_paren(curbuf->b_ind_maxparen);
} while (trypos && trypos->lnum == outermost.lnum);
curwin->w_cursor = cursor_save;
line = ml_get(outermost.lnum);
is_if_for_while =
cin_is_if_for_while_before_offset(line, &outermost.col);
}
amount = skip_label(our_paren_pos.lnum, &look);
look = skipwhite(look);
if (*look == '(') {
linenr_T save_lnum = curwin->w_cursor.lnum;
char_u *line;
int look_col;
curwin->w_cursor.lnum = our_paren_pos.lnum;
line = get_cursor_line_ptr();
look_col = (int)(look - line);
curwin->w_cursor.col = look_col + 1;
if ((trypos = findmatchlimit(NULL, ')', 0,
curbuf->b_ind_maxparen))
!= NULL
&& trypos->lnum == our_paren_pos.lnum
&& trypos->col < our_paren_pos.col)
ignore_paren_col = trypos->col + 1;
curwin->w_cursor.lnum = save_lnum;
look = ml_get(our_paren_pos.lnum) + look_col;
}
if (theline[0] == ')' || (curbuf->b_ind_unclosed == 0
&& is_if_for_while == 0)
|| (!curbuf->b_ind_unclosed_noignore && *look == '('
&& ignore_paren_col == 0)) {
if (theline[0] != ')') {
cur_amount = MAXCOL;
l = ml_get(our_paren_pos.lnum);
if (curbuf->b_ind_unclosed_wrapped
&& cin_ends_in(l, (char_u *)"(", NULL)) {
n = 1;
for (col = 0; col < our_paren_pos.col; ++col) {
switch (l[col]) {
case '(':
case '{': ++n;
break;
case ')':
case '}': if (n > 1)
--n;
break;
}
}
our_paren_pos.col = 0;
amount += n * curbuf->b_ind_unclosed_wrapped;
} else if (curbuf->b_ind_unclosed_whiteok)
our_paren_pos.col++;
else {
col = our_paren_pos.col + 1;
while (ascii_iswhite(l[col]))
col++;
if (l[col] != NUL) 
our_paren_pos.col = col;
else
our_paren_pos.col++;
}
}
if (our_paren_pos.col > 0) {
getvcol(curwin, &our_paren_pos, &col, NULL, NULL);
if (cur_amount > (int)col)
cur_amount = col;
}
}
if (theline[0] == ')' && curbuf->b_ind_matching_paren) {
} else if ((curbuf->b_ind_unclosed == 0 && is_if_for_while == 0)
|| (!curbuf->b_ind_unclosed_noignore
&& *look == '(' && ignore_paren_col == 0)) {
if (cur_amount != MAXCOL)
amount = cur_amount;
} else {
col = our_paren_pos.col;
while ((int)our_paren_pos.col > ignore_paren_col) {
--our_paren_pos.col;
switch (*ml_get_pos(&our_paren_pos)) {
case '(': amount += curbuf->b_ind_unclosed2;
col = our_paren_pos.col;
break;
case ')': amount -= curbuf->b_ind_unclosed2;
col = MAXCOL;
break;
}
}
if (col == MAXCOL)
amount += curbuf->b_ind_unclosed;
else {
curwin->w_cursor.lnum = our_paren_pos.lnum;
curwin->w_cursor.col = col;
if (find_match_paren_after_brace(curbuf->b_ind_maxparen)) {
amount += curbuf->b_ind_unclosed2;
} else {
if (is_if_for_while) {
amount += curbuf->b_ind_if_for_while;
} else {
amount += curbuf->b_ind_unclosed;
}
}
}
if (cur_amount < amount)
amount = cur_amount;
}
}
if (cin_iscomment(theline))
amount += curbuf->b_ind_comment;
} else {
tryposCopy = *tryposBrace;
tryposBrace = &tryposCopy;
trypos = tryposBrace;
ourscope = trypos->lnum;
start = ml_get(ourscope);
look = skipwhite(start);
if (*look == '{') {
getvcol(curwin, trypos, &col, NULL, NULL);
amount = col;
if (*start == '{')
start_brace = BRACE_IN_COL0;
else
start_brace = BRACE_AT_START;
} else {
curwin->w_cursor.lnum = ourscope;
lnum = ourscope;
if (find_last_paren(start, '(', ')')
&& (trypos = find_match_paren(curbuf->b_ind_maxparen)) != NULL) {
lnum = trypos->lnum;
}
if ((curbuf->b_ind_js || curbuf->b_ind_keep_case_label)
&& cin_iscase(skipwhite(get_cursor_line_ptr()), FALSE)) {
amount = get_indent();
} else if (curbuf->b_ind_js) {
amount = get_indent_lnum(lnum);
} else {
amount = skip_label(lnum, &l);
}
start_brace = BRACE_AT_END;
}
bool js_cur_has_key = curbuf->b_ind_js ? cin_has_js_key(theline) : false;
if (theline[0] == '}') {
amount += curbuf->b_ind_close_extra;
} else {
lookfor = LOOKFOR_INITIAL;
if (cin_iselse(theline))
lookfor = LOOKFOR_IF;
else if (cin_iswhileofdo(theline, cur_curpos.lnum)) 
lookfor = LOOKFOR_DO;
if (lookfor != LOOKFOR_INITIAL) {
curwin->w_cursor.lnum = cur_curpos.lnum;
if (find_match(lookfor, ourscope) == OK) {
amount = get_indent(); 
goto theend;
}
}
if (start_brace == BRACE_IN_COL0) { 
amount = curbuf->b_ind_open_left_imag;
lookfor_cpp_namespace = true;
} else if (start_brace == BRACE_AT_START
&& lookfor_cpp_namespace) { 
lookfor_cpp_namespace = true;
} else {
if (start_brace == BRACE_AT_END) { 
amount += curbuf->b_ind_open_imag;
l = skipwhite(get_cursor_line_ptr());
if (cin_is_cpp_namespace(l)) {
amount += curbuf->b_ind_cpp_namespace;
} else if (cin_is_cpp_extern_c(l)) {
amount += curbuf->b_ind_cpp_extern_c;
}
} else {
amount -= curbuf->b_ind_open_extra;
if (amount < 0)
amount = 0;
}
}
lookfor_break = FALSE;
if (cin_iscase(theline, FALSE)) { 
lookfor = LOOKFOR_CASE; 
amount += curbuf->b_ind_case;
} else if (cin_isscopedecl(theline)) { 
lookfor = LOOKFOR_SCOPEDECL; 
amount += curbuf->b_ind_scopedecl;
} else {
if (curbuf->b_ind_case_break && cin_isbreak(theline))
lookfor_break = TRUE;
lookfor = LOOKFOR_INITIAL;
amount += curbuf->b_ind_level;
}
scope_amount = amount;
whilelevel = 0;
curwin->w_cursor = cur_curpos;
for (;; ) {
curwin->w_cursor.lnum--;
curwin->w_cursor.col = 0;
if (curwin->w_cursor.lnum <= ourscope) {
if (lookfor == LOOKFOR_ENUM_OR_INIT) {
if (curwin->w_cursor.lnum == 0
|| curwin->w_cursor.lnum
< ourscope - curbuf->b_ind_maxparen) {
if (cont_amount > 0)
amount = cont_amount;
else if (!curbuf->b_ind_js)
amount += ind_continuation;
break;
}
l = get_cursor_line_ptr();
trypos = ind_find_start_CORS(NULL);
if (trypos != NULL) {
curwin->w_cursor.lnum = trypos->lnum + 1;
curwin->w_cursor.col = 0;
continue;
}
if (cin_ispreproc_cont(&l, &curwin->w_cursor.lnum, &amount)) {
continue;
}
if (cin_nocode(l))
continue;
terminated = cin_isterminated(l, FALSE, TRUE);
if (start_brace != BRACE_IN_COL0
|| !cin_isfuncdecl(&l, curwin->w_cursor.lnum, 0)) {
if (terminated == ',')
break;
if (terminated != ';' && cin_isinit())
break;
if (terminated == 0 || terminated == '{')
continue;
}
if (terminated != ';') {
trypos = NULL;
if (find_last_paren(l, '(', ')'))
trypos = find_match_paren(
curbuf->b_ind_maxparen);
if (trypos == NULL && find_last_paren(l, '{', '}'))
trypos = find_start_brace();
if (trypos != NULL) {
curwin->w_cursor.lnum = trypos->lnum + 1;
curwin->w_cursor.col = 0;
continue;
}
}
if (cont_amount > 0)
amount = cont_amount;
else
amount += ind_continuation;
} else if (lookfor == LOOKFOR_UNTERM) {
if (cont_amount > 0)
amount = cont_amount;
else
amount += ind_continuation;
} else {
if (lookfor != LOOKFOR_TERM
&& lookfor != LOOKFOR_CPP_BASECLASS
&& lookfor != LOOKFOR_COMMA) {
amount = scope_amount;
if (theline[0] == '{') {
amount += curbuf->b_ind_open_extra;
added_to_amount = curbuf->b_ind_open_extra;
}
}
if (lookfor_cpp_namespace) {
if (curwin->w_cursor.lnum == ourscope)
continue;
if (curwin->w_cursor.lnum == 0
|| curwin->w_cursor.lnum
< ourscope - FIND_NAMESPACE_LIM)
break;
l = get_cursor_line_ptr();
trypos = ind_find_start_CORS(NULL);
if (trypos != NULL) {
curwin->w_cursor.lnum = trypos->lnum + 1;
curwin->w_cursor.col = 0;
continue;
}
if (cin_ispreproc_cont(&l, &curwin->w_cursor.lnum, &amount)) {
continue;
}
if (cin_is_cpp_namespace(l)) {
amount += curbuf->b_ind_cpp_namespace
- added_to_amount;
break;
} else if (cin_is_cpp_extern_c(l)) {
amount += curbuf->b_ind_cpp_extern_c - added_to_amount;
break;
}
if (cin_nocode(l))
continue;
}
}
break;
}
if ((trypos = ind_find_start_CORS(&raw_string_start)) != NULL) {
curwin->w_cursor.lnum = trypos->lnum + 1;
curwin->w_cursor.col = 0;
continue;
}
l = get_cursor_line_ptr();
iscase = cin_iscase(l, FALSE);
if (iscase || cin_isscopedecl(l)) {
if (lookfor == LOOKFOR_CPP_BASECLASS)
break;
if (whilelevel > 0)
continue;
if (lookfor == LOOKFOR_UNTERM
|| lookfor == LOOKFOR_ENUM_OR_INIT) {
if (cont_amount > 0)
amount = cont_amount;
else
amount += ind_continuation;
break;
}
if ( (iscase && lookfor == LOOKFOR_CASE)
|| (iscase && lookfor_break)
|| (!iscase && lookfor == LOOKFOR_SCOPEDECL)) {
if ((trypos = find_start_brace()) == NULL
|| trypos->lnum == ourscope) {
amount = get_indent(); 
break;
}
continue;
}
n = get_indent_nolabel(curwin->w_cursor.lnum); 
if (lookfor == LOOKFOR_TERM) {
if (n)
amount = n;
if (!lookfor_break)
break;
}
if (n) {
amount = n;
l = after_label(get_cursor_line_ptr());
if (l != NULL && cin_is_cinword(l)) {
if (theline[0] == '{')
amount += curbuf->b_ind_open_extra;
else
amount += curbuf->b_ind_level
+ curbuf->b_ind_no_brace;
}
break;
}
scope_amount = get_indent() + (iscase 
? curbuf->b_ind_case_code
: curbuf->b_ind_scopedecl_code);
lookfor = curbuf->b_ind_case_break
? LOOKFOR_NOBREAK : LOOKFOR_ANY;
continue;
}
if (lookfor == LOOKFOR_CASE || lookfor == LOOKFOR_SCOPEDECL) {
if (find_last_paren(l, '{', '}')
&& (trypos = find_start_brace()) != NULL) {
curwin->w_cursor.lnum = trypos->lnum + 1;
curwin->w_cursor.col = 0;
}
continue;
}
if (!curbuf->b_ind_js && cin_islabel()) {
l = after_label(get_cursor_line_ptr());
if (l == NULL || cin_nocode(l))
continue;
}
l = get_cursor_line_ptr();
if (cin_ispreproc_cont(&l, &curwin->w_cursor.lnum, &amount)
|| cin_nocode(l)) {
continue;
}
n = FALSE;
if (lookfor != LOOKFOR_TERM && curbuf->b_ind_cpp_baseclass > 0) {
n = cin_is_cpp_baseclass(&cache_cpp_baseclass);
l = get_cursor_line_ptr();
}
if (n) {
if (lookfor == LOOKFOR_UNTERM) {
if (cont_amount > 0)
amount = cont_amount;
else
amount += ind_continuation;
} else if (theline[0] == '{') {
lookfor = LOOKFOR_UNTERM;
ind_continuation = 0;
continue;
} else
amount = get_baseclass_amount(cache_cpp_baseclass.lpos.col);
break;
} else if (lookfor == LOOKFOR_CPP_BASECLASS) {
if (cin_isterminated(l, TRUE, FALSE))
break;
else
continue;
}
terminated = cin_isterminated(l, FALSE, TRUE);
if (js_cur_has_key) {
js_cur_has_key = false; 
if (curbuf->b_ind_js && terminated == ',') {
lookfor = LOOKFOR_JS_KEY;
}
}
if (lookfor == LOOKFOR_JS_KEY && cin_has_js_key(l)) {
amount = get_indent();
break;
}
if (lookfor == LOOKFOR_COMMA) {
if (tryposBrace != NULL && tryposBrace->lnum
>= curwin->w_cursor.lnum) {
break;
}
if (terminated == ',') {
break;
} else {
amount = get_indent();
if (curwin->w_cursor.lnum - 1 == ourscope) {
break;
}
}
}
if (terminated == 0 || (lookfor != LOOKFOR_UNTERM
&& terminated == ',')) {
if (lookfor != LOOKFOR_ENUM_OR_INIT
&& (*skipwhite(l) == '[' || l[STRLEN(l) - 1] == '[')) {
amount += ind_continuation;
}
(void)find_last_paren(l, '(', ')');
trypos = find_match_paren(corr_ind_maxparen(&cur_curpos));
if (trypos != NULL && (trypos->lnum < tryposBrace->lnum
|| (trypos->lnum == tryposBrace->lnum
&& trypos->col < tryposBrace->col))) {
trypos = NULL;
}
if (trypos == NULL && terminated == ','
&& find_last_paren(l, '{', '}'))
trypos = find_start_brace();
if (trypos != NULL) {
curwin->w_cursor = *trypos;
l = get_cursor_line_ptr();
if (cin_iscase(l, FALSE) || cin_isscopedecl(l)) {
++curwin->w_cursor.lnum;
curwin->w_cursor.col = 0;
continue;
}
}
if (terminated == ',') {
while (curwin->w_cursor.lnum > 1) {
l = ml_get(curwin->w_cursor.lnum - 1);
if (*l == NUL || l[STRLEN(l) - 1] != '\\')
break;
--curwin->w_cursor.lnum;
curwin->w_cursor.col = 0;
}
}
if (curbuf->b_ind_js) {
cur_amount = get_indent();
} else {
cur_amount = skip_label(curwin->w_cursor.lnum, &l);
}
if (terminated != ',' && lookfor != LOOKFOR_TERM
&& theline[0] == '{') {
amount = cur_amount;
if (*skipwhite(l) != '{')
amount += curbuf->b_ind_open_extra;
if (curbuf->b_ind_cpp_baseclass && !curbuf->b_ind_js) {
lookfor = LOOKFOR_CPP_BASECLASS;
continue;
}
break;
}
if (cin_is_cinword(l) || cin_iselse(skipwhite(l))) {
if (lookfor == LOOKFOR_UNTERM
|| lookfor == LOOKFOR_ENUM_OR_INIT) {
if (cont_amount > 0)
amount = cont_amount;
else
amount += ind_continuation;
break;
}
amount = cur_amount;
if (theline[0] == '{')
amount += curbuf->b_ind_open_extra;
if (lookfor != LOOKFOR_TERM) {
amount += curbuf->b_ind_level
+ curbuf->b_ind_no_brace;
break;
}
l = skipwhite(get_cursor_line_ptr());
if (cin_isdo(l)) {
if (whilelevel == 0)
break;
--whilelevel;
}
if (cin_iselse(l) && whilelevel == 0) {
if (*l == '}')
curwin->w_cursor.col =
(colnr_T)(l - get_cursor_line_ptr()) + 1;
if ((trypos = find_start_brace()) == NULL
|| find_match(LOOKFOR_IF, trypos->lnum)
== FAIL)
break;
}
}
else {
if (lookfor == LOOKFOR_UNTERM) {
if (terminated == ',')
amount += ind_continuation;
break;
}
if (lookfor == LOOKFOR_ENUM_OR_INIT) {
if (terminated == ',') {
if (curbuf->b_ind_cpp_baseclass == 0)
break;
lookfor = LOOKFOR_CPP_BASECLASS;
continue;
}
if (amount > cur_amount) {
amount = cur_amount;
}
} else {
l = get_cursor_line_ptr();
amount = cur_amount;
n = (int)STRLEN(l);
if (terminated == ','
&& (*skipwhite(l) == ']'
|| (n >=2 && l[n - 2] == ']'))) {
break;
}
if (lookfor == LOOKFOR_INITIAL && terminated == ',') {
if (curbuf->b_ind_js) {
if (cin_iscomment(skipwhite(l))) {
break;
}
lookfor = LOOKFOR_COMMA;
trypos = find_match_char('[', curbuf->b_ind_maxparen);
if (trypos != NULL) {
if (trypos->lnum == curwin->w_cursor.lnum - 1) {
break;
}
ourscope = trypos->lnum;
}
} else {
lookfor = LOOKFOR_ENUM_OR_INIT;
cont_amount = cin_first_id_amount();
}
} else {
if (lookfor == LOOKFOR_INITIAL
&& *l != NUL
&& l[STRLEN(l) - 1] == '\\') {
cont_amount = cin_get_equal_amount( curwin->w_cursor.lnum);
}
if (lookfor != LOOKFOR_TERM
&& lookfor != LOOKFOR_JS_KEY
&& lookfor != LOOKFOR_COMMA
&& raw_string_start != curwin->w_cursor.lnum) {
lookfor = LOOKFOR_UNTERM;
}
}
}
}
}
else if (cin_iswhileofdo_end(terminated)) { 
if (lookfor == LOOKFOR_UNTERM
|| lookfor == LOOKFOR_ENUM_OR_INIT) {
if (cont_amount > 0)
amount = cont_amount;
else
amount += ind_continuation;
break;
}
if (whilelevel == 0) {
lookfor = LOOKFOR_TERM;
amount = get_indent(); 
if (theline[0] == '{')
amount += curbuf->b_ind_open_extra;
}
++whilelevel;
}
else {
if (lookfor == LOOKFOR_NOBREAK
&& cin_isbreak(skipwhite(get_cursor_line_ptr()))) {
lookfor = LOOKFOR_ANY;
continue;
}
if (whilelevel > 0) {
l = cin_skipcomment(get_cursor_line_ptr());
if (cin_isdo(l)) {
amount = get_indent(); 
--whilelevel;
continue;
}
}
if (lookfor == LOOKFOR_UNTERM
|| lookfor == LOOKFOR_ENUM_OR_INIT) {
if (cont_amount > 0)
amount = cont_amount;
else
amount += ind_continuation;
break;
}
if (lookfor == LOOKFOR_TERM) {
if (!lookfor_break && whilelevel == 0)
break;
}
else {
term_again:
l = get_cursor_line_ptr();
if (find_last_paren(l, '(', ')')
&& (trypos = find_match_paren(
curbuf->b_ind_maxparen)) != NULL) {
curwin->w_cursor = *trypos;
l = get_cursor_line_ptr();
if (cin_iscase(l, FALSE) || cin_isscopedecl(l)) {
++curwin->w_cursor.lnum;
curwin->w_cursor.col = 0;
continue;
}
}
iscase = (curbuf->b_ind_keep_case_label
&& cin_iscase(l, FALSE));
amount = skip_label(curwin->w_cursor.lnum, &l);
if (theline[0] == '{')
amount += curbuf->b_ind_open_extra;
l = skipwhite(l);
if (*l == '{')
amount -= curbuf->b_ind_open_extra;
lookfor = iscase ? LOOKFOR_ANY : LOOKFOR_TERM;
if (lookfor == LOOKFOR_TERM
&& *l != '}'
&& cin_iselse(l)
&& whilelevel == 0) {
if ((trypos = find_start_brace()) == NULL
|| find_match(LOOKFOR_IF, trypos->lnum)
== FAIL)
break;
continue;
}
l = get_cursor_line_ptr();
if (find_last_paren(l, '{', '}') 
&& (trypos = find_start_brace()) != NULL) {
curwin->w_cursor = *trypos;
l = cin_skipcomment(get_cursor_line_ptr());
if (*l == '}' || !cin_iselse(l))
goto term_again;
++curwin->w_cursor.lnum;
curwin->w_cursor.col = 0;
}
}
}
}
}
}
if (cin_iscomment(theline))
amount += curbuf->b_ind_comment;
if (curbuf->b_ind_jump_label > 0 && original_line_islabel)
amount -= curbuf->b_ind_jump_label;
goto theend;
}
if (theline[0] == '{') {
amount = curbuf->b_ind_first_open;
goto theend;
}
if (cur_curpos.lnum < curbuf->b_ml.ml_line_count
&& !cin_nocode(theline)
&& vim_strchr(theline, '{') == NULL
&& vim_strchr(theline, '}') == NULL
&& !cin_ends_in(theline, (char_u *)":", NULL)
&& !cin_ends_in(theline, (char_u *)",", NULL)
&& cin_isfuncdecl(NULL, cur_curpos.lnum + 1,
cur_curpos.lnum + 1)
&& !cin_isterminated(theline, false, true)) {
amount = curbuf->b_ind_func_type;
goto theend;
}
amount = 0;
curwin->w_cursor = cur_curpos;
while (curwin->w_cursor.lnum > 1) {
curwin->w_cursor.lnum--;
curwin->w_cursor.col = 0;
l = get_cursor_line_ptr();
if ((trypos = ind_find_start_CORS(NULL)) != NULL) {
curwin->w_cursor.lnum = trypos->lnum + 1;
curwin->w_cursor.col = 0;
continue;
}
n = 0;
if (curbuf->b_ind_cpp_baseclass != 0 && theline[0] != '{') {
n = cin_is_cpp_baseclass(&cache_cpp_baseclass);
l = get_cursor_line_ptr();
}
if (n) {
amount = get_baseclass_amount(cache_cpp_baseclass.lpos.col);
break;
}
if (cin_ispreproc_cont(&l, &curwin->w_cursor.lnum, &amount)) {
continue;
}
if (cin_nocode(l))
continue;
if (cin_ends_in(l, (char_u *)",", NULL)
|| (*l != NUL && (n = l[STRLEN(l) - 1]) == '\\')) {
if (find_last_paren(l, '(', ')')
&& (trypos = find_match_paren(
curbuf->b_ind_maxparen)) != NULL)
curwin->w_cursor = *trypos;
while (n == 0 && curwin->w_cursor.lnum > 1) {
l = ml_get(curwin->w_cursor.lnum - 1);
if (*l == NUL || l[STRLEN(l) - 1] != '\\')
break;
--curwin->w_cursor.lnum;
curwin->w_cursor.col = 0;
}
amount = get_indent(); 
if (amount == 0)
amount = cin_first_id_amount();
if (amount == 0)
amount = ind_continuation;
break;
}
if (cin_isfuncdecl(NULL, cur_curpos.lnum, 0)) 
break;
l = get_cursor_line_ptr();
if (*skipwhite(l) == '}')
break;
if (cin_ends_in(l, (char_u *)"};", NULL))
break;
if (cin_ends_in(l, (char_u *)"[", NULL)) {
amount = get_indent() + ind_continuation;
break;
}
if (*(look = skipwhite(l)) == ';' && cin_nocode(look + 1)) {
pos_T curpos_save = curwin->w_cursor;
while (curwin->w_cursor.lnum > 1) {
look = ml_get(--curwin->w_cursor.lnum);
if (!(cin_nocode(look)
|| cin_ispreproc_cont(&look, &curwin->w_cursor.lnum, &amount))) {
break;
}
}
if (curwin->w_cursor.lnum > 0
&& cin_ends_in(look, (char_u *)"}", NULL))
break;
curwin->w_cursor = curpos_save;
}
if (cin_isfuncdecl(&l, curwin->w_cursor.lnum, 0)) {
amount = curbuf->b_ind_param;
break;
}
if (cin_ends_in(l, (char_u *)";", NULL)) {
l = ml_get(curwin->w_cursor.lnum - 1);
if (cin_ends_in(l, (char_u *)",", NULL)
|| (*l != NUL && l[STRLEN(l) - 1] == '\\'))
break;
l = get_cursor_line_ptr();
}
find_last_paren(l, '(', ')');
if ((trypos = find_match_paren(curbuf->b_ind_maxparen)) != NULL)
curwin->w_cursor = *trypos;
amount = get_indent(); 
break;
}
if (cin_iscomment(theline))
amount += curbuf->b_ind_comment;
if (cur_curpos.lnum > 1) {
l = ml_get(cur_curpos.lnum - 1);
if (*l != NUL && l[STRLEN(l) - 1] == '\\') {
cur_amount = cin_get_equal_amount(cur_curpos.lnum - 1);
if (cur_amount > 0)
amount = cur_amount;
else if (cur_amount == 0)
amount += ind_continuation;
}
}
theend:
if (amount < 0)
amount = 0;
laterend:
curwin->w_cursor = cur_curpos;
xfree(linecopy);
return amount;
}
static int find_match(int lookfor, linenr_T ourscope)
{
char_u *look;
pos_T *theirscope;
char_u *mightbeif;
int elselevel;
int whilelevel;
if (lookfor == LOOKFOR_IF) {
elselevel = 1;
whilelevel = 0;
} else {
elselevel = 0;
whilelevel = 1;
}
curwin->w_cursor.col = 0;
while (curwin->w_cursor.lnum > ourscope + 1) {
curwin->w_cursor.lnum--;
curwin->w_cursor.col = 0;
look = cin_skipcomment(get_cursor_line_ptr());
if (!cin_iselse(look)
&& !cin_isif(look)
&& !cin_isdo(look) 
&& !cin_iswhileofdo(look, curwin->w_cursor.lnum)) {
continue;
}
theirscope = find_start_brace(); 
if (theirscope == NULL)
break;
if (theirscope->lnum < ourscope)
break;
if (theirscope->lnum > ourscope)
continue;
look = cin_skipcomment(get_cursor_line_ptr());
if (cin_iselse(look)) {
mightbeif = cin_skipcomment(look + 4);
if (!cin_isif(mightbeif))
++elselevel;
continue;
}
if (cin_iswhileofdo(look, curwin->w_cursor.lnum)) {
++whilelevel;
continue;
}
look = cin_skipcomment(get_cursor_line_ptr());
if (cin_isif(look)) {
elselevel--;
if (elselevel == 0 && lookfor == LOOKFOR_IF)
whilelevel = 0;
}
if (cin_isdo(look))
whilelevel--;
if (elselevel <= 0 && whilelevel <= 0) {
return OK;
}
}
return FAIL;
}
void do_c_expr_indent(void)
{
if (*curbuf->b_p_inde != NUL)
fixthisline(get_expr_indent);
else
fixthisline(get_c_indent);
}
