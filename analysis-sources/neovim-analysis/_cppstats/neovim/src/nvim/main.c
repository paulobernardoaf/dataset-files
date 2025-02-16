#define EXTERN
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <msgpack.h>
#include "nvim/ascii.h"
#include "nvim/channel.h"
#include "nvim/vim.h"
#include "nvim/main.h"
#include "nvim/aucmd.h"
#include "nvim/buffer.h"
#include "nvim/charset.h"
#include "nvim/diff.h"
#include "nvim/eval.h"
#include "nvim/ex_cmds.h"
#include "nvim/ex_cmds2.h"
#include "nvim/ex_docmd.h"
#include "nvim/fileio.h"
#include "nvim/fold.h"
#include "nvim/getchar.h"
#include "nvim/hashtab.h"
#include "nvim/highlight.h"
#include "nvim/iconv.h"
#include "nvim/if_cscope.h"
#include "nvim/lua/executor.h"
#if defined(HAVE_LOCALE_H)
#include <locale.h>
#endif
#include "nvim/mark.h"
#include "nvim/mbyte.h"
#include "nvim/memline.h"
#include "nvim/message.h"
#include "nvim/misc1.h"
#include "nvim/garray.h"
#include "nvim/log.h"
#include "nvim/memory.h"
#include "nvim/move.h"
#include "nvim/mouse.h"
#include "nvim/normal.h"
#include "nvim/ops.h"
#include "nvim/option.h"
#include "nvim/os_unix.h"
#include "nvim/os/os_defs.h"
#include "nvim/path.h"
#include "nvim/profile.h"
#include "nvim/popupmnu.h"
#include "nvim/quickfix.h"
#include "nvim/screen.h"
#include "nvim/sign.h"
#include "nvim/state.h"
#include "nvim/strings.h"
#include "nvim/syntax.h"
#include "nvim/ui.h"
#include "nvim/ui_compositor.h"
#include "nvim/version.h"
#include "nvim/window.h"
#include "nvim/shada.h"
#include "nvim/os/input.h"
#include "nvim/os/os.h"
#include "nvim/os/time.h"
#include "nvim/os/fileio.h"
#if defined(WIN32)
#include "nvim/os/os_win_console.h"
#endif
#include "nvim/event/loop.h"
#include "nvim/os/signal.h"
#include "nvim/event/process.h"
#include "nvim/msgpack_rpc/helpers.h"
#include "nvim/msgpack_rpc/server.h"
#include "nvim/msgpack_rpc/channel.h"
#include "nvim/api/ui.h"
#include "nvim/api/private/defs.h"
#include "nvim/api/private/helpers.h"
#include "nvim/api/private/handle.h"
#include "nvim/api/private/dispatch.h"
#if !defined(WIN32)
#include "nvim/os/pty_process_unix.h"
#endif
#include "nvim/api/vim.h"
#define MAX_ARG_CMDS 10
#define WIN_HOR 1 
#define WIN_VER 2 
#define WIN_TABS 3 
typedef struct {
int argc;
char **argv;
char *use_vimrc; 
int n_commands; 
char *commands[MAX_ARG_CMDS]; 
char_u cmds_tofree[MAX_ARG_CMDS]; 
int n_pre_commands; 
char *pre_commands[MAX_ARG_CMDS]; 
int edit_type; 
char_u *tagname; 
char_u *use_ef; 
bool input_isatty; 
bool output_isatty; 
bool err_isatty; 
int no_swap_file; 
int use_debug_break_level;
int window_count; 
int window_layout; 
int diff_mode; 
char *listen_addr; 
} mparm_T;
#define EDIT_NONE 0 
#define EDIT_FILE 1 
#define EDIT_STDIN 2 
#define EDIT_TAG 3 
#define EDIT_QF 4 
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "main.c.generated.h"
#endif
Loop main_loop;
static char *argv0 = NULL;
static const char *err_arg_missing = N_("Argument missing after");
static const char *err_opt_garbage = N_("Garbage after option argument");
static const char *err_opt_unknown = N_("Unknown option argument");
static const char *err_too_many_args = N_("Too many edit arguments");
static const char *err_extra_cmd =
N_("Too many \"+command\", \"-c command\" or \"--cmd command\" arguments");
void event_init(void)
{
loop_init(&main_loop, NULL);
resize_events = multiqueue_new_child(main_loop.events);
msgpack_rpc_init_method_table();
msgpack_rpc_helpers_init();
input_init();
signal_init();
channel_init();
remote_ui_init();
api_vim_init();
terminal_init();
ui_init();
}
bool event_teardown(void)
{
if (!main_loop.events) {
input_stop();
return true;
}
multiqueue_process_events(main_loop.events);
loop_poll_events(&main_loop, 0); 
input_stop();
channel_teardown();
process_teardown(&main_loop);
timer_teardown();
server_teardown();
signal_teardown();
terminal_teardown();
return loop_close(&main_loop, true);
}
void early_init(void)
{
env_init();
fs_init();
handle_init();
eval_init(); 
init_path(argv0 ? argv0 : "nvim");
init_normal_cmds(); 
highlight_init();
#if defined(HAVE_LOCALE_H)
init_locale();
#endif
if (!win_alloc_first()) {
os_exit(0);
}
init_yank(); 
alist_init(&global_alist); 
global_alist.id = 0;
init_homedir(); 
set_init_1();
log_init();
TIME_MSG("inits 1");
set_lang_var(); 
init_signs();
ui_comp_syn_init();
}
#if defined(MAKE_LIB)
int nvim_main(int argc, char **argv); 
int nvim_main(int argc, char **argv)
#elif defined(WIN32)
int wmain(int argc, wchar_t **argv_w) 
#else
int main(int argc, char **argv)
#endif
{
#if defined(WIN32) && !defined(MAKE_LIB)
char **argv = xmalloc((size_t)argc * sizeof(char *));
for (int i = 0; i < argc; i++) {
char *buf = NULL;
utf16_to_utf8(argv_w[i], -1, &buf);
assert(buf);
argv[i] = buf;
}
#endif
argv0 = argv[0];
char_u *fname = NULL; 
mparm_T params; 
char_u *cwd = NULL; 
time_init();
init_params(&params, argc, argv);
init_startuptime(&params);
event_init();
early_init();
check_and_set_isatty(&params);
command_line_scan(&params);
if (embedded_mode) {
const char *err;
if (!channel_from_stdio(true, CALLBACK_READER_INIT, &err)) {
abort();
}
}
server_init(params.listen_addr);
if (GARGCOUNT > 0) {
fname = get_fname(&params, cwd);
}
TIME_MSG("expanding arguments");
if (params.diff_mode && params.window_count == -1)
params.window_count = 0; 
RedrawingDisabled++;
setbuf(stdout, NULL);
full_screen = !silent_mode;
win_init_size();
if (params.diff_mode) {
diff_win_options(firstwin, false);
}
assert(p_ch >= 0 && Rows >= p_ch && Rows - p_ch <= INT_MAX);
cmdline_row = (int)(Rows - p_ch);
msg_row = cmdline_row;
screenalloc(); 
set_init_2(headless_mode);
TIME_MSG("inits 2");
msg_scroll = true;
no_wait_return = true;
init_highlight(true, false); 
TIME_MSG("init highlight");
debug_break_level = params.use_debug_break_level;
if (!params.input_isatty && silent_mode && exmode_active == EXMODE_NORMAL) {
input_start(STDIN_FILENO);
}
#define PROTO "term://"
do_cmdline_cmd("augroup nvim_terminal");
do_cmdline_cmd("autocmd!");
do_cmdline_cmd("autocmd BufReadCmd " PROTO "* nested "
":if !exists('b:term_title')|call termopen( "
"matchstr(expand(\"<amatch>\"), "
"'\\c\\m" PROTO "\\%(.\\{-}//\\%(\\d\\+:\\)\\?\\)\\?\\zs.*'), "
"{'cwd': expand(get(matchlist(expand(\"<amatch>\"), "
"'\\c\\m" PROTO "\\(.\\{-}\\)//'), 1, ''))})"
"|endif");
do_cmdline_cmd("augroup END");
#undef PROTO
if (params.use_vimrc != NULL && strequal(params.use_vimrc, "NONE")) {
p_lpl = false;
}
bool use_remote_ui = (embedded_mode && !headless_mode);
bool use_builtin_ui = (!headless_mode && !embedded_mode && !silent_mode);
if (use_remote_ui || use_builtin_ui) {
TIME_MSG("waiting for UI");
if (use_remote_ui) {
remote_ui_wait_for_attach();
} else {
ui_builtin_start();
}
TIME_MSG("done waiting for UI");
starting = NO_BUFFERS;
screenclear();
TIME_MSG("initialized screen early for UI");
}
exe_pre_commands(&params);
source_startup_scripts(&params);
if (params.use_vimrc == NULL || !strequal(params.use_vimrc, "NONE")) {
filetype_maybe_enable();
syn_maybe_on();
}
load_plugins();
set_window_layout(&params);
if (recoverymode && fname == NULL) {
recover_names(NULL, true, 0, NULL);
os_exit(0);
}
set_init_3();
TIME_MSG("inits 3");
if (params.no_swap_file) {
p_uc = 0;
}
if (silent_mode) {
p_ut = 1;
}
if (*p_shada != NUL) {
shada_read_everything(NULL, false, true);
TIME_MSG("reading ShaDa");
}
if (get_vim_var_list(VV_OLDFILES) == NULL) {
set_vim_var_list(VV_OLDFILES, tv_list_alloc(0));
}
handle_quickfix(&params);
starting = NO_BUFFERS;
no_wait_return = false;
if (!exmode_active) {
msg_scroll = false;
}
if (params.edit_type == EDIT_STDIN && !recoverymode) {
read_stdin();
}
setmouse(); 
if (exmode_active || use_remote_ui || use_builtin_ui) {
redraw_later(VALID);
} else {
screenclear(); 
TIME_MSG("clearing screen");
}
no_wait_return = true;
create_windows(&params);
TIME_MSG("opening buffers");
set_vim_var_string(VV_SWAPCOMMAND, NULL, -1);
if (exmode_active) {
curwin->w_cursor.lnum = curbuf->b_ml.ml_line_count;
}
apply_autocmds(EVENT_BUFENTER, NULL, NULL, FALSE, curbuf);
TIME_MSG("BufEnter autocommands");
setpcmark();
if (params.edit_type == EDIT_QF) {
qf_jump(NULL, 0, 0, FALSE);
TIME_MSG("jump to first error");
}
edit_buffers(&params, cwd);
xfree(cwd);
if (params.diff_mode) {
FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
diff_win_options(wp, TRUE);
}
}
shorten_fnames(false);
handle_tag(params.tagname);
if (params.n_commands > 0) {
exe_commands(&params);
}
starting = 0;
RedrawingDisabled = 0;
redraw_all_later(NOT_VALID);
no_wait_return = false;
do_autochdir();
if (p_im) {
need_start_insertmode = true;
}
set_vim_var_nr(VV_VIM_DID_ENTER, 1L);
apply_autocmds(EVENT_VIMENTER, NULL, NULL, false, curbuf);
TIME_MSG("VimEnter autocommands");
if (use_remote_ui || use_builtin_ui) {
do_autocmd_uienter(use_remote_ui ? CHAN_STDIO : 0, true);
TIME_MSG("UIEnter autocommands");
}
set_reg_var(get_default_register_name());
if (curwin->w_p_diff && curwin->w_p_scb) {
update_topline();
check_scrollbind((linenr_T)0, 0L);
TIME_MSG("diff scrollbinding");
}
if (restart_edit != 0) {
stuffcharReadbuff(K_NOP);
}
if (cb_flags & CB_UNNAMEDMASK) {
(void)eval_has_provider("clipboard");
}
TIME_MSG("before starting main loop");
ILOG("starting main loop");
normal_enter(false, false);
#if defined(WIN32) && !defined(MAKE_LIB)
xfree(argv);
#endif
return 0;
}
void os_exit(int r)
FUNC_ATTR_NORETURN
{
exiting = true;
ui_flush();
ui_call_stop();
ml_close_all(true); 
if (!event_teardown() && r == 0) {
r = 1; 
}
if (input_global_fd() >= 0) {
stream_set_blocking(input_global_fd(), true); 
}
ILOG("Nvim exit: %d", r);
#if defined(EXITFREE)
free_all_mem();
#endif
exit(r);
}
void getout(int exitval)
FUNC_ATTR_NORETURN
{
exiting = true;
if (exmode_active)
exitval += ex_exitval;
set_vim_var_nr(VV_EXITING, exitval);
ui_cursor_goto(Rows - 1, 0);
hash_debug_results();
if (get_vim_var_nr(VV_DYING) <= 1) {
const tabpage_T *next_tp;
for (const tabpage_T *tp = first_tabpage; tp != NULL; tp = next_tp) {
next_tp = tp->tp_next;
FOR_ALL_WINDOWS_IN_TAB(wp, tp) {
if (wp->w_buffer == NULL) {
continue;
}
buf_T *buf = wp->w_buffer;
if (buf_get_changedtick(buf) != -1) {
bufref_T bufref;
set_bufref(&bufref, buf);
apply_autocmds(EVENT_BUFWINLEAVE, buf->b_fname,
buf->b_fname, false, buf);
if (bufref_valid(&bufref)) {
buf_set_changedtick(buf, -1); 
}
next_tp = first_tabpage;
break;
}
}
}
FOR_ALL_BUFFERS(buf) {
if (buf->b_ml.ml_mfp != NULL) {
bufref_T bufref;
set_bufref(&bufref, buf);
apply_autocmds(EVENT_BUFUNLOAD, buf->b_fname, buf->b_fname, false, buf);
if (!bufref_valid(&bufref)) {
break;
}
}
}
apply_autocmds(EVENT_VIMLEAVEPRE, NULL, NULL, FALSE, curbuf);
}
if (p_shada && *p_shada != NUL) {
shada_write_file(NULL, false);
}
if (get_vim_var_nr(VV_DYING) <= 1)
apply_autocmds(EVENT_VIMLEAVE, NULL, NULL, FALSE, curbuf);
profile_dump();
if (did_emsg
) {
no_wait_return = FALSE;
wait_return(FALSE);
}
ui_cursor_goto(Rows - 1, 0);
if (p_title && *p_titleold != NUL) {
ui_call_set_title(cstr_as_string((char *)p_titleold));
}
cs_end();
if (garbage_collect_at_exit) {
garbage_collect(false);
}
os_exit(exitval);
}
static int get_number_arg(const char *p, int *idx, int def)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_WARN_UNUSED_RESULT
{
if (ascii_isdigit(p[*idx])) { 
def = atoi(&(p[*idx]));
while (ascii_isdigit(p[*idx])) {
*idx = *idx + 1;
}
}
return def;
}
#if defined(HAVE_LOCALE_H)
static void init_locale(void)
{
setlocale(LC_ALL, "");
#if defined(LC_NUMERIC)
setlocale(LC_NUMERIC, "C");
#endif
char localepath[MAXPATHL] = { 0 };
snprintf(localepath, sizeof(localepath), "%s", get_vim_var_str(VV_PROGPATH));
char *tail = (char *)path_tail_with_sep((char_u *)localepath);
*tail = NUL;
tail = (char *)path_tail((char_u *)localepath);
xstrlcpy(tail, "share/locale",
sizeof(localepath) - (size_t)(tail - localepath));
bindtextdomain(PROJECT_NAME, localepath);
textdomain(PROJECT_NAME);
TIME_MSG("locale set");
}
#endif
static bool edit_stdin(bool explicit, mparm_T *parmp)
{
bool implicit = !headless_mode
&& !embedded_mode
&& exmode_active != EXMODE_NORMAL 
&& !parmp->input_isatty
&& scriptin[0] == NULL; 
return explicit || implicit;
}
static void command_line_scan(mparm_T *parmp)
{
int argc = parmp->argc;
char **argv = parmp->argv;
int argv_idx; 
bool had_stdin_file = false; 
bool had_minmin = false; 
int want_argument; 
int c;
long n;
argc--;
argv++;
argv_idx = 1; 
while (argc > 0) {
if (argv[0][0] == '+' && !had_minmin) {
if (parmp->n_commands >= MAX_ARG_CMDS) {
mainerr(err_extra_cmd, NULL);
}
argv_idx = -1; 
if (argv[0][1] == NUL) {
parmp->commands[parmp->n_commands++] = "$";
} else {
parmp->commands[parmp->n_commands++] = &(argv[0][1]);
}
} else if (argv[0][0] == '-' && !had_minmin) {
want_argument = false;
c = argv[0][argv_idx++];
switch (c) {
case NUL: { 
if (exmode_active) {
silent_mode = true;
parmp->no_swap_file = true;
} else {
if (parmp->edit_type != EDIT_NONE
&& parmp->edit_type != EDIT_FILE
&& parmp->edit_type != EDIT_STDIN) {
mainerr(err_too_many_args, argv[0]);
}
had_stdin_file = true;
parmp->edit_type = EDIT_STDIN;
}
argv_idx = -1; 
break;
}
case '-': { 
if (STRICMP(argv[0] + argv_idx, "help") == 0) {
usage();
os_exit(0);
} else if (STRICMP(argv[0] + argv_idx, "version") == 0) {
version();
os_exit(0);
} else if (STRICMP(argv[0] + argv_idx, "api-info") == 0) {
FileDescriptor fp;
const int fof_ret = file_open_fd(&fp, STDOUT_FILENO,
kFileWriteOnly);
msgpack_packer *p = msgpack_packer_new(&fp, msgpack_file_write);
if (fof_ret != 0) {
emsgf(_("E5421: Failed to open stdin: %s"), os_strerror(fof_ret));
}
if (p == NULL) {
EMSG(_(e_outofmem));
}
Object md = DICTIONARY_OBJ(api_metadata());
msgpack_rpc_from_object(md, p);
msgpack_packer_free(p);
const int ff_ret = file_flush(&fp);
if (ff_ret < 0) {
msgpack_file_write_error(ff_ret);
}
os_exit(0);
} else if (STRICMP(argv[0] + argv_idx, "headless") == 0) {
headless_mode = true;
} else if (STRICMP(argv[0] + argv_idx, "embed") == 0) {
embedded_mode = true;
} else if (STRNICMP(argv[0] + argv_idx, "listen", 6) == 0) {
want_argument = true;
argv_idx += 6;
} else if (STRNICMP(argv[0] + argv_idx, "literal", 7) == 0) {
} else if (STRNICMP(argv[0] + argv_idx, "noplugin", 8) == 0) {
p_lpl = false;
} else if (STRNICMP(argv[0] + argv_idx, "cmd", 3) == 0) {
want_argument = true;
argv_idx += 3;
} else if (STRNICMP(argv[0] + argv_idx, "startuptime", 11) == 0) {
want_argument = true;
argv_idx += 11;
} else if (STRNICMP(argv[0] + argv_idx, "clean", 5) == 0) {
parmp->use_vimrc = "NONE";
set_option_value("shadafile", 0L, "NONE", 0);
} else {
if (argv[0][argv_idx])
mainerr(err_opt_unknown, argv[0]);
had_minmin = true;
}
if (!want_argument) {
argv_idx = -1; 
}
break;
}
case 'A': { 
set_option_value("arabic", 1L, NULL, 0);
break;
}
case 'b': { 
set_options_bin(curbuf->b_p_bin, 1, 0);
curbuf->b_p_bin = 1; 
break;
}
case 'D': { 
parmp->use_debug_break_level = 9999;
break;
}
case 'd': { 
parmp->diff_mode = true;
break;
}
case 'e': { 
exmode_active = EXMODE_NORMAL;
break;
}
case 'E': { 
exmode_active = EXMODE_VIM;
break;
}
case 'f': { 
break;
}
case '?': 
case 'h': { 
usage();
os_exit(0);
}
case 'H': { 
p_hkmap = true;
set_option_value("rl", 1L, NULL, 0);
break;
}
case 'l': { 
set_option_value("lisp", 1L, NULL, 0);
p_sm = true;
break;
}
case 'M': { 
reset_modifiable();
FALLTHROUGH;
}
case 'm': { 
p_write = false;
break;
}
case 'N': 
case 'X': 
break;
case 'n': { 
parmp->no_swap_file = true;
break;
}
case 'p': { 
parmp->window_count = get_number_arg(argv[0], &argv_idx, 0);
parmp->window_layout = WIN_TABS;
break;
}
case 'o': { 
parmp->window_count = get_number_arg(argv[0], &argv_idx, 0);
parmp->window_layout = WIN_HOR;
break;
}
case 'O': { 
parmp->window_count = get_number_arg(argv[0], &argv_idx, 0);
parmp->window_layout = WIN_VER;
break;
}
case 'q': { 
if (parmp->edit_type != EDIT_NONE) {
mainerr(err_too_many_args, argv[0]);
}
parmp->edit_type = EDIT_QF;
if (argv[0][argv_idx]) { 
parmp->use_ef = (char_u *)argv[0] + argv_idx;
argv_idx = -1;
} else if (argc > 1) { 
want_argument = true;
}
break;
}
case 'R': { 
readonlymode = true;
curbuf->b_p_ro = true;
p_uc = 10000; 
break;
}
case 'r': 
case 'L': { 
recoverymode = 1;
headless_mode = true;
break;
}
case 's': {
if (exmode_active) { 
silent_mode = true;
parmp->no_swap_file = true;
} else { 
want_argument = true;
}
break;
}
case 't': { 
if (parmp->edit_type != EDIT_NONE) {
mainerr(err_too_many_args, argv[0]);
}
parmp->edit_type = EDIT_TAG;
if (argv[0][argv_idx]) { 
parmp->tagname = (char_u *)argv[0] + argv_idx;
argv_idx = -1;
} else { 
want_argument = true;
}
break;
}
case 'v': {
version();
os_exit(0);
}
case 'V': { 
p_verbose = get_number_arg(argv[0], &argv_idx, 10);
if (argv[0][argv_idx] != NUL) {
set_option_value("verbosefile", 0L, argv[0] + argv_idx, 0);
argv_idx = (int)STRLEN(argv[0]);
}
break;
}
case 'w': { 
if (ascii_isdigit(((char_u *)argv[0])[argv_idx])) {
n = get_number_arg(argv[0], &argv_idx, 10);
set_option_value("window", n, NULL, 0);
break;
}
want_argument = true;
break;
}
case 'Z': { 
restricted = true;
break;
}
case 'c': { 
if (argv[0][argv_idx] != NUL) {
if (parmp->n_commands >= MAX_ARG_CMDS) {
mainerr(err_extra_cmd, NULL);
}
parmp->commands[parmp->n_commands++] = argv[0] + argv_idx;
argv_idx = -1;
break;
}
FALLTHROUGH;
}
case 'S': 
case 'i': 
case 'u': 
case 'U': 
case 'W': { 
want_argument = true;
break;
}
default: {
mainerr(err_opt_unknown, argv[0]);
}
}
if (want_argument) {
if (argv[0][argv_idx] != NUL) {
mainerr(err_opt_garbage, argv[0]);
}
argc--;
if (argc < 1 && c != 'S') { 
mainerr(err_arg_missing, argv[0]);
}
argv++;
argv_idx = -1;
switch (c) {
case 'c': 
case 'S': { 
if (parmp->n_commands >= MAX_ARG_CMDS) {
mainerr(err_extra_cmd, NULL);
}
if (c == 'S') {
char *a;
if (argc < 1) {
a = SESSION_FILE;
} else if (argv[0][0] == '-') {
a = SESSION_FILE;
++argc;
--argv;
} else {
a = argv[0];
}
size_t s_size = STRLEN(a) + 4;
char *s = xmalloc(s_size);
snprintf(s, s_size, "so %s", a);
parmp->cmds_tofree[parmp->n_commands] = true;
parmp->commands[parmp->n_commands++] = s;
} else {
parmp->commands[parmp->n_commands++] = argv[0];
}
break;
}
case '-': {
if (strequal(argv[-1], "--cmd")) {
if (parmp->n_pre_commands >= MAX_ARG_CMDS) {
mainerr(err_extra_cmd, NULL);
}
parmp->pre_commands[parmp->n_pre_commands++] = argv[0];
} else if (strequal(argv[-1], "--listen")) {
parmp->listen_addr = argv[0];
}
break;
}
case 'q': { 
parmp->use_ef = (char_u *)argv[0];
break;
}
case 'i': { 
set_option_value("shadafile", 0L, argv[0], 0);
break;
}
case 's': { 
if (scriptin[0] != NULL) {
scripterror:
vim_snprintf((char *)IObuff, IOSIZE,
_("Attempt to open script file again: \"%s %s\"\n"),
argv[-1], argv[0]);
mch_errmsg((const char *)IObuff);
os_exit(2);
}
int error;
if (strequal(argv[0], "-")) {
const int stdin_dup_fd = os_dup(STDIN_FILENO);
#if defined(WIN32)
os_replace_stdin_to_conin();
#endif
FileDescriptor *const stdin_dup = file_open_fd_new(
&error, stdin_dup_fd, kFileReadOnly|kFileNonBlocking);
assert(stdin_dup != NULL);
scriptin[0] = stdin_dup;
} else if ((scriptin[0] = file_open_new(
&error, argv[0], kFileReadOnly|kFileNonBlocking, 0)) == NULL) {
vim_snprintf((char *)IObuff, IOSIZE,
_("Cannot open for reading: \"%s\": %s\n"),
argv[0], os_strerror(error));
mch_errmsg((const char *)IObuff);
os_exit(2);
}
save_typebuf();
break;
}
case 't': { 
parmp->tagname = (char_u *)argv[0];
break;
}
case 'u': { 
parmp->use_vimrc = argv[0];
break;
}
case 'U': { 
break;
}
case 'w': { 
if (ascii_isdigit(*((char_u *)argv[0]))) {
argv_idx = 0;
n = get_number_arg(argv[0], &argv_idx, 10);
set_option_value("window", n, NULL, 0);
argv_idx = -1;
break;
}
FALLTHROUGH;
}
case 'W': { 
if (scriptout != NULL) {
goto scripterror;
}
if ((scriptout = os_fopen(argv[0], c == 'w' ? APPENDBIN : WRITEBIN))
== NULL) {
mch_errmsg(_("Cannot open for script output: \""));
mch_errmsg(argv[0]);
mch_errmsg("\"\n");
os_exit(2);
}
break;
}
}
}
} else { 
argv_idx = -1; 
if (parmp->edit_type != EDIT_NONE
&& parmp->edit_type != EDIT_FILE
&& parmp->edit_type != EDIT_STDIN) {
mainerr(err_too_many_args, argv[0]);
}
parmp->edit_type = EDIT_FILE;
ga_grow(&global_alist.al_ga, 1);
char_u *p = vim_strsave((char_u *)argv[0]);
if (parmp->diff_mode && os_isdir(p) && GARGCOUNT > 0
&& !os_isdir(alist_name(&GARGLIST[0]))) {
char_u *r = (char_u *)concat_fnames((char *)p,
(char *)path_tail(alist_name(&GARGLIST[0])), true);
xfree(p);
p = r;
}
#if defined(USE_FNAME_CASE)
path_fix_case(p);
#endif
int alist_fnum_flag = edit_stdin(had_stdin_file, parmp)
? 1 
: 2; 
alist_add(&global_alist, p, alist_fnum_flag);
}
if (argv_idx <= 0 || argv[0][argv_idx] == NUL) {
argc--;
argv++;
argv_idx = 1;
}
}
if (embedded_mode && silent_mode) {
mainerr(_("--embed conflicts with -es/-Es"), NULL);
}
if (parmp->n_commands > 0) {
const size_t swcmd_len = STRLEN(parmp->commands[0]) + 3;
char *const swcmd = xmalloc(swcmd_len);
snprintf(swcmd, swcmd_len, ":%s\r", parmp->commands[0]);
set_vim_var_string(VV_SWAPCOMMAND, swcmd, -1);
xfree(swcmd);
}
if (edit_stdin(had_stdin_file, parmp)) {
parmp->edit_type = EDIT_STDIN;
}
TIME_MSG("parsing arguments");
}
static void init_params(mparm_T *paramp, int argc, char **argv)
{
memset(paramp, 0, sizeof(*paramp));
paramp->argc = argc;
paramp->argv = argv;
paramp->use_debug_break_level = -1;
paramp->window_count = -1;
paramp->listen_addr = NULL;
}
static void init_startuptime(mparm_T *paramp)
{
for (int i = 1; i < paramp->argc; i++) {
if (STRICMP(paramp->argv[i], "--startuptime") == 0
&& i + 1 < paramp->argc) {
time_fd = os_fopen(paramp->argv[i + 1], "a");
time_start("--- NVIM STARTING ---");
break;
}
}
starttime = time(NULL);
}
static void check_and_set_isatty(mparm_T *paramp)
{
stdin_isatty
= paramp->input_isatty = os_isatty(STDIN_FILENO);
stdout_isatty
= paramp->output_isatty = os_isatty(STDOUT_FILENO);
paramp->err_isatty = os_isatty(STDERR_FILENO);
#if !defined(WIN32)
int tty_fd = paramp->input_isatty
? STDIN_FILENO
: (paramp->output_isatty
? STDOUT_FILENO
: (paramp->err_isatty ? STDERR_FILENO : -1));
pty_process_save_termios(tty_fd);
#endif
TIME_MSG("window checked");
}
static void init_path(const char *exename)
FUNC_ATTR_NONNULL_ALL
{
char exepath[MAXPATHL] = { 0 };
size_t exepathlen = MAXPATHL;
if (os_exepath(exepath, &exepathlen) != 0) {
path_guess_exepath(exename, exepath, sizeof(exepath));
}
set_vim_var_string(VV_PROGPATH, exepath, -1);
set_vim_var_string(VV_PROGNAME, (char *)path_tail((char_u *)exename), -1);
#if defined(WIN32)
os_setenv_append_path(exepath);
#endif
}
static char_u *get_fname(mparm_T *parmp, char_u *cwd)
{
return alist_name(&GARGLIST[0]);
}
static void set_window_layout(mparm_T *paramp)
{
if (paramp->diff_mode && paramp->window_layout == 0) {
if (diffopt_horizontal())
paramp->window_layout = WIN_HOR; 
else
paramp->window_layout = WIN_VER; 
}
}
static void load_plugins(void)
{
if (p_lpl) {
char_u *rtp_copy = NULL;
if (!did_source_packages) {
rtp_copy = vim_strsave(p_rtp);
add_pack_start_dirs();
}
source_in_path(rtp_copy == NULL ? p_rtp : rtp_copy,
(char_u *)"plugin/**/*.vim", 
DIP_ALL | DIP_NOAFTER);
TIME_MSG("loading plugins");
xfree(rtp_copy);
if (!did_source_packages) {
load_start_packages();
}
TIME_MSG("loading packages");
source_runtime((char_u *)"plugin/**/*.vim", DIP_ALL | DIP_AFTER);
TIME_MSG("loading after plugins");
}
}
static void handle_quickfix(mparm_T *paramp)
{
if (paramp->edit_type == EDIT_QF) {
if (paramp->use_ef != NULL)
set_string_option_direct((char_u *)"ef", -1,
paramp->use_ef, OPT_FREE, SID_CARG);
vim_snprintf((char *)IObuff, IOSIZE, "cfile %s", p_ef);
if (qf_init(NULL, p_ef, p_efm, true, IObuff, p_menc) < 0) {
msg_putchar('\n');
os_exit(3);
}
TIME_MSG("reading errorfile");
}
}
static void handle_tag(char_u *tagname)
{
if (tagname != NULL) {
swap_exists_did_quit = FALSE;
vim_snprintf((char *)IObuff, IOSIZE, "ta %s", tagname);
do_cmdline_cmd((char *)IObuff);
TIME_MSG("jumping to tag");
if (swap_exists_did_quit)
getout(1);
}
}
static void read_stdin(void)
{
swap_exists_action = SEA_DIALOG;
no_wait_return = true;
int save_msg_didany = msg_didany;
set_buflisted(true);
(void)open_buffer(true, NULL, 0); 
if (BUFEMPTY() && curbuf->b_next != NULL) {
do_cmdline_cmd("silent! bnext");
do_cmdline_cmd("bwipeout 1");
}
no_wait_return = false;
msg_didany = save_msg_didany;
TIME_MSG("reading stdin");
check_swap_exists_action();
}
static void create_windows(mparm_T *parmp)
{
int dorewind;
int done = 0;
if (parmp->window_count == -1) 
parmp->window_count = 1;
if (parmp->window_count == 0)
parmp->window_count = GARGCOUNT;
if (parmp->window_count > 1) {
if (parmp->window_layout == 0)
parmp->window_layout = WIN_HOR;
if (parmp->window_layout == WIN_TABS) {
parmp->window_count = make_tabpages(parmp->window_count);
TIME_MSG("making tab pages");
} else if (firstwin->w_next == NULL) {
parmp->window_count = make_windows(parmp->window_count,
parmp->window_layout == WIN_VER);
TIME_MSG("making windows");
} else
parmp->window_count = win_count();
} else
parmp->window_count = 1;
if (recoverymode) { 
msg_scroll = true; 
ml_recover(true);
if (curbuf->b_ml.ml_mfp == NULL) { 
getout(1);
}
do_modelines(0); 
} else {
++autocmd_no_enter;
++autocmd_no_leave;
dorewind = TRUE;
while (done++ < 1000) {
if (dorewind) {
if (parmp->window_layout == WIN_TABS)
goto_tabpage(1);
else
curwin = firstwin;
} else if (parmp->window_layout == WIN_TABS) {
if (curtab->tp_next == NULL)
break;
goto_tabpage(0);
} else {
if (curwin->w_next == NULL)
break;
curwin = curwin->w_next;
}
dorewind = FALSE;
curbuf = curwin->w_buffer;
if (curbuf->b_ml.ml_mfp == NULL) {
if (p_fdls >= 0) {
curwin->w_p_fdl = p_fdls;
}
swap_exists_action = SEA_DIALOG;
set_buflisted(TRUE);
(void)open_buffer(FALSE, NULL, 0);
if (swap_exists_action == SEA_QUIT) {
if (got_int || only_one_window()) {
did_emsg = FALSE; 
getout(1);
}
setfname(curbuf, NULL, NULL, false);
curwin->w_arg_idx = -1;
swap_exists_action = SEA_NONE;
} else
handle_swap_exists(NULL);
dorewind = TRUE; 
}
os_breakcheck();
if (got_int) {
(void)vgetc(); 
break;
}
}
if (parmp->window_layout == WIN_TABS)
goto_tabpage(1);
else
curwin = firstwin;
curbuf = curwin->w_buffer;
--autocmd_no_enter;
--autocmd_no_leave;
}
}
static void edit_buffers(mparm_T *parmp, char_u *cwd)
{
int arg_idx; 
int i;
bool advance = true;
win_T *win;
char *p_shm_save = NULL;
++autocmd_no_enter;
++autocmd_no_leave;
if (curwin->w_arg_idx == -1) {
win_close(curwin, true);
advance = false;
}
arg_idx = 1;
for (i = 1; i < parmp->window_count; ++i) {
if (cwd != NULL) {
os_chdir((char *)cwd);
}
if (curwin->w_arg_idx == -1) {
arg_idx++;
win_close(curwin, true);
advance = false;
continue;
}
if (advance) {
if (parmp->window_layout == WIN_TABS) {
if (curtab->tp_next == NULL) 
break;
goto_tabpage(0);
if (i == 1) {
char buf[100];
p_shm_save = xstrdup((char *)p_shm);
snprintf(buf, sizeof(buf), "F%s", p_shm);
set_option_value("shm", 0L, buf, 0);
}
} else {
if (curwin->w_next == NULL) 
break;
win_enter(curwin->w_next, false);
}
}
advance = true;
if (curbuf == firstwin->w_buffer || curbuf->b_ffname == NULL) {
curwin->w_arg_idx = arg_idx;
swap_exists_did_quit = FALSE;
(void)do_ecmd(0, arg_idx < GARGCOUNT
? alist_name(&GARGLIST[arg_idx]) : NULL,
NULL, NULL, ECMD_LASTL, ECMD_HIDE, curwin);
if (swap_exists_did_quit) {
if (got_int || only_one_window()) {
did_emsg = FALSE; 
getout(1);
}
win_close(curwin, true);
advance = false;
}
if (arg_idx == GARGCOUNT - 1) {
arg_had_last = true;
}
arg_idx++;
}
os_breakcheck();
if (got_int) {
(void)vgetc(); 
break;
}
}
if (p_shm_save != NULL) {
set_option_value("shm", 0L, p_shm_save, 0);
xfree(p_shm_save);
}
if (parmp->window_layout == WIN_TABS)
goto_tabpage(1);
--autocmd_no_enter;
win = firstwin;
while (win->w_p_pvw) {
win = win->w_next;
if (win == NULL) {
win = firstwin;
break;
}
}
win_enter(win, false);
--autocmd_no_leave;
TIME_MSG("editing files in windows");
if (parmp->window_count > 1 && parmp->window_layout != WIN_TABS)
win_equal(curwin, false, 'b'); 
}
static void exe_pre_commands(mparm_T *parmp)
{
char **cmds = parmp->pre_commands;
int cnt = parmp->n_pre_commands;
int i;
if (cnt > 0) {
curwin->w_cursor.lnum = 0; 
sourcing_name = (char_u *)_("pre-vimrc command line");
current_sctx.sc_sid = SID_CMDARG;
for (i = 0; i < cnt; i++) {
do_cmdline_cmd(cmds[i]);
}
sourcing_name = NULL;
current_sctx.sc_sid = 0;
TIME_MSG("--cmd commands");
}
}
static void exe_commands(mparm_T *parmp)
{
int i;
msg_scroll = TRUE;
if (parmp->tagname == NULL && curwin->w_cursor.lnum <= 1)
curwin->w_cursor.lnum = 0;
sourcing_name = (char_u *)"command line";
current_sctx.sc_sid = SID_CARG;
current_sctx.sc_seq = 0;
for (i = 0; i < parmp->n_commands; i++) {
do_cmdline_cmd(parmp->commands[i]);
if (parmp->cmds_tofree[i])
xfree(parmp->commands[i]);
}
sourcing_name = NULL;
current_sctx.sc_sid = 0;
if (curwin->w_cursor.lnum == 0) {
curwin->w_cursor.lnum = 1;
}
if (!exmode_active)
msg_scroll = FALSE;
if (parmp->edit_type == EDIT_QF)
qf_jump(NULL, 0, 0, FALSE);
TIME_MSG("executing command arguments");
}
static void do_system_initialization(void)
{
char *const config_dirs = stdpaths_get_xdg_var(kXDGConfigDirs);
if (config_dirs != NULL) {
const void *iter = NULL;
const char path_tail[] = {
'n', 'v', 'i', 'm', PATHSEP,
's', 'y', 's', 'i', 'n', 'i', 't', '.', 'v', 'i', 'm', NUL
};
do {
const char *dir;
size_t dir_len;
iter = vim_env_iter(':', config_dirs, iter, &dir, &dir_len);
if (dir == NULL || dir_len == 0) {
break;
}
char *vimrc = xmalloc(dir_len + sizeof(path_tail) + 1);
memcpy(vimrc, dir, dir_len);
vimrc[dir_len] = PATHSEP;
memcpy(vimrc + dir_len + 1, path_tail, sizeof(path_tail));
if (do_source((char_u *)vimrc, false, DOSO_NONE) != FAIL) {
xfree(vimrc);
xfree(config_dirs);
return;
}
xfree(vimrc);
} while (iter != NULL);
xfree(config_dirs);
}
#if defined(SYS_VIMRC_FILE)
(void)do_source((char_u *)SYS_VIMRC_FILE, false, DOSO_NONE);
#endif
}
static bool do_user_initialization(void)
FUNC_ATTR_WARN_UNUSED_RESULT
{
bool do_exrc = p_exrc;
if (execute_env("VIMINIT") == OK) {
do_exrc = p_exrc;
return do_exrc;
}
char_u *user_vimrc = (char_u *)stdpaths_user_conf_subpath("init.vim");
if (do_source(user_vimrc, true, DOSO_VIMRC) != FAIL) {
do_exrc = p_exrc;
if (do_exrc) {
do_exrc = (path_full_compare((char_u *)VIMRC_FILE, user_vimrc,
false, true)
!= kEqualFiles);
}
xfree(user_vimrc);
return do_exrc;
}
xfree(user_vimrc);
char *const config_dirs = stdpaths_get_xdg_var(kXDGConfigDirs);
if (config_dirs != NULL) {
const void *iter = NULL;
do {
const char *dir;
size_t dir_len;
iter = vim_env_iter(':', config_dirs, iter, &dir, &dir_len);
if (dir == NULL || dir_len == 0) {
break;
}
const char path_tail[] = { 'n', 'v', 'i', 'm', PATHSEP,
'i', 'n', 'i', 't', '.', 'v', 'i', 'm', NUL };
char *vimrc = xmalloc(dir_len + sizeof(path_tail) + 1);
memmove(vimrc, dir, dir_len);
vimrc[dir_len] = PATHSEP;
memmove(vimrc + dir_len + 1, path_tail, sizeof(path_tail));
if (do_source((char_u *) vimrc, true, DOSO_VIMRC) != FAIL) {
do_exrc = p_exrc;
if (do_exrc) {
do_exrc = (path_full_compare((char_u *)VIMRC_FILE, (char_u *)vimrc,
false, true) != kEqualFiles);
}
xfree(vimrc);
xfree(config_dirs);
return do_exrc;
}
xfree(vimrc);
} while (iter != NULL);
xfree(config_dirs);
}
if (execute_env("EXINIT") == OK) {
do_exrc = p_exrc;
return do_exrc;
}
return do_exrc;
}
static void source_startup_scripts(const mparm_T *const parmp)
FUNC_ATTR_NONNULL_ALL
{
if (parmp->use_vimrc != NULL) {
if (strequal(parmp->use_vimrc, "NONE")
|| strequal(parmp->use_vimrc, "NORC")) {
} else {
if (do_source((char_u *)parmp->use_vimrc, false, DOSO_NONE) != OK) {
EMSG2(_("E282: Cannot read from \"%s\""), parmp->use_vimrc);
}
}
} else if (!silent_mode) {
do_system_initialization();
if (do_user_initialization()) {
#if defined(UNIX)
if (!file_owned(VIMRC_FILE))
#endif
secure = p_secure;
if (do_source((char_u *)VIMRC_FILE, true, DOSO_VIMRC) == FAIL) {
#if defined(UNIX)
if (!file_owned(EXRC_FILE)) {
secure = p_secure;
} else {
secure = 0;
}
#endif
(void)do_source((char_u *)EXRC_FILE, false, DOSO_NONE);
}
}
if (secure == 2) {
need_wait_return = true;
}
secure = 0;
}
TIME_MSG("sourcing vimrc file(s)");
}
static int execute_env(char *env)
FUNC_ATTR_NONNULL_ALL
{
const char *initstr = os_getenv(env);
if (initstr != NULL) {
char_u *save_sourcing_name = sourcing_name;
linenr_T save_sourcing_lnum = sourcing_lnum;
sourcing_name = (char_u *)env;
sourcing_lnum = 0;
const sctx_T save_current_sctx = current_sctx;
current_sctx.sc_sid = SID_ENV;
current_sctx.sc_seq = 0;
current_sctx.sc_lnum = 0;
do_cmdline_cmd((char *)initstr);
sourcing_name = save_sourcing_name;
sourcing_lnum = save_sourcing_lnum;
current_sctx = save_current_sctx;
return OK;
}
return FAIL;
}
#if defined(UNIX)
static bool file_owned(const char *fname)
{
assert(fname != NULL);
uid_t uid = getuid();
FileInfo file_info;
bool file_owned = os_fileinfo(fname, &file_info)
&& file_info.stat.st_uid == uid;
bool link_owned = os_fileinfo_link(fname, &file_info)
&& file_info.stat.st_uid == uid;
return file_owned && link_owned;
}
#endif
static void mainerr(const char *errstr, const char *str)
{
char *prgname = (char *)path_tail((char_u *)argv0);
signal_stop(); 
mch_errmsg(prgname);
mch_errmsg(": ");
mch_errmsg(_(errstr));
if (str != NULL) {
mch_errmsg(": \"");
mch_errmsg(str);
mch_errmsg("\"");
}
mch_errmsg(_("\nMore info with \""));
mch_errmsg(prgname);
mch_errmsg(" -h\"\n");
os_exit(1);
}
static void version(void)
{
info_message = TRUE; 
list_version();
msg_putchar('\n');
msg_didout = FALSE;
}
static void usage(void)
{
signal_stop(); 
mch_msg(_("Usage:\n"));
mch_msg(_(" nvim [options] [file ...] Edit file(s)\n"));
mch_msg(_(" nvim [options] -t <tag> Edit file where tag is defined\n"));
mch_msg(_(" nvim [options] -q [errorfile] Edit file with first error\n"));
mch_msg(_("\nOptions:\n"));
mch_msg(_(" -- Only file names after this\n"));
mch_msg(_(" + Start at end of file\n"));
mch_msg(_(" --cmd <cmd> Execute <cmd> before any config\n"));
mch_msg(_(" +<cmd>, -c <cmd> Execute <cmd> after config and first file\n"));
mch_msg("\n");
mch_msg(_(" -b Binary mode\n"));
mch_msg(_(" -d Diff mode\n"));
mch_msg(_(" -e, -E Ex mode\n"));
mch_msg(_(" -es, -Es Silent (batch) mode\n"));
mch_msg(_(" -h, --help Print this help message\n"));
mch_msg(_(" -i <shada> Use this shada file\n"));
mch_msg(_(" -m Modifications (writing files) not allowed\n"));
mch_msg(_(" -M Modifications in text not allowed\n"));
mch_msg(_(" -n No swap file, use memory only\n"));
mch_msg(_(" -o[N] Open N windows (default: one per file)\n"));
mch_msg(_(" -O[N] Open N vertical windows (default: one per file)\n"));
mch_msg(_(" -p[N] Open N tab pages (default: one per file)\n"));
mch_msg(_(" -r, -L List swap files\n"));
mch_msg(_(" -r <file> Recover edit state for this file\n"));
mch_msg(_(" -R Read-only mode\n"));
mch_msg(_(" -S <session> Source <session> after loading the first file\n"));
mch_msg(_(" -s <scriptin> Read Normal mode commands from <scriptin>\n"));
mch_msg(_(" -u <config> Use this config file\n"));
mch_msg(_(" -v, --version Print version information\n"));
mch_msg(_(" -V[N][file] Verbose [level][file]\n"));
mch_msg(_(" -Z Restricted mode\n"));
mch_msg("\n");
mch_msg(_(" --api-info Write msgpack-encoded API metadata to stdout\n"));
mch_msg(_(" --embed Use stdin/stdout as a msgpack-rpc channel\n"));
mch_msg(_(" --headless Don't start a user interface\n"));
mch_msg(_(" --listen <address> Serve RPC API from this address\n"));
mch_msg(_(" --noplugin Don't load plugins\n"));
mch_msg(_(" --startuptime <file> Write startup timing messages to <file>\n"));
mch_msg(_("\nSee \":help startup-options\" for all options.\n"));
}
static void check_swap_exists_action(void)
{
if (swap_exists_action == SEA_QUIT)
getout(1);
handle_swap_exists(NULL);
}
