




#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <lauxlib.h>

#include "nvim/api/buffer.h"
#include "nvim/api/private/helpers.h"
#include "nvim/api/private/defs.h"
#include "nvim/lua/executor.h"
#include "nvim/vim.h"
#include "nvim/buffer.h"
#include "nvim/change.h"
#include "nvim/charset.h"
#include "nvim/cursor.h"
#include "nvim/getchar.h"
#include "nvim/memline.h"
#include "nvim/memory.h"
#include "nvim/misc1.h"
#include "nvim/ex_cmds.h"
#include "nvim/map_defs.h"
#include "nvim/map.h"
#include "nvim/mark.h"
#include "nvim/extmark.h"
#include "nvim/fileio.h"
#include "nvim/move.h"
#include "nvim/syntax.h"
#include "nvim/window.h"
#include "nvim/undo.h"
#include "nvim/ex_docmd.h"
#include "nvim/buffer_updates.h"

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "api/buffer.c.generated.h"
#endif





















Integer nvim_buf_line_count(Buffer buffer, Error *err)
FUNC_API_SINCE(1)
{
buf_T *buf = find_buffer_by_handle(buffer, err);

if (!buf) {
return 0;
}


if (buf->b_ml.ml_mfp == NULL) {
return 0;
}

return buf->b_ml.ml_line_count;
}













String buffer_get_line(Buffer buffer, Integer index, Error *err)
{
String rv = { .size = 0 };

index = convert_index(index);
Array slice = nvim_buf_get_lines(0, buffer, index, index+1, true, err);

if (!ERROR_SET(err) && slice.size) {
rv = slice.items[0].data.string;
}

xfree(slice.items);

return rv;
}










































Boolean nvim_buf_attach(uint64_t channel_id,
Buffer buffer,
Boolean send_buffer,
DictionaryOf(LuaRef) opts,
Error *err)
FUNC_API_SINCE(4)
{
buf_T *buf = find_buffer_by_handle(buffer, err);

if (!buf) {
return false;
}

bool is_lua = (channel_id == LUA_INTERNAL_CALL);
BufUpdateCallbacks cb = BUF_UPDATE_CALLBACKS_INIT;
for (size_t i = 0; i < opts.size; i++) {
String k = opts.items[i].key;
Object *v = &opts.items[i].value;
if (is_lua && strequal("on_lines", k.data)) {
if (v->type != kObjectTypeLuaRef) {
api_set_error(err, kErrorTypeValidation, "callback is not a function");
goto error;
}
cb.on_lines = v->data.luaref;
v->data.luaref = LUA_NOREF;
} else if (is_lua && strequal("_on_bytes", k.data)) {

if (v->type != kObjectTypeLuaRef) {
api_set_error(err, kErrorTypeValidation, "callback is not a function");
goto error;
}
cb.on_bytes = v->data.luaref;
v->data.luaref = LUA_NOREF;
} else if (is_lua && strequal("on_changedtick", k.data)) {
if (v->type != kObjectTypeLuaRef) {
api_set_error(err, kErrorTypeValidation, "callback is not a function");
goto error;
}
cb.on_changedtick = v->data.luaref;
v->data.luaref = LUA_NOREF;
} else if (is_lua && strequal("on_detach", k.data)) {
if (v->type != kObjectTypeLuaRef) {
api_set_error(err, kErrorTypeValidation, "callback is not a function");
goto error;
}
cb.on_detach = v->data.luaref;
v->data.luaref = LUA_NOREF;
} else if (is_lua && strequal("utf_sizes", k.data)) {
if (v->type != kObjectTypeBoolean) {
api_set_error(err, kErrorTypeValidation, "utf_sizes must be boolean");
goto error;
}
cb.utf_sizes = v->data.boolean;
} else {
api_set_error(err, kErrorTypeValidation, "unexpected key: %s", k.data);
goto error;
}
}

return buf_updates_register(buf, channel_id, cb, send_buffer);

error:

executor_free_luaref(cb.on_lines);
executor_free_luaref(cb.on_bytes);
executor_free_luaref(cb.on_changedtick);
executor_free_luaref(cb.on_detach);
return false;
}











Boolean nvim_buf_detach(uint64_t channel_id,
Buffer buffer,
Error *err)
FUNC_API_SINCE(4) FUNC_API_REMOTE_ONLY
{
buf_T *buf = find_buffer_by_handle(buffer, err);

if (!buf) {
return false;
}

buf_updates_unregister(buf, channel_id);
return true;
}

static void buf_clear_luahl(buf_T *buf, bool force)
{
if (buf->b_luahl || force) {
executor_free_luaref(buf->b_luahl_start);
executor_free_luaref(buf->b_luahl_window);
executor_free_luaref(buf->b_luahl_line);
executor_free_luaref(buf->b_luahl_end);
}
buf->b_luahl_start = LUA_NOREF;
buf->b_luahl_window = LUA_NOREF;
buf->b_luahl_line = LUA_NOREF;
buf->b_luahl_end = LUA_NOREF;
}










void nvim__buf_set_luahl(uint64_t channel_id, Buffer buffer,
DictionaryOf(LuaRef) opts, Error *err)
FUNC_API_LUA_ONLY
{
buf_T *buf = find_buffer_by_handle(buffer, err);

if (!buf) {
return;
}

redraw_buf_later(buf, NOT_VALID);
buf_clear_luahl(buf, false);

for (size_t i = 0; i < opts.size; i++) {
String k = opts.items[i].key;
Object *v = &opts.items[i].value;
if (strequal("on_start", k.data)) {
if (v->type != kObjectTypeLuaRef) {
api_set_error(err, kErrorTypeValidation, "callback is not a function");
goto error;
}
buf->b_luahl_start = v->data.luaref;
v->data.luaref = LUA_NOREF;
} else if (strequal("on_window", k.data)) {
if (v->type != kObjectTypeLuaRef) {
api_set_error(err, kErrorTypeValidation, "callback is not a function");
goto error;
}
buf->b_luahl_window = v->data.luaref;
v->data.luaref = LUA_NOREF;
} else if (strequal("on_line", k.data)) {
if (v->type != kObjectTypeLuaRef) {
api_set_error(err, kErrorTypeValidation, "callback is not a function");
goto error;
}
buf->b_luahl_line = v->data.luaref;
v->data.luaref = LUA_NOREF;
} else {
api_set_error(err, kErrorTypeValidation, "unexpected key: %s", k.data);
goto error;
}
}
buf->b_luahl = true;
return;
error:
buf_clear_luahl(buf, true);
buf->b_luahl = false;
}

void nvim__buf_redraw_range(Buffer buffer, Integer first, Integer last,
Error *err)
FUNC_API_LUA_ONLY
{
buf_T *buf = find_buffer_by_handle(buffer, err);
if (!buf) {
return;
}

redraw_buf_range_later(buf, (linenr_T)first+1, (linenr_T)last);
}













void buffer_set_line(Buffer buffer, Integer index, String line, Error *err)
{
Object l = STRING_OBJ(line);
Array array = { .items = &l, .size = 1 };
index = convert_index(index);
nvim_buf_set_lines(0, buffer, index, index+1, true, array, err);
}











void buffer_del_line(Buffer buffer, Integer index, Error *err)
{
Array array = ARRAY_DICT_INIT;
index = convert_index(index);
nvim_buf_set_lines(0, buffer, index, index+1, true, array, err);
}














ArrayOf(String) buffer_get_line_slice(Buffer buffer,
Integer start,
Integer end,
Boolean include_start,
Boolean include_end,
Error *err)
{
start = convert_index(start) + !include_start;
end = convert_index(end) + include_end;
return nvim_buf_get_lines(0, buffer, start , end, false, err);
}

















ArrayOf(String) nvim_buf_get_lines(uint64_t channel_id,
Buffer buffer,
Integer start,
Integer end,
Boolean strict_indexing,
Error *err)
FUNC_API_SINCE(1)
{
Array rv = ARRAY_DICT_INIT;
buf_T *buf = find_buffer_by_handle(buffer, err);

if (!buf) {
return rv;
}


if (buf->b_ml.ml_mfp == NULL) {
return rv;
}

bool oob = false;
start = normalize_index(buf, start, &oob);
end = normalize_index(buf, end, &oob);

if (strict_indexing && oob) {
api_set_error(err, kErrorTypeValidation, "Index out of bounds");
return rv;
}

if (start >= end) {

return rv;
}

rv.size = (size_t)(end - start);
rv.items = xcalloc(sizeof(Object), rv.size);

if (!buf_collect_lines(buf, rv.size, start,
(channel_id != VIML_INTERNAL_CALL), &rv, err)) {
goto end;
}

end:
if (ERROR_SET(err)) {
for (size_t i = 0; i < rv.size; i++) {
xfree(rv.items[i].data.string.data);
}

xfree(rv.items);
rv.items = NULL;
}

return rv;
}

















void buffer_set_line_slice(Buffer buffer,
Integer start,
Integer end,
Boolean include_start,
Boolean include_end,
ArrayOf(String) replacement,
Error *err)
{
start = convert_index(start) + !include_start;
end = convert_index(end) + include_end;
nvim_buf_set_lines(0, buffer, start, end, false, replacement, err);
}





















void nvim_buf_set_lines(uint64_t channel_id,
Buffer buffer,
Integer start,
Integer end,
Boolean strict_indexing,
ArrayOf(String) replacement,
Error *err)
FUNC_API_SINCE(1)
{
buf_T *buf = find_buffer_by_handle(buffer, err);

if (!buf) {
return;
}

bool oob = false;
start = normalize_index(buf, start, &oob);
end = normalize_index(buf, end, &oob);

if (strict_indexing && oob) {
api_set_error(err, kErrorTypeValidation, "Index out of bounds");
return;
}


if (start > end) {
api_set_error(err,
kErrorTypeValidation,
"Argument \"start\" is higher than \"end\"");
return;
}

for (size_t i = 0; i < replacement.size; i++) {
if (replacement.items[i].type != kObjectTypeString) {
api_set_error(err,
kErrorTypeValidation,
"All items in the replacement array must be strings");
return;
}

if (channel_id != VIML_INTERNAL_CALL) {
const String l = replacement.items[i].data.string;
if (memchr(l.data, NL, l.size)) {
api_set_error(err, kErrorTypeValidation,
"String cannot contain newlines");
return;
}
}
}

size_t new_len = replacement.size;
size_t old_len = (size_t)(end - start);
ptrdiff_t extra = 0; 
char **lines = (new_len != 0) ? xcalloc(new_len, sizeof(char *)) : NULL;

for (size_t i = 0; i < new_len; i++) {
const String l = replacement.items[i].data.string;



lines[i] = xmemdupz(l.data, l.size);
memchrsub(lines[i], NUL, NL, l.size);
}

try_start();
aco_save_T aco;
aucmd_prepbuf(&aco, (buf_T *)buf);

if (!MODIFIABLE(buf)) {
api_set_error(err, kErrorTypeException, "Buffer is not 'modifiable'");
goto end;
}

if (u_save((linenr_T)(start - 1), (linenr_T)end) == FAIL) {
api_set_error(err, kErrorTypeException, "Failed to save undo information");
goto end;
}




size_t to_delete = (new_len < old_len) ? (size_t)(old_len - new_len) : 0;
for (size_t i = 0; i < to_delete; i++) {
if (ml_delete((linenr_T)start, false) == FAIL) {
api_set_error(err, kErrorTypeException, "Failed to delete line");
goto end;
}
}

if (to_delete > 0) {
extra -= (ptrdiff_t)to_delete;
}




size_t to_replace = old_len < new_len ? old_len : new_len;
for (size_t i = 0; i < to_replace; i++) {
int64_t lnum = start + (int64_t)i;

if (lnum >= MAXLNUM) {
api_set_error(err, kErrorTypeValidation, "Index value is too high");
goto end;
}

if (ml_replace((linenr_T)lnum, (char_u *)lines[i], false) == FAIL) {
api_set_error(err, kErrorTypeException, "Failed to replace line");
goto end;
}


lines[i] = NULL;
}


for (size_t i = to_replace; i < new_len; i++) {
int64_t lnum = start + (int64_t)i - 1;

if (lnum >= MAXLNUM) {
api_set_error(err, kErrorTypeValidation, "Index value is too high");
goto end;
}

if (ml_append((linenr_T)lnum, (char_u *)lines[i], 0, false) == FAIL) {
api_set_error(err, kErrorTypeException, "Failed to insert line");
goto end;
}


xfree(lines[i]);
lines[i] = NULL;
extra++;
}





mark_adjust((linenr_T)start,
(linenr_T)(end - 1),
MAXLNUM,
(long)extra,
kExtmarkUndo);

changed_lines((linenr_T)start, 0, (linenr_T)end, (long)extra, true);
fix_cursor((linenr_T)start, (linenr_T)end, (linenr_T)extra);

end:
for (size_t i = 0; i < new_len; i++) {
xfree(lines[i]);
}

xfree(lines);
aucmd_restbuf(&aco);
try_end(err);
}















Integer nvim_buf_get_offset(Buffer buffer, Integer index, Error *err)
FUNC_API_SINCE(5)
{
buf_T *buf = find_buffer_by_handle(buffer, err);
if (!buf) {
return 0;
}


if (buf->b_ml.ml_mfp == NULL) {
return -1;
}

if (index < 0 || index > buf->b_ml.ml_line_count) {
api_set_error(err, kErrorTypeValidation, "Index out of bounds");
return 0;
}

return ml_find_line_or_offset(buf, (int)index+1, NULL, true);
}







Object nvim_buf_get_var(Buffer buffer, String name, Error *err)
FUNC_API_SINCE(1)
{
buf_T *buf = find_buffer_by_handle(buffer, err);

if (!buf) {
return (Object) OBJECT_INIT;
}

return dict_get_value(buf->b_vars, name, err);
}







Integer nvim_buf_get_changedtick(Buffer buffer, Error *err)
FUNC_API_SINCE(2)
{
const buf_T *const buf = find_buffer_by_handle(buffer, err);

if (!buf) {
return -1;
}

return buf_get_changedtick(buf);
}








ArrayOf(Dictionary) nvim_buf_get_keymap(Buffer buffer, String mode, Error *err)
FUNC_API_SINCE(3)
{
buf_T *buf = find_buffer_by_handle(buffer, err);

if (!buf) {
return (Array)ARRAY_DICT_INIT;
}

return keymap_array(mode, buf);
}






void nvim_buf_set_keymap(Buffer buffer, String mode, String lhs, String rhs,
Dictionary opts, Error *err)
FUNC_API_SINCE(6)
{
modify_keymap(buffer, false, mode, lhs, rhs, opts, err);
}






void nvim_buf_del_keymap(Buffer buffer, String mode, String lhs, Error *err)
FUNC_API_SINCE(6)
{
String rhs = { .data = "", .size = 0 };
Dictionary opts = ARRAY_DICT_INIT;
modify_keymap(buffer, true, mode, lhs, rhs, opts, err);
}








Dictionary nvim_buf_get_commands(Buffer buffer, Dictionary opts, Error *err)
FUNC_API_SINCE(4)
{
bool global = (buffer == -1);
bool builtin = false;

for (size_t i = 0; i < opts.size; i++) {
String k = opts.items[i].key;
Object v = opts.items[i].value;
if (!strequal("builtin", k.data)) {
api_set_error(err, kErrorTypeValidation, "unexpected key: %s", k.data);
return (Dictionary)ARRAY_DICT_INIT;
}
if (strequal("builtin", k.data)) {
builtin = v.data.boolean;
}
}

if (global) {
if (builtin) {
api_set_error(err, kErrorTypeValidation, "builtin=true not implemented");
return (Dictionary)ARRAY_DICT_INIT;
}
return commands_array(NULL);
}

buf_T *buf = find_buffer_by_handle(buffer, err);
if (builtin || !buf) {
return (Dictionary)ARRAY_DICT_INIT;
}
return commands_array(buf);
}







void nvim_buf_set_var(Buffer buffer, String name, Object value, Error *err)
FUNC_API_SINCE(1)
{
buf_T *buf = find_buffer_by_handle(buffer, err);

if (!buf) {
return;
}

dict_set_var(buf->b_vars, name, value, false, false, err);
}






void nvim_buf_del_var(Buffer buffer, String name, Error *err)
FUNC_API_SINCE(1)
{
buf_T *buf = find_buffer_by_handle(buffer, err);

if (!buf) {
return;
}

dict_set_var(buf->b_vars, name, NIL, true, false, err);
}













Object buffer_set_var(Buffer buffer, String name, Object value, Error *err)
{
buf_T *buf = find_buffer_by_handle(buffer, err);

if (!buf) {
return (Object) OBJECT_INIT;
}

return dict_set_var(buf->b_vars, name, value, false, true, err);
}









Object buffer_del_var(Buffer buffer, String name, Error *err)
{
buf_T *buf = find_buffer_by_handle(buffer, err);

if (!buf) {
return (Object) OBJECT_INIT;
}

return dict_set_var(buf->b_vars, name, NIL, true, true, err);
}








Object nvim_buf_get_option(Buffer buffer, String name, Error *err)
FUNC_API_SINCE(1)
{
buf_T *buf = find_buffer_by_handle(buffer, err);

if (!buf) {
return (Object) OBJECT_INIT;
}

return get_option_from(buf, SREQ_BUF, name, err);
}









void nvim_buf_set_option(uint64_t channel_id, Buffer buffer,
String name, Object value, Error *err)
FUNC_API_SINCE(1)
{
buf_T *buf = find_buffer_by_handle(buffer, err);

if (!buf) {
return;
}

set_option_to(channel_id, buf, SREQ_BUF, name, value, err);
}









Integer nvim_buf_get_number(Buffer buffer, Error *err)
FUNC_API_SINCE(1)
FUNC_API_DEPRECATED_SINCE(2)
{
Integer rv = 0;
buf_T *buf = find_buffer_by_handle(buffer, err);

if (!buf) {
return rv;
}

return buf->b_fnum;
}






String nvim_buf_get_name(Buffer buffer, Error *err)
FUNC_API_SINCE(1)
{
String rv = STRING_INIT;
buf_T *buf = find_buffer_by_handle(buffer, err);

if (!buf || buf->b_ffname == NULL) {
return rv;
}

return cstr_to_string((char *)buf->b_ffname);
}






void nvim_buf_set_name(Buffer buffer, String name, Error *err)
FUNC_API_SINCE(1)
{
buf_T *buf = find_buffer_by_handle(buffer, err);

if (!buf) {
return;
}

try_start();


aco_save_T aco;
aucmd_prepbuf(&aco, buf);
int ren_ret = rename_buffer((char_u *) name.data);
aucmd_restbuf(&aco);

if (try_end(err)) {
return;
}

if (ren_ret == FAIL) {
api_set_error(err, kErrorTypeException, "Failed to rename buffer");
}
}






Boolean nvim_buf_is_loaded(Buffer buffer)
FUNC_API_SINCE(5)
{
Error stub = ERROR_INIT;
buf_T *buf = find_buffer_by_handle(buffer, &stub);
api_clear_error(&stub);
return buf && buf->b_ml.ml_mfp != NULL;
}








Boolean nvim_buf_is_valid(Buffer buffer)
FUNC_API_SINCE(1)
{
Error stub = ERROR_INIT;
Boolean ret = find_buffer_by_handle(buffer, &stub) != NULL;
api_clear_error(&stub);
return ret;
}










void buffer_insert(Buffer buffer,
Integer lnum,
ArrayOf(String) lines,
Error *err)
{


nvim_buf_set_lines(0, buffer, lnum, lnum, true, lines, err);
}









ArrayOf(Integer, 2) nvim_buf_get_mark(Buffer buffer, String name, Error *err)
FUNC_API_SINCE(1)
{
Array rv = ARRAY_DICT_INIT;
buf_T *buf = find_buffer_by_handle(buffer, err);

if (!buf) {
return rv;
}

if (name.size != 1) {
api_set_error(err, kErrorTypeValidation,
"Mark name must be a single character");
return rv;
}

pos_T *posp;
char mark = *name.data;

try_start();
bufref_T save_buf;
switch_buffer(&save_buf, buf);
posp = getmark(mark, false);
restore_buffer(&save_buf);

if (try_end(err)) {
return rv;
}

if (posp == NULL) {
api_set_error(err, kErrorTypeValidation, "Invalid mark name");
return rv;
}

ADD(rv, INTEGER_OBJ(posp->lnum));
ADD(rv, INTEGER_OBJ(posp->col));

return rv;
}








ArrayOf(Integer) nvim_buf_get_extmark_by_id(Buffer buffer, Integer ns_id,
Integer id, Error *err)
FUNC_API_SINCE(7)
{
Array rv = ARRAY_DICT_INIT;

buf_T *buf = find_buffer_by_handle(buffer, err);

if (!buf) {
return rv;
}

if (!ns_initialized((uint64_t)ns_id)) {
api_set_error(err, kErrorTypeValidation, "Invalid ns_id");
return rv;
}

ExtmarkInfo extmark = extmark_from_id(buf, (uint64_t)ns_id, (uint64_t)id);
if (extmark.row < 0) {
return rv;
}
ADD(rv, INTEGER_OBJ((Integer)extmark.row));
ADD(rv, INTEGER_OBJ((Integer)extmark.col));
return rv;
}











































Array nvim_buf_get_extmarks(Buffer buffer, Integer ns_id, Object start,
Object end, Dictionary opts, Error *err)
FUNC_API_SINCE(7)
{
Array rv = ARRAY_DICT_INIT;

buf_T *buf = find_buffer_by_handle(buffer, err);
if (!buf) {
return rv;
}

if (!ns_initialized((uint64_t)ns_id)) {
api_set_error(err, kErrorTypeValidation, "Invalid ns_id");
return rv;
}
Integer limit = -1;

for (size_t i = 0; i < opts.size; i++) {
String k = opts.items[i].key;
Object *v = &opts.items[i].value;
if (strequal("limit", k.data)) {
if (v->type != kObjectTypeInteger) {
api_set_error(err, kErrorTypeValidation, "limit is not an integer");
return rv;
}
limit = v->data.integer;
} else {
api_set_error(err, kErrorTypeValidation, "unexpected key: %s", k.data);
return rv;
}
}

if (limit == 0) {
return rv;
} else if (limit < 0) {
limit = INT64_MAX;
}


bool reverse = false;

int l_row;
colnr_T l_col;
if (!extmark_get_index_from_obj(buf, ns_id, start, &l_row, &l_col, err)) {
return rv;
}

int u_row;
colnr_T u_col;
if (!extmark_get_index_from_obj(buf, ns_id, end, &u_row, &u_col, err)) {
return rv;
}

if (l_row > u_row || (l_row == u_row && l_col > u_col)) {
reverse = true;
}


ExtmarkArray marks = extmark_get(buf, (uint64_t)ns_id, l_row, l_col, u_row,
u_col, (int64_t)limit, reverse);

for (size_t i = 0; i < kv_size(marks); i++) {
Array mark = ARRAY_DICT_INIT;
ExtmarkInfo extmark = kv_A(marks, i);
ADD(mark, INTEGER_OBJ((Integer)extmark.mark_id));
ADD(mark, INTEGER_OBJ(extmark.row));
ADD(mark, INTEGER_OBJ(extmark.col));
ADD(rv, ARRAY_OBJ(mark));
}

kv_destroy(marks);
return rv;
}


















Integer nvim_buf_set_extmark(Buffer buffer, Integer ns_id, Integer id,
Integer line, Integer col,
Dictionary opts, Error *err)
FUNC_API_SINCE(7)
{
buf_T *buf = find_buffer_by_handle(buffer, err);
if (!buf) {
return 0;
}

if (!ns_initialized((uint64_t)ns_id)) {
api_set_error(err, kErrorTypeValidation, "Invalid ns_id");
return 0;
}

if (opts.size > 0) {
api_set_error(err, kErrorTypeValidation, "opts dict isn't empty");
return 0;
}

size_t len = 0;
if (line < 0 || line > buf->b_ml.ml_line_count) {
api_set_error(err, kErrorTypeValidation, "line value outside range");
return 0;
} else if (line < buf->b_ml.ml_line_count) {
len = STRLEN(ml_get_buf(buf, (linenr_T)line+1, false));
}

if (col == -1) {
col = (Integer)len;
} else if (col < -1 || col > (Integer)len) {
api_set_error(err, kErrorTypeValidation, "col value outside range");
return 0;
}

uint64_t id_num;
if (id >= 0) {
id_num = (uint64_t)id;
} else {
api_set_error(err, kErrorTypeValidation, "Invalid mark id");
return 0;
}

id_num = extmark_set(buf, (uint64_t)ns_id, id_num,
(int)line, (colnr_T)col, kExtmarkUndo);

return (Integer)id_num;
}








Boolean nvim_buf_del_extmark(Buffer buffer,
Integer ns_id,
Integer id,
Error *err)
FUNC_API_SINCE(7)
{
buf_T *buf = find_buffer_by_handle(buffer, err);

if (!buf) {
return false;
}
if (!ns_initialized((uint64_t)ns_id)) {
api_set_error(err, kErrorTypeValidation, "Invalid ns_id");
return false;
}

return extmark_del(buf, (uint64_t)ns_id, (uint64_t)id);
}































Integer nvim_buf_add_highlight(Buffer buffer,
Integer src_id,
String hl_group,
Integer line,
Integer col_start,
Integer col_end,
Error *err)
FUNC_API_SINCE(1)
{
buf_T *buf = find_buffer_by_handle(buffer, err);
if (!buf) {
return 0;
}

if (line < 0 || line >= MAXLNUM) {
api_set_error(err, kErrorTypeValidation, "Line number outside range");
return 0;
}
if (col_start < 0 || col_start > MAXCOL) {
api_set_error(err, kErrorTypeValidation, "Column value outside range");
return 0;
}
if (col_end < 0 || col_end > MAXCOL) {
col_end = MAXCOL;
}

uint64_t ns_id = src2ns(&src_id);

if (!(0 <= line && line < buf->b_ml.ml_line_count)) {

return src_id;
}

int hlg_id = 0;
if (hl_group.size > 0) {
hlg_id = syn_check_group((char_u *)hl_group.data, (int)hl_group.size);
} else {
return src_id;
}

int end_line = (int)line;
if (col_end == MAXCOL) {
col_end = 0;
end_line++;
}

extmark_add_decoration(buf, ns_id, hlg_id,
(int)line, (colnr_T)col_start,
end_line, (colnr_T)col_end,
VIRTTEXT_EMPTY);
return src_id;
}













void nvim_buf_clear_namespace(Buffer buffer,
Integer ns_id,
Integer line_start,
Integer line_end,
Error *err)
FUNC_API_SINCE(5)
{
buf_T *buf = find_buffer_by_handle(buffer, err);
if (!buf) {
return;
}

if (line_start < 0 || line_start >= MAXLNUM) {
api_set_error(err, kErrorTypeValidation, "Line number outside range");
return;
}
if (line_end < 0 || line_end > MAXLNUM) {
line_end = MAXLNUM;
}
extmark_clear(buf, (ns_id < 0 ? 0 : (uint64_t)ns_id),
(int)line_start, 0,
(int)line_end-1, MAXCOL);
}











void nvim_buf_clear_highlight(Buffer buffer,
Integer ns_id,
Integer line_start,
Integer line_end,
Error *err)
FUNC_API_SINCE(1)
{
nvim_buf_clear_namespace(buffer, ns_id, line_start, line_end, err);
}

static VirtText parse_virt_text(Array chunks, Error *err)
{
VirtText virt_text = KV_INITIAL_VALUE;
for (size_t i = 0; i < chunks.size; i++) {
if (chunks.items[i].type != kObjectTypeArray) {
api_set_error(err, kErrorTypeValidation, "Chunk is not an array");
goto free_exit;
}
Array chunk = chunks.items[i].data.array;
if (chunk.size == 0 || chunk.size > 2
|| chunk.items[0].type != kObjectTypeString
|| (chunk.size == 2 && chunk.items[1].type != kObjectTypeString)) {
api_set_error(err, kErrorTypeValidation,
"Chunk is not an array with one or two strings");
goto free_exit;
}

String str = chunk.items[0].data.string;
char *text = transstr(str.size > 0 ? str.data : ""); 

int hl_id = 0;
if (chunk.size == 2) {
String hl = chunk.items[1].data.string;
if (hl.size > 0) {
hl_id = syn_check_group((char_u *)hl.data, (int)hl.size);
}
}
kv_push(virt_text, ((VirtTextChunk){ .text = text, .hl_id = hl_id }));
}

return virt_text;

free_exit:
clear_virttext(&virt_text);
return virt_text;
}





























Integer nvim_buf_set_virtual_text(Buffer buffer,
Integer src_id,
Integer line,
Array chunks,
Dictionary opts,
Error *err)
FUNC_API_SINCE(5)
{
buf_T *buf = find_buffer_by_handle(buffer, err);
if (!buf) {
return 0;
}

if (line < 0 || line >= MAXLNUM) {
api_set_error(err, kErrorTypeValidation, "Line number outside range");
return 0;
}

if (opts.size > 0) {
api_set_error(err, kErrorTypeValidation, "opts dict isn't empty");
return 0;
}

uint64_t ns_id = src2ns(&src_id);

VirtText virt_text = parse_virt_text(chunks, err);
if (ERROR_SET(err)) {
return 0;
}


VirtText *existing = extmark_find_virttext(buf, (int)line, ns_id);

if (existing) {
clear_virttext(existing);
*existing = virt_text;
return src_id;
}

extmark_add_decoration(buf, ns_id, 0,
(int)line, 0, -1, -1,
virt_text);
return src_id;
}
















Array nvim_buf_get_virtual_text(Buffer buffer, Integer line, Error *err)
FUNC_API_SINCE(7)
{
Array chunks = ARRAY_DICT_INIT;

buf_T *buf = find_buffer_by_handle(buffer, err);
if (!buf) {
return chunks;
}

if (line < 0 || line >= MAXLNUM) {
api_set_error(err, kErrorTypeValidation, "Line number outside range");
return chunks;
}

VirtText *virt_text = extmark_find_virttext(buf, (int)line, 0);

if (!virt_text) {
return chunks;
}

for (size_t i = 0; i < virt_text->size; i++) {
Array chunk = ARRAY_DICT_INIT;
VirtTextChunk *vtc = &virt_text->items[i];
ADD(chunk, STRING_OBJ(cstr_to_string(vtc->text)));
if (vtc->hl_id > 0) {
ADD(chunk, STRING_OBJ(cstr_to_string(
(const char *)syn_id2name(vtc->hl_id))));
}
ADD(chunks, ARRAY_OBJ(chunk));
}

return chunks;
}

Integer nvim__buf_add_decoration(Buffer buffer, Integer ns_id, String hl_group,
Integer start_row, Integer start_col,
Integer end_row, Integer end_col,
Array virt_text,
Error *err)
{
buf_T *buf = find_buffer_by_handle(buffer, err);
if (!buf) {
return 0;
}

if (!ns_initialized((uint64_t)ns_id)) {
api_set_error(err, kErrorTypeValidation, "Invalid ns_id");
return 0;
}


if (start_row < 0 || start_row >= MAXLNUM || end_row > MAXCOL) {
api_set_error(err, kErrorTypeValidation, "Line number outside range");
return 0;
}

if (start_col < 0 || start_col > MAXCOL || end_col > MAXCOL) {
api_set_error(err, kErrorTypeValidation, "Column value outside range");
return 0;
}
if (end_row < 0 || end_col < 0) {
end_row = -1;
end_col = -1;
}

if (start_row >= buf->b_ml.ml_line_count
|| end_row >= buf->b_ml.ml_line_count) {

return 0;
}

int hlg_id = 0;
if (hl_group.size > 0) {
hlg_id = syn_check_group((char_u *)hl_group.data, (int)hl_group.size);
}

VirtText vt = parse_virt_text(virt_text, err);
if (ERROR_SET(err)) {
return 0;
}

uint64_t mark_id = extmark_add_decoration(buf, (uint64_t)ns_id, hlg_id,
(int)start_row, (colnr_T)start_col,
(int)end_row, (colnr_T)end_col, vt);
return (Integer)mark_id;
}

Dictionary nvim__buf_stats(Buffer buffer, Error *err)
{
Dictionary rv = ARRAY_DICT_INIT;

buf_T *buf = find_buffer_by_handle(buffer, err);
if (!buf) {
return rv;
}




PUT(rv, "flush_count", INTEGER_OBJ(buf->flush_count));

PUT(rv, "current_lnum", INTEGER_OBJ(buf->b_ml.ml_line_lnum));

PUT(rv, "line_dirty", BOOLEAN_OBJ(buf->b_ml.ml_flags & ML_LINE_DIRTY));


PUT(rv, "dirty_bytes", INTEGER_OBJ((Integer)buf->deleted_bytes));

u_header_T *uhp = NULL;
if (buf->b_u_curhead != NULL) {
uhp = buf->b_u_curhead;
} else if (buf->b_u_newhead) {
uhp = buf->b_u_newhead;
}
if (uhp) {
PUT(rv, "uhp_extmark_size", INTEGER_OBJ((Integer)kv_size(uhp->uh_extmark)));
}

return rv;
}



static void fix_cursor(linenr_T lo, linenr_T hi, linenr_T extra)
{
if (curwin->w_cursor.lnum >= lo) {

if (curwin->w_cursor.lnum >= hi) {
curwin->w_cursor.lnum += extra;
check_cursor_col();
} else if (extra < 0) {
curwin->w_cursor.lnum = lo;
check_cursor();
} else {
check_cursor_col();
}
changed_cline_bef_curs();
}
invalidate_botline();
}


static int64_t normalize_index(buf_T *buf, int64_t index, bool *oob)
{
int64_t line_count = buf->b_ml.ml_line_count;

index = index < 0 ? line_count + index +1 : index;


if (index > line_count) {
*oob = true;
index = line_count;
} else if (index < 0) {
*oob = true;
index = 0;
}

index++;
return index;
}

static int64_t convert_index(int64_t index)
{
return index < 0 ? index - 1 : index;
}
