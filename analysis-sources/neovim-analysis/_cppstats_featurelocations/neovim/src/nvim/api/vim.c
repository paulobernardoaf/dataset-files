


#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "nvim/api/vim.h"
#include "nvim/ascii.h"
#include "nvim/api/private/helpers.h"
#include "nvim/api/private/defs.h"
#include "nvim/api/private/dispatch.h"
#include "nvim/api/buffer.h"
#include "nvim/api/window.h"
#include "nvim/msgpack_rpc/channel.h"
#include "nvim/msgpack_rpc/helpers.h"
#include "nvim/lua/executor.h"
#include "nvim/vim.h"
#include "nvim/buffer.h"
#include "nvim/context.h"
#include "nvim/file_search.h"
#include "nvim/highlight.h"
#include "nvim/window.h"
#include "nvim/types.h"
#include "nvim/ex_cmds2.h"
#include "nvim/ex_docmd.h"
#include "nvim/screen.h"
#include "nvim/memline.h"
#include "nvim/mark.h"
#include "nvim/memory.h"
#include "nvim/message.h"
#include "nvim/popupmnu.h"
#include "nvim/edit.h"
#include "nvim/eval.h"
#include "nvim/eval/typval.h"
#include "nvim/fileio.h"
#include "nvim/ops.h"
#include "nvim/option.h"
#include "nvim/state.h"
#include "nvim/extmark.h"
#include "nvim/syntax.h"
#include "nvim/getchar.h"
#include "nvim/os/input.h"
#include "nvim/os/process.h"
#include "nvim/viml/parser/expressions.h"
#include "nvim/viml/parser/parser.h"
#include "nvim/ui.h"

#define LINE_BUFFER_SIZE 4096

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "api/vim.c.generated.h"
#endif

void api_vim_init(void)
FUNC_API_NOEXPORT
{
namespace_ids = map_new(String, handle_T)();
}

void api_vim_free_all_mem(void)
FUNC_API_NOEXPORT
{
String name;
handle_T id;
map_foreach(namespace_ids, name, id, {
(void)id;
xfree(name.data);
})
map_free(String, handle_T)(namespace_ids);
}

















String nvim_exec(String src, Boolean output, Error *err)
FUNC_API_SINCE(7)
{
const int save_msg_silent = msg_silent;
garray_T *const save_capture_ga = capture_ga;
garray_T capture_local;
if (output) {
ga_init(&capture_local, 1, 80);
capture_ga = &capture_local;
}

try_start();
msg_silent++;
do_source_str(src.data, "nvim_exec()");
capture_ga = save_capture_ga;
msg_silent = save_msg_silent;
try_end(err);

if (ERROR_SET(err)) {
goto theend;
}

if (output && capture_local.ga_len > 1) {
String s = (String){
.data = capture_local.ga_data,
.size = (size_t)capture_local.ga_len,
};

if (s.data[0] == '\n') {
memmove(s.data, s.data + 1, s.size - 1);
s.data[s.size - 1] = '\0';
s.size = s.size - 1;
}
return s; 
}
theend:
if (output) {
ga_clear(&capture_local);
}
return (String)STRING_INIT;
}









void nvim_command(String command, Error *err)
FUNC_API_SINCE(1)
{
try_start();
do_cmdline_cmd(command.data);
try_end(err);
}








Dictionary nvim_get_hl_by_name(String name, Boolean rgb, Error *err)
FUNC_API_SINCE(3)
{
Dictionary result = ARRAY_DICT_INIT;
int id = syn_name2id((const char_u *)name.data);

if (id == 0) {
api_set_error(err, kErrorTypeException, "Invalid highlight name: %s",
name.data);
return result;
}
result = nvim_get_hl_by_id(id, rgb, err);
return result;
}








Dictionary nvim_get_hl_by_id(Integer hl_id, Boolean rgb, Error *err)
FUNC_API_SINCE(3)
{
Dictionary dic = ARRAY_DICT_INIT;
if (syn_get_final_id((int)hl_id) == 0) {
api_set_error(err, kErrorTypeException,
"Invalid highlight id: %" PRId64, hl_id);
return dic;
}
int attrcode = syn_id2attr((int)hl_id);
return hl_get_attr_by_id(attrcode, rgb, err);
}




Integer nvim_get_hl_id_by_name(String name)
FUNC_API_SINCE(7)
{
return syn_check_group((const char_u *)name.data, (int)name.size);
}











void nvim_feedkeys(String keys, String mode, Boolean escape_csi)
FUNC_API_SINCE(1)
{
bool remap = true;
bool insert = false;
bool typed = false;
bool execute = false;
bool dangerous = false;

for (size_t i = 0; i < mode.size; ++i) {
switch (mode.data[i]) {
case 'n': remap = false; break;
case 'm': remap = true; break;
case 't': typed = true; break;
case 'i': insert = true; break;
case 'x': execute = true; break;
case '!': dangerous = true; break;
}
}

if (keys.size == 0 && !execute) {
return;
}

char *keys_esc;
if (escape_csi) {


keys_esc = (char *)vim_strsave_escape_csi((char_u *)keys.data);
} else {
keys_esc = keys.data;
}
ins_typebuf((char_u *)keys_esc, (remap ? REMAP_YES : REMAP_NONE),
insert ? 0 : typebuf.tb_len, !typed, false);
if (vgetc_busy) {
typebuf_was_filled = true;
}

if (escape_csi) {
xfree(keys_esc);
}

if (execute) {
int save_msg_scroll = msg_scroll;


msg_scroll = false;
if (!dangerous) {
ex_normal_busy++;
}
exec_normal(true);
if (!dangerous) {
ex_normal_busy--;
}
msg_scroll |= save_msg_scroll;
}
}
















Integer nvim_input(String keys)
FUNC_API_SINCE(1) FUNC_API_FAST
{
return (Integer)input_enqueue(keys);
}























void nvim_input_mouse(String button, String action, String modifier,
Integer grid, Integer row, Integer col, Error *err)
FUNC_API_SINCE(6) FUNC_API_FAST
{
if (button.data == NULL || action.data == NULL) {
goto error;
}

int code = 0;

if (strequal(button.data, "left")) {
code = KE_LEFTMOUSE;
} else if (strequal(button.data, "middle")) {
code = KE_MIDDLEMOUSE;
} else if (strequal(button.data, "right")) {
code = KE_RIGHTMOUSE;
} else if (strequal(button.data, "wheel")) {
code = KE_MOUSEDOWN;
} else {
goto error;
}

if (code == KE_MOUSEDOWN) {
if (strequal(action.data, "down")) {
code = KE_MOUSEUP;
} else if (strequal(action.data, "up")) {

} else if (strequal(action.data, "left")) {
code = KE_MOUSERIGHT;
} else if (strequal(action.data, "right")) {
code = KE_MOUSELEFT;
} else {
goto error;
}
} else {
if (strequal(action.data, "press")) {

} else if (strequal(action.data, "drag")) {
code += KE_LEFTDRAG - KE_LEFTMOUSE;
} else if (strequal(action.data, "release")) {
code += KE_LEFTRELEASE - KE_LEFTMOUSE;
} else {
goto error;
}
}

int modmask = 0;
for (size_t i = 0; i < modifier.size; i++) {
char byte = modifier.data[i];
if (byte == '-') {
continue;
}
int mod = name_to_mod_mask(byte);
if (mod == 0) {
api_set_error(err, kErrorTypeValidation,
"invalid modifier %c", byte);
return;
}
modmask |= mod;
}

input_enqueue_mouse(code, (uint8_t)modmask, (int)grid, (int)row, (int)col);
return;

error:
api_set_error(err, kErrorTypeValidation,
"invalid button or action");
}










String nvim_replace_termcodes(String str, Boolean from_part, Boolean do_lt,
Boolean special)
FUNC_API_SINCE(1)
{
if (str.size == 0) {

return (String) { .data = NULL, .size = 0 };
}

char *ptr = NULL;
replace_termcodes((char_u *)str.data, str.size, (char_u **)&ptr,
from_part, do_lt, special, CPO_TO_CPO_FLAGS);
return cstr_as_string(ptr);
}



String nvim_command_output(String command, Error *err)
FUNC_API_SINCE(1)
FUNC_API_DEPRECATED_SINCE(7)
{
return nvim_exec(command, true, err);
}









Object nvim_eval(String expr, Error *err)
FUNC_API_SINCE(1)
{
static int recursive = 0; 
Object rv = OBJECT_INIT;

TRY_WRAP({

if (!recursive) {
force_abort = false;
suppress_errthrow = false;
current_exception = NULL;

did_emsg = false;
}
recursive++;
try_start();

typval_T rettv;
int ok = eval0((char_u *)expr.data, &rettv, NULL, true);

if (!try_end(err)) {
if (ok == FAIL) {

api_set_error(err, kErrorTypeException,
"Failed to evaluate expression: '%.*s'", 256, expr.data);
} else {
rv = vim_to_object(&rettv);
}
}

tv_clear(&rettv);
recursive--;
});

return rv;
}



Object nvim_execute_lua(String code, Array args, Error *err)
FUNC_API_SINCE(3)
FUNC_API_DEPRECATED_SINCE(7)
FUNC_API_REMOTE_ONLY
{
return executor_exec_lua_api(code, args, err);
}













Object nvim_exec_lua(String code, Array args, Error *err)
FUNC_API_SINCE(7)
FUNC_API_REMOTE_ONLY
{
return executor_exec_lua_api(code, args, err);
}








static Object _call_function(String fn, Array args, dict_T *self, Error *err)
{
static int recursive = 0; 
Object rv = OBJECT_INIT;

if (args.size > MAX_FUNC_ARGS) {
api_set_error(err, kErrorTypeValidation,
"Function called with too many arguments");
return rv;
}


typval_T vim_args[MAX_FUNC_ARGS + 1];
size_t i = 0; 
for (; i < args.size; i++) {
if (!object_to_vim(args.items[i], &vim_args[i], err)) {
goto free_vim_args;
}
}

TRY_WRAP({

if (!recursive) {
force_abort = false;
suppress_errthrow = false;
current_exception = NULL;

did_emsg = false;
}
recursive++;
try_start();
typval_T rettv;
int dummy;


(void)call_func((char_u *)fn.data, (int)fn.size, &rettv, (int)args.size,
vim_args, NULL, curwin->w_cursor.lnum, curwin->w_cursor.lnum,
&dummy, true, NULL, self);
if (!try_end(err)) {
rv = vim_to_object(&rettv);
}
tv_clear(&rettv);
recursive--;
});

free_vim_args:
while (i > 0) {
tv_clear(&vim_args[--i]);
}

return rv;
}









Object nvim_call_function(String fn, Array args, Error *err)
FUNC_API_SINCE(1)
{
return _call_function(fn, args, NULL, err);
}










Object nvim_call_dict_function(Object dict, String fn, Array args, Error *err)
FUNC_API_SINCE(4)
{
Object rv = OBJECT_INIT;

typval_T rettv;
bool mustfree = false;
switch (dict.type) {
case kObjectTypeString: {
try_start();
if (eval0((char_u *)dict.data.string.data, &rettv, NULL, true) == FAIL) {
api_set_error(err, kErrorTypeException,
"Failed to evaluate dict expression");
}
if (try_end(err)) {
return rv;
}


mustfree = true;
break;
}
case kObjectTypeDictionary: {
if (!object_to_vim(dict, &rettv, err)) {
goto end;
}
break;
}
default: {
api_set_error(err, kErrorTypeValidation,
"dict argument type must be String or Dictionary");
return rv;
}
}
dict_T *self_dict = rettv.vval.v_dict;
if (rettv.v_type != VAR_DICT || !self_dict) {
api_set_error(err, kErrorTypeValidation, "dict not found");
goto end;
}

if (fn.data && fn.size > 0 && dict.type != kObjectTypeDictionary) {
dictitem_T *const di = tv_dict_find(self_dict, fn.data, (ptrdiff_t)fn.size);
if (di == NULL) {
api_set_error(err, kErrorTypeValidation, "Not found: %s", fn.data);
goto end;
}
if (di->di_tv.v_type == VAR_PARTIAL) {
api_set_error(err, kErrorTypeValidation,
"partial function not supported");
goto end;
}
if (di->di_tv.v_type != VAR_FUNC) {
api_set_error(err, kErrorTypeValidation, "Not a function: %s", fn.data);
goto end;
}
fn = (String) {
.data = (char *)di->di_tv.vval.v_string,
.size = strlen((char *)di->di_tv.vval.v_string),
};
}

if (!fn.data || fn.size < 1) {
api_set_error(err, kErrorTypeValidation, "Invalid (empty) function name");
goto end;
}

rv = _call_function(fn, args, self_dict, err);
end:
if (mustfree) {
tv_clear(&rettv);
}

return rv;
}







Integer nvim_strwidth(String text, Error *err)
FUNC_API_SINCE(1)
{
if (text.size > INT_MAX) {
api_set_error(err, kErrorTypeValidation, "String is too long");
return 0;
}

return (Integer)mb_string2cells((char_u *)text.data);
}




ArrayOf(String) nvim_list_runtime_paths(void)
FUNC_API_SINCE(1)
{
Array rv = ARRAY_DICT_INIT;
char_u *rtp = p_rtp;

if (*rtp == NUL) {

return rv;
}


while (*rtp != NUL) {
if (*rtp == ',') {
rv.size++;
}
rtp++;
}
rv.size++;


rv.items = xmalloc(sizeof(*rv.items) * rv.size);

rtp = p_rtp;

for (size_t i = 0; i < rv.size; i++) {
rv.items[i].type = kObjectTypeString;
rv.items[i].data.string.data = xmalloc(MAXPATHL);

size_t length = copy_option_part(&rtp,
(char_u *)rv.items[i].data.string.data,
MAXPATHL,
",");
rv.items[i].data.string.size = length;
}

return rv;
}












ArrayOf(String) nvim_get_runtime_file(String name, Boolean all)
FUNC_API_SINCE(7)
{
Array rv = ARRAY_DICT_INIT;
if (!name.data) {
return rv;
}
int flags = DIP_START | (all ? DIP_ALL : 0);
do_in_runtimepath((char_u *)name.data, flags, find_runtime_cb, &rv);
return rv;
}

static void find_runtime_cb(char_u *fname, void *cookie)
{
Array *rv = (Array *)cookie;
ADD(*rv, STRING_OBJ(cstr_to_string((char *)fname)));
}

String nvim__get_lib_dir(void)
{
return cstr_as_string(get_lib_dir());
}





void nvim_set_current_dir(String dir, Error *err)
FUNC_API_SINCE(1)
{
if (dir.size >= MAXPATHL) {
api_set_error(err, kErrorTypeValidation, "Directory name is too long");
return;
}

char string[MAXPATHL];
memcpy(string, dir.data, dir.size);
string[dir.size] = NUL;

try_start();

if (vim_chdir((char_u *)string)) {
if (!try_end(err)) {
api_set_error(err, kErrorTypeException, "Failed to change directory");
}
return;
}

post_chdir(kCdScopeGlobal, true);
try_end(err);
}





String nvim_get_current_line(Error *err)
FUNC_API_SINCE(1)
{
return buffer_get_line(curbuf->handle, curwin->w_cursor.lnum - 1, err);
}





void nvim_set_current_line(String line, Error *err)
FUNC_API_SINCE(1)
{
buffer_set_line(curbuf->handle, curwin->w_cursor.lnum - 1, line, err);
}




void nvim_del_current_line(Error *err)
FUNC_API_SINCE(1)
{
buffer_del_line(curbuf->handle, curwin->w_cursor.lnum - 1, err);
}






Object nvim_get_var(String name, Error *err)
FUNC_API_SINCE(1)
{
return dict_get_value(&globvardict, name, err);
}






void nvim_set_var(String name, Object value, Error *err)
FUNC_API_SINCE(1)
{
dict_set_var(&globvardict, name, value, false, false, err);
}





void nvim_del_var(String name, Error *err)
FUNC_API_SINCE(1)
{
dict_set_var(&globvardict, name, NIL, true, false, err);
}






Object vim_set_var(String name, Object value, Error *err)
{
return dict_set_var(&globvardict, name, value, false, true, err);
}



Object vim_del_var(String name, Error *err)
{
return dict_set_var(&globvardict, name, NIL, true, true, err);
}






Object nvim_get_vvar(String name, Error *err)
FUNC_API_SINCE(1)
{
return dict_get_value(&vimvardict, name, err);
}






void nvim_set_vvar(String name, Object value, Error *err)
FUNC_API_SINCE(6)
{
dict_set_var(&vimvardict, name, value, false, false, err);
}






Object nvim_get_option(String name, Error *err)
FUNC_API_SINCE(1)
{
return get_option_from(NULL, SREQ_GLOBAL, name, err);
}







void nvim_set_option(uint64_t channel_id, String name, Object value, Error *err)
FUNC_API_SINCE(1)
{
set_option_to(channel_id, NULL, SREQ_GLOBAL, name, value, err);
}





void nvim_out_write(String str)
FUNC_API_SINCE(1)
{
write_msg(str, false);
}





void nvim_err_write(String str)
FUNC_API_SINCE(1)
{
write_msg(str, true);
}






void nvim_err_writeln(String str)
FUNC_API_SINCE(1)
{
nvim_err_write(str);
nvim_err_write((String) { .data = "\n", .size = 1 });
}







ArrayOf(Buffer) nvim_list_bufs(void)
FUNC_API_SINCE(1)
{
Array rv = ARRAY_DICT_INIT;

FOR_ALL_BUFFERS(b) {
rv.size++;
}

rv.items = xmalloc(sizeof(Object) * rv.size);
size_t i = 0;

FOR_ALL_BUFFERS(b) {
rv.items[i++] = BUFFER_OBJ(b->handle);
}

return rv;
}




Buffer nvim_get_current_buf(void)
FUNC_API_SINCE(1)
{
return curbuf->handle;
}





void nvim_set_current_buf(Buffer buffer, Error *err)
FUNC_API_SINCE(1)
{
buf_T *buf = find_buffer_by_handle(buffer, err);

if (!buf) {
return;
}

try_start();
int result = do_buffer(DOBUF_GOTO, DOBUF_FIRST, FORWARD, buf->b_fnum, 0);
if (!try_end(err) && result == FAIL) {
api_set_error(err,
kErrorTypeException,
"Failed to switch to buffer %d",
buffer);
}
}




ArrayOf(Window) nvim_list_wins(void)
FUNC_API_SINCE(1)
{
Array rv = ARRAY_DICT_INIT;

FOR_ALL_TAB_WINDOWS(tp, wp) {
rv.size++;
}

rv.items = xmalloc(sizeof(Object) * rv.size);
size_t i = 0;

FOR_ALL_TAB_WINDOWS(tp, wp) {
rv.items[i++] = WINDOW_OBJ(wp->handle);
}

return rv;
}




Window nvim_get_current_win(void)
FUNC_API_SINCE(1)
{
return curwin->handle;
}





void nvim_set_current_win(Window window, Error *err)
FUNC_API_SINCE(1)
{
win_T *win = find_window_by_handle(window, err);

if (!win) {
return;
}

try_start();
goto_tabpage_win(win_find_tabpage(win), win);
if (!try_end(err) && win != curwin) {
api_set_error(err,
kErrorTypeException,
"Failed to switch to window %d",
window);
}
}










Buffer nvim_create_buf(Boolean listed, Boolean scratch, Error *err)
FUNC_API_SINCE(6)
{
try_start();
buf_T *buf = buflist_new(NULL, NULL, (linenr_T)0,
BLN_NOOPT | BLN_NEW | (listed ? BLN_LISTED : 0));
try_end(err);
if (buf == NULL) {
goto fail;
}



try_start();
block_autocmds();
int status = ml_open(buf);
unblock_autocmds();
try_end(err);
if (status == FAIL) {
goto fail;
}

if (scratch) {
aco_save_T aco;
aucmd_prepbuf(&aco, buf);
set_option_value("bh", 0L, "hide", OPT_LOCAL);
set_option_value("bt", 0L, "nofile", OPT_LOCAL);
set_option_value("swf", 0L, NULL, OPT_LOCAL);
aucmd_restbuf(&aco);
}
return buf->b_fnum;

fail:
if (!ERROR_SET(err)) {
api_set_error(err, kErrorTypeException, "Failed to create buffer");
}
return 0;
}

















































































Window nvim_open_win(Buffer buffer, Boolean enter, Dictionary config,
Error *err)
FUNC_API_SINCE(6)
{
FloatConfig fconfig = FLOAT_CONFIG_INIT;
if (!parse_float_config(config, &fconfig, false, err)) {
return 0;
}
win_T *wp = win_new_float(NULL, fconfig, err);
if (!wp) {
return 0;
}
if (enter) {
win_enter(wp, false);
}
if (!win_valid(wp)) {
api_set_error(err, kErrorTypeException, "Window was closed immediately");
return 0;
}
if (buffer > 0) {
nvim_win_set_buf(wp->handle, buffer, err);
}

if (fconfig.style == kWinStyleMinimal) {
win_set_minimal_style(wp);
didset_window_options(wp);
}
return wp->handle;
}




ArrayOf(Tabpage) nvim_list_tabpages(void)
FUNC_API_SINCE(1)
{
Array rv = ARRAY_DICT_INIT;

FOR_ALL_TABS(tp) {
rv.size++;
}

rv.items = xmalloc(sizeof(Object) * rv.size);
size_t i = 0;

FOR_ALL_TABS(tp) {
rv.items[i++] = TABPAGE_OBJ(tp->handle);
}

return rv;
}




Tabpage nvim_get_current_tabpage(void)
FUNC_API_SINCE(1)
{
return curtab->handle;
}





void nvim_set_current_tabpage(Tabpage tabpage, Error *err)
FUNC_API_SINCE(1)
{
tabpage_T *tp = find_tab_by_handle(tabpage, err);

if (!tp) {
return;
}

try_start();
goto_tabpage_tp(tp, true, true);
if (!try_end(err) && tp != curtab) {
api_set_error(err,
kErrorTypeException,
"Failed to switch to tabpage %d",
tabpage);
}
}












Integer nvim_create_namespace(String name)
FUNC_API_SINCE(5)
{
handle_T id = map_get(String, handle_T)(namespace_ids, name);
if (id > 0) {
return id;
}
id = next_namespace_id++;
if (name.size > 0) {
String name_alloc = copy_string(name);
map_put(String, handle_T)(namespace_ids, name_alloc, id);
}
return (Integer)id;
}




Dictionary nvim_get_namespaces(void)
FUNC_API_SINCE(5)
{
Dictionary retval = ARRAY_DICT_INIT;
String name;
handle_T id;

map_foreach(namespace_ids, name, id, {
PUT(retval, name.data, INTEGER_OBJ(id));
})

return retval;
}























Boolean nvim_paste(String data, Boolean crlf, Integer phase, Error *err)
FUNC_API_SINCE(6)
{
static bool draining = false;
bool cancel = false;

if (phase < -1 || phase > 3) {
api_set_error(err, kErrorTypeValidation, "Invalid phase: %"PRId64, phase);
return false;
}
Array args = ARRAY_DICT_INIT;
Object rv = OBJECT_INIT;
if (phase == -1 || phase == 1) { 
draining = false;
} else if (draining) {

goto theend;
}
Array lines = string_to_array(data, crlf);
ADD(args, ARRAY_OBJ(lines));
ADD(args, INTEGER_OBJ(phase));
rv = nvim_exec_lua(STATIC_CSTR_AS_STRING("return vim.paste(...)"), args,
err);
if (ERROR_SET(err)) {
draining = true;
goto theend;
}
if (!(State & (CMDLINE | INSERT)) && (phase == -1 || phase == 1)) {
ResetRedobuff();
AppendCharToRedobuff('a'); 
}


cancel = (rv.type == kObjectTypeBoolean && !rv.data.boolean);
if (!cancel && !(State & CMDLINE)) { 
for (size_t i = 0; i < lines.size; i++) {
String s = lines.items[i].data.string;
assert(data.size <= INT_MAX);
AppendToRedobuffLit((char_u *)s.data, (int)s.size);

if (i + 1 < lines.size) {
AppendCharToRedobuff(NL);
}
}
}
if (!(State & (CMDLINE | INSERT)) && (phase == -1 || phase == 3)) {
AppendCharToRedobuff(ESC); 
}
theend:
api_free_object(rv);
api_free_array(args);
if (cancel || phase == -1 || phase == 3) { 
draining = false;
}

return !cancel;
}














void nvim_put(ArrayOf(String) lines, String type, Boolean after,
Boolean follow, Error *err)
FUNC_API_SINCE(6)
{
yankreg_T *reg = xcalloc(sizeof(yankreg_T), 1);
if (!prepare_yankreg_from_object(reg, type, lines.size)) {
api_set_error(err, kErrorTypeValidation, "Invalid type: '%s'", type.data);
goto cleanup;
}
if (lines.size == 0) {
goto cleanup; 
}

for (size_t i = 0; i < lines.size; i++) {
if (lines.items[i].type != kObjectTypeString) {
api_set_error(err, kErrorTypeValidation,
"Invalid lines (expected array of strings)");
goto cleanup;
}
String line = lines.items[i].data.string;
reg->y_array[i] = (char_u *)xmemdupz(line.data, line.size);
memchrsub(reg->y_array[i], NUL, NL, line.size);
}

finish_yankreg_from_object(reg, false);

TRY_WRAP({
try_start();
bool VIsual_was_active = VIsual_active;
msg_silent++; 
do_put(0, reg, after ? FORWARD : BACKWARD, 1, follow ? PUT_CURSEND : 0);
msg_silent--;
VIsual_active = VIsual_was_active;
try_end(err);
});

cleanup:
free_register(reg);
xfree(reg);
}





void nvim_subscribe(uint64_t channel_id, String event)
FUNC_API_SINCE(1) FUNC_API_REMOTE_ONLY
{
size_t length = (event.size < METHOD_MAXLEN ? event.size : METHOD_MAXLEN);
char e[METHOD_MAXLEN + 1];
memcpy(e, event.data, length);
e[length] = NUL;
rpc_subscribe(channel_id, e);
}





void nvim_unsubscribe(uint64_t channel_id, String event)
FUNC_API_SINCE(1) FUNC_API_REMOTE_ONLY
{
size_t length = (event.size < METHOD_MAXLEN ?
event.size :
METHOD_MAXLEN);
char e[METHOD_MAXLEN + 1];
memcpy(e, event.data, length);
e[length] = NUL;
rpc_unsubscribe(channel_id, e);
}












Integer nvim_get_color_by_name(String name)
FUNC_API_SINCE(1)
{
return name_to_color((char_u *)name.data);
}







Dictionary nvim_get_color_map(void)
FUNC_API_SINCE(1)
{
Dictionary colors = ARRAY_DICT_INIT;

for (int i = 0; color_name_table[i].name != NULL; i++) {
PUT(colors, color_name_table[i].name,
INTEGER_OBJ(color_name_table[i].color));
}
return colors;
}









Dictionary nvim_get_context(Dictionary opts, Error *err)
FUNC_API_SINCE(6)
{
Array types = ARRAY_DICT_INIT;
for (size_t i = 0; i < opts.size; i++) {
String k = opts.items[i].key;
Object v = opts.items[i].value;
if (strequal("types", k.data)) {
if (v.type != kObjectTypeArray) {
api_set_error(err, kErrorTypeValidation, "invalid value for key: %s",
k.data);
return (Dictionary)ARRAY_DICT_INIT;
}
types = v.data.array;
} else {
api_set_error(err, kErrorTypeValidation, "unexpected key: %s", k.data);
return (Dictionary)ARRAY_DICT_INIT;
}
}

int int_types = types.size > 0 ? 0 : kCtxAll;
if (types.size > 0) {
for (size_t i = 0; i < types.size; i++) {
if (types.items[i].type == kObjectTypeString) {
const char *const s = types.items[i].data.string.data;
if (strequal(s, "regs")) {
int_types |= kCtxRegs;
} else if (strequal(s, "jumps")) {
int_types |= kCtxJumps;
} else if (strequal(s, "bufs")) {
int_types |= kCtxBufs;
} else if (strequal(s, "gvars")) {
int_types |= kCtxGVars;
} else if (strequal(s, "sfuncs")) {
int_types |= kCtxSFuncs;
} else if (strequal(s, "funcs")) {
int_types |= kCtxFuncs;
} else {
api_set_error(err, kErrorTypeValidation, "unexpected type: %s", s);
return (Dictionary)ARRAY_DICT_INIT;
}
}
}
}

Context ctx = CONTEXT_INIT;
ctx_save(&ctx, int_types);
Dictionary dict = ctx_to_dict(&ctx);
ctx_free(&ctx);
return dict;
}




Object nvim_load_context(Dictionary dict)
FUNC_API_SINCE(6)
{
Context ctx = CONTEXT_INIT;

int save_did_emsg = did_emsg;
did_emsg = false;

ctx_from_dict(dict, &ctx);
if (!did_emsg) {
ctx_restore(&ctx, kCtxAll);
}

ctx_free(&ctx);

did_emsg = save_did_emsg;
return (Object)OBJECT_INIT;
}





Dictionary nvim_get_mode(void)
FUNC_API_SINCE(2) FUNC_API_FAST
{
Dictionary rv = ARRAY_DICT_INIT;
char *modestr = get_mode();
bool blocked = input_blocking();

PUT(rv, "mode", STRING_OBJ(cstr_as_string(modestr)));
PUT(rv, "blocking", BOOLEAN_OBJ(blocked));

return rv;
}






ArrayOf(Dictionary) nvim_get_keymap(String mode)
FUNC_API_SINCE(3)
{
return keymap_array(mode, NULL);
}


























void nvim_set_keymap(String mode, String lhs, String rhs,
Dictionary opts, Error *err)
FUNC_API_SINCE(6)
{
modify_keymap(-1, false, mode, lhs, rhs, opts, err);
}






void nvim_del_keymap(String mode, String lhs, Error *err)
FUNC_API_SINCE(6)
{
nvim_buf_del_keymap(-1, mode, lhs, err);
}










Dictionary nvim_get_commands(Dictionary opts, Error *err)
FUNC_API_SINCE(4)
{
return nvim_buf_get_commands(-1, opts, err);
}





Array nvim_get_api_info(uint64_t channel_id)
FUNC_API_SINCE(1) FUNC_API_FAST FUNC_API_REMOTE_ONLY
{
Array rv = ARRAY_DICT_INIT;

assert(channel_id <= INT64_MAX);
ADD(rv, INTEGER_OBJ((int64_t)channel_id));
ADD(rv, DICTIONARY_OBJ(api_metadata()));

return rv;
}


















































void nvim_set_client_info(uint64_t channel_id, String name,
Dictionary version, String type,
Dictionary methods, Dictionary attributes,
Error *err)
FUNC_API_SINCE(4) FUNC_API_REMOTE_ONLY
{
Dictionary info = ARRAY_DICT_INIT;
PUT(info, "name", copy_object(STRING_OBJ(name)));

version = copy_dictionary(version);
bool has_major = false;
for (size_t i = 0; i < version.size; i++) {
if (strequal(version.items[i].key.data, "major")) {
has_major = true;
break;
}
}
if (!has_major) {
PUT(version, "major", INTEGER_OBJ(0));
}
PUT(info, "version", DICTIONARY_OBJ(version));

PUT(info, "type", copy_object(STRING_OBJ(type)));
PUT(info, "methods", DICTIONARY_OBJ(copy_dictionary(methods)));
PUT(info, "attributes", DICTIONARY_OBJ(copy_dictionary(attributes)));

rpc_set_client_info(channel_id, info);
}























Dictionary nvim_get_chan_info(Integer chan, Error *err)
FUNC_API_SINCE(4)
{
if (chan < 0) {
return (Dictionary)ARRAY_DICT_INIT;
}
return channel_info((uint64_t)chan);
}





Array nvim_list_chans(void)
FUNC_API_SINCE(4)
{
return channel_all_info();
}





















Array nvim_call_atomic(uint64_t channel_id, Array calls, Error *err)
FUNC_API_SINCE(1) FUNC_API_REMOTE_ONLY
{
Array rv = ARRAY_DICT_INIT;
Array results = ARRAY_DICT_INIT;
Error nested_error = ERROR_INIT;

size_t i; 
for (i = 0; i < calls.size; i++) {
if (calls.items[i].type != kObjectTypeArray) {
api_set_error(err,
kErrorTypeValidation,
"Items in calls array must be arrays");
goto validation_error;
}
Array call = calls.items[i].data.array;
if (call.size != 2) {
api_set_error(err,
kErrorTypeValidation,
"Items in calls array must be arrays of size 2");
goto validation_error;
}

if (call.items[0].type != kObjectTypeString) {
api_set_error(err,
kErrorTypeValidation,
"Name must be String");
goto validation_error;
}
String name = call.items[0].data.string;

if (call.items[1].type != kObjectTypeArray) {
api_set_error(err,
kErrorTypeValidation,
"Args must be Array");
goto validation_error;
}
Array args = call.items[1].data.array;

MsgpackRpcRequestHandler handler =
msgpack_rpc_get_handler_for(name.data,
name.size,
&nested_error);

if (ERROR_SET(&nested_error)) {
break;
}
Object result = handler.fn(channel_id, args, &nested_error);
if (ERROR_SET(&nested_error)) {

break;
}

ADD(results, result);
}

ADD(rv, ARRAY_OBJ(results));
if (ERROR_SET(&nested_error)) {
Array errval = ARRAY_DICT_INIT;
ADD(errval, INTEGER_OBJ((Integer)i));
ADD(errval, INTEGER_OBJ(nested_error.type));
ADD(errval, STRING_OBJ(cstr_to_string(nested_error.msg)));
ADD(rv, ARRAY_OBJ(errval));
} else {
ADD(rv, NIL);
}
goto theend;

validation_error:
api_free_array(results);
theend:
api_clear_error(&nested_error);
return rv;
}

typedef struct {
ExprASTNode **node_p;
Object *ret_node_p;
} ExprASTConvStackItem;


typedef kvec_withinit_t(ExprASTConvStackItem, 16) ExprASTConvStack;












































































Dictionary nvim_parse_expression(String expr, String flags, Boolean highlight,
Error *err)
FUNC_API_SINCE(4) FUNC_API_FAST
{
int pflags = 0;
for (size_t i = 0 ; i < flags.size ; i++) {
switch (flags.data[i]) {
case 'm': { pflags |= kExprFlagsMulti; break; }
case 'E': { pflags |= kExprFlagsDisallowEOC; break; }
case 'l': { pflags |= kExprFlagsParseLet; break; }
case NUL: {
api_set_error(err, kErrorTypeValidation, "Invalid flag: '\\0' (%u)",
(unsigned)flags.data[i]);
return (Dictionary)ARRAY_DICT_INIT;
}
default: {
api_set_error(err, kErrorTypeValidation, "Invalid flag: '%c' (%u)",
flags.data[i], (unsigned)flags.data[i]);
return (Dictionary)ARRAY_DICT_INIT;
}
}
}
ParserLine plines[] = {
{
.data = expr.data,
.size = expr.size,
.allocated = false,
},
{ NULL, 0, false },
};
ParserLine *plines_p = plines;
ParserHighlight colors;
kvi_init(colors);
ParserHighlight *const colors_p = (highlight ? &colors : NULL);
ParserState pstate;
viml_parser_init(
&pstate, parser_simple_get_line, &plines_p, colors_p);
ExprAST east = viml_pexpr_parse(&pstate, pflags);

const size_t ret_size = (
2 
+ (size_t)(east.err.msg != NULL) 
+ (size_t)highlight 
+ 0);
Dictionary ret = {
.items = xmalloc(ret_size * sizeof(ret.items[0])),
.size = 0,
.capacity = ret_size,
};
ret.items[ret.size++] = (KeyValuePair) {
.key = STATIC_CSTR_TO_STRING("ast"),
.value = NIL,
};
ret.items[ret.size++] = (KeyValuePair) {
.key = STATIC_CSTR_TO_STRING("len"),
.value = INTEGER_OBJ((Integer)(pstate.pos.line == 1
? plines[0].size
: pstate.pos.col)),
};
if (east.err.msg != NULL) {
Dictionary err_dict = {
.items = xmalloc(2 * sizeof(err_dict.items[0])),
.size = 2,
.capacity = 2,
};
err_dict.items[0] = (KeyValuePair) {
.key = STATIC_CSTR_TO_STRING("message"),
.value = STRING_OBJ(cstr_to_string(east.err.msg)),
};
if (east.err.arg == NULL) {
err_dict.items[1] = (KeyValuePair) {
.key = STATIC_CSTR_TO_STRING("arg"),
.value = STRING_OBJ(STRING_INIT),
};
} else {
err_dict.items[1] = (KeyValuePair) {
.key = STATIC_CSTR_TO_STRING("arg"),
.value = STRING_OBJ(((String) {
.data = xmemdupz(east.err.arg, (size_t)east.err.arg_len),
.size = (size_t)east.err.arg_len,
})),
};
}
ret.items[ret.size++] = (KeyValuePair) {
.key = STATIC_CSTR_TO_STRING("error"),
.value = DICTIONARY_OBJ(err_dict),
};
}
if (highlight) {
Array hl = (Array) {
.items = xmalloc(kv_size(colors) * sizeof(hl.items[0])),
.capacity = kv_size(colors),
.size = kv_size(colors),
};
for (size_t i = 0 ; i < kv_size(colors) ; i++) {
const ParserHighlightChunk chunk = kv_A(colors, i);
Array chunk_arr = (Array) {
.items = xmalloc(4 * sizeof(chunk_arr.items[0])),
.capacity = 4,
.size = 4,
};
chunk_arr.items[0] = INTEGER_OBJ((Integer)chunk.start.line);
chunk_arr.items[1] = INTEGER_OBJ((Integer)chunk.start.col);
chunk_arr.items[2] = INTEGER_OBJ((Integer)chunk.end_col);
chunk_arr.items[3] = STRING_OBJ(cstr_to_string(chunk.group));
hl.items[i] = ARRAY_OBJ(chunk_arr);
}
ret.items[ret.size++] = (KeyValuePair) {
.key = STATIC_CSTR_TO_STRING("highlight"),
.value = ARRAY_OBJ(hl),
};
}
kvi_destroy(colors);


ExprASTConvStack ast_conv_stack;
kvi_init(ast_conv_stack);
kvi_push(ast_conv_stack, ((ExprASTConvStackItem) {
.node_p = &east.root,
.ret_node_p = &ret.items[0].value,
}));
while (kv_size(ast_conv_stack)) {
ExprASTConvStackItem cur_item = kv_last(ast_conv_stack);
ExprASTNode *const node = *cur_item.node_p;
if (node == NULL) {
assert(kv_size(ast_conv_stack) == 1);
kv_drop(ast_conv_stack, 1);
} else {
if (cur_item.ret_node_p->type == kObjectTypeNil) {
const size_t ret_node_items_size = (size_t)(
3 
+ (node->children != NULL) 
+ (node->type == kExprNodeOption
|| node->type == kExprNodePlainIdentifier) 
+ (node->type == kExprNodeOption
|| node->type == kExprNodePlainIdentifier
|| node->type == kExprNodePlainKey
|| node->type == kExprNodeEnvironment) 
+ (node->type == kExprNodeRegister) 
+ (3 
* (node->type == kExprNodeComparison))
+ (node->type == kExprNodeInteger) 
+ (node->type == kExprNodeFloat) 
+ (node->type == kExprNodeDoubleQuotedString
|| node->type == kExprNodeSingleQuotedString) 
+ (node->type == kExprNodeAssignment) 
+ 0);
Dictionary ret_node = {
.items = xmalloc(ret_node_items_size * sizeof(ret_node.items[0])),
.capacity = ret_node_items_size,
.size = 0,
};
*cur_item.ret_node_p = DICTIONARY_OBJ(ret_node);
}
Dictionary *ret_node = &cur_item.ret_node_p->data.dictionary;
if (node->children != NULL) {
const size_t num_children = 1 + (node->children->next != NULL);
Array children_array = {
.items = xmalloc(num_children * sizeof(children_array.items[0])),
.capacity = num_children,
.size = num_children,
};
for (size_t i = 0; i < num_children; i++) {
children_array.items[i] = NIL;
}
ret_node->items[ret_node->size++] = (KeyValuePair) {
.key = STATIC_CSTR_TO_STRING("children"),
.value = ARRAY_OBJ(children_array),
};
kvi_push(ast_conv_stack, ((ExprASTConvStackItem) {
.node_p = &node->children,
.ret_node_p = &children_array.items[0],
}));
} else if (node->next != NULL) {
kvi_push(ast_conv_stack, ((ExprASTConvStackItem) {
.node_p = &node->next,
.ret_node_p = cur_item.ret_node_p + 1,
}));
} else {
kv_drop(ast_conv_stack, 1);
ret_node->items[ret_node->size++] = (KeyValuePair) {
.key = STATIC_CSTR_TO_STRING("type"),
.value = STRING_OBJ(cstr_to_string(east_node_type_tab[node->type])),
};
Array start_array = {
.items = xmalloc(2 * sizeof(start_array.items[0])),
.capacity = 2,
.size = 2,
};
start_array.items[0] = INTEGER_OBJ((Integer)node->start.line);
start_array.items[1] = INTEGER_OBJ((Integer)node->start.col);
ret_node->items[ret_node->size++] = (KeyValuePair) {
.key = STATIC_CSTR_TO_STRING("start"),
.value = ARRAY_OBJ(start_array),
};
ret_node->items[ret_node->size++] = (KeyValuePair) {
.key = STATIC_CSTR_TO_STRING("len"),
.value = INTEGER_OBJ((Integer)node->len),
};
switch (node->type) {
case kExprNodeDoubleQuotedString:
case kExprNodeSingleQuotedString: {
ret_node->items[ret_node->size++] = (KeyValuePair) {
.key = STATIC_CSTR_TO_STRING("svalue"),
.value = STRING_OBJ(((String) {
.data = node->data.str.value,
.size = node->data.str.size,
})),
};
break;
}
case kExprNodeOption: {
ret_node->items[ret_node->size++] = (KeyValuePair) {
.key = STATIC_CSTR_TO_STRING("scope"),
.value = INTEGER_OBJ(node->data.opt.scope),
};
ret_node->items[ret_node->size++] = (KeyValuePair) {
.key = STATIC_CSTR_TO_STRING("ident"),
.value = STRING_OBJ(((String) {
.data = xmemdupz(node->data.opt.ident,
node->data.opt.ident_len),
.size = node->data.opt.ident_len,
})),
};
break;
}
case kExprNodePlainIdentifier: {
ret_node->items[ret_node->size++] = (KeyValuePair) {
.key = STATIC_CSTR_TO_STRING("scope"),
.value = INTEGER_OBJ(node->data.var.scope),
};
ret_node->items[ret_node->size++] = (KeyValuePair) {
.key = STATIC_CSTR_TO_STRING("ident"),
.value = STRING_OBJ(((String) {
.data = xmemdupz(node->data.var.ident,
node->data.var.ident_len),
.size = node->data.var.ident_len,
})),
};
break;
}
case kExprNodePlainKey: {
ret_node->items[ret_node->size++] = (KeyValuePair) {
.key = STATIC_CSTR_TO_STRING("ident"),
.value = STRING_OBJ(((String) {
.data = xmemdupz(node->data.var.ident,
node->data.var.ident_len),
.size = node->data.var.ident_len,
})),
};
break;
}
case kExprNodeEnvironment: {
ret_node->items[ret_node->size++] = (KeyValuePair) {
.key = STATIC_CSTR_TO_STRING("ident"),
.value = STRING_OBJ(((String) {
.data = xmemdupz(node->data.env.ident,
node->data.env.ident_len),
.size = node->data.env.ident_len,
})),
};
break;
}
case kExprNodeRegister: {
ret_node->items[ret_node->size++] = (KeyValuePair) {
.key = STATIC_CSTR_TO_STRING("name"),
.value = INTEGER_OBJ(node->data.reg.name),
};
break;
}
case kExprNodeComparison: {
ret_node->items[ret_node->size++] = (KeyValuePair) {
.key = STATIC_CSTR_TO_STRING("cmp_type"),
.value = STRING_OBJ(cstr_to_string(
eltkn_cmp_type_tab[node->data.cmp.type])),
};
ret_node->items[ret_node->size++] = (KeyValuePair) {
.key = STATIC_CSTR_TO_STRING("ccs_strategy"),
.value = STRING_OBJ(cstr_to_string(
ccs_tab[node->data.cmp.ccs])),
};
ret_node->items[ret_node->size++] = (KeyValuePair) {
.key = STATIC_CSTR_TO_STRING("invert"),
.value = BOOLEAN_OBJ(node->data.cmp.inv),
};
break;
}
case kExprNodeFloat: {
ret_node->items[ret_node->size++] = (KeyValuePair) {
.key = STATIC_CSTR_TO_STRING("fvalue"),
.value = FLOAT_OBJ(node->data.flt.value),
};
break;
}
case kExprNodeInteger: {
ret_node->items[ret_node->size++] = (KeyValuePair) {
.key = STATIC_CSTR_TO_STRING("ivalue"),
.value = INTEGER_OBJ((Integer)(
node->data.num.value > API_INTEGER_MAX
? API_INTEGER_MAX
: (Integer)node->data.num.value)),
};
break;
}
case kExprNodeAssignment: {
const ExprAssignmentType asgn_type = node->data.ass.type;
ret_node->items[ret_node->size++] = (KeyValuePair) {
.key = STATIC_CSTR_TO_STRING("augmentation"),
.value = STRING_OBJ(
asgn_type == kExprAsgnPlain
? (String)STRING_INIT
: cstr_to_string(expr_asgn_type_tab[asgn_type])),
};
break;
}
case kExprNodeMissing:
case kExprNodeOpMissing:
case kExprNodeTernary:
case kExprNodeTernaryValue:
case kExprNodeSubscript:
case kExprNodeListLiteral:
case kExprNodeUnaryPlus:
case kExprNodeBinaryPlus:
case kExprNodeNested:
case kExprNodeCall:
case kExprNodeComplexIdentifier:
case kExprNodeUnknownFigure:
case kExprNodeLambda:
case kExprNodeDictLiteral:
case kExprNodeCurlyBracesIdentifier:
case kExprNodeComma:
case kExprNodeColon:
case kExprNodeArrow:
case kExprNodeConcat:
case kExprNodeConcatOrSubscript:
case kExprNodeOr:
case kExprNodeAnd:
case kExprNodeUnaryMinus:
case kExprNodeBinaryMinus:
case kExprNodeNot:
case kExprNodeMultiplication:
case kExprNodeDivision:
case kExprNodeMod: {
break;
}
}
assert(cur_item.ret_node_p->data.dictionary.size
== cur_item.ret_node_p->data.dictionary.capacity);
xfree(*cur_item.node_p);
*cur_item.node_p = NULL;
}
}
}
kvi_destroy(ast_conv_stack);

assert(ret.size == ret.capacity);


viml_pexpr_free_ast(east);
viml_parser_destroy(&pstate);
return ret;
}








static void write_msg(String message, bool to_err)
{
static size_t out_pos = 0, err_pos = 0;
static char out_line_buf[LINE_BUFFER_SIZE], err_line_buf[LINE_BUFFER_SIZE];

#define PUSH_CHAR(i, pos, line_buf, msg) if (message.data[i] == NL || pos == LINE_BUFFER_SIZE - 1) { line_buf[pos] = NUL; msg((char_u *)line_buf); pos = 0; continue; } line_buf[pos++] = message.data[i];









++no_wait_return;
for (uint32_t i = 0; i < message.size; i++) {
if (to_err) {
PUSH_CHAR(i, err_pos, err_line_buf, emsg);
} else {
PUSH_CHAR(i, out_pos, out_line_buf, msg);
}
}
--no_wait_return;
msg_end();
}











Object nvim__id(Object obj)
{
return copy_object(obj);
}









Array nvim__id_array(Array arr)
{
return copy_object(ARRAY_OBJ(arr)).data.array;
}









Dictionary nvim__id_dictionary(Dictionary dct)
{
return copy_object(DICTIONARY_OBJ(dct)).data.dictionary;
}









Float nvim__id_float(Float flt)
{
return flt;
}




Dictionary nvim__stats(void)
{
Dictionary rv = ARRAY_DICT_INIT;
PUT(rv, "fsync", INTEGER_OBJ(g_stats.fsync));
PUT(rv, "redraw", INTEGER_OBJ(g_stats.redraw));
return rv;
}









Array nvim_list_uis(void)
FUNC_API_SINCE(4)
{
return ui_array();
}




Array nvim_get_proc_children(Integer pid, Error *err)
FUNC_API_SINCE(4)
{
Array rvobj = ARRAY_DICT_INIT;
int *proc_list = NULL;

if (pid <= 0 || pid > INT_MAX) {
api_set_error(err, kErrorTypeException, "Invalid pid: %" PRId64, pid);
goto end;
}

size_t proc_count;
int rv = os_proc_children((int)pid, &proc_list, &proc_count);
if (rv != 0) {

DLOG("fallback to vim._os_proc_children()");
Array a = ARRAY_DICT_INIT;
ADD(a, INTEGER_OBJ(pid));
String s = cstr_to_string("return vim._os_proc_children(select(1, ...))");
Object o = nvim_exec_lua(s, a, err);
api_free_string(s);
api_free_array(a);
if (o.type == kObjectTypeArray) {
rvobj = o.data.array;
} else if (!ERROR_SET(err)) {
api_set_error(err, kErrorTypeException,
"Failed to get process children. pid=%" PRId64 " error=%d",
pid, rv);
}
goto end;
}

for (size_t i = 0; i < proc_count; i++) {
ADD(rvobj, INTEGER_OBJ(proc_list[i]));
}

end:
xfree(proc_list);
return rvobj;
}




Object nvim_get_proc(Integer pid, Error *err)
FUNC_API_SINCE(4)
{
Object rvobj = OBJECT_INIT;
rvobj.data.dictionary = (Dictionary)ARRAY_DICT_INIT;
rvobj.type = kObjectTypeDictionary;

if (pid <= 0 || pid > INT_MAX) {
api_set_error(err, kErrorTypeException, "Invalid pid: %" PRId64, pid);
return NIL;
}
#if defined(WIN32)
rvobj.data.dictionary = os_proc_info((int)pid);
if (rvobj.data.dictionary.size == 0) { 
return NIL;
}
#else

Array a = ARRAY_DICT_INIT;
ADD(a, INTEGER_OBJ(pid));
String s = cstr_to_string("return vim._os_proc_info(select(1, ...))");
Object o = nvim_exec_lua(s, a, err);
api_free_string(s);
api_free_array(a);
if (o.type == kObjectTypeArray && o.data.array.size == 0) {
return NIL; 
} else if (o.type == kObjectTypeDictionary) {
rvobj.data.dictionary = o.data.dictionary;
} else if (!ERROR_SET(err)) {
api_set_error(err, kErrorTypeException,
"Failed to get process info. pid=%" PRId64, pid);
}
#endif
return rvobj;
}















void nvim_select_popupmenu_item(Integer item, Boolean insert, Boolean finish,
Dictionary opts, Error *err)
FUNC_API_SINCE(6)
{
if (opts.size > 0) {
api_set_error(err, kErrorTypeValidation, "opts dict isn't empty");
return;
}

if (finish) {
insert = true;
}

pum_ext_select_item((int)item, insert, finish);
}


Array nvim__inspect_cell(Integer grid, Integer row, Integer col, Error *err)
{
Array ret = ARRAY_DICT_INIT;



ScreenGrid *g = &default_grid;
if (grid == pum_grid.handle) {
g = &pum_grid;
} else if (grid > 1) {
win_T *wp = get_win_by_grid_handle((handle_T)grid);
if (wp != NULL && wp->w_grid.chars != NULL) {
g = &wp->w_grid;
} else {
api_set_error(err, kErrorTypeValidation,
"No grid with the given handle");
return ret;
}
}

if (row < 0 || row >= g->Rows
|| col < 0 || col >= g->Columns) {
return ret;
}
size_t off = g->line_offset[(size_t)row] + (size_t)col;
ADD(ret, STRING_OBJ(cstr_to_string((char *)g->chars[off])));
int attr = g->attrs[off];
ADD(ret, DICTIONARY_OBJ(hl_get_attr_by_id(attr, true, err)));

if (!highlight_use_hlstate()) {
ADD(ret, ARRAY_OBJ(hl_inspect(attr)));
}
return ret;
}







void nvim__put_attr(Integer id, Integer start_row, Integer start_col,
Integer end_row, Integer end_col)
FUNC_API_LUA_ONLY
{
if (!lua_attr_active) {
return;
}
if (id == 0 || syn_get_final_id((int)id) == 0) {
return;
}
int attr = syn_id2attr((int)id);
if (attr == 0) {
return;
}
decorations_add_luahl_attr(attr, (int)start_row, (colnr_T)start_col,
(int)end_row, (colnr_T)end_col);
}
