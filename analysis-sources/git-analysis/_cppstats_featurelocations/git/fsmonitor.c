#include "cache.h"
#include "config.h"
#include "dir.h"
#include "ewah/ewok.h"
#include "fsmonitor.h"
#include "run-command.h"
#include "strbuf.h"

#define INDEX_EXTENSION_VERSION1 (1)
#define INDEX_EXTENSION_VERSION2 (2)
#define HOOK_INTERFACE_VERSION1 (1)
#define HOOK_INTERFACE_VERSION2 (2)

struct trace_key trace_fsmonitor = TRACE_KEY_INIT(FSMONITOR);

static void fsmonitor_ewah_callback(size_t pos, void *is)
{
struct index_state *istate = (struct index_state *)is;
struct cache_entry *ce;

if (pos >= istate->cache_nr)
BUG("fsmonitor_dirty has more entries than the index (%"PRIuMAX" >= %u)",
(uintmax_t)pos, istate->cache_nr);

ce = istate->cache[pos];
ce->ce_flags &= ~CE_FSMONITOR_VALID;
}

static int fsmonitor_hook_version(void)
{
int hook_version;

if (git_config_get_int("core.fsmonitorhookversion", &hook_version))
return -1;

if (hook_version == HOOK_INTERFACE_VERSION1 ||
hook_version == HOOK_INTERFACE_VERSION2)
return hook_version;

warning("Invalid hook version '%i' in core.fsmonitorhookversion. "
"Must be 1 or 2.", hook_version);
return -1;
}

int read_fsmonitor_extension(struct index_state *istate, const void *data,
unsigned long sz)
{
const char *index = data;
uint32_t hdr_version;
uint32_t ewah_size;
struct ewah_bitmap *fsmonitor_dirty;
int ret;
uint64_t timestamp;
struct strbuf last_update = STRBUF_INIT;

if (sz < sizeof(uint32_t) + 1 + sizeof(uint32_t))
return error("corrupt fsmonitor extension (too short)");

hdr_version = get_be32(index);
index += sizeof(uint32_t);
if (hdr_version == INDEX_EXTENSION_VERSION1) {
timestamp = get_be64(index);
strbuf_addf(&last_update, "%"PRIu64"", timestamp);
index += sizeof(uint64_t);
} else if (hdr_version == INDEX_EXTENSION_VERSION2) {
strbuf_addstr(&last_update, index);
index += last_update.len + 1;
} else {
return error("bad fsmonitor version %d", hdr_version);
}

istate->fsmonitor_last_update = strbuf_detach(&last_update, NULL);

ewah_size = get_be32(index);
index += sizeof(uint32_t);

fsmonitor_dirty = ewah_new();
ret = ewah_read_mmap(fsmonitor_dirty, index, ewah_size);
if (ret != ewah_size) {
ewah_free(fsmonitor_dirty);
return error("failed to parse ewah bitmap reading fsmonitor index extension");
}
istate->fsmonitor_dirty = fsmonitor_dirty;

if (!istate->split_index &&
istate->fsmonitor_dirty->bit_size > istate->cache_nr)
BUG("fsmonitor_dirty has more entries than the index (%"PRIuMAX" > %u)",
(uintmax_t)istate->fsmonitor_dirty->bit_size, istate->cache_nr);

trace_printf_key(&trace_fsmonitor, "read fsmonitor extension successful");
return 0;
}

void fill_fsmonitor_bitmap(struct index_state *istate)
{
unsigned int i, skipped = 0;
istate->fsmonitor_dirty = ewah_new();
for (i = 0; i < istate->cache_nr; i++) {
if (istate->cache[i]->ce_flags & CE_REMOVE)
skipped++;
else if (!(istate->cache[i]->ce_flags & CE_FSMONITOR_VALID))
ewah_set(istate->fsmonitor_dirty, i - skipped);
}
}

void write_fsmonitor_extension(struct strbuf *sb, struct index_state *istate)
{
uint32_t hdr_version;
uint32_t ewah_start;
uint32_t ewah_size = 0;
int fixup = 0;

if (!istate->split_index &&
istate->fsmonitor_dirty->bit_size > istate->cache_nr)
BUG("fsmonitor_dirty has more entries than the index (%"PRIuMAX" > %u)",
(uintmax_t)istate->fsmonitor_dirty->bit_size, istate->cache_nr);

put_be32(&hdr_version, INDEX_EXTENSION_VERSION2);
strbuf_add(sb, &hdr_version, sizeof(uint32_t));

strbuf_addstr(sb, istate->fsmonitor_last_update);
strbuf_addch(sb, 0); 

fixup = sb->len;
strbuf_add(sb, &ewah_size, sizeof(uint32_t)); 

ewah_start = sb->len;
ewah_serialize_strbuf(istate->fsmonitor_dirty, sb);
ewah_free(istate->fsmonitor_dirty);
istate->fsmonitor_dirty = NULL;


put_be32(&ewah_size, sb->len - ewah_start);
memcpy(sb->buf + fixup, &ewah_size, sizeof(uint32_t));

trace_printf_key(&trace_fsmonitor, "write fsmonitor extension successful");
}




static int query_fsmonitor(int version, const char *last_update, struct strbuf *query_result)
{
struct child_process cp = CHILD_PROCESS_INIT;

if (!core_fsmonitor)
return -1;

argv_array_push(&cp.args, core_fsmonitor);
argv_array_pushf(&cp.args, "%d", version);
argv_array_pushf(&cp.args, "%s", last_update);
cp.use_shell = 1;
cp.dir = get_git_work_tree();

return capture_command(&cp, query_result, 1024);
}

static void fsmonitor_refresh_callback(struct index_state *istate, const char *name)
{
int pos = index_name_pos(istate, name, strlen(name));

if (pos >= 0) {
struct cache_entry *ce = istate->cache[pos];
ce->ce_flags &= ~CE_FSMONITOR_VALID;
}





trace_printf_key(&trace_fsmonitor, "fsmonitor_refresh_callback '%s'", name);
untracked_cache_invalidate_path(istate, name, 0);
}

void refresh_fsmonitor(struct index_state *istate)
{
struct strbuf query_result = STRBUF_INIT;
int query_success = 0, hook_version = -1;
size_t bol = 0; 
uint64_t last_update;
struct strbuf last_update_token = STRBUF_INIT;
char *buf;
unsigned int i;

if (!core_fsmonitor || istate->fsmonitor_has_run_once)
return;

hook_version = fsmonitor_hook_version();

istate->fsmonitor_has_run_once = 1;

trace_printf_key(&trace_fsmonitor, "refresh fsmonitor");




last_update = getnanotime();
if (hook_version == HOOK_INTERFACE_VERSION1)
strbuf_addf(&last_update_token, "%"PRIu64"", last_update);






if (istate->fsmonitor_last_update) {
if (hook_version == -1 || hook_version == HOOK_INTERFACE_VERSION2) {
query_success = !query_fsmonitor(HOOK_INTERFACE_VERSION2,
istate->fsmonitor_last_update, &query_result);

if (query_success) {
if (hook_version < 0)
hook_version = HOOK_INTERFACE_VERSION2;








buf = query_result.buf;
strbuf_addstr(&last_update_token, buf);
if (!last_update_token.len) {
warning("Empty last update token.");
query_success = 0;
} else {
bol = last_update_token.len + 1;
}
} else if (hook_version < 0) {
hook_version = HOOK_INTERFACE_VERSION1;
if (!last_update_token.len)
strbuf_addf(&last_update_token, "%"PRIu64"", last_update);
}
}

if (hook_version == HOOK_INTERFACE_VERSION1) {
query_success = !query_fsmonitor(HOOK_INTERFACE_VERSION1,
istate->fsmonitor_last_update, &query_result);
}

trace_performance_since(last_update, "fsmonitor process '%s'", core_fsmonitor);
trace_printf_key(&trace_fsmonitor, "fsmonitor process '%s' returned %s",
core_fsmonitor, query_success ? "success" : "failure");
}


if (query_success && query_result.buf[bol] != '/') {

buf = query_result.buf;
for (i = bol; i < query_result.len; i++) {
if (buf[i] != '\0')
continue;
fsmonitor_refresh_callback(istate, buf + bol);
bol = i + 1;
}
if (bol < query_result.len)
fsmonitor_refresh_callback(istate, buf + bol);


if (istate->untracked)
istate->untracked->use_fsmonitor = 1;
} else {



int is_cache_changed = 0;

for (i = 0; i < istate->cache_nr; i++) {
if (istate->cache[i]->ce_flags & CE_FSMONITOR_VALID) {
is_cache_changed = 1;
istate->cache[i]->ce_flags &= ~CE_FSMONITOR_VALID;
}
}


if (is_cache_changed)
istate->cache_changed |= FSMONITOR_CHANGED;

if (istate->untracked)
istate->untracked->use_fsmonitor = 0;
}
strbuf_release(&query_result);


FREE_AND_NULL(istate->fsmonitor_last_update);
istate->fsmonitor_last_update = strbuf_detach(&last_update_token, NULL);
}

void add_fsmonitor(struct index_state *istate)
{
unsigned int i;
struct strbuf last_update = STRBUF_INIT;

if (!istate->fsmonitor_last_update) {
trace_printf_key(&trace_fsmonitor, "add fsmonitor");
istate->cache_changed |= FSMONITOR_CHANGED;
strbuf_addf(&last_update, "%"PRIu64"", getnanotime());
istate->fsmonitor_last_update = strbuf_detach(&last_update, NULL);


for (i = 0; i < istate->cache_nr; i++)
istate->cache[i]->ce_flags &= ~CE_FSMONITOR_VALID;


if (istate->untracked) {
add_untracked_cache(istate);
istate->untracked->use_fsmonitor = 1;
}


refresh_fsmonitor(istate);
}
}

void remove_fsmonitor(struct index_state *istate)
{
if (istate->fsmonitor_last_update) {
trace_printf_key(&trace_fsmonitor, "remove fsmonitor");
istate->cache_changed |= FSMONITOR_CHANGED;
FREE_AND_NULL(istate->fsmonitor_last_update);
}
}

void tweak_fsmonitor(struct index_state *istate)
{
unsigned int i;
int fsmonitor_enabled = git_config_get_fsmonitor();

if (istate->fsmonitor_dirty) {
if (fsmonitor_enabled) {

for (i = 0; i < istate->cache_nr; i++) {
istate->cache[i]->ce_flags |= CE_FSMONITOR_VALID;
}


if (istate->fsmonitor_dirty->bit_size > istate->cache_nr)
BUG("fsmonitor_dirty has more entries than the index (%"PRIuMAX" > %u)",
(uintmax_t)istate->fsmonitor_dirty->bit_size, istate->cache_nr);
ewah_each_bit(istate->fsmonitor_dirty, fsmonitor_ewah_callback, istate);

refresh_fsmonitor(istate);
}

ewah_free(istate->fsmonitor_dirty);
istate->fsmonitor_dirty = NULL;
}

switch (fsmonitor_enabled) {
case -1: 
break;
case 0: 
remove_fsmonitor(istate);
break;
case 1: 
add_fsmonitor(istate);
break;
default: 
break;
}
}
