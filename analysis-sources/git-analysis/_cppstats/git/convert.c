#include "cache.h"
#include "config.h"
#include "object-store.h"
#include "attr.h"
#include "run-command.h"
#include "quote.h"
#include "sigchain.h"
#include "pkt-line.h"
#include "sub-process.h"
#include "utf8.h"
#include "ll-merge.h"
#define CONVERT_STAT_BITS_TXT_LF 0x1
#define CONVERT_STAT_BITS_TXT_CRLF 0x2
#define CONVERT_STAT_BITS_BIN 0x4
enum crlf_action {
CRLF_UNDEFINED,
CRLF_BINARY,
CRLF_TEXT,
CRLF_TEXT_INPUT,
CRLF_TEXT_CRLF,
CRLF_AUTO,
CRLF_AUTO_INPUT,
CRLF_AUTO_CRLF
};
struct text_stat {
unsigned nul, lonecr, lonelf, crlf;
unsigned printable, nonprintable;
};
static void gather_stats(const char *buf, unsigned long size, struct text_stat *stats)
{
unsigned long i;
memset(stats, 0, sizeof(*stats));
for (i = 0; i < size; i++) {
unsigned char c = buf[i];
if (c == '\r') {
if (i+1 < size && buf[i+1] == '\n') {
stats->crlf++;
i++;
} else
stats->lonecr++;
continue;
}
if (c == '\n') {
stats->lonelf++;
continue;
}
if (c == 127)
stats->nonprintable++;
else if (c < 32) {
switch (c) {
case '\b': case '\t': case '\033': case '\014':
stats->printable++;
break;
case 0:
stats->nul++;
default:
stats->nonprintable++;
}
}
else
stats->printable++;
}
if (size >= 1 && buf[size-1] == '\032')
stats->nonprintable--;
}
static int convert_is_binary(const struct text_stat *stats)
{
if (stats->lonecr)
return 1;
if (stats->nul)
return 1;
if ((stats->printable >> 7) < stats->nonprintable)
return 1;
return 0;
}
static unsigned int gather_convert_stats(const char *data, unsigned long size)
{
struct text_stat stats;
int ret = 0;
if (!data || !size)
return 0;
gather_stats(data, size, &stats);
if (convert_is_binary(&stats))
ret |= CONVERT_STAT_BITS_BIN;
if (stats.crlf)
ret |= CONVERT_STAT_BITS_TXT_CRLF;
if (stats.lonelf)
ret |= CONVERT_STAT_BITS_TXT_LF;
return ret;
}
static const char *gather_convert_stats_ascii(const char *data, unsigned long size)
{
unsigned int convert_stats = gather_convert_stats(data, size);
if (convert_stats & CONVERT_STAT_BITS_BIN)
return "-text";
switch (convert_stats) {
case CONVERT_STAT_BITS_TXT_LF:
return "lf";
case CONVERT_STAT_BITS_TXT_CRLF:
return "crlf";
case CONVERT_STAT_BITS_TXT_LF | CONVERT_STAT_BITS_TXT_CRLF:
return "mixed";
default:
return "none";
}
}
const char *get_cached_convert_stats_ascii(const struct index_state *istate,
const char *path)
{
const char *ret;
unsigned long sz;
void *data = read_blob_data_from_index(istate, path, &sz);
ret = gather_convert_stats_ascii(data, sz);
free(data);
return ret;
}
const char *get_wt_convert_stats_ascii(const char *path)
{
const char *ret = "";
struct strbuf sb = STRBUF_INIT;
if (strbuf_read_file(&sb, path, 0) >= 0)
ret = gather_convert_stats_ascii(sb.buf, sb.len);
strbuf_release(&sb);
return ret;
}
static int text_eol_is_crlf(void)
{
if (auto_crlf == AUTO_CRLF_TRUE)
return 1;
else if (auto_crlf == AUTO_CRLF_INPUT)
return 0;
if (core_eol == EOL_CRLF)
return 1;
if (core_eol == EOL_UNSET && EOL_NATIVE == EOL_CRLF)
return 1;
return 0;
}
static enum eol output_eol(enum crlf_action crlf_action)
{
switch (crlf_action) {
case CRLF_BINARY:
return EOL_UNSET;
case CRLF_TEXT_CRLF:
return EOL_CRLF;
case CRLF_TEXT_INPUT:
return EOL_LF;
case CRLF_UNDEFINED:
case CRLF_AUTO_CRLF:
return EOL_CRLF;
case CRLF_AUTO_INPUT:
return EOL_LF;
case CRLF_TEXT:
case CRLF_AUTO:
return text_eol_is_crlf() ? EOL_CRLF : EOL_LF;
}
warning(_("illegal crlf_action %d"), (int)crlf_action);
return core_eol;
}
static void check_global_conv_flags_eol(const char *path, enum crlf_action crlf_action,
struct text_stat *old_stats, struct text_stat *new_stats,
int conv_flags)
{
if (old_stats->crlf && !new_stats->crlf ) {
if (conv_flags & CONV_EOL_RNDTRP_DIE)
die(_("CRLF would be replaced by LF in %s"), path);
else if (conv_flags & CONV_EOL_RNDTRP_WARN)
warning(_("CRLF will be replaced by LF in %s.\n"
"The file will have its original line"
" endings in your working directory"), path);
} else if (old_stats->lonelf && !new_stats->lonelf ) {
if (conv_flags & CONV_EOL_RNDTRP_DIE)
die(_("LF would be replaced by CRLF in %s"), path);
else if (conv_flags & CONV_EOL_RNDTRP_WARN)
warning(_("LF will be replaced by CRLF in %s.\n"
"The file will have its original line"
" endings in your working directory"), path);
}
}
static int has_crlf_in_index(const struct index_state *istate, const char *path)
{
unsigned long sz;
void *data;
const char *crp;
int has_crlf = 0;
data = read_blob_data_from_index(istate, path, &sz);
if (!data)
return 0;
crp = memchr(data, '\r', sz);
if (crp) {
unsigned int ret_stats;
ret_stats = gather_convert_stats(data, sz);
if (!(ret_stats & CONVERT_STAT_BITS_BIN) &&
(ret_stats & CONVERT_STAT_BITS_TXT_CRLF))
has_crlf = 1;
}
free(data);
return has_crlf;
}
static int will_convert_lf_to_crlf(struct text_stat *stats,
enum crlf_action crlf_action)
{
if (output_eol(crlf_action) != EOL_CRLF)
return 0;
if (!stats->lonelf)
return 0;
if (crlf_action == CRLF_AUTO || crlf_action == CRLF_AUTO_INPUT || crlf_action == CRLF_AUTO_CRLF) {
if (stats->lonecr || stats->crlf)
return 0;
if (convert_is_binary(stats))
return 0;
}
return 1;
}
static int validate_encoding(const char *path, const char *enc,
const char *data, size_t len, int die_on_error)
{
const char *stripped;
if (skip_iprefix(enc, "UTF", &stripped)) {
skip_prefix(stripped, "-", &stripped);
if (has_prohibited_utf_bom(enc, data, len)) {
const char *error_msg = _(
"BOM is prohibited in '%s' if encoded as %s");
const char *advise_msg = _(
"The file '%s' contains a byte order "
"mark (BOM). Please use UTF-%.*s as "
"working-tree-encoding.");
int stripped_len = strlen(stripped) - strlen("BE");
advise(advise_msg, path, stripped_len, stripped);
if (die_on_error)
die(error_msg, path, enc);
else {
return error(error_msg, path, enc);
}
} else if (is_missing_required_utf_bom(enc, data, len)) {
const char *error_msg = _(
"BOM is required in '%s' if encoded as %s");
const char *advise_msg = _(
"The file '%s' is missing a byte order "
"mark (BOM). Please use UTF-%sBE or UTF-%sLE "
"(depending on the byte order) as "
"working-tree-encoding.");
advise(advise_msg, path, stripped, stripped);
if (die_on_error)
die(error_msg, path, enc);
else {
return error(error_msg, path, enc);
}
}
}
return 0;
}
static void trace_encoding(const char *context, const char *path,
const char *encoding, const char *buf, size_t len)
{
static struct trace_key coe = TRACE_KEY_INIT(WORKING_TREE_ENCODING);
struct strbuf trace = STRBUF_INIT;
int i;
strbuf_addf(&trace, "%s (%s, considered %s):\n", context, path, encoding);
for (i = 0; i < len && buf; ++i) {
strbuf_addf(
&trace, "| \033[2m%2i:\033[0m %2x \033[2m%c\033[0m%c",
i,
(unsigned char) buf[i],
(buf[i] > 32 && buf[i] < 127 ? buf[i] : ' '),
((i+1) % 8 && (i+1) < len ? ' ' : '\n')
);
}
strbuf_addchars(&trace, '\n', 1);
trace_strbuf(&coe, &trace);
strbuf_release(&trace);
}
static int check_roundtrip(const char *enc_name)
{
const char *found = strcasestr(check_roundtrip_encoding, enc_name);
const char *next;
int len;
if (!found)
return 0;
next = found + strlen(enc_name);
len = strlen(check_roundtrip_encoding);
return (found && (
found == check_roundtrip_encoding || (
(isspace(found[-1]) || found[-1] == ',')
)
) && (
next == check_roundtrip_encoding + len || (
next < check_roundtrip_encoding + len &&
(isspace(next[0]) || next[0] == ',')
)
));
}
static const char *default_encoding = "UTF-8";
static int encode_to_git(const char *path, const char *src, size_t src_len,
struct strbuf *buf, const char *enc, int conv_flags)
{
char *dst;
size_t dst_len;
int die_on_error = conv_flags & CONV_WRITE_OBJECT;
if (!enc || (src && !src_len))
return 0;
if (!buf && !src)
return 1;
if (validate_encoding(path, enc, src, src_len, die_on_error))
return 0;
trace_encoding("source", path, enc, src, src_len);
dst = reencode_string_len(src, src_len, default_encoding, enc,
&dst_len);
if (!dst) {
const char* msg = _("failed to encode '%s' from %s to %s");
if (die_on_error)
die(msg, path, enc, default_encoding);
else {
error(msg, path, enc, default_encoding);
return 0;
}
}
trace_encoding("destination", path, default_encoding, dst, dst_len);
if (die_on_error && check_roundtrip(enc)) {
char *re_src;
size_t re_src_len;
re_src = reencode_string_len(dst, dst_len,
enc, default_encoding,
&re_src_len);
trace_printf("Checking roundtrip encoding for %s...\n", enc);
trace_encoding("reencoded source", path, enc,
re_src, re_src_len);
if (!re_src || src_len != re_src_len ||
memcmp(src, re_src, src_len)) {
const char* msg = _("encoding '%s' from %s to %s and "
"back is not the same");
die(msg, path, enc, default_encoding);
}
free(re_src);
}
strbuf_attach(buf, dst, dst_len, dst_len + 1);
return 1;
}
static int encode_to_worktree(const char *path, const char *src, size_t src_len,
struct strbuf *buf, const char *enc)
{
char *dst;
size_t dst_len;
if (!enc || (src && !src_len))
return 0;
dst = reencode_string_len(src, src_len, enc, default_encoding,
&dst_len);
if (!dst) {
error(_("failed to encode '%s' from %s to %s"),
path, default_encoding, enc);
return 0;
}
strbuf_attach(buf, dst, dst_len, dst_len + 1);
return 1;
}
static int crlf_to_git(const struct index_state *istate,
const char *path, const char *src, size_t len,
struct strbuf *buf,
enum crlf_action crlf_action, int conv_flags)
{
struct text_stat stats;
char *dst;
int convert_crlf_into_lf;
if (crlf_action == CRLF_BINARY ||
(src && !len))
return 0;
if (!buf && !src)
return 1;
gather_stats(src, len, &stats);
convert_crlf_into_lf = !!stats.crlf;
if (crlf_action == CRLF_AUTO || crlf_action == CRLF_AUTO_INPUT || crlf_action == CRLF_AUTO_CRLF) {
if (convert_is_binary(&stats))
return 0;
if ((!(conv_flags & CONV_EOL_RENORMALIZE)) &&
has_crlf_in_index(istate, path))
convert_crlf_into_lf = 0;
}
if (((conv_flags & CONV_EOL_RNDTRP_WARN) ||
((conv_flags & CONV_EOL_RNDTRP_DIE) && len))) {
struct text_stat new_stats;
memcpy(&new_stats, &stats, sizeof(new_stats));
if (convert_crlf_into_lf) {
new_stats.lonelf += new_stats.crlf;
new_stats.crlf = 0;
}
if (will_convert_lf_to_crlf(&new_stats, crlf_action)) {
new_stats.crlf += new_stats.lonelf;
new_stats.lonelf = 0;
}
check_global_conv_flags_eol(path, crlf_action, &stats, &new_stats, conv_flags);
}
if (!convert_crlf_into_lf)
return 0;
if (!buf)
return 1;
if (strbuf_avail(buf) + buf->len < len)
strbuf_grow(buf, len - buf->len);
dst = buf->buf;
if (crlf_action == CRLF_AUTO || crlf_action == CRLF_AUTO_INPUT || crlf_action == CRLF_AUTO_CRLF) {
do {
unsigned char c = *src++;
if (c != '\r')
*dst++ = c;
} while (--len);
} else {
do {
unsigned char c = *src++;
if (! (c == '\r' && (1 < len && *src == '\n')))
*dst++ = c;
} while (--len);
}
strbuf_setlen(buf, dst - buf->buf);
return 1;
}
static int crlf_to_worktree(const char *src, size_t len,
struct strbuf *buf, enum crlf_action crlf_action)
{
char *to_free = NULL;
struct text_stat stats;
if (!len || output_eol(crlf_action) != EOL_CRLF)
return 0;
gather_stats(src, len, &stats);
if (!will_convert_lf_to_crlf(&stats, crlf_action))
return 0;
if (src == buf->buf)
to_free = strbuf_detach(buf, NULL);
strbuf_grow(buf, len + stats.lonelf);
for (;;) {
const char *nl = memchr(src, '\n', len);
if (!nl)
break;
if (nl > src && nl[-1] == '\r') {
strbuf_add(buf, src, nl + 1 - src);
} else {
strbuf_add(buf, src, nl - src);
strbuf_addstr(buf, "\r\n");
}
len -= nl + 1 - src;
src = nl + 1;
}
strbuf_add(buf, src, len);
free(to_free);
return 1;
}
struct filter_params {
const char *src;
unsigned long size;
int fd;
const char *cmd;
const char *path;
};
static int filter_buffer_or_fd(int in, int out, void *data)
{
struct child_process child_process = CHILD_PROCESS_INIT;
struct filter_params *params = (struct filter_params *)data;
int write_err, status;
const char *argv[] = { NULL, NULL };
struct strbuf cmd = STRBUF_INIT;
struct strbuf path = STRBUF_INIT;
struct strbuf_expand_dict_entry dict[] = {
{ "f", NULL, },
{ NULL, NULL, },
};
sq_quote_buf(&path, params->path);
dict[0].value = path.buf;
strbuf_expand(&cmd, params->cmd, strbuf_expand_dict_cb, &dict);
strbuf_release(&path);
argv[0] = cmd.buf;
child_process.argv = argv;
child_process.use_shell = 1;
child_process.in = -1;
child_process.out = out;
if (start_command(&child_process)) {
strbuf_release(&cmd);
return error(_("cannot fork to run external filter '%s'"),
params->cmd);
}
sigchain_push(SIGPIPE, SIG_IGN);
if (params->src) {
write_err = (write_in_full(child_process.in,
params->src, params->size) < 0);
if (errno == EPIPE)
write_err = 0;
} else {
write_err = copy_fd(params->fd, child_process.in);
if (write_err == COPY_WRITE_ERROR && errno == EPIPE)
write_err = 0;
}
if (close(child_process.in))
write_err = 1;
if (write_err)
error(_("cannot feed the input to external filter '%s'"),
params->cmd);
sigchain_pop(SIGPIPE);
status = finish_command(&child_process);
if (status)
error(_("external filter '%s' failed %d"), params->cmd, status);
strbuf_release(&cmd);
return (write_err || status);
}
static int apply_single_file_filter(const char *path, const char *src, size_t len, int fd,
struct strbuf *dst, const char *cmd)
{
int err = 0;
struct strbuf nbuf = STRBUF_INIT;
struct async async;
struct filter_params params;
memset(&async, 0, sizeof(async));
async.proc = filter_buffer_or_fd;
async.data = &params;
async.out = -1;
params.src = src;
params.size = len;
params.fd = fd;
params.cmd = cmd;
params.path = path;
fflush(NULL);
if (start_async(&async))
return 0; 
if (strbuf_read(&nbuf, async.out, 0) < 0) {
err = error(_("read from external filter '%s' failed"), cmd);
}
if (close(async.out)) {
err = error(_("read from external filter '%s' failed"), cmd);
}
if (finish_async(&async)) {
err = error(_("external filter '%s' failed"), cmd);
}
if (!err) {
strbuf_swap(dst, &nbuf);
}
strbuf_release(&nbuf);
return !err;
}
#define CAP_CLEAN (1u<<0)
#define CAP_SMUDGE (1u<<1)
#define CAP_DELAY (1u<<2)
struct cmd2process {
struct subprocess_entry subprocess; 
unsigned int supported_capabilities;
};
static int subprocess_map_initialized;
static struct hashmap subprocess_map;
static int start_multi_file_filter_fn(struct subprocess_entry *subprocess)
{
static int versions[] = {2, 0};
static struct subprocess_capability capabilities[] = {
{ "clean", CAP_CLEAN },
{ "smudge", CAP_SMUDGE },
{ "delay", CAP_DELAY },
{ NULL, 0 }
};
struct cmd2process *entry = (struct cmd2process *)subprocess;
return subprocess_handshake(subprocess, "git-filter", versions, NULL,
capabilities,
&entry->supported_capabilities);
}
static void handle_filter_error(const struct strbuf *filter_status,
struct cmd2process *entry,
const unsigned int wanted_capability)
{
if (!strcmp(filter_status->buf, "error"))
; 
else if (!strcmp(filter_status->buf, "abort") && wanted_capability) {
entry->supported_capabilities &= ~wanted_capability;
} else {
error(_("external filter '%s' failed"), entry->subprocess.cmd);
subprocess_stop(&subprocess_map, &entry->subprocess);
free(entry);
}
}
static int apply_multi_file_filter(const char *path, const char *src, size_t len,
int fd, struct strbuf *dst, const char *cmd,
const unsigned int wanted_capability,
const struct checkout_metadata *meta,
struct delayed_checkout *dco)
{
int err;
int can_delay = 0;
struct cmd2process *entry;
struct child_process *process;
struct strbuf nbuf = STRBUF_INIT;
struct strbuf filter_status = STRBUF_INIT;
const char *filter_type;
if (!subprocess_map_initialized) {
subprocess_map_initialized = 1;
hashmap_init(&subprocess_map, cmd2process_cmp, NULL, 0);
entry = NULL;
} else {
entry = (struct cmd2process *)subprocess_find_entry(&subprocess_map, cmd);
}
fflush(NULL);
if (!entry) {
entry = xmalloc(sizeof(*entry));
entry->supported_capabilities = 0;
if (subprocess_start(&subprocess_map, &entry->subprocess, cmd, start_multi_file_filter_fn)) {
free(entry);
return 0;
}
}
process = &entry->subprocess.process;
if (!(entry->supported_capabilities & wanted_capability))
return 0;
if (wanted_capability & CAP_CLEAN)
filter_type = "clean";
else if (wanted_capability & CAP_SMUDGE)
filter_type = "smudge";
else
die(_("unexpected filter type"));
sigchain_push(SIGPIPE, SIG_IGN);
assert(strlen(filter_type) < LARGE_PACKET_DATA_MAX - strlen("command=\n"));
err = packet_write_fmt_gently(process->in, "command=%s\n", filter_type);
if (err)
goto done;
err = strlen(path) > LARGE_PACKET_DATA_MAX - strlen("pathname=\n");
if (err) {
error(_("path name too long for external filter"));
goto done;
}
err = packet_write_fmt_gently(process->in, "pathname=%s\n", path);
if (err)
goto done;
if (meta && meta->refname) {
err = packet_write_fmt_gently(process->in, "ref=%s\n", meta->refname);
if (err)
goto done;
}
if (meta && !is_null_oid(&meta->treeish)) {
err = packet_write_fmt_gently(process->in, "treeish=%s\n", oid_to_hex(&meta->treeish));
if (err)
goto done;
}
if (meta && !is_null_oid(&meta->blob)) {
err = packet_write_fmt_gently(process->in, "blob=%s\n", oid_to_hex(&meta->blob));
if (err)
goto done;
}
if ((entry->supported_capabilities & CAP_DELAY) &&
dco && dco->state == CE_CAN_DELAY) {
can_delay = 1;
err = packet_write_fmt_gently(process->in, "can-delay=1\n");
if (err)
goto done;
}
err = packet_flush_gently(process->in);
if (err)
goto done;
if (fd >= 0)
err = write_packetized_from_fd(fd, process->in);
else
err = write_packetized_from_buf(src, len, process->in);
if (err)
goto done;
err = subprocess_read_status(process->out, &filter_status);
if (err)
goto done;
if (can_delay && !strcmp(filter_status.buf, "delayed")) {
string_list_insert(&dco->filters, cmd);
string_list_insert(&dco->paths, path);
} else {
err = strcmp(filter_status.buf, "success");
if (err)
goto done;
err = read_packetized_to_strbuf(process->out, &nbuf) < 0;
if (err)
goto done;
err = subprocess_read_status(process->out, &filter_status);
if (err)
goto done;
err = strcmp(filter_status.buf, "success");
}
done:
sigchain_pop(SIGPIPE);
if (err)
handle_filter_error(&filter_status, entry, wanted_capability);
else
strbuf_swap(dst, &nbuf);
strbuf_release(&nbuf);
return !err;
}
int async_query_available_blobs(const char *cmd, struct string_list *available_paths)
{
int err;
char *line;
struct cmd2process *entry;
struct child_process *process;
struct strbuf filter_status = STRBUF_INIT;
assert(subprocess_map_initialized);
entry = (struct cmd2process *)subprocess_find_entry(&subprocess_map, cmd);
if (!entry) {
error(_("external filter '%s' is not available anymore although "
"not all paths have been filtered"), cmd);
return 0;
}
process = &entry->subprocess.process;
sigchain_push(SIGPIPE, SIG_IGN);
err = packet_write_fmt_gently(
process->in, "command=list_available_blobs\n");
if (err)
goto done;
err = packet_flush_gently(process->in);
if (err)
goto done;
while ((line = packet_read_line(process->out, NULL))) {
const char *path;
if (skip_prefix(line, "pathname=", &path))
string_list_insert(available_paths, xstrdup(path));
else
; 
}
err = subprocess_read_status(process->out, &filter_status);
if (err)
goto done;
err = strcmp(filter_status.buf, "success");
done:
sigchain_pop(SIGPIPE);
if (err)
handle_filter_error(&filter_status, entry, 0);
return !err;
}
static struct convert_driver {
const char *name;
struct convert_driver *next;
const char *smudge;
const char *clean;
const char *process;
int required;
} *user_convert, **user_convert_tail;
static int apply_filter(const char *path, const char *src, size_t len,
int fd, struct strbuf *dst, struct convert_driver *drv,
const unsigned int wanted_capability,
const struct checkout_metadata *meta,
struct delayed_checkout *dco)
{
const char *cmd = NULL;
if (!drv)
return 0;
if (!dst)
return 1;
if ((wanted_capability & CAP_CLEAN) && !drv->process && drv->clean)
cmd = drv->clean;
else if ((wanted_capability & CAP_SMUDGE) && !drv->process && drv->smudge)
cmd = drv->smudge;
if (cmd && *cmd)
return apply_single_file_filter(path, src, len, fd, dst, cmd);
else if (drv->process && *drv->process)
return apply_multi_file_filter(path, src, len, fd, dst,
drv->process, wanted_capability, meta, dco);
return 0;
}
static int read_convert_config(const char *var, const char *value, void *cb)
{
const char *key, *name;
int namelen;
struct convert_driver *drv;
if (parse_config_key(var, "filter", &name, &namelen, &key) < 0 || !name)
return 0;
for (drv = user_convert; drv; drv = drv->next)
if (!strncmp(drv->name, name, namelen) && !drv->name[namelen])
break;
if (!drv) {
drv = xcalloc(1, sizeof(struct convert_driver));
drv->name = xmemdupz(name, namelen);
*user_convert_tail = drv;
user_convert_tail = &(drv->next);
}
if (!strcmp("smudge", key))
return git_config_string(&drv->smudge, var, value);
if (!strcmp("clean", key))
return git_config_string(&drv->clean, var, value);
if (!strcmp("process", key))
return git_config_string(&drv->process, var, value);
if (!strcmp("required", key)) {
drv->required = git_config_bool(var, value);
return 0;
}
return 0;
}
static int count_ident(const char *cp, unsigned long size)
{
int cnt = 0;
char ch;
while (size) {
ch = *cp++;
size--;
if (ch != '$')
continue;
if (size < 3)
break;
if (memcmp("Id", cp, 2))
continue;
ch = cp[2];
cp += 3;
size -= 3;
if (ch == '$')
cnt++; 
if (ch != ':')
continue;
while (size) {
ch = *cp++;
size--;
if (ch == '$') {
cnt++;
break;
}
if (ch == '\n')
break;
}
}
return cnt;
}
static int ident_to_git(const char *src, size_t len,
struct strbuf *buf, int ident)
{
char *dst, *dollar;
if (!ident || (src && !count_ident(src, len)))
return 0;
if (!buf)
return 1;
if (strbuf_avail(buf) + buf->len < len)
strbuf_grow(buf, len - buf->len);
dst = buf->buf;
for (;;) {
dollar = memchr(src, '$', len);
if (!dollar)
break;
memmove(dst, src, dollar + 1 - src);
dst += dollar + 1 - src;
len -= dollar + 1 - src;
src = dollar + 1;
if (len > 3 && !memcmp(src, "Id:", 3)) {
dollar = memchr(src + 3, '$', len - 3);
if (!dollar)
break;
if (memchr(src + 3, '\n', dollar - src - 3)) {
continue;
}
memcpy(dst, "Id$", 3);
dst += 3;
len -= dollar + 1 - src;
src = dollar + 1;
}
}
memmove(dst, src, len);
strbuf_setlen(buf, dst + len - buf->buf);
return 1;
}
static int ident_to_worktree(const char *src, size_t len,
struct strbuf *buf, int ident)
{
struct object_id oid;
char *to_free = NULL, *dollar, *spc;
int cnt;
if (!ident)
return 0;
cnt = count_ident(src, len);
if (!cnt)
return 0;
if (src == buf->buf)
to_free = strbuf_detach(buf, NULL);
hash_object_file(the_hash_algo, src, len, "blob", &oid);
strbuf_grow(buf, len + cnt * (the_hash_algo->hexsz + 3));
for (;;) {
dollar = memchr(src, '$', len);
if (!dollar)
break;
strbuf_add(buf, src, dollar + 1 - src);
len -= dollar + 1 - src;
src = dollar + 1;
if (len < 3 || memcmp("Id", src, 2))
continue;
if (src[2] == '$') {
src += 3;
len -= 3;
} else if (src[2] == ':') {
dollar = memchr(src + 3, '$', len - 3);
if (!dollar) {
break;
}
if (memchr(src + 3, '\n', dollar - src - 3)) {
continue;
}
spc = memchr(src + 4, ' ', dollar - src - 4);
if (spc && spc < dollar-1) {
continue;
}
len -= dollar + 1 - src;
src = dollar + 1;
} else {
continue;
}
strbuf_addstr(buf, "Id: ");
strbuf_addstr(buf, oid_to_hex(&oid));
strbuf_addstr(buf, " $");
}
strbuf_add(buf, src, len);
free(to_free);
return 1;
}
static const char *git_path_check_encoding(struct attr_check_item *check)
{
const char *value = check->value;
if (ATTR_UNSET(value) || !strlen(value))
return NULL;
if (ATTR_TRUE(value) || ATTR_FALSE(value)) {
die(_("true/false are no valid working-tree-encodings"));
}
if (same_encoding(value, default_encoding))
return NULL;
return value;
}
static enum crlf_action git_path_check_crlf(struct attr_check_item *check)
{
const char *value = check->value;
if (ATTR_TRUE(value))
return CRLF_TEXT;
else if (ATTR_FALSE(value))
return CRLF_BINARY;
else if (ATTR_UNSET(value))
;
else if (!strcmp(value, "input"))
return CRLF_TEXT_INPUT;
else if (!strcmp(value, "auto"))
return CRLF_AUTO;
return CRLF_UNDEFINED;
}
static enum eol git_path_check_eol(struct attr_check_item *check)
{
const char *value = check->value;
if (ATTR_UNSET(value))
;
else if (!strcmp(value, "lf"))
return EOL_LF;
else if (!strcmp(value, "crlf"))
return EOL_CRLF;
return EOL_UNSET;
}
static struct convert_driver *git_path_check_convert(struct attr_check_item *check)
{
const char *value = check->value;
struct convert_driver *drv;
if (ATTR_TRUE(value) || ATTR_FALSE(value) || ATTR_UNSET(value))
return NULL;
for (drv = user_convert; drv; drv = drv->next)
if (!strcmp(value, drv->name))
return drv;
return NULL;
}
static int git_path_check_ident(struct attr_check_item *check)
{
const char *value = check->value;
return !!ATTR_TRUE(value);
}
struct conv_attrs {
struct convert_driver *drv;
enum crlf_action attr_action; 
enum crlf_action crlf_action; 
int ident;
const char *working_tree_encoding; 
};
static struct attr_check *check;
static void convert_attrs(const struct index_state *istate,
struct conv_attrs *ca, const char *path)
{
struct attr_check_item *ccheck = NULL;
if (!check) {
check = attr_check_initl("crlf", "ident", "filter",
"eol", "text", "working-tree-encoding",
NULL);
user_convert_tail = &user_convert;
git_config(read_convert_config, NULL);
}
git_check_attr(istate, path, check);
ccheck = check->items;
ca->crlf_action = git_path_check_crlf(ccheck + 4);
if (ca->crlf_action == CRLF_UNDEFINED)
ca->crlf_action = git_path_check_crlf(ccheck + 0);
ca->ident = git_path_check_ident(ccheck + 1);
ca->drv = git_path_check_convert(ccheck + 2);
if (ca->crlf_action != CRLF_BINARY) {
enum eol eol_attr = git_path_check_eol(ccheck + 3);
if (ca->crlf_action == CRLF_AUTO && eol_attr == EOL_LF)
ca->crlf_action = CRLF_AUTO_INPUT;
else if (ca->crlf_action == CRLF_AUTO && eol_attr == EOL_CRLF)
ca->crlf_action = CRLF_AUTO_CRLF;
else if (eol_attr == EOL_LF)
ca->crlf_action = CRLF_TEXT_INPUT;
else if (eol_attr == EOL_CRLF)
ca->crlf_action = CRLF_TEXT_CRLF;
}
ca->working_tree_encoding = git_path_check_encoding(ccheck + 5);
ca->attr_action = ca->crlf_action;
if (ca->crlf_action == CRLF_TEXT)
ca->crlf_action = text_eol_is_crlf() ? CRLF_TEXT_CRLF : CRLF_TEXT_INPUT;
if (ca->crlf_action == CRLF_UNDEFINED && auto_crlf == AUTO_CRLF_FALSE)
ca->crlf_action = CRLF_BINARY;
if (ca->crlf_action == CRLF_UNDEFINED && auto_crlf == AUTO_CRLF_TRUE)
ca->crlf_action = CRLF_AUTO_CRLF;
if (ca->crlf_action == CRLF_UNDEFINED && auto_crlf == AUTO_CRLF_INPUT)
ca->crlf_action = CRLF_AUTO_INPUT;
}
void reset_parsed_attributes(void)
{
struct convert_driver *drv, *next;
attr_check_free(check);
check = NULL;
reset_merge_attributes();
for (drv = user_convert; drv; drv = next) {
next = drv->next;
free((void *)drv->name);
free(drv);
}
user_convert = NULL;
user_convert_tail = NULL;
}
int would_convert_to_git_filter_fd(const struct index_state *istate, const char *path)
{
struct conv_attrs ca;
convert_attrs(istate, &ca, path);
if (!ca.drv)
return 0;
if (!ca.drv->required)
return 0;
return apply_filter(path, NULL, 0, -1, NULL, ca.drv, CAP_CLEAN, NULL, NULL);
}
const char *get_convert_attr_ascii(const struct index_state *istate, const char *path)
{
struct conv_attrs ca;
convert_attrs(istate, &ca, path);
switch (ca.attr_action) {
case CRLF_UNDEFINED:
return "";
case CRLF_BINARY:
return "-text";
case CRLF_TEXT:
return "text";
case CRLF_TEXT_INPUT:
return "text eol=lf";
case CRLF_TEXT_CRLF:
return "text eol=crlf";
case CRLF_AUTO:
return "text=auto";
case CRLF_AUTO_CRLF:
return "text=auto eol=crlf";
case CRLF_AUTO_INPUT:
return "text=auto eol=lf";
}
return "";
}
int convert_to_git(const struct index_state *istate,
const char *path, const char *src, size_t len,
struct strbuf *dst, int conv_flags)
{
int ret = 0;
struct conv_attrs ca;
convert_attrs(istate, &ca, path);
ret |= apply_filter(path, src, len, -1, dst, ca.drv, CAP_CLEAN, NULL, NULL);
if (!ret && ca.drv && ca.drv->required)
die(_("%s: clean filter '%s' failed"), path, ca.drv->name);
if (ret && dst) {
src = dst->buf;
len = dst->len;
}
ret |= encode_to_git(path, src, len, dst, ca.working_tree_encoding, conv_flags);
if (ret && dst) {
src = dst->buf;
len = dst->len;
}
if (!(conv_flags & CONV_EOL_KEEP_CRLF)) {
ret |= crlf_to_git(istate, path, src, len, dst, ca.crlf_action, conv_flags);
if (ret && dst) {
src = dst->buf;
len = dst->len;
}
}
return ret | ident_to_git(src, len, dst, ca.ident);
}
void convert_to_git_filter_fd(const struct index_state *istate,
const char *path, int fd, struct strbuf *dst,
int conv_flags)
{
struct conv_attrs ca;
convert_attrs(istate, &ca, path);
assert(ca.drv);
assert(ca.drv->clean || ca.drv->process);
if (!apply_filter(path, NULL, 0, fd, dst, ca.drv, CAP_CLEAN, NULL, NULL))
die(_("%s: clean filter '%s' failed"), path, ca.drv->name);
encode_to_git(path, dst->buf, dst->len, dst, ca.working_tree_encoding, conv_flags);
crlf_to_git(istate, path, dst->buf, dst->len, dst, ca.crlf_action, conv_flags);
ident_to_git(dst->buf, dst->len, dst, ca.ident);
}
static int convert_to_working_tree_internal(const struct index_state *istate,
const char *path, const char *src,
size_t len, struct strbuf *dst,
int normalizing,
const struct checkout_metadata *meta,
struct delayed_checkout *dco)
{
int ret = 0, ret_filter = 0;
struct conv_attrs ca;
convert_attrs(istate, &ca, path);
ret |= ident_to_worktree(src, len, dst, ca.ident);
if (ret) {
src = dst->buf;
len = dst->len;
}
if ((ca.drv && (ca.drv->smudge || ca.drv->process)) || !normalizing) {
ret |= crlf_to_worktree(src, len, dst, ca.crlf_action);
if (ret) {
src = dst->buf;
len = dst->len;
}
}
ret |= encode_to_worktree(path, src, len, dst, ca.working_tree_encoding);
if (ret) {
src = dst->buf;
len = dst->len;
}
ret_filter = apply_filter(
path, src, len, -1, dst, ca.drv, CAP_SMUDGE, meta, dco);
if (!ret_filter && ca.drv && ca.drv->required)
die(_("%s: smudge filter %s failed"), path, ca.drv->name);
return ret | ret_filter;
}
int async_convert_to_working_tree(const struct index_state *istate,
const char *path, const char *src,
size_t len, struct strbuf *dst,
const struct checkout_metadata *meta,
void *dco)
{
return convert_to_working_tree_internal(istate, path, src, len, dst, 0, meta, dco);
}
int convert_to_working_tree(const struct index_state *istate,
const char *path, const char *src,
size_t len, struct strbuf *dst,
const struct checkout_metadata *meta)
{
return convert_to_working_tree_internal(istate, path, src, len, dst, 0, meta, NULL);
}
int renormalize_buffer(const struct index_state *istate, const char *path,
const char *src, size_t len, struct strbuf *dst)
{
int ret = convert_to_working_tree_internal(istate, path, src, len, dst, 1, NULL, NULL);
if (ret) {
src = dst->buf;
len = dst->len;
}
return ret | convert_to_git(istate, path, src, len, dst, CONV_EOL_RENORMALIZE);
}
typedef int (*filter_fn)(struct stream_filter *,
const char *input, size_t *isize_p,
char *output, size_t *osize_p);
typedef void (*free_fn)(struct stream_filter *);
struct stream_filter_vtbl {
filter_fn filter;
free_fn free;
};
struct stream_filter {
struct stream_filter_vtbl *vtbl;
};
static int null_filter_fn(struct stream_filter *filter,
const char *input, size_t *isize_p,
char *output, size_t *osize_p)
{
size_t count;
if (!input)
return 0; 
count = *isize_p;
if (*osize_p < count)
count = *osize_p;
if (count) {
memmove(output, input, count);
*isize_p -= count;
*osize_p -= count;
}
return 0;
}
static void null_free_fn(struct stream_filter *filter)
{
; 
}
static struct stream_filter_vtbl null_vtbl = {
null_filter_fn,
null_free_fn,
};
static struct stream_filter null_filter_singleton = {
&null_vtbl,
};
int is_null_stream_filter(struct stream_filter *filter)
{
return filter == &null_filter_singleton;
}
struct lf_to_crlf_filter {
struct stream_filter filter;
unsigned has_held:1;
char held;
};
static int lf_to_crlf_filter_fn(struct stream_filter *filter,
const char *input, size_t *isize_p,
char *output, size_t *osize_p)
{
size_t count, o = 0;
struct lf_to_crlf_filter *lf_to_crlf = (struct lf_to_crlf_filter *)filter;
if (lf_to_crlf->has_held && (lf_to_crlf->held != '\r' || !input)) {
output[o++] = lf_to_crlf->held;
lf_to_crlf->has_held = 0;
}
if (!input) {
*osize_p -= o;
return 0;
}
count = *isize_p;
if (count || lf_to_crlf->has_held) {
size_t i;
int was_cr = 0;
if (lf_to_crlf->has_held) {
was_cr = 1;
lf_to_crlf->has_held = 0;
}
for (i = 0; o < *osize_p && i < count; i++) {
char ch = input[i];
if (ch == '\n') {
output[o++] = '\r';
} else if (was_cr) {
output[o++] = '\r';
}
if (*osize_p <= o) {
lf_to_crlf->has_held = 1;
lf_to_crlf->held = ch;
continue; 
}
if (ch == '\r') {
was_cr = 1;
continue;
}
was_cr = 0;
output[o++] = ch;
}
*osize_p -= o;
*isize_p -= i;
if (!lf_to_crlf->has_held && was_cr) {
lf_to_crlf->has_held = 1;
lf_to_crlf->held = '\r';
}
}
return 0;
}
static void lf_to_crlf_free_fn(struct stream_filter *filter)
{
free(filter);
}
static struct stream_filter_vtbl lf_to_crlf_vtbl = {
lf_to_crlf_filter_fn,
lf_to_crlf_free_fn,
};
static struct stream_filter *lf_to_crlf_filter(void)
{
struct lf_to_crlf_filter *lf_to_crlf = xcalloc(1, sizeof(*lf_to_crlf));
lf_to_crlf->filter.vtbl = &lf_to_crlf_vtbl;
return (struct stream_filter *)lf_to_crlf;
}
#define FILTER_BUFFER 1024
struct cascade_filter {
struct stream_filter filter;
struct stream_filter *one;
struct stream_filter *two;
char buf[FILTER_BUFFER];
int end, ptr;
};
static int cascade_filter_fn(struct stream_filter *filter,
const char *input, size_t *isize_p,
char *output, size_t *osize_p)
{
struct cascade_filter *cas = (struct cascade_filter *) filter;
size_t filled = 0;
size_t sz = *osize_p;
size_t to_feed, remaining;
while (filled < sz) {
remaining = sz - filled;
if (cas->ptr < cas->end) {
to_feed = cas->end - cas->ptr;
if (stream_filter(cas->two,
cas->buf + cas->ptr, &to_feed,
output + filled, &remaining))
return -1;
cas->ptr += (cas->end - cas->ptr) - to_feed;
filled = sz - remaining;
continue;
}
to_feed = input ? *isize_p : 0;
if (input && !to_feed)
break;
remaining = sizeof(cas->buf);
if (stream_filter(cas->one,
input, &to_feed,
cas->buf, &remaining))
return -1;
cas->end = sizeof(cas->buf) - remaining;
cas->ptr = 0;
if (input) {
size_t fed = *isize_p - to_feed;
*isize_p -= fed;
input += fed;
}
if (input || cas->end)
continue;
to_feed = 0;
remaining = sz - filled;
if (stream_filter(cas->two,
NULL, &to_feed,
output + filled, &remaining))
return -1;
if (remaining == (sz - filled))
break; 
filled = sz - remaining;
}
*osize_p -= filled;
return 0;
}
static void cascade_free_fn(struct stream_filter *filter)
{
struct cascade_filter *cas = (struct cascade_filter *)filter;
free_stream_filter(cas->one);
free_stream_filter(cas->two);
free(filter);
}
static struct stream_filter_vtbl cascade_vtbl = {
cascade_filter_fn,
cascade_free_fn,
};
static struct stream_filter *cascade_filter(struct stream_filter *one,
struct stream_filter *two)
{
struct cascade_filter *cascade;
if (!one || is_null_stream_filter(one))
return two;
if (!two || is_null_stream_filter(two))
return one;
cascade = xmalloc(sizeof(*cascade));
cascade->one = one;
cascade->two = two;
cascade->end = cascade->ptr = 0;
cascade->filter.vtbl = &cascade_vtbl;
return (struct stream_filter *)cascade;
}
#define IDENT_DRAINING (-1)
#define IDENT_SKIPPING (-2)
struct ident_filter {
struct stream_filter filter;
struct strbuf left;
int state;
char ident[GIT_MAX_HEXSZ + 5]; 
};
static int is_foreign_ident(const char *str)
{
int i;
if (!skip_prefix(str, "$Id: ", &str))
return 0;
for (i = 0; str[i]; i++) {
if (isspace(str[i]) && str[i+1] != '$')
return 1;
}
return 0;
}
static void ident_drain(struct ident_filter *ident, char **output_p, size_t *osize_p)
{
size_t to_drain = ident->left.len;
if (*osize_p < to_drain)
to_drain = *osize_p;
if (to_drain) {
memcpy(*output_p, ident->left.buf, to_drain);
strbuf_remove(&ident->left, 0, to_drain);
*output_p += to_drain;
*osize_p -= to_drain;
}
if (!ident->left.len)
ident->state = 0;
}
static int ident_filter_fn(struct stream_filter *filter,
const char *input, size_t *isize_p,
char *output, size_t *osize_p)
{
struct ident_filter *ident = (struct ident_filter *)filter;
static const char head[] = "$Id";
if (!input) {
switch (ident->state) {
default:
strbuf_add(&ident->left, head, ident->state);
case IDENT_SKIPPING:
case IDENT_DRAINING:
ident_drain(ident, &output, osize_p);
}
return 0;
}
while (*isize_p || (ident->state == IDENT_DRAINING)) {
int ch;
if (ident->state == IDENT_DRAINING) {
ident_drain(ident, &output, osize_p);
if (!*osize_p)
break;
continue;
}
ch = *(input++);
(*isize_p)--;
if (ident->state == IDENT_SKIPPING) {
strbuf_addch(&ident->left, ch);
if (ch != '\n' && ch != '$')
continue;
if (ch == '$' && !is_foreign_ident(ident->left.buf)) {
strbuf_setlen(&ident->left, sizeof(head) - 1);
strbuf_addstr(&ident->left, ident->ident);
}
ident->state = IDENT_DRAINING;
continue;
}
if (ident->state < sizeof(head) &&
head[ident->state] == ch) {
ident->state++;
continue;
}
if (ident->state)
strbuf_add(&ident->left, head, ident->state);
if (ident->state == sizeof(head) - 1) {
if (ch != ':' && ch != '$') {
strbuf_addch(&ident->left, ch);
ident->state = 0;
continue;
}
if (ch == ':') {
strbuf_addch(&ident->left, ch);
ident->state = IDENT_SKIPPING;
} else {
strbuf_addstr(&ident->left, ident->ident);
ident->state = IDENT_DRAINING;
}
continue;
}
strbuf_addch(&ident->left, ch);
ident->state = IDENT_DRAINING;
}
return 0;
}
static void ident_free_fn(struct stream_filter *filter)
{
struct ident_filter *ident = (struct ident_filter *)filter;
strbuf_release(&ident->left);
free(filter);
}
static struct stream_filter_vtbl ident_vtbl = {
ident_filter_fn,
ident_free_fn,
};
static struct stream_filter *ident_filter(const struct object_id *oid)
{
struct ident_filter *ident = xmalloc(sizeof(*ident));
xsnprintf(ident->ident, sizeof(ident->ident),
": %s $", oid_to_hex(oid));
strbuf_init(&ident->left, 0);
ident->filter.vtbl = &ident_vtbl;
ident->state = 0;
return (struct stream_filter *)ident;
}
struct stream_filter *get_stream_filter(const struct index_state *istate,
const char *path,
const struct object_id *oid)
{
struct conv_attrs ca;
struct stream_filter *filter = NULL;
convert_attrs(istate, &ca, path);
if (ca.drv && (ca.drv->process || ca.drv->smudge || ca.drv->clean))
return NULL;
if (ca.working_tree_encoding)
return NULL;
if (ca.crlf_action == CRLF_AUTO || ca.crlf_action == CRLF_AUTO_CRLF)
return NULL;
if (ca.ident)
filter = ident_filter(oid);
if (output_eol(ca.crlf_action) == EOL_CRLF)
filter = cascade_filter(filter, lf_to_crlf_filter());
else
filter = cascade_filter(filter, &null_filter_singleton);
return filter;
}
void free_stream_filter(struct stream_filter *filter)
{
filter->vtbl->free(filter);
}
int stream_filter(struct stream_filter *filter,
const char *input, size_t *isize_p,
char *output, size_t *osize_p)
{
return filter->vtbl->filter(filter, input, isize_p, output, osize_p);
}
void init_checkout_metadata(struct checkout_metadata *meta, const char *refname,
const struct object_id *treeish,
const struct object_id *blob)
{
memset(meta, 0, sizeof(*meta));
if (refname)
meta->refname = refname;
if (treeish)
oidcpy(&meta->treeish, treeish);
if (blob)
oidcpy(&meta->blob, blob);
}
void clone_checkout_metadata(struct checkout_metadata *dst,
const struct checkout_metadata *src,
const struct object_id *blob)
{
memcpy(dst, src, sizeof(*dst));
if (blob)
oidcpy(&dst->blob, blob);
}
