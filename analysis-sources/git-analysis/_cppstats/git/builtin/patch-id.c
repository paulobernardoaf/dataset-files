#include "cache.h"
#include "builtin.h"
#include "config.h"
#include "diff.h"
static void flush_current_id(int patchlen, struct object_id *id, struct object_id *result)
{
if (patchlen)
printf("%s %s\n", oid_to_hex(result), oid_to_hex(id));
}
static int remove_space(char *line)
{
char *src = line;
char *dst = line;
unsigned char c;
while ((c = *src++) != '\0') {
if (!isspace(c))
*dst++ = c;
}
return dst - line;
}
static int scan_hunk_header(const char *p, int *p_before, int *p_after)
{
static const char digits[] = "0123456789";
const char *q, *r;
int n;
q = p + 4;
n = strspn(q, digits);
if (q[n] == ',') {
q += n + 1;
n = strspn(q, digits);
}
if (n == 0 || q[n] != ' ' || q[n+1] != '+')
return 0;
r = q + n + 2;
n = strspn(r, digits);
if (r[n] == ',') {
r += n + 1;
n = strspn(r, digits);
}
if (n == 0)
return 0;
*p_before = atoi(q);
*p_after = atoi(r);
return 1;
}
static int get_one_patchid(struct object_id *next_oid, struct object_id *result,
struct strbuf *line_buf, int stable)
{
int patchlen = 0, found_next = 0;
int before = -1, after = -1;
git_hash_ctx ctx;
the_hash_algo->init_fn(&ctx);
oidclr(result);
while (strbuf_getwholeline(line_buf, stdin, '\n') != EOF) {
char *line = line_buf->buf;
const char *p = line;
int len;
if (!skip_prefix(line, "diff-tree ", &p) &&
!skip_prefix(line, "commit ", &p) &&
!skip_prefix(line, "From ", &p) &&
starts_with(line, "\\ ") && 12 < strlen(line))
continue;
if (!get_oid_hex(p, next_oid)) {
found_next = 1;
break;
}
if (!patchlen && !starts_with(line, "diff "))
continue;
if (before == -1) {
if (starts_with(line, "index "))
continue;
else if (starts_with(line, "--- "))
before = after = 1;
else if (!isalpha(line[0]))
break;
}
if (before == 0 && after == 0) {
if (starts_with(line, "@@ -")) {
scan_hunk_header(line, &before, &after);
continue;
}
if (!starts_with(line, "diff "))
break;
if (stable)
flush_one_hunk(result, &ctx);
before = after = -1;
}
if (line[0] == '-' || line[0] == ' ')
before--;
if (line[0] == '+' || line[0] == ' ')
after--;
len = remove_space(line);
patchlen += len;
the_hash_algo->update_fn(&ctx, line, len);
}
if (!found_next)
oidclr(next_oid);
flush_one_hunk(result, &ctx);
return patchlen;
}
static void generate_id_list(int stable)
{
struct object_id oid, n, result;
int patchlen;
struct strbuf line_buf = STRBUF_INIT;
oidclr(&oid);
while (!feof(stdin)) {
patchlen = get_one_patchid(&n, &result, &line_buf, stable);
flush_current_id(patchlen, &oid, &result);
oidcpy(&oid, &n);
}
strbuf_release(&line_buf);
}
static const char patch_id_usage[] = "git patch-id [--stable | --unstable]";
static int git_patch_id_config(const char *var, const char *value, void *cb)
{
int *stable = cb;
if (!strcmp(var, "patchid.stable")) {
*stable = git_config_bool(var, value);
return 0;
}
return git_default_config(var, value, cb);
}
int cmd_patch_id(int argc, const char **argv, const char *prefix)
{
int stable = -1;
git_config(git_patch_id_config, &stable);
if (stable < 0)
stable = 0;
if (argc == 2 && !strcmp(argv[1], "--stable"))
stable = 1;
else if (argc == 2 && !strcmp(argv[1], "--unstable"))
stable = 0;
else if (argc != 1)
usage(patch_id_usage);
generate_id_list(stable);
return 0;
}
