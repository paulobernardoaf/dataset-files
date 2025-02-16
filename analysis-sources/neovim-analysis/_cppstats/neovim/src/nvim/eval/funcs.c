#include <float.h>
#include <math.h>
#include "nvim/api/private/helpers.h"
#include "nvim/api/vim.h"
#include "nvim/ascii.h"
#include "nvim/assert.h"
#include "nvim/buffer.h"
#include "nvim/change.h"
#include "nvim/channel.h"
#include "nvim/charset.h"
#include "nvim/context.h"
#include "nvim/cursor.h"
#include "nvim/diff.h"
#include "nvim/edit.h"
#include "nvim/eval.h"
#include "nvim/eval/decode.h"
#include "nvim/eval/encode.h"
#include "nvim/eval/executor.h"
#include "nvim/eval/funcs.h"
#include "nvim/ex_cmds2.h"
#include "nvim/ex_docmd.h"
#include "nvim/ex_getln.h"
#include "nvim/file_search.h"
#include "nvim/fileio.h"
#include "nvim/fold.h"
#include "nvim/if_cscope.h"
#include "nvim/indent.h"
#include "nvim/indent_c.h"
#include "nvim/lua/executor.h"
#include "nvim/mark.h"
#include "nvim/math.h"
#include "nvim/memline.h"
#include "nvim/misc1.h"
#include "nvim/mouse.h"
#include "nvim/move.h"
#include "nvim/msgpack_rpc/channel.h"
#include "nvim/msgpack_rpc/server.h"
#include "nvim/ops.h"
#include "nvim/option.h"
#include "nvim/os/dl.h"
#include "nvim/os/input.h"
#include "nvim/os/shell.h"
#include "nvim/path.h"
#include "nvim/popupmnu.h"
#include "nvim/quickfix.h"
#include "nvim/regexp.h"
#include "nvim/screen.h"
#include "nvim/search.h"
#include "nvim/sha256.h"
#include "nvim/sign.h"
#include "nvim/spell.h"
#include "nvim/state.h"
#include "nvim/syntax.h"
#include "nvim/tag.h"
#include "nvim/ui.h"
#include "nvim/undo.h"
#include "nvim/version.h"
#include "nvim/vim.h"
typedef enum {
kSomeMatch, 
kSomeMatchEnd, 
kSomeMatchList, 
kSomeMatchStr, 
kSomeMatchStrPos, 
} SomeMatchType;
KHASH_MAP_INIT_STR(functions, VimLFuncDef)
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "eval/funcs.c.generated.h"
#if defined(_MSC_VER)
#pragma function(ceil)
#pragma function(floor)
#endif
PRAGMA_DIAG_PUSH_IGNORE_MISSING_PROTOTYPES
#include "funcs.generated.h"
PRAGMA_DIAG_POP
#endif
static char *e_listarg = N_("E686: Argument of %s must be a List");
static char *e_stringreq = N_("E928: String required");
static va_list dummy_ap;
char_u *get_function_name(expand_T *xp, int idx)
{
static int intidx = -1;
char_u *name;
if (idx == 0)
intidx = -1;
if (intidx < 0) {
name = get_user_func_name(xp, idx);
if (name != NULL)
return name;
}
while ((size_t)++intidx < ARRAY_SIZE(functions)
&& functions[intidx].name[0] == '\0') {
}
if ((size_t)intidx >= ARRAY_SIZE(functions)) {
return NULL;
}
const char *const key = functions[intidx].name;
const size_t key_len = strlen(key);
memcpy(IObuff, key, key_len);
IObuff[key_len] = '(';
if (functions[intidx].max_argc == 0) {
IObuff[key_len + 1] = ')';
IObuff[key_len + 2] = NUL;
} else {
IObuff[key_len + 1] = NUL;
}
return IObuff;
}
char_u *get_expr_name(expand_T *xp, int idx)
{
static int intidx = -1;
char_u *name;
if (idx == 0)
intidx = -1;
if (intidx < 0) {
name = get_function_name(xp, idx);
if (name != NULL)
return name;
}
return get_user_var_name(xp, ++intidx);
}
const VimLFuncDef *find_internal_func(const char *const name)
FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_PURE FUNC_ATTR_NONNULL_ALL
{
size_t len = strlen(name);
return find_internal_func_gperf(name, len);
}
static int non_zero_arg(typval_T *argvars)
{
return ((argvars[0].v_type == VAR_NUMBER
&& argvars[0].vval.v_number != 0)
|| (argvars[0].v_type == VAR_SPECIAL
&& argvars[0].vval.v_special == kSpecialVarTrue)
|| (argvars[0].v_type == VAR_STRING
&& argvars[0].vval.v_string != NULL
&& *argvars[0].vval.v_string != NUL));
}
static void float_op_wrapper(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
float_T f;
float_T (*function)(float_T) = (float_T (*)(float_T))fptr;
rettv->v_type = VAR_FLOAT;
if (tv_get_float_chk(argvars, &f)) {
rettv->vval.v_float = function(f);
} else {
rettv->vval.v_float = 0.0;
}
}
static void api_wrapper(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
if (check_restricted() || check_secure()) {
return;
}
ApiDispatchWrapper fn = (ApiDispatchWrapper)fptr;
Array args = ARRAY_DICT_INIT;
for (typval_T *tv = argvars; tv->v_type != VAR_UNKNOWN; tv++) {
ADD(args, vim_to_object(tv));
}
Error err = ERROR_INIT;
Object result = fn(VIML_INTERNAL_CALL, args, &err);
if (ERROR_SET(&err)) {
emsgf_multiline((const char *)e_api_error, err.msg);
goto end;
}
if (!object_to_vim(result, rettv, &err)) {
EMSG2(_("Error converting the call result: %s"), err.msg);
}
end:
api_free_array(args);
api_free_object(result);
api_clear_error(&err);
}
static void f_abs(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
if (argvars[0].v_type == VAR_FLOAT) {
float_op_wrapper(argvars, rettv, (FunPtr)&fabs);
} else {
varnumber_T n;
bool error = false;
n = tv_get_number_chk(&argvars[0], &error);
if (error) {
rettv->vval.v_number = -1;
} else if (n > 0) {
rettv->vval.v_number = n;
} else {
rettv->vval.v_number = -n;
}
}
}
static void f_add(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = 1; 
if (argvars[0].v_type == VAR_LIST) {
list_T *const l = argvars[0].vval.v_list;
if (!tv_check_lock(tv_list_locked(l), N_("add() argument"), TV_TRANSLATE)) {
tv_list_append_tv(l, &argvars[1]);
tv_copy(&argvars[0], rettv);
}
} else {
EMSG(_(e_listreq));
}
}
static void f_and(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = tv_get_number_chk(&argvars[0], NULL)
& tv_get_number_chk(&argvars[1], NULL);
}
static void f_api_info(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
Dictionary metadata = api_metadata();
(void)object_to_vim(DICTIONARY_OBJ(metadata), rettv, NULL);
api_free_dictionary(metadata);
}
static void f_append(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const linenr_T lnum = tv_get_lnum(&argvars[0]);
set_buffer_lines(curbuf, lnum, true, &argvars[1], rettv);
}
static void f_appendbufline(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
buf_T *const buf = tv_get_buf(&argvars[0], false);
if (buf == NULL) {
rettv->vval.v_number = 1; 
} else {
const linenr_T lnum = tv_get_lnum_buf(&argvars[1], buf);
set_buffer_lines(buf, lnum, true, &argvars[2], rettv);
}
}
static void f_argc(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
if (argvars[0].v_type == VAR_UNKNOWN) {
rettv->vval.v_number = ARGCOUNT;
} else if (argvars[0].v_type == VAR_NUMBER
&& tv_get_number(&argvars[0]) == -1) {
rettv->vval.v_number = GARGCOUNT;
} else {
win_T *wp = find_win_by_nr_or_id(&argvars[0]);
if (wp != NULL) {
rettv->vval.v_number = WARGCOUNT(wp);
} else {
rettv->vval.v_number = -1;
}
}
}
static void f_argidx(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = curwin->w_arg_idx;
}
static void f_arglistid(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = -1;
win_T *wp = find_tabwin(&argvars[0], &argvars[1]);
if (wp != NULL) {
rettv->vval.v_number = wp->w_alist->id;
}
}
static void f_argv(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
aentry_T *arglist = NULL;
int argcount = -1;
if (argvars[0].v_type != VAR_UNKNOWN) {
if (argvars[1].v_type == VAR_UNKNOWN) {
arglist = ARGLIST;
argcount = ARGCOUNT;
} else if (argvars[1].v_type == VAR_NUMBER
&& tv_get_number(&argvars[1]) == -1) {
arglist = GARGLIST;
argcount = GARGCOUNT;
} else {
win_T *wp = find_win_by_nr_or_id(&argvars[1]);
if (wp != NULL) {
arglist = WARGLIST(wp);
argcount = WARGCOUNT(wp);
}
}
rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;
int idx = tv_get_number_chk(&argvars[0], NULL);
if (arglist != NULL && idx >= 0 && idx < argcount) {
rettv->vval.v_string = (char_u *)xstrdup(
(const char *)alist_name(&arglist[idx]));
} else if (idx == -1) {
get_arglist_as_rettv(arglist, argcount, rettv);
}
} else {
get_arglist_as_rettv(ARGLIST, ARGCOUNT, rettv);
}
}
static void f_assert_beeps(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const char *const cmd = tv_get_string_chk(&argvars[0]);
garray_T ga;
int ret = 0;
called_vim_beep = false;
suppress_errthrow = true;
emsg_silent = false;
do_cmdline_cmd(cmd);
if (!called_vim_beep) {
prepare_assert_error(&ga);
ga_concat(&ga, (const char_u *)"command did not beep: ");
ga_concat(&ga, (const char_u *)cmd);
assert_error(&ga);
ga_clear(&ga);
ret = 1;
}
suppress_errthrow = false;
emsg_on_display = false;
rettv->vval.v_number = ret;
}
static void f_assert_equal(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = assert_equal_common(argvars, ASSERT_EQUAL);
}
static void f_assert_equalfile(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = assert_equalfile(argvars);
}
static void f_assert_notequal(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = assert_equal_common(argvars, ASSERT_NOTEQUAL);
}
static void f_assert_report(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
garray_T ga;
prepare_assert_error(&ga);
ga_concat(&ga, (const char_u *)tv_get_string(&argvars[0]));
assert_error(&ga);
ga_clear(&ga);
rettv->vval.v_number = 1;
}
static void f_assert_exception(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = assert_exception(argvars);
}
static void f_assert_fails(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = assert_fails(argvars);
}
static void f_assert_false(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = assert_bool(argvars, false);
}
static void f_assert_inrange(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = assert_inrange(argvars);
}
static void f_assert_match(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = assert_match_common(argvars, ASSERT_MATCH);
}
static void f_assert_notmatch(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = assert_match_common(argvars, ASSERT_NOTMATCH);
}
static void f_assert_true(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = assert_bool(argvars, true);
}
static void f_atan2(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
float_T fx;
float_T fy;
rettv->v_type = VAR_FLOAT;
if (tv_get_float_chk(argvars, &fx) && tv_get_float_chk(&argvars[1], &fy)) {
rettv->vval.v_float = atan2(fx, fy);
} else {
rettv->vval.v_float = 0.0;
}
}
static void f_browse(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_string = NULL;
rettv->v_type = VAR_STRING;
}
static void f_browsedir(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
f_browse(argvars, rettv, NULL);
}
static buf_T *find_buffer(typval_T *avar)
{
buf_T *buf = NULL;
if (avar->v_type == VAR_NUMBER)
buf = buflist_findnr((int)avar->vval.v_number);
else if (avar->v_type == VAR_STRING && avar->vval.v_string != NULL) {
buf = buflist_findname_exp(avar->vval.v_string);
if (buf == NULL) {
FOR_ALL_BUFFERS(bp) {
if (bp->b_fname != NULL
&& (path_with_url((char *)bp->b_fname)
|| bt_nofile(bp)
)
&& STRCMP(bp->b_fname, avar->vval.v_string) == 0) {
buf = bp;
break;
}
}
}
}
return buf;
}
static void f_bufadd(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
char_u *name = (char_u *)tv_get_string(&argvars[0]);
rettv->vval.v_number = buflist_add(*name == NUL ? NULL : name, 0);
}
static void f_bufexists(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = (find_buffer(&argvars[0]) != NULL);
}
static void f_buflisted(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
buf_T *buf;
buf = find_buffer(&argvars[0]);
rettv->vval.v_number = (buf != NULL && buf->b_p_bl);
}
static void f_bufload(typval_T *argvars, typval_T *unused, FunPtr fptr)
{
buf_T *buf = get_buf_arg(&argvars[0]);
if (buf != NULL && buf->b_ml.ml_mfp == NULL) {
aco_save_T aco;
aucmd_prepbuf(&aco, buf);
swap_exists_action = SEA_NONE;
open_buffer(false, NULL, 0);
aucmd_restbuf(&aco);
}
}
static void f_bufloaded(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
buf_T *buf;
buf = find_buffer(&argvars[0]);
rettv->vval.v_number = (buf != NULL && buf->b_ml.ml_mfp != NULL);
}
static void f_bufname(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const buf_T *buf;
rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;
if (argvars[0].v_type == VAR_UNKNOWN) {
buf = curbuf;
} else {
if (!tv_check_str_or_nr(&argvars[0])) {
return;
}
emsg_off++;
buf = tv_get_buf(&argvars[0], false);
emsg_off--;
}
if (buf != NULL && buf->b_fname != NULL) {
rettv->vval.v_string = (char_u *)xstrdup((char *)buf->b_fname);
}
}
static void f_bufnr(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const buf_T *buf;
bool error = false;
rettv->vval.v_number = -1;
if (argvars[0].v_type == VAR_UNKNOWN) {
buf = curbuf;
} else {
if (!tv_check_str_or_nr(&argvars[0])) {
return;
}
emsg_off++;
buf = tv_get_buf(&argvars[0], false);
emsg_off--;
}
const char *name;
if (buf == NULL
&& argvars[1].v_type != VAR_UNKNOWN
&& tv_get_number_chk(&argvars[1], &error) != 0
&& !error
&& (name = tv_get_string_chk(&argvars[0])) != NULL) {
buf = buflist_new((char_u *)name, NULL, 1, 0);
}
if (buf != NULL) {
rettv->vval.v_number = buf->b_fnum;
}
}
static void buf_win_common(typval_T *argvars, typval_T *rettv, bool get_nr)
{
if (!tv_check_str_or_nr(&argvars[0])) {
rettv->vval.v_number = -1;
return;
}
emsg_off++;
buf_T *buf = tv_get_buf(&argvars[0], true);
if (buf == NULL) { 
rettv->vval.v_number = -1;
goto end;
}
int winnr = 0;
int winid;
bool found_buf = false;
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
winnr++;
if (wp->w_buffer == buf) {
found_buf = true;
winid = wp->handle;
break;
}
}
rettv->vval.v_number = (found_buf ? (get_nr ? winnr : winid) : -1);
end:
emsg_off--;
}
static void f_bufwinid(typval_T *argvars, typval_T *rettv, FunPtr fptr) {
buf_win_common(argvars, rettv, false);
}
static void f_bufwinnr(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
buf_win_common(argvars, rettv, true);
}
buf_T *tv_get_buf(typval_T *tv, int curtab_only)
{
char_u *name = tv->vval.v_string;
int save_magic;
char_u *save_cpo;
buf_T *buf;
if (tv->v_type == VAR_NUMBER)
return buflist_findnr((int)tv->vval.v_number);
if (tv->v_type != VAR_STRING)
return NULL;
if (name == NULL || *name == NUL)
return curbuf;
if (name[0] == '$' && name[1] == NUL)
return lastbuf;
save_magic = p_magic;
p_magic = TRUE;
save_cpo = p_cpo;
p_cpo = (char_u *)"";
buf = buflist_findnr(buflist_findpat(name, name + STRLEN(name),
TRUE, FALSE, curtab_only));
p_magic = save_magic;
p_cpo = save_cpo;
if (buf == NULL) {
buf = find_buffer(tv);
}
return buf;
}
buf_T * get_buf_arg(typval_T *arg)
{
buf_T *buf;
emsg_off++;
buf = tv_get_buf(arg, false);
emsg_off--;
if (buf == NULL) {
EMSG2(_("E158: Invalid buffer name: %s"), tv_get_string(arg));
}
return buf;
}
static void f_byte2line(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
long boff = tv_get_number(&argvars[0]) - 1;
if (boff < 0) {
rettv->vval.v_number = -1;
} else {
rettv->vval.v_number = (varnumber_T)ml_find_line_or_offset(curbuf, 0,
&boff, false);
}
}
static void byteidx(typval_T *argvars, typval_T *rettv, int comp)
{
const char *const str = tv_get_string_chk(&argvars[0]);
varnumber_T idx = tv_get_number_chk(&argvars[1], NULL);
rettv->vval.v_number = -1;
if (str == NULL || idx < 0) {
return;
}
const char *t = str;
for (; idx > 0; idx--) {
if (*t == NUL) { 
return;
}
if (enc_utf8 && comp) {
t += utf_ptr2len((const char_u *)t);
} else {
t += (*mb_ptr2len)((const char_u *)t);
}
}
rettv->vval.v_number = (varnumber_T)(t - str);
}
static void f_byteidx(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
byteidx(argvars, rettv, FALSE);
}
static void f_byteidxcomp(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
byteidx(argvars, rettv, TRUE);
}
static void f_call(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
if (argvars[1].v_type != VAR_LIST) {
EMSG(_(e_listreq));
return;
}
if (argvars[1].vval.v_list == NULL) {
return;
}
char_u *func;
partial_T *partial = NULL;
dict_T *selfdict = NULL;
if (argvars[0].v_type == VAR_FUNC) {
func = argvars[0].vval.v_string;
} else if (argvars[0].v_type == VAR_PARTIAL) {
partial = argvars[0].vval.v_partial;
func = partial_name(partial);
} else {
func = (char_u *)tv_get_string(&argvars[0]);
}
if (*func == NUL) {
return; 
}
if (argvars[2].v_type != VAR_UNKNOWN) {
if (argvars[2].v_type != VAR_DICT) {
EMSG(_(e_dictreq));
return;
}
selfdict = argvars[2].vval.v_dict;
}
func_call(func, &argvars[1], partial, selfdict, rettv);
}
static void f_changenr(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = curbuf->b_u_seq_cur;
}
static void f_chanclose(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = 0;
if (check_restricted() || check_secure()) {
return;
}
if (argvars[0].v_type != VAR_NUMBER || (argvars[1].v_type != VAR_STRING
&& argvars[1].v_type != VAR_UNKNOWN)) {
EMSG(_(e_invarg));
return;
}
ChannelPart part = kChannelPartAll;
if (argvars[1].v_type == VAR_STRING) {
char *stream = (char *)argvars[1].vval.v_string;
if (!strcmp(stream, "stdin")) {
part = kChannelPartStdin;
} else if (!strcmp(stream, "stdout")) {
part = kChannelPartStdout;
} else if (!strcmp(stream, "stderr")) {
part = kChannelPartStderr;
} else if (!strcmp(stream, "rpc")) {
part = kChannelPartRpc;
} else {
EMSG2(_("Invalid channel stream \"%s\""), stream);
return;
}
}
const char *error;
rettv->vval.v_number = channel_close(argvars[0].vval.v_number, part, &error);
if (!rettv->vval.v_number) {
EMSG(error);
}
}
static void f_chansend(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = 0;
if (check_restricted() || check_secure()) {
return;
}
if (argvars[0].v_type != VAR_NUMBER || argvars[1].v_type == VAR_UNKNOWN) {
EMSG(_(e_invarg));
return;
}
ptrdiff_t input_len = 0;
char *input = save_tv_as_string(&argvars[1], &input_len, false);
if (!input) {
return;
}
uint64_t id = argvars[0].vval.v_number;
const char *error = NULL;
rettv->vval.v_number = channel_send(id, input, input_len, &error);
if (error) {
EMSG(error);
}
}
static void f_char2nr(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
if (argvars[1].v_type != VAR_UNKNOWN) {
if (!tv_check_num(&argvars[1])) {
return;
}
}
rettv->vval.v_number = utf_ptr2char(
(const char_u *)tv_get_string(&argvars[0]));
}
static void f_cindent(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
pos_T pos;
linenr_T lnum;
pos = curwin->w_cursor;
lnum = tv_get_lnum(argvars);
if (lnum >= 1 && lnum <= curbuf->b_ml.ml_line_count) {
curwin->w_cursor.lnum = lnum;
rettv->vval.v_number = get_c_indent();
curwin->w_cursor = pos;
} else
rettv->vval.v_number = -1;
}
static void f_clearmatches(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
clear_matches(curwin);
}
static void f_col(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
colnr_T col = 0;
pos_T *fp;
int fnum = curbuf->b_fnum;
fp = var2fpos(&argvars[0], FALSE, &fnum);
if (fp != NULL && fnum == curbuf->b_fnum) {
if (fp->col == MAXCOL) {
if (fp->lnum <= curbuf->b_ml.ml_line_count)
col = (colnr_T)STRLEN(ml_get(fp->lnum)) + 1;
else
col = MAXCOL;
} else {
col = fp->col + 1;
if (virtual_active() && fp == &curwin->w_cursor) {
char_u *p = get_cursor_pos_ptr();
if (curwin->w_cursor.coladd >= (colnr_T)chartabsize(p,
curwin->w_virtcol - curwin->w_cursor.coladd)) {
int l;
if (*p != NUL && p[(l = (*mb_ptr2len)(p))] == NUL)
col += l;
}
}
}
}
rettv->vval.v_number = col;
}
static void f_complete(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
if ((State & INSERT) == 0) {
EMSG(_("E785: complete() can only be used in Insert mode"));
return;
}
if (!undo_allowed())
return;
if (argvars[1].v_type != VAR_LIST) {
EMSG(_(e_invarg));
return;
}
const colnr_T startcol = tv_get_number_chk(&argvars[0], NULL);
if (startcol <= 0) {
return;
}
set_completion(startcol - 1, argvars[1].vval.v_list);
}
static void f_complete_add(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = ins_compl_add_tv(&argvars[0], 0);
}
static void f_complete_check(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
int saved = RedrawingDisabled;
RedrawingDisabled = 0;
ins_compl_check_keys(0, true);
rettv->vval.v_number = compl_interrupted;
RedrawingDisabled = saved;
}
static void f_complete_info(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
tv_dict_alloc_ret(rettv);
list_T *what_list = NULL;
if (argvars[0].v_type != VAR_UNKNOWN) {
if (argvars[0].v_type != VAR_LIST) {
EMSG(_(e_listreq));
return;
}
what_list = argvars[0].vval.v_list;
}
get_complete_info(what_list, rettv->vval.v_dict);
}
static void f_confirm(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
char buf[NUMBUFLEN];
char buf2[NUMBUFLEN];
const char *message;
const char *buttons = NULL;
int def = 1;
int type = VIM_GENERIC;
const char *typestr;
bool error = false;
message = tv_get_string_chk(&argvars[0]);
if (message == NULL) {
error = true;
}
if (argvars[1].v_type != VAR_UNKNOWN) {
buttons = tv_get_string_buf_chk(&argvars[1], buf);
if (buttons == NULL) {
error = true;
}
if (argvars[2].v_type != VAR_UNKNOWN) {
def = tv_get_number_chk(&argvars[2], &error);
if (argvars[3].v_type != VAR_UNKNOWN) {
typestr = tv_get_string_buf_chk(&argvars[3], buf2);
if (typestr == NULL) {
error = true;
} else {
switch (TOUPPER_ASC(*typestr)) {
case 'E': type = VIM_ERROR; break;
case 'Q': type = VIM_QUESTION; break;
case 'I': type = VIM_INFO; break;
case 'W': type = VIM_WARNING; break;
case 'G': type = VIM_GENERIC; break;
}
}
}
}
}
if (buttons == NULL || *buttons == NUL) {
buttons = _("&Ok");
}
if (!error) {
rettv->vval.v_number = do_dialog(
type, NULL, (char_u *)message, (char_u *)buttons, def, NULL, false);
}
}
static void f_copy(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
var_item_copy(NULL, &argvars[0], rettv, false, 0);
}
static void f_count(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
long n = 0;
int ic = 0;
bool error = false;
if (argvars[2].v_type != VAR_UNKNOWN) {
ic = tv_get_number_chk(&argvars[2], &error);
}
if (argvars[0].v_type == VAR_STRING) {
const char_u *expr = (char_u *)tv_get_string_chk(&argvars[1]);
const char_u *p = argvars[0].vval.v_string;
if (!error && expr != NULL && *expr != NUL && p != NULL) {
if (ic) {
const size_t len = STRLEN(expr);
while (*p != NUL) {
if (mb_strnicmp(p, expr, len) == 0) {
n++;
p += len;
} else {
MB_PTR_ADV(p);
}
}
} else {
char_u *next;
while ((next = (char_u *)strstr((char *)p, (char *)expr)) != NULL) {
n++;
p = next + STRLEN(expr);
}
}
}
} else if (argvars[0].v_type == VAR_LIST) {
listitem_T *li;
list_T *l;
long idx;
if ((l = argvars[0].vval.v_list) != NULL) {
li = tv_list_first(l);
if (argvars[2].v_type != VAR_UNKNOWN) {
if (argvars[3].v_type != VAR_UNKNOWN) {
idx = tv_get_number_chk(&argvars[3], &error);
if (!error) {
li = tv_list_find(l, idx);
if (li == NULL) {
EMSGN(_(e_listidx), idx);
}
}
}
if (error)
li = NULL;
}
for (; li != NULL; li = TV_LIST_ITEM_NEXT(l, li)) {
if (tv_equal(TV_LIST_ITEM_TV(li), &argvars[1], ic, false)) {
n++;
}
}
}
} else if (argvars[0].v_type == VAR_DICT) {
int todo;
dict_T *d;
hashitem_T *hi;
if ((d = argvars[0].vval.v_dict) != NULL) {
if (argvars[2].v_type != VAR_UNKNOWN) {
if (argvars[3].v_type != VAR_UNKNOWN) {
EMSG(_(e_invarg));
}
}
todo = error ? 0 : (int)d->dv_hashtab.ht_used;
for (hi = d->dv_hashtab.ht_array; todo > 0; ++hi) {
if (!HASHITEM_EMPTY(hi)) {
todo--;
if (tv_equal(&TV_DICT_HI2DI(hi)->di_tv, &argvars[1], ic, false)) {
n++;
}
}
}
}
} else {
EMSG2(_(e_listdictarg), "count()");
}
rettv->vval.v_number = n;
}
static void f_cscope_connection(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
int num = 0;
const char *dbpath = NULL;
const char *prepend = NULL;
char buf[NUMBUFLEN];
if (argvars[0].v_type != VAR_UNKNOWN
&& argvars[1].v_type != VAR_UNKNOWN) {
num = (int)tv_get_number(&argvars[0]);
dbpath = tv_get_string(&argvars[1]);
if (argvars[2].v_type != VAR_UNKNOWN) {
prepend = tv_get_string_buf(&argvars[2], buf);
}
}
rettv->vval.v_number = cs_connection(num, (char_u *)dbpath,
(char_u *)prepend);
}
static void f_ctxget(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
size_t index = 0;
if (argvars[0].v_type == VAR_NUMBER) {
index = argvars[0].vval.v_number;
} else if (argvars[0].v_type != VAR_UNKNOWN) {
EMSG2(_(e_invarg2), "expected nothing or a Number as an argument");
return;
}
Context *ctx = ctx_get(index);
if (ctx == NULL) {
EMSG3(_(e_invargNval), "index", "out of bounds");
return;
}
Dictionary ctx_dict = ctx_to_dict(ctx);
Error err = ERROR_INIT;
object_to_vim(DICTIONARY_OBJ(ctx_dict), rettv, &err);
api_free_dictionary(ctx_dict);
api_clear_error(&err);
}
static void f_ctxpop(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
if (!ctx_restore(NULL, kCtxAll)) {
EMSG(_("Context stack is empty"));
}
}
static void f_ctxpush(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
int types = kCtxAll;
if (argvars[0].v_type == VAR_LIST) {
types = 0;
TV_LIST_ITER(argvars[0].vval.v_list, li, {
typval_T *tv_li = TV_LIST_ITEM_TV(li);
if (tv_li->v_type == VAR_STRING) {
if (strequal((char *)tv_li->vval.v_string, "regs")) {
types |= kCtxRegs;
} else if (strequal((char *)tv_li->vval.v_string, "jumps")) {
types |= kCtxJumps;
} else if (strequal((char *)tv_li->vval.v_string, "bufs")) {
types |= kCtxBufs;
} else if (strequal((char *)tv_li->vval.v_string, "gvars")) {
types |= kCtxGVars;
} else if (strequal((char *)tv_li->vval.v_string, "sfuncs")) {
types |= kCtxSFuncs;
} else if (strequal((char *)tv_li->vval.v_string, "funcs")) {
types |= kCtxFuncs;
}
}
});
} else if (argvars[0].v_type != VAR_UNKNOWN) {
EMSG2(_(e_invarg2), "expected nothing or a List as an argument");
return;
}
ctx_save(NULL, types);
}
static void f_ctxset(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
if (argvars[0].v_type != VAR_DICT) {
EMSG2(_(e_invarg2), "expected dictionary as first argument");
return;
}
size_t index = 0;
if (argvars[1].v_type == VAR_NUMBER) {
index = argvars[1].vval.v_number;
} else if (argvars[1].v_type != VAR_UNKNOWN) {
EMSG2(_(e_invarg2), "expected nothing or a Number as second argument");
return;
}
Context *ctx = ctx_get(index);
if (ctx == NULL) {
EMSG3(_(e_invargNval), "index", "out of bounds");
return;
}
int save_did_emsg = did_emsg;
did_emsg = false;
Dictionary dict = vim_to_object(&argvars[0]).data.dictionary;
Context tmp = CONTEXT_INIT;
ctx_from_dict(dict, &tmp);
if (did_emsg) {
ctx_free(&tmp);
} else {
ctx_free(ctx);
*ctx = tmp;
}
api_free_dictionary(dict);
did_emsg = save_did_emsg;
}
static void f_ctxsize(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = ctx_size();
}
static void f_cursor(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
long line, col;
long coladd = 0;
bool set_curswant = true;
rettv->vval.v_number = -1;
if (argvars[1].v_type == VAR_UNKNOWN) {
pos_T pos;
colnr_T curswant = -1;
if (list2fpos(argvars, &pos, NULL, &curswant) == FAIL) {
EMSG(_(e_invarg));
return;
}
line = pos.lnum;
col = pos.col;
coladd = pos.coladd;
if (curswant >= 0) {
curwin->w_curswant = curswant - 1;
set_curswant = false;
}
} else {
line = tv_get_lnum(argvars);
col = (long)tv_get_number_chk(&argvars[1], NULL);
if (argvars[2].v_type != VAR_UNKNOWN) {
coladd = (long)tv_get_number_chk(&argvars[2], NULL);
}
}
if (line < 0 || col < 0
|| coladd < 0) {
return; 
}
if (line > 0) {
curwin->w_cursor.lnum = line;
}
if (col > 0) {
curwin->w_cursor.col = col - 1;
}
curwin->w_cursor.coladd = coladd;
check_cursor();
if (has_mbyte) {
mb_adjust_cursor();
}
curwin->w_set_curswant = set_curswant;
rettv->vval.v_number = 0;
}
static void f_debugbreak(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
int pid;
rettv->vval.v_number = FAIL;
pid = (int)tv_get_number(&argvars[0]);
if (pid == 0) {
EMSG(_(e_invarg));
} else {
#if defined(WIN32)
HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);
if (hProcess != NULL) {
DebugBreakProcess(hProcess);
CloseHandle(hProcess);
rettv->vval.v_number = OK;
}
#else
uv_kill(pid, SIGINT);
#endif
}
}
static void f_deepcopy(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
int noref = 0;
if (argvars[1].v_type != VAR_UNKNOWN) {
noref = tv_get_number_chk(&argvars[1], NULL);
}
if (noref < 0 || noref > 1) {
EMSG(_(e_invarg));
} else {
var_item_copy(NULL, &argvars[0], rettv, true, (noref == 0
? get_copyID()
: 0));
}
}
static void f_delete(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = -1;
if (check_restricted() || check_secure()) {
return;
}
const char *const name = tv_get_string(&argvars[0]);
if (*name == NUL) {
EMSG(_(e_invarg));
return;
}
char nbuf[NUMBUFLEN];
const char *flags;
if (argvars[1].v_type != VAR_UNKNOWN) {
flags = tv_get_string_buf(&argvars[1], nbuf);
} else {
flags = "";
}
if (*flags == NUL) {
rettv->vval.v_number = os_remove(name) == 0 ? 0 : -1;
} else if (strcmp(flags, "d") == 0) {
rettv->vval.v_number = os_rmdir(name) == 0 ? 0 : -1;
} else if (strcmp(flags, "rf") == 0) {
rettv->vval.v_number = delete_recursive(name);
} else {
emsgf(_(e_invexpr2), flags);
}
}
static void f_dictwatcheradd(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
if (check_restricted() || check_secure()) {
return;
}
if (argvars[0].v_type != VAR_DICT) {
emsgf(_(e_invarg2), "dict");
return;
} else if (argvars[0].vval.v_dict == NULL) {
const char *const arg_errmsg = _("dictwatcheradd() argument");
const size_t arg_errmsg_len = strlen(arg_errmsg);
emsgf(_(e_readonlyvar), (int)arg_errmsg_len, arg_errmsg);
return;
}
if (argvars[1].v_type != VAR_STRING && argvars[1].v_type != VAR_NUMBER) {
emsgf(_(e_invarg2), "key");
return;
}
const char *const key_pattern = tv_get_string_chk(argvars + 1);
if (key_pattern == NULL) {
return;
}
const size_t key_pattern_len = strlen(key_pattern);
Callback callback;
if (!callback_from_typval(&callback, &argvars[2])) {
emsgf(_(e_invarg2), "funcref");
return;
}
tv_dict_watcher_add(argvars[0].vval.v_dict, key_pattern, key_pattern_len,
callback);
}
static void f_dictwatcherdel(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
if (check_restricted() || check_secure()) {
return;
}
if (argvars[0].v_type != VAR_DICT) {
emsgf(_(e_invarg2), "dict");
return;
}
if (argvars[2].v_type != VAR_FUNC && argvars[2].v_type != VAR_STRING) {
emsgf(_(e_invarg2), "funcref");
return;
}
const char *const key_pattern = tv_get_string_chk(argvars + 1);
if (key_pattern == NULL) {
return;
}
Callback callback;
if (!callback_from_typval(&callback, &argvars[2])) {
return;
}
if (!tv_dict_watcher_remove(argvars[0].vval.v_dict, key_pattern,
strlen(key_pattern), callback)) {
EMSG("Couldn't find a watcher matching key and callback");
}
callback_free(&callback);
}
static void f_deletebufline(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
linenr_T last;
buf_T *curbuf_save = NULL;
win_T *curwin_save = NULL;
buf_T *const buf = tv_get_buf(&argvars[0], false);
if (buf == NULL) {
rettv->vval.v_number = 1; 
return;
}
const bool is_curbuf = buf == curbuf;
const linenr_T first = tv_get_lnum_buf(&argvars[1], buf);
if (argvars[2].v_type != VAR_UNKNOWN) {
last = tv_get_lnum_buf(&argvars[2], buf);
} else {
last = first;
}
if (buf->b_ml.ml_mfp == NULL || first < 1
|| first > buf->b_ml.ml_line_count || last < first) {
rettv->vval.v_number = 1; 
return;
}
if (!is_curbuf) {
curbuf_save = curbuf;
curwin_save = curwin;
curbuf = buf;
find_win_for_curbuf();
}
if (last > curbuf->b_ml.ml_line_count) {
last = curbuf->b_ml.ml_line_count;
}
const long count = last - first + 1;
if (u_sync_once == 2) {
u_sync_once = 1; 
u_sync(true);
}
if (u_save(first - 1, last + 1) == FAIL) {
rettv->vval.v_number = 1; 
return;
}
for (linenr_T lnum = first; lnum <= last; lnum++) {
ml_delete(first, true);
}
FOR_ALL_TAB_WINDOWS(tp, wp) {
if (wp->w_buffer == buf) {
if (wp->w_cursor.lnum > last) {
wp->w_cursor.lnum -= count;
} else if (wp->w_cursor.lnum> first) {
wp->w_cursor.lnum = first;
}
if (wp->w_cursor.lnum > wp->w_buffer->b_ml.ml_line_count) {
wp->w_cursor.lnum = wp->w_buffer->b_ml.ml_line_count;
}
}
}
check_cursor_col();
deleted_lines_mark(first, count);
if (!is_curbuf) {
curbuf = curbuf_save;
curwin = curwin_save;
}
}
static void f_did_filetype(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = did_filetype;
}
static void f_diff_filler(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = diff_check_fill(curwin, tv_get_lnum(argvars));
}
static void f_diff_hlID(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
linenr_T lnum = tv_get_lnum(argvars);
static linenr_T prev_lnum = 0;
static int changedtick = 0;
static int fnum = 0;
static int change_start = 0;
static int change_end = 0;
static hlf_T hlID = (hlf_T)0;
int filler_lines;
int col;
if (lnum < 0) 
lnum = 0;
if (lnum != prev_lnum
|| changedtick != buf_get_changedtick(curbuf)
|| fnum != curbuf->b_fnum) {
filler_lines = diff_check(curwin, lnum);
if (filler_lines < 0) {
if (filler_lines == -1) {
change_start = MAXCOL;
change_end = -1;
if (diff_find_change(curwin, lnum, &change_start, &change_end))
hlID = HLF_ADD; 
else
hlID = HLF_CHD; 
} else
hlID = HLF_ADD; 
} else
hlID = (hlf_T)0;
prev_lnum = lnum;
changedtick = buf_get_changedtick(curbuf);
fnum = curbuf->b_fnum;
}
if (hlID == HLF_CHD || hlID == HLF_TXD) {
col = tv_get_number(&argvars[1]) - 1; 
if (col >= change_start && col <= change_end) {
hlID = HLF_TXD; 
} else {
hlID = HLF_CHD; 
}
}
rettv->vval.v_number = hlID == (hlf_T)0 ? 0 : (int)(hlID + 1);
}
static void f_empty(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
bool n = true;
switch (argvars[0].v_type) {
case VAR_STRING:
case VAR_FUNC: {
n = argvars[0].vval.v_string == NULL
|| *argvars[0].vval.v_string == NUL;
break;
}
case VAR_PARTIAL: {
n = false;
break;
}
case VAR_NUMBER: {
n = argvars[0].vval.v_number == 0;
break;
}
case VAR_FLOAT: {
n = argvars[0].vval.v_float == 0.0;
break;
}
case VAR_LIST: {
n = (tv_list_len(argvars[0].vval.v_list) == 0);
break;
}
case VAR_DICT: {
n = (tv_dict_len(argvars[0].vval.v_dict) == 0);
break;
}
case VAR_SPECIAL: {
switch (argvars[0].vval.v_special) {
case kSpecialVarTrue: {
n = false;
break;
}
case kSpecialVarFalse:
case kSpecialVarNull: {
n = true;
break;
}
}
break;
}
case VAR_UNKNOWN: {
internal_error("f_empty(UNKNOWN)");
break;
}
}
rettv->vval.v_number = n;
}
static void f_environ(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
tv_dict_alloc_ret(rettv);
size_t env_size = os_get_fullenv_size();
char **env = xmalloc(sizeof(*env) * (env_size + 1));
env[env_size] = NULL;
os_copy_fullenv(env, env_size);
for (size_t i = 0; i < env_size; i++) {
const char * str = env[i];
const char * const end = strchr(str + (str[0] == '=' ? 1 : 0),
'=');
assert(end != NULL);
ptrdiff_t len = end - str;
assert(len > 0);
const char * value = str + len + 1;
tv_dict_add_str(rettv->vval.v_dict,
str, len,
value);
}
os_free_fullenv(env);
}
static void f_escape(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
char buf[NUMBUFLEN];
rettv->vval.v_string = vim_strsave_escaped(
(const char_u *)tv_get_string(&argvars[0]),
(const char_u *)tv_get_string_buf(&argvars[1], buf));
rettv->v_type = VAR_STRING;
}
static void f_getenv(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
char_u *p = (char_u *)vim_getenv(tv_get_string(&argvars[0]));
if (p == NULL) {
rettv->v_type = VAR_SPECIAL;
rettv->vval.v_special = kSpecialVarNull;
return;
}
rettv->vval.v_string = p;
rettv->v_type = VAR_STRING;
}
static void f_eval(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const char *s = tv_get_string_chk(&argvars[0]);
if (s != NULL) {
s = (const char *)skipwhite((const char_u *)s);
}
const char *const expr_start = s;
if (s == NULL || eval1((char_u **)&s, rettv, true) == FAIL) {
if (expr_start != NULL && !aborting()) {
EMSG2(_(e_invexpr2), expr_start);
}
need_clr_eos = FALSE;
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = 0;
} else if (*s != NUL) {
EMSG(_(e_trailing));
}
}
static void f_eventhandler(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = vgetc_busy;
}
static void f_executable(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const char *name = tv_get_string(&argvars[0]);
rettv->vval.v_number = os_can_exe(name, NULL, true);
}
typedef struct {
const list_T *const l;
const listitem_T *li;
} GetListLineCookie;
static char_u *get_list_line(int c, void *cookie, int indent, bool do_concat)
{
GetListLineCookie *const p = (GetListLineCookie *)cookie;
const listitem_T *const item = p->li;
if (item == NULL) {
return NULL;
}
char buf[NUMBUFLEN];
const char *const s = tv_get_string_buf_chk(TV_LIST_ITEM_TV(item), buf);
p->li = TV_LIST_ITEM_NEXT(p->l, item);
return (char_u *)(s == NULL ? NULL : xstrdup(s));
}
static void f_execute(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const int save_msg_silent = msg_silent;
const int save_emsg_silent = emsg_silent;
const bool save_emsg_noredir = emsg_noredir;
const bool save_redir_off = redir_off;
garray_T *const save_capture_ga = capture_ga;
const int save_msg_col = msg_col;
bool echo_output = false;
if (check_secure()) {
return;
}
if (argvars[1].v_type != VAR_UNKNOWN) {
char buf[NUMBUFLEN];
const char *const s = tv_get_string_buf_chk(&argvars[1], buf);
if (s == NULL) {
return;
}
if (*s == NUL) {
echo_output = true;
}
if (strncmp(s, "silent", 6) == 0) {
msg_silent++;
}
if (strcmp(s, "silent!") == 0) {
emsg_silent = true;
emsg_noredir = true;
}
} else {
msg_silent++;
}
garray_T capture_local;
ga_init(&capture_local, (int)sizeof(char), 80);
capture_ga = &capture_local;
redir_off = false;
if (!echo_output) {
msg_col = 0; 
}
if (argvars[0].v_type != VAR_LIST) {
do_cmdline_cmd(tv_get_string(&argvars[0]));
} else if (argvars[0].vval.v_list != NULL) {
list_T *const list = argvars[0].vval.v_list;
tv_list_ref(list);
GetListLineCookie cookie = {
.l = list,
.li = tv_list_first(list),
};
do_cmdline(NULL, get_list_line, (void *)&cookie,
DOCMD_NOWAIT|DOCMD_VERBOSE|DOCMD_REPEAT|DOCMD_KEYTYPED);
tv_list_unref(list);
}
msg_silent = save_msg_silent;
emsg_silent = save_emsg_silent;
emsg_noredir = save_emsg_noredir;
redir_off = save_redir_off;
if (echo_output) {
msg_col = 0;
} else {
msg_col = save_msg_col;
}
ga_append(capture_ga, NUL);
rettv->v_type = VAR_STRING;
rettv->vval.v_string = capture_ga->ga_data;
capture_ga = save_capture_ga;
}
static void f_exepath(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const char *arg = tv_get_string(&argvars[0]);
char *path = NULL;
(void)os_can_exe(arg, &path, true);
rettv->v_type = VAR_STRING;
rettv->vval.v_string = (char_u *)path;
}
static void f_exists(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
int n = false;
int len = 0;
const char *p = tv_get_string(&argvars[0]);
if (*p == '$') { 
if (os_env_exists(p + 1)) {
n = true;
} else {
char_u *const exp = expand_env_save((char_u *)p);
if (exp != NULL && *exp != '$') {
n = true;
}
xfree(exp);
}
} else if (*p == '&' || *p == '+') { 
n = (get_option_tv(&p, NULL, true) == OK);
if (*skipwhite((const char_u *)p) != NUL) {
n = false; 
}
} else if (*p == '*') { 
n = function_exists(p + 1, false);
} else if (*p == ':') {
n = cmd_exists(p + 1);
} else if (*p == '#') {
if (p[1] == '#') {
n = autocmd_supported(p + 2);
} else {
n = au_exists(p + 1);
}
} else { 
typval_T tv;
const char *name = p;
char *tofree;
len = get_name_len((const char **)&p, &tofree, true, false);
if (len > 0) {
if (tofree != NULL) {
name = tofree;
}
n = (get_var_tv(name, len, &tv, NULL, false, true) == OK);
if (n) {
n = (handle_subscript(&p, &tv, true, false) == OK);
if (n) {
tv_clear(&tv);
}
}
}
if (*p != NUL)
n = FALSE;
xfree(tofree);
}
rettv->vval.v_number = n;
}
static void f_expand(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
size_t len;
char_u *errormsg;
int options = WILD_SILENT|WILD_USE_NL|WILD_LIST_NOTFOUND;
expand_T xpc;
bool error = false;
char_u *result;
rettv->v_type = VAR_STRING;
if (argvars[1].v_type != VAR_UNKNOWN
&& argvars[2].v_type != VAR_UNKNOWN
&& tv_get_number_chk(&argvars[2], &error)
&& !error) {
tv_list_set_ret(rettv, NULL);
}
const char *s = tv_get_string(&argvars[0]);
if (*s == '%' || *s == '#' || *s == '<') {
emsg_off++;
result = eval_vars((char_u *)s, (char_u *)s, &len, NULL, &errormsg, NULL);
emsg_off--;
if (rettv->v_type == VAR_LIST) {
tv_list_alloc_ret(rettv, (result != NULL));
if (result != NULL) {
tv_list_append_string(rettv->vval.v_list, (const char *)result, -1);
}
} else
rettv->vval.v_string = result;
} else {
if (argvars[1].v_type != VAR_UNKNOWN
&& tv_get_number_chk(&argvars[1], &error)) {
options |= WILD_KEEP_ALL;
}
if (!error) {
ExpandInit(&xpc);
xpc.xp_context = EXPAND_FILES;
if (p_wic) {
options += WILD_ICASE;
}
if (rettv->v_type == VAR_STRING) {
rettv->vval.v_string = ExpandOne(&xpc, (char_u *)s, NULL, options,
WILD_ALL);
} else {
ExpandOne(&xpc, (char_u *)s, NULL, options, WILD_ALL_KEEP);
tv_list_alloc_ret(rettv, xpc.xp_numfiles);
for (int i = 0; i < xpc.xp_numfiles; i++) {
tv_list_append_string(rettv->vval.v_list,
(const char *)xpc.xp_files[i], -1);
}
ExpandCleanup(&xpc);
}
} else {
rettv->vval.v_string = NULL;
}
}
}
static void f_menu_get(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
tv_list_alloc_ret(rettv, kListLenMayKnow);
int modes = MENU_ALL_MODES;
if (argvars[1].v_type == VAR_STRING) {
const char_u *const strmodes = (char_u *)tv_get_string(&argvars[1]);
modes = get_menu_cmd_modes(strmodes, false, NULL, NULL);
}
menu_get((char_u *)tv_get_string(&argvars[0]), modes, rettv->vval.v_list);
}
static void f_expandcmd(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
char_u *errormsg = NULL;
rettv->v_type = VAR_STRING;
char_u *cmdstr = (char_u *)xstrdup(tv_get_string(&argvars[0]));
exarg_T eap = {
.cmd = cmdstr,
.arg = cmdstr,
.usefilter = false,
.nextcmd = NULL,
.cmdidx = CMD_USER,
};
eap.argt |= NOSPC;
expand_filename(&eap, &cmdstr, &errormsg);
if (errormsg != NULL && *errormsg != NUL) {
EMSG(errormsg);
}
rettv->vval.v_string = cmdstr;
}
static void f_extend(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const char *const arg_errmsg = N_("extend() argument");
if (argvars[0].v_type == VAR_LIST && argvars[1].v_type == VAR_LIST) {
long before;
bool error = false;
list_T *const l1 = argvars[0].vval.v_list;
list_T *const l2 = argvars[1].vval.v_list;
if (!tv_check_lock(tv_list_locked(l1), arg_errmsg, TV_TRANSLATE)) {
listitem_T *item;
if (argvars[2].v_type != VAR_UNKNOWN) {
before = (long)tv_get_number_chk(&argvars[2], &error);
if (error) {
return; 
}
if (before == tv_list_len(l1)) {
item = NULL;
} else {
item = tv_list_find(l1, before);
if (item == NULL) {
EMSGN(_(e_listidx), before);
return;
}
}
} else {
item = NULL;
}
tv_list_extend(l1, l2, item);
tv_copy(&argvars[0], rettv);
}
} else if (argvars[0].v_type == VAR_DICT && argvars[1].v_type ==
VAR_DICT) {
dict_T *const d1 = argvars[0].vval.v_dict;
dict_T *const d2 = argvars[1].vval.v_dict;
if (d1 == NULL) {
const bool locked = tv_check_lock(VAR_FIXED, arg_errmsg, TV_TRANSLATE);
(void)locked;
assert(locked == true);
} else if (d2 == NULL) {
tv_copy(&argvars[0], rettv);
} else if (!tv_check_lock(d1->dv_lock, arg_errmsg, TV_TRANSLATE)) {
const char *action = "force";
if (argvars[2].v_type != VAR_UNKNOWN) {
const char *const av[] = { "keep", "force", "error" };
action = tv_get_string_chk(&argvars[2]);
if (action == NULL) {
return; 
}
size_t i;
for (i = 0; i < ARRAY_SIZE(av); i++) {
if (strcmp(action, av[i]) == 0) {
break;
}
}
if (i == 3) {
EMSG2(_(e_invarg2), action);
return;
}
}
tv_dict_extend(d1, d2, action);
tv_copy(&argvars[0], rettv);
}
} else {
EMSG2(_(e_listdictarg), "extend()");
}
}
static void f_feedkeys(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
if (check_secure()) {
return;
}
const char *const keys = tv_get_string(&argvars[0]);
char nbuf[NUMBUFLEN];
const char *flags = NULL;
if (argvars[1].v_type != VAR_UNKNOWN) {
flags = tv_get_string_buf(&argvars[1], nbuf);
}
nvim_feedkeys(cstr_as_string((char *)keys),
cstr_as_string((char *)flags), true);
}
static void f_filereadable(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const char *const p = tv_get_string(&argvars[0]);
rettv->vval.v_number =
(*p && !os_isdir((const char_u *)p) && os_file_is_readable(p));
}
static void f_filewritable(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const char *filename = tv_get_string(&argvars[0]);
rettv->vval.v_number = os_file_is_writable(filename);
}
static void findfilendir(typval_T *argvars, typval_T *rettv, int find_what)
{
char_u *fresult = NULL;
char_u *path = *curbuf->b_p_path == NUL ? p_path : curbuf->b_p_path;
int count = 1;
bool first = true;
bool error = false;
rettv->vval.v_string = NULL;
rettv->v_type = VAR_STRING;
const char *fname = tv_get_string(&argvars[0]);
char pathbuf[NUMBUFLEN];
if (argvars[1].v_type != VAR_UNKNOWN) {
const char *p = tv_get_string_buf_chk(&argvars[1], pathbuf);
if (p == NULL) {
error = true;
} else {
if (*p != NUL) {
path = (char_u *)p;
}
if (argvars[2].v_type != VAR_UNKNOWN) {
count = tv_get_number_chk(&argvars[2], &error);
}
}
}
if (count < 0) {
tv_list_alloc_ret(rettv, kListLenUnknown);
}
if (*fname != NUL && !error) {
do {
if (rettv->v_type == VAR_STRING || rettv->v_type == VAR_LIST)
xfree(fresult);
fresult = find_file_in_path_option(first ? (char_u *)fname : NULL,
first ? strlen(fname) : 0,
0, first, path,
find_what, curbuf->b_ffname,
(find_what == FINDFILE_DIR
? (char_u *)""
: curbuf->b_p_sua));
first = false;
if (fresult != NULL && rettv->v_type == VAR_LIST) {
tv_list_append_string(rettv->vval.v_list, (const char *)fresult, -1);
}
} while ((rettv->v_type == VAR_LIST || --count > 0) && fresult != NULL);
}
if (rettv->v_type == VAR_STRING)
rettv->vval.v_string = fresult;
}
static void f_filter(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
filter_map(argvars, rettv, FALSE);
}
static void f_finddir(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
findfilendir(argvars, rettv, FINDFILE_DIR);
}
static void f_findfile(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
findfilendir(argvars, rettv, FINDFILE_FILE);
}
static void f_float2nr(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
float_T f;
if (tv_get_float_chk(argvars, &f)) {
if (f <= -VARNUMBER_MAX + DBL_EPSILON) {
rettv->vval.v_number = -VARNUMBER_MAX;
} else if (f >= VARNUMBER_MAX - DBL_EPSILON) {
rettv->vval.v_number = VARNUMBER_MAX;
} else {
rettv->vval.v_number = (varnumber_T)f;
}
}
}
static void f_fmod(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
float_T fx;
float_T fy;
rettv->v_type = VAR_FLOAT;
if (tv_get_float_chk(argvars, &fx) && tv_get_float_chk(&argvars[1], &fy)) {
rettv->vval.v_float = fmod(fx, fy);
} else {
rettv->vval.v_float = 0.0;
}
}
static void f_fnameescape(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_string = (char_u *)vim_strsave_fnameescape(
tv_get_string(&argvars[0]), false);
rettv->v_type = VAR_STRING;
}
static void f_fnamemodify(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
char_u *fbuf = NULL;
size_t len;
char buf[NUMBUFLEN];
const char *fname = tv_get_string_chk(&argvars[0]);
const char *const mods = tv_get_string_buf_chk(&argvars[1], buf);
if (fname == NULL || mods == NULL) {
fname = NULL;
} else {
len = strlen(fname);
size_t usedlen = 0;
(void)modify_fname((char_u *)mods, false, &usedlen,
(char_u **)&fname, &fbuf, &len);
}
rettv->v_type = VAR_STRING;
if (fname == NULL) {
rettv->vval.v_string = NULL;
} else {
rettv->vval.v_string = (char_u *)xmemdupz(fname, len);
}
xfree(fbuf);
}
static void foldclosed_both(typval_T *argvars, typval_T *rettv, int end)
{
const linenr_T lnum = tv_get_lnum(argvars);
if (lnum >= 1 && lnum <= curbuf->b_ml.ml_line_count) {
linenr_T first;
linenr_T last;
if (hasFoldingWin(curwin, lnum, &first, &last, false, NULL)) {
if (end) {
rettv->vval.v_number = (varnumber_T)last;
} else {
rettv->vval.v_number = (varnumber_T)first;
}
return;
}
}
rettv->vval.v_number = -1;
}
static void f_foldclosed(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
foldclosed_both(argvars, rettv, FALSE);
}
static void f_foldclosedend(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
foldclosed_both(argvars, rettv, TRUE);
}
static void f_foldlevel(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const linenr_T lnum = tv_get_lnum(argvars);
if (lnum >= 1 && lnum <= curbuf->b_ml.ml_line_count) {
rettv->vval.v_number = foldLevel(lnum);
}
}
static void f_foldtext(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
linenr_T foldstart;
linenr_T foldend;
char_u *dashes;
linenr_T lnum;
char_u *s;
char_u *r;
int len;
char *txt;
rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;
foldstart = (linenr_T)get_vim_var_nr(VV_FOLDSTART);
foldend = (linenr_T)get_vim_var_nr(VV_FOLDEND);
dashes = get_vim_var_str(VV_FOLDDASHES);
if (foldstart > 0 && foldend <= curbuf->b_ml.ml_line_count) {
for (lnum = foldstart; lnum < foldend; lnum++) {
if (!linewhite(lnum)) {
break;
}
}
s = skipwhite(ml_get(lnum));
if (s[0] == '/' && (s[1] == '*' || s[1] == '/')) {
s = skipwhite(s + 2);
if (*skipwhite(s) == NUL && lnum + 1 < foldend) {
s = skipwhite(ml_get(lnum + 1));
if (*s == '*')
s = skipwhite(s + 1);
}
}
unsigned long count = (unsigned long)(foldend - foldstart + 1);
txt = NGETTEXT("+-%s%3ld line: ", "+-%s%3ld lines: ", count);
r = xmalloc(STRLEN(txt)
+ STRLEN(dashes) 
+ 20 
+ STRLEN(s)); 
sprintf((char *)r, txt, dashes, count);
len = (int)STRLEN(r);
STRCAT(r, s);
foldtext_cleanup(r + len);
rettv->vval.v_string = r;
}
}
static void f_foldtextresult(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
char_u *text;
char_u buf[FOLD_TEXT_LEN];
foldinfo_T foldinfo;
int fold_count;
static bool entered = false;
rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;
if (entered) {
return; 
}
entered = true;
linenr_T lnum = tv_get_lnum(argvars);
if (lnum < 0) {
lnum = 0;
}
fold_count = foldedCount(curwin, lnum, &foldinfo);
if (fold_count > 0) {
text = get_foldtext(curwin, lnum, lnum + fold_count - 1, &foldinfo, buf);
if (text == buf) {
text = vim_strsave(text);
}
rettv->vval.v_string = text;
}
entered = false;
}
static void f_foreground(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
}
static void f_funcref(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
common_function(argvars, rettv, true, fptr);
}
static void f_function(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
common_function(argvars, rettv, false, fptr);
}
static void f_garbagecollect(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
want_garbage_collect = true;
if (argvars[0].v_type != VAR_UNKNOWN && tv_get_number(&argvars[0]) == 1) {
garbage_collect_at_exit = true;
}
}
static void f_get(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
listitem_T *li;
list_T *l;
dictitem_T *di;
dict_T *d;
typval_T *tv = NULL;
bool what_is_dict = false;
if (argvars[0].v_type == VAR_LIST) {
if ((l = argvars[0].vval.v_list) != NULL) {
bool error = false;
li = tv_list_find(l, tv_get_number_chk(&argvars[1], &error));
if (!error && li != NULL) {
tv = TV_LIST_ITEM_TV(li);
}
}
} else if (argvars[0].v_type == VAR_DICT) {
if ((d = argvars[0].vval.v_dict) != NULL) {
di = tv_dict_find(d, tv_get_string(&argvars[1]), -1);
if (di != NULL) {
tv = &di->di_tv;
}
}
} else if (tv_is_func(argvars[0])) {
partial_T *pt;
partial_T fref_pt;
if (argvars[0].v_type == VAR_PARTIAL) {
pt = argvars[0].vval.v_partial;
} else {
memset(&fref_pt, 0, sizeof(fref_pt));
fref_pt.pt_name = argvars[0].vval.v_string;
pt = &fref_pt;
}
if (pt != NULL) {
const char *const what = tv_get_string(&argvars[1]);
if (strcmp(what, "func") == 0 || strcmp(what, "name") == 0) {
rettv->v_type = (*what == 'f' ? VAR_FUNC : VAR_STRING);
const char *const n = (const char *)partial_name(pt);
assert(n != NULL);
rettv->vval.v_string = (char_u *)xstrdup(n);
if (rettv->v_type == VAR_FUNC) {
func_ref(rettv->vval.v_string);
}
} else if (strcmp(what, "dict") == 0) {
what_is_dict = true;
if (pt->pt_dict != NULL) {
tv_dict_set_ret(rettv, pt->pt_dict);
}
} else if (strcmp(what, "args") == 0) {
rettv->v_type = VAR_LIST;
if (tv_list_alloc_ret(rettv, pt->pt_argc) != NULL) {
for (int i = 0; i < pt->pt_argc; i++) {
tv_list_append_tv(rettv->vval.v_list, &pt->pt_argv[i]);
}
}
} else {
EMSG2(_(e_invarg2), what);
}
if (!what_is_dict) {
return;
}
}
} else {
EMSG2(_(e_listdictarg), "get()");
}
if (tv == NULL) {
if (argvars[2].v_type != VAR_UNKNOWN) {
tv_copy(&argvars[2], rettv);
}
} else {
tv_copy(tv, rettv);
}
}
static void f_getbufinfo(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
buf_T *argbuf = NULL;
bool filtered = false;
bool sel_buflisted = false;
bool sel_bufloaded = false;
bool sel_bufmodified = false;
tv_list_alloc_ret(rettv, kListLenMayKnow);
if (argvars[0].v_type == VAR_DICT) {
dict_T *sel_d = argvars[0].vval.v_dict;
if (sel_d != NULL) {
dictitem_T *di;
filtered = true;
di = tv_dict_find(sel_d, S_LEN("buflisted"));
if (di != NULL && tv_get_number(&di->di_tv)) {
sel_buflisted = true;
}
di = tv_dict_find(sel_d, S_LEN("bufloaded"));
if (di != NULL && tv_get_number(&di->di_tv)) {
sel_bufloaded = true;
}
di = tv_dict_find(sel_d, S_LEN("bufmodified"));
if (di != NULL && tv_get_number(&di->di_tv)) {
sel_bufmodified = true;
}
}
} else if (argvars[0].v_type != VAR_UNKNOWN) {
if (tv_check_num(&argvars[0])) { 
emsg_off++;
argbuf = tv_get_buf(&argvars[0], false);
emsg_off--;
if (argbuf == NULL) {
return;
}
}
}
FOR_ALL_BUFFERS(buf) {
if (argbuf != NULL && argbuf != buf) {
continue;
}
if (filtered && ((sel_bufloaded && buf->b_ml.ml_mfp == NULL)
|| (sel_buflisted && !buf->b_p_bl)
|| (sel_bufmodified && !buf->b_changed))) {
continue;
}
dict_T *const d = get_buffer_info(buf);
tv_list_append_dict(rettv->vval.v_list, d);
if (argbuf != NULL) {
return;
}
}
}
static void get_buffer_lines(buf_T *buf,
linenr_T start,
linenr_T end,
int retlist,
typval_T *rettv)
{
rettv->v_type = (retlist ? VAR_LIST : VAR_STRING);
rettv->vval.v_string = NULL;
if (buf == NULL || buf->b_ml.ml_mfp == NULL || start < 0 || end < start) {
if (retlist) {
tv_list_alloc_ret(rettv, 0);
}
return;
}
if (retlist) {
if (start < 1) {
start = 1;
}
if (end > buf->b_ml.ml_line_count) {
end = buf->b_ml.ml_line_count;
}
tv_list_alloc_ret(rettv, end - start + 1);
while (start <= end) {
tv_list_append_string(rettv->vval.v_list,
(const char *)ml_get_buf(buf, start++, false), -1);
}
} else {
rettv->v_type = VAR_STRING;
rettv->vval.v_string = ((start >= 1 && start <= buf->b_ml.ml_line_count)
? vim_strsave(ml_get_buf(buf, start, false))
: NULL);
}
}
static void f_getbufline(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
buf_T *buf = NULL;
if (tv_check_str_or_nr(&argvars[0])) {
emsg_off++;
buf = tv_get_buf(&argvars[0], false);
emsg_off--;
}
const linenr_T lnum = tv_get_lnum_buf(&argvars[1], buf);
const linenr_T end = (argvars[2].v_type == VAR_UNKNOWN
? lnum
: tv_get_lnum_buf(&argvars[2], buf));
get_buffer_lines(buf, lnum, end, true, rettv);
}
static void f_getbufvar(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
bool done = false;
rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;
if (!tv_check_str_or_nr(&argvars[0])) {
goto f_getbufvar_end;
}
const char *varname = tv_get_string_chk(&argvars[1]);
emsg_off++;
buf_T *const buf = tv_get_buf(&argvars[0], false);
if (buf != NULL && varname != NULL) {
buf_T *const save_curbuf = curbuf;
curbuf = buf;
if (*varname == '&') { 
if (varname[1] == NUL) {
dict_T *opts = get_winbuf_options(true);
if (opts != NULL) {
tv_dict_set_ret(rettv, opts);
done = true;
}
} else if (get_option_tv(&varname, rettv, true) == OK) {
done = true;
}
} else {
dictitem_T *const v = find_var_in_ht(&curbuf->b_vars->dv_hashtab, 'b',
varname, strlen(varname), false);
if (v != NULL) {
tv_copy(&v->di_tv, rettv);
done = true;
}
}
curbuf = save_curbuf;
}
emsg_off--;
f_getbufvar_end:
if (!done && argvars[2].v_type != VAR_UNKNOWN) {
tv_copy(&argvars[2], rettv);
}
}
static void f_getchangelist(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
tv_list_alloc_ret(rettv, 2);
vim_ignored = tv_get_number(&argvars[0]); 
emsg_off++;
const buf_T *const buf = tv_get_buf(&argvars[0], false);
emsg_off--;
if (buf == NULL) {
return;
}
list_T *const l = tv_list_alloc(buf->b_changelistlen);
tv_list_append_list(rettv->vval.v_list, l);
tv_list_append_number(rettv->vval.v_list,
(buf == curwin->w_buffer)
? curwin->w_changelistidx
: buf->b_changelistlen);
for (int i = 0; i < buf->b_changelistlen; i++) {
if (buf->b_changelist[i].mark.lnum == 0) {
continue;
}
dict_T *const d = tv_dict_alloc();
tv_list_append_dict(l, d);
tv_dict_add_nr(d, S_LEN("lnum"), buf->b_changelist[i].mark.lnum);
tv_dict_add_nr(d, S_LEN("col"), buf->b_changelist[i].mark.col);
tv_dict_add_nr(d, S_LEN("coladd"), buf->b_changelist[i].mark.coladd);
}
}
static void f_getchar(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
varnumber_T n;
bool error = false;
no_mapping++;
for (;; ) {
ui_cursor_goto(msg_row, msg_col);
if (argvars[0].v_type == VAR_UNKNOWN) {
if (!(char_avail() || using_script() || input_available())) {
(void)os_inchar(NULL, 0, -1, 0, main_loop.events);
if (!multiqueue_empty(main_loop.events)) {
multiqueue_process_events(main_loop.events);
continue;
}
}
n = safe_vgetc();
} else if (tv_get_number_chk(&argvars[0], &error) == 1) {
n = vpeekc_any();
} else if (error || vpeekc_any() == NUL) {
n = 0;
} else {
n = safe_vgetc();
}
if (n == K_IGNORE) {
continue;
}
break;
}
no_mapping--;
set_vim_var_nr(VV_MOUSE_WIN, 0);
set_vim_var_nr(VV_MOUSE_WINID, 0);
set_vim_var_nr(VV_MOUSE_LNUM, 0);
set_vim_var_nr(VV_MOUSE_COL, 0);
rettv->vval.v_number = n;
if (IS_SPECIAL(n) || mod_mask != 0) {
char_u temp[10]; 
int i = 0;
if (mod_mask != 0) {
temp[i++] = K_SPECIAL;
temp[i++] = KS_MODIFIER;
temp[i++] = mod_mask;
}
if (IS_SPECIAL(n)) {
temp[i++] = K_SPECIAL;
temp[i++] = K_SECOND(n);
temp[i++] = K_THIRD(n);
} else {
i += utf_char2bytes(n, temp + i);
}
temp[i++] = NUL;
rettv->v_type = VAR_STRING;
rettv->vval.v_string = vim_strsave(temp);
if (is_mouse_key(n)) {
int row = mouse_row;
int col = mouse_col;
int grid = mouse_grid;
win_T *win;
linenr_T lnum;
win_T *wp;
int winnr = 1;
if (row >= 0 && col >= 0) {
win = mouse_find_win(&grid, &row, &col);
if (win == NULL) {
return;
}
(void)mouse_comp_pos(win, &row, &col, &lnum);
for (wp = firstwin; wp != win; wp = wp->w_next)
++winnr;
set_vim_var_nr(VV_MOUSE_WIN, winnr);
set_vim_var_nr(VV_MOUSE_WINID, wp->handle);
set_vim_var_nr(VV_MOUSE_LNUM, lnum);
set_vim_var_nr(VV_MOUSE_COL, col + 1);
}
}
}
}
static void f_getcharmod(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = mod_mask;
}
static void f_getcharsearch(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
tv_dict_alloc_ret(rettv);
dict_T *dict = rettv->vval.v_dict;
tv_dict_add_str(dict, S_LEN("char"), last_csearch());
tv_dict_add_nr(dict, S_LEN("forward"), last_csearch_forward());
tv_dict_add_nr(dict, S_LEN("until"), last_csearch_until());
}
static void f_getcmdline(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_STRING;
rettv->vval.v_string = get_cmdline_str();
}
static void f_getcmdpos(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = get_cmdline_pos() + 1;
}
static void f_getcmdtype(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_STRING;
rettv->vval.v_string = xmallocz(1);
rettv->vval.v_string[0] = get_cmdline_type();
}
static void f_getcmdwintype(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;
rettv->vval.v_string = xmallocz(1);
rettv->vval.v_string[0] = cmdwin_type;
}
static void f_getcompletion(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
char_u *pat;
expand_T xpc;
bool filtered = false;
int options = WILD_SILENT | WILD_USE_NL | WILD_ADD_SLASH
| WILD_NO_BEEP;
if (argvars[2].v_type != VAR_UNKNOWN) {
filtered = (bool)tv_get_number_chk(&argvars[2], NULL);
}
if (p_wic) {
options |= WILD_ICASE;
}
if (!filtered) {
options |= WILD_KEEP_ALL;
}
if (argvars[0].v_type != VAR_STRING || argvars[1].v_type != VAR_STRING) {
EMSG(_(e_invarg));
return;
}
if (strcmp(tv_get_string(&argvars[1]), "cmdline") == 0) {
set_one_cmd_context(&xpc, tv_get_string(&argvars[0]));
xpc.xp_pattern_len = STRLEN(xpc.xp_pattern);
goto theend;
}
ExpandInit(&xpc);
xpc.xp_pattern = (char_u *)tv_get_string(&argvars[0]);
xpc.xp_pattern_len = STRLEN(xpc.xp_pattern);
xpc.xp_context = cmdcomplete_str_to_type(
(char_u *)tv_get_string(&argvars[1]));
if (xpc.xp_context == EXPAND_NOTHING) {
EMSG2(_(e_invarg2), argvars[1].vval.v_string);
return;
}
if (xpc.xp_context == EXPAND_MENUS) {
set_context_in_menu_cmd(&xpc, (char_u *)"menu", xpc.xp_pattern, false);
xpc.xp_pattern_len = STRLEN(xpc.xp_pattern);
}
if (xpc.xp_context == EXPAND_CSCOPE) {
set_context_in_cscope_cmd(&xpc, (const char *)xpc.xp_pattern, CMD_cscope);
xpc.xp_pattern_len = STRLEN(xpc.xp_pattern);
}
if (xpc.xp_context == EXPAND_SIGN) {
set_context_in_sign_cmd(&xpc, xpc.xp_pattern);
xpc.xp_pattern_len = STRLEN(xpc.xp_pattern);
}
theend:
pat = addstar(xpc.xp_pattern, xpc.xp_pattern_len, xpc.xp_context);
ExpandOne(&xpc, pat, NULL, options, WILD_ALL_KEEP);
tv_list_alloc_ret(rettv, xpc.xp_numfiles);
for (int i = 0; i < xpc.xp_numfiles; i++) {
tv_list_append_string(rettv->vval.v_list, (const char *)xpc.xp_files[i],
-1);
}
xfree(pat);
ExpandCleanup(&xpc);
}
static void f_getcwd(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
CdScope scope = kCdScopeInvalid;
int scope_number[] = {
[kCdScopeWindow] = 0, 
[kCdScopeTab ] = 0, 
};
char_u *cwd = NULL; 
char_u *from = NULL; 
tabpage_T *tp = curtab; 
win_T *win = curwin; 
rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;
for (int i = MIN_CD_SCOPE; i < MAX_CD_SCOPE; i++) {
if (argvars[i].v_type == VAR_UNKNOWN) {
break;
}
if (argvars[i].v_type != VAR_NUMBER) {
EMSG(_(e_invarg));
return;
}
scope_number[i] = argvars[i].vval.v_number;
if (scope_number[i] < -1) {
EMSG(_(e_invarg));
return;
}
if (scope_number[i] >= 0 && scope == kCdScopeInvalid) {
scope = i;
} else if (scope_number[i] < 0) {
scope = i + 1;
}
}
if (scope == kCdScopeInvalid) {
scope = MIN_CD_SCOPE;
}
if (scope_number[kCdScopeTab] > 0) {
tp = find_tabpage(scope_number[kCdScopeTab]);
if (!tp) {
EMSG(_("E5000: Cannot find tab number."));
return;
}
}
if (scope_number[kCdScopeWindow] >= 0) {
if (scope_number[kCdScopeTab] < 0) {
EMSG(_("E5001: Higher scope cannot be -1 if lower scope is >= 0."));
return;
}
if (scope_number[kCdScopeWindow] > 0) {
win = find_win_by_nr(&argvars[0], tp);
if (!win) {
EMSG(_("E5002: Cannot find window number."));
return;
}
}
}
cwd = xmalloc(MAXPATHL);
switch (scope) {
case kCdScopeWindow:
assert(win);
from = win->w_localdir;
if (from) {
break;
}
FALLTHROUGH;
case kCdScopeTab:
assert(tp);
from = tp->tp_localdir;
if (from) {
break;
}
FALLTHROUGH;
case kCdScopeGlobal:
if (globaldir) { 
from = globaldir;
} else if (os_dirname(cwd, MAXPATHL) == FAIL) { 
from = (char_u *)""; 
}
break;
case kCdScopeInvalid: 
assert(false);
}
if (from) {
xstrlcpy((char *)cwd, (char *)from, MAXPATHL);
}
rettv->vval.v_string = vim_strsave(cwd);
#if defined(BACKSLASH_IN_FILENAME)
slash_adjust(rettv->vval.v_string);
#endif
xfree(cwd);
}
static void f_getfontname(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;
}
static void f_getfperm(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
char *perm = NULL;
char_u flags[] = "rwx";
const char *filename = tv_get_string(&argvars[0]);
int32_t file_perm = os_getperm(filename);
if (file_perm >= 0) {
perm = xstrdup("---------");
for (int i = 0; i < 9; i++) {
if (file_perm & (1 << (8 - i))) {
perm[i] = flags[i % 3];
}
}
}
rettv->v_type = VAR_STRING;
rettv->vval.v_string = (char_u *)perm;
}
static void f_getfsize(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const char *fname = tv_get_string(&argvars[0]);
rettv->v_type = VAR_NUMBER;
FileInfo file_info;
if (os_fileinfo(fname, &file_info)) {
uint64_t filesize = os_fileinfo_size(&file_info);
if (os_isdir((const char_u *)fname)) {
rettv->vval.v_number = 0;
} else {
rettv->vval.v_number = (varnumber_T)filesize;
if ((uint64_t)rettv->vval.v_number != filesize) {
rettv->vval.v_number = -2;
}
}
} else {
rettv->vval.v_number = -1;
}
}
static void f_getftime(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const char *fname = tv_get_string(&argvars[0]);
FileInfo file_info;
if (os_fileinfo(fname, &file_info)) {
rettv->vval.v_number = (varnumber_T)file_info.stat.st_mtim.tv_sec;
} else {
rettv->vval.v_number = -1;
}
}
static void f_getftype(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
char_u *type = NULL;
char *t;
const char *fname = tv_get_string(&argvars[0]);
rettv->v_type = VAR_STRING;
FileInfo file_info;
if (os_fileinfo_link(fname, &file_info)) {
uint64_t mode = file_info.stat.st_mode;
#if defined(S_ISREG)
if (S_ISREG(mode))
t = "file";
else if (S_ISDIR(mode))
t = "dir";
#if defined(S_ISLNK)
else if (S_ISLNK(mode))
t = "link";
#endif
#if defined(S_ISBLK)
else if (S_ISBLK(mode))
t = "bdev";
#endif
#if defined(S_ISCHR)
else if (S_ISCHR(mode))
t = "cdev";
#endif
#if defined(S_ISFIFO)
else if (S_ISFIFO(mode))
t = "fifo";
#endif
#if defined(S_ISSOCK)
else if (S_ISSOCK(mode))
t = "socket";
#endif
else
t = "other";
#else
#if defined(S_IFMT)
switch (mode & S_IFMT) {
case S_IFREG: t = "file"; break;
case S_IFDIR: t = "dir"; break;
#if defined(S_IFLNK)
case S_IFLNK: t = "link"; break;
#endif
#if defined(S_IFBLK)
case S_IFBLK: t = "bdev"; break;
#endif
#if defined(S_IFCHR)
case S_IFCHR: t = "cdev"; break;
#endif
#if defined(S_IFIFO)
case S_IFIFO: t = "fifo"; break;
#endif
#if defined(S_IFSOCK)
case S_IFSOCK: t = "socket"; break;
#endif
default: t = "other";
}
#else
if (os_isdir((const char_u *)fname)) {
t = "dir";
} else {
t = "file";
}
#endif
#endif
type = vim_strsave((char_u *)t);
}
rettv->vval.v_string = type;
}
static void f_getjumplist(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
tv_list_alloc_ret(rettv, kListLenMayKnow);
win_T *const wp = find_tabwin(&argvars[0], &argvars[1]);
if (wp == NULL) {
return;
}
cleanup_jumplist(wp, true);
list_T *const l = tv_list_alloc(wp->w_jumplistlen);
tv_list_append_list(rettv->vval.v_list, l);
tv_list_append_number(rettv->vval.v_list, wp->w_jumplistidx);
for (int i = 0; i < wp->w_jumplistlen; i++) {
if (wp->w_jumplist[i].fmark.mark.lnum == 0) {
continue;
}
dict_T *const d = tv_dict_alloc();
tv_list_append_dict(l, d);
tv_dict_add_nr(d, S_LEN("lnum"), wp->w_jumplist[i].fmark.mark.lnum);
tv_dict_add_nr(d, S_LEN("col"), wp->w_jumplist[i].fmark.mark.col);
tv_dict_add_nr(d, S_LEN("coladd"), wp->w_jumplist[i].fmark.mark.coladd);
tv_dict_add_nr(d, S_LEN("bufnr"), wp->w_jumplist[i].fmark.fnum);
if (wp->w_jumplist[i].fname != NULL) {
tv_dict_add_str(d, S_LEN("filename"), (char *)wp->w_jumplist[i].fname);
}
}
}
static void f_getline(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
linenr_T end;
bool retlist;
const linenr_T lnum = tv_get_lnum(argvars);
if (argvars[1].v_type == VAR_UNKNOWN) {
end = lnum;
retlist = false;
} else {
end = tv_get_lnum(&argvars[1]);
retlist = true;
}
get_buffer_lines(curbuf, lnum, end, retlist, rettv);
}
static void f_getloclist(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
win_T *wp = find_win_by_nr_or_id(&argvars[0]);
get_qf_loc_list(false, wp, &argvars[1], rettv);
}
static void f_getmatches(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
matchitem_T *cur = curwin->w_match_head;
int i;
tv_list_alloc_ret(rettv, kListLenMayKnow);
while (cur != NULL) {
dict_T *dict = tv_dict_alloc();
if (cur->match.regprog == NULL) {
for (i = 0; i < MAXPOSMATCH; i++) {
llpos_T *llpos;
char buf[30]; 
llpos = &cur->pos.pos[i];
if (llpos->lnum == 0) {
break;
}
list_T *const l = tv_list_alloc(1 + (llpos->col > 0 ? 2 : 0));
tv_list_append_number(l, (varnumber_T)llpos->lnum);
if (llpos->col > 0) {
tv_list_append_number(l, (varnumber_T)llpos->col);
tv_list_append_number(l, (varnumber_T)llpos->len);
}
int len = snprintf(buf, sizeof(buf), "pos%d", i + 1);
assert((size_t)len < sizeof(buf));
tv_dict_add_list(dict, buf, (size_t)len, l);
}
} else {
tv_dict_add_str(dict, S_LEN("pattern"), (const char *)cur->pattern);
}
tv_dict_add_str(dict, S_LEN("group"),
(const char *)syn_id2name(cur->hlg_id));
tv_dict_add_nr(dict, S_LEN("priority"), (varnumber_T)cur->priority);
tv_dict_add_nr(dict, S_LEN("id"), (varnumber_T)cur->id);
if (cur->conceal_char) {
char buf[MB_MAXBYTES + 1];
buf[utf_char2bytes((int)cur->conceal_char, (char_u *)buf)] = NUL;
tv_dict_add_str(dict, S_LEN("conceal"), buf);
}
tv_list_append_dict(rettv->vval.v_list, dict);
cur = cur->next;
}
}
static void f_getpid(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = os_get_pid();
}
static void getpos_both(typval_T *argvars, typval_T *rettv, bool getcurpos)
{
pos_T *fp;
int fnum = -1;
if (getcurpos) {
fp = &curwin->w_cursor;
} else {
fp = var2fpos(&argvars[0], true, &fnum);
}
list_T *const l = tv_list_alloc_ret(rettv, 4 + (!!getcurpos));
tv_list_append_number(l, (fnum != -1) ? (varnumber_T)fnum : (varnumber_T)0);
tv_list_append_number(l, ((fp != NULL)
? (varnumber_T)fp->lnum
: (varnumber_T)0));
tv_list_append_number(
l, ((fp != NULL)
? (varnumber_T)(fp->col == MAXCOL ? MAXCOL : fp->col + 1)
: (varnumber_T)0));
tv_list_append_number(
l, (fp != NULL) ? (varnumber_T)fp->coladd : (varnumber_T)0);
if (getcurpos) {
const int save_set_curswant = curwin->w_set_curswant;
const colnr_T save_curswant = curwin->w_curswant;
const colnr_T save_virtcol = curwin->w_virtcol;
update_curswant();
tv_list_append_number(l, (curwin->w_curswant == MAXCOL
? (varnumber_T)MAXCOL
: (varnumber_T)curwin->w_curswant + 1));
if (save_set_curswant) {
curwin->w_set_curswant = save_set_curswant;
curwin->w_curswant = save_curswant;
curwin->w_virtcol = save_virtcol;
curwin->w_valid &= ~VALID_VIRTCOL;
}
}
}
static void f_getcurpos(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
getpos_both(argvars, rettv, true);
}
static void f_getpos(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
getpos_both(argvars, rettv, false);
}
static void f_getqflist(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
get_qf_loc_list(true, NULL, &argvars[0], rettv);
}
static void f_getreg(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const char *strregname;
int arg2 = false;
bool return_list = false;
bool error = false;
if (argvars[0].v_type != VAR_UNKNOWN) {
strregname = tv_get_string_chk(&argvars[0]);
error = strregname == NULL;
if (argvars[1].v_type != VAR_UNKNOWN) {
arg2 = tv_get_number_chk(&argvars[1], &error);
if (!error && argvars[2].v_type != VAR_UNKNOWN) {
return_list = tv_get_number_chk(&argvars[2], &error);
}
}
} else {
strregname = _(get_vim_var_str(VV_REG));
}
if (error) {
return;
}
int regname = (uint8_t)(strregname == NULL ? '"' : *strregname);
if (regname == 0) {
regname = '"';
}
if (return_list) {
rettv->v_type = VAR_LIST;
rettv->vval.v_list =
get_reg_contents(regname, (arg2 ? kGRegExprSrc : 0) | kGRegList);
if (rettv->vval.v_list == NULL) {
rettv->vval.v_list = tv_list_alloc(0);
}
tv_list_ref(rettv->vval.v_list);
} else {
rettv->v_type = VAR_STRING;
rettv->vval.v_string = get_reg_contents(regname, arg2 ? kGRegExprSrc : 0);
}
}
static void f_getregtype(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const char *strregname;
if (argvars[0].v_type != VAR_UNKNOWN) {
strregname = tv_get_string_chk(&argvars[0]);
if (strregname == NULL) { 
rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;
return;
}
} else {
strregname = _(get_vim_var_str(VV_REG));
}
int regname = (uint8_t)(strregname == NULL ? '"' : *strregname);
if (regname == 0) {
regname = '"';
}
colnr_T reglen = 0;
char buf[NUMBUFLEN + 2];
MotionType reg_type = get_reg_type(regname, &reglen);
format_reg_type(reg_type, reglen, buf, ARRAY_SIZE(buf));
rettv->v_type = VAR_STRING;
rettv->vval.v_string = (char_u *)xstrdup(buf);
}
static void f_gettabinfo(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
tabpage_T *tparg = NULL;
tv_list_alloc_ret(rettv, (argvars[0].v_type == VAR_UNKNOWN
? 1
: kListLenMayKnow));
if (argvars[0].v_type != VAR_UNKNOWN) {
tparg = find_tabpage((int)tv_get_number_chk(&argvars[0], NULL));
if (tparg == NULL) {
return;
}
}
int tpnr = 0;
FOR_ALL_TABS(tp) {
tpnr++;
if (tparg != NULL && tp != tparg) {
continue;
}
dict_T *const d = get_tabpage_info(tp, tpnr);
tv_list_append_dict(rettv->vval.v_list, d);
if (tparg != NULL) {
return;
}
}
}
static void f_gettabvar(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
win_T *oldcurwin;
tabpage_T *oldtabpage;
bool done = false;
rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;
const char *const varname = tv_get_string_chk(&argvars[1]);
tabpage_T *const tp = find_tabpage((int)tv_get_number_chk(&argvars[0], NULL));
if (tp != NULL && varname != NULL) {
win_T *const window = tp == curtab || tp->tp_firstwin == NULL
? firstwin
: tp->tp_firstwin;
if (switch_win(&oldcurwin, &oldtabpage, window, tp, true) == OK) {
const dictitem_T *const v = find_var_in_ht(&tp->tp_vars->dv_hashtab, 't',
varname, strlen(varname),
false);
if (v != NULL) {
tv_copy(&v->di_tv, rettv);
done = true;
}
}
restore_win(oldcurwin, oldtabpage, true);
}
if (!done && argvars[2].v_type != VAR_UNKNOWN) {
tv_copy(&argvars[2], rettv);
}
}
static void f_gettabwinvar(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
getwinvar(argvars, rettv, 1);
}
static void f_gettagstack(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
win_T *wp = curwin; 
tv_dict_alloc_ret(rettv);
if (argvars[0].v_type != VAR_UNKNOWN) {
wp = find_win_by_nr_or_id(&argvars[0]);
if (wp == NULL) {
return;
}
}
get_tagstack(wp, rettv->vval.v_dict);
}
static void f_getwininfo(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
win_T *wparg = NULL;
tv_list_alloc_ret(rettv, kListLenMayKnow);
if (argvars[0].v_type != VAR_UNKNOWN) {
wparg = win_id2wp(argvars);
if (wparg == NULL) {
return;
}
}
int16_t tabnr = 0;
FOR_ALL_TABS(tp) {
tabnr++;
int16_t winnr = 0;
FOR_ALL_WINDOWS_IN_TAB(wp, tp) {
winnr++;
if (wparg != NULL && wp != wparg) {
continue;
}
dict_T *const d = get_win_info(wp, tabnr, winnr);
tv_list_append_dict(rettv->vval.v_list, d);
if (wparg != NULL) {
return;
}
}
}
}
static void dummy_timer_due_cb(TimeWatcher *tw, void *data)
{
}
static void dummy_timer_close_cb(TimeWatcher *tw, void *data)
{
xfree(tw);
}
static void f_wait(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = -1;
if (argvars[0].v_type != VAR_NUMBER) {
EMSG2(_(e_invargval), "1");
return;
}
if ((argvars[2].v_type != VAR_NUMBER && argvars[2].v_type != VAR_UNKNOWN)
|| (argvars[2].v_type == VAR_NUMBER && argvars[2].vval.v_number <= 0)) {
EMSG2(_(e_invargval), "3");
return;
}
int timeout = argvars[0].vval.v_number;
typval_T expr = argvars[1];
int interval = argvars[2].v_type == VAR_NUMBER
? argvars[2].vval.v_number
: 200; 
TimeWatcher *tw = xmalloc(sizeof(TimeWatcher));
time_watcher_init(&main_loop, tw, NULL);
tw->events = main_loop.events;
tw->blockable = true;
time_watcher_start(tw, dummy_timer_due_cb, interval, interval);
typval_T argv = TV_INITIAL_VALUE;
typval_T exprval = TV_INITIAL_VALUE;
bool error = false;
int save_called_emsg = called_emsg;
called_emsg = false;
LOOP_PROCESS_EVENTS_UNTIL(&main_loop, main_loop.events, timeout,
eval_expr_typval(&expr, &argv, 0, &exprval) != OK
|| tv_get_number_chk(&exprval, &error)
|| called_emsg || error || got_int);
if (called_emsg || error) {
rettv->vval.v_number = -3;
} else if (got_int) {
got_int = false;
vgetc();
rettv->vval.v_number = -2;
} else if (tv_get_number_chk(&exprval, &error)) {
rettv->vval.v_number = 0;
}
called_emsg = save_called_emsg;
time_watcher_stop(tw);
time_watcher_close(tw, dummy_timer_close_cb);
}
static void f_win_screenpos(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
tv_list_alloc_ret(rettv, 2);
const win_T *const wp = find_win_by_nr_or_id(&argvars[0]);
tv_list_append_number(rettv->vval.v_list, wp == NULL ? 0 : wp->w_winrow + 1);
tv_list_append_number(rettv->vval.v_list, wp == NULL ? 0 : wp->w_wincol + 1);
}
static void f_getwinpos(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
tv_list_alloc_ret(rettv, 2);
tv_list_append_number(rettv->vval.v_list, -1);
tv_list_append_number(rettv->vval.v_list, -1);
}
static void f_getwinposx(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = -1;
}
static void f_getwinposy(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = -1;
}
static void f_getwinvar(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
getwinvar(argvars, rettv, 0);
}
static void f_glob(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
int options = WILD_SILENT|WILD_USE_NL;
expand_T xpc;
bool error = false;
rettv->v_type = VAR_STRING;
if (argvars[1].v_type != VAR_UNKNOWN) {
if (tv_get_number_chk(&argvars[1], &error)) {
options |= WILD_KEEP_ALL;
}
if (argvars[2].v_type != VAR_UNKNOWN) {
if (tv_get_number_chk(&argvars[2], &error)) {
tv_list_set_ret(rettv, NULL);
}
if (argvars[3].v_type != VAR_UNKNOWN
&& tv_get_number_chk(&argvars[3], &error)) {
options |= WILD_ALLLINKS;
}
}
}
if (!error) {
ExpandInit(&xpc);
xpc.xp_context = EXPAND_FILES;
if (p_wic)
options += WILD_ICASE;
if (rettv->v_type == VAR_STRING) {
rettv->vval.v_string = ExpandOne(
&xpc, (char_u *)tv_get_string(&argvars[0]), NULL, options, WILD_ALL);
} else {
ExpandOne(&xpc, (char_u *)tv_get_string(&argvars[0]), NULL, options,
WILD_ALL_KEEP);
tv_list_alloc_ret(rettv, xpc.xp_numfiles);
for (int i = 0; i < xpc.xp_numfiles; i++) {
tv_list_append_string(rettv->vval.v_list, (const char *)xpc.xp_files[i],
-1);
}
ExpandCleanup(&xpc);
}
} else
rettv->vval.v_string = NULL;
}
static void f_globpath(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
int flags = 0; 
bool error = false;
rettv->v_type = VAR_STRING;
if (argvars[2].v_type != VAR_UNKNOWN) {
if (tv_get_number_chk(&argvars[2], &error)) {
flags |= WILD_KEEP_ALL;
}
if (argvars[3].v_type != VAR_UNKNOWN) {
if (tv_get_number_chk(&argvars[3], &error)) {
tv_list_set_ret(rettv, NULL);
}
if (argvars[4].v_type != VAR_UNKNOWN
&& tv_get_number_chk(&argvars[4], &error)) {
flags |= WILD_ALLLINKS;
}
}
}
char buf1[NUMBUFLEN];
const char *const file = tv_get_string_buf_chk(&argvars[1], buf1);
if (file != NULL && !error) {
garray_T ga;
ga_init(&ga, (int)sizeof(char_u *), 10);
globpath((char_u *)tv_get_string(&argvars[0]), (char_u *)file, &ga, flags);
if (rettv->v_type == VAR_STRING) {
rettv->vval.v_string = ga_concat_strings_sep(&ga, "\n");
} else {
tv_list_alloc_ret(rettv, ga.ga_len);
for (int i = 0; i < ga.ga_len; i++) {
tv_list_append_string(rettv->vval.v_list,
((const char **)(ga.ga_data))[i], -1);
}
}
ga_clear_strings(&ga);
} else {
rettv->vval.v_string = NULL;
}
}
static void f_glob2regpat(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const char *const pat = tv_get_string_chk(&argvars[0]); 
rettv->v_type = VAR_STRING;
rettv->vval.v_string = ((pat == NULL)
? NULL
: file_pat_to_reg_pat((char_u *)pat, NULL, NULL,
false));
}
static void f_has(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
static const char *const has_list[] = {
#if defined(BSD) && !defined(__APPLE__)
"bsd",
#endif
#if defined(UNIX)
"unix",
#endif
#if defined(WIN32)
"win32",
#endif
#if defined(WIN64) || defined(_WIN64)
"win64",
#endif
"fname_case",
#if defined(HAVE_ACL)
"acl",
#endif
"autochdir",
"arabic",
"autocmd",
"browsefilter",
"byte_offset",
"cindent",
"cmdline_compl",
"cmdline_hist",
"comments",
"conceal",
"cscope",
"cursorbind",
"cursorshape",
#if defined(DEBUG)
"debug",
#endif
"dialog_con",
"diff",
"digraphs",
"eval", 
"ex_extra",
"extra_search",
"file_in_path",
"filterpipe",
"find_in_path",
"float",
"folding",
#if defined(UNIX)
"fork",
#endif
"gettext",
#if defined(HAVE_ICONV)
"iconv",
#endif
"insert_expand",
"jumplist",
"keymap",
"lambda",
"langmap",
"libcall",
"linebreak",
"lispindent",
"listcmds",
"localmap",
#if defined(__APPLE__)
"mac",
"macunix",
"osx",
"osxdarwin",
#endif
"menu",
"mksession",
"modify_fname",
"mouse",
"multi_byte",
"multi_lang",
"num64",
"packages",
"path_extra",
"persistent_undo",
"postscript",
"printer",
"profile",
"pythonx",
"reltime",
"quickfix",
"rightleft",
"scrollbind",
"showcmd",
"cmdline_info",
"shada",
"signs",
"smartindent",
"startuptime",
"statusline",
"spell",
"syntax",
#if !defined(UNIX)
"system", 
#endif
"tablineat",
"tag_binary",
"termguicolors",
"termresponse",
"textobjects",
"timers",
"title",
"user-commands", 
"user_commands",
"vertsplit",
"virtualedit",
"visual",
"visualextra",
"vreplace",
"wildignore",
"wildmenu",
"windows",
"winaltkeys",
"writebackup",
#if defined(HAVE_WSL)
"wsl",
#endif
"nvim",
};
bool n = false;
const char *const name = tv_get_string(&argvars[0]);
for (size_t i = 0; i < ARRAY_SIZE(has_list); i++) {
if (STRICMP(name, has_list[i]) == 0) {
n = true;
break;
}
}
if (!n) {
if (STRNICMP(name, "patch", 5) == 0) {
if (name[5] == '-'
&& strlen(name) >= 11
&& ascii_isdigit(name[6])
&& ascii_isdigit(name[8])
&& ascii_isdigit(name[10])) {
int major = atoi(name + 6);
int minor = atoi(name + 8);
n = (major < VIM_VERSION_MAJOR
|| (major == VIM_VERSION_MAJOR
&& (minor < VIM_VERSION_MINOR
|| (minor == VIM_VERSION_MINOR
&& has_vim_patch(atoi(name + 10))))));
} else {
n = has_vim_patch(atoi(name + 5));
}
} else if (STRNICMP(name, "nvim-", 5) == 0) {
n = has_nvim_version(name + 5);
} else if (STRICMP(name, "vim_starting") == 0) {
n = (starting != 0);
} else if (STRICMP(name, "ttyin") == 0) {
n = stdin_isatty;
} else if (STRICMP(name, "ttyout") == 0) {
n = stdout_isatty;
} else if (STRICMP(name, "multi_byte_encoding") == 0) {
n = has_mbyte != 0;
} else if (STRICMP(name, "syntax_items") == 0) {
n = syntax_present(curwin);
#if defined(UNIX)
} else if (STRICMP(name, "unnamedplus") == 0) {
n = eval_has_provider("clipboard");
#endif
}
}
if (!n && eval_has_provider(name)) {
n = true;
}
rettv->vval.v_number = n;
}
static void f_has_key(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
if (argvars[0].v_type != VAR_DICT) {
EMSG(_(e_dictreq));
return;
}
if (argvars[0].vval.v_dict == NULL)
return;
rettv->vval.v_number = tv_dict_find(argvars[0].vval.v_dict,
tv_get_string(&argvars[1]),
-1) != NULL;
}
static void f_haslocaldir(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
CdScope scope = kCdScopeInvalid;
int scope_number[] = {
[kCdScopeWindow] = 0, 
[kCdScopeTab ] = 0, 
};
tabpage_T *tp = curtab; 
win_T *win = curwin; 
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = 0;
for (int i = MIN_CD_SCOPE; i < MAX_CD_SCOPE; i++) {
if (argvars[i].v_type == VAR_UNKNOWN) {
break;
}
if (argvars[i].v_type != VAR_NUMBER) {
EMSG(_(e_invarg));
return;
}
scope_number[i] = argvars[i].vval.v_number;
if (scope_number[i] < -1) {
EMSG(_(e_invarg));
return;
}
if (scope_number[i] >= 0 && scope == kCdScopeInvalid) {
scope = i;
} else if (scope_number[i] < 0) {
scope = i + 1;
}
}
if (scope == kCdScopeInvalid) {
scope = MIN_CD_SCOPE;
}
if (scope_number[kCdScopeTab] > 0) {
tp = find_tabpage(scope_number[kCdScopeTab]);
if (!tp) {
EMSG(_("E5000: Cannot find tab number."));
return;
}
}
if (scope_number[kCdScopeWindow] >= 0) {
if (scope_number[kCdScopeTab] < 0) {
EMSG(_("E5001: Higher scope cannot be -1 if lower scope is >= 0."));
return;
}
if (scope_number[kCdScopeWindow] > 0) {
win = find_win_by_nr(&argvars[0], tp);
if (!win) {
EMSG(_("E5002: Cannot find window number."));
return;
}
}
}
switch (scope) {
case kCdScopeWindow:
assert(win);
rettv->vval.v_number = win->w_localdir ? 1 : 0;
break;
case kCdScopeTab:
assert(tp);
rettv->vval.v_number = tp->tp_localdir ? 1 : 0;
break;
case kCdScopeGlobal:
break;
case kCdScopeInvalid:
assert(false);
}
}
static void f_hasmapto(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const char *mode;
const char *const name = tv_get_string(&argvars[0]);
bool abbr = false;
char buf[NUMBUFLEN];
if (argvars[1].v_type == VAR_UNKNOWN) {
mode = "nvo";
} else {
mode = tv_get_string_buf(&argvars[1], buf);
if (argvars[2].v_type != VAR_UNKNOWN) {
abbr = tv_get_number(&argvars[2]);
}
}
if (map_to_exists(name, mode, abbr)) {
rettv->vval.v_number = true;
} else {
rettv->vval.v_number = false;
}
}
static void f_histadd(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
HistoryType histype;
rettv->vval.v_number = false;
if (check_secure()) {
return;
}
const char *str = tv_get_string_chk(&argvars[0]); 
histype = str != NULL ? get_histtype(str, strlen(str), false) : HIST_INVALID;
if (histype != HIST_INVALID) {
char buf[NUMBUFLEN];
str = tv_get_string_buf(&argvars[1], buf);
if (*str != NUL) {
init_history();
add_to_history(histype, (char_u *)str, false, NUL);
rettv->vval.v_number = true;
return;
}
}
}
static void f_histdel(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
int n;
const char *const str = tv_get_string_chk(&argvars[0]); 
if (str == NULL) {
n = 0;
} else if (argvars[1].v_type == VAR_UNKNOWN) {
n = clr_history(get_histtype(str, strlen(str), false));
} else if (argvars[1].v_type == VAR_NUMBER) {
n = del_history_idx(get_histtype(str, strlen(str), false),
(int)tv_get_number(&argvars[1]));
} else {
char buf[NUMBUFLEN];
n = del_history_entry(get_histtype(str, strlen(str), false),
(char_u *)tv_get_string_buf(&argvars[1], buf));
}
rettv->vval.v_number = n;
}
static void f_histget(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
HistoryType type;
int idx;
const char *const str = tv_get_string_chk(&argvars[0]); 
if (str == NULL) {
rettv->vval.v_string = NULL;
} else {
type = get_histtype(str, strlen(str), false);
if (argvars[1].v_type == VAR_UNKNOWN) {
idx = get_history_idx(type);
} else {
idx = (int)tv_get_number_chk(&argvars[1], NULL);
}
rettv->vval.v_string = vim_strsave(get_history_entry(type, idx));
}
rettv->v_type = VAR_STRING;
}
static void f_histnr(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const char *const history = tv_get_string_chk(&argvars[0]);
HistoryType i = history == NULL
? HIST_INVALID
: get_histtype(history, strlen(history), false);
if (i != HIST_INVALID) {
i = get_history_idx(i);
}
rettv->vval.v_number = i;
}
static void f_hlID(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = syn_name2id(
(const char_u *)tv_get_string(&argvars[0]));
}
static void f_hlexists(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = highlight_exists(
(const char_u *)tv_get_string(&argvars[0]));
}
static void f_hostname(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
char hostname[256];
os_get_hostname(hostname, 256);
rettv->v_type = VAR_STRING;
rettv->vval.v_string = vim_strsave((char_u *)hostname);
}
static void f_iconv(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
vimconv_T vimconv;
rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;
const char *const str = tv_get_string(&argvars[0]);
char buf1[NUMBUFLEN];
char_u *const from = enc_canonize(enc_skip(
(char_u *)tv_get_string_buf(&argvars[1], buf1)));
char buf2[NUMBUFLEN];
char_u *const to = enc_canonize(enc_skip(
(char_u *)tv_get_string_buf(&argvars[2], buf2)));
vimconv.vc_type = CONV_NONE;
convert_setup(&vimconv, from, to);
if (vimconv.vc_type == CONV_NONE) {
rettv->vval.v_string = (char_u *)xstrdup(str);
} else {
rettv->vval.v_string = string_convert(&vimconv, (char_u *)str, NULL);
}
convert_setup(&vimconv, NULL, NULL);
xfree(from);
xfree(to);
}
static void f_indent(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const linenr_T lnum = tv_get_lnum(argvars);
if (lnum >= 1 && lnum <= curbuf->b_ml.ml_line_count) {
rettv->vval.v_number = get_indent_lnum(lnum);
} else {
rettv->vval.v_number = -1;
}
}
static void f_index(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
long idx = 0;
bool ic = false;
rettv->vval.v_number = -1;
if (argvars[0].v_type != VAR_LIST) {
EMSG(_(e_listreq));
return;
}
list_T *const l = argvars[0].vval.v_list;
if (l != NULL) {
listitem_T *item = tv_list_first(l);
if (argvars[2].v_type != VAR_UNKNOWN) {
bool error = false;
idx = tv_list_uidx(l, tv_get_number_chk(&argvars[2], &error));
if (error || idx == -1) {
item = NULL;
} else {
item = tv_list_find(l, idx);
assert(item != NULL);
}
if (argvars[3].v_type != VAR_UNKNOWN) {
ic = !!tv_get_number_chk(&argvars[3], &error);
if (error) {
item = NULL;
}
}
}
for (; item != NULL; item = TV_LIST_ITEM_NEXT(l, item), idx++) {
if (tv_equal(TV_LIST_ITEM_TV(item), &argvars[1], ic, false)) {
rettv->vval.v_number = idx;
break;
}
}
}
}
static bool inputsecret_flag = false;
static void f_input(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
get_user_input(argvars, rettv, FALSE, inputsecret_flag);
}
static void f_inputdialog(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
get_user_input(argvars, rettv, TRUE, inputsecret_flag);
}
static void f_inputlist(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
int selected;
int mouse_used;
if (argvars[0].v_type != VAR_LIST) {
EMSG2(_(e_listarg), "inputlist()");
return;
}
msg_start();
msg_row = Rows - 1; 
lines_left = Rows; 
msg_scroll = TRUE;
msg_clr_eos();
TV_LIST_ITER_CONST(argvars[0].vval.v_list, li, {
msg_puts(tv_get_string(TV_LIST_ITEM_TV(li)));
msg_putchar('\n');
});
selected = prompt_for_number(&mouse_used);
if (mouse_used) {
selected -= lines_left;
}
rettv->vval.v_number = selected;
}
static garray_T ga_userinput = { 0, 0, sizeof(tasave_T), 4, NULL };
static void f_inputrestore(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
if (!GA_EMPTY(&ga_userinput)) {
ga_userinput.ga_len--;
restore_typeahead((tasave_T *)(ga_userinput.ga_data)
+ ga_userinput.ga_len);
} else if (p_verbose > 1) {
verb_msg(_("called inputrestore() more often than inputsave()"));
rettv->vval.v_number = 1; 
}
}
static void f_inputsave(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
tasave_T *p = GA_APPEND_VIA_PTR(tasave_T, &ga_userinput);
save_typeahead(p);
}
static void f_inputsecret(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
cmdline_star++;
inputsecret_flag = true;
f_input(argvars, rettv, NULL);
cmdline_star--;
inputsecret_flag = false;
}
static void f_insert(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
list_T *l;
bool error = false;
if (argvars[0].v_type != VAR_LIST) {
EMSG2(_(e_listarg), "insert()");
} else if (!tv_check_lock(tv_list_locked((l = argvars[0].vval.v_list)),
N_("insert() argument"), TV_TRANSLATE)) {
long before = 0;
if (argvars[2].v_type != VAR_UNKNOWN) {
before = tv_get_number_chk(&argvars[2], &error);
}
if (error) {
return;
}
listitem_T *item = NULL;
if (before != tv_list_len(l)) {
item = tv_list_find(l, before);
if (item == NULL) {
EMSGN(_(e_listidx), before);
l = NULL;
}
}
if (l != NULL) {
tv_list_insert_tv(l, &argvars[1], item);
tv_copy(&argvars[0], rettv);
}
}
}
static void f_invert(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = ~tv_get_number_chk(&argvars[0], NULL);
}
static void f_isdirectory(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = os_isdir((const char_u *)tv_get_string(&argvars[0]));
}
static void f_islocked(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
lval_T lv;
dictitem_T *di;
rettv->vval.v_number = -1;
const char_u *const end = get_lval((char_u *)tv_get_string(&argvars[0]),
NULL,
&lv, false, false,
GLV_NO_AUTOLOAD|GLV_READ_ONLY,
FNE_CHECK_START);
if (end != NULL && lv.ll_name != NULL) {
if (*end != NUL) {
EMSG(_(e_trailing));
} else {
if (lv.ll_tv == NULL) {
di = find_var((const char *)lv.ll_name, lv.ll_name_len, NULL, true);
if (di != NULL) {
rettv->vval.v_number = ((di->di_flags & DI_FLAGS_LOCK)
|| tv_islocked(&di->di_tv));
}
} else if (lv.ll_range) {
EMSG(_("E786: Range not allowed"));
} else if (lv.ll_newkey != NULL) {
EMSG2(_(e_dictkey), lv.ll_newkey);
} else if (lv.ll_list != NULL) {
rettv->vval.v_number = tv_islocked(TV_LIST_ITEM_TV(lv.ll_li));
} else {
rettv->vval.v_number = tv_islocked(&lv.ll_di->di_tv);
}
}
}
clear_lval(&lv);
}
static void f_isinf(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
if (argvars[0].v_type == VAR_FLOAT
&& xisinf(argvars[0].vval.v_float)) {
rettv->vval.v_number = argvars[0].vval.v_float > 0.0 ? 1 : -1;
}
}
static void f_isnan(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = argvars[0].v_type == VAR_FLOAT
&& xisnan(argvars[0].vval.v_float);
}
static void f_id(typval_T *argvars, typval_T *rettv, FunPtr fptr)
FUNC_ATTR_NONNULL_ALL
{
const int len = vim_vsnprintf_typval(NULL, 0, "%p", dummy_ap, argvars);
rettv->v_type = VAR_STRING;
rettv->vval.v_string = xmalloc(len + 1);
vim_vsnprintf_typval((char *)rettv->vval.v_string, len + 1, "%p",
dummy_ap, argvars);
}
static void f_items(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
dict_list(argvars, rettv, 2);
}
static void f_jobpid(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = 0;
if (check_restricted() || check_secure()) {
return;
}
if (argvars[0].v_type != VAR_NUMBER) {
EMSG(_(e_invarg));
return;
}
Channel *data = find_job(argvars[0].vval.v_number, true);
if (!data) {
return;
}
Process *proc = (Process *)&data->stream.proc;
rettv->vval.v_number = proc->pid;
}
static void f_jobresize(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = 0;
if (check_restricted() || check_secure()) {
return;
}
if (argvars[0].v_type != VAR_NUMBER || argvars[1].v_type != VAR_NUMBER
|| argvars[2].v_type != VAR_NUMBER) {
EMSG(_(e_invarg));
return;
}
Channel *data = find_job(argvars[0].vval.v_number, true);
if (!data) {
return;
}
if (data->stream.proc.type != kProcessTypePty) {
EMSG(_(e_channotpty));
return;
}
pty_process_resize(&data->stream.pty, argvars[1].vval.v_number,
argvars[2].vval.v_number);
rettv->vval.v_number = 1;
}
static void f_jobstart(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = 0;
if (check_restricted() || check_secure()) {
return;
}
bool executable = true;
char **argv = tv_to_argv(&argvars[0], NULL, &executable);
char **env = NULL;
if (!argv) {
rettv->vval.v_number = executable ? 0 : -1;
return; 
}
if (argvars[1].v_type != VAR_DICT && argvars[1].v_type != VAR_UNKNOWN) {
EMSG2(_(e_invarg2), "expected dictionary");
shell_free_argv(argv);
return;
}
dict_T *job_opts = NULL;
bool detach = false;
bool rpc = false;
bool pty = false;
bool clear_env = false;
CallbackReader on_stdout = CALLBACK_READER_INIT,
on_stderr = CALLBACK_READER_INIT;
Callback on_exit = CALLBACK_NONE;
char *cwd = NULL;
if (argvars[1].v_type == VAR_DICT) {
job_opts = argvars[1].vval.v_dict;
detach = tv_dict_get_number(job_opts, "detach") != 0;
rpc = tv_dict_get_number(job_opts, "rpc") != 0;
pty = tv_dict_get_number(job_opts, "pty") != 0;
clear_env = tv_dict_get_number(job_opts, "clear_env") != 0;
if (pty && rpc) {
EMSG2(_(e_invarg2), "job cannot have both 'pty' and 'rpc' options set");
shell_free_argv(argv);
return;
}
char *new_cwd = tv_dict_get_string(job_opts, "cwd", false);
if (new_cwd && strlen(new_cwd) > 0) {
cwd = new_cwd;
if (!os_isdir_executable((const char *)cwd)) {
EMSG2(_(e_invarg2), "expected valid directory");
shell_free_argv(argv);
return;
}
}
dictitem_T *job_env = tv_dict_find(job_opts, S_LEN("env"));
if (job_env) {
if (job_env->di_tv.v_type != VAR_DICT) {
EMSG2(_(e_invarg2), "env");
shell_free_argv(argv);
return;
}
size_t custom_env_size = (size_t)tv_dict_len(job_env->di_tv.vval.v_dict);
size_t i = 0;
size_t env_size = 0;
if (clear_env) {
env = xmalloc((custom_env_size + 1) * sizeof(*env));
env_size = 0;
} else {
env_size = os_get_fullenv_size();
env = xmalloc((custom_env_size + env_size + 1) * sizeof(*env));
os_copy_fullenv(env, env_size);
i = env_size;
}
assert(env); 
TV_DICT_ITER(job_env->di_tv.vval.v_dict, var, {
const char *str = tv_get_string(&var->di_tv);
assert(str);
size_t len = STRLEN(var->di_key) + strlen(str) + strlen("=") + 1;
env[i] = xmalloc(len);
snprintf(env[i], len, "%s=%s", (char *)var->di_key, str);
i++;
});
env[env_size + custom_env_size] = NULL;
}
if (!common_job_callbacks(job_opts, &on_stdout, &on_stderr, &on_exit)) {
shell_free_argv(argv);
return;
}
}
uint16_t width = 0, height = 0;
char *term_name = NULL;
if (pty) {
width = (uint16_t)tv_dict_get_number(job_opts, "width");
height = (uint16_t)tv_dict_get_number(job_opts, "height");
term_name = tv_dict_get_string(job_opts, "TERM", true);
}
Channel *chan = channel_job_start(argv, on_stdout, on_stderr, on_exit, pty,
rpc, detach, cwd, width, height,
term_name, env, &rettv->vval.v_number);
if (chan) {
channel_create_event(chan, NULL);
}
}
static void f_jobstop(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = 0;
if (check_restricted() || check_secure()) {
return;
}
if (argvars[0].v_type != VAR_NUMBER) {
EMSG(_(e_invarg));
return;
}
Channel *data = find_job(argvars[0].vval.v_number, false);
if (!data) {
return;
}
const char *error = NULL;
if (data->is_rpc) {
(void)channel_close(data->id, kChannelPartRpc, &error);
}
process_stop((Process *)&data->stream.proc);
rettv->vval.v_number = 1;
if (error) {
EMSG(error);
}
}
static void f_jobwait(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = 0;
if (check_restricted() || check_secure()) {
return;
}
if (argvars[0].v_type != VAR_LIST || (argvars[1].v_type != VAR_NUMBER
&& argvars[1].v_type != VAR_UNKNOWN)) {
EMSG(_(e_invarg));
return;
}
ui_busy_start();
list_T *args = argvars[0].vval.v_list;
Channel **jobs = xcalloc(tv_list_len(args), sizeof(*jobs));
MultiQueue *waiting_jobs = multiqueue_new_parent(loop_on_put, &main_loop);
int i = 0;
TV_LIST_ITER_CONST(args, arg, {
Channel *chan = NULL;
if (TV_LIST_ITEM_TV(arg)->v_type != VAR_NUMBER
|| !(chan = find_job(TV_LIST_ITEM_TV(arg)->vval.v_number, false))) {
jobs[i] = NULL; 
} else {
jobs[i] = chan;
channel_incref(chan);
if (chan->stream.proc.status < 0) {
multiqueue_process_events(chan->events);
multiqueue_replace_parent(chan->events, waiting_jobs);
}
}
i++;
});
int remaining = -1;
uint64_t before = 0;
if (argvars[1].v_type == VAR_NUMBER && argvars[1].vval.v_number >= 0) {
remaining = argvars[1].vval.v_number;
before = os_hrtime();
}
for (i = 0; i < tv_list_len(args); i++) {
if (remaining == 0) {
break; 
}
if (jobs[i] == NULL) {
continue; 
}
int status = process_wait(&jobs[i]->stream.proc, remaining,
waiting_jobs);
if (status < 0) {
break; 
}
if (remaining > 0) {
uint64_t now = os_hrtime();
remaining = MIN(0, remaining - (int)((now - before) / 1000000));
before = now;
}
}
list_T *const rv = tv_list_alloc(tv_list_len(args));
for (i = 0; i < tv_list_len(args); i++) {
if (jobs[i] == NULL) {
tv_list_append_number(rv, -3);
continue;
}
multiqueue_process_events(jobs[i]->events);
multiqueue_replace_parent(jobs[i]->events, main_loop.events);
tv_list_append_number(rv, jobs[i]->stream.proc.status);
channel_decref(jobs[i]);
}
multiqueue_free(waiting_jobs);
xfree(jobs);
ui_busy_stop();
tv_list_ref(rv);
rettv->v_type = VAR_LIST;
rettv->vval.v_list = rv;
}
static void f_join(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
if (argvars[0].v_type != VAR_LIST) {
EMSG(_(e_listreq));
return;
}
const char *const sep = (argvars[1].v_type == VAR_UNKNOWN
? " "
: tv_get_string_chk(&argvars[1]));
rettv->v_type = VAR_STRING;
if (sep != NULL) {
garray_T ga;
ga_init(&ga, (int)sizeof(char), 80);
tv_list_join(&ga, argvars[0].vval.v_list, sep);
ga_append(&ga, NUL);
rettv->vval.v_string = (char_u *)ga.ga_data;
} else {
rettv->vval.v_string = NULL;
}
}
static void f_json_decode(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
char numbuf[NUMBUFLEN];
const char *s = NULL;
char *tofree = NULL;
size_t len;
if (argvars[0].v_type == VAR_LIST) {
if (!encode_vim_list_to_buf(argvars[0].vval.v_list, &len, &tofree)) {
EMSG(_("E474: Failed to convert list to string"));
return;
}
s = tofree;
if (s == NULL) {
assert(len == 0);
s = "";
}
} else {
s = tv_get_string_buf_chk(&argvars[0], numbuf);
if (s) {
len = strlen(s);
} else {
return;
}
}
if (json_decode_string(s, len, rettv) == FAIL) {
emsgf(_("E474: Failed to parse %.*s"), (int)len, s);
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = 0;
}
assert(rettv->v_type != VAR_UNKNOWN);
xfree(tofree);
}
static void f_json_encode(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_STRING;
rettv->vval.v_string = (char_u *)encode_tv2json(&argvars[0], NULL);
}
static void f_keys(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
dict_list(argvars, rettv, 0);
}
static void f_last_buffer_nr(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
int n = 0;
FOR_ALL_BUFFERS(buf) {
if (n < buf->b_fnum) {
n = buf->b_fnum;
}
}
rettv->vval.v_number = n;
}
static void f_len(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
switch (argvars[0].v_type) {
case VAR_STRING:
case VAR_NUMBER: {
rettv->vval.v_number = (varnumber_T)strlen(
tv_get_string(&argvars[0]));
break;
}
case VAR_LIST: {
rettv->vval.v_number = tv_list_len(argvars[0].vval.v_list);
break;
}
case VAR_DICT: {
rettv->vval.v_number = tv_dict_len(argvars[0].vval.v_dict);
break;
}
case VAR_UNKNOWN:
case VAR_SPECIAL:
case VAR_FLOAT:
case VAR_PARTIAL:
case VAR_FUNC: {
EMSG(_("E701: Invalid type for len()"));
break;
}
}
}
static void libcall_common(typval_T *argvars, typval_T *rettv, int out_type)
{
rettv->v_type = out_type;
if (out_type != VAR_NUMBER) {
rettv->vval.v_string = NULL;
}
if (check_restricted() || check_secure()) {
return;
}
if (argvars[0].v_type != VAR_STRING || argvars[1].v_type != VAR_STRING) {
return;
}
const char *libname = (char *)argvars[0].vval.v_string;
const char *funcname = (char *)argvars[1].vval.v_string;
VarType in_type = argvars[2].v_type;
char *str_in = (in_type == VAR_STRING)
? (char *)argvars[2].vval.v_string : NULL;
int64_t int_in = argvars[2].vval.v_number;
char **str_out = (out_type == VAR_STRING)
? (char **)&rettv->vval.v_string : NULL;
int int_out = 0;
bool success = os_libcall(libname, funcname,
str_in, int_in,
str_out, &int_out);
if (!success) {
EMSG2(_(e_libcall), funcname);
return;
}
if (out_type == VAR_NUMBER) {
rettv->vval.v_number = (varnumber_T)int_out;
}
}
static void f_libcall(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
libcall_common(argvars, rettv, VAR_STRING);
}
static void f_libcallnr(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
libcall_common(argvars, rettv, VAR_NUMBER);
}
static void f_line(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
linenr_T lnum = 0;
pos_T *fp;
int fnum;
fp = var2fpos(&argvars[0], TRUE, &fnum);
if (fp != NULL)
lnum = fp->lnum;
rettv->vval.v_number = lnum;
}
static void f_line2byte(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const linenr_T lnum = tv_get_lnum(argvars);
if (lnum < 1 || lnum > curbuf->b_ml.ml_line_count + 1) {
rettv->vval.v_number = -1;
} else {
rettv->vval.v_number = ml_find_line_or_offset(curbuf, lnum, NULL, false);
}
if (rettv->vval.v_number >= 0) {
rettv->vval.v_number++;
}
}
static void f_lispindent(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const pos_T pos = curwin->w_cursor;
const linenr_T lnum = tv_get_lnum(argvars);
if (lnum >= 1 && lnum <= curbuf->b_ml.ml_line_count) {
curwin->w_cursor.lnum = lnum;
rettv->vval.v_number = get_lisp_indent();
curwin->w_cursor = pos;
} else {
rettv->vval.v_number = -1;
}
}
static void f_list2str(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
garray_T ga;
rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;
if (argvars[0].v_type != VAR_LIST) {
EMSG(_(e_invarg));
return;
}
list_T *const l = argvars[0].vval.v_list;
if (l == NULL) {
return; 
}
ga_init(&ga, 1, 80);
char_u buf[MB_MAXBYTES + 1];
TV_LIST_ITER_CONST(l, li, {
buf[utf_char2bytes(tv_get_number(TV_LIST_ITEM_TV(li)), buf)] = NUL;
ga_concat(&ga, buf);
});
ga_append(&ga, NUL);
rettv->vval.v_string = ga.ga_data;
}
static void f_localtime(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = (varnumber_T)time(NULL);
}
static void get_maparg(typval_T *argvars, typval_T *rettv, int exact)
{
char_u *keys_buf = NULL;
char_u *rhs;
int mode;
int abbr = FALSE;
int get_dict = FALSE;
mapblock_T *mp;
int buffer_local;
rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;
char_u *keys = (char_u *)tv_get_string(&argvars[0]);
if (*keys == NUL) {
return;
}
char buf[NUMBUFLEN];
const char *which;
if (argvars[1].v_type != VAR_UNKNOWN) {
which = tv_get_string_buf_chk(&argvars[1], buf);
if (argvars[2].v_type != VAR_UNKNOWN) {
abbr = tv_get_number(&argvars[2]);
if (argvars[3].v_type != VAR_UNKNOWN) {
get_dict = tv_get_number(&argvars[3]);
}
}
} else {
which = "";
}
if (which == NULL) {
return;
}
mode = get_map_mode((char_u **)&which, 0);
keys = replace_termcodes(keys, STRLEN(keys), &keys_buf, true, true, true,
CPO_TO_CPO_FLAGS);
rhs = check_map(keys, mode, exact, false, abbr, &mp, &buffer_local);
xfree(keys_buf);
if (!get_dict) {
if (rhs != NULL) {
if (*rhs == NUL) {
rettv->vval.v_string = vim_strsave((char_u *)"<Nop>");
} else {
rettv->vval.v_string = (char_u *)str2special_save(
(char *)rhs, false, false);
}
}
} else {
tv_dict_alloc_ret(rettv);
if (rhs != NULL) {
mapblock_fill_dict(rettv->vval.v_dict, mp, buffer_local, true);
}
}
}
static void f_luaeval(typval_T *argvars, typval_T *rettv, FunPtr fptr)
FUNC_ATTR_NONNULL_ALL
{
const char *const str = (const char *)tv_get_string_chk(&argvars[0]);
if (str == NULL) {
return;
}
executor_eval_lua(cstr_as_string((char *)str), &argvars[1], rettv);
}
static void f_map(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
filter_map(argvars, rettv, TRUE);
}
static void f_maparg(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
get_maparg(argvars, rettv, TRUE);
}
static void f_mapcheck(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
get_maparg(argvars, rettv, FALSE);
}
static void find_some_match(typval_T *const argvars, typval_T *const rettv,
const SomeMatchType type)
{
char_u *str = NULL;
long len = 0;
char_u *expr = NULL;
regmatch_T regmatch;
char_u *save_cpo;
long start = 0;
long nth = 1;
colnr_T startcol = 0;
bool match = false;
list_T *l = NULL;
listitem_T *li = NULL;
long idx = 0;
char_u *tofree = NULL;
save_cpo = p_cpo;
p_cpo = (char_u *)"";
rettv->vval.v_number = -1;
switch (type) {
case kSomeMatchList: {
tv_list_alloc_ret(rettv, kListLenMayKnow);
break;
}
case kSomeMatchStrPos: {
tv_list_alloc_ret(rettv, 4);
tv_list_append_string(rettv->vval.v_list, "", 0);
tv_list_append_number(rettv->vval.v_list, -1);
tv_list_append_number(rettv->vval.v_list, -1);
tv_list_append_number(rettv->vval.v_list, -1);
break;
}
case kSomeMatchStr: {
rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;
break;
}
case kSomeMatch:
case kSomeMatchEnd: {
break;
}
}
if (argvars[0].v_type == VAR_LIST) {
if ((l = argvars[0].vval.v_list) == NULL) {
goto theend;
}
li = tv_list_first(l);
} else {
expr = str = (char_u *)tv_get_string(&argvars[0]);
len = (long)STRLEN(str);
}
char patbuf[NUMBUFLEN];
const char *const pat = tv_get_string_buf_chk(&argvars[1], patbuf);
if (pat == NULL) {
goto theend;
}
if (argvars[2].v_type != VAR_UNKNOWN) {
bool error = false;
start = tv_get_number_chk(&argvars[2], &error);
if (error) {
goto theend;
}
if (l != NULL) {
idx = tv_list_uidx(l, start);
if (idx == -1) {
goto theend;
}
li = tv_list_find(l, idx);
} else {
if (start < 0)
start = 0;
if (start > len)
goto theend;
if (argvars[3].v_type != VAR_UNKNOWN)
startcol = start;
else {
str += start;
len -= start;
}
}
if (argvars[3].v_type != VAR_UNKNOWN) {
nth = tv_get_number_chk(&argvars[3], &error);
}
if (error) {
goto theend;
}
}
regmatch.regprog = vim_regcomp((char_u *)pat, RE_MAGIC + RE_STRING);
if (regmatch.regprog != NULL) {
regmatch.rm_ic = p_ic;
for (;; ) {
if (l != NULL) {
if (li == NULL) {
match = false;
break;
}
xfree(tofree);
tofree = expr = str = (char_u *)encode_tv2echo(TV_LIST_ITEM_TV(li),
NULL);
if (str == NULL) {
break;
}
}
match = vim_regexec_nl(&regmatch, str, (colnr_T)startcol);
if (match && --nth <= 0)
break;
if (l == NULL && !match)
break;
if (l != NULL) {
li = TV_LIST_ITEM_NEXT(l, li);
idx++;
} else {
startcol = (colnr_T)(regmatch.startp[0]
+ (*mb_ptr2len)(regmatch.startp[0]) - str);
if (startcol > (colnr_T)len || str + startcol <= regmatch.startp[0]) {
match = false;
break;
}
}
}
if (match) {
switch (type) {
case kSomeMatchStrPos: {
list_T *const ret_l = rettv->vval.v_list;
listitem_T *li1 = tv_list_first(ret_l);
listitem_T *li2 = TV_LIST_ITEM_NEXT(ret_l, li1);
listitem_T *li3 = TV_LIST_ITEM_NEXT(ret_l, li2);
listitem_T *li4 = TV_LIST_ITEM_NEXT(ret_l, li3);
xfree(TV_LIST_ITEM_TV(li1)->vval.v_string);
const size_t rd = (size_t)(regmatch.endp[0] - regmatch.startp[0]);
TV_LIST_ITEM_TV(li1)->vval.v_string = xmemdupz(
(const char *)regmatch.startp[0], rd);
TV_LIST_ITEM_TV(li3)->vval.v_number = (varnumber_T)(
regmatch.startp[0] - expr);
TV_LIST_ITEM_TV(li4)->vval.v_number = (varnumber_T)(
regmatch.endp[0] - expr);
if (l != NULL) {
TV_LIST_ITEM_TV(li2)->vval.v_number = (varnumber_T)idx;
}
break;
}
case kSomeMatchList: {
for (int i = 0; i < NSUBEXP; i++) {
if (regmatch.endp[i] == NULL) {
tv_list_append_string(rettv->vval.v_list, NULL, 0);
} else {
tv_list_append_string(rettv->vval.v_list,
(const char *)regmatch.startp[i],
(regmatch.endp[i] - regmatch.startp[i]));
}
}
break;
}
case kSomeMatchStr: {
if (l != NULL) {
tv_copy(TV_LIST_ITEM_TV(li), rettv);
} else {
rettv->vval.v_string = (char_u *)xmemdupz(
(const char *)regmatch.startp[0],
(size_t)(regmatch.endp[0] - regmatch.startp[0]));
}
break;
}
case kSomeMatch:
case kSomeMatchEnd: {
if (l != NULL) {
rettv->vval.v_number = idx;
} else {
if (type == kSomeMatch) {
rettv->vval.v_number =
(varnumber_T)(regmatch.startp[0] - str);
} else {
rettv->vval.v_number =
(varnumber_T)(regmatch.endp[0] - str);
}
rettv->vval.v_number += (varnumber_T)(str - expr);
}
break;
}
}
}
vim_regfree(regmatch.regprog);
}
theend:
if (type == kSomeMatchStrPos && l == NULL && rettv->vval.v_list != NULL) {
list_T *const ret_l = rettv->vval.v_list;
tv_list_item_remove(ret_l, TV_LIST_ITEM_NEXT(ret_l, tv_list_first(ret_l)));
}
xfree(tofree);
p_cpo = save_cpo;
}
static void f_match(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
find_some_match(argvars, rettv, kSomeMatch);
}
static void f_matchadd(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
char grpbuf[NUMBUFLEN];
char patbuf[NUMBUFLEN];
const char *const grp = tv_get_string_buf_chk(&argvars[0], grpbuf);
const char *const pat = tv_get_string_buf_chk(&argvars[1], patbuf);
int prio = 10;
int id = -1;
bool error = false;
const char *conceal_char = NULL;
win_T *win = curwin;
rettv->vval.v_number = -1;
if (grp == NULL || pat == NULL) {
return;
}
if (argvars[2].v_type != VAR_UNKNOWN) {
prio = tv_get_number_chk(&argvars[2], &error);
if (argvars[3].v_type != VAR_UNKNOWN) {
id = tv_get_number_chk(&argvars[3], &error);
if (argvars[4].v_type != VAR_UNKNOWN
&& matchadd_dict_arg(&argvars[4], &conceal_char, &win) == FAIL) {
return;
}
}
}
if (error) {
return;
}
if (id >= 1 && id <= 3) {
EMSGN(_("E798: ID is reserved for \":match\": %" PRId64), id);
return;
}
rettv->vval.v_number = match_add(win, grp, pat, prio, id, NULL, conceal_char);
}
static void f_matchaddpos(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = -1;
char buf[NUMBUFLEN];
const char *const group = tv_get_string_buf_chk(&argvars[0], buf);
if (group == NULL) {
return;
}
if (argvars[1].v_type != VAR_LIST) {
EMSG2(_(e_listarg), "matchaddpos()");
return;
}
list_T *l;
l = argvars[1].vval.v_list;
if (l == NULL) {
return;
}
bool error = false;
int prio = 10;
int id = -1;
const char *conceal_char = NULL;
win_T *win = curwin;
if (argvars[2].v_type != VAR_UNKNOWN) {
prio = tv_get_number_chk(&argvars[2], &error);
if (argvars[3].v_type != VAR_UNKNOWN) {
id = tv_get_number_chk(&argvars[3], &error);
if (argvars[4].v_type != VAR_UNKNOWN
&& matchadd_dict_arg(&argvars[4], &conceal_char, &win) == FAIL) {
return;
}
}
}
if (error == true) {
return;
}
if (id == 1 || id == 2) {
EMSGN(_("E798: ID is reserved for \"match\": %" PRId64), id);
return;
}
rettv->vval.v_number = match_add(win, group, NULL, prio, id, l, conceal_char);
}
static void f_matcharg(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const int id = tv_get_number(&argvars[0]);
tv_list_alloc_ret(rettv, (id >= 1 && id <= 3
? 2
: 0));
if (id >= 1 && id <= 3) {
matchitem_T *const m = (matchitem_T *)get_match(curwin, id);
if (m != NULL) {
tv_list_append_string(rettv->vval.v_list,
(const char *)syn_id2name(m->hlg_id), -1);
tv_list_append_string(rettv->vval.v_list, (const char *)m->pattern, -1);
} else {
tv_list_append_string(rettv->vval.v_list, NULL, 0);
tv_list_append_string(rettv->vval.v_list, NULL, 0);
}
}
}
static void f_matchdelete(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = match_delete(curwin,
(int)tv_get_number(&argvars[0]), true);
}
static void f_matchend(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
find_some_match(argvars, rettv, kSomeMatchEnd);
}
static void f_matchlist(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
find_some_match(argvars, rettv, kSomeMatchList);
}
static void f_matchstr(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
find_some_match(argvars, rettv, kSomeMatchStr);
}
static void f_matchstrpos(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
find_some_match(argvars, rettv, kSomeMatchStrPos);
}
static void max_min(const typval_T *const tv, typval_T *const rettv,
const bool domax)
FUNC_ATTR_NONNULL_ALL
{
bool error = false;
rettv->vval.v_number = 0;
varnumber_T n = (domax ? VARNUMBER_MIN : VARNUMBER_MAX);
if (tv->v_type == VAR_LIST) {
if (tv_list_len(tv->vval.v_list) == 0) {
return;
}
TV_LIST_ITER_CONST(tv->vval.v_list, li, {
const varnumber_T i = tv_get_number_chk(TV_LIST_ITEM_TV(li), &error);
if (error) {
return;
}
if (domax ? i > n : i < n) {
n = i;
}
});
} else if (tv->v_type == VAR_DICT) {
if (tv_dict_len(tv->vval.v_dict) == 0) {
return;
}
TV_DICT_ITER(tv->vval.v_dict, di, {
const varnumber_T i = tv_get_number_chk(&di->di_tv, &error);
if (error) {
return;
}
if (domax ? i > n : i < n) {
n = i;
}
});
} else {
EMSG2(_(e_listdictarg), domax ? "max()" : "min()");
return;
}
rettv->vval.v_number = n;
}
static void f_max(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
max_min(argvars, rettv, TRUE);
}
static void f_min(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
max_min(argvars, rettv, FALSE);
}
static void f_mkdir(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
int prot = 0755; 
rettv->vval.v_number = FAIL;
if (check_restricted() || check_secure())
return;
char buf[NUMBUFLEN];
const char *const dir = tv_get_string_buf(&argvars[0], buf);
if (*dir == NUL) {
return;
}
if (*path_tail((char_u *)dir) == NUL) {
*path_tail_with_sep((char_u *)dir) = NUL;
}
if (argvars[1].v_type != VAR_UNKNOWN) {
if (argvars[2].v_type != VAR_UNKNOWN) {
prot = tv_get_number_chk(&argvars[2], NULL);
if (prot == -1) {
return;
}
}
if (strcmp(tv_get_string(&argvars[1]), "p") == 0) {
char *failed_dir;
int ret = os_mkdir_recurse(dir, prot, &failed_dir);
if (ret != 0) {
EMSG3(_(e_mkdir), failed_dir, os_strerror(ret));
xfree(failed_dir);
rettv->vval.v_number = FAIL;
return;
} else {
rettv->vval.v_number = OK;
return;
}
}
}
rettv->vval.v_number = vim_mkdir_emsg(dir, prot);
}
static void f_mode(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
char *mode = get_mode();
if (!non_zero_arg(&argvars[0])) {
mode[1] = NUL;
}
rettv->vval.v_string = (char_u *)mode;
rettv->v_type = VAR_STRING;
}
static void f_msgpackdump(typval_T *argvars, typval_T *rettv, FunPtr fptr)
FUNC_ATTR_NONNULL_ALL
{
if (argvars[0].v_type != VAR_LIST) {
EMSG2(_(e_listarg), "msgpackdump()");
return;
}
list_T *const ret_list = tv_list_alloc_ret(rettv, kListLenMayKnow);
list_T *const list = argvars[0].vval.v_list;
msgpack_packer *lpacker = msgpack_packer_new(ret_list, &encode_list_write);
const char *const msg = _("msgpackdump() argument, index %i");
char msgbuf[sizeof("msgpackdump() argument, index ") * 4 + NUMBUFLEN];
int idx = 0;
TV_LIST_ITER(list, li, {
vim_snprintf(msgbuf, sizeof(msgbuf), (char *)msg, idx);
idx++;
if (encode_vim_to_msgpack(lpacker, TV_LIST_ITEM_TV(li), msgbuf) == FAIL) {
break;
}
});
msgpack_packer_free(lpacker);
}
static void f_msgpackparse(typval_T *argvars, typval_T *rettv, FunPtr fptr)
FUNC_ATTR_NONNULL_ALL
{
if (argvars[0].v_type != VAR_LIST) {
EMSG2(_(e_listarg), "msgpackparse()");
return;
}
list_T *const ret_list = tv_list_alloc_ret(rettv, kListLenMayKnow);
const list_T *const list = argvars[0].vval.v_list;
if (tv_list_len(list) == 0) {
return;
}
if (TV_LIST_ITEM_TV(tv_list_first(list))->v_type != VAR_STRING) {
EMSG2(_(e_invarg2), "List item is not a string");
return;
}
ListReaderState lrstate = encode_init_lrstate(list);
msgpack_unpacker *const unpacker = msgpack_unpacker_new(IOSIZE);
if (unpacker == NULL) {
EMSG(_(e_outofmem));
return;
}
msgpack_unpacked unpacked;
msgpack_unpacked_init(&unpacked);
do {
if (!msgpack_unpacker_reserve_buffer(unpacker, IOSIZE)) {
EMSG(_(e_outofmem));
goto f_msgpackparse_exit;
}
size_t read_bytes;
const int rlret = encode_read_from_list(
&lrstate, msgpack_unpacker_buffer(unpacker), IOSIZE, &read_bytes);
if (rlret == FAIL) {
EMSG2(_(e_invarg2), "List item is not a string");
goto f_msgpackparse_exit;
}
msgpack_unpacker_buffer_consumed(unpacker, read_bytes);
if (read_bytes == 0) {
break;
}
while (unpacker->off < unpacker->used) {
const msgpack_unpack_return result = msgpack_unpacker_next(unpacker,
&unpacked);
if (result == MSGPACK_UNPACK_PARSE_ERROR) {
EMSG2(_(e_invarg2), "Failed to parse msgpack string");
goto f_msgpackparse_exit;
}
if (result == MSGPACK_UNPACK_NOMEM_ERROR) {
EMSG(_(e_outofmem));
goto f_msgpackparse_exit;
}
if (result == MSGPACK_UNPACK_SUCCESS) {
typval_T tv = { .v_type = VAR_UNKNOWN };
if (msgpack_to_vim(unpacked.data, &tv) == FAIL) {
EMSG2(_(e_invarg2), "Failed to convert msgpack string");
goto f_msgpackparse_exit;
}
tv_list_append_owned_tv(ret_list, tv);
}
if (result == MSGPACK_UNPACK_CONTINUE) {
if (rlret == OK) {
EMSG2(_(e_invarg2), "Incomplete msgpack string");
}
break;
}
}
if (rlret == OK) {
break;
}
} while (true);
f_msgpackparse_exit:
msgpack_unpacked_destroy(&unpacked);
msgpack_unpacker_free(unpacker);
return;
}
static void f_nextnonblank(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
linenr_T lnum;
for (lnum = tv_get_lnum(argvars);; lnum++) {
if (lnum < 0 || lnum > curbuf->b_ml.ml_line_count) {
lnum = 0;
break;
}
if (*skipwhite(ml_get(lnum)) != NUL) {
break;
}
}
rettv->vval.v_number = lnum;
}
static void f_nr2char(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
if (argvars[1].v_type != VAR_UNKNOWN) {
if (!tv_check_num(&argvars[1])) {
return;
}
}
bool error = false;
const varnumber_T num = tv_get_number_chk(&argvars[0], &error);
if (error) {
return;
}
if (num < 0) {
EMSG(_("E5070: Character number must not be less than zero"));
return;
}
if (num > INT_MAX) {
emsgf(_("E5071: Character number must not be greater than INT_MAX (%i)"),
INT_MAX);
return;
}
char buf[MB_MAXBYTES];
const int len = utf_char2bytes((int)num, (char_u *)buf);
rettv->v_type = VAR_STRING;
rettv->vval.v_string = xmemdupz(buf, (size_t)len);
}
static void f_or(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = tv_get_number_chk(&argvars[0], NULL)
| tv_get_number_chk(&argvars[1], NULL);
}
static void f_pathshorten(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_STRING;
const char *const s = tv_get_string_chk(&argvars[0]);
if (!s) {
return;
}
rettv->vval.v_string = shorten_dir((char_u *)xstrdup(s));
}
static void f_pow(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
float_T fx;
float_T fy;
rettv->v_type = VAR_FLOAT;
if (tv_get_float_chk(argvars, &fx) && tv_get_float_chk(&argvars[1], &fy)) {
rettv->vval.v_float = pow(fx, fy);
} else {
rettv->vval.v_float = 0.0;
}
}
static void f_prevnonblank(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
linenr_T lnum = tv_get_lnum(argvars);
if (lnum < 1 || lnum > curbuf->b_ml.ml_line_count) {
lnum = 0;
} else {
while (lnum >= 1 && *skipwhite(ml_get(lnum)) == NUL) {
lnum--;
}
}
rettv->vval.v_number = lnum;
}
static void f_printf(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;
{
int len;
int saved_did_emsg = did_emsg;
did_emsg = false;
char buf[NUMBUFLEN];
const char *fmt = tv_get_string_buf(&argvars[0], buf);
len = vim_vsnprintf_typval(NULL, 0, fmt, dummy_ap, argvars + 1);
if (!did_emsg) {
char *s = xmalloc(len + 1);
rettv->vval.v_string = (char_u *)s;
(void)vim_vsnprintf_typval(s, len + 1, fmt, dummy_ap, argvars + 1);
}
did_emsg |= saved_did_emsg;
}
}
static void f_prompt_setcallback(typval_T *argvars,
typval_T *rettv, FunPtr fptr)
{
buf_T *buf;
Callback prompt_callback = { .type = kCallbackNone };
if (check_secure()) {
return;
}
buf = tv_get_buf(&argvars[0], false);
if (buf == NULL) {
return;
}
if (argvars[1].v_type != VAR_STRING || *argvars[1].vval.v_string != NUL) {
if (!callback_from_typval(&prompt_callback, &argvars[1])) {
return;
}
}
callback_free(&buf->b_prompt_callback);
buf->b_prompt_callback = prompt_callback;
}
static void f_prompt_setinterrupt(typval_T *argvars,
typval_T *rettv, FunPtr fptr)
{
buf_T *buf;
Callback interrupt_callback = { .type = kCallbackNone };
if (check_secure()) {
return;
}
buf = tv_get_buf(&argvars[0], false);
if (buf == NULL) {
return;
}
if (argvars[1].v_type != VAR_STRING || *argvars[1].vval.v_string != NUL) {
if (!callback_from_typval(&interrupt_callback, &argvars[1])) {
return;
}
}
callback_free(&buf->b_prompt_interrupt);
buf->b_prompt_interrupt= interrupt_callback;
}
static void f_prompt_setprompt(typval_T *argvars,
typval_T *rettv, FunPtr fptr)
{
buf_T *buf;
const char_u *text;
if (check_secure()) {
return;
}
buf = tv_get_buf(&argvars[0], false);
if (buf == NULL) {
return;
}
text = (const char_u *)tv_get_string(&argvars[1]);
xfree(buf->b_prompt_text);
buf->b_prompt_text = vim_strsave(text);
}
static void f_pum_getpos(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
tv_dict_alloc_ret(rettv);
pum_set_event_info(rettv->vval.v_dict);
}
static void f_pumvisible(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
if (pum_visible())
rettv->vval.v_number = 1;
}
static void f_pyeval(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
script_host_eval("python", argvars, rettv);
}
static void f_py3eval(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
script_host_eval("python3", argvars, rettv);
}
static void f_pyxeval(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
init_pyxversion();
if (p_pyx == 2) {
f_pyeval(argvars, rettv, NULL);
} else {
f_py3eval(argvars, rettv, NULL);
}
}
static void f_range(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
varnumber_T start;
varnumber_T end;
varnumber_T stride = 1;
varnumber_T i;
bool error = false;
start = tv_get_number_chk(&argvars[0], &error);
if (argvars[1].v_type == VAR_UNKNOWN) {
end = start - 1;
start = 0;
} else {
end = tv_get_number_chk(&argvars[1], &error);
if (argvars[2].v_type != VAR_UNKNOWN) {
stride = tv_get_number_chk(&argvars[2], &error);
}
}
if (error) {
return; 
}
if (stride == 0) {
EMSG(_("E726: Stride is zero"));
} else if (stride > 0 ? end + 1 < start : end - 1 > start) {
EMSG(_("E727: Start past end"));
} else {
tv_list_alloc_ret(rettv, (end - start) / stride);
for (i = start; stride > 0 ? i <= end : i >= end; i += stride) {
tv_list_append_number(rettv->vval.v_list, (varnumber_T)i);
}
}
}
static void f_readfile(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
bool binary = false;
FILE *fd;
char_u buf[(IOSIZE/256)*256]; 
int io_size = sizeof(buf);
int readlen; 
char_u *prev = NULL; 
long prevlen = 0; 
long prevsize = 0; 
long maxline = MAXLNUM;
if (argvars[1].v_type != VAR_UNKNOWN) {
if (strcmp(tv_get_string(&argvars[1]), "b") == 0) {
binary = true;
}
if (argvars[2].v_type != VAR_UNKNOWN) {
maxline = tv_get_number(&argvars[2]);
}
}
list_T *const l = tv_list_alloc_ret(rettv, kListLenUnknown);
const char *const fname = tv_get_string(&argvars[0]);
if (*fname == NUL || (fd = os_fopen(fname, READBIN)) == NULL) {
EMSG2(_(e_notopen), *fname == NUL ? _("<empty>") : fname);
return;
}
while (maxline < 0 || tv_list_len(l) < maxline) {
readlen = (int)fread(buf, 1, io_size, fd);
char_u *p; 
char_u *start; 
for (p = buf, start = buf;
p < buf + readlen || (readlen <= 0 && (prevlen > 0 || binary));
p++) {
if (*p == '\n' || readlen <= 0) {
char_u *s = NULL;
size_t len = p - start;
if (readlen > 0 && !binary) {
while (len > 0 && start[len - 1] == '\r')
--len;
if (len == 0)
while (prevlen > 0 && prev[prevlen - 1] == '\r')
--prevlen;
}
if (prevlen == 0) {
assert(len < INT_MAX);
s = vim_strnsave(start, (int)len);
} else {
s = xrealloc(prev, prevlen + len + 1);
memcpy(s + prevlen, start, len);
s[prevlen + len] = NUL;
prev = NULL; 
prevlen = prevsize = 0;
}
tv_list_append_owned_tv(l, (typval_T) {
.v_type = VAR_STRING,
.v_lock = VAR_UNLOCKED,
.vval.v_string = s,
});
start = p + 1; 
if (maxline < 0) {
if (tv_list_len(l) > -maxline) {
assert(tv_list_len(l) == 1 + (-maxline));
tv_list_item_remove(l, tv_list_first(l));
}
} else if (tv_list_len(l) >= maxline) {
assert(tv_list_len(l) == maxline);
break;
}
if (readlen <= 0) {
break;
}
} else if (*p == NUL) {
*p = '\n';
} else if (*p == 0xbf && !binary) {
char_u back1 = p >= buf + 1 ? p[-1]
: prevlen >= 1 ? prev[prevlen - 1] : NUL;
char_u back2 = p >= buf + 2 ? p[-2]
: p == buf + 1 && prevlen >= 1 ? prev[prevlen - 1]
: prevlen >= 2 ? prev[prevlen - 2] : NUL;
if (back2 == 0xef && back1 == 0xbb) {
char_u *dest = p - 2;
if (start == dest)
start = p + 1;
else {
int adjust_prevlen = 0;
if (dest < buf) { 
adjust_prevlen = (int)(buf - dest); 
dest = buf;
}
if (readlen > p - buf + 1)
memmove(dest, p + 1, readlen - (p - buf) - 1);
readlen -= 3 - adjust_prevlen;
prevlen -= adjust_prevlen;
p = dest - 1;
}
}
}
} 
if ((maxline >= 0 && tv_list_len(l) >= maxline) || readlen <= 0) {
break;
}
if (start < p) {
if (p - start + prevlen >= prevsize) {
if (prevsize == 0)
prevsize = (long)(p - start);
else {
long grow50pc = (prevsize * 3) / 2;
long growmin = (long)((p - start) * 2 + prevlen);
prevsize = grow50pc > growmin ? grow50pc : growmin;
}
prev = xrealloc(prev, prevsize);
}
memmove(prev + prevlen, start, p - start);
prevlen += (long)(p - start);
}
} 
xfree(prev);
fclose(fd);
}
static void f_reg_executing(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
return_register(reg_executing, rettv);
}
static void f_reg_recording(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
return_register(reg_recording, rettv);
}
static int list2proftime(typval_T *arg, proftime_T *tm) FUNC_ATTR_NONNULL_ALL
{
if (arg->v_type != VAR_LIST || tv_list_len(arg->vval.v_list) != 2) {
return FAIL;
}
bool error = false;
varnumber_T n1 = tv_list_find_nr(arg->vval.v_list, 0L, &error);
varnumber_T n2 = tv_list_find_nr(arg->vval.v_list, 1L, &error);
if (error) {
return FAIL;
}
union {
struct { int32_t low, high; } split;
proftime_T prof;
} u = { .split.high = n1, .split.low = n2 };
*tm = u.prof;
return OK;
}
static void f_reltime(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
proftime_T res;
proftime_T start;
if (argvars[0].v_type == VAR_UNKNOWN) {
res = profile_start();
} else if (argvars[1].v_type == VAR_UNKNOWN) {
if (list2proftime(&argvars[0], &res) == FAIL) {
return;
}
res = profile_end(res);
} else {
if (list2proftime(&argvars[0], &start) == FAIL
|| list2proftime(&argvars[1], &res) == FAIL) {
return;
}
res = profile_sub(res, start);
}
union {
struct { int32_t low, high; } split;
proftime_T prof;
} u = { .prof = res };
STATIC_ASSERT(sizeof(u.prof) == sizeof(u) && sizeof(u.split) == sizeof(u),
"type punning will produce incorrect results on this platform");
tv_list_alloc_ret(rettv, 2);
tv_list_append_number(rettv->vval.v_list, u.split.high);
tv_list_append_number(rettv->vval.v_list, u.split.low);
}
static void f_reltimestr(typval_T *argvars, typval_T *rettv, FunPtr fptr)
FUNC_ATTR_NONNULL_ALL
{
proftime_T tm;
rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;
if (list2proftime(&argvars[0], &tm) == OK) {
rettv->vval.v_string = (char_u *)xstrdup(profile_msg(tm));
}
}
static void f_remove(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
list_T *l;
listitem_T *item, *item2;
listitem_T *li;
long idx;
long end;
dict_T *d;
dictitem_T *di;
const char *const arg_errmsg = N_("remove() argument");
if (argvars[0].v_type == VAR_DICT) {
if (argvars[2].v_type != VAR_UNKNOWN) {
EMSG2(_(e_toomanyarg), "remove()");
} else if ((d = argvars[0].vval.v_dict) != NULL
&& !tv_check_lock(d->dv_lock, arg_errmsg, TV_TRANSLATE)) {
const char *key = tv_get_string_chk(&argvars[1]);
if (key != NULL) {
di = tv_dict_find(d, key, -1);
if (di == NULL) {
EMSG2(_(e_dictkey), key);
} else if (!var_check_fixed(di->di_flags, arg_errmsg, TV_TRANSLATE)
&& !var_check_ro(di->di_flags, arg_errmsg, TV_TRANSLATE)) {
*rettv = di->di_tv;
di->di_tv = TV_INITIAL_VALUE;
tv_dict_item_remove(d, di);
if (tv_dict_is_watched(d)) {
tv_dict_watcher_notify(d, key, NULL, rettv);
}
}
}
}
} else if (argvars[0].v_type != VAR_LIST) {
EMSG2(_(e_listdictarg), "remove()");
} else if (!tv_check_lock(tv_list_locked((l = argvars[0].vval.v_list)),
arg_errmsg, TV_TRANSLATE)) {
bool error = false;
idx = tv_get_number_chk(&argvars[1], &error);
if (error) {
} else if ((item = tv_list_find(l, idx)) == NULL) {
EMSGN(_(e_listidx), idx);
} else {
if (argvars[2].v_type == VAR_UNKNOWN) {
tv_list_drop_items(l, item, item);
*rettv = *TV_LIST_ITEM_TV(item);
xfree(item);
} else {
end = tv_get_number_chk(&argvars[2], &error);
if (error) {
} else if ((item2 = tv_list_find(l, end)) == NULL) {
EMSGN(_(e_listidx), end);
} else {
int cnt = 0;
for (li = item; li != NULL; li = TV_LIST_ITEM_NEXT(l, li)) {
cnt++;
if (li == item2) {
break;
}
}
if (li == NULL) { 
EMSG(_(e_invrange));
} else {
tv_list_move_items(l, item, item2, tv_list_alloc_ret(rettv, cnt),
cnt);
}
}
}
}
}
}
static void f_rename(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
if (check_restricted() || check_secure()) {
rettv->vval.v_number = -1;
} else {
char buf[NUMBUFLEN];
rettv->vval.v_number = vim_rename(
(const char_u *)tv_get_string(&argvars[0]),
(const char_u *)tv_get_string_buf(&argvars[1], buf));
}
}
static void f_repeat(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
varnumber_T n = tv_get_number(&argvars[1]);
if (argvars[0].v_type == VAR_LIST) {
tv_list_alloc_ret(rettv, (n > 0) * n * tv_list_len(argvars[0].vval.v_list));
while (n-- > 0) {
tv_list_extend(rettv->vval.v_list, argvars[0].vval.v_list, NULL);
}
} else {
rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;
if (n <= 0) {
return;
}
const char *const p = tv_get_string(&argvars[0]);
const size_t slen = strlen(p);
if (slen == 0) {
return;
}
const size_t len = slen * n;
if (len / n != slen) {
return;
}
char *const r = xmallocz(len);
for (varnumber_T i = 0; i < n; i++) {
memmove(r + i * slen, p, slen);
}
rettv->vval.v_string = (char_u *)r;
}
}
static void f_resolve(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_STRING;
const char *fname = tv_get_string(&argvars[0]);
#if defined(WIN32)
char *const v = os_resolve_shortcut(fname);
rettv->vval.v_string = (char_u *)(v == NULL ? xstrdup(fname) : v);
#else
#if defined(HAVE_READLINK)
{
bool is_relative_to_current = false;
bool has_trailing_pathsep = false;
int limit = 100;
char *p = xstrdup(fname);
if (p[0] == '.' && (vim_ispathsep(p[1])
|| (p[1] == '.' && (vim_ispathsep(p[2]))))) {
is_relative_to_current = true;
}
ptrdiff_t len = (ptrdiff_t)strlen(p);
if (len > 0 && after_pathsep(p, p + len)) {
has_trailing_pathsep = true;
p[len - 1] = NUL; 
}
char *q = (char *)path_next_component(p);
char *remain = NULL;
if (*q != NUL) {
remain = xstrdup(q - 1);
q[-1] = NUL;
}
char *const buf = xmallocz(MAXPATHL);
char *cpy;
for (;; ) {
for (;; ) {
len = readlink(p, buf, MAXPATHL);
if (len <= 0) {
break;
}
buf[len] = NUL;
if (limit-- == 0) {
xfree(p);
xfree(remain);
EMSG(_("E655: Too many symbolic links (cycle?)"));
rettv->vval.v_string = NULL;
xfree(buf);
return;
}
if (remain == NULL && has_trailing_pathsep) {
add_pathsep(buf);
}
q = (char *)path_next_component(vim_ispathsep(*buf) ? buf + 1 : buf);
if (*q != NUL) {
cpy = remain;
remain = (remain
? (char *)concat_str((char_u *)q - 1, (char_u *)remain)
: xstrdup(q - 1));
xfree(cpy);
q[-1] = NUL;
}
q = (char *)path_tail((char_u *)p);
if (q > p && *q == NUL) {
q[-1] = NUL;
q = (char *)path_tail((char_u *)p);
}
if (q > p && !path_is_absolute((const char_u *)buf)) {
const size_t p_len = strlen(p);
const size_t buf_len = strlen(buf);
p = xrealloc(p, p_len + buf_len + 1);
memcpy(path_tail((char_u *)p), buf, buf_len + 1);
} else {
xfree(p);
p = xstrdup(buf);
}
}
if (remain == NULL) {
break;
}
q = (char *)path_next_component(remain + 1);
len = q - remain - (*q != NUL);
const size_t p_len = strlen(p);
cpy = xmallocz(p_len + len);
memcpy(cpy, p, p_len + 1);
xstrlcat(cpy + p_len, remain, len + 1);
xfree(p);
p = cpy;
if (*q != NUL) {
STRMOVE(remain, q - 1);
} else {
XFREE_CLEAR(remain);
}
}
if (!vim_ispathsep(*p)) {
if (is_relative_to_current
&& *p != NUL
&& !(p[0] == '.'
&& (p[1] == NUL
|| vim_ispathsep(p[1])
|| (p[1] == '.'
&& (p[2] == NUL
|| vim_ispathsep(p[2])))))) {
cpy = (char *)concat_str((const char_u *)"./", (const char_u *)p);
xfree(p);
p = cpy;
} else if (!is_relative_to_current) {
q = p;
while (q[0] == '.' && vim_ispathsep(q[1])) {
q += 2;
}
if (q > p) {
STRMOVE(p, p + 2);
}
}
}
if (!has_trailing_pathsep) {
q = p + strlen(p);
if (after_pathsep(p, q)) {
*path_tail_with_sep((char_u *)p) = NUL;
}
}
rettv->vval.v_string = (char_u *)p;
xfree(buf);
}
#else
rettv->vval.v_string = (char_u *)xstrdup(p);
#endif
#endif
simplify_filename(rettv->vval.v_string);
}
static void f_reverse(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
list_T *l;
if (argvars[0].v_type != VAR_LIST) {
EMSG2(_(e_listarg), "reverse()");
} else if (!tv_check_lock(tv_list_locked((l = argvars[0].vval.v_list)),
N_("reverse() argument"), TV_TRANSLATE)) {
tv_list_reverse(l);
tv_list_set_ret(rettv, l);
}
}
#define SP_NOMOVE 0x01 
#define SP_REPEAT 0x02 
#define SP_RETCOUNT 0x04 
#define SP_SETPCMARK 0x08 
#define SP_START 0x10 
#define SP_SUBPAT 0x20 
#define SP_END 0x40 
#define SP_COLUMN 0x80 
static int get_search_arg(typval_T *varp, int *flagsp)
{
int dir = FORWARD;
int mask;
if (varp->v_type != VAR_UNKNOWN) {
char nbuf[NUMBUFLEN];
const char *flags = tv_get_string_buf_chk(varp, nbuf);
if (flags == NULL) {
return 0; 
}
while (*flags != NUL) {
switch (*flags) {
case 'b': dir = BACKWARD; break;
case 'w': p_ws = true; break;
case 'W': p_ws = false; break;
default: {
mask = 0;
if (flagsp != NULL) {
switch (*flags) {
case 'c': mask = SP_START; break;
case 'e': mask = SP_END; break;
case 'm': mask = SP_RETCOUNT; break;
case 'n': mask = SP_NOMOVE; break;
case 'p': mask = SP_SUBPAT; break;
case 'r': mask = SP_REPEAT; break;
case 's': mask = SP_SETPCMARK; break;
case 'z': mask = SP_COLUMN; break;
}
}
if (mask == 0) {
emsgf(_(e_invarg2), flags);
dir = 0;
} else {
*flagsp |= mask;
}
}
}
if (dir == 0) {
break;
}
flags++;
}
}
return dir;
}
static int search_cmn(typval_T *argvars, pos_T *match_pos, int *flagsp)
{
int flags;
pos_T pos;
pos_T save_cursor;
bool save_p_ws = p_ws;
int dir;
int retval = 0; 
long lnum_stop = 0;
proftime_T tm;
long time_limit = 0;
int options = SEARCH_KEEP;
int subpatnum;
searchit_arg_T sia;
const char *const pat = tv_get_string(&argvars[0]);
dir = get_search_arg(&argvars[1], flagsp); 
if (dir == 0) {
goto theend;
}
flags = *flagsp;
if (flags & SP_START) {
options |= SEARCH_START;
}
if (flags & SP_END) {
options |= SEARCH_END;
}
if (flags & SP_COLUMN) {
options |= SEARCH_COL;
}
if (argvars[1].v_type != VAR_UNKNOWN && argvars[2].v_type != VAR_UNKNOWN) {
lnum_stop = tv_get_number_chk(&argvars[2], NULL);
if (lnum_stop < 0) {
goto theend;
}
if (argvars[3].v_type != VAR_UNKNOWN) {
time_limit = tv_get_number_chk(&argvars[3], NULL);
if (time_limit < 0) {
goto theend;
}
}
}
tm = profile_setlimit(time_limit);
if (((flags & (SP_REPEAT | SP_RETCOUNT)) != 0)
|| ((flags & SP_NOMOVE) && (flags & SP_SETPCMARK))) {
EMSG2(_(e_invarg2), tv_get_string(&argvars[1]));
goto theend;
}
pos = save_cursor = curwin->w_cursor;
memset(&sia, 0, sizeof(sia));
sia.sa_stop_lnum = (linenr_T)lnum_stop;
sia.sa_tm = &tm;
subpatnum = searchit(curwin, curbuf, &pos, NULL, dir, (char_u *)pat, 1,
options, RE_SEARCH, &sia);
if (subpatnum != FAIL) {
if (flags & SP_SUBPAT)
retval = subpatnum;
else
retval = pos.lnum;
if (flags & SP_SETPCMARK)
setpcmark();
curwin->w_cursor = pos;
if (match_pos != NULL) {
match_pos->lnum = pos.lnum;
match_pos->col = pos.col + 1;
}
check_cursor();
}
if (flags & SP_NOMOVE)
curwin->w_cursor = save_cursor;
else
curwin->w_set_curswant = TRUE;
theend:
p_ws = save_p_ws;
return retval;
}
static void f_rpcnotify(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = 0;
if (check_restricted() || check_secure()) {
return;
}
if (argvars[0].v_type != VAR_NUMBER || argvars[0].vval.v_number < 0) {
EMSG2(_(e_invarg2), "Channel id must be a positive integer");
return;
}
if (argvars[1].v_type != VAR_STRING) {
EMSG2(_(e_invarg2), "Event type must be a string");
return;
}
Array args = ARRAY_DICT_INIT;
for (typval_T *tv = argvars + 2; tv->v_type != VAR_UNKNOWN; tv++) {
ADD(args, vim_to_object(tv));
}
if (!rpc_send_event((uint64_t)argvars[0].vval.v_number,
tv_get_string(&argvars[1]), args)) {
EMSG2(_(e_invarg2), "Channel doesn't exist");
return;
}
rettv->vval.v_number = 1;
}
static void f_rpcrequest(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = 0;
const int l_provider_call_nesting = provider_call_nesting;
if (check_restricted() || check_secure()) {
return;
}
if (argvars[0].v_type != VAR_NUMBER || argvars[0].vval.v_number <= 0) {
EMSG2(_(e_invarg2), "Channel id must be a positive integer");
return;
}
if (argvars[1].v_type != VAR_STRING) {
EMSG2(_(e_invarg2), "Method name must be a string");
return;
}
Array args = ARRAY_DICT_INIT;
for (typval_T *tv = argvars + 2; tv->v_type != VAR_UNKNOWN; tv++) {
ADD(args, vim_to_object(tv));
}
sctx_T save_current_sctx;
uint8_t *save_sourcing_name, *save_autocmd_fname, *save_autocmd_match;
linenr_T save_sourcing_lnum;
int save_autocmd_bufnr;
void *save_funccalp;
if (l_provider_call_nesting) {
save_current_sctx = current_sctx;
save_sourcing_name = sourcing_name;
save_sourcing_lnum = sourcing_lnum;
save_autocmd_fname = autocmd_fname;
save_autocmd_match = autocmd_match;
save_autocmd_bufnr = autocmd_bufnr;
save_funccalp = save_funccal();
current_sctx = provider_caller_scope.script_ctx;
sourcing_name = provider_caller_scope.sourcing_name;
sourcing_lnum = provider_caller_scope.sourcing_lnum;
autocmd_fname = provider_caller_scope.autocmd_fname;
autocmd_match = provider_caller_scope.autocmd_match;
autocmd_bufnr = provider_caller_scope.autocmd_bufnr;
restore_funccal(provider_caller_scope.funccalp);
}
Error err = ERROR_INIT;
uint64_t chan_id = (uint64_t)argvars[0].vval.v_number;
const char *method = tv_get_string(&argvars[1]);
Object result = rpc_send_call(chan_id, method, args, &err);
if (l_provider_call_nesting) {
current_sctx = save_current_sctx;
sourcing_name = save_sourcing_name;
sourcing_lnum = save_sourcing_lnum;
autocmd_fname = save_autocmd_fname;
autocmd_match = save_autocmd_match;
autocmd_bufnr = save_autocmd_bufnr;
restore_funccal(save_funccalp);
}
if (ERROR_SET(&err)) {
const char *name = NULL;
Channel *chan = find_channel(chan_id);
if (chan) {
name = rpc_client_name(chan);
}
msg_ext_set_kind("rpc_error");
if (name) {
emsgf_multiline("Error invoking '%s' on channel %"PRIu64" (%s):\n%s",
method, chan_id, name, err.msg);
} else {
emsgf_multiline("Error invoking '%s' on channel %"PRIu64":\n%s",
method, chan_id, err.msg);
}
goto end;
}
if (!object_to_vim(result, rettv, &err)) {
EMSG2(_("Error converting the call result: %s"), err.msg);
}
end:
api_free_object(result);
api_clear_error(&err);
}
static void f_rpcstart(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = 0;
if (check_restricted() || check_secure()) {
return;
}
if (argvars[0].v_type != VAR_STRING
|| (argvars[1].v_type != VAR_LIST && argvars[1].v_type != VAR_UNKNOWN)) {
EMSG(_(e_invarg));
return;
}
list_T *args = NULL;
int argsl = 0;
if (argvars[1].v_type == VAR_LIST) {
args = argvars[1].vval.v_list;
argsl = tv_list_len(args);
int i = 0;
TV_LIST_ITER_CONST(args, arg, {
if (TV_LIST_ITEM_TV(arg)->v_type != VAR_STRING) {
emsgf(_("E5010: List item %d of the second argument is not a string"),
i);
return;
}
i++;
});
}
if (argvars[0].vval.v_string == NULL || argvars[0].vval.v_string[0] == NUL) {
EMSG(_(e_api_spawn_failed));
return;
}
int argvl = argsl + 2;
char **argv = xmalloc(sizeof(char_u *) * argvl);
argv[0] = xstrdup((char *)argvars[0].vval.v_string);
int i = 1;
if (argsl > 0) {
TV_LIST_ITER_CONST(args, arg, {
argv[i++] = xstrdup(tv_get_string(TV_LIST_ITEM_TV(arg)));
});
}
argv[i] = NULL;
Channel *chan = channel_job_start(argv, CALLBACK_READER_INIT,
CALLBACK_READER_INIT, CALLBACK_NONE,
false, true, false, NULL, 0, 0, NULL, NULL,
&rettv->vval.v_number);
if (chan) {
channel_create_event(chan, NULL);
}
}
static void f_rpcstop(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = 0;
if (check_restricted() || check_secure()) {
return;
}
if (argvars[0].v_type != VAR_NUMBER) {
EMSG(_(e_invarg));
return;
}
uint64_t id = argvars[0].vval.v_number;
if (find_job(id, false)) {
f_jobstop(argvars, rettv, NULL);
} else {
const char *error;
rettv->vval.v_number = channel_close(argvars[0].vval.v_number,
kChannelPartRpc, &error);
if (!rettv->vval.v_number) {
EMSG(error);
}
}
}
static void f_screenattr(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
int c;
int row = (int)tv_get_number_chk(&argvars[0], NULL) - 1;
int col = (int)tv_get_number_chk(&argvars[1], NULL) - 1;
if (row < 0 || row >= default_grid.Rows
|| col < 0 || col >= default_grid.Columns) {
c = -1;
} else {
ScreenGrid *grid = &default_grid;
screenchar_adjust_grid(&grid, &row, &col);
c = grid->attrs[grid->line_offset[row] + col];
}
rettv->vval.v_number = c;
}
static void f_screenchar(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
int c;
int row = tv_get_number_chk(&argvars[0], NULL) - 1;
int col = tv_get_number_chk(&argvars[1], NULL) - 1;
if (row < 0 || row >= default_grid.Rows
|| col < 0 || col >= default_grid.Columns) {
c = -1;
} else {
ScreenGrid *grid = &default_grid;
screenchar_adjust_grid(&grid, &row, &col);
c = utf_ptr2char(grid->chars[grid->line_offset[row] + col]);
}
rettv->vval.v_number = c;
}
static void f_screencol(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = ui_current_col() + 1;
}
static void f_screenpos(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
pos_T pos;
int row = 0;
int scol = 0, ccol = 0, ecol = 0;
tv_dict_alloc_ret(rettv);
dict_T *dict = rettv->vval.v_dict;
win_T *wp = find_win_by_nr_or_id(&argvars[0]);
if (wp == NULL) {
return;
}
pos.lnum = tv_get_number(&argvars[1]);
pos.col = tv_get_number(&argvars[2]) - 1;
pos.coladd = 0;
textpos2screenpos(wp, &pos, &row, &scol, &ccol, &ecol, false);
tv_dict_add_nr(dict, S_LEN("row"), row);
tv_dict_add_nr(dict, S_LEN("col"), scol);
tv_dict_add_nr(dict, S_LEN("curscol"), ccol);
tv_dict_add_nr(dict, S_LEN("endcol"), ecol);
}
static void f_screenrow(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = ui_current_row() + 1;
}
static void f_search(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
int flags = 0;
rettv->vval.v_number = search_cmn(argvars, NULL, &flags);
}
static void f_searchdecl(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
int locally = 1;
int thisblock = 0;
bool error = false;
rettv->vval.v_number = 1; 
const char *const name = tv_get_string_chk(&argvars[0]);
if (argvars[1].v_type != VAR_UNKNOWN) {
locally = tv_get_number_chk(&argvars[1], &error) == 0;
if (!error && argvars[2].v_type != VAR_UNKNOWN) {
thisblock = tv_get_number_chk(&argvars[2], &error) != 0;
}
}
if (!error && name != NULL) {
rettv->vval.v_number = find_decl((char_u *)name, strlen(name), locally,
thisblock, SEARCH_KEEP) == FAIL;
}
}
static int searchpair_cmn(typval_T *argvars, pos_T *match_pos)
{
bool save_p_ws = p_ws;
int dir;
int flags = 0;
int retval = 0; 
long lnum_stop = 0;
long time_limit = 0;
char nbuf1[NUMBUFLEN];
char nbuf2[NUMBUFLEN];
const char *spat = tv_get_string_chk(&argvars[0]);
const char *mpat = tv_get_string_buf_chk(&argvars[1], nbuf1);
const char *epat = tv_get_string_buf_chk(&argvars[2], nbuf2);
if (spat == NULL || mpat == NULL || epat == NULL) {
goto theend; 
}
dir = get_search_arg(&argvars[3], &flags); 
if (dir == 0) {
goto theend;
}
if ((flags & (SP_END | SP_SUBPAT)) != 0
|| ((flags & SP_NOMOVE) && (flags & SP_SETPCMARK))) {
EMSG2(_(e_invarg2), tv_get_string(&argvars[3]));
goto theend;
}
if (flags & SP_REPEAT) {
p_ws = false;
}
const typval_T *skip;
if (argvars[3].v_type == VAR_UNKNOWN
|| argvars[4].v_type == VAR_UNKNOWN) {
skip = NULL;
} else {
skip = &argvars[4];
if (skip->v_type != VAR_FUNC
&& skip->v_type != VAR_PARTIAL
&& skip->v_type != VAR_STRING) {
emsgf(_(e_invarg2), tv_get_string(&argvars[4]));
goto theend; 
}
if (argvars[5].v_type != VAR_UNKNOWN) {
lnum_stop = tv_get_number_chk(&argvars[5], NULL);
if (lnum_stop < 0) {
emsgf(_(e_invarg2), tv_get_string(&argvars[5]));
goto theend;
}
if (argvars[6].v_type != VAR_UNKNOWN) {
time_limit = tv_get_number_chk(&argvars[6], NULL);
if (time_limit < 0) {
emsgf(_(e_invarg2), tv_get_string(&argvars[6]));
goto theend;
}
}
}
}
retval = do_searchpair(
(char_u *)spat, (char_u *)mpat, (char_u *)epat, dir, skip,
flags, match_pos, lnum_stop, time_limit);
theend:
p_ws = save_p_ws;
return retval;
}
static void f_searchpair(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = searchpair_cmn(argvars, NULL);
}
static void f_searchpairpos(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
pos_T match_pos;
int lnum = 0;
int col = 0;
tv_list_alloc_ret(rettv, 2);
if (searchpair_cmn(argvars, &match_pos) > 0) {
lnum = match_pos.lnum;
col = match_pos.col;
}
tv_list_append_number(rettv->vval.v_list, (varnumber_T)lnum);
tv_list_append_number(rettv->vval.v_list, (varnumber_T)col);
}
long
do_searchpair(
char_u *spat, 
char_u *mpat, 
char_u *epat, 
int dir, 
const typval_T *skip, 
int flags, 
pos_T *match_pos,
linenr_T lnum_stop, 
long time_limit 
)
{
char_u *save_cpo;
char_u *pat, *pat2 = NULL, *pat3 = NULL;
long retval = 0;
pos_T pos;
pos_T firstpos;
pos_T foundpos;
pos_T save_cursor;
pos_T save_pos;
int n;
int nest = 1;
bool use_skip = false;
int options = SEARCH_KEEP;
proftime_T tm;
size_t pat2_len;
size_t pat3_len;
save_cpo = p_cpo;
p_cpo = empty_option;
tm = profile_setlimit(time_limit);
pat2_len = STRLEN(spat) + STRLEN(epat) + 17;
pat2 = xmalloc(pat2_len);
pat3_len = STRLEN(spat) + STRLEN(mpat) + STRLEN(epat) + 25;
pat3 = xmalloc(pat3_len);
snprintf((char *)pat2, pat2_len, "\\m\\(%s\\m\\)\\|\\(%s\\m\\)", spat, epat);
if (*mpat == NUL) {
STRCPY(pat3, pat2);
} else {
snprintf((char *)pat3, pat3_len,
"\\m\\(%s\\m\\)\\|\\(%s\\m\\)\\|\\(%s\\m\\)", spat, epat, mpat);
}
if (flags & SP_START) {
options |= SEARCH_START;
}
if (skip != NULL) {
if (skip->v_type == VAR_STRING || skip->v_type == VAR_FUNC) {
use_skip = skip->vval.v_string != NULL && *skip->vval.v_string != NUL;
}
}
save_cursor = curwin->w_cursor;
pos = curwin->w_cursor;
clearpos(&firstpos);
clearpos(&foundpos);
pat = pat3;
for (;; ) {
searchit_arg_T sia;
memset(&sia, 0, sizeof(sia));
sia.sa_stop_lnum = lnum_stop;
sia.sa_tm = &tm;
n = searchit(curwin, curbuf, &pos, NULL, dir, pat, 1L,
options, RE_SEARCH, &sia);
if (n == FAIL || (firstpos.lnum != 0 && equalpos(pos, firstpos))) {
break;
}
if (firstpos.lnum == 0)
firstpos = pos;
if (equalpos(pos, foundpos)) {
if (dir == BACKWARD)
decl(&pos);
else
incl(&pos);
}
foundpos = pos;
options &= ~SEARCH_START;
if (use_skip) {
save_pos = curwin->w_cursor;
curwin->w_cursor = pos;
bool err = false;
const bool r = eval_expr_to_bool(skip, &err);
curwin->w_cursor = save_pos;
if (err) {
curwin->w_cursor = save_cursor;
retval = -1;
break;
}
if (r)
continue;
}
if ((dir == BACKWARD && n == 3) || (dir == FORWARD && n == 2)) {
++nest;
pat = pat2; 
} else {
if (--nest == 1)
pat = pat3; 
}
if (nest == 0) {
if (flags & SP_RETCOUNT)
++retval;
else
retval = pos.lnum;
if (flags & SP_SETPCMARK)
setpcmark();
curwin->w_cursor = pos;
if (!(flags & SP_REPEAT))
break;
nest = 1; 
}
}
if (match_pos != NULL) {
match_pos->lnum = curwin->w_cursor.lnum;
match_pos->col = curwin->w_cursor.col + 1;
}
if ((flags & SP_NOMOVE) || retval == 0)
curwin->w_cursor = save_cursor;
xfree(pat2);
xfree(pat3);
if (p_cpo == empty_option)
p_cpo = save_cpo;
else
free_string_option(save_cpo);
return retval;
}
static void f_searchpos(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
pos_T match_pos;
int flags = 0;
const int n = search_cmn(argvars, &match_pos, &flags);
tv_list_alloc_ret(rettv, 2 + (!!(flags & SP_SUBPAT)));
const int lnum = (n > 0 ? match_pos.lnum : 0);
const int col = (n > 0 ? match_pos.col : 0);
tv_list_append_number(rettv->vval.v_list, (varnumber_T)lnum);
tv_list_append_number(rettv->vval.v_list, (varnumber_T)col);
if (flags & SP_SUBPAT) {
tv_list_append_number(rettv->vval.v_list, (varnumber_T)n);
}
}
static void f_serverlist(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
size_t n;
char **addrs = server_address_list(&n);
list_T *const l = tv_list_alloc_ret(rettv, n);
for (size_t i = 0; i < n; i++) {
tv_list_append_allocated_string(l, addrs[i]);
}
xfree(addrs);
}
static void f_serverstart(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL; 
if (check_restricted() || check_secure()) {
return;
}
char *address;
if (argvars[0].v_type != VAR_UNKNOWN) {
if (argvars[0].v_type != VAR_STRING) {
EMSG(_(e_invarg));
return;
} else {
address = xstrdup(tv_get_string(argvars));
}
} else {
address = server_address_new();
}
int result = server_start(address);
xfree(address);
if (result != 0) {
EMSG2("Failed to start server: %s",
result > 0 ? "Unknown system error" : uv_strerror(result));
return;
}
size_t n;
char **addrs = server_address_list(&n);
rettv->vval.v_string = (char_u *)addrs[n - 1];
n--;
for (size_t i = 0; i < n; i++) {
xfree(addrs[i]);
}
xfree(addrs);
}
static void f_serverstop(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
if (check_restricted() || check_secure()) {
return;
}
if (argvars[0].v_type != VAR_STRING) {
EMSG(_(e_invarg));
return;
}
rettv->v_type = VAR_NUMBER;
rettv->vval.v_number = 0;
if (argvars[0].vval.v_string) {
bool rv = server_stop((char *)argvars[0].vval.v_string);
rettv->vval.v_number = (rv ? 1 : 0);
}
}
static void f_setbufline(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
linenr_T lnum;
buf_T *buf;
buf = tv_get_buf(&argvars[0], false);
if (buf == NULL) {
rettv->vval.v_number = 1; 
} else {
lnum = tv_get_lnum_buf(&argvars[1], buf);
set_buffer_lines(buf, lnum, false, &argvars[2], rettv);
}
}
static void f_setbufvar(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
if (check_secure()
|| !tv_check_str_or_nr(&argvars[0])) {
return;
}
const char *varname = tv_get_string_chk(&argvars[1]);
buf_T *const buf = tv_get_buf(&argvars[0], false);
typval_T *varp = &argvars[2];
if (buf != NULL && varname != NULL) {
if (*varname == '&') {
long numval;
bool error = false;
aco_save_T aco;
aucmd_prepbuf(&aco, buf);
varname++;
numval = tv_get_number_chk(varp, &error);
char nbuf[NUMBUFLEN];
const char *const strval = tv_get_string_buf_chk(varp, nbuf);
if (!error && strval != NULL) {
set_option_value(varname, numval, strval, OPT_LOCAL);
}
aucmd_restbuf(&aco);
} else {
buf_T *save_curbuf = curbuf;
const size_t varname_len = STRLEN(varname);
char *const bufvarname = xmalloc(varname_len + 3);
curbuf = buf;
memcpy(bufvarname, "b:", 2);
memcpy(bufvarname + 2, varname, varname_len + 1);
set_var(bufvarname, varname_len + 2, varp, true);
xfree(bufvarname);
curbuf = save_curbuf;
}
}
}
static void f_setcharsearch(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
dict_T *d;
dictitem_T *di;
if (argvars[0].v_type != VAR_DICT) {
EMSG(_(e_dictreq));
return;
}
if ((d = argvars[0].vval.v_dict) != NULL) {
char_u *const csearch = (char_u *)tv_dict_get_string(d, "char", false);
if (csearch != NULL) {
if (enc_utf8) {
int pcc[MAX_MCO];
int c = utfc_ptr2char(csearch, pcc);
set_last_csearch(c, csearch, utfc_ptr2len(csearch));
}
else
set_last_csearch(PTR2CHAR(csearch),
csearch, utfc_ptr2len(csearch));
}
di = tv_dict_find(d, S_LEN("forward"));
if (di != NULL) {
set_csearch_direction(tv_get_number(&di->di_tv) ? FORWARD : BACKWARD);
}
di = tv_dict_find(d, S_LEN("until"));
if (di != NULL) {
set_csearch_until(!!tv_get_number(&di->di_tv));
}
}
}
static void f_setcmdpos(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const int pos = (int)tv_get_number(&argvars[0]) - 1;
if (pos >= 0) {
rettv->vval.v_number = set_cmdline_pos(pos);
}
}
static void f_setenv(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
char namebuf[NUMBUFLEN];
char valbuf[NUMBUFLEN];
const char *name = tv_get_string_buf(&argvars[0], namebuf);
if (argvars[1].v_type == VAR_SPECIAL
&& argvars[1].vval.v_special == kSpecialVarNull) {
os_unsetenv(name);
} else {
os_setenv(name, tv_get_string_buf(&argvars[1], valbuf), 1);
}
}
static void f_setfperm(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = 0;
const char *const fname = tv_get_string_chk(&argvars[0]);
if (fname == NULL) {
return;
}
char modebuf[NUMBUFLEN];
const char *const mode_str = tv_get_string_buf_chk(&argvars[1], modebuf);
if (mode_str == NULL) {
return;
}
if (strlen(mode_str) != 9) {
EMSG2(_(e_invarg2), mode_str);
return;
}
int mask = 1;
int mode = 0;
for (int i = 8; i >= 0; i--) {
if (mode_str[i] != '-') {
mode |= mask;
}
mask = mask << 1;
}
rettv->vval.v_number = os_setperm(fname, mode) == OK;
}
static void f_setline(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
linenr_T lnum = tv_get_lnum(&argvars[0]);
set_buffer_lines(curbuf, lnum, false, &argvars[1], rettv);
}
static void set_qf_ll_list(win_T *wp, typval_T *args, typval_T *rettv)
FUNC_ATTR_NONNULL_ARG(2, 3)
{
static char *e_invact = N_("E927: Invalid action: '%s'");
const char *title = NULL;
int action = ' ';
static int recursive = 0;
rettv->vval.v_number = -1;
dict_T *d = NULL;
typval_T *list_arg = &args[0];
if (list_arg->v_type != VAR_LIST) {
EMSG(_(e_listreq));
return;
} else if (recursive != 0) {
EMSG(_(e_au_recursive));
return;
}
typval_T *action_arg = &args[1];
if (action_arg->v_type == VAR_UNKNOWN) {
goto skip_args;
} else if (action_arg->v_type != VAR_STRING) {
EMSG(_(e_stringreq));
return;
}
const char *const act = tv_get_string_chk(action_arg);
if ((*act == 'a' || *act == 'r' || *act == ' ' || *act == 'f')
&& act[1] == NUL) {
action = *act;
} else {
EMSG2(_(e_invact), act);
return;
}
typval_T *title_arg = &args[2];
if (title_arg->v_type == VAR_UNKNOWN) {
goto skip_args;
} else if (title_arg->v_type == VAR_STRING) {
title = tv_get_string_chk(title_arg);
if (!title) {
return;
}
} else if (title_arg->v_type == VAR_DICT) {
d = title_arg->vval.v_dict;
} else {
EMSG(_(e_dictreq));
return;
}
skip_args:
if (!title) {
title = (wp ? ":setloclist()" : ":setqflist()");
}
recursive++;
list_T *const l = list_arg->vval.v_list;
if (set_errorlist(wp, l, action, (char_u *)title, d) == OK) {
rettv->vval.v_number = 0;
}
recursive--;
}
static void f_setloclist(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
win_T *win;
rettv->vval.v_number = -1;
win = find_win_by_nr_or_id(&argvars[0]);
if (win != NULL) {
set_qf_ll_list(win, &argvars[1], rettv);
}
}
static void f_setmatches(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
dict_T *d;
list_T *s = NULL;
rettv->vval.v_number = -1;
if (argvars[0].v_type != VAR_LIST) {
EMSG(_(e_listreq));
return;
}
list_T *const l = argvars[0].vval.v_list;
int li_idx = 0;
TV_LIST_ITER_CONST(l, li, {
if (TV_LIST_ITEM_TV(li)->v_type != VAR_DICT
|| (d = TV_LIST_ITEM_TV(li)->vval.v_dict) == NULL) {
emsgf(_("E474: List item %d is either not a dictionary "
"or an empty one"), li_idx);
return;
}
if (!(tv_dict_find(d, S_LEN("group")) != NULL
&& (tv_dict_find(d, S_LEN("pattern")) != NULL
|| tv_dict_find(d, S_LEN("pos1")) != NULL)
&& tv_dict_find(d, S_LEN("priority")) != NULL
&& tv_dict_find(d, S_LEN("id")) != NULL)) {
emsgf(_("E474: List item %d is missing one of the required keys"),
li_idx);
return;
}
li_idx++;
});
clear_matches(curwin);
bool match_add_failed = false;
TV_LIST_ITER_CONST(l, li, {
int i = 0;
d = TV_LIST_ITEM_TV(li)->vval.v_dict;
dictitem_T *const di = tv_dict_find(d, S_LEN("pattern"));
if (di == NULL) {
if (s == NULL) {
s = tv_list_alloc(9);
}
for (i = 1; i < 9; i++) {
char buf[30]; 
snprintf(buf, sizeof(buf), "pos%d", i);
dictitem_T *const pos_di = tv_dict_find(d, buf, -1);
if (pos_di != NULL) {
if (pos_di->di_tv.v_type != VAR_LIST) {
return;
}
tv_list_append_tv(s, &pos_di->di_tv);
tv_list_ref(s);
} else {
break;
}
}
}
char group_buf[NUMBUFLEN];
const char *const group = tv_dict_get_string_buf(d, "group", group_buf);
const int priority = (int)tv_dict_get_number(d, "priority");
const int id = (int)tv_dict_get_number(d, "id");
dictitem_T *const conceal_di = tv_dict_find(d, S_LEN("conceal"));
const char *const conceal = (conceal_di != NULL
? tv_get_string(&conceal_di->di_tv)
: NULL);
if (i == 0) {
if (match_add(curwin, group,
tv_dict_get_string(d, "pattern", false),
priority, id, NULL, conceal) != id) {
match_add_failed = true;
}
} else {
if (match_add(curwin, group, NULL, priority, id, s, conceal) != id) {
match_add_failed = true;
}
tv_list_unref(s);
s = NULL;
}
});
if (!match_add_failed) {
rettv->vval.v_number = 0;
}
}
static void f_setpos(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
pos_T pos;
int fnum;
colnr_T curswant = -1;
rettv->vval.v_number = -1;
const char *const name = tv_get_string_chk(argvars);
if (name != NULL) {
if (list2fpos(&argvars[1], &pos, &fnum, &curswant) == OK) {
if (pos.col != MAXCOL && --pos.col < 0) {
pos.col = 0;
}
if (name[0] == '.' && name[1] == NUL) {
curwin->w_cursor = pos;
if (curswant >= 0) {
curwin->w_curswant = curswant - 1;
curwin->w_set_curswant = false;
}
check_cursor();
rettv->vval.v_number = 0;
} else if (name[0] == '\'' && name[1] != NUL && name[2] == NUL) {
if (setmark_pos((uint8_t)name[1], &pos, fnum) == OK) {
rettv->vval.v_number = 0;
}
} else {
EMSG(_(e_invarg));
}
}
}
}
static void f_setqflist(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
set_qf_ll_list(NULL, argvars, rettv);
}
static void f_setreg(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
int regname;
bool append = false;
MotionType yank_type;
long block_len;
block_len = -1;
yank_type = kMTUnknown;
rettv->vval.v_number = 1; 
const char *const strregname = tv_get_string_chk(argvars);
if (strregname == NULL) {
return; 
}
regname = (uint8_t)(*strregname);
if (regname == 0 || regname == '@') {
regname = '"';
}
bool set_unnamed = false;
if (argvars[2].v_type != VAR_UNKNOWN) {
const char *stropt = tv_get_string_chk(&argvars[2]);
if (stropt == NULL) {
return; 
}
for (; *stropt != NUL; stropt++) {
switch (*stropt) {
case 'a': case 'A': { 
append = true;
break;
}
case 'v': case 'c': { 
yank_type = kMTCharWise;
break;
}
case 'V': case 'l': { 
yank_type = kMTLineWise;
break;
}
case 'b': case Ctrl_V: { 
yank_type = kMTBlockWise;
if (ascii_isdigit(stropt[1])) {
stropt++;
block_len = getdigits_long((char_u **)&stropt, true, 0) - 1;
stropt--;
}
break;
}
case 'u': case '"': { 
set_unnamed = true;
break;
}
}
}
}
if (argvars[1].v_type == VAR_LIST) {
list_T *ll = argvars[1].vval.v_list;
const int len = tv_list_len(ll);
char **lstval = xmalloc(sizeof(char *) * ((len + 1) * 2));
const char **curval = (const char **)lstval;
char **allocval = lstval + len + 2;
char **curallocval = allocval;
TV_LIST_ITER_CONST(ll, li, {
char buf[NUMBUFLEN];
*curval = tv_get_string_buf_chk(TV_LIST_ITEM_TV(li), buf);
if (*curval == NULL) {
goto free_lstval;
}
if (*curval == buf) {
*curallocval = xstrdup(*curval);
*curval = *curallocval;
curallocval++;
}
curval++;
});
*curval++ = NULL;
write_reg_contents_lst(regname, (char_u **)lstval, append, yank_type,
block_len);
free_lstval:
while (curallocval > allocval) {
xfree(*--curallocval);
}
xfree(lstval);
} else {
const char *strval = tv_get_string_chk(&argvars[1]);
if (strval == NULL) {
return;
}
write_reg_contents_ex(regname, (const char_u *)strval, STRLEN(strval),
append, yank_type, block_len);
}
rettv->vval.v_number = 0;
if (set_unnamed) {
if (op_reg_set_previous(regname)) { }
}
}
static void f_settabvar(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = 0;
if (check_secure()) {
return;
}
tabpage_T *const tp = find_tabpage((int)tv_get_number_chk(&argvars[0], NULL));
const char *const varname = tv_get_string_chk(&argvars[1]);
typval_T *const varp = &argvars[2];
if (varname != NULL && tp != NULL) {
tabpage_T *const save_curtab = curtab;
goto_tabpage_tp(tp, false, false);
const size_t varname_len = strlen(varname);
char *const tabvarname = xmalloc(varname_len + 3);
memcpy(tabvarname, "t:", 2);
memcpy(tabvarname + 2, varname, varname_len + 1);
set_var(tabvarname, varname_len + 2, varp, true);
xfree(tabvarname);
if (valid_tabpage(save_curtab)) {
goto_tabpage_tp(save_curtab, false, false);
}
}
}
static void f_settabwinvar(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
setwinvar(argvars, rettv, 1);
}
static void f_settagstack(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
static char *e_invact2 = N_("E962: Invalid action: '%s'");
win_T *wp;
dict_T *d;
int action = 'r';
rettv->vval.v_number = -1;
wp = find_win_by_nr_or_id(&argvars[0]);
if (wp == NULL) {
return;
}
if (argvars[1].v_type != VAR_DICT) {
EMSG(_(e_dictreq));
return;
}
d = argvars[1].vval.v_dict;
if (d == NULL) {
return;
}
if (argvars[2].v_type == VAR_UNKNOWN) {
action = 'r';
} else if (argvars[2].v_type == VAR_STRING) {
const char *actstr;
actstr = tv_get_string_chk(&argvars[2]);
if (actstr == NULL) {
return;
}
if ((*actstr == 'r' || *actstr == 'a' || *actstr == 't')
&& actstr[1] == NUL) {
action = *actstr;
} else {
EMSG2(_(e_invact2), actstr);
return;
}
} else {
EMSG(_(e_stringreq));
return;
}
if (set_tagstack(wp, d, action) == OK) {
rettv->vval.v_number = 0;
} else {
EMSG(_(e_listreq));
}
}
static void f_setwinvar(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
setwinvar(argvars, rettv, 0);
}
static void f_sha256(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const char *p = tv_get_string(&argvars[0]);
const char *hash = sha256_bytes((const uint8_t *)p, strlen(p) , NULL, 0);
rettv->vval.v_string = (char_u *)xstrdup(hash);
rettv->v_type = VAR_STRING;
}
static void f_shellescape(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const bool do_special = non_zero_arg(&argvars[1]);
rettv->vval.v_string = vim_strsave_shellescape(
(const char_u *)tv_get_string(&argvars[0]), do_special, do_special);
rettv->v_type = VAR_STRING;
}
static void f_shiftwidth(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = get_sw_value(curbuf);
}
static void f_sign_define(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const char *name;
dict_T *dict;
char *icon = NULL;
char *linehl = NULL;
char *text = NULL;
char *texthl = NULL;
char *numhl = NULL;
rettv->vval.v_number = -1;
name = tv_get_string_chk(&argvars[0]);
if (name == NULL) {
return;
}
if (argvars[1].v_type != VAR_UNKNOWN) {
if (argvars[1].v_type != VAR_DICT) {
EMSG(_(e_dictreq));
return;
}
dict = argvars[1].vval.v_dict;
if (tv_dict_find(dict, "icon", -1) != NULL) {
icon = tv_dict_get_string(dict, "icon", true);
}
if (tv_dict_find(dict, "linehl", -1) != NULL) {
linehl = tv_dict_get_string(dict, "linehl", true);
}
if (tv_dict_find(dict, "text", -1) != NULL) {
text = tv_dict_get_string(dict, "text", true);
}
if (tv_dict_find(dict, "texthl", -1) != NULL) {
texthl = tv_dict_get_string(dict, "texthl", true);
}
if (tv_dict_find(dict, "numhl", -1) != NULL) {
numhl = tv_dict_get_string(dict, "numhl", true);
}
}
if (sign_define_by_name((char_u *)name, (char_u *)icon, (char_u *)linehl,
(char_u *)text, (char_u *)texthl, (char_u *)numhl)
== OK) {
rettv->vval.v_number = 0;
}
xfree(icon);
xfree(linehl);
xfree(text);
xfree(texthl);
}
static void f_sign_getdefined(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const char *name = NULL;
tv_list_alloc_ret(rettv, 0);
if (argvars[0].v_type != VAR_UNKNOWN) {
name = tv_get_string(&argvars[0]);
}
sign_getlist((const char_u *)name, rettv->vval.v_list);
}
static void f_sign_getplaced(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
buf_T *buf = NULL;
dict_T *dict;
dictitem_T *di;
linenr_T lnum = 0;
int sign_id = 0;
const char *group = NULL;
bool notanum = false;
tv_list_alloc_ret(rettv, 0);
if (argvars[0].v_type != VAR_UNKNOWN) {
buf = get_buf_arg(&argvars[0]);
if (buf == NULL) {
return;
}
if (argvars[1].v_type != VAR_UNKNOWN) {
if (argvars[1].v_type != VAR_DICT
|| ((dict = argvars[1].vval.v_dict) == NULL)) {
EMSG(_(e_dictreq));
return;
}
if ((di = tv_dict_find(dict, "lnum", -1)) != NULL) {
lnum = (linenr_T)tv_get_number_chk(&di->di_tv, &notanum);
if (notanum) {
return;
}
(void)lnum;
lnum = tv_get_lnum(&di->di_tv);
}
if ((di = tv_dict_find(dict, "id", -1)) != NULL) {
sign_id = (int)tv_get_number_chk(&di->di_tv, &notanum);
if (notanum) {
return;
}
}
if ((di = tv_dict_find(dict, "group", -1)) != NULL) {
group = tv_get_string_chk(&di->di_tv);
if (group == NULL) {
return;
}
if (*group == '\0') { 
group = NULL;
}
}
}
}
sign_get_placed(buf, lnum, sign_id, (const char_u *)group,
rettv->vval.v_list);
}
static void f_sign_jump(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
int sign_id;
char *sign_group = NULL;
buf_T *buf;
bool notanum = false;
rettv->vval.v_number = -1;
sign_id = (int)tv_get_number_chk(&argvars[0], &notanum);
if (notanum) {
return;
}
if (sign_id <= 0) {
EMSG(_(e_invarg));
return;
}
const char * sign_group_chk = tv_get_string_chk(&argvars[1]);
if (sign_group_chk == NULL) {
return;
}
if (sign_group_chk[0] == '\0') {
sign_group = NULL; 
} else {
sign_group = xstrdup(sign_group_chk);
}
buf = get_buf_arg(&argvars[2]);
if (buf == NULL) {
goto cleanup;
}
rettv->vval.v_number = sign_jump(sign_id, (char_u *)sign_group, buf);
cleanup:
xfree(sign_group);
}
static void f_sign_place(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
int sign_id;
char_u *group = NULL;
const char *sign_name;
buf_T *buf;
dict_T *dict;
dictitem_T *di;
linenr_T lnum = 0;
int prio = SIGN_DEF_PRIO;
bool notanum = false;
rettv->vval.v_number = -1;
sign_id = (int)tv_get_number_chk(&argvars[0], &notanum);
if (notanum) {
return;
}
if (sign_id < 0) {
EMSG(_(e_invarg));
return;
}
const char *group_chk = tv_get_string_chk(&argvars[1]);
if (group_chk == NULL) {
return;
}
if (group_chk[0] == '\0') {
group = NULL; 
} else {
group = vim_strsave((const char_u *)group_chk);
}
sign_name = tv_get_string_chk(&argvars[2]);
if (sign_name == NULL) {
goto cleanup;
}
buf = get_buf_arg(&argvars[3]);
if (buf == NULL) {
goto cleanup;
}
if (argvars[4].v_type != VAR_UNKNOWN) {
if (argvars[4].v_type != VAR_DICT
|| ((dict = argvars[4].vval.v_dict) == NULL)) {
EMSG(_(e_dictreq));
goto cleanup;
}
if ((di = tv_dict_find(dict, "lnum", -1)) != NULL) {
lnum = (linenr_T)tv_get_number_chk(&di->di_tv, &notanum);
if (notanum) {
goto cleanup;
}
(void)lnum;
lnum = tv_get_lnum(&di->di_tv);
}
if ((di = tv_dict_find(dict, "priority", -1)) != NULL) {
prio = (int)tv_get_number_chk(&di->di_tv, &notanum);
if (notanum) {
goto cleanup;
}
}
}
if (sign_place(&sign_id, group, (const char_u *)sign_name, buf, lnum, prio)
== OK) {
rettv->vval.v_number = sign_id;
}
cleanup:
xfree(group);
}
static void f_sign_undefine(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const char *name;
rettv->vval.v_number = -1;
if (argvars[0].v_type == VAR_UNKNOWN) {
free_signs();
rettv->vval.v_number = 0;
} else {
name = tv_get_string_chk(&argvars[0]);
if (name == NULL) {
return;
}
if (sign_undefine_by_name((const char_u *)name) == OK) {
rettv->vval.v_number = 0;
}
}
}
static void f_sign_unplace(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
dict_T *dict;
dictitem_T *di;
int sign_id = 0;
buf_T *buf = NULL;
char_u *group = NULL;
rettv->vval.v_number = -1;
if (argvars[0].v_type != VAR_STRING) {
EMSG(_(e_invarg));
return;
}
const char *group_chk = tv_get_string(&argvars[0]);
if (group_chk[0] == '\0') {
group = NULL; 
} else {
group = vim_strsave((const char_u *)group_chk);
}
if (argvars[1].v_type != VAR_UNKNOWN) {
if (argvars[1].v_type != VAR_DICT) {
EMSG(_(e_dictreq));
goto cleanup;
}
dict = argvars[1].vval.v_dict;
if ((di = tv_dict_find(dict, "buffer", -1)) != NULL) {
buf = get_buf_arg(&di->di_tv);
if (buf == NULL) {
goto cleanup;
}
}
if (tv_dict_find(dict, "id", -1) != NULL) {
sign_id = tv_dict_get_number(dict, "id");
}
}
if (buf == NULL) {
FOR_ALL_BUFFERS(cbuf) {
if (sign_unplace(sign_id, group, cbuf, 0) == OK) {
rettv->vval.v_number = 0;
}
}
} else {
if (sign_unplace(sign_id, group, buf, 0) == OK) {
rettv->vval.v_number = 0;
}
}
cleanup:
xfree(group);
}
static void f_simplify(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const char *const p = tv_get_string(&argvars[0]);
rettv->vval.v_string = (char_u *)xstrdup(p);
simplify_filename(rettv->vval.v_string); 
rettv->v_type = VAR_STRING;
}
static void f_sockconnect(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
if (argvars[0].v_type != VAR_STRING || argvars[1].v_type != VAR_STRING) {
EMSG(_(e_invarg));
return;
}
if (argvars[2].v_type != VAR_DICT && argvars[2].v_type != VAR_UNKNOWN) {
EMSG2(_(e_invarg2), "expected dictionary");
return;
}
const char *mode = tv_get_string(&argvars[0]);
const char *address = tv_get_string(&argvars[1]);
bool tcp;
if (strcmp(mode, "tcp") == 0) {
tcp = true;
} else if (strcmp(mode, "pipe") == 0) {
tcp = false;
} else {
EMSG2(_(e_invarg2), "invalid mode");
return;
}
bool rpc = false;
CallbackReader on_data = CALLBACK_READER_INIT;
if (argvars[2].v_type == VAR_DICT) {
dict_T *opts = argvars[2].vval.v_dict;
rpc = tv_dict_get_number(opts, "rpc") != 0;
if (!tv_dict_get_callback(opts, S_LEN("on_data"), &on_data.cb)) {
return;
}
on_data.buffered = tv_dict_get_number(opts, "data_buffered");
if (on_data.buffered && on_data.cb.type == kCallbackNone) {
on_data.self = opts;
}
}
const char *error = NULL;
uint64_t id = channel_connect(tcp, address, rpc, on_data, 50, &error);
if (error) {
EMSG2(_("connection failed: %s"), error);
}
rettv->vval.v_number = (varnumber_T)id;
rettv->v_type = VAR_NUMBER;
}
typedef struct {
int item_compare_ic;
bool item_compare_numeric;
bool item_compare_numbers;
bool item_compare_float;
const char *item_compare_func;
partial_T *item_compare_partial;
dict_T *item_compare_selfdict;
bool item_compare_func_err;
} sortinfo_T;
static sortinfo_T *sortinfo = NULL;
#define ITEM_COMPARE_FAIL 999
static int item_compare(const void *s1, const void *s2, bool keep_zero)
{
ListSortItem *const si1 = (ListSortItem *)s1;
ListSortItem *const si2 = (ListSortItem *)s2;
typval_T *const tv1 = TV_LIST_ITEM_TV(si1->item);
typval_T *const tv2 = TV_LIST_ITEM_TV(si2->item);
int res;
if (sortinfo->item_compare_numbers) {
const varnumber_T v1 = tv_get_number(tv1);
const varnumber_T v2 = tv_get_number(tv2);
res = v1 == v2 ? 0 : v1 > v2 ? 1 : -1;
goto item_compare_end;
}
if (sortinfo->item_compare_float) {
const float_T v1 = tv_get_float(tv1);
const float_T v2 = tv_get_float(tv2);
res = v1 == v2 ? 0 : v1 > v2 ? 1 : -1;
goto item_compare_end;
}
char *tofree1 = NULL;
char *tofree2 = NULL;
char *p1;
char *p2;
if (tv1->v_type == VAR_STRING) {
if (tv2->v_type != VAR_STRING || sortinfo->item_compare_numeric) {
p1 = "'";
} else {
p1 = (char *)tv1->vval.v_string;
}
} else {
tofree1 = p1 = encode_tv2string(tv1, NULL);
}
if (tv2->v_type == VAR_STRING) {
if (tv1->v_type != VAR_STRING || sortinfo->item_compare_numeric) {
p2 = "'";
} else {
p2 = (char *)tv2->vval.v_string;
}
} else {
tofree2 = p2 = encode_tv2string(tv2, NULL);
}
if (p1 == NULL) {
p1 = "";
}
if (p2 == NULL) {
p2 = "";
}
if (!sortinfo->item_compare_numeric) {
if (sortinfo->item_compare_ic) {
res = STRICMP(p1, p2);
} else {
res = STRCMP(p1, p2);
}
} else {
double n1, n2;
n1 = strtod(p1, &p1);
n2 = strtod(p2, &p2);
res = n1 == n2 ? 0 : n1 > n2 ? 1 : -1;
}
xfree(tofree1);
xfree(tofree2);
item_compare_end:
if (res == 0 && !keep_zero) {
res = si1->idx > si2->idx ? 1 : -1;
}
return res;
}
static int item_compare_keeping_zero(const void *s1, const void *s2)
{
return item_compare(s1, s2, true);
}
static int item_compare_not_keeping_zero(const void *s1, const void *s2)
{
return item_compare(s1, s2, false);
}
static int item_compare2(const void *s1, const void *s2, bool keep_zero)
{
ListSortItem *si1, *si2;
int res;
typval_T rettv;
typval_T argv[3];
int dummy;
const char *func_name;
partial_T *partial = sortinfo->item_compare_partial;
if (sortinfo->item_compare_func_err) {
return 0;
}
si1 = (ListSortItem *)s1;
si2 = (ListSortItem *)s2;
if (partial == NULL) {
func_name = sortinfo->item_compare_func;
} else {
func_name = (const char *)partial_name(partial);
}
tv_copy(TV_LIST_ITEM_TV(si1->item), &argv[0]);
tv_copy(TV_LIST_ITEM_TV(si2->item), &argv[1]);
rettv.v_type = VAR_UNKNOWN; 
res = call_func((const char_u *)func_name,
(int)STRLEN(func_name),
&rettv, 2, argv, NULL, 0L, 0L, &dummy, true,
partial, sortinfo->item_compare_selfdict);
tv_clear(&argv[0]);
tv_clear(&argv[1]);
if (res == FAIL) {
res = ITEM_COMPARE_FAIL;
} else {
res = tv_get_number_chk(&rettv, &sortinfo->item_compare_func_err);
}
if (sortinfo->item_compare_func_err) {
res = ITEM_COMPARE_FAIL; 
}
tv_clear(&rettv);
if (res == 0 && !keep_zero) {
res = si1->idx > si2->idx ? 1 : -1;
}
return res;
}
static int item_compare2_keeping_zero(const void *s1, const void *s2)
{
return item_compare2(s1, s2, true);
}
static int item_compare2_not_keeping_zero(const void *s1, const void *s2)
{
return item_compare2(s1, s2, false);
}
static void do_sort_uniq(typval_T *argvars, typval_T *rettv, bool sort)
{
ListSortItem *ptrs;
long len;
long i;
sortinfo_T info;
sortinfo_T *old_sortinfo = sortinfo;
sortinfo = &info;
const char *const arg_errmsg = (sort
? N_("sort() argument")
: N_("uniq() argument"));
if (argvars[0].v_type != VAR_LIST) {
EMSG2(_(e_listarg), sort ? "sort()" : "uniq()");
} else {
list_T *const l = argvars[0].vval.v_list;
if (tv_check_lock(tv_list_locked(l), arg_errmsg, TV_TRANSLATE)) {
goto theend;
}
tv_list_set_ret(rettv, l);
len = tv_list_len(l);
if (len <= 1) {
goto theend; 
}
info.item_compare_ic = false;
info.item_compare_numeric = false;
info.item_compare_numbers = false;
info.item_compare_float = false;
info.item_compare_func = NULL;
info.item_compare_partial = NULL;
info.item_compare_selfdict = NULL;
if (argvars[1].v_type != VAR_UNKNOWN) {
if (argvars[1].v_type == VAR_FUNC) {
info.item_compare_func = (const char *)argvars[1].vval.v_string;
} else if (argvars[1].v_type == VAR_PARTIAL) {
info.item_compare_partial = argvars[1].vval.v_partial;
} else {
bool error = false;
i = tv_get_number_chk(&argvars[1], &error);
if (error) {
goto theend; 
}
if (i == 1) {
info.item_compare_ic = true;
} else if (argvars[1].v_type != VAR_NUMBER) {
info.item_compare_func = tv_get_string(&argvars[1]);
} else if (i != 0) {
EMSG(_(e_invarg));
goto theend;
}
if (info.item_compare_func != NULL) {
if (*info.item_compare_func == NUL) {
info.item_compare_func = NULL;
} else if (strcmp(info.item_compare_func, "n") == 0) {
info.item_compare_func = NULL;
info.item_compare_numeric = true;
} else if (strcmp(info.item_compare_func, "N") == 0) {
info.item_compare_func = NULL;
info.item_compare_numbers = true;
} else if (strcmp(info.item_compare_func, "f") == 0) {
info.item_compare_func = NULL;
info.item_compare_float = true;
} else if (strcmp(info.item_compare_func, "i") == 0) {
info.item_compare_func = NULL;
info.item_compare_ic = true;
}
}
}
if (argvars[2].v_type != VAR_UNKNOWN) {
if (argvars[2].v_type != VAR_DICT) {
EMSG(_(e_dictreq));
goto theend;
}
info.item_compare_selfdict = argvars[2].vval.v_dict;
}
}
ptrs = xmalloc((size_t)(len * sizeof(ListSortItem)));
if (sort) {
info.item_compare_func_err = false;
tv_list_item_sort(l, ptrs,
((info.item_compare_func == NULL
&& info.item_compare_partial == NULL)
? item_compare_not_keeping_zero
: item_compare2_not_keeping_zero),
&info.item_compare_func_err);
if (info.item_compare_func_err) {
EMSG(_("E702: Sort compare function failed"));
}
} else {
ListSorter item_compare_func_ptr;
info.item_compare_func_err = false;
if (info.item_compare_func != NULL
|| info.item_compare_partial != NULL) {
item_compare_func_ptr = item_compare2_keeping_zero;
} else {
item_compare_func_ptr = item_compare_keeping_zero;
}
int idx = 0;
for (listitem_T *li = TV_LIST_ITEM_NEXT(l, tv_list_first(l))
; li != NULL;) {
listitem_T *const prev_li = TV_LIST_ITEM_PREV(l, li);
if (item_compare_func_ptr(&prev_li, &li) == 0) {
if (info.item_compare_func_err) { 
EMSG(_("E882: Uniq compare function failed"));
break;
}
li = tv_list_item_remove(l, li);
} else {
idx++;
li = TV_LIST_ITEM_NEXT(l, li);
}
}
}
xfree(ptrs);
}
theend:
sortinfo = old_sortinfo;
}
static void f_sort(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
do_sort_uniq(argvars, rettv, true);
}
static void f_stdioopen(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
if (argvars[0].v_type != VAR_DICT) {
EMSG(_(e_invarg));
return;
}
bool rpc = false;
CallbackReader on_stdin = CALLBACK_READER_INIT;
dict_T *opts = argvars[0].vval.v_dict;
rpc = tv_dict_get_number(opts, "rpc") != 0;
if (!tv_dict_get_callback(opts, S_LEN("on_stdin"), &on_stdin.cb)) {
return;
}
on_stdin.buffered = tv_dict_get_number(opts, "stdin_buffered");
if (on_stdin.buffered && on_stdin.cb.type == kCallbackNone) {
on_stdin.self = opts;
}
const char *error;
uint64_t id = channel_from_stdio(rpc, on_stdin, &error);
if (!id) {
EMSG2(e_stdiochan2, error);
}
rettv->vval.v_number = (varnumber_T)id;
rettv->v_type = VAR_NUMBER;
}
static void f_uniq(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
do_sort_uniq(argvars, rettv, false);
}
static void f_reltimefloat(typval_T *argvars , typval_T *rettv, FunPtr fptr)
FUNC_ATTR_NONNULL_ALL
{
proftime_T tm;
rettv->v_type = VAR_FLOAT;
rettv->vval.v_float = 0;
if (list2proftime(&argvars[0], &tm) == OK) {
rettv->vval.v_float = (float_T)profile_signed(tm) / 1000000000.0;
}
}
static void f_soundfold(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_STRING;
const char *const s = tv_get_string(&argvars[0]);
rettv->vval.v_string = (char_u *)eval_soundfold(s);
}
static void f_spellbadword(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const char *word = "";
hlf_T attr = HLF_COUNT;
size_t len = 0;
if (argvars[0].v_type == VAR_UNKNOWN) {
len = spell_move_to(curwin, FORWARD, true, true, &attr);
if (len != 0) {
word = (char *)get_cursor_pos_ptr();
curwin->w_set_curswant = true;
}
} else if (curwin->w_p_spell && *curbuf->b_s.b_p_spl != NUL) {
const char *str = tv_get_string_chk(&argvars[0]);
int capcol = -1;
if (str != NULL) {
while (*str != NUL) {
len = spell_check(curwin, (char_u *)str, &attr, &capcol, false);
if (attr != HLF_COUNT) {
word = str;
break;
}
str += len;
capcol -= len;
len = 0;
}
}
}
assert(len <= INT_MAX);
tv_list_alloc_ret(rettv, 2);
tv_list_append_string(rettv->vval.v_list, word, len);
tv_list_append_string(rettv->vval.v_list,
(attr == HLF_SPB ? "bad"
: attr == HLF_SPR ? "rare"
: attr == HLF_SPL ? "local"
: attr == HLF_SPC ? "caps"
: NULL), -1);
}
static void f_spellsuggest(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
bool typeerr = false;
int maxcount;
garray_T ga = GA_EMPTY_INIT_VALUE;
bool need_capital = false;
if (curwin->w_p_spell && *curwin->w_s->b_p_spl != NUL) {
const char *const str = tv_get_string(&argvars[0]);
if (argvars[1].v_type != VAR_UNKNOWN) {
maxcount = tv_get_number_chk(&argvars[1], &typeerr);
if (maxcount <= 0) {
goto f_spellsuggest_return;
}
if (argvars[2].v_type != VAR_UNKNOWN) {
need_capital = tv_get_number_chk(&argvars[2], &typeerr);
if (typeerr) {
goto f_spellsuggest_return;
}
}
} else {
maxcount = 25;
}
spell_suggest_list(&ga, (char_u *)str, maxcount, need_capital, false);
}
f_spellsuggest_return:
tv_list_alloc_ret(rettv, (ptrdiff_t)ga.ga_len);
for (int i = 0; i < ga.ga_len; i++) {
char *const p = ((char **)ga.ga_data)[i];
tv_list_append_allocated_string(rettv->vval.v_list, p);
}
ga_clear(&ga);
}
static void f_split(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
char_u *save_cpo;
int match;
colnr_T col = 0;
bool keepempty = false;
bool typeerr = false;
save_cpo = p_cpo;
p_cpo = (char_u *)"";
const char *str = tv_get_string(&argvars[0]);
const char *pat = NULL;
char patbuf[NUMBUFLEN];
if (argvars[1].v_type != VAR_UNKNOWN) {
pat = tv_get_string_buf_chk(&argvars[1], patbuf);
if (pat == NULL) {
typeerr = true;
}
if (argvars[2].v_type != VAR_UNKNOWN) {
keepempty = (bool)tv_get_number_chk(&argvars[2], &typeerr);
}
}
if (pat == NULL || *pat == NUL) {
pat = "[\\x01- ]\\+";
}
tv_list_alloc_ret(rettv, kListLenMayKnow);
if (typeerr) {
return;
}
regmatch_T regmatch = {
.regprog = vim_regcomp((char_u *)pat, RE_MAGIC + RE_STRING),
.startp = { NULL },
.endp = { NULL },
.rm_ic = false,
};
if (regmatch.regprog != NULL) {
while (*str != NUL || keepempty) {
if (*str == NUL) {
match = false; 
} else {
match = vim_regexec_nl(&regmatch, (char_u *)str, col);
}
const char *end;
if (match) {
end = (const char *)regmatch.startp[0];
} else {
end = str + strlen(str);
}
if (keepempty || end > str || (tv_list_len(rettv->vval.v_list) > 0
&& *str != NUL
&& match
&& end < (const char *)regmatch.endp[0])) {
tv_list_append_string(rettv->vval.v_list, str, end - str);
}
if (!match) {
break;
}
if (regmatch.endp[0] > (char_u *)str) {
col = 0;
} else {
col = (*mb_ptr2len)(regmatch.endp[0]);
}
str = (const char *)regmatch.endp[0];
}
vim_regfree(regmatch.regprog);
}
p_cpo = save_cpo;
}
static void f_stdpath(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;
const char *const p = tv_get_string_chk(&argvars[0]);
if (p == NULL) {
return; 
}
if (strequal(p, "config")) {
rettv->vval.v_string = (char_u *)get_xdg_home(kXDGConfigHome);
} else if (strequal(p, "data")) {
rettv->vval.v_string = (char_u *)get_xdg_home(kXDGDataHome);
} else if (strequal(p, "cache")) {
rettv->vval.v_string = (char_u *)get_xdg_home(kXDGCacheHome);
} else if (strequal(p, "config_dirs")) {
get_xdg_var_list(kXDGConfigDirs, rettv);
} else if (strequal(p, "data_dirs")) {
get_xdg_var_list(kXDGDataDirs, rettv);
} else {
EMSG2(_("E6100: \"%s\" is not a valid stdpath"), p);
}
}
static void f_str2float(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
char_u *p = skipwhite((const char_u *)tv_get_string(&argvars[0]));
bool isneg = (*p == '-');
if (*p == '+' || *p == '-') {
p = skipwhite(p + 1);
}
(void)string2float((char *)p, &rettv->vval.v_float);
if (isneg) {
rettv->vval.v_float *= -1;
}
rettv->v_type = VAR_FLOAT;
}
static void f_str2list(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
tv_list_alloc_ret(rettv, kListLenUnknown);
const char_u *p = (const char_u *)tv_get_string(&argvars[0]);
for (; *p != NUL; p += utf_ptr2len(p)) {
tv_list_append_number(rettv->vval.v_list, utf_ptr2char(p));
}
}
static void f_str2nr(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
int base = 10;
varnumber_T n;
int what;
if (argvars[1].v_type != VAR_UNKNOWN) {
base = tv_get_number(&argvars[1]);
if (base != 2 && base != 8 && base != 10 && base != 16) {
EMSG(_(e_invarg));
return;
}
}
char_u *p = skipwhite((const char_u *)tv_get_string(&argvars[0]));
bool isneg = (*p == '-');
if (*p == '+' || *p == '-') {
p = skipwhite(p + 1);
}
switch (base) {
case 2: {
what = STR2NR_BIN | STR2NR_FORCE;
break;
}
case 8: {
what = STR2NR_OCT | STR2NR_FORCE;
break;
}
case 16: {
what = STR2NR_HEX | STR2NR_FORCE;
break;
}
default: {
what = 0;
}
}
vim_str2nr(p, NULL, NULL, what, &n, NULL, 0);
if (isneg) {
rettv->vval.v_number = -n;
} else {
rettv->vval.v_number = n;
}
}
static void f_strftime(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
time_t seconds;
rettv->v_type = VAR_STRING;
char *p = (char *)tv_get_string(&argvars[0]);
if (argvars[1].v_type == VAR_UNKNOWN) {
seconds = time(NULL);
} else {
seconds = (time_t)tv_get_number(&argvars[1]);
}
struct tm curtime;
struct tm *curtime_ptr = os_localtime_r(&seconds, &curtime);
if (curtime_ptr == NULL)
rettv->vval.v_string = vim_strsave((char_u *)_("(Invalid)"));
else {
vimconv_T conv;
char_u *enc;
conv.vc_type = CONV_NONE;
enc = enc_locale();
convert_setup(&conv, p_enc, enc);
if (conv.vc_type != CONV_NONE) {
p = (char *)string_convert(&conv, (char_u *)p, NULL);
}
char result_buf[256];
if (p != NULL) {
(void)strftime(result_buf, sizeof(result_buf), p, curtime_ptr);
} else {
result_buf[0] = NUL;
}
if (conv.vc_type != CONV_NONE) {
xfree(p);
}
convert_setup(&conv, enc, p_enc);
if (conv.vc_type != CONV_NONE) {
rettv->vval.v_string = string_convert(&conv, (char_u *)result_buf, NULL);
} else {
rettv->vval.v_string = (char_u *)xstrdup(result_buf);
}
convert_setup(&conv, NULL, NULL);
xfree(enc);
}
}
static void f_strgetchar(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = -1;
const char *const str = tv_get_string_chk(&argvars[0]);
if (str == NULL) {
return;
}
bool error = false;
varnumber_T charidx = tv_get_number_chk(&argvars[1], &error);
if (error) {
return;
}
const size_t len = STRLEN(str);
size_t byteidx = 0;
while (charidx >= 0 && byteidx < len) {
if (charidx == 0) {
rettv->vval.v_number = utf_ptr2char((const char_u *)str + byteidx);
break;
}
charidx--;
byteidx += MB_CPTR2LEN((const char_u *)str + byteidx);
}
}
static void f_stridx(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = -1;
char buf[NUMBUFLEN];
const char *const needle = tv_get_string_chk(&argvars[1]);
const char *haystack = tv_get_string_buf_chk(&argvars[0], buf);
const char *const haystack_start = haystack;
if (needle == NULL || haystack == NULL) {
return; 
}
if (argvars[2].v_type != VAR_UNKNOWN) {
bool error = false;
const ptrdiff_t start_idx = (ptrdiff_t)tv_get_number_chk(&argvars[2],
&error);
if (error || start_idx >= (ptrdiff_t)strlen(haystack)) {
return;
}
if (start_idx >= 0) {
haystack += start_idx;
}
}
const char *pos = strstr(haystack, needle);
if (pos != NULL) {
rettv->vval.v_number = (varnumber_T)(pos - haystack_start);
}
}
void f_string(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_STRING;
rettv->vval.v_string = (char_u *)encode_tv2string(&argvars[0], NULL);
}
static void f_strlen(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = (varnumber_T)strlen(tv_get_string(&argvars[0]));
}
static void f_strchars(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const char *s = tv_get_string(&argvars[0]);
int skipcc = 0;
varnumber_T len = 0;
int (*func_mb_ptr2char_adv)(const char_u **pp);
if (argvars[1].v_type != VAR_UNKNOWN) {
skipcc = tv_get_number_chk(&argvars[1], NULL);
}
if (skipcc < 0 || skipcc > 1) {
EMSG(_(e_invarg));
} else {
func_mb_ptr2char_adv = skipcc ? mb_ptr2char_adv : mb_cptr2char_adv;
while (*s != NUL) {
func_mb_ptr2char_adv((const char_u **)&s);
len++;
}
rettv->vval.v_number = len;
}
}
static void f_strdisplaywidth(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const char *const s = tv_get_string(&argvars[0]);
int col = 0;
if (argvars[1].v_type != VAR_UNKNOWN) {
col = tv_get_number(&argvars[1]);
}
rettv->vval.v_number = (varnumber_T)(linetabsize_col(col, (char_u *)s) - col);
}
static void f_strwidth(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const char *const s = tv_get_string(&argvars[0]);
rettv->vval.v_number = (varnumber_T)mb_string2cells((const char_u *)s);
}
static void f_strcharpart(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const char *const p = tv_get_string(&argvars[0]);
const size_t slen = STRLEN(p);
int nbyte = 0;
bool error = false;
varnumber_T nchar = tv_get_number_chk(&argvars[1], &error);
if (!error) {
if (nchar > 0) {
while (nchar > 0 && (size_t)nbyte < slen) {
nbyte += MB_CPTR2LEN((const char_u *)p + nbyte);
nchar--;
}
} else {
nbyte = nchar;
}
}
int len = 0;
if (argvars[2].v_type != VAR_UNKNOWN) {
int charlen = tv_get_number(&argvars[2]);
while (charlen > 0 && nbyte + len < (int)slen) {
int off = nbyte + len;
if (off < 0) {
len += 1;
} else {
len += (size_t)MB_CPTR2LEN((const char_u *)p + off);
}
charlen--;
}
} else {
len = slen - nbyte; 
}
if (nbyte < 0) {
len += nbyte;
nbyte = 0;
} else if ((size_t)nbyte > slen) {
nbyte = slen;
}
if (len < 0) {
len = 0;
} else if (nbyte + len > (int)slen) {
len = slen - nbyte;
}
rettv->v_type = VAR_STRING;
rettv->vval.v_string = (char_u *)xstrndup(p + nbyte, (size_t)len);
}
static void f_strpart(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
bool error = false;
const char *const p = tv_get_string(&argvars[0]);
const size_t slen = strlen(p);
varnumber_T n = tv_get_number_chk(&argvars[1], &error);
varnumber_T len;
if (error) {
len = 0;
} else if (argvars[2].v_type != VAR_UNKNOWN) {
len = tv_get_number(&argvars[2]);
} else {
len = slen - n; 
}
if (n < 0) {
len += n;
n = 0;
} else if (n > (varnumber_T)slen) {
n = slen;
}
if (len < 0) {
len = 0;
} else if (n + len > (varnumber_T)slen) {
len = slen - n;
}
rettv->v_type = VAR_STRING;
rettv->vval.v_string = (char_u *)xmemdupz(p + n, (size_t)len);
}
static void f_strridx(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
char buf[NUMBUFLEN];
const char *const needle = tv_get_string_chk(&argvars[1]);
const char *const haystack = tv_get_string_buf_chk(&argvars[0], buf);
rettv->vval.v_number = -1;
if (needle == NULL || haystack == NULL) {
return; 
}
const size_t haystack_len = STRLEN(haystack);
ptrdiff_t end_idx;
if (argvars[2].v_type != VAR_UNKNOWN) {
end_idx = (ptrdiff_t)tv_get_number_chk(&argvars[2], NULL);
if (end_idx < 0) {
return; 
}
} else {
end_idx = (ptrdiff_t)haystack_len;
}
const char *lastmatch = NULL;
if (*needle == NUL) {
lastmatch = haystack + end_idx;
} else {
for (const char *rest = haystack; *rest != NUL; rest++) {
rest = strstr(rest, needle);
if (rest == NULL || rest > haystack + end_idx) {
break;
}
lastmatch = rest;
}
}
if (lastmatch != NULL) {
rettv->vval.v_number = (varnumber_T)(lastmatch - haystack);
}
}
static void f_strtrans(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_STRING;
rettv->vval.v_string = (char_u *)transstr(tv_get_string(&argvars[0]));
}
static void f_submatch(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
bool error = false;
int no = (int)tv_get_number_chk(&argvars[0], &error);
if (error) {
return;
}
if (no < 0 || no >= NSUBEXP) {
emsgf(_("E935: invalid submatch number: %d"), no);
return;
}
int retList = 0;
if (argvars[1].v_type != VAR_UNKNOWN) {
retList = tv_get_number_chk(&argvars[1], &error);
if (error) {
return;
}
}
if (retList == 0) {
rettv->v_type = VAR_STRING;
rettv->vval.v_string = reg_submatch(no);
} else {
rettv->v_type = VAR_LIST;
rettv->vval.v_list = reg_submatch_list(no);
}
}
static void f_substitute(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
char patbuf[NUMBUFLEN];
char subbuf[NUMBUFLEN];
char flagsbuf[NUMBUFLEN];
const char *const str = tv_get_string_chk(&argvars[0]);
const char *const pat = tv_get_string_buf_chk(&argvars[1], patbuf);
const char *sub = NULL;
const char *const flg = tv_get_string_buf_chk(&argvars[3], flagsbuf);
typval_T *expr = NULL;
if (tv_is_func(argvars[2])) {
expr = &argvars[2];
} else {
sub = tv_get_string_buf_chk(&argvars[2], subbuf);
}
rettv->v_type = VAR_STRING;
if (str == NULL || pat == NULL || (sub == NULL && expr == NULL)
|| flg == NULL) {
rettv->vval.v_string = NULL;
} else {
rettv->vval.v_string = do_string_sub((char_u *)str, (char_u *)pat,
(char_u *)sub, expr, (char_u *)flg);
}
}
static void f_swapinfo(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
tv_dict_alloc_ret(rettv);
get_b0_dict(tv_get_string(argvars), rettv->vval.v_dict);
}
static void f_swapname(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_STRING;
buf_T *buf = tv_get_buf(&argvars[0], false);
if (buf == NULL
|| buf->b_ml.ml_mfp == NULL
|| buf->b_ml.ml_mfp->mf_fname == NULL) {
rettv->vval.v_string = NULL;
} else {
rettv->vval.v_string = vim_strsave(buf->b_ml.ml_mfp->mf_fname);
}
}
static void f_synID(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const linenr_T lnum = tv_get_lnum(argvars);
const colnr_T col = (colnr_T)tv_get_number(&argvars[1]) - 1;
bool transerr = false;
const int trans = tv_get_number_chk(&argvars[2], &transerr);
int id = 0;
if (!transerr && lnum >= 1 && lnum <= curbuf->b_ml.ml_line_count
&& col >= 0 && (size_t)col < STRLEN(ml_get(lnum))) {
id = syn_get_id(curwin, lnum, col, trans, NULL, false);
}
rettv->vval.v_number = id;
}
static void f_synIDattr(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const int id = (int)tv_get_number(&argvars[0]);
const char *const what = tv_get_string(&argvars[1]);
int modec;
if (argvars[2].v_type != VAR_UNKNOWN) {
char modebuf[NUMBUFLEN];
const char *const mode = tv_get_string_buf(&argvars[2], modebuf);
modec = TOLOWER_ASC(mode[0]);
if (modec != 'c' && modec != 'g') {
modec = 0; 
}
} else if (ui_rgb_attached()) {
modec = 'g';
} else {
modec = 'c';
}
const char *p = NULL;
switch (TOLOWER_ASC(what[0])) {
case 'b': {
if (TOLOWER_ASC(what[1]) == 'g') { 
p = highlight_color(id, what, modec);
} else { 
p = highlight_has_attr(id, HL_BOLD, modec);
}
break;
}
case 'f': { 
p = highlight_color(id, what, modec);
break;
}
case 'i': {
if (TOLOWER_ASC(what[1]) == 'n') { 
p = highlight_has_attr(id, HL_INVERSE, modec);
} else { 
p = highlight_has_attr(id, HL_ITALIC, modec);
}
break;
}
case 'n': { 
p = get_highlight_name_ext(NULL, id - 1, false);
break;
}
case 'r': { 
p = highlight_has_attr(id, HL_INVERSE, modec);
break;
}
case 's': {
if (TOLOWER_ASC(what[1]) == 'p') { 
p = highlight_color(id, what, modec);
} else if (TOLOWER_ASC(what[1]) == 't'
&& TOLOWER_ASC(what[2]) == 'r') { 
p = highlight_has_attr(id, HL_STRIKETHROUGH, modec);
} else { 
p = highlight_has_attr(id, HL_STANDOUT, modec);
}
break;
}
case 'u': {
if (STRLEN(what) <= 5 || TOLOWER_ASC(what[5]) != 'c') { 
p = highlight_has_attr(id, HL_UNDERLINE, modec);
} else { 
p = highlight_has_attr(id, HL_UNDERCURL, modec);
}
break;
}
}
rettv->v_type = VAR_STRING;
rettv->vval.v_string = (char_u *)(p == NULL ? p : xstrdup(p));
}
static void f_synIDtrans(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
int id = tv_get_number(&argvars[0]);
if (id > 0) {
id = syn_get_final_id(id);
} else {
id = 0;
}
rettv->vval.v_number = id;
}
static void f_synconcealed(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
int syntax_flags = 0;
int cchar;
int matchid = 0;
char_u str[NUMBUFLEN];
tv_list_set_ret(rettv, NULL);
const linenr_T lnum = tv_get_lnum(argvars);
const colnr_T col = (colnr_T)tv_get_number(&argvars[1]) - 1;
memset(str, NUL, sizeof(str));
if (lnum >= 1 && lnum <= curbuf->b_ml.ml_line_count && col >= 0
&& (size_t)col <= STRLEN(ml_get(lnum)) && curwin->w_p_cole > 0) {
(void)syn_get_id(curwin, lnum, col, false, NULL, false);
syntax_flags = get_syntax_info(&matchid);
if ((syntax_flags & HL_CONCEAL) && curwin->w_p_cole < 3) {
cchar = syn_get_sub_char();
if (cchar == NUL && curwin->w_p_cole == 1) {
cchar = (curwin->w_p_lcs_chars.conceal == NUL)
? ' '
: curwin->w_p_lcs_chars.conceal;
}
if (cchar != NUL) {
utf_char2bytes(cchar, str);
}
}
}
tv_list_alloc_ret(rettv, 3);
tv_list_append_number(rettv->vval.v_list, (syntax_flags & HL_CONCEAL) != 0);
tv_list_append_string(rettv->vval.v_list, (const char *)str, -1);
tv_list_append_number(rettv->vval.v_list, matchid);
}
static void f_synstack(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
tv_list_set_ret(rettv, NULL);
const linenr_T lnum = tv_get_lnum(argvars);
const colnr_T col = (colnr_T)tv_get_number(&argvars[1]) - 1;
if (lnum >= 1
&& lnum <= curbuf->b_ml.ml_line_count
&& col >= 0
&& (size_t)col <= STRLEN(ml_get(lnum))) {
tv_list_alloc_ret(rettv, kListLenMayKnow);
(void)syn_get_id(curwin, lnum, col, false, NULL, true);
int id;
int i = 0;
while ((id = syn_get_stack_item(i++)) >= 0) {
tv_list_append_number(rettv->vval.v_list, id);
}
}
}
static void f_system(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
get_system_output_as_rettv(argvars, rettv, false);
}
static void f_systemlist(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
get_system_output_as_rettv(argvars, rettv, true);
}
static void f_tabpagebuflist(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
win_T *wp = NULL;
if (argvars[0].v_type == VAR_UNKNOWN) {
wp = firstwin;
} else {
tabpage_T *const tp = find_tabpage((int)tv_get_number(&argvars[0]));
if (tp != NULL) {
wp = (tp == curtab) ? firstwin : tp->tp_firstwin;
}
}
if (wp != NULL) {
tv_list_alloc_ret(rettv, kListLenMayKnow);
while (wp != NULL) {
tv_list_append_number(rettv->vval.v_list, wp->w_buffer->b_fnum);
wp = wp->w_next;
}
}
}
static void f_tabpagenr(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
int nr = 1;
if (argvars[0].v_type != VAR_UNKNOWN) {
const char *const arg = tv_get_string_chk(&argvars[0]);
nr = 0;
if (arg != NULL) {
if (strcmp(arg, "$") == 0) {
nr = tabpage_index(NULL) - 1;
} else if (strcmp(arg, "#") == 0) {
nr = valid_tabpage(lastused_tabpage)
? tabpage_index(lastused_tabpage)
: nr;
} else {
EMSG2(_(e_invexpr2), arg);
}
}
} else {
nr = tabpage_index(curtab);
}
rettv->vval.v_number = nr;
}
static int get_winnr(tabpage_T *tp, typval_T *argvar)
{
win_T *twin;
int nr = 1;
win_T *wp;
twin = (tp == curtab) ? curwin : tp->tp_curwin;
if (argvar->v_type != VAR_UNKNOWN) {
bool invalid_arg = false;
const char *const arg = tv_get_string_chk(argvar);
if (arg == NULL) {
nr = 0; 
} else if (strcmp(arg, "$") == 0) {
twin = (tp == curtab) ? lastwin : tp->tp_lastwin;
} else if (strcmp(arg, "#") == 0) {
twin = (tp == curtab) ? prevwin : tp->tp_prevwin;
if (twin == NULL) {
nr = 0;
}
} else {
char_u *endp;
long count = strtol((char *)arg, (char **)&endp, 10);
if (count <= 0) {
count = 1;
}
if (endp != NULL && *endp != '\0') {
if (strequal((char *)endp, "j")) {
twin = win_vert_neighbor(tp, twin, false, count);
} else if (strequal((char *)endp, "k")) {
twin = win_vert_neighbor(tp, twin, true, count);
} else if (strequal((char *)endp, "h")) {
twin = win_horz_neighbor(tp, twin, true, count);
} else if (strequal((char *)endp, "l")) {
twin = win_horz_neighbor(tp, twin, false, count);
} else {
invalid_arg = true;
}
} else {
invalid_arg = true;
}
}
if (invalid_arg) {
EMSG2(_(e_invexpr2), arg);
nr = 0;
}
}
if (nr > 0)
for (wp = (tp == curtab) ? firstwin : tp->tp_firstwin;
wp != twin; wp = wp->w_next) {
if (wp == NULL) {
nr = 0;
break;
}
++nr;
}
return nr;
}
static void f_tabpagewinnr(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
int nr = 1;
tabpage_T *const tp = find_tabpage((int)tv_get_number(&argvars[0]));
if (tp == NULL) {
nr = 0;
} else {
nr = get_winnr(tp, &argvars[1]);
}
rettv->vval.v_number = nr;
}
static void f_tagfiles(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
char *fname;
tagname_T tn;
tv_list_alloc_ret(rettv, kListLenUnknown);
fname = xmalloc(MAXPATHL);
bool first = true;
while (get_tagfname(&tn, first, (char_u *)fname) == OK) {
tv_list_append_string(rettv->vval.v_list, fname, -1);
first = false;
}
tagname_free(&tn);
xfree(fname);
}
static void f_taglist(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
const char *const tag_pattern = tv_get_string(&argvars[0]);
rettv->vval.v_number = false;
if (*tag_pattern == NUL) {
return;
}
const char *fname = NULL;
if (argvars[1].v_type != VAR_UNKNOWN) {
fname = tv_get_string(&argvars[1]);
}
(void)get_tags(tv_list_alloc_ret(rettv, kListLenUnknown),
(char_u *)tag_pattern, (char_u *)fname);
}
static void f_tempname(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_STRING;
rettv->vval.v_string = vim_tempname();
}
static void f_termopen(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
if (check_restricted() || check_secure()) {
return;
}
if (curbuf->b_changed) {
EMSG(_("Can only call this function in an unmodified buffer"));
return;
}
const char *cmd;
bool executable = true;
char **argv = tv_to_argv(&argvars[0], &cmd, &executable);
if (!argv) {
rettv->vval.v_number = executable ? 0 : -1;
return; 
}
if (argvars[1].v_type != VAR_DICT && argvars[1].v_type != VAR_UNKNOWN) {
EMSG2(_(e_invarg2), "expected dictionary");
shell_free_argv(argv);
return;
}
CallbackReader on_stdout = CALLBACK_READER_INIT,
on_stderr = CALLBACK_READER_INIT;
Callback on_exit = CALLBACK_NONE;
dict_T *job_opts = NULL;
const char *cwd = ".";
if (argvars[1].v_type == VAR_DICT) {
job_opts = argvars[1].vval.v_dict;
const char *const new_cwd = tv_dict_get_string(job_opts, "cwd", false);
if (new_cwd && *new_cwd != NUL) {
cwd = new_cwd;
if (!os_isdir_executable((const char *)cwd)) {
EMSG2(_(e_invarg2), "expected valid directory");
shell_free_argv(argv);
return;
}
}
if (!common_job_callbacks(job_opts, &on_stdout, &on_stderr, &on_exit)) {
shell_free_argv(argv);
return;
}
}
uint16_t term_width = MAX(0, curwin->w_width_inner - win_col_off(curwin));
Channel *chan = channel_job_start(argv, on_stdout, on_stderr, on_exit,
true, false, false, cwd,
term_width, curwin->w_height_inner,
xstrdup("xterm-256color"), NULL,
&rettv->vval.v_number);
if (rettv->vval.v_number <= 0) {
return;
}
int pid = chan->stream.pty.process.pid;
vim_FullName(cwd, (char *)NameBuff, sizeof(NameBuff), false);
size_t len = home_replace(NULL, NameBuff, IObuff, sizeof(IObuff), true);
if (IObuff[len - 1] == '\\' || IObuff[len - 1] == '/') {
IObuff[len - 1] = '\0';
}
snprintf((char *)NameBuff, sizeof(NameBuff), "term://%s//%d:%s",
(char *)IObuff, pid, cmd);
curbuf->b_p_swf = false;
(void)setfname(curbuf, NameBuff, NULL, true);
Error err = ERROR_INIT;
dict_set_var(curbuf->b_vars, cstr_as_string("terminal_job_id"),
INTEGER_OBJ(chan->id), false, false, &err);
api_clear_error(&err);
dict_set_var(curbuf->b_vars, cstr_as_string("terminal_job_pid"),
INTEGER_OBJ(pid), false, false, &err);
api_clear_error(&err);
channel_terminal_open(chan);
channel_create_event(chan, NULL);
}
static void f_test_garbagecollect_now(typval_T *argvars,
typval_T *rettv, FunPtr fptr)
{
garbage_collect(true);
}
static void f_test_write_list_log(typval_T *const argvars,
typval_T *const rettv,
FunPtr fptr)
{
const char *const fname = tv_get_string_chk(&argvars[0]);
if (fname == NULL) {
return;
}
list_write_log(fname);
}
static void f_timer_info(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
if (argvars[0].v_type != VAR_UNKNOWN) {
if (argvars[0].v_type != VAR_NUMBER) {
EMSG(_(e_number_exp));
return;
}
tv_list_alloc_ret(rettv, 1);
timer_T *timer = find_timer_by_nr(tv_get_number(&argvars[0]));
if (timer != NULL && !timer->stopped) {
add_timer_info(rettv, timer);
}
} else {
add_timer_info_all(rettv);
}
}
static void f_timer_pause(typval_T *argvars, typval_T *unused, FunPtr fptr)
{
if (argvars[0].v_type != VAR_NUMBER) {
EMSG(_(e_number_exp));
return;
}
int paused = (bool)tv_get_number(&argvars[1]);
timer_T *timer = find_timer_by_nr(tv_get_number(&argvars[0]));
if (timer != NULL) {
if (!timer->paused && paused) {
time_watcher_stop(&timer->tw);
} else if (timer->paused && !paused) {
time_watcher_start(&timer->tw, timer_due_cb, timer->timeout,
timer->timeout);
}
timer->paused = paused;
}
}
static void f_timer_start(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
int repeat = 1;
dict_T *dict;
rettv->vval.v_number = -1;
if (argvars[2].v_type != VAR_UNKNOWN) {
if (argvars[2].v_type != VAR_DICT
|| (dict = argvars[2].vval.v_dict) == NULL) {
EMSG2(_(e_invarg2), tv_get_string(&argvars[2]));
return;
}
dictitem_T *const di = tv_dict_find(dict, S_LEN("repeat"));
if (di != NULL) {
repeat = tv_get_number(&di->di_tv);
if (repeat == 0) {
repeat = 1;
}
}
}
Callback callback;
if (!callback_from_typval(&callback, &argvars[1])) {
return;
}
rettv->vval.v_number =
timer_start(tv_get_number(&argvars[0]), repeat, &callback);
}
static void f_timer_stop(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
if (argvars[0].v_type != VAR_NUMBER) {
EMSG(_(e_number_exp));
return;
}
timer_T *timer = find_timer_by_nr(tv_get_number(&argvars[0]));
if (timer == NULL) {
return;
}
timer_stop(timer);
}
static void f_timer_stopall(typval_T *argvars, typval_T *unused, FunPtr fptr)
{
timer_stop_all();
}
static void f_tolower(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_STRING;
rettv->vval.v_string = (char_u *)strcase_save(tv_get_string(&argvars[0]),
false);
}
static void f_toupper(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_STRING;
rettv->vval.v_string = (char_u *)strcase_save(tv_get_string(&argvars[0]),
true);
}
static void f_tr(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
char buf[NUMBUFLEN];
char buf2[NUMBUFLEN];
const char *in_str = tv_get_string(&argvars[0]);
const char *fromstr = tv_get_string_buf_chk(&argvars[1], buf);
const char *tostr = tv_get_string_buf_chk(&argvars[2], buf2);
rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;
if (fromstr == NULL || tostr == NULL) {
return; 
}
garray_T ga;
ga_init(&ga, (int)sizeof(char), 80);
if (!has_mbyte) {
if (strlen(fromstr) != strlen(tostr)) {
goto error;
}
}
bool first = true;
while (*in_str != NUL) {
if (has_mbyte) {
const char *cpstr = in_str;
const int inlen = (*mb_ptr2len)((const char_u *)in_str);
int cplen = inlen;
int idx = 0;
int fromlen;
for (const char *p = fromstr; *p != NUL; p += fromlen) {
fromlen = (*mb_ptr2len)((const char_u *)p);
if (fromlen == inlen && STRNCMP(in_str, p, inlen) == 0) {
int tolen;
for (p = tostr; *p != NUL; p += tolen) {
tolen = (*mb_ptr2len)((const char_u *)p);
if (idx-- == 0) {
cplen = tolen;
cpstr = (char *)p;
break;
}
}
if (*p == NUL) { 
goto error;
}
break;
}
idx++;
}
if (first && cpstr == in_str) {
first = false;
int tolen;
for (const char *p = tostr; *p != NUL; p += tolen) {
tolen = (*mb_ptr2len)((const char_u *)p);
idx--;
}
if (idx != 0) {
goto error;
}
}
ga_grow(&ga, cplen);
memmove((char *)ga.ga_data + ga.ga_len, cpstr, (size_t)cplen);
ga.ga_len += cplen;
in_str += inlen;
} else {
const char *const p = strchr(fromstr, *in_str);
if (p != NULL) {
ga_append(&ga, tostr[p - fromstr]);
} else {
ga_append(&ga, *in_str);
}
in_str++;
}
}
ga_append(&ga, NUL);
rettv->vval.v_string = ga.ga_data;
return;
error:
EMSG2(_(e_invarg2), fromstr);
ga_clear(&ga);
return;
}
static void f_trim(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
char buf1[NUMBUFLEN];
char buf2[NUMBUFLEN];
const char_u *head = (const char_u *)tv_get_string_buf_chk(&argvars[0], buf1);
const char_u *mask = NULL;
const char_u *tail;
const char_u *prev;
const char_u *p;
int c1;
rettv->v_type = VAR_STRING;
if (head == NULL) {
rettv->vval.v_string = NULL;
return;
}
if (argvars[1].v_type == VAR_STRING) {
mask = (const char_u *)tv_get_string_buf_chk(&argvars[1], buf2);
}
while (*head != NUL) {
c1 = PTR2CHAR(head);
if (mask == NULL) {
if (c1 > ' ' && c1 != 0xa0) {
break;
}
} else {
for (p = mask; *p != NUL; MB_PTR_ADV(p)) {
if (c1 == PTR2CHAR(p)) {
break;
}
}
if (*p == NUL) {
break;
}
}
MB_PTR_ADV(head);
}
for (tail = head + STRLEN(head); tail > head; tail = prev) {
prev = tail;
MB_PTR_BACK(head, prev);
c1 = PTR2CHAR(prev);
if (mask == NULL) {
if (c1 > ' ' && c1 != 0xa0) {
break;
}
} else {
for (p = mask; *p != NUL; MB_PTR_ADV(p)) {
if (c1 == PTR2CHAR(p)) {
break;
}
}
if (*p == NUL) {
break;
}
}
}
rettv->vval.v_string = vim_strnsave(head, (int)(tail - head));
}
static void f_type(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
int n = -1;
switch (argvars[0].v_type) {
case VAR_NUMBER: n = VAR_TYPE_NUMBER; break;
case VAR_STRING: n = VAR_TYPE_STRING; break;
case VAR_PARTIAL:
case VAR_FUNC: n = VAR_TYPE_FUNC; break;
case VAR_LIST: n = VAR_TYPE_LIST; break;
case VAR_DICT: n = VAR_TYPE_DICT; break;
case VAR_FLOAT: n = VAR_TYPE_FLOAT; break;
case VAR_SPECIAL: {
switch (argvars[0].vval.v_special) {
case kSpecialVarTrue:
case kSpecialVarFalse: {
n = VAR_TYPE_BOOL;
break;
}
case kSpecialVarNull: {
n = 7;
break;
}
}
break;
}
case VAR_UNKNOWN: {
internal_error("f_type(UNKNOWN)");
break;
}
}
rettv->vval.v_number = n;
}
static void f_undofile(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->v_type = VAR_STRING;
const char *const fname = tv_get_string(&argvars[0]);
if (*fname == NUL) {
rettv->vval.v_string = NULL;
} else {
char *ffname = FullName_save(fname, true);
if (ffname != NULL) {
rettv->vval.v_string = (char_u *)u_get_undo_file_name(ffname, false);
}
xfree(ffname);
}
}
static void f_undotree(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
tv_dict_alloc_ret(rettv);
dict_T *dict = rettv->vval.v_dict;
tv_dict_add_nr(dict, S_LEN("synced"), (varnumber_T)curbuf->b_u_synced);
tv_dict_add_nr(dict, S_LEN("seq_last"), (varnumber_T)curbuf->b_u_seq_last);
tv_dict_add_nr(dict, S_LEN("save_last"),
(varnumber_T)curbuf->b_u_save_nr_last);
tv_dict_add_nr(dict, S_LEN("seq_cur"), (varnumber_T)curbuf->b_u_seq_cur);
tv_dict_add_nr(dict, S_LEN("time_cur"), (varnumber_T)curbuf->b_u_time_cur);
tv_dict_add_nr(dict, S_LEN("save_cur"), (varnumber_T)curbuf->b_u_save_nr_cur);
tv_dict_add_list(dict, S_LEN("entries"), u_eval_tree(curbuf->b_u_oldhead));
}
static void f_values(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
dict_list(argvars, rettv, 1);
}
static void f_virtcol(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
colnr_T vcol = 0;
pos_T *fp;
int fnum = curbuf->b_fnum;
fp = var2fpos(&argvars[0], FALSE, &fnum);
if (fp != NULL && fp->lnum <= curbuf->b_ml.ml_line_count
&& fnum == curbuf->b_fnum) {
if (fp->col < 0) {
fp->col = 0;
} else {
const size_t len = STRLEN(ml_get(fp->lnum));
if (fp->col > (colnr_T)len) {
fp->col = (colnr_T)len;
}
}
getvvcol(curwin, fp, NULL, NULL, &vcol);
++vcol;
}
rettv->vval.v_number = vcol;
}
static void f_visualmode(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
char_u str[2];
rettv->v_type = VAR_STRING;
str[0] = curbuf->b_visual_mode_eval;
str[1] = NUL;
rettv->vval.v_string = vim_strsave(str);
if (non_zero_arg(&argvars[0]))
curbuf->b_visual_mode_eval = NUL;
}
static void f_wildmenumode(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
if (wild_menu_showing || ((State & CMDLINE) && pum_visible())) {
rettv->vval.v_number = 1;
}
}
static void f_win_findbuf(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
tv_list_alloc_ret(rettv, kListLenMayKnow);
win_findbuf(argvars, rettv->vval.v_list);
}
static void f_win_getid(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = win_getid(argvars);
}
static void f_win_gotoid(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = win_gotoid(argvars);
}
static void f_win_id2tabwin(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
win_id2tabwin(argvars, rettv);
}
static void f_win_id2win(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = win_id2win(argvars);
}
static void f_winbufnr(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
win_T *wp = find_win_by_nr_or_id(&argvars[0]);
if (wp == NULL) {
rettv->vval.v_number = -1;
} else {
rettv->vval.v_number = wp->w_buffer->b_fnum;
}
}
static void f_wincol(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
validate_cursor();
rettv->vval.v_number = curwin->w_wcol + 1;
}
static void f_winheight(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
win_T *wp = find_win_by_nr_or_id(&argvars[0]);
if (wp == NULL) {
rettv->vval.v_number = -1;
} else {
rettv->vval.v_number = wp->w_height;
}
}
static void f_winlayout(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
tabpage_T *tp;
tv_list_alloc_ret(rettv, 2);
if (argvars[0].v_type == VAR_UNKNOWN) {
tp = curtab;
} else {
tp = find_tabpage((int)tv_get_number(&argvars[0]));
if (tp == NULL) {
return;
}
}
get_framelayout(tp->tp_topframe, rettv->vval.v_list, true);
}
static void f_winline(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
validate_cursor();
rettv->vval.v_number = curwin->w_wrow + 1;
}
static void f_winnr(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
int nr = 1;
nr = get_winnr(curtab, &argvars[0]);
rettv->vval.v_number = nr;
}
static void f_winrestcmd(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
int winnr = 1;
garray_T ga;
char_u buf[50];
ga_init(&ga, (int)sizeof(char), 70);
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
sprintf((char *)buf, "%dresize %d|", winnr, wp->w_height);
ga_concat(&ga, buf);
sprintf((char *)buf, "vert %dresize %d|", winnr, wp->w_width);
ga_concat(&ga, buf);
++winnr;
}
ga_append(&ga, NUL);
rettv->vval.v_string = ga.ga_data;
rettv->v_type = VAR_STRING;
}
static void f_winrestview(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
dict_T *dict;
if (argvars[0].v_type != VAR_DICT
|| (dict = argvars[0].vval.v_dict) == NULL) {
EMSG(_(e_invarg));
} else {
dictitem_T *di;
if ((di = tv_dict_find(dict, S_LEN("lnum"))) != NULL) {
curwin->w_cursor.lnum = tv_get_number(&di->di_tv);
}
if ((di = tv_dict_find(dict, S_LEN("col"))) != NULL) {
curwin->w_cursor.col = tv_get_number(&di->di_tv);
}
if ((di = tv_dict_find(dict, S_LEN("coladd"))) != NULL) {
curwin->w_cursor.coladd = tv_get_number(&di->di_tv);
}
if ((di = tv_dict_find(dict, S_LEN("curswant"))) != NULL) {
curwin->w_curswant = tv_get_number(&di->di_tv);
curwin->w_set_curswant = false;
}
if ((di = tv_dict_find(dict, S_LEN("topline"))) != NULL) {
set_topline(curwin, tv_get_number(&di->di_tv));
}
if ((di = tv_dict_find(dict, S_LEN("topfill"))) != NULL) {
curwin->w_topfill = tv_get_number(&di->di_tv);
}
if ((di = tv_dict_find(dict, S_LEN("leftcol"))) != NULL) {
curwin->w_leftcol = tv_get_number(&di->di_tv);
}
if ((di = tv_dict_find(dict, S_LEN("skipcol"))) != NULL) {
curwin->w_skipcol = tv_get_number(&di->di_tv);
}
check_cursor();
win_new_height(curwin, curwin->w_height);
win_new_width(curwin, curwin->w_width);
changed_window_setting();
if (curwin->w_topline <= 0)
curwin->w_topline = 1;
if (curwin->w_topline > curbuf->b_ml.ml_line_count)
curwin->w_topline = curbuf->b_ml.ml_line_count;
check_topfill(curwin, true);
}
}
static void f_winsaveview(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
dict_T *dict;
tv_dict_alloc_ret(rettv);
dict = rettv->vval.v_dict;
tv_dict_add_nr(dict, S_LEN("lnum"), (varnumber_T)curwin->w_cursor.lnum);
tv_dict_add_nr(dict, S_LEN("col"), (varnumber_T)curwin->w_cursor.col);
tv_dict_add_nr(dict, S_LEN("coladd"), (varnumber_T)curwin->w_cursor.coladd);
update_curswant();
tv_dict_add_nr(dict, S_LEN("curswant"), (varnumber_T)curwin->w_curswant);
tv_dict_add_nr(dict, S_LEN("topline"), (varnumber_T)curwin->w_topline);
tv_dict_add_nr(dict, S_LEN("topfill"), (varnumber_T)curwin->w_topfill);
tv_dict_add_nr(dict, S_LEN("leftcol"), (varnumber_T)curwin->w_leftcol);
tv_dict_add_nr(dict, S_LEN("skipcol"), (varnumber_T)curwin->w_skipcol);
}
static void f_winwidth(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
win_T *wp = find_win_by_nr_or_id(&argvars[0]);
if (wp == NULL) {
rettv->vval.v_number = -1;
} else {
rettv->vval.v_number = wp->w_width;
}
}
static void f_wordcount(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
tv_dict_alloc_ret(rettv);
cursor_pos_info(rettv->vval.v_dict);
}
static void f_writefile(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = -1;
if (check_secure()) {
return;
}
if (argvars[0].v_type != VAR_LIST) {
EMSG2(_(e_listarg), "writefile()");
return;
}
const list_T *const list = argvars[0].vval.v_list;
TV_LIST_ITER_CONST(list, li, {
if (!tv_check_str_or_nr(TV_LIST_ITEM_TV(li))) {
return;
}
});
bool binary = false;
bool append = false;
bool do_fsync = !!p_fs;
if (argvars[2].v_type != VAR_UNKNOWN) {
const char *const flags = tv_get_string_chk(&argvars[2]);
if (flags == NULL) {
return;
}
for (const char *p = flags; *p; p++) {
switch (*p) {
case 'b': { binary = true; break; }
case 'a': { append = true; break; }
case 's': { do_fsync = true; break; }
case 'S': { do_fsync = false; break; }
default: {
emsgf(_("E5060: Unknown flag: %s"), p);
return;
}
}
}
}
char buf[NUMBUFLEN];
const char *const fname = tv_get_string_buf_chk(&argvars[1], buf);
if (fname == NULL) {
return;
}
FileDescriptor fp;
int error;
if (*fname == NUL) {
EMSG(_("E482: Can't open file with an empty name"));
} else if ((error = file_open(&fp, fname,
((append ? kFileAppend : kFileTruncate)
| kFileCreate), 0666)) != 0) {
emsgf(_("E482: Can't open file %s for writing: %s"),
fname, os_strerror(error));
} else {
if (write_list(&fp, list, binary)) {
rettv->vval.v_number = 0;
}
if ((error = file_close(&fp, do_fsync)) != 0) {
emsgf(_("E80: Error when closing file %s: %s"),
fname, os_strerror(error));
}
}
}
static void f_xor(typval_T *argvars, typval_T *rettv, FunPtr fptr)
{
rettv->vval.v_number = tv_get_number_chk(&argvars[0], NULL)
^ tv_get_number_chk(&argvars[1], NULL);
}
