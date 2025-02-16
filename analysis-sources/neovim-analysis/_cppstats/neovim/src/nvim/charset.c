#include <assert.h>
#include <string.h>
#include <wctype.h>
#include <inttypes.h>
#include "nvim/vim.h"
#include "nvim/ascii.h"
#include "nvim/charset.h"
#include "nvim/func_attr.h"
#include "nvim/indent.h"
#include "nvim/main.h"
#include "nvim/mark.h"
#include "nvim/mbyte.h"
#include "nvim/memline.h"
#include "nvim/memory.h"
#include "nvim/misc1.h"
#include "nvim/garray.h"
#include "nvim/move.h"
#include "nvim/option.h"
#include "nvim/os_unix.h"
#include "nvim/state.h"
#include "nvim/strings.h"
#include "nvim/path.h"
#include "nvim/cursor.h"
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "charset.c.generated.h"
#endif
static bool chartab_initialized = false;
#define SET_CHARTAB(buf, c) (buf)->b_chartab[(unsigned)(c) >> 6] |= (1ull << ((c) & 0x3f))
#define RESET_CHARTAB(buf, c) (buf)->b_chartab[(unsigned)(c) >> 6] &= ~(1ull << ((c) & 0x3f))
#define GET_CHARTAB_TAB(chartab, c) ((chartab)[(unsigned)(c) >> 6] & (1ull << ((c) & 0x3f)))
#define GET_CHARTAB(buf, c) GET_CHARTAB_TAB((buf)->b_chartab, c)
static char_u g_chartab[256];
#define CT_CELL_MASK 0x07 
#define CT_PRINT_CHAR 0x10 
#define CT_ID_CHAR 0x20 
#define CT_FNAME_CHAR 0x40 
int init_chartab(void)
{
return buf_init_chartab(curbuf, true);
}
int buf_init_chartab(buf_T *buf, int global)
{
int c;
int c2;
int i;
bool tilde;
bool do_isalpha;
if (global) {
c = 0;
while (c < ' ') {
g_chartab[c++] = (dy_flags & DY_UHEX) ? 4 : 2;
}
while (c <= '~') {
g_chartab[c++] = 1 + CT_PRINT_CHAR;
}
while (c < 256) {
if (c >= 0xa0) {
g_chartab[c++] = CT_PRINT_CHAR + 1;
} else {
g_chartab[c++] = (dy_flags & DY_UHEX) ? 4 : 2;
}
}
for (c = 1; c < 256; c++) {
if (c >= 0xa0) {
g_chartab[c] |= CT_FNAME_CHAR;
}
}
}
memset(buf->b_chartab, 0, (size_t)32);
if (buf->b_p_lisp) {
SET_CHARTAB(buf, '-');
}
for (i = global ? 0 : 3; i <= 3; i++) {
const char_u *p;
if (i == 0) {
p = p_isi;
} else if (i == 1) {
p = p_isp;
} else if (i == 2) {
p = p_isf;
} else { 
p = buf->b_p_isk;
}
while (*p) {
tilde = false;
do_isalpha = false;
if ((*p == '^') && (p[1] != NUL)) {
tilde = true;
++p;
}
if (ascii_isdigit(*p)) {
c = getdigits_int((char_u **)&p, true, 0);
} else {
c = mb_ptr2char_adv(&p);
}
c2 = -1;
if ((*p == '-') && (p[1] != NUL)) {
++p;
if (ascii_isdigit(*p)) {
c2 = getdigits_int((char_u **)&p, true, 0);
} else {
c2 = mb_ptr2char_adv(&p);
}
}
if ((c <= 0)
|| (c >= 256)
|| ((c2 < c) && (c2 != -1))
|| (c2 >= 256)
|| !((*p == NUL) || (*p == ','))) {
return FAIL;
}
if (c2 == -1) { 
if (c == '@') {
do_isalpha = true;
c = 1;
c2 = 255;
} else {
c2 = c;
}
}
while (c <= c2) {
if (!do_isalpha
|| mb_islower(c)
|| mb_isupper(c)) {
if (i == 0) {
if (tilde) {
g_chartab[c] &= (uint8_t)~CT_ID_CHAR;
} else {
g_chartab[c] |= CT_ID_CHAR;
}
} else if (i == 1) {
if (((c < ' ') || (c > '~'))) {
if (tilde) {
g_chartab[c] = (uint8_t)((g_chartab[c] & ~CT_CELL_MASK)
+ ((dy_flags & DY_UHEX) ? 4 : 2));
g_chartab[c] &= (uint8_t)~CT_PRINT_CHAR;
} else {
g_chartab[c] = (uint8_t)((g_chartab[c] & ~CT_CELL_MASK) + 1);
g_chartab[c] |= CT_PRINT_CHAR;
}
}
} else if (i == 2) {
if (tilde) {
g_chartab[c] &= (uint8_t)~CT_FNAME_CHAR;
} else {
g_chartab[c] |= CT_FNAME_CHAR;
}
} else { 
if (tilde) {
RESET_CHARTAB(buf, c);
} else {
SET_CHARTAB(buf, c);
}
}
}
++c;
}
c = *p;
p = skip_to_option_part(p);
if ((c == ',') && (*p == NUL)) {
return FAIL;
}
}
}
chartab_initialized = true;
return OK;
}
void trans_characters(char_u *buf, int bufsize)
{
int len; 
int room; 
char_u *trs; 
int trs_len; 
len = (int)STRLEN(buf);
room = bufsize - len;
while (*buf != 0) {
if ((trs_len = (*mb_ptr2len)(buf)) > 1) {
len -= trs_len;
} else {
trs = transchar_byte(*buf);
trs_len = (int)STRLEN(trs);
if (trs_len > 1) {
room -= trs_len - 1;
if (room <= 0) {
return;
}
memmove(buf + trs_len, buf + 1, (size_t)len);
}
memmove(buf, trs, (size_t)trs_len);
--len;
}
buf += trs_len;
}
}
size_t transstr_len(const char *const s)
FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_PURE
{
const char *p = s;
size_t len = 0;
while (*p) {
const size_t l = (size_t)utfc_ptr2len((const char_u *)p);
if (l > 1) {
int pcc[MAX_MCO + 1];
pcc[0] = utfc_ptr2char((const char_u *)p, &pcc[1]);
if (vim_isprintc(pcc[0])) {
len += l;
} else {
for (size_t i = 0; i < ARRAY_SIZE(pcc) && pcc[i]; i++) {
char hexbuf[9];
len += transchar_hex(hexbuf, pcc[i]);
}
}
p += l;
} else {
const int b2c_l = byte2cells((uint8_t)(*p++));
len += (size_t)(b2c_l > 0 ? b2c_l : 4);
}
}
return len;
}
size_t transstr_buf(const char *const s, char *const buf, const size_t len)
FUNC_ATTR_NONNULL_ALL
{
const char *p = s;
char *buf_p = buf;
char *const buf_e = buf_p + len - 1;
while (*p != NUL && buf_p < buf_e) {
const size_t l = (size_t)utfc_ptr2len((const char_u *)p);
if (l > 1) {
if (buf_p + l > buf_e) {
break; 
}
int pcc[MAX_MCO + 1];
pcc[0] = utfc_ptr2char((const char_u *)p, &pcc[1]);
if (vim_isprintc(pcc[0])) {
memmove(buf_p, p, l);
buf_p += l;
} else {
for (size_t i = 0; i < ARRAY_SIZE(pcc) && pcc[i]; i++) {
char hexbuf[9]; 
const size_t hexlen = transchar_hex(hexbuf, pcc[i]);
if (buf_p + hexlen > buf_e) {
break;
}
memmove(buf_p, hexbuf, hexlen);
buf_p += hexlen;
}
}
p += l;
} else {
const char *const tb = (const char *)transchar_byte((uint8_t)(*p++));
const size_t tb_len = strlen(tb);
if (buf_p + tb_len > buf_e) {
break; 
}
memmove(buf_p, tb, tb_len);
buf_p += tb_len;
}
}
*buf_p = NUL;
assert(buf_p <= buf_e);
return (size_t)(buf_p - buf);
}
char *transstr(const char *const s)
FUNC_ATTR_NONNULL_RET
{
const size_t len = transstr_len((const char *)s) + 1;
char *const buf = xmalloc(len);
transstr_buf(s, buf, len);
return buf;
}
char_u* str_foldcase(char_u *str, int orglen, char_u *buf, int buflen)
FUNC_ATTR_NONNULL_RET
{
garray_T ga;
int i;
int len = orglen;
#define GA_CHAR(i) ((char_u *)ga.ga_data)[i]
#define GA_PTR(i) ((char_u *)ga.ga_data + i)
#define STR_CHAR(i) (buf == NULL ? GA_CHAR(i) : buf[i])
#define STR_PTR(i) (buf == NULL ? GA_PTR(i) : buf + i)
if (buf == NULL) {
ga_init(&ga, 1, 10);
ga_grow(&ga, len + 1);
memmove(ga.ga_data, str, (size_t)len);
ga.ga_len = len;
} else {
if (len >= buflen) {
len = buflen - 1;
}
memmove(buf, str, (size_t)len);
}
if (buf == NULL) {
GA_CHAR(len) = NUL;
} else {
buf[len] = NUL;
}
i = 0;
while (STR_CHAR(i) != NUL) {
int c = utf_ptr2char(STR_PTR(i));
int olen = utf_ptr2len(STR_PTR(i));
int lc = mb_tolower(c);
if (((c < 0x80) || (olen > 1)) && (c != lc)) {
int nlen = utf_char2len(lc);
if (olen != nlen) {
if (nlen > olen) {
if (buf == NULL) {
ga_grow(&ga, nlen - olen + 1);
} else {
if (len + nlen - olen >= buflen) {
lc = c;
nlen = olen;
}
}
}
if (olen != nlen) {
if (buf == NULL) {
STRMOVE(GA_PTR(i) + nlen, GA_PTR(i) + olen);
ga.ga_len += nlen - olen;
} else {
STRMOVE(buf + i + nlen, buf + i + olen);
len += nlen - olen;
}
}
}
(void)utf_char2bytes(lc, STR_PTR(i));
}
i += (*mb_ptr2len)(STR_PTR(i));
}
if (buf == NULL) {
return (char_u *)ga.ga_data;
}
return buf;
}
static char_u transchar_buf[11];
char_u *transchar(int c)
{
int i = 0;
if (IS_SPECIAL(c)) {
transchar_buf[0] = '~';
transchar_buf[1] = '@';
i = 2;
c = K_SECOND(c);
}
if ((!chartab_initialized && (((c >= ' ') && (c <= '~'))))
|| ((c <= 0xFF) && vim_isprintc_strict(c))) {
transchar_buf[i] = (char_u)c;
transchar_buf[i + 1] = NUL;
} else if (c <= 0xFF) {
transchar_nonprint(transchar_buf + i, c);
} else {
transchar_hex((char *)transchar_buf + i, c);
}
return transchar_buf;
}
char_u *transchar_byte(const int c)
FUNC_ATTR_WARN_UNUSED_RESULT
{
if (c >= 0x80) {
transchar_nonprint(transchar_buf, c);
return transchar_buf;
}
return transchar(c);
}
void transchar_nonprint(char_u *buf, int c)
{
if (c == NL) {
c = NUL;
} else if ((c == CAR) && (get_fileformat(curbuf) == EOL_MAC)) {
c = NL;
}
assert(c <= 0xff);
if (dy_flags & DY_UHEX || c > 0x7f) {
transchar_hex((char *)buf, c);
} else {
buf[0] = '^';
buf[1] = (char_u)(c ^ 0x40);
buf[2] = NUL;
}
}
size_t transchar_hex(char *const buf, const int c)
FUNC_ATTR_NONNULL_ALL
{
size_t i = 0;
buf[i++] = '<';
if (c > 255) {
if (c > 255 * 256) {
buf[i++] = (char)nr2hex((unsigned)c >> 20);
buf[i++] = (char)nr2hex((unsigned)c >> 16);
}
buf[i++] = (char)nr2hex((unsigned)c >> 12);
buf[i++] = (char)nr2hex((unsigned)c >> 8);
}
buf[i++] = (char)(nr2hex((unsigned)c >> 4));
buf[i++] = (char)(nr2hex((unsigned)c));
buf[i++] = '>';
buf[i] = NUL;
return i;
}
static inline unsigned nr2hex(unsigned n)
FUNC_ATTR_CONST FUNC_ATTR_WARN_UNUSED_RESULT
{
if ((n & 0xf) <= 9) {
return (n & 0xf) + '0';
}
return (n & 0xf) - 10 + 'a';
}
int byte2cells(int b)
{
if (b >= 0x80) {
return 0;
}
return g_chartab[b] & CT_CELL_MASK;
}
int char2cells(int c)
{
if (IS_SPECIAL(c)) {
return char2cells(K_SECOND(c)) + 2;
}
if (c >= 0x80) {
return utf_char2cells(c);
}
return g_chartab[c & 0xff] & CT_CELL_MASK;
}
int ptr2cells(const char_u *p)
{
if (*p >= 0x80) {
return utf_ptr2cells(p);
}
return g_chartab[*p] & CT_CELL_MASK;
}
int vim_strsize(char_u *s)
{
return vim_strnsize(s, (int)MAXCOL);
}
int vim_strnsize(char_u *s, int len)
{
assert(s != NULL);
int size = 0;
while (*s != NUL && --len >= 0) {
int l = (*mb_ptr2len)(s);
size += ptr2cells(s);
s += l;
len -= l - 1;
}
return size;
}
#define RET_WIN_BUF_CHARTABSIZE(wp, buf, p, col) if (*(p) == TAB && (!(wp)->w_p_list || wp->w_p_lcs_chars.tab1)) { const int ts = (int)(buf)->b_p_ts; return (ts - (int)(col % ts)); } else { return ptr2cells(p); }
int chartabsize(char_u *p, colnr_T col)
{
RET_WIN_BUF_CHARTABSIZE(curwin, curbuf, p, col)
}
static int win_chartabsize(win_T *wp, char_u *p, colnr_T col)
{
RET_WIN_BUF_CHARTABSIZE(wp, wp->w_buffer, p, col)
}
int linetabsize(char_u *s)
{
return linetabsize_col(0, s);
}
int linetabsize_col(int startcol, char_u *s)
{
colnr_T col = startcol;
char_u *line = s; 
while (*s != NUL) {
col += lbr_chartabsize_adv(line, &s, col);
}
return (int)col;
}
unsigned int win_linetabsize(win_T *wp, char_u *line, colnr_T len)
{
colnr_T col = 0;
for (char_u *s = line;
*s != NUL && (len == MAXCOL || s < line + len);
MB_PTR_ADV(s)) {
col += win_lbr_chartabsize(wp, line, s, col, NULL);
}
return (unsigned int)col;
}
bool vim_isIDc(int c)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
return c > 0 && c < 0x100 && (g_chartab[c] & CT_ID_CHAR);
}
bool vim_iswordc(const int c)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
return vim_iswordc_buf(c, curbuf);
}
bool vim_iswordc_tab(const int c, const uint64_t *const chartab)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ALL
{
return (c >= 0x100
? (utf_class_tab(c, chartab) >= 2)
: (c > 0 && GET_CHARTAB_TAB(chartab, c) != 0));
}
bool vim_iswordc_buf(const int c, buf_T *const buf)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ARG(2)
{
return vim_iswordc_tab(c, buf->b_chartab);
}
bool vim_iswordp(const char_u *const p)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ALL
{
return vim_iswordp_buf(p, curbuf);
}
bool vim_iswordp_buf(const char_u *const p, buf_T *const buf)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ALL
{
int c = *p;
if (MB_BYTE2LEN(c) > 1) {
c = utf_ptr2char(p);
}
return vim_iswordc_buf(c, buf);
}
bool vim_isfilec(int c)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
return c >= 0x100 || (c > 0 && (g_chartab[c] & CT_FNAME_CHAR));
}
bool vim_isfilec_or_wc(int c)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
char_u buf[2];
buf[0] = (char_u)c;
buf[1] = NUL;
return vim_isfilec(c) || c == ']' || path_has_wildcard(buf);
}
bool vim_isprintc(int c)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
if (c >= 0x100) {
return utf_printable(c);
}
return c > 0 && (g_chartab[c] & CT_PRINT_CHAR);
}
bool vim_isprintc_strict(int c)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
if (c >= 0x100) {
return utf_printable(c);
}
return c > 0 && (g_chartab[c] & CT_PRINT_CHAR);
}
int lbr_chartabsize(char_u *line, unsigned char *s, colnr_T col)
{
if (!curwin->w_p_lbr && (*p_sbr == NUL) && !curwin->w_p_bri) {
if (curwin->w_p_wrap) {
return win_nolbr_chartabsize(curwin, s, col, NULL);
}
RET_WIN_BUF_CHARTABSIZE(curwin, curbuf, s, col)
}
return win_lbr_chartabsize(curwin, line == NULL ? s: line, s, col, NULL);
}
int lbr_chartabsize_adv(char_u *line, char_u **s, colnr_T col)
{
int retval;
retval = lbr_chartabsize(line, *s, col);
MB_PTR_ADV(*s);
return retval;
}
int win_lbr_chartabsize(win_T *wp, char_u *line, char_u *s, colnr_T col, int *headp)
{
colnr_T col2;
colnr_T col_adj = 0; 
colnr_T colmax;
int added;
int mb_added = 0;
int numberextra;
char_u *ps;
int n;
if (!wp->w_p_lbr && !wp->w_p_bri && (*p_sbr == NUL)) {
if (wp->w_p_wrap) {
return win_nolbr_chartabsize(wp, s, col, headp);
}
RET_WIN_BUF_CHARTABSIZE(wp, wp->w_buffer, s, col)
}
int size = win_chartabsize(wp, s, col);
int c = *s;
if (*s == TAB) {
col_adj = size - 1;
}
if (wp->w_p_lbr
&& vim_isbreak(c)
&& !vim_isbreak((int)s[1])
&& wp->w_p_wrap
&& (wp->w_width_inner != 0)) {
numberextra = win_col_off(wp);
col2 = col;
colmax = (colnr_T)(wp->w_width_inner - numberextra - col_adj);
if (col >= colmax) {
colmax += col_adj;
n = colmax + win_col_off2(wp);
if (n > 0) {
colmax += (((col - colmax) / n) + 1) * n - col_adj;
}
}
for (;;) {
ps = s;
MB_PTR_ADV(s);
c = *s;
if (!(c != NUL
&& (vim_isbreak(c) || col2 == col || !vim_isbreak((int)(*ps))))) {
break;
}
col2 += win_chartabsize(wp, s, col2);
if (col2 >= colmax) { 
size = colmax - col + col_adj;
break;
}
}
} else if ((size == 2)
&& (MB_BYTE2LEN(*s) > 1)
&& wp->w_p_wrap
&& in_win_border(wp, col)) {
++size;
mb_added = 1;
}
added = 0;
if ((*p_sbr != NUL || wp->w_p_bri) && wp->w_p_wrap && (col != 0)) {
colnr_T sbrlen = 0;
int numberwidth = win_col_off(wp);
numberextra = numberwidth;
col += numberextra + mb_added;
if (col >= (colnr_T)wp->w_width_inner) {
col -= wp->w_width_inner;
numberextra = wp->w_width_inner - (numberextra - win_col_off2(wp));
if (col >= numberextra && numberextra > 0) {
col %= numberextra;
}
if (*p_sbr != NUL) {
sbrlen = (colnr_T)MB_CHARLEN(p_sbr);
if (col >= sbrlen) {
col -= sbrlen;
}
}
if (col >= numberextra && numberextra > 0) {
col %= numberextra;
} else if (col > 0 && numberextra > 0) {
col += numberwidth - win_col_off2(wp);
}
numberwidth -= win_col_off2(wp);
}
if (col == 0 || (col + size + sbrlen > (colnr_T)wp->w_width_inner)) {
if (*p_sbr != NUL) {
if (size + sbrlen + numberwidth > (colnr_T)wp->w_width_inner) {
int width = (colnr_T)wp->w_width_inner - sbrlen - numberwidth;
int prev_width = col ? ((colnr_T)wp->w_width_inner - (sbrlen + col))
: 0;
if (width <= 0) {
width = 1;
}
added += ((size - prev_width) / width) * vim_strsize(p_sbr);
if ((size - prev_width) % width) {
added += vim_strsize(p_sbr);
}
} else {
added += vim_strsize(p_sbr);
}
}
if (wp->w_p_bri)
added += get_breakindent_win(wp, line);
size += added;
if (col != 0) {
added = 0;
}
}
}
if (headp != NULL) {
*headp = added + mb_added;
}
return size;
}
static int win_nolbr_chartabsize(win_T *wp, char_u *s, colnr_T col, int *headp)
{
int n;
if ((*s == TAB) && (!wp->w_p_list || wp->w_p_lcs_chars.tab1)) {
n = (int)wp->w_buffer->b_p_ts;
return n - (col % n);
}
n = ptr2cells(s);
if ((n == 2) && (MB_BYTE2LEN(*s) > 1) && in_win_border(wp, col)) {
if (headp != NULL) {
*headp = 1;
}
return 3;
}
return n;
}
bool in_win_border(win_T *wp, colnr_T vcol)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ARG(1)
{
int width1; 
int width2; 
if (wp->w_width_inner == 0) {
return false;
}
width1 = wp->w_width_inner - win_col_off(wp);
if ((int)vcol < width1 - 1) {
return false;
}
if ((int)vcol == width1 - 1) {
return true;
}
width2 = width1 + win_col_off2(wp);
if (width2 <= 0) {
return false;
}
return (vcol - width1) % width2 == width2 - 1;
}
void getvcol(win_T *wp, pos_T *pos, colnr_T *start, colnr_T *cursor,
colnr_T *end)
{
colnr_T vcol;
char_u *ptr; 
char_u *posptr; 
char_u *line; 
int incr;
int head;
int ts = (int)wp->w_buffer->b_p_ts;
int c;
vcol = 0;
line = ptr = ml_get_buf(wp->w_buffer, pos->lnum, false);
if (pos->col == MAXCOL) {
posptr = NULL;
} else {
if (*ptr == NUL) {
pos->col = 0;
}
posptr = ptr + pos->col;
posptr -= utf_head_off(line, posptr);
}
if ((!wp->w_p_list || (wp->w_p_lcs_chars.tab1 != NUL))
&& !wp->w_p_lbr
&& (*p_sbr == NUL)
&& !wp->w_p_bri ) {
for (;;) {
head = 0;
c = *ptr;
if (c == NUL) {
incr = 1;
break;
}
if (c == TAB) {
incr = ts - (vcol % ts);
} else {
if (c >= 0x80) {
incr = utf_ptr2cells(ptr);
} else {
incr = g_chartab[c] & CT_CELL_MASK;
}
if ((incr == 2)
&& wp->w_p_wrap
&& (MB_BYTE2LEN(*ptr) > 1)
&& in_win_border(wp, vcol)) {
incr++;
head = 1;
}
}
if ((posptr != NULL) && (ptr >= posptr)) {
break;
}
vcol += incr;
MB_PTR_ADV(ptr);
}
} else {
for (;;) {
head = 0;
incr = win_lbr_chartabsize(wp, line, ptr, vcol, &head);
if (*ptr == NUL) {
incr = 1;
break;
}
if ((posptr != NULL) && (ptr >= posptr)) {
break;
}
vcol += incr;
MB_PTR_ADV(ptr);
}
}
if (start != NULL) {
*start = vcol + head;
}
if (end != NULL) {
*end = vcol + incr - 1;
}
if (cursor != NULL) {
if ((*ptr == TAB)
&& (State & NORMAL)
&& !wp->w_p_list
&& !virtual_active()
&& !(VIsual_active && ((*p_sel == 'e') || ltoreq(*pos, VIsual)))) {
*cursor = vcol + incr - 1;
} else {
*cursor = vcol + head;
}
}
}
colnr_T getvcol_nolist(pos_T *posp)
{
int list_save = curwin->w_p_list;
colnr_T vcol;
curwin->w_p_list = false;
if (posp->coladd) {
getvvcol(curwin, posp, NULL, &vcol, NULL);
} else {
getvcol(curwin, posp, NULL, &vcol, NULL);
}
curwin->w_p_list = list_save;
return vcol;
}
void getvvcol(win_T *wp, pos_T *pos, colnr_T *start, colnr_T *cursor,
colnr_T *end)
{
colnr_T col;
colnr_T coladd;
colnr_T endadd;
char_u *ptr;
if (virtual_active()) {
getvcol(wp, pos, &col, NULL, NULL);
coladd = pos->coladd;
endadd = 0;
ptr = ml_get_buf(wp->w_buffer, pos->lnum, false);
if (pos->col < (colnr_T)STRLEN(ptr)) {
int c = utf_ptr2char(ptr + pos->col);
if ((c != TAB) && vim_isprintc(c)) {
endadd = (colnr_T)(char2cells(c) - 1);
if (coladd > endadd) {
endadd = 0;
} else {
coladd = 0;
}
}
}
col += coladd;
if (start != NULL) {
*start = col;
}
if (cursor != NULL) {
*cursor = col;
}
if (end != NULL) {
*end = col + endadd;
}
} else {
getvcol(wp, pos, start, cursor, end);
}
}
void getvcols(win_T *wp, pos_T *pos1, pos_T *pos2, colnr_T *left,
colnr_T *right)
{
colnr_T from1;
colnr_T from2;
colnr_T to1;
colnr_T to2;
if (lt(*pos1, *pos2)) {
getvvcol(wp, pos1, &from1, NULL, &to1);
getvvcol(wp, pos2, &from2, NULL, &to2);
} else {
getvvcol(wp, pos2, &from1, NULL, &to1);
getvvcol(wp, pos1, &from2, NULL, &to2);
}
if (from2 < from1) {
*left = from2;
} else {
*left = from1;
}
if (to2 > to1) {
if ((*p_sel == 'e') && (from2 - 1 >= to1)) {
*right = from2 - 1;
} else {
*right = to2;
}
} else {
*right = to1;
}
}
char_u *skipwhite(const char_u *q)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ALL
FUNC_ATTR_NONNULL_RET
{
const char_u *p = q;
while (ascii_iswhite(*p)) {
p++;
}
return (char_u *)p;
}
intptr_t getwhitecols_curline(void)
{
return getwhitecols(get_cursor_line_ptr());
}
intptr_t getwhitecols(const char_u *p)
{
return skipwhite(p) - p;
}
char_u *skipdigits(const char_u *q)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ALL
FUNC_ATTR_NONNULL_RET
{
const char_u *p = q;
while (ascii_isdigit(*p)) {
p++;
}
return (char_u *)p;
}
const char* skipbin(const char *q)
FUNC_ATTR_PURE
FUNC_ATTR_NONNULL_ALL
FUNC_ATTR_NONNULL_RET
{
const char *p = q;
while (ascii_isbdigit(*p)) {
p++;
}
return p;
}
char_u* skiphex(char_u *q)
{
char_u *p = q;
while (ascii_isxdigit(*p)) {
p++;
}
return p;
}
char_u* skiptodigit(char_u *q)
{
char_u *p = q;
while (*p != NUL && !ascii_isdigit(*p)) {
p++;
}
return p;
}
const char* skiptobin(const char *q)
FUNC_ATTR_PURE
FUNC_ATTR_NONNULL_ALL
FUNC_ATTR_NONNULL_RET
{
const char *p = q;
while (*p != NUL && !ascii_isbdigit(*p)) {
p++;
}
return p;
}
char_u* skiptohex(char_u *q)
{
char_u *p = q;
while (*p != NUL && !ascii_isxdigit(*p)) {
p++;
}
return p;
}
char_u *skiptowhite(const char_u *p)
FUNC_ATTR_NONNULL_ALL
{
while (*p != ' ' && *p != '\t' && *p != NUL) {
p++;
}
return (char_u *)p;
}
char_u* skiptowhite_esc(char_u *p) {
while (*p != ' ' && *p != '\t' && *p != NUL) {
if (((*p == '\\') || (*p == Ctrl_V)) && (*(p + 1) != NUL)) {
++p;
}
++p;
}
return p;
}
bool try_getdigits(char_u **pp, intmax_t *nr)
{
errno = 0;
*nr = strtoimax((char *)(*pp), (char **)pp, 10);
if (errno == ERANGE && (*nr == INTMAX_MIN || *nr == INTMAX_MAX)) {
return false;
}
return true;
}
intmax_t getdigits(char_u **pp, bool strict, intmax_t def)
{
intmax_t number;
int ok = try_getdigits(pp, &number);
if (strict && !ok) {
abort();
}
return ok ? number : def;
}
int getdigits_int(char_u **pp, bool strict, int def)
{
intmax_t number = getdigits(pp, strict, def);
#if SIZEOF_INTMAX_T > SIZEOF_INT
if (strict) {
assert(number >= INT_MIN && number <= INT_MAX);
} else if (!(number >= INT_MIN && number <= INT_MAX)) {
return def;
}
#endif
return (int)number;
}
long getdigits_long(char_u **pp, bool strict, long def)
{
intmax_t number = getdigits(pp, strict, def);
#if SIZEOF_INTMAX_T > SIZEOF_LONG
if (strict) {
assert(number >= LONG_MIN && number <= LONG_MAX);
} else if (!(number >= LONG_MIN && number <= LONG_MAX)) {
return def;
}
#endif
return (long)number;
}
bool vim_isblankline(char_u *lbuf)
{
char_u *p = skipwhite(lbuf);
return *p == NUL || *p == '\r' || *p == '\n';
}
void vim_str2nr(const char_u *const start, int *const prep, int *const len,
const int what, varnumber_T *const nptr,
uvarnumber_T *const unptr, const int maxlen)
FUNC_ATTR_NONNULL_ARG(1)
{
const char *ptr = (const char *)start;
#define STRING_ENDED(ptr) (!(maxlen == 0 || (int)((ptr) - (const char *)start) < maxlen))
int pre = 0; 
const bool negative = (ptr[0] == '-');
uvarnumber_T un = 0;
if (negative) {
ptr++;
}
if (what & STR2NR_FORCE) {
switch ((unsigned)what & (~(unsigned)STR2NR_FORCE)) {
case STR2NR_HEX: {
if (!STRING_ENDED(ptr + 2)
&& ptr[0] == '0'
&& (ptr[1] == 'x' || ptr[1] == 'X')
&& ascii_isxdigit(ptr[2])) {
ptr += 2;
}
goto vim_str2nr_hex;
}
case STR2NR_BIN: {
if (!STRING_ENDED(ptr + 2)
&& ptr[0] == '0'
&& (ptr[1] == 'b' || ptr[1] == 'B')
&& ascii_isbdigit(ptr[2])) {
ptr += 2;
}
goto vim_str2nr_bin;
}
case STR2NR_OCT: {
goto vim_str2nr_oct;
}
case 0: {
goto vim_str2nr_dec;
}
default: {
assert(false);
}
}
} else if ((what & (STR2NR_HEX|STR2NR_OCT|STR2NR_BIN))
&& !STRING_ENDED(ptr + 1)
&& ptr[0] == '0' && ptr[1] != '8' && ptr[1] != '9') {
pre = ptr[1];
if ((what & STR2NR_HEX)
&& !STRING_ENDED(ptr + 2)
&& (pre == 'X' || pre == 'x')
&& ascii_isxdigit(ptr[2])) {
ptr += 2;
goto vim_str2nr_hex;
}
if ((what & STR2NR_BIN)
&& !STRING_ENDED(ptr + 2)
&& (pre == 'B' || pre == 'b')
&& ascii_isbdigit(ptr[2])) {
ptr += 2;
goto vim_str2nr_bin;
}
pre = 0;
if (!(what & STR2NR_OCT)
|| !('0' <= ptr[1] && ptr[1] <= '7')) {
goto vim_str2nr_dec;
}
for (int i = 2; !STRING_ENDED(ptr + i) && ascii_isdigit(ptr[i]); i++) {
if (ptr[i] > '7') {
goto vim_str2nr_dec;
}
}
pre = '0';
goto vim_str2nr_oct;
} else {
goto vim_str2nr_dec;
}
assert(false); 
#define PARSE_NUMBER(base, cond, conv) do { while (!STRING_ENDED(ptr) && (cond)) { const uvarnumber_T digit = (uvarnumber_T)(conv); if (un < UVARNUMBER_MAX / base || (un == UVARNUMBER_MAX / base && (base != 10 || digit <= UVARNUMBER_MAX % 10))) { un = base * un + digit; } else { un = UVARNUMBER_MAX; } ptr++; } } while (0)
vim_str2nr_bin:
PARSE_NUMBER(2, (*ptr == '0' || *ptr == '1'), (*ptr - '0'));
goto vim_str2nr_proceed;
vim_str2nr_oct:
PARSE_NUMBER(8, ('0' <= *ptr && *ptr <= '7'), (*ptr - '0'));
goto vim_str2nr_proceed;
vim_str2nr_dec:
PARSE_NUMBER(10, (ascii_isdigit(*ptr)), (*ptr - '0'));
goto vim_str2nr_proceed;
vim_str2nr_hex:
PARSE_NUMBER(16, (ascii_isxdigit(*ptr)), (hex2nr(*ptr)));
goto vim_str2nr_proceed;
#undef PARSE_NUMBER
vim_str2nr_proceed:
if (prep != NULL) {
*prep = pre;
}
if (len != NULL) {
*len = (int)(ptr - (const char *)start);
}
if (nptr != NULL) {
if (negative) { 
if (un > VARNUMBER_MAX) {
*nptr = VARNUMBER_MIN;
} else {
*nptr = -(varnumber_T)un;
}
} else {
if (un > VARNUMBER_MAX) {
un = VARNUMBER_MAX;
}
*nptr = (varnumber_T)un;
}
}
if (unptr != NULL) {
*unptr = un;
}
#undef STRING_ENDED
}
int hex2nr(int c)
{
if ((c >= 'a') && (c <= 'f')) {
return c - 'a' + 10;
}
if ((c >= 'A') && (c <= 'F')) {
return c - 'A' + 10;
}
return c - '0';
}
bool rem_backslash(const char_u *str)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ALL
{
#if defined(BACKSLASH_IN_FILENAME)
return str[0] == '\\'
&& str[1] < 0x80
&& (str[1] == ' '
|| (str[1] != NUL
&& str[1] != '*'
&& str[1] != '?'
&& !vim_isfilec(str[1])));
#else 
return str[0] == '\\' && str[1] != NUL;
#endif 
}
void backslash_halve(char_u *p)
{
for (; *p; ++p) {
if (rem_backslash(p)) {
STRMOVE(p, p + 1);
}
}
}
char_u *backslash_halve_save(const char_u *p)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_NONNULL_RET
{
char_u *res = vim_strsave(p);
backslash_halve(res);
return res;
}
