#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include "nvim/vim.h"
#include "nvim/ascii.h"
#include "nvim/quickfix.h"
#include "nvim/buffer.h"
#include "nvim/charset.h"
#include "nvim/cursor.h"
#include "nvim/edit.h"
#include "nvim/eval.h"
#include "nvim/ex_cmds.h"
#include "nvim/ex_cmds2.h"
#include "nvim/ex_docmd.h"
#include "nvim/ex_eval.h"
#include "nvim/ex_getln.h"
#include "nvim/fileio.h"
#include "nvim/fold.h"
#include "nvim/mark.h"
#include "nvim/mbyte.h"
#include "nvim/memline.h"
#include "nvim/message.h"
#include "nvim/misc1.h"
#include "nvim/memory.h"
#include "nvim/move.h"
#include "nvim/normal.h"
#include "nvim/option.h"
#include "nvim/os_unix.h"
#include "nvim/path.h"
#include "nvim/regexp.h"
#include "nvim/screen.h"
#include "nvim/search.h"
#include "nvim/strings.h"
#include "nvim/syntax.h"
#include "nvim/ui.h"
#include "nvim/window.h"
#include "nvim/os/os.h"
#include "nvim/os/input.h"
#include "nvim/api/private/helpers.h"
struct dir_stack_T {
struct dir_stack_T *next;
char_u *dirname;
};
typedef struct qfline_S qfline_T;
struct qfline_S {
qfline_T *qf_next; 
qfline_T *qf_prev; 
linenr_T qf_lnum; 
int qf_fnum; 
int qf_col; 
int qf_nr; 
char_u *qf_module; 
char_u *qf_pattern; 
char_u *qf_text; 
char_u qf_viscol; 
char_u qf_cleared; 
char_u qf_type; 
char_u qf_valid; 
};
#define LISTCOUNT 10
#define INVALID_QFIDX (-1)
typedef enum
{
QFLT_QUICKFIX, 
QFLT_LOCATION, 
QFLT_INTERNAL 
} qfltype_T;
typedef struct qf_list_S {
unsigned qf_id; 
qfltype_T qfl_type;
qfline_T *qf_start; 
qfline_T *qf_last; 
qfline_T *qf_ptr; 
int qf_count; 
int qf_index; 
int qf_nonevalid; 
char_u *qf_title; 
typval_T *qf_ctx; 
struct dir_stack_T *qf_dir_stack;
char_u *qf_directory;
struct dir_stack_T *qf_file_stack;
char_u *qf_currfile;
bool qf_multiline;
bool qf_multiignore;
bool qf_multiscan;
long qf_changedtick;
} qf_list_T;
struct qf_info_S {
int qf_refcount;
int qf_listcount; 
int qf_curlist; 
qf_list_T qf_lists[LISTCOUNT];
qfltype_T qfl_type; 
};
static qf_info_T ql_info; 
static unsigned last_qf_id = 0; 
#define FMT_PATTERNS 11 
typedef struct efm_S efm_T;
struct efm_S {
regprog_T *prog; 
efm_T *next; 
char_u addr[FMT_PATTERNS]; 
char_u prefix; 
char_u flags; 
int conthere; 
};
typedef struct qf_delq_S {
struct qf_delq_S *next;
qf_info_T *qi;
} qf_delq_T;
enum {
QF_FAIL = 0,
QF_OK = 1,
QF_END_OF_INPUT = 2,
QF_NOMEM = 3,
QF_IGNORE_LINE = 4,
QF_MULTISCAN = 5,
};
typedef struct {
char_u *linebuf;
size_t linelen;
char_u *growbuf;
size_t growbufsiz;
FILE *fd;
typval_T *tv;
char_u *p_str;
list_T *p_list;
listitem_T *p_li;
buf_T *buf;
linenr_T buflnum;
linenr_T lnumlast;
vimconv_T vc;
} qfstate_T;
typedef struct {
char_u *namebuf;
char_u *module;
char_u *errmsg;
size_t errmsglen;
long lnum;
int col;
bool use_viscol;
char_u *pattern;
int enr;
char_u type;
bool valid;
} qffields_T;
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "quickfix.c.generated.h"
#endif
static char_u *e_no_more_items = (char_u *)N_("E553: No more items");
#define IS_QF_WINDOW(wp) (bt_quickfix(wp->w_buffer) && wp->w_llist_ref == NULL)
#define IS_LL_WINDOW(wp) (bt_quickfix(wp->w_buffer) && wp->w_llist_ref != NULL)
#define IS_QF_STACK(qi) (qi->qfl_type == QFLT_QUICKFIX)
#define IS_LL_STACK(qi) (qi->qfl_type == QFLT_LOCATION)
#define IS_QF_LIST(qfl) (qfl->qfl_type == QFLT_QUICKFIX)
#define IS_LL_LIST(qfl) (qfl->qfl_type == QFLT_LOCATION)
#define GET_LOC_LIST(wp) (IS_LL_WINDOW(wp) ? wp->w_llist_ref : wp->w_llist)
#define FOR_ALL_QFL_ITEMS(qfl, qfp, i) for (i = 1, qfp = qfl->qf_start; !got_int && i <= qfl->qf_count && qfp != NULL; i++, qfp = qfp->qf_next)
static char_u *qf_last_bufname = NULL;
static bufref_T qf_last_bufref = { NULL, 0, 0 };
static char *e_loc_list_changed = N_("E926: Current location list was changed");
static int quickfix_busy = 0;
static qf_delq_T *qf_delq_head = NULL;
static int qf_init_process_nextline(qf_list_T *qfl,
efm_T *fmt_first,
qfstate_T *state,
qffields_T *fields)
{
int status;
status = qf_get_nextline(state);
if (status != QF_OK) {
return status;
}
status = qf_parse_line(qfl, state->linebuf, state->linelen,
fmt_first, fields);
if (status != QF_OK) {
return status;
}
return qf_add_entry(qfl,
qfl->qf_directory,
(*fields->namebuf || qfl->qf_directory != NULL)
? fields->namebuf
: ((qfl->qf_currfile != NULL && fields->valid)
? qfl->qf_currfile : (char_u *)NULL),
fields->module,
0,
fields->errmsg,
fields->lnum,
fields->col,
fields->use_viscol,
fields->pattern,
fields->enr,
fields->type,
fields->valid);
}
int qf_init(win_T *wp, const char_u *restrict efile,
char_u *restrict errorformat, int newlist,
const char_u *restrict qf_title, char_u *restrict enc)
{
qf_info_T *qi = &ql_info;
if (wp != NULL) {
qi = ll_get_or_alloc_list(wp);
}
return qf_init_ext(qi, qi->qf_curlist, efile, curbuf, NULL, errorformat,
newlist, (linenr_T)0, (linenr_T)0, qf_title, enc);
}
static const size_t LINE_MAXLEN = 4096;
static struct fmtpattern
{
char_u convchar;
char *pattern;
} fmt_pat[FMT_PATTERNS] =
{
{ 'f', ".\\+" }, 
{ 'n', "\\d\\+" },
{ 'l', "\\d\\+" },
{ 'c', "\\d\\+" },
{ 't', "." },
{ 'm', ".\\+" },
{ 'r', ".*" },
{ 'p', "[- .]*" }, 
{ 'v', "\\d\\+" },
{ 's', ".\\+" },
{ 'o', ".\\+" }
};
static char_u * efmpat_to_regpat(
const char_u *efmpat,
char_u *regpat,
efm_T *efminfo,
int idx,
int round,
char_u *errmsg,
size_t errmsglen)
FUNC_ATTR_NONNULL_ALL
{
if (efminfo->addr[idx]) {
snprintf((char *)errmsg, errmsglen,
_("E372: Too many %%%c in format string"), *efmpat);
EMSG(errmsg);
return NULL;
}
if ((idx && idx < 6
&& vim_strchr((char_u *)"DXOPQ", efminfo->prefix) != NULL)
|| (idx == 6
&& vim_strchr((char_u *)"OPQ", efminfo->prefix) == NULL)) {
snprintf((char *)errmsg, errmsglen,
_("E373: Unexpected %%%c in format string"), *efmpat);
EMSG(errmsg);
return NULL;
}
efminfo->addr[idx] = (char_u)++round;
*regpat++ = '\\';
*regpat++ = '(';
#if defined(BACKSLASH_IN_FILENAME)
if (*efmpat == 'f') {
STRCPY(regpat, "\\%(\\a:\\)\\=");
regpat += 10;
}
#endif
if (*efmpat == 'f' && efmpat[1] != NUL) {
if (efmpat[1] != '\\' && efmpat[1] != '%') {
STRCPY(regpat, ".\\{-1,}");
regpat += 7;
} else {
STRCPY(regpat, "\\f\\+");
regpat += 4;
}
} else {
char_u *srcptr = (char_u *)fmt_pat[idx].pattern;
while ((*regpat = *srcptr++) != NUL) {
regpat++;
}
}
*regpat++ = '\\';
*regpat++ = ')';
return regpat;
}
static char_u * scanf_fmt_to_regpat(
const char_u **pefmp,
const char_u *efm,
int len,
char_u *regpat,
char_u *errmsg,
size_t errmsglen)
FUNC_ATTR_NONNULL_ALL
{
const char_u *efmp = *pefmp;
if (*efmp == '[' || *efmp == '\\') {
if ((*regpat++ = *efmp) == '[') { 
if (efmp[1] == '^') {
*regpat++ = *++efmp;
}
if (efmp < efm + len) {
*regpat++ = *++efmp; 
while (efmp < efm + len && (*regpat++ = *++efmp) != ']') {
}
if (efmp == efm + len) {
EMSG(_("E374: Missing ] in format string"));
return NULL;
}
}
} else if (efmp < efm + len) { 
*regpat++ = *++efmp;
}
*regpat++ = '\\';
*regpat++ = '+';
} else {
snprintf((char *)errmsg, errmsglen,
_("E375: Unsupported %%%c in format string"), *efmp);
EMSG(errmsg);
return NULL;
}
*pefmp = efmp;
return regpat;
}
static const char_u *efm_analyze_prefix(const char_u *efmp, efm_T *efminfo,
char_u *errmsg, size_t errmsglen)
FUNC_ATTR_NONNULL_ALL
{
if (vim_strchr((char_u *)"+-", *efmp) != NULL) {
efminfo->flags = *efmp++;
}
if (vim_strchr((char_u *)"DXAEWICZGOPQ", *efmp) != NULL) {
efminfo->prefix = *efmp;
} else {
snprintf((char *)errmsg, errmsglen,
_("E376: Invalid %%%c in format string prefix"), *efmp);
EMSG(errmsg);
return NULL;
}
return efmp;
}
static int efm_to_regpat(const char_u *efm, int len, efm_T *fmt_ptr,
char_u *regpat, char_u *errmsg, size_t errmsglen)
FUNC_ATTR_NONNULL_ALL
{
char_u *ptr = regpat;
*ptr++ = '^';
int round = 0;
for (const char_u *efmp = efm; efmp < efm + len; efmp++) {
if (*efmp == '%') {
efmp++;
int idx;
for (idx = 0; idx < FMT_PATTERNS; idx++) {
if (fmt_pat[idx].convchar == *efmp) {
break;
}
}
if (idx < FMT_PATTERNS) {
ptr = efmpat_to_regpat(efmp, ptr, fmt_ptr, idx, round, errmsg,
errmsglen);
if (ptr == NULL) {
return FAIL;
}
round++;
} else if (*efmp == '*') {
efmp++;
ptr = scanf_fmt_to_regpat(&efmp, efm, len, ptr, errmsg, errmsglen);
if (ptr == NULL) {
return FAIL;
}
} else if (vim_strchr((char_u *)"%\\.^$~[", *efmp) != NULL) {
*ptr++ = *efmp; 
} else if (*efmp == '#') {
*ptr++ = '*';
} else if (*efmp == '>') {
fmt_ptr->conthere = true;
} else if (efmp == efm + 1) { 
efmp = efm_analyze_prefix(efmp, fmt_ptr, errmsg, errmsglen);
if (efmp == NULL) {
return FAIL;
}
} else {
snprintf((char *)errmsg, CMDBUFFSIZE + 1,
_("E377: Invalid %%%c in format string"), *efmp);
EMSG(errmsg);
return FAIL;
}
} else { 
if (*efmp == '\\' && efmp + 1 < efm + len) {
efmp++;
} else if (vim_strchr((char_u *)".*^$~[", *efmp) != NULL) {
*ptr++ = '\\'; 
}
if (*efmp) {
*ptr++ = *efmp;
}
}
}
*ptr++ = '$';
*ptr = NUL;
return OK;
}
static efm_T *fmt_start = NULL; 
static void free_efm_list(efm_T **efm_first)
{
for (efm_T *efm_ptr = *efm_first; efm_ptr != NULL; efm_ptr = *efm_first) {
*efm_first = efm_ptr->next;
vim_regfree(efm_ptr->prog);
xfree(efm_ptr);
}
fmt_start = NULL;
}
static size_t efm_regpat_bufsz(char_u *efm)
{
size_t sz;
sz = (FMT_PATTERNS * 3) + (STRLEN(efm) << 2);
for (int i = FMT_PATTERNS - 1; i >= 0; ) {
sz += STRLEN(fmt_pat[i--].pattern);
}
#if defined(BACKSLASH_IN_FILENAME)
sz += 12; 
#else
sz += 2; 
#endif
return sz;
}
static int efm_option_part_len(char_u *efm)
{
int len;
for (len = 0; efm[len] != NUL && efm[len] != ','; len++) {
if (efm[len] == '\\' && efm[len + 1] != NUL) {
len++;
}
}
return len;
}
static efm_T * parse_efm_option(char_u *efm)
{
efm_T *fmt_ptr = NULL;
efm_T *fmt_first = NULL;
efm_T *fmt_last = NULL;
int len;
size_t errmsglen = CMDBUFFSIZE + 1;
char_u *errmsg = xmalloc(errmsglen);
size_t sz = efm_regpat_bufsz(efm);
char_u *fmtstr = xmalloc(sz);
while (efm[0] != NUL) {
fmt_ptr = (efm_T *)xcalloc(1, sizeof(efm_T));
if (fmt_first == NULL) { 
fmt_first = fmt_ptr;
} else {
fmt_last->next = fmt_ptr;
}
fmt_last = fmt_ptr;
len = efm_option_part_len(efm);
if (efm_to_regpat(efm, len, fmt_ptr, fmtstr, errmsg, errmsglen) == FAIL) {
goto parse_efm_error;
}
if ((fmt_ptr->prog = vim_regcomp(fmtstr, RE_MAGIC + RE_STRING)) == NULL) {
goto parse_efm_error;
}
efm = skip_to_option_part(efm + len); 
}
if (fmt_first == NULL) { 
EMSG(_("E378: 'errorformat' contains no pattern"));
}
goto parse_efm_end;
parse_efm_error:
free_efm_list(&fmt_first);
parse_efm_end:
xfree(fmtstr);
xfree(errmsg);
return fmt_first;
}
static char_u *qf_grow_linebuf(qfstate_T *state, size_t newsz)
{
state->linelen = newsz > LINE_MAXLEN ? LINE_MAXLEN - 1 : newsz;
if (state->growbuf == NULL) {
state->growbuf = xmalloc(state->linelen + 1);
state->growbufsiz = state->linelen;
} else if (state->linelen > state->growbufsiz) {
state->growbuf = xrealloc(state->growbuf, state->linelen + 1);
state->growbufsiz = state->linelen;
}
return state->growbuf;
}
static int qf_get_next_str_line(qfstate_T *state)
{
char_u *p_str = state->p_str;
char_u *p;
size_t len;
if (*p_str == NUL) { 
return QF_END_OF_INPUT;
}
p = vim_strchr(p_str, '\n');
if (p != NULL) {
len = (size_t)(p - p_str) + 1;
} else {
len = STRLEN(p_str);
}
if (len > IOSIZE - 2) {
state->linebuf = qf_grow_linebuf(state, len);
} else {
state->linebuf = IObuff;
state->linelen = len;
}
STRLCPY(state->linebuf, p_str, state->linelen + 1);
p_str += len;
state->p_str = p_str;
return QF_OK;
}
static int qf_get_next_list_line(qfstate_T *state)
{
listitem_T *p_li = state->p_li;
size_t len;
while (p_li != NULL
&& (TV_LIST_ITEM_TV(p_li)->v_type != VAR_STRING
|| TV_LIST_ITEM_TV(p_li)->vval.v_string == NULL)) {
p_li = TV_LIST_ITEM_NEXT(state->p_list, p_li); 
}
if (p_li == NULL) { 
state->p_li = NULL;
return QF_END_OF_INPUT;
}
len = STRLEN(TV_LIST_ITEM_TV(p_li)->vval.v_string);
if (len > IOSIZE - 2) {
state->linebuf = qf_grow_linebuf(state, len);
} else {
state->linebuf = IObuff;
state->linelen = len;
}
STRLCPY(state->linebuf, TV_LIST_ITEM_TV(p_li)->vval.v_string,
state->linelen + 1);
state->p_li = TV_LIST_ITEM_NEXT(state->p_list, p_li);
return QF_OK;
}
static int qf_get_next_buf_line(qfstate_T *state)
{
char_u *p_buf = NULL;
size_t len;
if (state->buflnum > state->lnumlast) {
return QF_END_OF_INPUT;
}
p_buf = ml_get_buf(state->buf, state->buflnum, false);
state->buflnum += 1;
len = STRLEN(p_buf);
if (len > IOSIZE - 2) {
state->linebuf = qf_grow_linebuf(state, len);
} else {
state->linebuf = IObuff;
state->linelen = len;
}
STRLCPY(state->linebuf, p_buf, state->linelen + 1);
return QF_OK;
}
static int qf_get_next_file_line(qfstate_T *state)
{
size_t growbuflen;
retry:
errno = 0;
if (fgets((char *)IObuff, IOSIZE, state->fd) == NULL) {
if (errno == EINTR) {
goto retry;
}
return QF_END_OF_INPUT;
}
bool discard = false;
state->linelen = STRLEN(IObuff);
if (state->linelen == IOSIZE - 1
&& !(IObuff[state->linelen - 1] == '\n')) {
if (state->growbuf == NULL) {
state->growbufsiz = 2 * (IOSIZE - 1);
state->growbuf = xmalloc(state->growbufsiz);
}
memcpy(state->growbuf, IObuff, IOSIZE - 1);
growbuflen = state->linelen;
for (;;) {
errno = 0;
if (fgets((char *)state->growbuf + growbuflen,
(int)(state->growbufsiz - growbuflen), state->fd) == NULL) {
if (errno == EINTR) {
continue;
}
break;
}
state->linelen = STRLEN(state->growbuf + growbuflen);
growbuflen += state->linelen;
if (state->growbuf[growbuflen - 1] == '\n') {
break;
}
if (state->growbufsiz == LINE_MAXLEN) {
discard = true;
break;
}
state->growbufsiz = (2 * state->growbufsiz < LINE_MAXLEN)
? 2 * state->growbufsiz : LINE_MAXLEN;
state->growbuf = xrealloc(state->growbuf, state->growbufsiz);
}
while (discard) {
errno = 0;
if (fgets((char *)IObuff, IOSIZE, state->fd) == NULL) {
if (errno == EINTR) {
continue;
}
break;
}
if (STRLEN(IObuff) < IOSIZE - 1 || IObuff[IOSIZE - 1] == '\n') {
break;
}
}
state->linebuf = state->growbuf;
state->linelen = growbuflen;
} else {
state->linebuf = IObuff;
}
if (state->vc.vc_type != CONV_NONE && has_non_ascii(state->linebuf)) {
char_u *line = string_convert(&state->vc, state->linebuf, &state->linelen);
if (line != NULL) {
if (state->linelen < IOSIZE) {
STRLCPY(state->linebuf, line, state->linelen + 1);
xfree(line);
} else {
xfree(state->growbuf);
state->linebuf = state->growbuf = line;
state->growbufsiz = state->linelen < LINE_MAXLEN
? state->linelen : LINE_MAXLEN;
}
}
}
return QF_OK;
}
static int qf_get_nextline(qfstate_T *state)
{
int status = QF_FAIL;
if (state->fd == NULL) {
if (state->tv != NULL) {
if (state->tv->v_type == VAR_STRING) {
status = qf_get_next_str_line(state);
} else if (state->tv->v_type == VAR_LIST) {
status = qf_get_next_list_line(state);
}
} else {
status = qf_get_next_buf_line(state);
}
} else {
status = qf_get_next_file_line(state);
}
if (status != QF_OK) {
return status;
}
if (state->linelen > 0 && state->linebuf[state->linelen - 1] == '\n') {
state->linebuf[state->linelen - 1] = NUL;
#if defined(USE_CRNL)
if (state->linelen > 1 && state->linebuf[state->linelen - 2] == '\r') {
state->linebuf[state->linelen - 2] = NUL;
}
#endif
}
remove_bom(state->linebuf);
return QF_OK;
}
static bool qf_stack_empty(const qf_info_T *qi)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
return qi == NULL || qi->qf_listcount <= 0;
}
static bool qf_list_empty(qf_list_T *qfl)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
return qfl == NULL || qfl->qf_count <= 0;
}
static bool qf_list_has_valid_entries(qf_list_T *qfl)
{
return !qf_list_empty(qfl) && !qfl->qf_nonevalid;
}
static qf_list_T * qf_get_list(qf_info_T *qi, int idx)
{
return &qi->qf_lists[idx];
}
static int qf_parse_line(qf_list_T *qfl, char_u *linebuf,
size_t linelen, efm_T *fmt_first, qffields_T *fields)
{
efm_T *fmt_ptr;
int idx = 0;
char_u *tail = NULL;
int status;
restofline:
if (fmt_start == NULL) {
fmt_ptr = fmt_first;
} else {
fmt_ptr = fmt_start;
fmt_start = NULL;
}
fields->valid = true;
for (; fmt_ptr != NULL; fmt_ptr = fmt_ptr->next) {
idx = fmt_ptr->prefix;
status = qf_parse_get_fields(linebuf, linelen, fmt_ptr, fields,
qfl->qf_multiline, qfl->qf_multiscan,
&tail);
if (status == QF_NOMEM) {
return status;
}
if (status == QF_OK) {
break;
}
}
qfl->qf_multiscan = false;
if (fmt_ptr == NULL || idx == 'D' || idx == 'X') {
if (fmt_ptr != NULL) {
status = qf_parse_dir_pfx(idx, fields, qfl);
if (status != QF_OK) {
return status;
}
}
status = qf_parse_line_nomatch(linebuf, linelen, fields);
if (status != QF_OK) {
return status;
}
if (fmt_ptr == NULL) {
qfl->qf_multiline = qfl->qf_multiignore = false;
}
} else {
if (fmt_ptr->conthere) {
fmt_start = fmt_ptr;
}
if (vim_strchr((char_u *)"AEWI", idx) != NULL) {
qfl->qf_multiline = true; 
qfl->qf_multiignore = false; 
} else if (vim_strchr((char_u *)"CZ", idx) != NULL) {
status = qf_parse_multiline_pfx(idx, qfl, fields);
if (status != QF_OK) {
return status;
}
} else if (vim_strchr((char_u *)"OPQ", idx) != NULL) {
status = qf_parse_file_pfx(idx, fields, qfl, tail);
if (status == QF_MULTISCAN) {
goto restofline;
}
}
if (fmt_ptr->flags == '-') { 
if (qfl->qf_multiline) {
qfl->qf_multiignore = true;
}
return QF_IGNORE_LINE;
}
}
return QF_OK;
}
static void qf_alloc_fields(qffields_T *pfields)
FUNC_ATTR_NONNULL_ALL
{
pfields->namebuf = xmalloc(CMDBUFFSIZE + 1);
pfields->module = xmalloc(CMDBUFFSIZE + 1);
pfields->errmsglen = CMDBUFFSIZE + 1;
pfields->errmsg = xmalloc(pfields->errmsglen);
pfields->pattern = xmalloc(CMDBUFFSIZE + 1);
}
static void qf_free_fields(qffields_T *pfields)
FUNC_ATTR_NONNULL_ALL
{
xfree(pfields->namebuf);
xfree(pfields->module);
xfree(pfields->errmsg);
xfree(pfields->pattern);
}
static int qf_setup_state(
qfstate_T *pstate,
char_u *restrict enc,
const char_u *restrict efile,
typval_T *tv,
buf_T *buf,
linenr_T lnumfirst,
linenr_T lnumlast)
FUNC_ATTR_NONNULL_ARG(1)
{
pstate->vc.vc_type = CONV_NONE;
if (enc != NULL && *enc != NUL) {
convert_setup(&pstate->vc, enc, p_enc);
}
if (efile != NULL
&& (pstate->fd = os_fopen((const char *)efile, "r")) == NULL) {
EMSG2(_(e_openerrf), efile);
return FAIL;
}
if (tv != NULL) {
if (tv->v_type == VAR_STRING) {
pstate->p_str = tv->vval.v_string;
} else if (tv->v_type == VAR_LIST) {
pstate->p_li = tv_list_first(tv->vval.v_list);
}
pstate->tv = tv;
}
pstate->buf = buf;
pstate->buflnum = lnumfirst;
pstate->lnumlast = lnumlast;
return OK;
}
static void qf_cleanup_state(qfstate_T *pstate)
FUNC_ATTR_NONNULL_ALL
{
if (pstate->fd != NULL) {
fclose(pstate->fd);
}
xfree(pstate->growbuf);
if (pstate->vc.vc_type != CONV_NONE) {
convert_setup(&pstate->vc, NULL, NULL);
}
}
static int
qf_init_ext(
qf_info_T *qi,
int qf_idx,
const char_u *restrict efile,
buf_T *buf,
typval_T *tv,
char_u *restrict errorformat,
bool newlist, 
linenr_T lnumfirst, 
linenr_T lnumlast, 
const char_u *restrict qf_title,
char_u *restrict enc
)
FUNC_ATTR_NONNULL_ARG(1)
{
qfstate_T state = { 0 };
qffields_T fields = { 0 };
qfline_T *old_last = NULL;
bool adding = false;
static efm_T *fmt_first = NULL;
char_u *efm;
static char_u *last_efm = NULL;
int retval = -1; 
int status;
XFREE_CLEAR(qf_last_bufname);
qf_alloc_fields(&fields);
if (qf_setup_state(&state, enc, efile, tv, buf,
lnumfirst, lnumlast) == FAIL) {
goto qf_init_end;
}
if (newlist || qf_idx == qi->qf_listcount) {
qf_new_list(qi, qf_title);
qf_idx = qi->qf_curlist;
} else {
adding = true;
if (!qf_list_empty(qf_get_list(qi, qf_idx) )) {
old_last = qi->qf_lists[qf_idx].qf_last;
}
}
qf_list_T *qfl = qf_get_list(qi, qf_idx);
if (errorformat == p_efm && tv == NULL && buf && *buf->b_p_efm != NUL) {
efm = buf->b_p_efm;
} else {
efm = errorformat;
}
if (last_efm == NULL || (STRCMP(last_efm, efm) != 0)) {
XFREE_CLEAR(last_efm);
free_efm_list(&fmt_first);
fmt_first = parse_efm_option(efm);
if (fmt_first != NULL) {
last_efm = vim_strsave(efm);
}
}
if (fmt_first == NULL) { 
goto error2;
}
got_int = false;
while (!got_int) {
status = qf_init_process_nextline(qfl, fmt_first, &state, &fields);
if (status == QF_END_OF_INPUT) { 
break;
}
if (status == QF_FAIL) {
goto error2;
}
line_breakcheck();
}
if (state.fd == NULL || !ferror(state.fd)) {
if (qfl->qf_index == 0) {
qfl->qf_ptr = qfl->qf_start;
qfl->qf_index = 1;
qfl->qf_nonevalid = true;
} else {
qfl->qf_nonevalid = false;
if (qfl->qf_ptr == NULL) {
qfl->qf_ptr = qfl->qf_start;
}
}
retval = qfl->qf_count;
goto qf_init_end;
}
EMSG(_(e_readerrf));
error2:
if (!adding) {
qf_free(qfl);
qi->qf_listcount--;
if (qi->qf_curlist > 0) {
qi->qf_curlist--;
}
}
qf_init_end:
if (qf_idx == qi->qf_curlist) {
qf_update_buffer(qi, old_last);
}
qf_cleanup_state(&state);
qf_free_fields(&fields);
return retval;
}
static void qf_store_title(qf_list_T *qfl, const char_u *title)
FUNC_ATTR_NONNULL_ARG(1)
{
XFREE_CLEAR(qfl->qf_title);
if (title != NULL) {
size_t len = STRLEN(title) + 1;
char_u *p = xmallocz(len);
qfl->qf_title = p;
xstrlcpy((char *)p, (const char *)title, len + 1);
}
}
static char_u * qf_cmdtitle(char_u *cmd)
{
static char_u qftitle_str[IOSIZE];
snprintf((char *)qftitle_str, IOSIZE, ":%s", (char *)cmd);
return qftitle_str;
}
static qf_list_T * qf_get_curlist(qf_info_T *qi)
{
return qf_get_list(qi, qi->qf_curlist);
}
static void qf_new_list(qf_info_T *qi, const char_u *qf_title)
{
int i;
qf_list_T *qfl;
while (qi->qf_listcount > qi->qf_curlist + 1) {
qf_free(&qi->qf_lists[--qi->qf_listcount]);
}
if (qi->qf_listcount == LISTCOUNT) {
qf_free(&qi->qf_lists[0]);
for (i = 1; i < LISTCOUNT; i++) {
qi->qf_lists[i - 1] = qi->qf_lists[i];
}
qi->qf_curlist = LISTCOUNT - 1;
} else
qi->qf_curlist = qi->qf_listcount++;
qfl = qf_get_curlist(qi);
memset(qfl, 0, sizeof(qf_list_T));
qf_store_title(qfl, qf_title);
qfl->qfl_type = qi->qfl_type;
qfl->qf_id = ++last_qf_id;
}
static int qf_parse_fmt_f(regmatch_T *rmp,
int midx,
qffields_T *fields,
int prefix)
{
char_u c;
if (rmp->startp[midx] == NULL || rmp->endp[midx] == NULL) {
return QF_FAIL;
}
c = *rmp->endp[midx];
*rmp->endp[midx] = NUL;
expand_env(rmp->startp[midx], fields->namebuf, CMDBUFFSIZE);
*rmp->endp[midx] = c;
if (vim_strchr((char_u *)"OPQ", prefix) != NULL
&& !os_path_exists(fields->namebuf)) {
return QF_FAIL;
}
return QF_OK;
}
static int qf_parse_fmt_n(regmatch_T *rmp, int midx, qffields_T *fields)
{
if (rmp->startp[midx] == NULL) {
return QF_FAIL;
}
fields->enr = (int)atol((char *)rmp->startp[midx]);
return QF_OK;
}
static int qf_parse_fmt_l(regmatch_T *rmp, int midx, qffields_T *fields)
{
if (rmp->startp[midx] == NULL) {
return QF_FAIL;
}
fields->lnum = atol((char *)rmp->startp[midx]);
return QF_OK;
}
static int qf_parse_fmt_c(regmatch_T *rmp, int midx, qffields_T *fields)
{
if (rmp->startp[midx] == NULL) {
return QF_FAIL;
}
fields->col = (int)atol((char *)rmp->startp[midx]);
return QF_OK;
}
static int qf_parse_fmt_t(regmatch_T *rmp, int midx, qffields_T *fields)
{
if (rmp->startp[midx] == NULL) {
return QF_FAIL;
}
fields->type = *rmp->startp[midx];
return QF_OK;
}
static void qf_parse_fmt_plus(const char_u *linebuf,
size_t linelen,
qffields_T *fields)
FUNC_ATTR_NONNULL_ALL
{
if (linelen >= fields->errmsglen) {
fields->errmsg = xrealloc(fields->errmsg, linelen + 1);
fields->errmsglen = linelen + 1;
}
STRLCPY(fields->errmsg, linebuf, linelen + 1);
}
static int qf_parse_fmt_m(regmatch_T *rmp, int midx, qffields_T *fields)
{
size_t len;
if (rmp->startp[midx] == NULL || rmp->endp[midx] == NULL) {
return QF_FAIL;
}
len = (size_t)(rmp->endp[midx] - rmp->startp[midx]);
if (len >= fields->errmsglen) {
fields->errmsg = xrealloc(fields->errmsg, len + 1);
fields->errmsglen = len + 1;
}
STRLCPY(fields->errmsg, rmp->startp[midx], len + 1);
return QF_OK;
}
static int qf_parse_fmt_r(regmatch_T *rmp, int midx, char_u **tail)
{
if (rmp->startp[midx] == NULL) {
return QF_FAIL;
}
*tail = rmp->startp[midx];
return QF_OK;
}
static int qf_parse_fmt_p(regmatch_T *rmp, int midx, qffields_T *fields)
{
char_u *match_ptr;
if (rmp->startp[midx] == NULL || rmp->endp[midx] == NULL) {
return QF_FAIL;
}
fields->col = 0;
for (match_ptr = rmp->startp[midx]; match_ptr != rmp->endp[midx];
match_ptr++) {
fields->col++;
if (*match_ptr == TAB) {
fields->col += 7;
fields->col -= fields->col % 8;
}
}
fields->col++;
fields->use_viscol = true;
return QF_OK;
}
static int qf_parse_fmt_v(regmatch_T *rmp, int midx, qffields_T *fields)
{
if (rmp->startp[midx] == NULL) {
return QF_FAIL;
}
fields->col = (int)atol((char *)rmp->startp[midx]);
fields->use_viscol = true;
return QF_OK;
}
static int qf_parse_fmt_s(regmatch_T *rmp, int midx, qffields_T *fields)
{
size_t len;
if (rmp->startp[midx] == NULL || rmp->endp[midx] == NULL) {
return QF_FAIL;
}
len = (size_t)(rmp->endp[midx] - rmp->startp[midx]);
if (len > CMDBUFFSIZE - 5) {
len = CMDBUFFSIZE - 5;
}
STRCPY(fields->pattern, "^\\V");
xstrlcat((char *)fields->pattern, (char *)rmp->startp[midx], len + 4);
fields->pattern[len + 3] = '\\';
fields->pattern[len + 4] = '$';
fields->pattern[len + 5] = NUL;
return QF_OK;
}
static int qf_parse_fmt_o(regmatch_T *rmp, int midx, qffields_T *fields)
{
size_t len;
size_t dsize;
if (rmp->startp[midx] == NULL || rmp->endp[midx] == NULL) {
return QF_FAIL;
}
len = (size_t)(rmp->endp[midx] - rmp->startp[midx]);
dsize = STRLEN(fields->module) + len + 1;
if (dsize > CMDBUFFSIZE) {
dsize = CMDBUFFSIZE;
}
xstrlcat((char *)fields->module, (char *)rmp->startp[midx], dsize);
return QF_OK;
}
static int (*qf_parse_fmt[FMT_PATTERNS])(regmatch_T *, int, qffields_T *) = {
NULL,
qf_parse_fmt_n,
qf_parse_fmt_l,
qf_parse_fmt_c,
qf_parse_fmt_t,
qf_parse_fmt_m,
NULL,
qf_parse_fmt_p,
qf_parse_fmt_v,
qf_parse_fmt_s,
qf_parse_fmt_o
};
static int qf_parse_match(char_u *linebuf, size_t linelen, efm_T *fmt_ptr,
regmatch_T *regmatch, qffields_T *fields,
int qf_multiline, int qf_multiscan, char_u **tail)
{
char_u idx = fmt_ptr->prefix;
int i;
int midx;
int status;
if ((idx == 'C' || idx == 'Z') && !qf_multiline) {
return QF_FAIL;
}
if (vim_strchr((char_u *)"EWI", idx) != NULL) {
fields->type = idx;
} else {
fields->type = 0;
}
for (i = 0; i < FMT_PATTERNS; i++) {
status = QF_OK;
midx = (int)fmt_ptr->addr[i];
if (i == 0 && midx > 0) { 
status = qf_parse_fmt_f(regmatch, midx, fields, idx);
} else if (i == 5) {
if (fmt_ptr->flags == '+' && !qf_multiscan) { 
qf_parse_fmt_plus(linebuf, linelen, fields);
} else if (midx > 0) { 
status = qf_parse_fmt_m(regmatch, midx, fields);
}
} else if (i == 6 && midx > 0) { 
status = qf_parse_fmt_r(regmatch, midx, tail);
} else if (midx > 0) { 
status = (qf_parse_fmt[i])(regmatch, midx, fields);
}
if (status != QF_OK) {
return status;
}
}
return QF_OK;
}
static int qf_parse_get_fields(char_u *linebuf, size_t linelen, efm_T *fmt_ptr,
qffields_T *fields, int qf_multiline,
int qf_multiscan, char_u **tail)
{
regmatch_T regmatch;
int status = QF_FAIL;
int r;
if (qf_multiscan && vim_strchr((char_u *)"OPQ", fmt_ptr->prefix) == NULL) {
return QF_FAIL;
}
fields->namebuf[0] = NUL;
fields->module[0] = NUL;
fields->pattern[0] = NUL;
if (!qf_multiscan) {
fields->errmsg[0] = NUL;
}
fields->lnum = 0;
fields->col = 0;
fields->use_viscol = false;
fields->enr = -1;
fields->type = 0;
*tail = NULL;
regmatch.rm_ic = true;
regmatch.regprog = fmt_ptr->prog;
r = vim_regexec(&regmatch, linebuf, (colnr_T)0);
fmt_ptr->prog = regmatch.regprog;
if (r) {
status = qf_parse_match(linebuf, linelen, fmt_ptr, &regmatch, fields,
qf_multiline, qf_multiscan, tail);
}
return status;
}
static int qf_parse_dir_pfx(int idx, qffields_T *fields, qf_list_T *qfl)
{
if (idx == 'D') { 
if (*fields->namebuf == NUL) {
EMSG(_("E379: Missing or empty directory name"));
return QF_FAIL;
}
qfl->qf_directory = qf_push_dir(fields->namebuf, &qfl->qf_dir_stack, false);
if (qfl->qf_directory == NULL) {
return QF_FAIL;
}
} else if (idx == 'X') { 
qfl->qf_directory = qf_pop_dir(&qfl->qf_dir_stack);
}
return QF_OK;
}
static int qf_parse_file_pfx(int idx, qffields_T *fields, qf_list_T *qfl,
char_u *tail)
{
fields->valid = false;
if (*fields->namebuf == NUL || os_path_exists(fields->namebuf)) {
if (*fields->namebuf && idx == 'P') {
qfl->qf_currfile = qf_push_dir(fields->namebuf, &qfl->qf_file_stack,
true);
} else if (idx == 'Q') {
qfl->qf_currfile = qf_pop_dir(&qfl->qf_file_stack);
}
*fields->namebuf = NUL;
if (tail && *tail) {
STRMOVE(IObuff, skipwhite(tail));
qfl->qf_multiscan = true;
return QF_MULTISCAN;
}
}
return QF_OK;
}
static int qf_parse_line_nomatch(char_u *linebuf, size_t linelen,
qffields_T *fields)
{
fields->namebuf[0] = NUL; 
fields->lnum = 0; 
fields->valid = false;
if (linelen >= fields->errmsglen) {
fields->errmsg = xrealloc(fields->errmsg, linelen + 1);
fields->errmsglen = linelen + 1;
}
STRLCPY(fields->errmsg, linebuf, linelen + 1);
return QF_OK;
}
static int qf_parse_multiline_pfx(int idx, qf_list_T *qfl, qffields_T *fields)
{
if (!qfl->qf_multiignore) {
qfline_T *qfprev = qfl->qf_last;
if (qfprev == NULL) {
return QF_FAIL;
}
if (*fields->errmsg) {
size_t textlen = strlen((char *)qfprev->qf_text);
size_t errlen = strlen((char *)fields->errmsg);
qfprev->qf_text = xrealloc(qfprev->qf_text, textlen + errlen + 2);
qfprev->qf_text[textlen] = '\n';
STRCPY(qfprev->qf_text + textlen + 1, fields->errmsg);
}
if (qfprev->qf_nr == -1) {
qfprev->qf_nr = fields->enr;
}
if (vim_isprintc(fields->type) && !qfprev->qf_type) {
qfprev->qf_type = fields->type;
}
if (!qfprev->qf_lnum) {
qfprev->qf_lnum = fields->lnum;
}
if (!qfprev->qf_col) {
qfprev->qf_col = fields->col;
}
qfprev->qf_viscol = fields->use_viscol;
if (!qfprev->qf_fnum) {
qfprev->qf_fnum = qf_get_fnum(qfl, qfl->qf_directory,
*fields->namebuf || qfl->qf_directory
? fields->namebuf
: qfl->qf_currfile && fields->valid
? qfl->qf_currfile : 0);
}
}
if (idx == 'Z') {
qfl->qf_multiline = qfl->qf_multiignore = false;
}
line_breakcheck();
return QF_IGNORE_LINE;
}
static void locstack_queue_delreq(qf_info_T *qi)
{
qf_delq_T *q;
q = xmalloc(sizeof(qf_delq_T));
q->qi = qi;
q->next = qf_delq_head;
qf_delq_head = q;
}
static void ll_free_all(qf_info_T **pqi)
{
int i;
qf_info_T *qi;
qi = *pqi;
if (qi == NULL)
return;
*pqi = NULL; 
qi->qf_refcount--;
if (qi->qf_refcount < 1) {
if (quickfix_busy > 0) {
locstack_queue_delreq(qi);
} else {
for (i = 0; i < qi->qf_listcount; i++) {
qf_free(qf_get_list(qi, i));
}
xfree(qi);
}
}
}
void qf_free_all(win_T *wp)
{
int i;
qf_info_T *qi = &ql_info;
if (wp != NULL) {
ll_free_all(&wp->w_llist);
ll_free_all(&wp->w_llist_ref);
} else {
for (i = 0; i < qi->qf_listcount; i++) {
qf_free(qf_get_list(qi, i));
}
}
}
static void incr_quickfix_busy(void)
{
quickfix_busy++;
}
static void decr_quickfix_busy(void)
{
quickfix_busy--;
if (quickfix_busy == 0) {
while (qf_delq_head != NULL) {
qf_delq_T *q = qf_delq_head;
qf_delq_head = q->next;
ll_free_all(&q->qi);
xfree(q);
}
}
#if defined(ABORT_ON_INTERNAL_ERROR)
if (quickfix_busy < 0) {
EMSG("quickfix_busy has become negative");
abort();
}
#endif
}
#if defined(EXITFREE)
void check_quickfix_busy(void)
{
if (quickfix_busy != 0) {
EMSGN("quickfix_busy not zero on exit: %ld", (long)quickfix_busy);
#if defined(ABORT_ON_INTERNAL_ERROR)
abort();
#endif
}
}
#endif
static int qf_add_entry(qf_list_T *qfl, char_u *dir, char_u *fname,
char_u *module, int bufnum, char_u *mesg, long lnum,
int col, char_u vis_col, char_u *pattern, int nr,
char_u type, char_u valid)
{
qfline_T *qfp = xmalloc(sizeof(qfline_T));
qfline_T **lastp; 
if (bufnum != 0) {
buf_T *buf = buflist_findnr(bufnum);
qfp->qf_fnum = bufnum;
if (buf != NULL) {
buf->b_has_qf_entry |=
IS_QF_LIST(qfl) ? BUF_HAS_QF_ENTRY : BUF_HAS_LL_ENTRY;
}
} else {
qfp->qf_fnum = qf_get_fnum(qfl, dir, fname);
}
qfp->qf_text = vim_strsave(mesg);
qfp->qf_lnum = lnum;
qfp->qf_col = col;
qfp->qf_viscol = vis_col;
if (pattern == NULL || *pattern == NUL) {
qfp->qf_pattern = NULL;
} else {
qfp->qf_pattern = vim_strsave(pattern);
}
if (module == NULL || *module == NUL) {
qfp->qf_module = NULL;
} else {
qfp->qf_module = vim_strsave(module);
}
qfp->qf_nr = nr;
if (type != 1 && !vim_isprintc(type)) { 
type = 0;
}
qfp->qf_type = (char_u)type;
qfp->qf_valid = valid;
lastp = &qfl->qf_last;
if (qf_list_empty(qfl)) {
qfl->qf_start = qfp;
qfl->qf_ptr = qfp;
qfl->qf_index = 0;
qfp->qf_prev = NULL;
} else {
assert(*lastp);
qfp->qf_prev = *lastp;
(*lastp)->qf_next = qfp;
}
qfp->qf_next = NULL;
qfp->qf_cleared = false;
*lastp = qfp;
qfl->qf_count++;
if (qfl->qf_index == 0 && qfp->qf_valid) {
qfl->qf_index = qfl->qf_count;
qfl->qf_ptr = qfp;
}
return QF_OK;
}
static qf_info_T *qf_alloc_stack(qfltype_T qfltype)
FUNC_ATTR_NONNULL_RET
{
qf_info_T *qi = xcalloc(1, sizeof(qf_info_T));
qi->qf_refcount++;
qi->qfl_type = qfltype;
return qi;
}
static qf_info_T *ll_get_or_alloc_list(win_T *wp)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_NONNULL_RET
{
if (IS_LL_WINDOW(wp)) {
return wp->w_llist_ref;
}
ll_free_all(&wp->w_llist_ref);
if (wp->w_llist == NULL) {
wp->w_llist = qf_alloc_stack(QFLT_LOCATION); 
}
return wp->w_llist;
}
static qf_info_T * qf_cmd_get_stack(exarg_T *eap, int print_emsg)
{
qf_info_T *qi = &ql_info;
if (is_loclist_cmd(eap->cmdidx)) {
qi = GET_LOC_LIST(curwin);
if (qi == NULL) {
if (print_emsg) {
EMSG(_(e_loclist));
}
return NULL;
}
}
return qi;
}
static qf_info_T *qf_cmd_get_or_alloc_stack(const exarg_T *eap, win_T **pwinp)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_NONNULL_RET
{
qf_info_T *qi = &ql_info;
if (is_loclist_cmd(eap->cmdidx)) {
qi = ll_get_or_alloc_list(curwin);
*pwinp = curwin;
}
return qi;
}
static int copy_loclist_entries(const qf_list_T *from_qfl, qf_list_T *to_qfl)
FUNC_ATTR_NONNULL_ALL
{
int i;
qfline_T *from_qfp;
FOR_ALL_QFL_ITEMS(from_qfl, from_qfp, i) {
if (qf_add_entry(to_qfl,
NULL,
NULL,
from_qfp->qf_module,
0,
from_qfp->qf_text,
from_qfp->qf_lnum,
from_qfp->qf_col,
from_qfp->qf_viscol,
from_qfp->qf_pattern,
from_qfp->qf_nr,
0,
from_qfp->qf_valid) == QF_FAIL) {
return FAIL;
}
qfline_T *const prevp = to_qfl->qf_last;
prevp->qf_fnum = from_qfp->qf_fnum; 
prevp->qf_type = from_qfp->qf_type; 
if (from_qfl->qf_ptr == from_qfp) {
to_qfl->qf_ptr = prevp; 
}
}
return OK;
}
static int copy_loclist(const qf_list_T *from_qfl, qf_list_T *to_qfl)
FUNC_ATTR_NONNULL_ALL
{
to_qfl->qfl_type = from_qfl->qfl_type;
to_qfl->qf_nonevalid = from_qfl->qf_nonevalid;
to_qfl->qf_count = 0;
to_qfl->qf_index = 0;
to_qfl->qf_start = NULL;
to_qfl->qf_last = NULL;
to_qfl->qf_ptr = NULL;
if (from_qfl->qf_title != NULL) {
to_qfl->qf_title = vim_strsave(from_qfl->qf_title);
} else {
to_qfl->qf_title = NULL;
}
if (from_qfl->qf_ctx != NULL) {
to_qfl->qf_ctx = xcalloc(1, sizeof(*to_qfl->qf_ctx));
tv_copy(from_qfl->qf_ctx, to_qfl->qf_ctx);
} else {
to_qfl->qf_ctx = NULL;
}
if (from_qfl->qf_count) {
if (copy_loclist_entries(from_qfl, to_qfl) == FAIL) {
return FAIL;
}
}
to_qfl->qf_index = from_qfl->qf_index; 
to_qfl->qf_id = ++last_qf_id;
to_qfl->qf_changedtick = 0L;
if (to_qfl->qf_nonevalid) {
to_qfl->qf_ptr = to_qfl->qf_start;
to_qfl->qf_index = 1;
}
return OK;
}
void copy_loclist_stack(win_T *from, win_T *to)
FUNC_ATTR_NONNULL_ALL
{
qf_info_T *qi;
if (IS_LL_WINDOW(from)) {
qi = from->w_llist_ref;
} else {
qi = from->w_llist;
}
if (qi == NULL) { 
return;
}
to->w_llist = qf_alloc_stack(QFLT_LOCATION);
to->w_llist->qf_listcount = qi->qf_listcount;
for (int idx = 0; idx < qi->qf_listcount; idx++) {
to->w_llist->qf_curlist = idx;
if (copy_loclist(qf_get_list(qi, idx),
qf_get_list(to->w_llist, idx)) == FAIL) {
qf_free_all(to);
return;
}
}
to->w_llist->qf_curlist = qi->qf_curlist; 
}
static int qf_get_fnum(qf_list_T *qfl, char_u *directory, char_u *fname )
{
char_u *ptr = NULL;
char_u *bufname;
buf_T *buf;
if (fname == NULL || *fname == NUL) { 
return 0;
}
#if defined(BACKSLASH_IN_FILENAME)
if (directory != NULL) {
slash_adjust(directory);
}
slash_adjust(fname);
#endif
if (directory != NULL && !vim_isAbsName(fname)) {
ptr = (char_u *)concat_fnames((char *)directory, (char *)fname, true);
if (!os_path_exists(ptr)) {
xfree(ptr);
directory = qf_guess_filepath(qfl, fname);
if (directory) {
ptr = (char_u *)concat_fnames((char *)directory, (char *)fname, true);
} else {
ptr = vim_strsave(fname);
}
}
bufname = ptr;
} else {
bufname = fname;
}
if (qf_last_bufname != NULL
&& STRCMP(bufname, qf_last_bufname) == 0
&& bufref_valid(&qf_last_bufref)) {
buf = qf_last_bufref.br_buf;
xfree(ptr);
} else {
xfree(qf_last_bufname);
buf = buflist_new(bufname, NULL, (linenr_T)0, BLN_NOOPT);
qf_last_bufname = (bufname == ptr) ? bufname : vim_strsave(bufname);
set_bufref(&qf_last_bufref, buf);
}
if (buf == NULL) {
return 0;
}
buf->b_has_qf_entry =
IS_QF_LIST(qfl) ? BUF_HAS_QF_ENTRY : BUF_HAS_LL_ENTRY;
return buf->b_fnum;
}
static char_u *qf_push_dir(char_u *dirbuf, struct dir_stack_T **stackptr,
bool is_file_stack)
{
struct dir_stack_T *ds_ptr;
struct dir_stack_T *ds_new = xmalloc(sizeof(struct dir_stack_T));
ds_new->next = *stackptr;
*stackptr = ds_new;
if (vim_isAbsName(dirbuf)
|| (*stackptr)->next == NULL
|| (*stackptr && is_file_stack)) {
(*stackptr)->dirname = vim_strsave(dirbuf);
} else {
ds_new = (*stackptr)->next;
(*stackptr)->dirname = NULL;
while (ds_new) {
xfree((*stackptr)->dirname);
(*stackptr)->dirname = (char_u *)concat_fnames((char *)ds_new->dirname,
(char *)dirbuf, TRUE);
if (os_isdir((*stackptr)->dirname))
break;
ds_new = ds_new->next;
}
while ((*stackptr)->next != ds_new) {
ds_ptr = (*stackptr)->next;
(*stackptr)->next = (*stackptr)->next->next;
xfree(ds_ptr->dirname);
xfree(ds_ptr);
}
if (ds_new == NULL) {
xfree((*stackptr)->dirname);
(*stackptr)->dirname = vim_strsave(dirbuf);
}
}
if ((*stackptr)->dirname != NULL)
return (*stackptr)->dirname;
else {
ds_ptr = *stackptr;
*stackptr = (*stackptr)->next;
xfree(ds_ptr);
return NULL;
}
}
static char_u *qf_pop_dir(struct dir_stack_T **stackptr)
{
struct dir_stack_T *ds_ptr;
if (*stackptr != NULL) {
ds_ptr = *stackptr;
*stackptr = (*stackptr)->next;
xfree(ds_ptr->dirname);
xfree(ds_ptr);
}
return *stackptr ? (*stackptr)->dirname : NULL;
}
static void qf_clean_dir_stack(struct dir_stack_T **stackptr)
{
struct dir_stack_T *ds_ptr;
while ((ds_ptr = *stackptr) != NULL) {
*stackptr = (*stackptr)->next;
xfree(ds_ptr->dirname);
xfree(ds_ptr);
}
}
static char_u *qf_guess_filepath(qf_list_T *qfl, char_u *filename)
{
struct dir_stack_T *ds_ptr;
struct dir_stack_T *ds_tmp;
char_u *fullname;
if (qfl->qf_dir_stack == NULL) {
return NULL;
}
ds_ptr = qfl->qf_dir_stack->next;
fullname = NULL;
while (ds_ptr) {
xfree(fullname);
fullname = (char_u *)concat_fnames((char *)ds_ptr->dirname, (char *)filename, TRUE);
if (os_path_exists(fullname)) {
break;
}
ds_ptr = ds_ptr->next;
}
xfree(fullname);
while (qfl->qf_dir_stack->next != ds_ptr) {
ds_tmp = qfl->qf_dir_stack->next;
qfl->qf_dir_stack->next = qfl->qf_dir_stack->next->next;
xfree(ds_tmp->dirname);
xfree(ds_tmp);
}
return ds_ptr == NULL ? NULL : ds_ptr->dirname;
}
static bool qflist_valid(win_T *wp, unsigned int qf_id)
{
qf_info_T *qi = &ql_info;
if (wp) {
qi = GET_LOC_LIST(wp);
if (!qi) {
return false;
}
}
for (int i = 0; i < qi->qf_listcount; i++) {
if (qi->qf_lists[i].qf_id == qf_id) {
return true;
}
}
return false;
}
static bool is_qf_entry_present(qf_list_T *qfl, qfline_T *qf_ptr)
{
qfline_T *qfp;
int i;
FOR_ALL_QFL_ITEMS(qfl, qfp, i) {
if (qfp == qf_ptr) {
break;
}
}
if (i > qfl->qf_count) { 
return false;
}
return true;
}
static qfline_T *get_next_valid_entry(qf_list_T *qfl, qfline_T *qf_ptr,
int *qf_index, int dir)
{
int idx = *qf_index;
int old_qf_fnum = qf_ptr->qf_fnum;
do {
if (idx == qfl->qf_count || qf_ptr->qf_next == NULL) {
return NULL;
}
idx++;
qf_ptr = qf_ptr->qf_next;
} while ((!qfl->qf_nonevalid && !qf_ptr->qf_valid)
|| (dir == FORWARD_FILE && qf_ptr->qf_fnum == old_qf_fnum));
*qf_index = idx;
return qf_ptr;
}
static qfline_T *get_prev_valid_entry(qf_list_T *qfl, qfline_T *qf_ptr,
int *qf_index, int dir)
{
int idx = *qf_index;
int old_qf_fnum = qf_ptr->qf_fnum;
do {
if (idx == 1 || qf_ptr->qf_prev == NULL) {
return NULL;
}
idx--;
qf_ptr = qf_ptr->qf_prev;
} while ((!qfl->qf_nonevalid && !qf_ptr->qf_valid)
|| (dir == BACKWARD_FILE && qf_ptr->qf_fnum == old_qf_fnum));
*qf_index = idx;
return qf_ptr;
}
static qfline_T *get_nth_valid_entry(qf_list_T *qfl, int errornr,
qfline_T *qf_ptr, int *qf_index, int dir)
{
qfline_T *prev_qf_ptr;
int prev_index;
char_u *err = e_no_more_items;
while (errornr--) {
prev_qf_ptr = qf_ptr;
prev_index = *qf_index;
if (dir == FORWARD || dir == FORWARD_FILE) {
qf_ptr = get_next_valid_entry(qfl, qf_ptr, qf_index, dir);
} else {
qf_ptr = get_prev_valid_entry(qfl, qf_ptr, qf_index, dir);
}
if (qf_ptr == NULL) {
qf_ptr = prev_qf_ptr;
*qf_index = prev_index;
if (err != NULL) {
EMSG(_(err));
return NULL;
}
break;
}
err = NULL;
}
return qf_ptr;
}
static qfline_T *get_nth_entry(qf_list_T *qfl, int errornr, qfline_T *qf_ptr,
int *cur_qfidx)
{
int qf_idx = *cur_qfidx;
while (errornr < qf_idx && qf_idx > 1 && qf_ptr->qf_prev != NULL) {
qf_idx--;
qf_ptr = qf_ptr->qf_prev;
}
while (errornr > qf_idx
&& qf_idx < qfl->qf_count
&& qf_ptr->qf_next != NULL) {
qf_idx++;
qf_ptr = qf_ptr->qf_next;
}
*cur_qfidx = qf_idx;
return qf_ptr;
}
static win_T *qf_find_help_win(void)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (bt_help(wp->w_buffer)) {
return wp;
}
}
return NULL;
}
static void win_set_loclist(win_T *wp, qf_info_T *qi)
{
wp->w_llist = qi;
qi->qf_refcount++;
}
static int jump_to_help_window(qf_info_T *qi, int *opened_window)
{
win_T *wp = NULL;
if (cmdmod.tab != 0) {
wp = NULL;
} else {
wp = qf_find_help_win();
}
if (wp != NULL && wp->w_buffer->b_nwindows > 0) {
win_enter(wp, true);
} else {
int flags = WSP_HELP;
if (cmdmod.split == 0
&& curwin->w_width != Columns
&& curwin->w_width < 80) {
flags |= WSP_TOP;
}
if (IS_LL_STACK(qi)) {
flags |= WSP_NEWLOC; 
}
if (win_split(0, flags) == FAIL) {
return FAIL;
}
*opened_window = true;
if (curwin->w_height < p_hh) {
win_setheight((int)p_hh);
}
if (IS_LL_STACK(qi)) { 
win_set_loclist(curwin, qi);
}
}
if (!p_im) {
restart_edit = 0; 
}
return OK;
}
static win_T *qf_find_win_with_loclist(const qf_info_T *ll)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wp->w_llist == ll && !bt_quickfix(wp->w_buffer)) {
return wp;
}
}
return NULL;
}
static win_T *qf_find_win_with_normal_buf(void)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (bt_normal(wp->w_buffer)) {
return wp;
}
}
return NULL;
}
static bool qf_goto_tabwin_with_file(int fnum)
{
FOR_ALL_TAB_WINDOWS(tp, wp) {
if (wp->w_buffer->b_fnum == fnum) {
goto_tabpage_win(tp, wp);
return true;
}
}
return false;
}
static int qf_open_new_file_win(qf_info_T *ll_ref)
{
int flags = WSP_ABOVE;
if (ll_ref != NULL) {
flags |= WSP_NEWLOC;
}
if (win_split(0, flags) == FAIL) {
return FAIL; 
}
p_swb = empty_option; 
swb_flags = 0;
RESET_BINDING(curwin);
if (ll_ref != NULL) {
win_set_loclist(curwin, ll_ref);
}
return OK;
}
static void qf_goto_win_with_ll_file(win_T *use_win, int qf_fnum,
qf_info_T *ll_ref)
{
win_T *win = use_win;
if (win == NULL) {
FOR_ALL_WINDOWS_IN_TAB(win2, curtab) {
if (win2->w_buffer->b_fnum == qf_fnum) {
win = win2;
break;
}
}
if (win == NULL) {
win = curwin;
do {
if (bt_normal(win->w_buffer)) {
break;
}
if (win->w_prev == NULL) {
win = lastwin; 
} else {
win = win->w_prev; 
}
} while (win != curwin);
}
}
win_goto(win);
if (win->w_llist == NULL && ll_ref != NULL) {
win_set_loclist(win, ll_ref);
}
}
static void qf_goto_win_with_qfl_file(int qf_fnum)
{
win_T *win = curwin;
win_T *altwin = NULL;
for (;;) {
if (win->w_buffer->b_fnum == qf_fnum) {
break;
}
if (win->w_prev == NULL) {
win = lastwin; 
} else {
win = win->w_prev; 
}
if (IS_QF_WINDOW(win)) {
if ((swb_flags & SWB_USELAST) && win_valid(prevwin)) {
win = prevwin;
} else if (altwin != NULL) {
win = altwin;
} else if (curwin->w_prev != NULL) {
win = curwin->w_prev;
} else {
win = curwin->w_next;
}
break;
}
if (altwin == NULL
&& !win->w_p_pvw
&& bt_normal(win->w_buffer)) {
altwin = win;
}
}
win_goto(win);
}
static int qf_jump_to_usable_window(int qf_fnum, int *opened_window)
{
win_T *usable_win_ptr = NULL;
bool usable_win = false;
qf_info_T *ll_ref = curwin->w_llist_ref;
if (ll_ref != NULL) {
usable_win_ptr = qf_find_win_with_loclist(ll_ref);
if (usable_win_ptr != NULL) {
usable_win = true;
}
}
if (!usable_win) {
win_T *win = qf_find_win_with_normal_buf();
if (win != NULL) {
usable_win = true;
}
}
if (!usable_win && (swb_flags & SWB_USETAB)) {
usable_win = qf_goto_tabwin_with_file(qf_fnum);
}
if ((ONE_WINDOW && bt_quickfix(curbuf)) || !usable_win) {
if (qf_open_new_file_win(ll_ref) != OK) {
return FAIL;
}
*opened_window = true; 
} else {
if (curwin->w_llist_ref != NULL) { 
qf_goto_win_with_ll_file(usable_win_ptr, qf_fnum, ll_ref);
} else { 
qf_goto_win_with_qfl_file(qf_fnum);
}
}
return OK;
}
static int qf_jump_edit_buffer(qf_info_T *qi, qfline_T *qf_ptr, int forceit,
win_T *oldwin, int *opened_window, int *abort)
{
qf_list_T *qfl = qf_get_curlist(qi);
qfltype_T qfl_type = qfl->qfl_type;
int retval = OK;
if (qf_ptr->qf_type == 1) {
if (!can_abandon(curbuf, forceit)) {
no_write_message();
retval = FAIL;
} else {
retval = do_ecmd(qf_ptr->qf_fnum, NULL, NULL, NULL, (linenr_T)1,
ECMD_HIDE + ECMD_SET_HELP,
oldwin == curwin ? curwin : NULL);
}
} else {
unsigned save_qfid = qfl->qf_id;
retval = buflist_getfile(qf_ptr->qf_fnum, (linenr_T)1,
GETF_SETMARK | GETF_SWITCH, forceit);
if (qfl_type == QFLT_LOCATION) {
if (!win_valid_any_tab(oldwin)) {
EMSG(_("E924: Current window was closed"));
*abort = true;
*opened_window = false;
} else if (!qflist_valid(oldwin, save_qfid)) {
EMSG(_(e_loc_list_changed));
*abort = true;
}
} else if (!is_qf_entry_present(qfl, qf_ptr)) {
if (qfl_type == QFLT_QUICKFIX) {
EMSG(_("E925: Current quickfix was changed"));
} else {
EMSG(_(e_loc_list_changed));
}
*abort = true;
}
if (*abort) {
retval = FAIL;
}
}
return retval;
}
static void qf_jump_goto_line(linenr_T qf_lnum, int qf_col, char_u qf_viscol,
char_u *qf_pattern)
{
linenr_T i;
if (qf_pattern == NULL) {
i = qf_lnum;
if (i > 0) {
if (i > curbuf->b_ml.ml_line_count) {
i = curbuf->b_ml.ml_line_count;
}
curwin->w_cursor.lnum = i;
}
if (qf_col > 0) {
curwin->w_cursor.coladd = 0;
if (qf_viscol == true) {
coladvance(qf_col - 1);
} else {
curwin->w_cursor.col = qf_col - 1;
}
curwin->w_set_curswant = true;
check_cursor();
} else {
beginline(BL_WHITE | BL_FIX);
}
} else {
pos_T save_cursor = curwin->w_cursor;
curwin->w_cursor.lnum = 0;
if (!do_search(NULL, '/', qf_pattern, (long)1, SEARCH_KEEP, NULL)) {
curwin->w_cursor = save_cursor;
}
}
}
static void qf_jump_print_msg(qf_info_T *qi, int qf_index, qfline_T *qf_ptr,
buf_T *old_curbuf, linenr_T old_lnum)
{
if (!msg_scrolled) {
update_topline_redraw();
}
snprintf((char *)IObuff, IOSIZE, _("(%d of %d)%s%s: "), qf_index,
qf_get_curlist(qi)->qf_count,
qf_ptr->qf_cleared ? _(" (line deleted)") : "",
(char *)qf_types(qf_ptr->qf_type, qf_ptr->qf_nr));
int len = (int)STRLEN(IObuff);
qf_fmt_text(skipwhite(qf_ptr->qf_text), IObuff + len, IOSIZE - len);
linenr_T i = msg_scroll;
if (curbuf == old_curbuf && curwin->w_cursor.lnum == old_lnum) {
msg_scroll = true;
} else if (!msg_scrolled && shortmess(SHM_OVERALL)) {
msg_scroll = false;
}
msg_ext_set_kind("quickfix");
msg_attr_keep(IObuff, 0, true, false);
msg_scroll = (int)i;
}
void qf_jump(qf_info_T *qi, int dir, int errornr, int forceit)
{
qf_list_T *qfl;
qfline_T *qf_ptr;
qfline_T *old_qf_ptr;
int qf_index;
int old_qf_index;
buf_T *old_curbuf;
linenr_T old_lnum;
char_u *old_swb = p_swb;
unsigned old_swb_flags = swb_flags;
int opened_window = false;
win_T *oldwin = curwin;
int print_message = true;
const bool old_KeyTyped = KeyTyped; 
int retval = OK;
if (qi == NULL)
qi = &ql_info;
if (qf_stack_empty(qi) || qf_list_empty(qf_get_curlist(qi))) {
EMSG(_(e_quickfix));
return;
}
qfl = qf_get_curlist(qi);
qf_ptr = qfl->qf_ptr;
old_qf_ptr = qf_ptr;
qf_index = qfl->qf_index;
old_qf_index = qf_index;
if (dir != 0) { 
qf_ptr = get_nth_valid_entry(qfl, errornr, qf_ptr, &qf_index, dir);
if (qf_ptr == NULL) {
qf_ptr = old_qf_ptr;
qf_index = old_qf_index;
goto theend; 
}
} else if (errornr != 0) { 
qf_ptr = get_nth_entry(qfl, errornr, qf_ptr, &qf_index);
}
qfl->qf_index = qf_index;
if (qf_win_pos_update(qi, old_qf_index)) {
print_message = false;
}
if (qf_ptr->qf_type == 1 && (!bt_help(curwin->w_buffer) || cmdmod.tab != 0)) {
if (jump_to_help_window(qi, &opened_window) == FAIL) {
goto theend;
}
}
if (bt_quickfix(curbuf) && !opened_window) {
if (qf_ptr->qf_fnum == 0) {
goto theend;
}
if (qf_jump_to_usable_window(qf_ptr->qf_fnum, &opened_window) == FAIL) {
goto failed;
}
}
old_curbuf = curbuf;
old_lnum = curwin->w_cursor.lnum;
if (qf_ptr->qf_fnum != 0) {
int abort = false;
retval = qf_jump_edit_buffer(qi, qf_ptr, forceit, oldwin, &opened_window,
&abort);
if (abort) {
qi = NULL;
qf_ptr = NULL;
}
}
if (retval == OK) {
if (curbuf == old_curbuf) {
setpcmark();
}
if (qf_ptr != NULL) {
qf_jump_goto_line(qf_ptr->qf_lnum, qf_ptr->qf_col, qf_ptr->qf_viscol,
qf_ptr->qf_pattern);
}
if ((fdo_flags & FDO_QUICKFIX) && old_KeyTyped)
foldOpenCursor();
if (print_message) {
qf_jump_print_msg(qi, qf_index, qf_ptr, old_curbuf, old_lnum);
}
} else {
if (opened_window) {
win_close(curwin, true); 
}
if (qf_ptr != NULL && qf_ptr->qf_fnum != 0) {
failed:
qf_ptr = old_qf_ptr;
qf_index = old_qf_index;
}
}
theend:
if (qi != NULL) {
qfl->qf_ptr = qf_ptr;
qfl->qf_index = qf_index;
}
if (p_swb != old_swb && opened_window) {
if (p_swb == empty_option) {
p_swb = old_swb;
swb_flags = old_swb_flags;
} else
free_string_option(old_swb);
}
}
static int qfFileAttr;
static int qfSepAttr;
static int qfLineAttr;
static void qf_list_entry(qfline_T *qfp, int qf_idx, bool cursel)
{
char_u *fname;
buf_T *buf;
fname = NULL;
if (qfp->qf_module != NULL && *qfp->qf_module != NUL) {
vim_snprintf((char *)IObuff, IOSIZE, "%2d %s", qf_idx,
(char *)qfp->qf_module);
} else {
if (qfp->qf_fnum != 0
&& (buf = buflist_findnr(qfp->qf_fnum)) != NULL) {
fname = buf->b_fname;
if (qfp->qf_type == 1) { 
fname = path_tail(fname);
}
}
if (fname == NULL) {
snprintf((char *)IObuff, IOSIZE, "%2d", qf_idx);
} else {
vim_snprintf((char *)IObuff, IOSIZE, "%2d %s",
qf_idx, (char *)fname);
}
}
bool filter_entry = true;
if (qfp->qf_module != NULL && *qfp->qf_module != NUL) {
filter_entry &= message_filtered(qfp->qf_module);
}
if (filter_entry && fname != NULL) {
filter_entry &= message_filtered(fname);
}
if (filter_entry && qfp->qf_pattern != NULL) {
filter_entry &= message_filtered(qfp->qf_pattern);
}
if (filter_entry) {
filter_entry &= message_filtered(qfp->qf_text);
}
if (filter_entry) {
return;
}
msg_putchar('\n');
msg_outtrans_attr(IObuff, cursel ? HL_ATTR(HLF_QFL) : qfFileAttr);
if (qfp->qf_lnum != 0) {
msg_puts_attr(":", qfSepAttr);
}
if (qfp->qf_lnum == 0) {
IObuff[0] = NUL;
} else if (qfp->qf_col == 0) {
vim_snprintf((char *)IObuff, IOSIZE, "%" PRIdLINENR, qfp->qf_lnum);
} else {
vim_snprintf((char *)IObuff, IOSIZE, "%" PRIdLINENR " col %d",
qfp->qf_lnum, qfp->qf_col);
}
vim_snprintf((char *)IObuff + STRLEN(IObuff), IOSIZE, "%s",
(char *)qf_types(qfp->qf_type, qfp->qf_nr));
msg_puts_attr((const char *)IObuff, qfLineAttr);
msg_puts_attr(":", qfSepAttr);
if (qfp->qf_pattern != NULL) {
qf_fmt_text(qfp->qf_pattern, IObuff, IOSIZE);
msg_puts((const char *)IObuff);
msg_puts_attr(":", qfSepAttr);
}
msg_puts(" ");
qf_fmt_text((fname != NULL || qfp->qf_lnum != 0)
? skipwhite(qfp->qf_text) : qfp->qf_text,
IObuff, IOSIZE);
msg_prt_line(IObuff, false);
ui_flush(); 
}
void qf_list(exarg_T *eap)
{
qf_list_T *qfl;
qfline_T *qfp;
int i;
int idx1 = 1;
int idx2 = -1;
char_u *arg = eap->arg;
int all = eap->forceit; 
qf_info_T *qi;
if ((qi = qf_cmd_get_stack(eap, true)) == NULL) {
return;
}
if (qf_stack_empty(qi) || qf_list_empty(qf_get_curlist(qi))) {
EMSG(_(e_quickfix));
return;
}
bool plus = false;
if (*arg == '+') {
arg++;
plus = true;
}
if (!get_list_range(&arg, &idx1, &idx2) || *arg != NUL) {
EMSG(_(e_trailing));
return;
}
qfl = qf_get_curlist(qi);
if (plus) {
i = qfl->qf_index;
idx2 = i + idx1;
idx1 = i;
} else {
i = qfl->qf_count;
if (idx1 < 0) {
idx1 = (-idx1 > i) ? 0 : idx1 + i + 1;
}
if (idx2 < 0) {
idx2 = (-idx2 > i) ? 0 : idx2 + i + 1;
}
}
shorten_fnames(false);
qfFileAttr = syn_name2attr((char_u *)"qfFileName");
if (qfFileAttr == 0) {
qfFileAttr = HL_ATTR(HLF_D);
}
qfSepAttr = syn_name2attr((char_u *)"qfSeparator");
if (qfSepAttr == 0) {
qfSepAttr = HL_ATTR(HLF_D);
}
qfLineAttr = syn_name2attr((char_u *)"qfLineNr");
if (qfLineAttr == 0) {
qfLineAttr = HL_ATTR(HLF_N);
}
if (qfl->qf_nonevalid) {
all = true;
}
FOR_ALL_QFL_ITEMS(qfl, qfp, i) {
if ((qfp->qf_valid || all) && idx1 <= i && i <= idx2) {
qf_list_entry(qfp, i, i == qfl->qf_index);
}
os_breakcheck();
}
}
static void qf_fmt_text(const char_u *restrict text, char_u *restrict buf,
int bufsize)
FUNC_ATTR_NONNULL_ALL
{
int i;
const char_u *p = text;
for (i = 0; *p != NUL && i < bufsize - 1; ++i) {
if (*p == '\n') {
buf[i] = ' ';
while (*++p != NUL)
if (!ascii_iswhite(*p) && *p != '\n')
break;
} else
buf[i] = *p++;
}
buf[i] = NUL;
}
static void qf_msg(qf_info_T *qi, int which, char *lead)
{
char *title = (char *)qi->qf_lists[which].qf_title;
int count = qi->qf_lists[which].qf_count;
char_u buf[IOSIZE];
vim_snprintf((char *)buf, IOSIZE, _("%serror list %d of %d; %d errors "),
lead,
which + 1,
qi->qf_listcount,
count);
if (title != NULL) {
size_t len = STRLEN(buf);
if (len < 34) {
memset(buf + len, ' ', 34 - len);
buf[34] = NUL;
}
xstrlcat((char *)buf, title, IOSIZE);
}
trunc_string(buf, buf, Columns - 1, IOSIZE);
msg(buf);
}
void qf_age(exarg_T *eap)
{
qf_info_T *qi;
int count;
if ((qi = qf_cmd_get_stack(eap, true)) == NULL) {
return;
}
if (eap->addr_count != 0) {
assert(eap->line2 <= INT_MAX);
count = (int)eap->line2;
} else {
count = 1;
}
while (count--) {
if (eap->cmdidx == CMD_colder || eap->cmdidx == CMD_lolder) {
if (qi->qf_curlist == 0) {
EMSG(_("E380: At bottom of quickfix stack"));
break;
}
--qi->qf_curlist;
} else {
if (qi->qf_curlist >= qi->qf_listcount - 1) {
EMSG(_("E381: At top of quickfix stack"));
break;
}
++qi->qf_curlist;
}
}
qf_msg(qi, qi->qf_curlist, "");
qf_update_buffer(qi, NULL);
}
void qf_history(exarg_T *eap)
{
qf_info_T *qi = qf_cmd_get_stack(eap, false);
int i;
if (qf_stack_empty(qi) || qf_list_empty(qf_get_curlist(qi))) {
MSG(_("No entries"));
} else {
for (i = 0; i < qi->qf_listcount; i++) {
qf_msg(qi, i, i == qi->qf_curlist ? "> " : " ");
}
}
}
static void qf_free_items(qf_list_T *qfl)
{
qfline_T *qfp;
qfline_T *qfpnext;
bool stop = false;
while (qfl->qf_count && qfl->qf_start != NULL) {
qfp = qfl->qf_start;
qfpnext = qfp->qf_next;
if (!stop) {
xfree(qfp->qf_module);
xfree(qfp->qf_text);
xfree(qfp->qf_pattern);
stop = (qfp == qfpnext);
xfree(qfp);
if (stop) {
qfl->qf_count = 1;
}
}
qfl->qf_start = qfpnext;
qfl->qf_count--;
}
qfl->qf_start = NULL;
qfl->qf_ptr = NULL;
qfl->qf_index = 0;
qfl->qf_start = NULL;
qfl->qf_last = NULL;
qfl->qf_ptr = NULL;
qfl->qf_nonevalid = true;
qf_clean_dir_stack(&qfl->qf_dir_stack);
qfl->qf_directory = NULL;
qf_clean_dir_stack(&qfl->qf_file_stack);
qfl->qf_currfile = NULL;
qfl->qf_multiline = false;
qfl->qf_multiignore = false;
qfl->qf_multiscan = false;
}
static void qf_free(qf_list_T *qfl)
{
qf_free_items(qfl);
XFREE_CLEAR(qfl->qf_title);
tv_free(qfl->qf_ctx);
qfl->qf_ctx = NULL;
qfl->qf_id = 0;
qfl->qf_changedtick = 0L;
}
bool qf_mark_adjust(win_T *wp, linenr_T line1, linenr_T line2, long amount,
long amount_after)
{
int i;
qfline_T *qfp;
int idx;
qf_info_T *qi = &ql_info;
bool found_one = false;
int buf_has_flag = wp == NULL ? BUF_HAS_QF_ENTRY : BUF_HAS_LL_ENTRY;
if (!(curbuf->b_has_qf_entry & buf_has_flag)) {
return false;
}
if (wp != NULL) {
if (wp->w_llist == NULL) {
return false;
}
qi = wp->w_llist;
}
for (idx = 0; idx < qi->qf_listcount; idx++) {
qf_list_T *qfl = qf_get_list(qi, idx);
if (!qf_list_empty(qfl)) {
FOR_ALL_QFL_ITEMS(qfl, qfp, i) {
if (qfp->qf_fnum == curbuf->b_fnum) {
found_one = true;
if (qfp->qf_lnum >= line1 && qfp->qf_lnum <= line2) {
if (amount == MAXLNUM)
qfp->qf_cleared = TRUE;
else
qfp->qf_lnum += amount;
} else if (amount_after && qfp->qf_lnum > line2)
qfp->qf_lnum += amount_after;
}
}
}
}
return found_one;
}
static char_u *qf_types(int c, int nr)
{
static char_u buf[20];
static char_u cc[3];
char_u *p;
if (c == 'W' || c == 'w')
p = (char_u *)" warning";
else if (c == 'I' || c == 'i')
p = (char_u *)" info";
else if (c == 'E' || c == 'e' || (c == 0 && nr > 0))
p = (char_u *)" error";
else if (c == 0 || c == 1)
p = (char_u *)"";
else {
cc[0] = ' ';
cc[1] = (char_u)c;
cc[2] = NUL;
p = cc;
}
if (nr <= 0)
return p;
sprintf((char *)buf, "%s %3d", (char *)p, nr);
return buf;
}
void qf_view_result(bool split)
{
qf_info_T *qi = &ql_info;
if (!bt_quickfix(curbuf)) {
return;
}
if (IS_LL_WINDOW(curwin)) {
qi = GET_LOC_LIST(curwin);
}
if (qf_list_empty(qf_get_curlist(qi))) {
EMSG(_(e_quickfix));
return;
}
if (split) {
char cmd[32];
snprintf(cmd, sizeof(cmd), "split +%" PRId64 "%s",
(int64_t)curwin->w_cursor.lnum,
IS_LL_WINDOW(curwin) ? "ll" : "cc");
if (do_cmdline_cmd(cmd) == OK) {
do_cmdline_cmd("clearjumps");
}
return;
}
do_cmdline_cmd((IS_LL_WINDOW(curwin) ? ".ll" : ".cc"));
}
void ex_cwindow(exarg_T *eap)
{
qf_info_T *qi;
qf_list_T *qfl;
win_T *win;
if ((qi = qf_cmd_get_stack(eap, true)) == NULL) {
return;
}
qfl = qf_get_curlist(qi);
win = qf_find_win(qi);
if (qf_stack_empty(qi)
|| qfl->qf_nonevalid
|| qf_list_empty(qf_get_curlist(qi))) {
if (win != NULL) {
ex_cclose(eap);
}
} else if (win == NULL) {
ex_copen(eap);
}
}
void ex_cclose(exarg_T *eap)
{
win_T *win = NULL;
qf_info_T *qi;
if ((qi = qf_cmd_get_stack(eap, false)) == NULL) {
return;
}
win = qf_find_win(qi);
if (win != NULL) {
win_close(win, false);
}
}
static int qf_goto_cwindow(const qf_info_T *qi, bool resize, int sz,
bool vertsplit)
{
win_T *const win = qf_find_win(qi);
if (win == NULL) {
return FAIL;
}
win_goto(win);
if (resize) {
if (vertsplit) {
if (sz != win->w_width) {
win_setwidth(sz);
}
} else if (sz != win->w_height
&& (win->w_height + win->w_status_height + tabline_height()
< cmdline_row)) {
win_setheight(sz);
}
}
return OK;
}
static int qf_open_new_cwindow(const qf_info_T *qi, int height)
{
win_T *oldwin = curwin;
const tabpage_T *const prevtab = curtab;
int flags = 0;
const buf_T *const qf_buf = qf_find_buf(qi);
win_T *const win = curwin;
if (IS_QF_STACK(qi) && cmdmod.split == 0) {
win_goto(lastwin);
}
if (cmdmod.split == 0) {
flags = WSP_BELOW;
}
flags |= WSP_NEWLOC;
if (win_split(height, flags) == FAIL) {
return FAIL; 
}
RESET_BINDING(curwin);
if (IS_LL_STACK(qi)) {
curwin->w_llist_ref = win->w_llist;
win->w_llist->qf_refcount++;
}
if (oldwin != curwin) {
oldwin = NULL; 
}
if (qf_buf != NULL) {
(void)do_ecmd(qf_buf->b_fnum, NULL, NULL, NULL, ECMD_ONE,
ECMD_HIDE + ECMD_OLDBUF, oldwin);
} else {
(void)do_ecmd(0, NULL, NULL, NULL, ECMD_ONE, ECMD_HIDE, oldwin);
set_option_value("swf", 0L, NULL, OPT_LOCAL);
set_option_value("bt", 0L, "quickfix", OPT_LOCAL);
set_option_value("bh", 0L, "wipe", OPT_LOCAL);
RESET_BINDING(curwin);
curwin->w_p_diff = false;
set_option_value("fdm", 0L, "manual", OPT_LOCAL);
}
if (curtab == prevtab && curwin->w_width == Columns) {
win_setheight(height);
}
curwin->w_p_wfh = true; 
if (win_valid(win)) {
prevwin = win;
}
return OK;
}
static void qf_set_title_var(qf_list_T *qfl)
{
if (qfl->qf_title != NULL) {
set_internal_string_var((char_u *)"w:quickfix_title", qfl->qf_title);
}
}
void ex_copen(exarg_T *eap)
{
qf_info_T *qi;
qf_list_T *qfl;
int height;
int status = FAIL;
int lnum;
if ((qi = qf_cmd_get_stack(eap, true)) == NULL) {
return;
}
incr_quickfix_busy();
if (eap->addr_count != 0) {
assert(eap->line2 <= INT_MAX);
height = (int)eap->line2;
} else {
height = QF_WINHEIGHT;
}
reset_VIsual_and_resel(); 
if (cmdmod.tab == 0) {
status = qf_goto_cwindow(qi, eap->addr_count != 0, height,
cmdmod.split & WSP_VERT);
}
if (status == FAIL) {
if (qf_open_new_cwindow(qi, height) == FAIL) {
decr_quickfix_busy();
return;
}
}
qfl = qf_get_curlist(qi);
qf_set_title_var(qfl);
lnum = qfl->qf_index;
qf_fill_buffer(qfl, curbuf, NULL);
decr_quickfix_busy();
curwin->w_cursor.lnum = lnum;
curwin->w_cursor.col = 0;
check_cursor();
update_topline(); 
}
static void qf_win_goto(win_T *win, linenr_T lnum)
{
win_T *old_curwin = curwin;
curwin = win;
curbuf = win->w_buffer;
curwin->w_cursor.lnum = lnum;
curwin->w_cursor.col = 0;
curwin->w_cursor.coladd = 0;
curwin->w_curswant = 0;
update_topline(); 
redraw_later(VALID);
curwin->w_redr_status = true; 
curwin = old_curwin;
curbuf = curwin->w_buffer;
}
void ex_cbottom(exarg_T *eap)
{
qf_info_T *qi;
if ((qi = qf_cmd_get_stack(eap, true)) == NULL) {
return;
}
win_T *win = qf_find_win(qi);
if (win != NULL && win->w_cursor.lnum != win->w_buffer->b_ml.ml_line_count) {
qf_win_goto(win, win->w_buffer->b_ml.ml_line_count);
}
}
linenr_T qf_current_entry(win_T *wp)
{
qf_info_T *qi = &ql_info;
if (IS_LL_WINDOW(wp)) {
qi = wp->w_llist_ref;
}
return qf_get_curlist(qi)->qf_index;
}
static int qf_win_pos_update(
qf_info_T *qi,
int old_qf_index 
)
{
win_T *win;
int qf_index = qf_get_curlist(qi)->qf_index;
win = qf_find_win(qi);
if (win != NULL
&& qf_index <= win->w_buffer->b_ml.ml_line_count
&& old_qf_index != qf_index) {
if (qf_index > old_qf_index) {
win->w_redraw_top = old_qf_index;
win->w_redraw_bot = qf_index;
} else {
win->w_redraw_top = qf_index;
win->w_redraw_bot = old_qf_index;
}
qf_win_goto(win, qf_index);
}
return win != NULL;
}
static int is_qf_win(const win_T *win, const qf_info_T *qi)
FUNC_ATTR_NONNULL_ARG(2) FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
if (bt_quickfix(win->w_buffer)) {
if ((IS_QF_STACK(qi) && win->w_llist_ref == NULL)
|| (IS_LL_STACK(qi) && win->w_llist_ref == qi)) {
return true;
}
}
return false;
}
static win_T *qf_find_win(const qf_info_T *qi)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
FOR_ALL_WINDOWS_IN_TAB(win, curtab) {
if (is_qf_win(win, qi)) {
return win;
}
}
return NULL;
}
static buf_T *qf_find_buf(const qf_info_T *qi)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
FOR_ALL_TAB_WINDOWS(tp, win) {
if (is_qf_win(win, qi)) {
return win->w_buffer;
}
}
return NULL;
}
static void qf_update_win_titlevar(qf_info_T *qi)
{
win_T *win;
if ((win = qf_find_win(qi)) != NULL) {
win_T *curwin_save = curwin;
curwin = win;
qf_set_title_var(qf_get_curlist(qi));
curwin = curwin_save;
}
}
static void qf_update_buffer(qf_info_T *qi, qfline_T *old_last)
{
buf_T *buf;
win_T *win;
aco_save_T aco;
buf = qf_find_buf(qi);
if (buf != NULL) {
linenr_T old_line_count = buf->b_ml.ml_line_count;
if (old_last == NULL) {
aucmd_prepbuf(&aco, buf);
}
qf_update_win_titlevar(qi);
qf_fill_buffer(qf_get_curlist(qi), buf, old_last);
buf_inc_changedtick(buf);
if (old_last == NULL) {
(void)qf_win_pos_update(qi, 0);
aucmd_restbuf(&aco);
}
if ((win = qf_find_win(qi)) != NULL && old_line_count < win->w_botline) {
redraw_buf_later(buf, NOT_VALID);
}
}
}
static int qf_buf_add_line(buf_T *buf, linenr_T lnum, const qfline_T *qfp,
char_u *dirname)
FUNC_ATTR_NONNULL_ALL
{
int len;
buf_T *errbuf;
if (qfp->qf_module != NULL) {
STRCPY(IObuff, qfp->qf_module);
len = (int)STRLEN(IObuff);
} else if (qfp->qf_fnum != 0
&& (errbuf = buflist_findnr(qfp->qf_fnum)) != NULL
&& errbuf->b_fname != NULL) {
if (qfp->qf_type == 1) { 
STRLCPY(IObuff, path_tail(errbuf->b_fname), sizeof(IObuff));
} else {
if (errbuf->b_sfname == NULL
|| path_is_absolute(errbuf->b_sfname)) {
if (*dirname == NUL) {
os_dirname(dirname, MAXPATHL);
}
shorten_buf_fname(errbuf, dirname, false);
}
STRLCPY(IObuff, errbuf->b_fname, sizeof(IObuff));
}
len = (int)STRLEN(IObuff);
} else {
len = 0;
}
IObuff[len++] = '|';
if (qfp->qf_lnum > 0) {
snprintf((char *)IObuff + len, sizeof(IObuff), "%" PRId64,
(int64_t)qfp->qf_lnum);
len += (int)STRLEN(IObuff + len);
if (qfp->qf_col > 0) {
snprintf((char *)IObuff + len, sizeof(IObuff), " col %d", qfp->qf_col);
len += (int)STRLEN(IObuff + len);
}
snprintf((char *)IObuff + len, sizeof(IObuff), "%s",
(char *)qf_types(qfp->qf_type, qfp->qf_nr));
len += (int)STRLEN(IObuff + len);
} else if (qfp->qf_pattern != NULL) {
qf_fmt_text(qfp->qf_pattern, IObuff + len, IOSIZE - len);
len += (int)STRLEN(IObuff + len);
}
IObuff[len++] = '|';
IObuff[len++] = ' ';
qf_fmt_text(len > 3 ? skipwhite(qfp->qf_text) : qfp->qf_text,
IObuff + len, IOSIZE - len);
if (ml_append_buf(buf, lnum, IObuff,
(colnr_T)STRLEN(IObuff) + 1, false) == FAIL) {
return FAIL;
}
return OK;
}
static void qf_fill_buffer(qf_list_T *qfl, buf_T *buf, qfline_T *old_last)
FUNC_ATTR_NONNULL_ARG(2)
{
linenr_T lnum;
qfline_T *qfp;
const bool old_KeyTyped = KeyTyped;
if (old_last == NULL) {
if (buf != curbuf) {
internal_error("qf_fill_buffer()");
return;
}
while ((curbuf->b_ml.ml_flags & ML_EMPTY) == 0) {
(void)ml_delete((linenr_T)1, false);
}
}
if (qfl != NULL) {
char_u dirname[MAXPATHL];
*dirname = NUL;
if (old_last == NULL || old_last->qf_next == NULL) {
qfp = qfl->qf_start;
lnum = 0;
} else {
qfp = old_last->qf_next;
lnum = buf->b_ml.ml_line_count;
}
while (lnum < qfl->qf_count) {
if (qf_buf_add_line(buf, lnum, qfp, dirname) == FAIL) {
break;
}
lnum++;
qfp = qfp->qf_next;
if (qfp == NULL) {
break;
}
}
if (old_last == NULL) {
(void)ml_delete(lnum + 1, false);
}
}
check_lnums(true);
if (old_last == NULL) {
curbuf_lock++;
set_option_value("ft", 0L, "qf", OPT_LOCAL);
curbuf->b_p_ma = false;
keep_filetype = true; 
apply_autocmds(EVENT_BUFREADPOST, (char_u *)"quickfix", NULL,
false, curbuf);
apply_autocmds(EVENT_BUFWINENTER, (char_u *)"quickfix", NULL,
false, curbuf);
keep_filetype = false;
curbuf_lock--;
redraw_curbuf_later(NOT_VALID);
}
KeyTyped = old_KeyTyped;
}
static void qf_list_changed(qf_list_T *qfl)
{
qfl->qf_changedtick++;
}
static int qf_id2nr(const qf_info_T *const qi, const unsigned qfid)
{
for (int qf_idx = 0; qf_idx < qi->qf_listcount; qf_idx++) {
if (qi->qf_lists[qf_idx].qf_id == qfid) {
return qf_idx;
}
}
return INVALID_QFIDX;
}
static int qf_restore_list(qf_info_T *qi, unsigned save_qfid)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_WARN_UNUSED_RESULT
{
if (qf_get_curlist(qi)->qf_id != save_qfid) {
const int curlist = qf_id2nr(qi, save_qfid);
if (curlist < 0) {
return FAIL;
}
qi->qf_curlist = curlist;
}
return OK;
}
static void qf_jump_first(qf_info_T *qi, unsigned save_qfid, int forceit)
FUNC_ATTR_NONNULL_ALL
{
if (qf_restore_list(qi, save_qfid) == FAIL) {
return;
}
if (!qf_list_empty(qf_get_curlist(qi))) {
qf_jump(qi, 0, 0, forceit);
}
}
int grep_internal(cmdidx_T cmdidx)
{
return (cmdidx == CMD_grep
|| cmdidx == CMD_lgrep
|| cmdidx == CMD_grepadd
|| cmdidx == CMD_lgrepadd)
&& STRCMP("internal",
*curbuf->b_p_gp == NUL ? p_gp : curbuf->b_p_gp) == 0;
}
static char_u *make_get_auname(cmdidx_T cmdidx)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
switch (cmdidx) {
case CMD_make:
return (char_u *)"make";
case CMD_lmake:
return (char_u *)"lmake";
case CMD_grep:
return (char_u *)"grep";
case CMD_lgrep:
return (char_u *)"lgrep";
case CMD_grepadd:
return (char_u *)"grepadd";
case CMD_lgrepadd:
return (char_u *)"lgrepadd";
default:
return NULL;
}
}
static char *make_get_fullcmd(const char_u *makecmd, const char_u *fname)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_NONNULL_RET
{
size_t len = STRLEN(p_shq) * 2 + STRLEN(makecmd) + 1;
if (*p_sp != NUL) {
len += STRLEN(p_sp) + STRLEN(fname) + 3;
}
char *const cmd = xmalloc(len);
snprintf(cmd, len, "%s%s%s", (char *)p_shq, (char *)makecmd, (char *)p_shq);
if (*p_sp != NUL) {
append_redir(cmd, len, (char *)p_sp, (char *)fname);
}
if (msg_col == 0) {
msg_didout = false;
}
msg_start();
MSG_PUTS(":!");
msg_outtrans((char_u *)cmd); 
return cmd;
}
void ex_make(exarg_T *eap)
{
char_u *fname;
win_T *wp = NULL;
qf_info_T *qi = &ql_info;
int res;
char_u *enc = (*curbuf->b_p_menc != NUL) ? curbuf->b_p_menc : p_menc;
if (grep_internal(eap->cmdidx)) {
ex_vimgrep(eap);
return;
}
char_u *const au_name = make_get_auname(eap->cmdidx);
if (au_name != NULL && apply_autocmds(EVENT_QUICKFIXCMDPRE, au_name,
curbuf->b_fname, true, curbuf)) {
if (aborting()) {
return;
}
}
if (is_loclist_cmd(eap->cmdidx)) {
wp = curwin;
}
autowrite_all();
fname = get_mef_name();
if (fname == NULL)
return;
os_remove((char *)fname); 
char *const cmd = make_get_fullcmd(eap->arg, fname);
do_shell((char_u *)cmd, 0);
incr_quickfix_busy();
res = qf_init(wp, fname, (eap->cmdidx != CMD_make
&& eap->cmdidx != CMD_lmake) ? p_gefm : p_efm,
(eap->cmdidx != CMD_grepadd && eap->cmdidx != CMD_lgrepadd),
qf_cmdtitle(*eap->cmdlinep), enc);
if (wp != NULL) {
qi = GET_LOC_LIST(wp);
if (qi == NULL) {
goto cleanup;
}
}
if (res >= 0) {
qf_list_changed(qf_get_curlist(qi));
}
unsigned save_qfid = qf_get_curlist(qi)->qf_id;
if (au_name != NULL) {
apply_autocmds(EVENT_QUICKFIXCMDPOST, au_name, curbuf->b_fname, true,
curbuf);
}
if (res > 0 && !eap->forceit && qflist_valid(wp, save_qfid)) {
qf_jump_first(qi, save_qfid, false);
}
cleanup:
decr_quickfix_busy();
os_remove((char *)fname);
xfree(fname);
xfree(cmd);
}
static char_u *get_mef_name(void)
{
char_u *p;
char_u *name;
static int start = -1;
static int off = 0;
if (*p_mef == NUL) {
name = vim_tempname();
if (name == NULL)
EMSG(_(e_notmp));
return name;
}
for (p = p_mef; *p; ++p)
if (p[0] == '#' && p[1] == '#')
break;
if (*p == NUL)
return vim_strsave(p_mef);
for (;; ) {
if (start == -1) {
start = (int)os_get_pid();
} else {
off += 19;
}
name = xmalloc(STRLEN(p_mef) + 30);
STRCPY(name, p_mef);
sprintf((char *)name + (p - p_mef), "%d%d", start, off);
STRCAT(name, p + 2);
FileInfo file_info;
bool file_or_link_found = os_fileinfo_link((char *)name, &file_info);
if (!file_or_link_found) {
break;
}
xfree(name);
}
return name;
}
size_t qf_get_size(exarg_T *eap)
FUNC_ATTR_NONNULL_ALL
{
qf_info_T *qi;
qf_list_T *qfl;
if ((qi = qf_cmd_get_stack(eap, false)) == NULL) {
return 0;
}
int prev_fnum = 0;
size_t sz = 0;
qfline_T *qfp;
int i;
assert(qf_get_curlist(qi)->qf_count >= 0);
qfl = qf_get_curlist(qi);
FOR_ALL_QFL_ITEMS(qfl, qfp, i) {
if (!qfp->qf_valid) {
continue;
}
if (eap->cmdidx == CMD_cdo || eap->cmdidx == CMD_ldo) {
sz++;
} else if (qfp->qf_fnum > 0 && qfp->qf_fnum != prev_fnum) {
sz++;
prev_fnum = qfp->qf_fnum;
}
}
return sz;
}
size_t qf_get_cur_idx(exarg_T *eap)
FUNC_ATTR_NONNULL_ALL
{
qf_info_T *qi;
if ((qi = qf_cmd_get_stack(eap, false)) == NULL) {
return 0;
}
assert(qf_get_curlist(qi)->qf_index >= 0);
return (size_t)qf_get_curlist(qi)->qf_index;
}
int qf_get_cur_valid_idx(exarg_T *eap)
FUNC_ATTR_NONNULL_ALL
{
qf_info_T *qi;
if ((qi = qf_cmd_get_stack(eap, false)) == NULL) {
return 1;
}
qf_list_T *qfl = qf_get_curlist(qi);
if (!qf_list_has_valid_entries(qfl)) {
return 1;
}
int prev_fnum = 0;
int eidx = 0;
qfline_T *qfp;
size_t i;
assert(qfl->qf_index >= 0);
for (i = 1, qfp = qfl->qf_start;
i <= (size_t)qfl->qf_index && qfp != NULL;
i++, qfp = qfp->qf_next) {
if (!qfp->qf_valid) {
continue;
}
if (eap->cmdidx == CMD_cfdo || eap->cmdidx == CMD_lfdo) {
if (qfp->qf_fnum > 0 && qfp->qf_fnum != prev_fnum) {
eidx++;
prev_fnum = qfp->qf_fnum;
}
} else {
eidx++;
}
}
return eidx != 0 ? eidx : 1;
}
static size_t qf_get_nth_valid_entry(qf_list_T *qfl, size_t n, int fdo)
FUNC_ATTR_NONNULL_ALL
{
if (!qf_list_has_valid_entries(qfl)) {
return 1;
}
int prev_fnum = 0;
size_t eidx = 0;
int i;
qfline_T *qfp;
assert(qfl->qf_count >= 0);
FOR_ALL_QFL_ITEMS(qfl, qfp, i) {
if (qfp->qf_valid) {
if (fdo) {
if (qfp->qf_fnum > 0 && qfp->qf_fnum != prev_fnum) {
eidx++;
prev_fnum = qfp->qf_fnum;
}
} else {
eidx++;
}
}
if (eidx == n) {
break;
}
}
return i <= qfl->qf_count ? (size_t)i : 1;
}
void ex_cc(exarg_T *eap)
{
qf_info_T *qi;
if ((qi = qf_cmd_get_stack(eap, true)) == NULL) {
return;
}
int errornr;
if (eap->addr_count > 0) {
errornr = (int)eap->line2;
} else {
switch (eap->cmdidx) {
case CMD_cc:
case CMD_ll:
errornr = 0;
break;
case CMD_crewind:
case CMD_lrewind:
case CMD_cfirst:
case CMD_lfirst:
errornr = 1;
break;
default:
errornr = 32767;
break;
}
}
if (eap->cmdidx == CMD_cdo || eap->cmdidx == CMD_ldo
|| eap->cmdidx == CMD_cfdo || eap->cmdidx == CMD_lfdo) {
size_t n;
if (eap->addr_count > 0) {
assert(eap->line1 >= 0);
n = (size_t)eap->line1;
} else {
n = 1;
}
size_t valid_entry = qf_get_nth_valid_entry(
qf_get_curlist(qi), n,
eap->cmdidx == CMD_cfdo || eap->cmdidx == CMD_lfdo);
assert(valid_entry <= INT_MAX);
errornr = (int)valid_entry;
}
qf_jump(qi, 0, errornr, eap->forceit);
}
void ex_cnext(exarg_T *eap)
{
qf_info_T *qi;
if ((qi = qf_cmd_get_stack(eap, true)) == NULL) {
return;
}
int errornr;
if (eap->addr_count > 0
&& (eap->cmdidx != CMD_cdo && eap->cmdidx != CMD_ldo
&& eap->cmdidx != CMD_cfdo && eap->cmdidx != CMD_lfdo)) {
errornr = (int)eap->line2;
} else {
errornr = 1;
}
Direction dir;
switch (eap->cmdidx) {
case CMD_cprevious:
case CMD_lprevious:
case CMD_cNext:
case CMD_lNext:
dir = BACKWARD;
break;
case CMD_cnfile:
case CMD_lnfile:
case CMD_cfdo:
case CMD_lfdo:
dir = FORWARD_FILE;
break;
case CMD_cpfile:
case CMD_lpfile:
case CMD_cNfile:
case CMD_lNfile:
dir = BACKWARD_FILE;
break;
case CMD_cnext:
case CMD_lnext:
case CMD_cdo:
case CMD_ldo:
default:
dir = FORWARD;
break;
}
qf_jump(qi, dir, errornr, eap->forceit);
}
static qfline_T *qf_find_first_entry_in_buf(qf_list_T *qfl,
int bnr,
int *errornr)
{
qfline_T *qfp = NULL;
int idx = 0;
FOR_ALL_QFL_ITEMS(qfl, qfp, idx) {
if (qfp->qf_fnum == bnr) {
break;
}
}
*errornr = idx;
return qfp;
}
static qfline_T * qf_find_first_entry_on_line(qfline_T *entry, int *errornr)
{
while (!got_int
&& entry->qf_prev != NULL
&& entry->qf_fnum == entry->qf_prev->qf_fnum
&& entry->qf_lnum == entry->qf_prev->qf_lnum) {
entry = entry->qf_prev;
(*errornr)--;
}
return entry;
}
static qfline_T * qf_find_last_entry_on_line(qfline_T *entry, int *errornr)
{
while (!got_int
&& entry->qf_next != NULL
&& entry->qf_fnum == entry->qf_next->qf_fnum
&& entry->qf_lnum == entry->qf_next->qf_lnum) {
entry = entry->qf_next;
(*errornr)++;
}
return entry;
}
static qfline_T *qf_find_entry_on_next_line(int bnr,
linenr_T lnum,
qfline_T *qfp,
int *errornr)
{
if (qfp->qf_lnum > lnum) {
return qfp;
}
while (qfp->qf_next != NULL
&& qfp->qf_next->qf_fnum == bnr
&& qfp->qf_next->qf_lnum <= lnum) {
qfp = qfp->qf_next;
(*errornr)++;
}
if (qfp->qf_next == NULL || qfp->qf_next->qf_fnum != bnr) {
return NULL;
}
qfp = qfp->qf_next;
(*errornr)++;
return qfp;
}
static qfline_T *qf_find_entry_on_prev_line(int bnr,
linenr_T lnum,
qfline_T *qfp,
int *errornr)
{
while (qfp->qf_next != NULL
&& qfp->qf_next->qf_fnum == bnr
&& qfp->qf_next->qf_lnum < lnum) {
qfp = qfp->qf_next;
(*errornr)++;
}
if (qfp->qf_lnum >= lnum) { 
return NULL;
}
qfp = qf_find_first_entry_on_line(qfp, errornr);
return qfp;
}
static qfline_T *qf_find_closest_entry(qf_list_T *qfl,
int bnr,
linenr_T lnum,
int dir,
int *errornr)
{
qfline_T *qfp;
*errornr = 0;
qfp = qf_find_first_entry_in_buf(qfl, bnr, errornr);
if (qfp == NULL) {
return NULL; 
}
if (dir == FORWARD) {
qfp = qf_find_entry_on_next_line(bnr, lnum, qfp, errornr);
} else {
qfp = qf_find_entry_on_prev_line(bnr, lnum, qfp, errornr);
}
return qfp;
}
static void qf_get_nth_below_entry(qfline_T *entry,
int *errornr,
linenr_T n)
{
while (n-- > 0 && !got_int) {
int first_errornr = *errornr;
entry = qf_find_last_entry_on_line(entry, errornr);
if (entry->qf_next == NULL
|| entry->qf_next->qf_fnum != entry->qf_fnum) {
*errornr = first_errornr;
break;
}
entry = entry->qf_next;
(*errornr)++;
}
}
static void qf_get_nth_above_entry(qfline_T *entry,
int *errornr,
linenr_T n)
{
while (n-- > 0 && !got_int) {
if (entry->qf_prev == NULL
|| entry->qf_prev->qf_fnum != entry->qf_fnum) {
break;
}
entry = entry->qf_prev;
(*errornr)--;
entry = qf_find_first_entry_on_line(entry, errornr);
}
}
static int qf_find_nth_adj_entry(qf_list_T *qfl,
int bnr,
linenr_T lnum,
linenr_T n,
int dir)
{
qfline_T *adj_entry;
int errornr;
adj_entry = qf_find_closest_entry(qfl, bnr, lnum, dir, &errornr);
if (adj_entry == NULL) {
return 0;
}
if (--n > 0) {
if (dir == FORWARD) {
qf_get_nth_below_entry(adj_entry, &errornr, n);
} else {
qf_get_nth_above_entry(adj_entry, &errornr, n);
}
}
return errornr;
}
void ex_cbelow(exarg_T *eap)
{
qf_info_T *qi;
qf_list_T *qfl;
int dir;
int buf_has_flag;
int errornr = 0;
if (eap->addr_count > 0 && eap->line2 <= 0) {
EMSG(_(e_invrange));
return;
}
if (eap->cmdidx == CMD_cabove || eap->cmdidx == CMD_cbelow) {
buf_has_flag = BUF_HAS_QF_ENTRY;
} else {
buf_has_flag = BUF_HAS_LL_ENTRY;
}
if (!(curbuf->b_has_qf_entry & buf_has_flag)) {
EMSG(_(e_quickfix));
return;
}
if ((qi = qf_cmd_get_stack(eap, true)) == NULL) {
return;
}
qfl = qf_get_curlist(qi);
if (!qf_list_has_valid_entries(qfl)) {
EMSG(_(e_quickfix));
return;
}
if (eap->cmdidx == CMD_cbelow || eap->cmdidx == CMD_lbelow) {
dir = FORWARD;
} else {
dir = BACKWARD;
}
errornr = qf_find_nth_adj_entry(qfl, curbuf->b_fnum, curwin->w_cursor.lnum,
eap->addr_count > 0 ? eap->line2 : 0, dir);
if (errornr > 0) {
qf_jump(qi, 0, errornr, false);
} else {
EMSG(_(e_no_more_items));
}
}
static char_u * cfile_get_auname(cmdidx_T cmdidx)
{
switch (cmdidx) {
case CMD_cfile: return (char_u *)"cfile";
case CMD_cgetfile: return (char_u *)"cgetfile";
case CMD_caddfile: return (char_u *)"caddfile";
case CMD_lfile: return (char_u *)"lfile";
case CMD_lgetfile: return (char_u *)"lgetfile";
case CMD_laddfile: return (char_u *)"laddfile";
default: return NULL;
}
}
void ex_cfile(exarg_T *eap)
{
win_T *wp = NULL;
qf_info_T *qi = &ql_info;
char_u *au_name = NULL;
au_name = cfile_get_auname(eap->cmdidx);
if (au_name != NULL
&& apply_autocmds(EVENT_QUICKFIXCMDPRE, au_name, NULL, false, curbuf)) {
if (aborting()) {
return;
}
}
if (*eap->arg != NUL) {
set_string_option_direct((char_u *)"ef", -1, eap->arg, OPT_FREE, 0);
}
char_u *enc = (*curbuf->b_p_menc != NUL) ? curbuf->b_p_menc : p_menc;
if (is_loclist_cmd(eap->cmdidx)) {
wp = curwin;
}
incr_quickfix_busy();
int res = qf_init(wp, p_ef, p_efm, (eap->cmdidx != CMD_caddfile
&& eap->cmdidx != CMD_laddfile),
qf_cmdtitle(*eap->cmdlinep), enc);
if (wp != NULL) {
qi = GET_LOC_LIST(wp);
if (qi == NULL) {
decr_quickfix_busy();
return;
}
}
if (res >= 0) {
qf_list_changed(qf_get_curlist(qi));
}
unsigned save_qfid = qf_get_curlist(qi)->qf_id;
if (au_name != NULL) {
apply_autocmds(EVENT_QUICKFIXCMDPOST, au_name, NULL, false, curbuf);
}
if (res > 0 && (eap->cmdidx == CMD_cfile || eap->cmdidx == CMD_lfile)
&& qflist_valid(wp, save_qfid)) {
qf_jump_first(qi, save_qfid, eap->forceit);
}
decr_quickfix_busy();
}
static char_u *vgr_get_auname(cmdidx_T cmdidx)
{
switch (cmdidx) {
case CMD_vimgrep: return (char_u *)"vimgrep";
case CMD_lvimgrep: return (char_u *)"lvimgrep";
case CMD_vimgrepadd: return (char_u *)"vimgrepadd";
case CMD_lvimgrepadd: return (char_u *)"lvimgrepadd";
case CMD_grep: return (char_u *)"grep";
case CMD_lgrep: return (char_u *)"lgrep";
case CMD_grepadd: return (char_u *)"grepadd";
case CMD_lgrepadd: return (char_u *)"lgrepadd";
default: return NULL;
}
}
static void vgr_init_regmatch(regmmatch_T *regmatch, char_u *s)
{
regmatch->regprog = NULL;
if (s == NULL || *s == NUL) {
if (last_search_pat() == NULL) {
EMSG(_(e_noprevre));
return;
}
regmatch->regprog = vim_regcomp(last_search_pat(), RE_MAGIC);
} else {
regmatch->regprog = vim_regcomp(s, RE_MAGIC);
}
regmatch->rmm_ic = p_ic;
regmatch->rmm_maxcol = 0;
}
static void vgr_display_fname(char_u *fname)
{
msg_start();
char_u *p = msg_strtrunc(fname, true);
if (p == NULL) {
msg_outtrans(fname);
} else {
msg_outtrans(p);
xfree(p);
}
msg_clr_eos();
msg_didout = false; 
msg_nowait = true; 
msg_col = 0;
ui_flush();
}
static buf_T *vgr_load_dummy_buf(char_u *fname, char_u *dirname_start,
char_u *dirname_now)
{
char_u *save_ei = au_event_disable(",Filetype");
long save_mls = p_mls;
p_mls = 0;
buf_T *buf = load_dummy_buffer(fname, dirname_start, dirname_now);
p_mls = save_mls;
au_event_restore(save_ei);
return buf;
}
static bool vgr_qflist_valid(win_T *wp, qf_info_T *qi, unsigned qfid,
char_u *title)
{
if (!qflist_valid(wp, qfid)) {
if (wp != NULL) {
EMSG(_(e_loc_list_changed));
return false;
} else {
qf_new_list(qi, title);
return true;
}
}
if (qf_restore_list(qi, qfid) == FAIL) {
return false;
}
return true;
}
static bool vgr_match_buflines(qf_info_T *qi, char_u *fname, buf_T *buf,
regmmatch_T *regmatch, long tomatch,
int duplicate_name, int flags)
{
bool found_match = false;
for (long lnum = 1; lnum <= buf->b_ml.ml_line_count && tomatch > 0; lnum++) {
colnr_T col = 0;
while (vim_regexec_multi(regmatch, curwin, buf, lnum, col, NULL,
NULL) > 0) {
if (qf_add_entry(qf_get_curlist(qi),
NULL, 
fname,
NULL,
duplicate_name ? 0 : buf->b_fnum,
ml_get_buf(buf, regmatch->startpos[0].lnum + lnum,
false),
regmatch->startpos[0].lnum + lnum,
regmatch->startpos[0].col + 1,
false, 
NULL, 
0, 
0, 
true) 
== QF_FAIL) {
got_int = true;
break;
}
found_match = true;
if (--tomatch == 0) {
break;
}
if ((flags & VGR_GLOBAL) == 0 || regmatch->endpos[0].lnum > 0) {
break;
}
col = regmatch->endpos[0].col + (col == regmatch->endpos[0].col);
if (col > (colnr_T)STRLEN(ml_get_buf(buf, lnum, false))) {
break;
}
}
line_breakcheck();
if (got_int) {
break;
}
}
return found_match;
}
static void vgr_jump_to_match(qf_info_T *qi, int forceit, int *redraw_for_dummy,
buf_T *first_match_buf, char_u *target_dir)
{
buf_T *buf = curbuf;
qf_jump(qi, 0, 0, forceit);
if (buf != curbuf) {
*redraw_for_dummy = false;
}
if (curbuf == first_match_buf && target_dir != NULL) {
exarg_T ea = {
.arg = target_dir,
.cmdidx = CMD_lcd,
};
ex_cd(&ea);
}
}
void ex_vimgrep(exarg_T *eap)
{
regmmatch_T regmatch;
int fcount;
char_u **fnames;
char_u *fname;
char_u *s;
char_u *p;
int fi;
qf_list_T *qfl;
win_T *wp = NULL;
buf_T *buf;
int duplicate_name = FALSE;
int using_dummy;
int redraw_for_dummy = FALSE;
int found_match;
buf_T *first_match_buf = NULL;
time_t seconds = 0;
aco_save_T aco;
int flags = 0;
long tomatch;
char_u *dirname_start = NULL;
char_u *dirname_now = NULL;
char_u *target_dir = NULL;
char_u *au_name = NULL;
au_name = vgr_get_auname(eap->cmdidx);
if (au_name != NULL && apply_autocmds(EVENT_QUICKFIXCMDPRE, au_name,
curbuf->b_fname, true, curbuf)) {
if (aborting()) {
return;
}
}
qf_info_T *qi = qf_cmd_get_or_alloc_stack(eap, &wp);
if (eap->addr_count > 0)
tomatch = eap->line2;
else
tomatch = MAXLNUM;
regmatch.regprog = NULL;
char_u *title = vim_strsave(qf_cmdtitle(*eap->cmdlinep));
p = skip_vimgrep_pat(eap->arg, &s, &flags);
if (p == NULL) {
EMSG(_(e_invalpat));
goto theend;
}
vgr_init_regmatch(&regmatch, s);
if (regmatch.regprog == NULL) {
goto theend;
}
p = skipwhite(p);
if (*p == NUL) {
EMSG(_("E683: File name missing or invalid pattern"));
goto theend;
}
if ((eap->cmdidx != CMD_grepadd && eap->cmdidx != CMD_lgrepadd
&& eap->cmdidx != CMD_vimgrepadd && eap->cmdidx != CMD_lvimgrepadd)
|| qf_stack_empty(qi)) {
qf_new_list(qi, title);
}
if (get_arglist_exp(p, &fcount, &fnames, true) == FAIL) {
goto theend;
}
if (fcount == 0) {
EMSG(_(e_nomatch));
goto theend;
}
dirname_start = xmalloc(MAXPATHL);
dirname_now = xmalloc(MAXPATHL);
os_dirname(dirname_start, MAXPATHL);
incr_quickfix_busy();
unsigned save_qfid = qf_get_curlist(qi)->qf_id;
seconds = (time_t)0;
for (fi = 0; fi < fcount && !got_int && tomatch > 0; fi++) {
fname = path_try_shorten_fname(fnames[fi]);
if (time(NULL) > seconds) {
seconds = time(NULL);
vgr_display_fname(fname);
}
buf = buflist_findname_exp(fnames[fi]);
if (buf == NULL || buf->b_ml.ml_mfp == NULL) {
duplicate_name = (buf != NULL);
using_dummy = TRUE;
redraw_for_dummy = TRUE;
buf = vgr_load_dummy_buf(fname, dirname_start, dirname_now);
} else {
using_dummy = false;
}
if (!vgr_qflist_valid(wp, qi, save_qfid, *eap->cmdlinep)) {
FreeWild(fcount, fnames);
decr_quickfix_busy();
goto theend;
}
save_qfid = qf_get_curlist(qi)->qf_id;
if (buf == NULL) {
if (!got_int)
smsg(_("Cannot open file \"%s\""), fname);
} else {
found_match = vgr_match_buflines(qi, fname, buf, &regmatch, tomatch,
duplicate_name, flags);
if (using_dummy) {
if (found_match && first_match_buf == NULL)
first_match_buf = buf;
if (duplicate_name) {
wipe_dummy_buffer(buf, dirname_start);
buf = NULL;
} else if (!cmdmod.hide
|| buf->b_p_bh[0] == 'u' 
|| buf->b_p_bh[0] == 'w' 
|| buf->b_p_bh[0] == 'd') { 
if (!found_match) {
wipe_dummy_buffer(buf, dirname_start);
buf = NULL;
} else if (buf != first_match_buf || (flags & VGR_NOJUMP)) {
unload_dummy_buffer(buf, dirname_start);
buf->b_flags &= ~BF_DUMMY;
buf = NULL;
}
}
if (buf != NULL) {
buf->b_flags &= ~BF_DUMMY;
if (buf == first_match_buf
&& target_dir == NULL
&& STRCMP(dirname_start, dirname_now) != 0) {
target_dir = vim_strsave(dirname_now);
}
aucmd_prepbuf(&aco, buf);
apply_autocmds(EVENT_FILETYPE, buf->b_p_ft,
buf->b_fname, TRUE, buf);
do_modelines(OPT_NOWIN);
aucmd_restbuf(&aco);
}
}
}
}
FreeWild(fcount, fnames);
qfl = qf_get_curlist(qi);
qfl->qf_nonevalid = false;
qfl->qf_ptr = qfl->qf_start;
qfl->qf_index = 1;
qf_list_changed(qfl);
qf_update_buffer(qi, NULL);
if (au_name != NULL)
apply_autocmds(EVENT_QUICKFIXCMDPOST, au_name,
curbuf->b_fname, TRUE, curbuf);
if (!qflist_valid(wp, save_qfid)
|| qf_restore_list(qi, save_qfid) == FAIL) {
decr_quickfix_busy();
goto theend;
}
if (!qf_list_empty(qf_get_curlist(qi))) {
if ((flags & VGR_NOJUMP) == 0) {
vgr_jump_to_match(qi, eap->forceit, &redraw_for_dummy, first_match_buf,
target_dir);
}
} else
EMSG2(_(e_nomatch2), s);
decr_quickfix_busy();
if (redraw_for_dummy) {
foldUpdateAll(curwin);
}
theend:
xfree(title);
xfree(dirname_now);
xfree(dirname_start);
xfree(target_dir);
vim_regfree(regmatch.regprog);
}
static void restore_start_dir(char_u *dirname_start)
{
char_u *dirname_now = xmalloc(MAXPATHL);
os_dirname(dirname_now, MAXPATHL);
if (STRCMP(dirname_start, dirname_now) != 0) {
exarg_T ea = {
.arg = dirname_start,
.cmdidx = (curwin->w_localdir == NULL) ? CMD_cd : CMD_lcd,
};
ex_cd(&ea);
}
xfree(dirname_now);
}
static buf_T *
load_dummy_buffer (
char_u *fname,
char_u *dirname_start, 
char_u *resulting_dir 
)
{
buf_T *newbuf;
bufref_T newbufref;
bufref_T newbuf_to_wipe;
int failed = true;
aco_save_T aco;
int readfile_result;
newbuf = buflist_new(NULL, NULL, (linenr_T)1, BLN_DUMMY);
if (newbuf == NULL) {
return NULL;
}
set_bufref(&newbufref, newbuf);
buf_copy_options(newbuf, BCO_ENTER | BCO_NOHELP);
if (ml_open(newbuf) == OK) {
newbuf->b_locked++;
aucmd_prepbuf(&aco, newbuf);
(void)setfname(curbuf, fname, NULL, false);
check_need_swap(true);
curbuf->b_flags &= ~BF_DUMMY;
newbuf_to_wipe.br_buf = NULL;
readfile_result = readfile(fname, NULL, (linenr_T)0, (linenr_T)0,
(linenr_T)MAXLNUM, NULL,
READ_NEW | READ_DUMMY);
newbuf->b_locked--;
if (readfile_result == OK
&& !got_int
&& !(curbuf->b_flags & BF_NEW)) {
failed = FALSE;
if (curbuf != newbuf) {
set_bufref(&newbuf_to_wipe, newbuf);
newbuf = curbuf;
}
}
aucmd_restbuf(&aco);
if (newbuf_to_wipe.br_buf != NULL && bufref_valid(&newbuf_to_wipe)) {
wipe_buffer(newbuf_to_wipe.br_buf, false);
}
newbuf->b_flags |= BF_DUMMY;
}
os_dirname(resulting_dir, MAXPATHL);
restore_start_dir(dirname_start);
if (!bufref_valid(&newbufref)) {
return NULL;
}
if (failed) {
wipe_dummy_buffer(newbuf, dirname_start);
return NULL;
}
return newbuf;
}
static void wipe_dummy_buffer(buf_T *buf, char_u *dirname_start)
{
if (curbuf != buf) { 
cleanup_T cs;
enter_cleanup(&cs);
wipe_buffer(buf, FALSE);
leave_cleanup(&cs);
restore_start_dir(dirname_start);
}
}
static void unload_dummy_buffer(buf_T *buf, char_u *dirname_start)
{
if (curbuf != buf) { 
close_buffer(NULL, buf, DOBUF_UNLOAD, false);
restore_start_dir(dirname_start);
}
}
static int get_qfline_items(qfline_T *qfp, list_T *list)
{
char_u buf[2];
int bufnum;
bufnum = qfp->qf_fnum;
if (bufnum != 0 && (buflist_findnr(bufnum) == NULL)) {
bufnum = 0;
}
dict_T *const dict = tv_dict_alloc();
tv_list_append_dict(list, dict);
buf[0] = qfp->qf_type;
buf[1] = NUL;
if (tv_dict_add_nr(dict, S_LEN("bufnr"), (varnumber_T)bufnum) == FAIL
|| (tv_dict_add_nr(dict, S_LEN("lnum"), (varnumber_T)qfp->qf_lnum)
== FAIL)
|| (tv_dict_add_nr(dict, S_LEN("col"), (varnumber_T)qfp->qf_col) == FAIL)
|| (tv_dict_add_nr(dict, S_LEN("vcol"), (varnumber_T)qfp->qf_viscol)
== FAIL)
|| (tv_dict_add_nr(dict, S_LEN("nr"), (varnumber_T)qfp->qf_nr) == FAIL)
|| (tv_dict_add_str(
dict, S_LEN("module"),
(qfp->qf_module == NULL ? "" : (const char *)qfp->qf_module))
== FAIL)
|| (tv_dict_add_str(
dict, S_LEN("pattern"),
(qfp->qf_pattern == NULL ? "" : (const char *)qfp->qf_pattern))
== FAIL)
|| (tv_dict_add_str(
dict, S_LEN("text"),
(qfp->qf_text == NULL ? "" : (const char *)qfp->qf_text))
== FAIL)
|| (tv_dict_add_str(dict, S_LEN("type"), (const char *)buf) == FAIL)
|| (tv_dict_add_nr(dict, S_LEN("valid"), (varnumber_T)qfp->qf_valid)
== FAIL)) {
assert(false);
}
return OK;
}
int get_errorlist(qf_info_T *qi_arg, win_T *wp, int qf_idx, list_T *list)
{
qf_info_T *qi = qi_arg;
qf_list_T *qfl;
qfline_T *qfp;
int i;
if (qi == NULL) {
qi = &ql_info;
if (wp != NULL) {
qi = GET_LOC_LIST(wp);
if (qi == NULL) {
return FAIL;
}
}
}
if (qf_idx == INVALID_QFIDX) {
qf_idx = qi->qf_curlist;
}
if (qf_idx >= qi->qf_listcount) {
return FAIL;
}
qfl = qf_get_list(qi, qf_idx);
if (qf_list_empty(qfl)) {
return FAIL;
}
FOR_ALL_QFL_ITEMS(qfl, qfp, i) {
get_qfline_items(qfp, list);
}
return OK;
}
enum {
QF_GETLIST_NONE = 0x0,
QF_GETLIST_TITLE = 0x1,
QF_GETLIST_ITEMS = 0x2,
QF_GETLIST_NR = 0x4,
QF_GETLIST_WINID = 0x8,
QF_GETLIST_CONTEXT = 0x10,
QF_GETLIST_ID = 0x20,
QF_GETLIST_IDX = 0x40,
QF_GETLIST_SIZE = 0x80,
QF_GETLIST_TICK = 0x100,
QF_GETLIST_FILEWINID = 0x200,
QF_GETLIST_ALL = 0x3FF,
};
static int qf_get_list_from_lines(dict_T *what, dictitem_T *di, dict_T *retdict)
{
int status = FAIL;
char_u *errorformat = p_efm;
dictitem_T *efm_di;
if (di->di_tv.v_type == VAR_LIST && di->di_tv.vval.v_list != NULL) {
if ((efm_di = tv_dict_find(what, S_LEN("efm"))) != NULL) {
if (efm_di->di_tv.v_type != VAR_STRING
|| efm_di->di_tv.vval.v_string == NULL) {
return FAIL;
}
errorformat = efm_di->di_tv.vval.v_string;
}
list_T *l = tv_list_alloc(kListLenMayKnow);
qf_info_T *const qi = qf_alloc_stack(QFLT_INTERNAL);
if (qf_init_ext(qi, 0, NULL, NULL, &di->di_tv, errorformat,
true, (linenr_T)0, (linenr_T)0, NULL, NULL) > 0) {
(void)get_errorlist(qi, NULL, 0, l);
qf_free(&qi->qf_lists[0]);
}
xfree(qi);
tv_dict_add_list(retdict, S_LEN("items"), l);
status = OK;
}
return status;
}
static int qf_winid(qf_info_T *qi)
{
if (qi == NULL) {
return 0;
}
win_T *win = qf_find_win(qi);
if (win != NULL) {
return win->handle;
}
return 0;
}
static int qf_getprop_keys2flags(const dict_T *what, bool loclist)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
int flags = QF_GETLIST_NONE;
if (tv_dict_find(what, S_LEN("all")) != NULL) {
flags |= QF_GETLIST_ALL;
if (!loclist) {
flags &= ~QF_GETLIST_FILEWINID;
}
}
if (tv_dict_find(what, S_LEN("title")) != NULL) {
flags |= QF_GETLIST_TITLE;
}
if (tv_dict_find(what, S_LEN("nr")) != NULL) {
flags |= QF_GETLIST_NR;
}
if (tv_dict_find(what, S_LEN("winid")) != NULL) {
flags |= QF_GETLIST_WINID;
}
if (tv_dict_find(what, S_LEN("context")) != NULL) {
flags |= QF_GETLIST_CONTEXT;
}
if (tv_dict_find(what, S_LEN("id")) != NULL) {
flags |= QF_GETLIST_ID;
}
if (tv_dict_find(what, S_LEN("items")) != NULL) {
flags |= QF_GETLIST_ITEMS;
}
if (tv_dict_find(what, S_LEN("idx")) != NULL) {
flags |= QF_GETLIST_IDX;
}
if (tv_dict_find(what, S_LEN("size")) != NULL) {
flags |= QF_GETLIST_SIZE;
}
if (tv_dict_find(what, S_LEN("changedtick")) != NULL) {
flags |= QF_GETLIST_TICK;
}
if (loclist && tv_dict_find(what, S_LEN("filewinid")) != NULL) {
flags |= QF_GETLIST_FILEWINID;
}
return flags;
}
static int qf_getprop_qfidx(qf_info_T *qi, dict_T *what)
{
dictitem_T *di = NULL;
int qf_idx = qi->qf_curlist; 
if ((di = tv_dict_find(what, S_LEN("nr"))) != NULL) {
if (di->di_tv.v_type == VAR_NUMBER) {
if (di->di_tv.vval.v_number != 0) {
qf_idx = (int)di->di_tv.vval.v_number - 1;
if (qf_idx < 0 || qf_idx >= qi->qf_listcount) {
qf_idx = INVALID_QFIDX;
}
}
} else if (di->di_tv.v_type == VAR_STRING
&& strequal((const char *)di->di_tv.vval.v_string, "$")) {
qf_idx = qi->qf_listcount - 1;
} else {
qf_idx = INVALID_QFIDX;
}
}
if ((di = tv_dict_find(what, S_LEN("id"))) != NULL) {
if (di->di_tv.v_type == VAR_NUMBER) {
if (di->di_tv.vval.v_number != 0) {
qf_idx = qf_id2nr(qi, (unsigned)di->di_tv.vval.v_number);
}
} else {
qf_idx = INVALID_QFIDX;
}
}
return qf_idx;
}
static int qf_getprop_defaults(qf_info_T *qi,
int flags,
int locstack,
dict_T *retdict)
{
int status = OK;
if (flags & QF_GETLIST_TITLE) {
status = tv_dict_add_str(retdict, S_LEN("title"), (const char *)"");
}
if ((status == OK) && (flags & QF_GETLIST_ITEMS)) {
list_T *l = tv_list_alloc(kListLenMayKnow);
status = tv_dict_add_list(retdict, S_LEN("items"), l);
}
if ((status == OK) && (flags & QF_GETLIST_NR)) {
status = tv_dict_add_nr(retdict, S_LEN("nr"), 0);
}
if ((status == OK) && (flags & QF_GETLIST_WINID)) {
status = tv_dict_add_nr(retdict, S_LEN("winid"), qf_winid(qi));
}
if ((status == OK) && (flags & QF_GETLIST_CONTEXT)) {
status = tv_dict_add_str(retdict, S_LEN("context"), (const char *)"");
}
if ((status == OK) && (flags & QF_GETLIST_ID)) {
status = tv_dict_add_nr(retdict, S_LEN("id"), 0);
}
if ((status == OK) && (flags & QF_GETLIST_IDX)) {
status = tv_dict_add_nr(retdict, S_LEN("idx"), 0);
}
if ((status == OK) && (flags & QF_GETLIST_SIZE)) {
status = tv_dict_add_nr(retdict, S_LEN("size"), 0);
}
if ((status == OK) && (flags & QF_GETLIST_TICK)) {
status = tv_dict_add_nr(retdict, S_LEN("changedtick"), 0);
}
if ((status == OK) && locstack && (flags & QF_GETLIST_FILEWINID)) {
status = tv_dict_add_nr(retdict, S_LEN("filewinid"), 0);
}
return status;
}
static int qf_getprop_title(qf_list_T *qfl, dict_T *retdict)
{
return tv_dict_add_str(retdict, S_LEN("title"),
(const char *)qfl->qf_title);
}
static int qf_getprop_filewinid(const win_T *wp, const qf_info_T *qi,
dict_T *retdict)
FUNC_ATTR_NONNULL_ARG(3)
{
handle_T winid = 0;
if (wp != NULL && IS_LL_WINDOW(wp)) {
win_T *ll_wp = qf_find_win_with_loclist(qi);
if (ll_wp != NULL) {
winid = ll_wp->handle;
}
}
return tv_dict_add_nr(retdict, S_LEN("filewinid"), winid);
}
static int qf_getprop_items(qf_info_T *qi, int qf_idx, dict_T *retdict)
{
list_T *l = tv_list_alloc(kListLenMayKnow);
get_errorlist(qi, NULL, qf_idx, l);
tv_dict_add_list(retdict, S_LEN("items"), l);
return OK;
}
static int qf_getprop_ctx(qf_list_T *qfl, dict_T *retdict)
{
int status;
if (qfl->qf_ctx != NULL) {
dictitem_T *di = tv_dict_item_alloc_len(S_LEN("context"));
tv_copy(qfl->qf_ctx, &di->di_tv);
status = tv_dict_add(retdict, di);
if (status == FAIL) {
tv_dict_item_free(di);
}
} else {
status = tv_dict_add_str(retdict, S_LEN("context"), "");
}
return status;
}
static int qf_getprop_idx(qf_list_T *qfl, dict_T *retdict)
{
int curidx = qfl->qf_index;
if (qf_list_empty(qfl)) {
curidx = 0;
}
return tv_dict_add_nr(retdict, S_LEN("idx"), curidx);
}
int qf_get_properties(win_T *wp, dict_T *what, dict_T *retdict)
{
qf_info_T *qi = &ql_info;
qf_list_T *qfl;
dictitem_T *di = NULL;
int status = OK;
int qf_idx = INVALID_QFIDX;
if ((di = tv_dict_find(what, S_LEN("lines"))) != NULL) {
return qf_get_list_from_lines(what, di, retdict);
}
if (wp != NULL) {
qi = GET_LOC_LIST(wp);
}
const int flags = qf_getprop_keys2flags(what, wp != NULL);
if (!qf_stack_empty(qi)) {
qf_idx = qf_getprop_qfidx(qi, what);
}
if (qf_stack_empty(qi) || qf_idx == INVALID_QFIDX) {
return qf_getprop_defaults(qi, flags, wp != NULL, retdict);
}
qfl = qf_get_list(qi, qf_idx);
if (flags & QF_GETLIST_TITLE) {
status = qf_getprop_title(qfl, retdict);
}
if ((status == OK) && (flags & QF_GETLIST_NR)) {
status = tv_dict_add_nr(retdict, S_LEN("nr"), qf_idx + 1);
}
if ((status == OK) && (flags & QF_GETLIST_WINID)) {
status = tv_dict_add_nr(retdict, S_LEN("winid"), qf_winid(qi));
}
if ((status == OK) && (flags & QF_GETLIST_ITEMS)) {
status = qf_getprop_items(qi, qf_idx, retdict);
}
if ((status == OK) && (flags & QF_GETLIST_CONTEXT)) {
status = qf_getprop_ctx(qfl, retdict);
}
if ((status == OK) && (flags & QF_GETLIST_ID)) {
status = tv_dict_add_nr(retdict, S_LEN("id"), qfl->qf_id);
}
if ((status == OK) && (flags & QF_GETLIST_IDX)) {
status = qf_getprop_idx(qfl, retdict);
}
if ((status == OK) && (flags & QF_GETLIST_SIZE)) {
status = tv_dict_add_nr(retdict, S_LEN("size"),
qfl->qf_count);
}
if ((status == OK) && (flags & QF_GETLIST_TICK)) {
status = tv_dict_add_nr(retdict, S_LEN("changedtick"),
qfl->qf_changedtick);
}
if ((status == OK) && (wp != NULL) && (flags & QF_GETLIST_FILEWINID)) {
status = qf_getprop_filewinid(wp, qi, retdict);
}
return status;
}
static int qf_add_entry_from_dict(
qf_list_T *qfl,
const dict_T *d,
bool first_entry,
bool *valid_entry)
FUNC_ATTR_NONNULL_ALL
{
static bool did_bufnr_emsg;
if (first_entry) {
did_bufnr_emsg = false;
}
char *const filename = tv_dict_get_string(d, "filename", true);
char *const module = tv_dict_get_string(d, "module", true);
int bufnum = (int)tv_dict_get_number(d, "bufnr");
const long lnum = (long)tv_dict_get_number(d, "lnum");
const int col = (int)tv_dict_get_number(d, "col");
const char_u vcol = (char_u)tv_dict_get_number(d, "vcol");
const int nr = (int)tv_dict_get_number(d, "nr");
const char *const type = tv_dict_get_string(d, "type", false);
char *const pattern = tv_dict_get_string(d, "pattern", true);
char *text = tv_dict_get_string(d, "text", true);
if (text == NULL) {
text = xcalloc(1, 1);
}
bool valid = true;
if ((filename == NULL && bufnum == 0)
|| (lnum == 0 && pattern == NULL)) {
valid = false;
}
if (bufnum != 0 && (buflist_findnr(bufnum) == NULL)) {
if (!did_bufnr_emsg) {
did_bufnr_emsg = true;
EMSGN(_("E92: Buffer %" PRId64 " not found"), bufnum);
}
valid = false;
bufnum = 0;
}
if (tv_dict_find(d, "valid", -1) != NULL) {
valid = tv_dict_get_number(d, "valid");
}
const int status = qf_add_entry(qfl,
NULL, 
(char_u *)filename,
(char_u *)module,
bufnum,
(char_u *)text,
lnum,
col,
vcol, 
(char_u *)pattern, 
nr,
(char_u)(type == NULL ? NUL : *type),
valid);
xfree(filename);
xfree(module);
xfree(pattern);
xfree(text);
if (valid) {
*valid_entry = true;
}
return status;
}
static int qf_add_entries(qf_info_T *qi, int qf_idx, list_T *list,
char_u *title, int action)
{
qf_list_T *qfl = qf_get_list(qi, qf_idx);
qfline_T *old_last = NULL;
int retval = OK;
bool valid_entry = false;
if (action == ' ' || qf_idx == qi->qf_listcount) {
qf_new_list(qi, title);
qf_idx = qi->qf_curlist;
qfl = qf_get_list(qi, qf_idx);
} else if (action == 'a' && !qf_list_empty(qfl)) {
old_last = qfl->qf_last;
} else if (action == 'r') {
qf_free_items(qfl);
qf_store_title(qfl, title);
}
TV_LIST_ITER_CONST(list, li, {
if (TV_LIST_ITEM_TV(li)->v_type != VAR_DICT) {
continue; 
}
const dict_T *const d = TV_LIST_ITEM_TV(li)->vval.v_dict;
if (d == NULL) {
continue;
}
retval = qf_add_entry_from_dict(qfl, d, li == tv_list_first(list),
&valid_entry);
if (retval == QF_FAIL) {
break;
}
});
if (valid_entry) {
qfl->qf_nonevalid = false;
} else if (qfl->qf_index == 0) {
qfl->qf_nonevalid = true;
}
if (action != 'a') {
qfl->qf_ptr = qfl->qf_start;
}
if ((action != 'a' || qfl->qf_index == 0)
&& !qf_list_empty(qfl)) {
qfl->qf_index = 1;
}
qf_update_buffer(qi, old_last);
return retval;
}
static int qf_setprop_get_qfidx(
const qf_info_T *qi,
const dict_T *what,
int action,
bool *newlist)
FUNC_ATTR_NONNULL_ALL
{
dictitem_T *di;
int qf_idx = qi->qf_curlist; 
if ((di = tv_dict_find(what, S_LEN("nr"))) != NULL) {
if (di->di_tv.v_type == VAR_NUMBER) {
if (di->di_tv.vval.v_number != 0) {
qf_idx = (int)di->di_tv.vval.v_number - 1;
}
if ((action == ' ' || action == 'a') && qf_idx == qi->qf_listcount) {
*newlist = true;
qf_idx = qf_stack_empty(qi) ? 0 : qi->qf_listcount - 1;
} else if (qf_idx < 0 || qf_idx >= qi->qf_listcount) {
return INVALID_QFIDX;
} else if (action != ' ') {
*newlist = false; 
}
} else if (di->di_tv.v_type == VAR_STRING
&& strequal((const char *)di->di_tv.vval.v_string, "$")) {
if (!qf_stack_empty(qi)) {
qf_idx = qi->qf_listcount - 1;
} else if (*newlist) {
qf_idx = 0;
} else {
return INVALID_QFIDX;
}
} else {
return INVALID_QFIDX;
}
}
if (!*newlist && (di = tv_dict_find(what, S_LEN("id"))) != NULL) {
if (di->di_tv.v_type != VAR_NUMBER) {
return INVALID_QFIDX;
}
return qf_id2nr(qi, (unsigned)di->di_tv.vval.v_number);
}
return qf_idx;
}
static int qf_setprop_title(qf_info_T *qi, int qf_idx, const dict_T *what,
const dictitem_T *di)
FUNC_ATTR_NONNULL_ALL
{
qf_list_T *qfl = qf_get_list(qi, qf_idx);
if (di->di_tv.v_type != VAR_STRING) {
return FAIL;
}
xfree(qfl->qf_title);
qfl->qf_title = (char_u *)tv_dict_get_string(what, "title", true);
if (qf_idx == qi->qf_curlist) {
qf_update_win_titlevar(qi);
}
return OK;
}
static int qf_setprop_items(qf_info_T *qi, int qf_idx, dictitem_T *di,
int action)
FUNC_ATTR_NONNULL_ALL
{
if (di->di_tv.v_type != VAR_LIST) {
return FAIL;
}
char_u *title_save = vim_strsave(qi->qf_lists[qf_idx].qf_title);
const int retval = qf_add_entries(qi, qf_idx, di->di_tv.vval.v_list,
title_save,
action == ' ' ? 'a' : action);
xfree(title_save);
return retval;
}
static int qf_setprop_items_from_lines(
qf_info_T *qi,
int qf_idx,
const dict_T *what,
dictitem_T *di,
int action)
FUNC_ATTR_NONNULL_ALL
{
char_u *errorformat = p_efm;
dictitem_T *efm_di;
int retval = FAIL;
if ((efm_di = tv_dict_find(what, S_LEN("efm"))) != NULL) {
if (efm_di->di_tv.v_type != VAR_STRING
|| efm_di->di_tv.vval.v_string == NULL) {
return FAIL;
}
errorformat = efm_di->di_tv.vval.v_string;
}
if (di->di_tv.v_type != VAR_LIST || di->di_tv.vval.v_list == NULL) {
return FAIL;
}
if (action == 'r') {
qf_free_items(&qi->qf_lists[qf_idx]);
}
if (qf_init_ext(qi, qf_idx, NULL, NULL, &di->di_tv, errorformat,
false, (linenr_T)0, (linenr_T)0, NULL, NULL) > 0) {
retval = OK;
}
return retval;
}
static int qf_setprop_context(qf_list_T *qfl, dictitem_T *di)
FUNC_ATTR_NONNULL_ALL
{
tv_free(qfl->qf_ctx);
typval_T *ctx = xcalloc(1, sizeof(typval_T));
tv_copy(&di->di_tv, ctx);
qfl->qf_ctx = ctx;
return OK;
}
static int qf_set_properties(qf_info_T *qi, const dict_T *what, int action,
char_u *title)
FUNC_ATTR_NONNULL_ALL
{
qf_list_T *qfl;
dictitem_T *di;
int retval = FAIL;
bool newlist = action == ' ' || qf_stack_empty(qi);
int qf_idx = qf_setprop_get_qfidx(qi, what, action, &newlist);
if (qf_idx == INVALID_QFIDX) { 
return FAIL;
}
if (newlist) {
qi->qf_curlist = qf_idx;
qf_new_list(qi, title);
qf_idx = qi->qf_curlist;
}
qfl = qf_get_list(qi, qf_idx);
if ((di = tv_dict_find(what, S_LEN("title"))) != NULL) {
retval = qf_setprop_title(qi, qf_idx, what, di);
}
if ((di = tv_dict_find(what, S_LEN("items"))) != NULL) {
retval = qf_setprop_items(qi, qf_idx, di, action);
}
if ((di = tv_dict_find(what, S_LEN("lines"))) != NULL) {
retval = qf_setprop_items_from_lines(qi, qf_idx, what, di, action);
}
if ((di = tv_dict_find(what, S_LEN("context"))) != NULL) {
retval = qf_setprop_context(qfl, di);
}
if (retval == OK) {
qf_list_changed(qfl);
}
return retval;
}
static win_T * find_win_with_ll(qf_info_T *qi)
{
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if ((wp->w_llist == qi) && !bt_quickfix(wp->w_buffer)) {
return wp;
}
}
return NULL;
}
static void qf_free_stack(win_T *wp, qf_info_T *qi)
{
win_T *qfwin = qf_find_win(qi);
if (qfwin != NULL) {
if (qi->qf_curlist < qi->qf_listcount) {
qf_free(qf_get_curlist(qi));
}
qf_update_buffer(qi, NULL);
}
win_T *llwin = NULL;
win_T *orig_wp = wp;
if (wp != NULL && IS_LL_WINDOW(wp)) {
llwin = find_win_with_ll(qi);
if (llwin != NULL) {
wp = llwin;
}
}
qf_free_all(wp);
if (wp == NULL) {
qi->qf_curlist = 0;
qi->qf_listcount = 0;
} else if (IS_LL_WINDOW(orig_wp)) {
qf_info_T *new_ll = qf_alloc_stack(QFLT_LOCATION);
ll_free_all(&orig_wp->w_llist_ref);
orig_wp->w_llist_ref = new_ll;
if (llwin != NULL) {
win_set_loclist(wp, new_ll);
}
}
}
int set_errorlist(win_T *wp, list_T *list, int action, char_u *title,
dict_T *what)
{
qf_info_T *qi = &ql_info;
int retval = OK;
if (wp != NULL) {
qi = ll_get_or_alloc_list(wp);
}
if (action == 'f') {
qf_free_stack(wp, qi);
return OK;
}
incr_quickfix_busy();
if (what != NULL) {
retval = qf_set_properties(qi, what, action, title);
} else {
retval = qf_add_entries(qi, qi->qf_curlist, list, title, action);
if (retval == OK) {
qf_list_changed(qf_get_curlist(qi));
}
}
decr_quickfix_busy();
return retval;
}
static bool mark_quickfix_ctx(qf_info_T *qi, int copyID)
{
bool abort = false;
for (int i = 0; i < LISTCOUNT && !abort; i++) {
typval_T *ctx = qi->qf_lists[i].qf_ctx;
if (ctx != NULL && ctx->v_type != VAR_NUMBER
&& ctx->v_type != VAR_STRING && ctx->v_type != VAR_FLOAT) {
abort = set_ref_in_item(ctx, copyID, NULL, NULL);
}
}
return abort;
}
bool set_ref_in_quickfix(int copyID)
{
bool abort = mark_quickfix_ctx(&ql_info, copyID);
if (abort) {
return abort;
}
FOR_ALL_TAB_WINDOWS(tp, win) {
if (win->w_llist != NULL) {
abort = mark_quickfix_ctx(win->w_llist, copyID);
if (abort) {
return abort;
}
}
if (IS_LL_WINDOW(win) && (win->w_llist_ref->qf_refcount == 1)) {
abort = mark_quickfix_ctx(win->w_llist_ref, copyID);
if (abort) {
return abort;
}
}
}
return abort;
}
static char_u * cbuffer_get_auname(cmdidx_T cmdidx)
{
switch (cmdidx) {
case CMD_cbuffer: return (char_u *)"cbuffer";
case CMD_cgetbuffer: return (char_u *)"cgetbuffer";
case CMD_caddbuffer: return (char_u *)"caddbuffer";
case CMD_lbuffer: return (char_u *)"lbuffer";
case CMD_lgetbuffer: return (char_u *)"lgetbuffer";
case CMD_laddbuffer: return (char_u *)"laddbuffer";
default: return NULL;
}
}
static int cbuffer_process_args(exarg_T *eap,
buf_T **bufp,
linenr_T *line1,
linenr_T *line2)
{
buf_T *buf = NULL;
if (*eap->arg == NUL)
buf = curbuf;
else if (*skipwhite(skipdigits(eap->arg)) == NUL)
buf = buflist_findnr(atoi((char *)eap->arg));
if (buf == NULL) {
EMSG(_(e_invarg));
return FAIL;
}
if (buf->b_ml.ml_mfp == NULL) {
EMSG(_("E681: Buffer is not loaded"));
return FAIL;
}
if (eap->addr_count == 0) {
eap->line1 = 1;
eap->line2 = buf->b_ml.ml_line_count;
}
if (eap->line1 < 1 || eap->line1 > buf->b_ml.ml_line_count
|| eap->line2 < 1 || eap->line2 > buf->b_ml.ml_line_count) {
EMSG(_(e_invrange));
return FAIL;
}
*line1 = eap->line1;
*line2 = eap->line2;
*bufp = buf;
return OK;
}
void ex_cbuffer(exarg_T *eap)
{
buf_T *buf = NULL;
char_u *au_name = NULL;
win_T *wp = NULL;
char_u *qf_title;
linenr_T line1;
linenr_T line2;
au_name = cbuffer_get_auname(eap->cmdidx);
if (au_name != NULL && apply_autocmds(EVENT_QUICKFIXCMDPRE, au_name,
curbuf->b_fname, true, curbuf)) {
if (aborting()) {
return;
}
}
qf_info_T *qi = qf_cmd_get_or_alloc_stack(eap, &wp);
if (cbuffer_process_args(eap, &buf, &line1, &line2) == FAIL) {
return;
}
qf_title = qf_cmdtitle(*eap->cmdlinep);
if (buf->b_sfname) {
vim_snprintf((char *)IObuff, IOSIZE, "%s (%s)",
(char *)qf_title, (char *)buf->b_sfname);
qf_title = IObuff;
}
incr_quickfix_busy();
int res = qf_init_ext(qi, qi->qf_curlist, NULL, buf, NULL, p_efm,
(eap->cmdidx != CMD_caddbuffer
&& eap->cmdidx != CMD_laddbuffer),
eap->line1, eap->line2, qf_title, NULL);
if (qf_stack_empty(qi)) {
decr_quickfix_busy();
return;
}
if (res >= 0) {
qf_list_changed(qf_get_curlist(qi));
}
unsigned save_qfid = qf_get_curlist(qi)->qf_id;
if (au_name != NULL) {
const buf_T *const curbuf_old = curbuf;
apply_autocmds(EVENT_QUICKFIXCMDPOST, au_name,
curbuf->b_fname, true, curbuf);
if (curbuf != curbuf_old) {
res = 0;
}
}
if (res > 0 && (eap->cmdidx == CMD_cbuffer || eap->cmdidx == CMD_lbuffer)
&& qflist_valid(wp, save_qfid)) {
qf_jump_first(qi, save_qfid, eap->forceit);
}
decr_quickfix_busy();
}
static char_u * cexpr_get_auname(cmdidx_T cmdidx)
{
switch (cmdidx) {
case CMD_cexpr: return (char_u *)"cexpr";
case CMD_cgetexpr: return (char_u *)"cgetexpr";
case CMD_caddexpr: return (char_u *)"caddexpr";
case CMD_lexpr: return (char_u *)"lexpr";
case CMD_lgetexpr: return (char_u *)"lgetexpr";
case CMD_laddexpr: return (char_u *)"laddexpr";
default: return NULL;
}
}
void ex_cexpr(exarg_T *eap)
{
char_u *au_name = NULL;
win_T *wp = NULL;
au_name = cexpr_get_auname(eap->cmdidx);
if (au_name != NULL && apply_autocmds(EVENT_QUICKFIXCMDPRE, au_name,
curbuf->b_fname, true, curbuf)) {
if (aborting()) {
return;
}
}
qf_info_T *qi = qf_cmd_get_or_alloc_stack(eap, &wp);
typval_T tv;
if (eval0(eap->arg, &tv, NULL, true) != FAIL) {
if ((tv.v_type == VAR_STRING && tv.vval.v_string != NULL)
|| tv.v_type == VAR_LIST) {
incr_quickfix_busy();
int res = qf_init_ext(qi, qi->qf_curlist, NULL, NULL, &tv, p_efm,
(eap->cmdidx != CMD_caddexpr
&& eap->cmdidx != CMD_laddexpr),
(linenr_T)0, (linenr_T)0,
qf_cmdtitle(*eap->cmdlinep), NULL);
if (qf_stack_empty(qi)) {
decr_quickfix_busy();
goto cleanup;
}
if (res >= 0) {
qf_list_changed(qf_get_curlist(qi));
}
unsigned save_qfid = qf_get_curlist(qi)->qf_id;
if (au_name != NULL) {
apply_autocmds(EVENT_QUICKFIXCMDPOST, au_name,
curbuf->b_fname, true, curbuf);
}
if (res > 0
&& (eap->cmdidx == CMD_cexpr || eap->cmdidx == CMD_lexpr)
&& qflist_valid(wp, save_qfid)) {
qf_jump_first(qi, save_qfid, eap->forceit);
}
decr_quickfix_busy();
} else {
EMSG(_("E777: String or List expected"));
}
cleanup:
tv_clear(&tv);
}
}
static qf_info_T *hgr_get_ll(bool *new_ll)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_NONNULL_RET
{
win_T *wp;
qf_info_T *qi;
if (bt_help(curwin->w_buffer)) {
wp = curwin;
} else {
wp = qf_find_help_win();
}
if (wp == NULL) { 
qi = NULL;
} else {
qi = wp->w_llist;
}
if (qi == NULL) {
qi = qf_alloc_stack(QFLT_LOCATION);
*new_ll = true;
}
return qi;
}
static void hgr_search_file(
qf_info_T *qi,
char_u *fname,
regmatch_T *p_regmatch)
FUNC_ATTR_NONNULL_ARG(1, 3)
{
FILE *const fd = os_fopen((char *)fname, "r");
if (fd == NULL) {
return;
}
long lnum = 1;
while (!vim_fgets(IObuff, IOSIZE, fd) && !got_int) {
char_u *line = IObuff;
if (vim_regexec(p_regmatch, line, (colnr_T)0)) {
int l = (int)STRLEN(line);
while (l > 0 && line[l - 1] <= ' ') {
line[--l] = NUL;
}
if (qf_add_entry(qf_get_curlist(qi),
NULL, 
fname,
NULL,
0,
line,
lnum,
(int)(p_regmatch->startp[0] - line) + 1, 
false, 
NULL, 
0, 
1, 
true) 
== QF_FAIL) {
got_int = true;
if (line != IObuff) {
xfree(line);
}
break;
}
}
if (line != IObuff) {
xfree(line);
}
lnum++;
line_breakcheck();
}
fclose(fd);
}
static void hgr_search_files_in_dir(
qf_info_T *qi,
char_u *dirname,
regmatch_T *p_regmatch,
const char_u *lang)
FUNC_ATTR_NONNULL_ARG(1, 2, 3)
{
int fcount;
char_u **fnames;
add_pathsep((char *)dirname);
STRCAT(dirname, "doc/*.\\(txt\\|??x\\)"); 
if (gen_expand_wildcards(1, &dirname, &fcount,
&fnames, EW_FILE|EW_SILENT) == OK
&& fcount > 0) {
for (int fi = 0; fi < fcount && !got_int; fi++) {
if (lang != NULL
&& STRNICMP(lang, fnames[fi] + STRLEN(fnames[fi]) - 3, 2) != 0
&& !(STRNICMP(lang, "en", 2) == 0
&& STRNICMP("txt", fnames[fi] + STRLEN(fnames[fi]) - 3, 3)
== 0)) {
continue;
}
hgr_search_file(qi, fnames[fi], p_regmatch);
}
FreeWild(fcount, fnames);
}
}
static void hgr_search_in_rtp(qf_info_T *qi, regmatch_T *p_regmatch,
const char_u *lang)
FUNC_ATTR_NONNULL_ARG(1, 2)
{
char_u *p = p_rtp;
while (*p != NUL && !got_int) {
copy_option_part(&p, NameBuff, MAXPATHL, ",");
hgr_search_files_in_dir(qi, NameBuff, p_regmatch, lang);
}
}
void ex_helpgrep(exarg_T *eap)
{
qf_info_T *qi = &ql_info;
bool new_qi = false;
char_u *au_name = NULL;
switch (eap->cmdidx) {
case CMD_helpgrep: au_name = (char_u *)"helpgrep"; break;
case CMD_lhelpgrep: au_name = (char_u *)"lhelpgrep"; break;
default: break;
}
if (au_name != NULL && apply_autocmds(EVENT_QUICKFIXCMDPRE, au_name,
curbuf->b_fname, true, curbuf)) {
if (aborting()) {
return;
}
}
char_u *const save_cpo = p_cpo;
p_cpo = empty_option;
if (is_loclist_cmd(eap->cmdidx)) {
qi = hgr_get_ll(&new_qi);
}
incr_quickfix_busy();
char_u *const lang = check_help_lang(eap->arg);
regmatch_T regmatch = {
.regprog = vim_regcomp(eap->arg, RE_MAGIC + RE_STRING),
.rm_ic = false,
};
if (regmatch.regprog != NULL) {
qf_new_list(qi, qf_cmdtitle(*eap->cmdlinep));
hgr_search_in_rtp(qi, &regmatch, lang);
vim_regfree(regmatch.regprog);
qf_list_T *qfl = qf_get_curlist(qi);
qfl->qf_nonevalid = false;
qfl->qf_ptr = qfl->qf_start;
qfl->qf_index = 1;
qf_list_changed(qfl);
qf_update_buffer(qi, NULL);
}
if (p_cpo == empty_option) {
p_cpo = save_cpo;
} else {
free_string_option(save_cpo);
}
if (au_name != NULL) {
apply_autocmds(EVENT_QUICKFIXCMDPOST, au_name,
curbuf->b_fname, true, curbuf);
if (!new_qi && IS_LL_STACK(qi) && qf_find_buf(qi) == NULL) {
decr_quickfix_busy();
return;
}
}
if (!qf_list_empty(qf_get_curlist(qi))) {
qf_jump(qi, 0, 0, false);
} else {
EMSG2(_(e_nomatch2), eap->arg);
}
decr_quickfix_busy();
if (eap->cmdidx == CMD_lhelpgrep) {
if (!bt_help(curwin->w_buffer) || curwin->w_llist == qi) {
if (new_qi) {
ll_free_all(&qi);
}
} else if (curwin->w_llist == NULL) {
curwin->w_llist = qi;
}
}
}
