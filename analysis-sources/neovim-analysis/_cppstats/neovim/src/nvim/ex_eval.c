#include <assert.h>
#include <stdbool.h>
#include <inttypes.h>
#include <limits.h>
#include "nvim/vim.h"
#include "nvim/ascii.h"
#include "nvim/ex_eval.h"
#include "nvim/charset.h"
#include "nvim/eval.h"
#include "nvim/eval/typval.h"
#include "nvim/ex_cmds2.h"
#include "nvim/ex_docmd.h"
#include "nvim/message.h"
#include "nvim/memory.h"
#include "nvim/regexp.h"
#include "nvim/strings.h"
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "ex_eval.c.generated.h"
#endif
#define THROW_ON_ERROR true
#define THROW_ON_ERROR_TRUE
#define THROW_ON_INTERRUPT true
#define THROW_ON_INTERRUPT_TRUE
#define CHECK_SKIP (did_emsg || got_int || current_exception || (cstack->cs_idx > 0 && !(cstack->cs_flags[cstack->cs_idx - 1] & CSF_ACTIVE)))
#define discard_pending_return(p) tv_free((typval_T *)(p))
static int cause_abort = FALSE;
int aborting(void)
{
return (did_emsg && force_abort) || got_int || current_exception;
}
void update_force_abort(void)
{
if (cause_abort)
force_abort = TRUE;
}
int should_abort(int retcode)
{
return (retcode == FAIL && trylevel != 0 && !emsg_silent) || aborting();
}
int aborted_in_try(void)
{
return force_abort;
}
int cause_errthrow(char_u *mesg, int severe, int *ignore)
{
struct msglist *elem;
struct msglist **plist;
if (suppress_errthrow)
return FALSE;
if (!did_emsg) {
cause_abort = force_abort;
force_abort = FALSE;
}
if (((trylevel == 0 && !cause_abort) || emsg_silent) && !current_exception) {
return false;
}
if (mesg == (char_u *)_(e_interr)) {
*ignore = TRUE;
return TRUE;
}
cause_abort = TRUE;
if (current_exception) {
if (current_exception->type == ET_INTERRUPT) {
got_int = false;
}
discard_current_exception();
}
#if defined(THROW_TEST)
if (!THROW_ON_ERROR) {
return FALSE;
} else
#endif
{
if (msg_list != NULL) {
plist = msg_list;
while (*plist != NULL)
plist = &(*plist)->next;
elem = xmalloc(sizeof(struct msglist));
elem->msg = vim_strsave(mesg);
elem->next = NULL;
elem->throw_msg = NULL;
*plist = elem;
if (plist == msg_list || severe) {
char_u *tmsg;
tmsg = elem->msg;
if (STRNCMP(tmsg, "Vim E", 5) == 0
&& ascii_isdigit(tmsg[5])
&& ascii_isdigit(tmsg[6])
&& ascii_isdigit(tmsg[7])
&& tmsg[8] == ':'
&& tmsg[9] == ' ')
(*msg_list)->throw_msg = &tmsg[4];
else
(*msg_list)->throw_msg = tmsg;
}
}
return TRUE;
}
}
static void free_msglist(struct msglist *l)
{
struct msglist *messages, *next;
messages = l;
while (messages != NULL) {
next = messages->next;
xfree(messages->msg);
xfree(messages);
messages = next;
}
}
void free_global_msglist(void)
{
free_msglist(*msg_list);
*msg_list = NULL;
}
void do_errthrow(cstack_T *cstack, char_u *cmdname)
{
if (cause_abort) {
cause_abort = FALSE;
force_abort = TRUE;
}
if (msg_list == NULL || *msg_list == NULL)
return;
if (throw_exception(*msg_list, ET_ERROR, cmdname) == FAIL)
free_msglist(*msg_list);
else {
if (cstack != NULL)
do_throw(cstack);
else
need_rethrow = TRUE;
}
*msg_list = NULL;
}
int do_intthrow(cstack_T *cstack)
{
if (!got_int || (trylevel == 0 && !current_exception)) {
return false;
}
#if defined(THROW_TEST)
if (!THROW_ON_INTERRUPT) {
if (current_exception) {
discard_current_exception();
}
} else {
#endif
if (current_exception) {
if (current_exception->type == ET_INTERRUPT) {
return false;
}
discard_current_exception();
}
if (throw_exception("Vim:Interrupt", ET_INTERRUPT, NULL) != FAIL) {
do_throw(cstack);
}
#if defined(THROW_TEST)
}
#endif
return true;
}
char_u *get_exception_string(void *value, except_type_T type, char_u *cmdname,
int *should_free)
{
char_u *ret, *mesg;
char_u *p, *val;
if (type == ET_ERROR) {
*should_free = true;
mesg = ((struct msglist *)value)->throw_msg;
if (cmdname != NULL && *cmdname != NUL) {
size_t cmdlen = STRLEN(cmdname);
ret = vim_strnsave((char_u *)"Vim(", 4 + cmdlen + 2 + STRLEN(mesg));
STRCPY(&ret[4], cmdname);
STRCPY(&ret[4 + cmdlen], "):");
val = ret + 4 + cmdlen + 2;
} else {
ret = vim_strnsave((char_u *)"Vim:", 4 + STRLEN(mesg));
val = ret + 4;
}
for (p = mesg;; p++) {
if (*p == NUL
|| (*p == 'E'
&& ascii_isdigit(p[1])
&& (p[2] == ':'
|| (ascii_isdigit(p[2])
&& (p[3] == ':'
|| (ascii_isdigit(p[3])
&& p[4] == ':')))))) {
if (*p == NUL || p == mesg) {
STRCAT(val, mesg); 
} else {
if (mesg[0] != '"' || p-2 < &mesg[1]
|| p[-2] != '"' || p[-1] != ' ') {
continue;
}
STRCAT(val, p);
p[-2] = NUL;
sprintf((char *)(val + STRLEN(p)), " (%s)", &mesg[1]);
p[-2] = '"';
}
break;
}
}
} else {
*should_free = FALSE;
ret = (char_u *) value;
}
return ret;
}
static int throw_exception(void *value, except_type_T type, char_u *cmdname)
{
except_T *excp;
int should_free;
if (type == ET_USER) {
if (STRNCMP((char_u *)value, "Vim", 3) == 0
&& (((char_u *)value)[3] == NUL || ((char_u *)value)[3] == ':'
|| ((char_u *)value)[3] == '(')) {
EMSG(_("E608: Cannot :throw exceptions with 'Vim' prefix"));
goto fail;
}
}
excp = xmalloc(sizeof(except_T));
if (type == ET_ERROR)
excp->messages = (struct msglist *)value;
excp->value = get_exception_string(value, type, cmdname, &should_free);
if (excp->value == NULL && should_free)
goto nomem;
excp->type = type;
excp->throw_name = vim_strsave(sourcing_name == NULL
? (char_u *)"" : sourcing_name);
excp->throw_lnum = sourcing_lnum;
if (p_verbose >= 13 || debug_break_level > 0) {
int save_msg_silent = msg_silent;
if (debug_break_level > 0)
msg_silent = FALSE; 
else
verbose_enter();
++no_wait_return;
if (debug_break_level > 0 || *p_vfile == NUL)
msg_scroll = TRUE; 
smsg(_("Exception thrown: %s"), excp->value);
msg_puts("\n"); 
if (debug_break_level > 0 || *p_vfile == NUL)
cmdline_row = msg_row;
--no_wait_return;
if (debug_break_level > 0)
msg_silent = save_msg_silent;
else
verbose_leave();
}
current_exception = excp;
return OK;
nomem:
xfree(excp);
suppress_errthrow = TRUE;
EMSG(_(e_outofmem));
fail:
current_exception = NULL;
return FAIL;
}
static void discard_exception(except_T *excp, int was_finished)
{
char_u *saved_IObuff;
if (excp == NULL) {
internal_error("discard_exception()");
return;
}
if (p_verbose >= 13 || debug_break_level > 0) {
int save_msg_silent = msg_silent;
saved_IObuff = vim_strsave(IObuff);
if (debug_break_level > 0)
msg_silent = FALSE; 
else
verbose_enter();
++no_wait_return;
if (debug_break_level > 0 || *p_vfile == NUL)
msg_scroll = TRUE; 
smsg(was_finished ? _("Exception finished: %s")
: _("Exception discarded: %s"),
excp->value);
msg_puts("\n"); 
if (debug_break_level > 0 || *p_vfile == NUL) {
cmdline_row = msg_row;
}
no_wait_return--;
if (debug_break_level > 0) {
msg_silent = save_msg_silent;
} else {
verbose_leave();
}
xstrlcpy((char *)IObuff, (const char *)saved_IObuff, IOSIZE);
xfree(saved_IObuff);
}
if (excp->type != ET_INTERRUPT)
xfree(excp->value);
if (excp->type == ET_ERROR)
free_msglist(excp->messages);
xfree(excp->throw_name);
xfree(excp);
}
void discard_current_exception(void)
{
if (current_exception != NULL) {
discard_exception(current_exception, false);
current_exception = NULL;
}
need_rethrow = false;
}
static void catch_exception(except_T *excp)
{
excp->caught = caught_stack;
caught_stack = excp;
set_vim_var_string(VV_EXCEPTION, (char *) excp->value, -1);
if (*excp->throw_name != NUL) {
if (excp->throw_lnum != 0) {
vim_snprintf((char *)IObuff, IOSIZE, _("%s, line %" PRId64),
excp->throw_name, (int64_t)excp->throw_lnum);
} else {
vim_snprintf((char *)IObuff, IOSIZE, "%s", excp->throw_name);
}
set_vim_var_string(VV_THROWPOINT, (char *) IObuff, -1);
} else {
set_vim_var_string(VV_THROWPOINT, NULL, -1);
}
if (p_verbose >= 13 || debug_break_level > 0) {
int save_msg_silent = msg_silent;
if (debug_break_level > 0)
msg_silent = FALSE; 
else
verbose_enter();
++no_wait_return;
if (debug_break_level > 0 || *p_vfile == NUL)
msg_scroll = TRUE; 
smsg(_("Exception caught: %s"), excp->value);
msg_puts("\n"); 
if (debug_break_level > 0 || *p_vfile == NUL)
cmdline_row = msg_row;
--no_wait_return;
if (debug_break_level > 0)
msg_silent = save_msg_silent;
else
verbose_leave();
}
}
static void finish_exception(except_T *excp)
{
if (excp != caught_stack) {
internal_error("finish_exception()");
}
caught_stack = caught_stack->caught;
if (caught_stack != NULL) {
set_vim_var_string(VV_EXCEPTION, (char *) caught_stack->value, -1);
if (*caught_stack->throw_name != NUL) {
if (caught_stack->throw_lnum != 0) {
vim_snprintf((char *)IObuff, IOSIZE,
_("%s, line %" PRId64), caught_stack->throw_name,
(int64_t)caught_stack->throw_lnum);
} else {
vim_snprintf((char *)IObuff, IOSIZE, "%s",
caught_stack->throw_name);
}
set_vim_var_string(VV_THROWPOINT, (char *) IObuff, -1);
} else {
set_vim_var_string(VV_THROWPOINT, NULL, -1);
}
} else {
set_vim_var_string(VV_EXCEPTION, NULL, -1);
set_vim_var_string(VV_THROWPOINT, NULL, -1);
}
discard_exception(excp, TRUE);
}
#define RP_MAKE 0
#define RP_RESUME 1
#define RP_DISCARD 2
static void report_pending(int action, int pending, void *value)
{
char *mesg;
char *s;
int save_msg_silent;
assert(value || !(pending & CSTP_THROW));
switch (action) {
case RP_MAKE:
mesg = _("%s made pending");
break;
case RP_RESUME:
mesg = _("%s resumed");
break;
default:
mesg = _("%s discarded");
break;
}
switch (pending) {
case CSTP_NONE:
return;
case CSTP_CONTINUE:
s = ":continue";
break;
case CSTP_BREAK:
s = ":break";
break;
case CSTP_FINISH:
s = ":finish";
break;
case CSTP_RETURN:
s = (char *)get_return_cmd(value);
break;
default:
if (pending & CSTP_THROW) {
vim_snprintf((char *)IObuff, IOSIZE,
mesg, _("Exception"));
mesg = (char *)concat_str(IObuff, (char_u *)": %s");
s = (char *)((except_T *)value)->value;
} else if ((pending & CSTP_ERROR) && (pending & CSTP_INTERRUPT))
s = _("Error and interrupt");
else if (pending & CSTP_ERROR)
s = _("Error");
else 
s = _("Interrupt");
}
save_msg_silent = msg_silent;
if (debug_break_level > 0)
msg_silent = FALSE; 
++no_wait_return;
msg_scroll = TRUE; 
smsg(mesg, s);
msg_puts("\n"); 
cmdline_row = msg_row;
--no_wait_return;
if (debug_break_level > 0)
msg_silent = save_msg_silent;
if (pending == CSTP_RETURN)
xfree(s);
else if (pending & CSTP_THROW)
xfree(mesg);
}
void report_make_pending(int pending, void *value)
{
if (p_verbose >= 14 || debug_break_level > 0) {
if (debug_break_level <= 0)
verbose_enter();
report_pending(RP_MAKE, pending, value);
if (debug_break_level <= 0)
verbose_leave();
}
}
void report_resume_pending(int pending, void *value)
{
if (p_verbose >= 14 || debug_break_level > 0) {
if (debug_break_level <= 0)
verbose_enter();
report_pending(RP_RESUME, pending, value);
if (debug_break_level <= 0)
verbose_leave();
}
}
void report_discard_pending(int pending, void *value)
{
if (p_verbose >= 14 || debug_break_level > 0) {
if (debug_break_level <= 0)
verbose_enter();
report_pending(RP_DISCARD, pending, value);
if (debug_break_level <= 0)
verbose_leave();
}
}
void ex_if(exarg_T *eap)
{
int skip;
int result;
cstack_T *const cstack = eap->cstack;
if (cstack->cs_idx == CSTACK_LEN - 1)
eap->errmsg = (char_u *)N_("E579: :if nesting too deep");
else {
++cstack->cs_idx;
cstack->cs_flags[cstack->cs_idx] = 0;
skip = CHECK_SKIP;
bool error;
result = eval_to_bool(eap->arg, &error, &eap->nextcmd, skip);
if (!skip && !error) {
if (result)
cstack->cs_flags[cstack->cs_idx] = CSF_ACTIVE | CSF_TRUE;
} else
cstack->cs_flags[cstack->cs_idx] = CSF_TRUE;
}
}
void ex_endif(exarg_T *eap)
{
did_endif = TRUE;
if (eap->cstack->cs_idx < 0
|| (eap->cstack->cs_flags[eap->cstack->cs_idx]
& (CSF_WHILE | CSF_FOR | CSF_TRY)))
eap->errmsg = (char_u *)N_("E580: :endif without :if");
else {
if (!(eap->cstack->cs_flags[eap->cstack->cs_idx] & CSF_TRUE)
&& dbg_check_skipped(eap))
(void)do_intthrow(eap->cstack);
--eap->cstack->cs_idx;
}
}
void ex_else(exarg_T *eap)
{
int skip;
int result;
cstack_T *const cstack = eap->cstack;
skip = CHECK_SKIP;
if (cstack->cs_idx < 0
|| (cstack->cs_flags[cstack->cs_idx]
& (CSF_WHILE | CSF_FOR | CSF_TRY))) {
if (eap->cmdidx == CMD_else) {
eap->errmsg = (char_u *)N_("E581: :else without :if");
return;
}
eap->errmsg = (char_u *)N_("E582: :elseif without :if");
skip = TRUE;
} else if (cstack->cs_flags[cstack->cs_idx] & CSF_ELSE) {
if (eap->cmdidx == CMD_else) {
eap->errmsg = (char_u *)N_("E583: multiple :else");
return;
}
eap->errmsg = (char_u *)N_("E584: :elseif after :else");
skip = TRUE;
}
if (skip || cstack->cs_flags[cstack->cs_idx] & CSF_TRUE) {
if (eap->errmsg == NULL)
cstack->cs_flags[cstack->cs_idx] = CSF_TRUE;
skip = TRUE; 
} else
cstack->cs_flags[cstack->cs_idx] = CSF_ACTIVE;
if (!skip && dbg_check_skipped(eap) && got_int) {
(void)do_intthrow(cstack);
skip = TRUE;
}
if (eap->cmdidx == CMD_elseif) {
bool error;
result = eval_to_bool(eap->arg, &error, &eap->nextcmd, skip);
if (!skip && !error) {
if (result)
cstack->cs_flags[cstack->cs_idx] = CSF_ACTIVE | CSF_TRUE;
else
cstack->cs_flags[cstack->cs_idx] = 0;
} else if (eap->errmsg == NULL)
cstack->cs_flags[cstack->cs_idx] = CSF_TRUE;
} else
cstack->cs_flags[cstack->cs_idx] |= CSF_ELSE;
}
void ex_while(exarg_T *eap)
{
bool error;
int skip;
int result;
cstack_T *const cstack = eap->cstack;
if (cstack->cs_idx == CSTACK_LEN - 1)
eap->errmsg = (char_u *)N_("E585: :while/:for nesting too deep");
else {
if ((cstack->cs_lflags & CSL_HAD_LOOP) == 0) {
++cstack->cs_idx;
++cstack->cs_looplevel;
cstack->cs_line[cstack->cs_idx] = -1;
}
cstack->cs_flags[cstack->cs_idx] =
eap->cmdidx == CMD_while ? CSF_WHILE : CSF_FOR;
skip = CHECK_SKIP;
if (eap->cmdidx == CMD_while) {
result = eval_to_bool(eap->arg, &error, &eap->nextcmd, skip);
} else {
void *fi;
if ((cstack->cs_lflags & CSL_HAD_LOOP) != 0) {
fi = cstack->cs_forinfo[cstack->cs_idx];
error = FALSE;
} else {
fi = eval_for_line(eap->arg, &error, &eap->nextcmd, skip);
cstack->cs_forinfo[cstack->cs_idx] = fi;
}
if (!error && fi != NULL && !skip)
result = next_for_item(fi, eap->arg);
else
result = FALSE;
if (!result) {
free_for_info(fi);
cstack->cs_forinfo[cstack->cs_idx] = NULL;
}
}
if (!skip && !error && result) {
cstack->cs_flags[cstack->cs_idx] |= (CSF_ACTIVE | CSF_TRUE);
cstack->cs_lflags ^= CSL_HAD_LOOP;
} else {
cstack->cs_lflags &= ~CSL_HAD_LOOP;
if (!skip && !error)
cstack->cs_flags[cstack->cs_idx] |= CSF_TRUE;
}
}
}
void ex_continue(exarg_T *eap)
{
int idx;
cstack_T *const cstack = eap->cstack;
if (cstack->cs_looplevel <= 0 || cstack->cs_idx < 0)
eap->errmsg = (char_u *)N_("E586: :continue without :while or :for");
else {
idx = cleanup_conditionals(cstack, CSF_WHILE | CSF_FOR, FALSE);
assert(idx >= 0);
if (cstack->cs_flags[idx] & (CSF_WHILE | CSF_FOR)) {
rewind_conditionals(cstack, idx, CSF_TRY, &cstack->cs_trylevel);
cstack->cs_lflags |= CSL_HAD_CONT; 
} else {
cstack->cs_pending[idx] = CSTP_CONTINUE;
report_make_pending(CSTP_CONTINUE, NULL);
}
}
}
void ex_break(exarg_T *eap)
{
int idx;
cstack_T *const cstack = eap->cstack;
if (cstack->cs_looplevel <= 0 || cstack->cs_idx < 0)
eap->errmsg = (char_u *)N_("E587: :break without :while or :for");
else {
idx = cleanup_conditionals(cstack, CSF_WHILE | CSF_FOR, TRUE);
if (idx >= 0 && !(cstack->cs_flags[idx] & (CSF_WHILE | CSF_FOR))) {
cstack->cs_pending[idx] = CSTP_BREAK;
report_make_pending(CSTP_BREAK, NULL);
}
}
}
void ex_endwhile(exarg_T *eap)
{
cstack_T *const cstack = eap->cstack;
int idx;
char_u *err;
int csf;
int fl;
if (eap->cmdidx == CMD_endwhile) {
err = e_while;
csf = CSF_WHILE;
} else {
err = e_for;
csf = CSF_FOR;
}
if (cstack->cs_looplevel <= 0 || cstack->cs_idx < 0)
eap->errmsg = err;
else {
fl = cstack->cs_flags[cstack->cs_idx];
if (!(fl & csf)) {
if (fl & CSF_WHILE)
eap->errmsg = (char_u *)_("E732: Using :endfor with :while");
else if (fl & CSF_FOR)
eap->errmsg = (char_u *)_("E733: Using :endwhile with :for");
}
if (!(fl & (CSF_WHILE | CSF_FOR))) {
if (!(fl & CSF_TRY))
eap->errmsg = e_endif;
else if (fl & CSF_FINALLY)
eap->errmsg = e_endtry;
for (idx = cstack->cs_idx; idx > 0; --idx) {
fl = cstack->cs_flags[idx];
if ((fl & CSF_TRY) && !(fl & CSF_FINALLY)) {
eap->errmsg = err;
return;
}
if (fl & csf)
break;
}
(void)cleanup_conditionals(cstack, CSF_WHILE | CSF_FOR, FALSE);
rewind_conditionals(cstack, idx, CSF_TRY, &cstack->cs_trylevel);
}
else if (cstack->cs_flags[cstack->cs_idx] & CSF_TRUE
&& !(cstack->cs_flags[cstack->cs_idx] & CSF_ACTIVE)
&& dbg_check_skipped(eap))
(void)do_intthrow(cstack);
cstack->cs_lflags |= CSL_HAD_ENDLOOP;
}
}
void ex_throw(exarg_T *eap)
{
const char *arg = (const char *)eap->arg;
char *value;
if (*arg != NUL && *arg != '|' && *arg != '\n') {
value = eval_to_string_skip(arg, (const char **)&eap->nextcmd,
(bool)eap->skip);
} else {
EMSG(_(e_argreq));
value = NULL;
}
if (!eap->skip && value != NULL) {
if (throw_exception((char_u *)value, ET_USER, NULL) == FAIL) {
xfree(value);
} else {
do_throw(eap->cstack);
}
}
}
void do_throw(cstack_T *cstack)
{
int idx;
int inactivate_try = FALSE;
#if !defined(THROW_ON_ERROR_TRUE)
if (did_emsg && !THROW_ON_ERROR) {
inactivate_try = TRUE;
did_emsg = FALSE;
}
#endif
#if !defined(THROW_ON_INTERRUPT_TRUE)
if (got_int && !THROW_ON_INTERRUPT) {
inactivate_try = TRUE;
got_int = FALSE;
}
#endif
idx = cleanup_conditionals(cstack, 0, inactivate_try);
if (idx >= 0) {
if (!(cstack->cs_flags[idx] & CSF_CAUGHT)) {
if (cstack->cs_flags[idx] & CSF_ACTIVE)
cstack->cs_flags[idx] |= CSF_THROWN;
else
cstack->cs_flags[idx] &= ~CSF_THROWN;
}
cstack->cs_flags[idx] &= ~CSF_ACTIVE;
cstack->cs_exception[idx] = current_exception;
}
}
void ex_try(exarg_T *eap)
{
int skip;
cstack_T *const cstack = eap->cstack;
if (cstack->cs_idx == CSTACK_LEN - 1)
eap->errmsg = (char_u *)N_("E601: :try nesting too deep");
else {
++cstack->cs_idx;
++cstack->cs_trylevel;
cstack->cs_flags[cstack->cs_idx] = CSF_TRY;
cstack->cs_pending[cstack->cs_idx] = CSTP_NONE;
skip = CHECK_SKIP;
if (!skip) {
cstack->cs_flags[cstack->cs_idx] |= CSF_ACTIVE | CSF_TRUE;
if (emsg_silent) {
eslist_T *elem = xmalloc(sizeof(*elem));
elem->saved_emsg_silent = emsg_silent;
elem->next = cstack->cs_emsg_silent_list;
cstack->cs_emsg_silent_list = elem;
cstack->cs_flags[cstack->cs_idx] |= CSF_SILENT;
emsg_silent = 0;
}
}
}
}
void ex_catch(exarg_T *eap)
{
int idx = 0;
int give_up = FALSE;
int skip = FALSE;
int caught = FALSE;
char_u *end;
char_u save_char = 0;
char_u *save_cpo;
regmatch_T regmatch;
int prev_got_int;
cstack_T *const cstack = eap->cstack;
char_u *pat;
if (cstack->cs_trylevel <= 0 || cstack->cs_idx < 0) {
eap->errmsg = (char_u *)N_("E603: :catch without :try");
give_up = TRUE;
} else {
if (!(cstack->cs_flags[cstack->cs_idx] & CSF_TRY)) {
eap->errmsg = get_end_emsg(cstack);
skip = TRUE;
}
for (idx = cstack->cs_idx; idx > 0; --idx)
if (cstack->cs_flags[idx] & CSF_TRY)
break;
if (cstack->cs_flags[idx] & CSF_FINALLY) {
eap->errmsg = (char_u *)N_("E604: :catch after :finally");
give_up = TRUE;
} else
rewind_conditionals(cstack, idx, CSF_WHILE | CSF_FOR,
&cstack->cs_looplevel);
}
if (ends_excmd(*eap->arg)) { 
pat = (char_u *)".*";
end = NULL;
eap->nextcmd = find_nextcmd(eap->arg);
} else {
pat = eap->arg + 1;
end = skip_regexp(pat, *eap->arg, TRUE, NULL);
}
if (!give_up) {
if (!current_exception || !(cstack->cs_flags[idx] & CSF_TRUE)) {
skip = true;
}
if (!skip && (cstack->cs_flags[idx] & CSF_THROWN)
&& !(cstack->cs_flags[idx] & CSF_CAUGHT)) {
if (end != NULL && *end != NUL && !ends_excmd(*skipwhite(end + 1))) {
EMSG(_(e_trailing));
return;
}
if (!dbg_check_skipped(eap) || !do_intthrow(cstack)) {
if (end != NULL) {
save_char = *end;
*end = NUL;
}
save_cpo = p_cpo;
p_cpo = (char_u *)"";
emsg_off++;
regmatch.regprog = vim_regcomp(pat, RE_MAGIC + RE_STRING);
emsg_off--;
regmatch.rm_ic = false;
if (end != NULL) {
*end = save_char;
}
p_cpo = save_cpo;
if (regmatch.regprog == NULL) {
EMSG2(_(e_invarg2), pat);
} else {
prev_got_int = got_int;
got_int = FALSE;
caught = vim_regexec_nl(&regmatch, current_exception->value,
(colnr_T)0);
got_int |= prev_got_int;
vim_regfree(regmatch.regprog);
}
}
}
if (caught) {
cstack->cs_flags[idx] |= CSF_ACTIVE | CSF_CAUGHT;
did_emsg = got_int = false;
catch_exception((except_T *)cstack->cs_exception[idx]);
if (cstack->cs_exception[cstack->cs_idx] != current_exception) {
internal_error("ex_catch()");
}
current_exception = NULL;
} else {
cleanup_conditionals(cstack, CSF_TRY, TRUE);
}
}
if (end != NULL)
eap->nextcmd = find_nextcmd(end);
}
void ex_finally(exarg_T *eap)
{
int idx;
int skip = FALSE;
int pending = CSTP_NONE;
cstack_T *const cstack = eap->cstack;
if (cstack->cs_trylevel <= 0 || cstack->cs_idx < 0)
eap->errmsg = (char_u *)N_("E606: :finally without :try");
else {
if (!(cstack->cs_flags[cstack->cs_idx] & CSF_TRY)) {
eap->errmsg = get_end_emsg(cstack);
for (idx = cstack->cs_idx - 1; idx > 0; --idx)
if (cstack->cs_flags[idx] & CSF_TRY)
break;
pending = CSTP_ERROR;
} else
idx = cstack->cs_idx;
if (cstack->cs_flags[idx] & CSF_FINALLY) {
eap->errmsg = (char_u *)N_("E607: multiple :finally");
return;
}
rewind_conditionals(cstack, idx, CSF_WHILE | CSF_FOR,
&cstack->cs_looplevel);
skip = !(cstack->cs_flags[cstack->cs_idx] & CSF_TRUE);
if (!skip) {
if (dbg_check_skipped(eap)) {
(void)do_intthrow(cstack);
}
cleanup_conditionals(cstack, CSF_TRY, FALSE);
if (pending == CSTP_ERROR || did_emsg || got_int || current_exception) {
if (cstack->cs_pending[cstack->cs_idx] == CSTP_RETURN) {
report_discard_pending(CSTP_RETURN,
cstack->cs_rettv[cstack->cs_idx]);
discard_pending_return(cstack->cs_rettv[cstack->cs_idx]);
}
if (pending == CSTP_ERROR && !did_emsg) {
pending |= (THROW_ON_ERROR ? CSTP_THROW : 0);
} else {
pending |= (current_exception ? CSTP_THROW : 0);
}
pending |= did_emsg ? CSTP_ERROR : 0;
pending |= got_int ? CSTP_INTERRUPT : 0;
assert(pending >= CHAR_MIN && pending <= CHAR_MAX);
cstack->cs_pending[cstack->cs_idx] = (char)pending;
if (current_exception
&& cstack->cs_exception[cstack->cs_idx] != current_exception) {
internal_error("ex_finally()");
}
}
cstack->cs_lflags |= CSL_HAD_FINA;
}
}
}
void ex_endtry(exarg_T *eap)
{
int idx;
int skip;
int rethrow = FALSE;
int pending = CSTP_NONE;
void *rettv = NULL;
cstack_T *const cstack = eap->cstack;
if (cstack->cs_trylevel <= 0 || cstack->cs_idx < 0) {
eap->errmsg = (char_u *)N_("E602: :endtry without :try");
} else {
skip = (did_emsg || got_int || current_exception
|| !(cstack->cs_flags[cstack->cs_idx] & CSF_TRUE));
if (!(cstack->cs_flags[cstack->cs_idx] & CSF_TRY)) {
eap->errmsg = get_end_emsg(cstack);
idx = cstack->cs_idx;
do
--idx;
while (idx > 0 && !(cstack->cs_flags[idx] & CSF_TRY));
rewind_conditionals(cstack, idx, CSF_WHILE | CSF_FOR,
&cstack->cs_looplevel);
skip = TRUE;
if (current_exception) {
discard_current_exception();
}
} else {
idx = cstack->cs_idx;
if (current_exception
&& (cstack->cs_flags[idx] & CSF_TRUE)
&& !(cstack->cs_flags[idx] & CSF_FINALLY)) {
rethrow = true;
}
}
if ((rethrow || (!skip
&& !(cstack->cs_flags[idx] & CSF_FINALLY)
&& !cstack->cs_pending[idx]))
&& dbg_check_skipped(eap)) {
if (got_int) {
skip = TRUE;
(void)do_intthrow(cstack);
rethrow = false;
if (current_exception && !(cstack->cs_flags[idx] & CSF_FINALLY)) {
rethrow = true;
}
}
}
if (!skip) {
pending = cstack->cs_pending[idx];
cstack->cs_pending[idx] = CSTP_NONE;
if (pending == CSTP_RETURN)
rettv = cstack->cs_rettv[idx];
else if (pending & CSTP_THROW)
current_exception = cstack->cs_exception[idx];
}
(void)cleanup_conditionals(cstack, CSF_TRY | CSF_SILENT, TRUE);
--cstack->cs_idx;
--cstack->cs_trylevel;
if (!skip) {
report_resume_pending(pending,
(pending == CSTP_RETURN) ? rettv :
(pending & CSTP_THROW) ? (void *)current_exception : NULL);
switch (pending) {
case CSTP_NONE:
break;
case CSTP_CONTINUE:
ex_continue(eap);
break;
case CSTP_BREAK:
ex_break(eap);
break;
case CSTP_RETURN:
do_return(eap, FALSE, FALSE, rettv);
break;
case CSTP_FINISH:
do_finish(eap, FALSE);
break;
default:
if (pending & CSTP_ERROR) {
did_emsg = true;
}
if (pending & CSTP_INTERRUPT) {
got_int = true;
}
if (pending & CSTP_THROW) {
rethrow = true;
}
break;
}
}
if (rethrow) {
do_throw(cstack);
}
}
}
void enter_cleanup(cleanup_T *csp)
{
int pending = CSTP_NONE;
if (did_emsg || got_int || current_exception || need_rethrow) {
csp->pending = (did_emsg ? CSTP_ERROR : 0)
| (got_int ? CSTP_INTERRUPT : 0)
| (current_exception ? CSTP_THROW : 0)
| (need_rethrow ? CSTP_THROW : 0);
if (current_exception || need_rethrow) {
csp->exception = current_exception;
current_exception = NULL;
} else {
csp->exception = NULL;
if (did_emsg) {
force_abort |= cause_abort;
cause_abort = FALSE;
}
}
did_emsg = got_int = need_rethrow = false;
current_exception = NULL;
report_make_pending(pending, csp->exception);
} else {
csp->pending = CSTP_NONE;
csp->exception = NULL;
}
}
void leave_cleanup(cleanup_T *csp)
{
int pending = csp->pending;
if (pending == CSTP_NONE) 
return;
if (aborting() || need_rethrow) {
if (pending & CSTP_THROW)
discard_exception(csp->exception, FALSE);
else
report_discard_pending(pending, NULL);
if (msg_list != NULL)
free_global_msglist();
}
else {
if (pending & CSTP_THROW)
current_exception = csp->exception;
else if (pending & CSTP_ERROR) {
cause_abort = force_abort;
force_abort = FALSE;
}
if (pending & CSTP_ERROR) {
did_emsg = true;
}
if (pending & CSTP_INTERRUPT) {
got_int = true;
}
if (pending & CSTP_THROW) {
need_rethrow = true; 
}
report_resume_pending(
pending, ((pending & CSTP_THROW) ? (void *)current_exception : NULL));
}
}
int cleanup_conditionals(cstack_T *cstack, int searched_cond, int inclusive)
{
int idx;
int stop = FALSE;
for (idx = cstack->cs_idx; idx >= 0; --idx) {
if (cstack->cs_flags[idx] & CSF_TRY) {
if (did_emsg || got_int || (cstack->cs_flags[idx] & CSF_FINALLY)) {
switch (cstack->cs_pending[idx]) {
case CSTP_NONE:
break;
case CSTP_CONTINUE:
case CSTP_BREAK:
case CSTP_FINISH:
report_discard_pending(cstack->cs_pending[idx], NULL);
cstack->cs_pending[idx] = CSTP_NONE;
break;
case CSTP_RETURN:
report_discard_pending(CSTP_RETURN,
cstack->cs_rettv[idx]);
discard_pending_return(cstack->cs_rettv[idx]);
cstack->cs_pending[idx] = CSTP_NONE;
break;
default:
if (cstack->cs_flags[idx] & CSF_FINALLY) {
if (cstack->cs_pending[idx] & CSTP_THROW) {
discard_exception((except_T *)
cstack->cs_exception[idx],
FALSE);
} else
report_discard_pending(cstack->cs_pending[idx],
NULL);
cstack->cs_pending[idx] = CSTP_NONE;
}
break;
}
}
if (!(cstack->cs_flags[idx] & CSF_FINALLY)) {
if ((cstack->cs_flags[idx] & CSF_ACTIVE)
&& (cstack->cs_flags[idx] & CSF_CAUGHT))
finish_exception((except_T *)cstack->cs_exception[idx]);
if (cstack->cs_flags[idx] & CSF_TRUE) {
if (searched_cond == 0 && !inclusive)
break;
stop = TRUE;
}
}
}
if (cstack->cs_flags[idx] & searched_cond) {
if (!inclusive)
break;
stop = TRUE;
}
cstack->cs_flags[idx] &= ~CSF_ACTIVE;
if (stop && searched_cond != (CSF_TRY | CSF_SILENT))
break;
if ((cstack->cs_flags[idx] & CSF_TRY)
&& (cstack->cs_flags[idx] & CSF_SILENT)) {
eslist_T *elem;
elem = cstack->cs_emsg_silent_list;
cstack->cs_emsg_silent_list = elem->next;
emsg_silent = elem->saved_emsg_silent;
xfree(elem);
cstack->cs_flags[idx] &= ~CSF_SILENT;
}
if (stop)
break;
}
return idx;
}
static char_u *get_end_emsg(cstack_T *cstack)
{
if (cstack->cs_flags[cstack->cs_idx] & CSF_WHILE)
return e_endwhile;
if (cstack->cs_flags[cstack->cs_idx] & CSF_FOR)
return e_endfor;
return e_endif;
}
void rewind_conditionals(cstack_T *cstack, int idx, int cond_type,
int *cond_level)
{
while (cstack->cs_idx > idx) {
if (cstack->cs_flags[cstack->cs_idx] & cond_type)
--*cond_level;
if (cstack->cs_flags[cstack->cs_idx] & CSF_FOR)
free_for_info(cstack->cs_forinfo[cstack->cs_idx]);
--cstack->cs_idx;
}
}
void ex_endfunction(exarg_T *eap)
{
EMSG(_("E193: :endfunction not inside a function"));
}
int has_loop_cmd(char_u *p)
{
int len;
for (;; ) {
while (*p == ' ' || *p == '\t' || *p == ':')
++p;
len = modifier_len(p);
if (len == 0)
break;
p += len;
}
if ((p[0] == 'w' && p[1] == 'h')
|| (p[0] == 'f' && p[1] == 'o' && p[2] == 'r'))
return TRUE;
return FALSE;
}
