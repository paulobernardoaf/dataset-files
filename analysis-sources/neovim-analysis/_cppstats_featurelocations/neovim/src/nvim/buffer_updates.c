


#include "nvim/buffer_updates.h"
#include "nvim/memline.h"
#include "nvim/api/private/helpers.h"
#include "nvim/msgpack_rpc/channel.h"
#include "nvim/lua/executor.h"
#include "nvim/assert.h"
#include "nvim/buffer.h"

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "buffer_updates.c.generated.h"
#endif




bool buf_updates_register(buf_T *buf, uint64_t channel_id,
BufUpdateCallbacks cb, bool send_buffer)
{

if (buf->b_ml.ml_mfp == NULL) {
return false;
}

if (channel_id == LUA_INTERNAL_CALL) {
kv_push(buf->update_callbacks, cb);
if (cb.utf_sizes) {
buf->update_need_codepoints = true;
}
return true;
}


size_t size = kv_size(buf->update_channels);
if (size) {
for (size_t i = 0; i < size; i++) {
if (kv_A(buf->update_channels, i) == channel_id) {

return true;
}
}
}


kv_push(buf->update_channels, channel_id);

if (send_buffer) {
Array args = ARRAY_DICT_INIT;
args.size = 6;
args.items = xcalloc(sizeof(Object), args.size);


args.items[0] = BUFFER_OBJ(buf->handle);
args.items[1] = INTEGER_OBJ(buf_get_changedtick(buf));

args.items[2] = INTEGER_OBJ(0);

args.items[3] = INTEGER_OBJ(-1);
Array linedata = ARRAY_DICT_INIT;



STATIC_ASSERT(SIZE_MAX >= MAXLNUM, "size_t smaller than MAXLNUM");
size_t line_count = (size_t)buf->b_ml.ml_line_count;

if (line_count >= 1) {
linedata.size = line_count;
linedata.items = xcalloc(sizeof(Object), line_count);

buf_collect_lines(buf, line_count, 1, true, &linedata, NULL);
}

args.items[4] = ARRAY_OBJ(linedata);
args.items[5] = BOOLEAN_OBJ(false);

rpc_send_event(channel_id, "nvim_buf_lines_event", args);
} else {
buf_updates_changedtick_single(buf, channel_id);
}

return true;
}

bool buf_updates_active(buf_T *buf)
{
return kv_size(buf->update_channels) || kv_size(buf->update_callbacks);
}

void buf_updates_send_end(buf_T *buf, uint64_t channelid)
{
Array args = ARRAY_DICT_INIT;
args.size = 1;
args.items = xcalloc(sizeof(Object), args.size);
args.items[0] = BUFFER_OBJ(buf->handle);
rpc_send_event(channelid, "nvim_buf_detach_event", args);
}

void buf_updates_unregister(buf_T *buf, uint64_t channelid)
{
size_t size = kv_size(buf->update_channels);
if (!size) {
return;
}



size_t j = 0;
size_t found = 0;
for (size_t i = 0; i < size; i++) {
if (kv_A(buf->update_channels, i) == channelid) {
found++;
} else {

if (i != j) {
kv_A(buf->update_channels, j) = kv_A(buf->update_channels, i);
}
j++;
}
}

if (found) {

buf->update_channels.size -= found;


buf_updates_send_end(buf, channelid);

if (found == size) {
kv_destroy(buf->update_channels);
kv_init(buf->update_channels);
}
}
}

void buf_updates_unregister_all(buf_T *buf)
{
size_t size = kv_size(buf->update_channels);
if (size) {
for (size_t i = 0; i < size; i++) {
buf_updates_send_end(buf, kv_A(buf->update_channels, i));
}
kv_destroy(buf->update_channels);
kv_init(buf->update_channels);
}

for (size_t i = 0; i < kv_size(buf->update_callbacks); i++) {
BufUpdateCallbacks cb = kv_A(buf->update_callbacks, i);
if (cb.on_detach != LUA_NOREF) {
Array args = ARRAY_DICT_INIT;
Object items[1];
args.size = 1;
args.items = items;


args.items[0] = BUFFER_OBJ(buf->handle);

textlock++;
executor_exec_lua_cb(cb.on_detach, "detach", args, false, NULL);
textlock--;
}
free_update_callbacks(cb);
}
kv_destroy(buf->update_callbacks);
kv_init(buf->update_callbacks);
}

void buf_updates_send_changes(buf_T *buf,
linenr_T firstline,
int64_t num_added,
int64_t num_removed,
bool send_tick)
{
size_t deleted_codepoints, deleted_codeunits;
size_t deleted_bytes = ml_flush_deleted_bytes(buf, &deleted_codepoints,
&deleted_codeunits);

if (!buf_updates_active(buf)) {
return;
}


uint64_t badchannelid = 0;


for (size_t i = 0; i < kv_size(buf->update_channels); i++) {
uint64_t channelid = kv_A(buf->update_channels, i);


Array args = ARRAY_DICT_INIT;
args.size = 6;
args.items = xcalloc(sizeof(Object), args.size);


args.items[0] = BUFFER_OBJ(buf->handle);


args.items[1] = send_tick ? INTEGER_OBJ(buf_get_changedtick(buf)) : NIL;


args.items[2] = INTEGER_OBJ(firstline - 1);


args.items[3] = INTEGER_OBJ(firstline - 1 + num_removed);


Array linedata = ARRAY_DICT_INIT;
if (num_added > 0) {
STATIC_ASSERT(SIZE_MAX >= MAXLNUM, "size_t smaller than MAXLNUM");
linedata.size = (size_t)num_added;
linedata.items = xcalloc(sizeof(Object), (size_t)num_added);
buf_collect_lines(buf, (size_t)num_added, firstline, true, &linedata,
NULL);
}
args.items[4] = ARRAY_OBJ(linedata);
args.items[5] = BOOLEAN_OBJ(false);
if (!rpc_send_event(channelid, "nvim_buf_lines_event", args)) {



badchannelid = channelid;
}
}




if (badchannelid != 0) {
ELOG("Disabling buffer updates for dead channel %"PRIu64, badchannelid);
buf_updates_unregister(buf, badchannelid);
}


size_t j = 0;
for (size_t i = 0; i < kv_size(buf->update_callbacks); i++) {
BufUpdateCallbacks cb = kv_A(buf->update_callbacks, i);
bool keep = true;
if (cb.on_lines != LUA_NOREF) {
Array args = ARRAY_DICT_INIT;
Object items[8];
args.size = 6; 
args.items = items;


args.items[0] = BUFFER_OBJ(buf->handle);


args.items[1] = send_tick ? INTEGER_OBJ(buf_get_changedtick(buf)) : NIL;


args.items[2] = INTEGER_OBJ(firstline - 1);


args.items[3] = INTEGER_OBJ(firstline - 1 + num_removed);


args.items[4] = INTEGER_OBJ(firstline - 1 + num_added);


args.items[5] = INTEGER_OBJ((Integer)deleted_bytes);
if (cb.utf_sizes) {
args.size = 8;
args.items[6] = INTEGER_OBJ((Integer)deleted_codepoints);
args.items[7] = INTEGER_OBJ((Integer)deleted_codeunits);
}
textlock++;
Object res = executor_exec_lua_cb(cb.on_lines, "lines", args, true, NULL);
textlock--;

if (res.type == kObjectTypeBoolean && res.data.boolean == true) {
free_update_callbacks(cb);
keep = false;
}
api_free_object(res);
}
if (keep) {
kv_A(buf->update_callbacks, j++) = kv_A(buf->update_callbacks, i);
}
}
kv_size(buf->update_callbacks) = j;
}

void buf_updates_send_splice(buf_T *buf,
linenr_T start_line, colnr_T start_col,
linenr_T oldextent_line, colnr_T oldextent_col,
linenr_T newextent_line, colnr_T newextent_col)
{
if (!buf_updates_active(buf)) {
return;
}


size_t j = 0;
for (size_t i = 0; i < kv_size(buf->update_callbacks); i++) {
BufUpdateCallbacks cb = kv_A(buf->update_callbacks, i);
bool keep = true;
if (cb.on_bytes != LUA_NOREF) {
FIXED_TEMP_ARRAY(args, 8);


args.items[0] = BUFFER_OBJ(buf->handle);


args.items[1] = INTEGER_OBJ(buf_get_changedtick(buf));

args.items[2] = INTEGER_OBJ(start_line);
args.items[3] = INTEGER_OBJ(start_col);
args.items[4] = INTEGER_OBJ(oldextent_line);
args.items[5] = INTEGER_OBJ(oldextent_col);
args.items[6] = INTEGER_OBJ(newextent_line);
args.items[7] = INTEGER_OBJ(newextent_col);

textlock++;
Object res = executor_exec_lua_cb(cb.on_bytes, "bytes", args, true, NULL);
textlock--;

if (res.type == kObjectTypeBoolean && res.data.boolean == true) {
free_update_callbacks(cb);
keep = false;
}
}
if (keep) {
kv_A(buf->update_callbacks, j++) = kv_A(buf->update_callbacks, i);
}
}
kv_size(buf->update_callbacks) = j;
}
void buf_updates_changedtick(buf_T *buf)
{

for (size_t i = 0; i < kv_size(buf->update_channels); i++) {
uint64_t channel_id = kv_A(buf->update_channels, i);
buf_updates_changedtick_single(buf, channel_id);
}
size_t j = 0;
for (size_t i = 0; i < kv_size(buf->update_callbacks); i++) {
BufUpdateCallbacks cb = kv_A(buf->update_callbacks, i);
bool keep = true;
if (cb.on_changedtick != LUA_NOREF) {
FIXED_TEMP_ARRAY(args, 2);


args.items[0] = BUFFER_OBJ(buf->handle);


args.items[1] = INTEGER_OBJ(buf_get_changedtick(buf));

textlock++;
Object res = executor_exec_lua_cb(cb.on_changedtick, "changedtick",
args, true, NULL);
textlock--;

if (res.type == kObjectTypeBoolean && res.data.boolean == true) {
free_update_callbacks(cb);
keep = false;
}
api_free_object(res);
}
if (keep) {
kv_A(buf->update_callbacks, j++) = kv_A(buf->update_callbacks, i);
}
}
kv_size(buf->update_callbacks) = j;
}

void buf_updates_changedtick_single(buf_T *buf, uint64_t channel_id)
{
Array args = ARRAY_DICT_INIT;
args.size = 2;
args.items = xcalloc(sizeof(Object), args.size);


args.items[0] = BUFFER_OBJ(buf->handle);


args.items[1] = INTEGER_OBJ(buf_get_changedtick(buf));


rpc_send_event(channel_id, "nvim_buf_changedtick_event", args);
}

static void free_update_callbacks(BufUpdateCallbacks cb)
{
executor_free_luaref(cb.on_lines);
executor_free_luaref(cb.on_changedtick);
}
