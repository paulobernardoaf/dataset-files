








#include "cache.h"
#include "config.h"
#include "object-store.h"
#include "blob.h"
#include "delta.h"
#include "diff.h"
#include "dir.h"
#include "xdiff-interface.h"
#include "ll-merge.h"
#include "lockfile.h"
#include "parse-options.h"
#include "quote.h"
#include "rerere.h"
#include "apply.h"

struct gitdiff_data {
struct strbuf *root;
int linenr;
int p_value;
};

static void git_apply_config(void)
{
git_config_get_string_const("apply.whitespace", &apply_default_whitespace);
git_config_get_string_const("apply.ignorewhitespace", &apply_default_ignorewhitespace);
git_config(git_xmerge_config, NULL);
}

static int parse_whitespace_option(struct apply_state *state, const char *option)
{
if (!option) {
state->ws_error_action = warn_on_ws_error;
return 0;
}
if (!strcmp(option, "warn")) {
state->ws_error_action = warn_on_ws_error;
return 0;
}
if (!strcmp(option, "nowarn")) {
state->ws_error_action = nowarn_ws_error;
return 0;
}
if (!strcmp(option, "error")) {
state->ws_error_action = die_on_ws_error;
return 0;
}
if (!strcmp(option, "error-all")) {
state->ws_error_action = die_on_ws_error;
state->squelch_whitespace_errors = 0;
return 0;
}
if (!strcmp(option, "strip") || !strcmp(option, "fix")) {
state->ws_error_action = correct_ws_error;
return 0;
}




return error(_("unrecognized whitespace option '%s'"), option);
}

static int parse_ignorewhitespace_option(struct apply_state *state,
const char *option)
{
if (!option || !strcmp(option, "no") ||
!strcmp(option, "false") || !strcmp(option, "never") ||
!strcmp(option, "none")) {
state->ws_ignore_action = ignore_ws_none;
return 0;
}
if (!strcmp(option, "change")) {
state->ws_ignore_action = ignore_ws_change;
return 0;
}
return error(_("unrecognized whitespace ignore option '%s'"), option);
}

int init_apply_state(struct apply_state *state,
struct repository *repo,
const char *prefix)
{
memset(state, 0, sizeof(*state));
state->prefix = prefix;
state->repo = repo;
state->apply = 1;
state->line_termination = '\n';
state->p_value = 1;
state->p_context = UINT_MAX;
state->squelch_whitespace_errors = 5;
state->ws_error_action = warn_on_ws_error;
state->ws_ignore_action = ignore_ws_none;
state->linenr = 1;
string_list_init(&state->fn_table, 0);
string_list_init(&state->limit_by_name, 0);
string_list_init(&state->symlink_changes, 0);
strbuf_init(&state->root, 0);

git_apply_config();
if (apply_default_whitespace && parse_whitespace_option(state, apply_default_whitespace))
return -1;
if (apply_default_ignorewhitespace && parse_ignorewhitespace_option(state, apply_default_ignorewhitespace))
return -1;
return 0;
}

void clear_apply_state(struct apply_state *state)
{
string_list_clear(&state->limit_by_name, 0);
string_list_clear(&state->symlink_changes, 0);
strbuf_release(&state->root);


}

static void mute_routine(const char *msg, va_list params)
{

}

int check_apply_state(struct apply_state *state, int force_apply)
{
int is_not_gitdir = !startup_info->have_repository;

if (state->apply_with_reject && state->threeway)
return error(_("--reject and --3way cannot be used together."));
if (state->cached && state->threeway)
return error(_("--cached and --3way cannot be used together."));
if (state->threeway) {
if (is_not_gitdir)
return error(_("--3way outside a repository"));
state->check_index = 1;
}
if (state->apply_with_reject) {
state->apply = 1;
if (state->apply_verbosity == verbosity_normal)
state->apply_verbosity = verbosity_verbose;
}
if (!force_apply && (state->diffstat || state->numstat || state->summary || state->check || state->fake_ancestor))
state->apply = 0;
if (state->check_index && is_not_gitdir)
return error(_("--index outside a repository"));
if (state->cached) {
if (is_not_gitdir)
return error(_("--cached outside a repository"));
state->check_index = 1;
}
if (state->ita_only && (state->check_index || is_not_gitdir))
state->ita_only = 0;
if (state->check_index)
state->unsafe_paths = 0;

if (state->apply_verbosity <= verbosity_silent) {
state->saved_error_routine = get_error_routine();
state->saved_warn_routine = get_warn_routine();
set_error_routine(mute_routine);
set_warn_routine(mute_routine);
}

return 0;
}

static void set_default_whitespace_mode(struct apply_state *state)
{
if (!state->whitespace_option && !apply_default_whitespace)
state->ws_error_action = (state->apply ? warn_on_ws_error : nowarn_ws_error);
}








struct fragment {
unsigned long leading, trailing;
unsigned long oldpos, oldlines;
unsigned long newpos, newlines;




const char *patch;
unsigned free_patch:1,
rejected:1;
int size;
int linenr;
struct fragment *next;
};






#define binary_patch_method leading
#define BINARY_DELTA_DEFLATED 1
#define BINARY_LITERAL_DEFLATED 2

static void free_fragment_list(struct fragment *list)
{
while (list) {
struct fragment *next = list->next;
if (list->free_patch)
free((char *)list->patch);
free(list);
list = next;
}
}

static void free_patch(struct patch *patch)
{
free_fragment_list(patch->fragments);
free(patch->def_name);
free(patch->old_name);
free(patch->new_name);
free(patch->result);
free(patch);
}

static void free_patch_list(struct patch *list)
{
while (list) {
struct patch *next = list->next;
free_patch(list);
list = next;
}
}






struct line {
size_t len;
unsigned hash : 24;
unsigned flag : 8;
#define LINE_COMMON 1
#define LINE_PATCHED 2
};




struct image {
char *buf;
size_t len;
size_t nr;
size_t alloc;
struct line *line_allocated;
struct line *line;
};

static uint32_t hash_line(const char *cp, size_t len)
{
size_t i;
uint32_t h;
for (i = 0, h = 0; i < len; i++) {
if (!isspace(cp[i])) {
h = h * 3 + (cp[i] & 0xff);
}
}
return h;
}





static int fuzzy_matchlines(const char *s1, size_t n1,
const char *s2, size_t n2)
{
const char *end1 = s1 + n1;
const char *end2 = s2 + n2;


while (s1 < end1 && (end1[-1] == '\r' || end1[-1] == '\n'))
end1--;
while (s2 < end2 && (end2[-1] == '\r' || end2[-1] == '\n'))
end2--;

while (s1 < end1 && s2 < end2) {
if (isspace(*s1)) {




if (!isspace(*s2))
return 0;
while (s1 < end1 && isspace(*s1))
s1++;
while (s2 < end2 && isspace(*s2))
s2++;
} else if (*s1++ != *s2++)
return 0;
}


return s1 == end1 && s2 == end2;
}

static void add_line_info(struct image *img, const char *bol, size_t len, unsigned flag)
{
ALLOC_GROW(img->line_allocated, img->nr + 1, img->alloc);
img->line_allocated[img->nr].len = len;
img->line_allocated[img->nr].hash = hash_line(bol, len);
img->line_allocated[img->nr].flag = flag;
img->nr++;
}






static void prepare_image(struct image *image, char *buf, size_t len,
int prepare_linetable)
{
const char *cp, *ep;

memset(image, 0, sizeof(*image));
image->buf = buf;
image->len = len;

if (!prepare_linetable)
return;

ep = image->buf + image->len;
cp = image->buf;
while (cp < ep) {
const char *next;
for (next = cp; next < ep && *next != '\n'; next++)
;
if (next < ep)
next++;
add_line_info(image, cp, next - cp, 0);
cp = next;
}
image->line = image->line_allocated;
}

static void clear_image(struct image *image)
{
free(image->buf);
free(image->line_allocated);
memset(image, 0, sizeof(*image));
}


static void say_patch_name(FILE *output, const char *fmt, struct patch *patch)
{
struct strbuf sb = STRBUF_INIT;

if (patch->old_name && patch->new_name &&
strcmp(patch->old_name, patch->new_name)) {
quote_c_style(patch->old_name, &sb, NULL, 0);
strbuf_addstr(&sb, " => ");
quote_c_style(patch->new_name, &sb, NULL, 0);
} else {
const char *n = patch->new_name;
if (!n)
n = patch->old_name;
quote_c_style(n, &sb, NULL, 0);
}
fprintf(output, fmt, sb.buf);
fputc('\n', output);
strbuf_release(&sb);
}

#define SLOP (16)

static int read_patch_file(struct strbuf *sb, int fd)
{
if (strbuf_read(sb, fd, 0) < 0)
return error_errno("git apply: failed to read");






strbuf_grow(sb, SLOP);
memset(sb->buf + sb->len, 0, SLOP);
return 0;
}

static unsigned long linelen(const char *buffer, unsigned long size)
{
unsigned long len = 0;
while (size--) {
len++;
if (*buffer++ == '\n')
break;
}
return len;
}

static int is_dev_null(const char *str)
{
return skip_prefix(str, "/dev/null", &str) && isspace(*str);
}

#define TERM_SPACE 1
#define TERM_TAB 2

static int name_terminate(int c, int terminate)
{
if (c == ' ' && !(terminate & TERM_SPACE))
return 0;
if (c == '\t' && !(terminate & TERM_TAB))
return 0;

return 1;
}


static char *squash_slash(char *name)
{
int i = 0, j = 0;

if (!name)
return NULL;

while (name[i]) {
if ((name[j++] = name[i++]) == '/')
while (name[i] == '/')
i++;
}
name[j] = '\0';
return name;
}

static char *find_name_gnu(struct strbuf *root,
const char *line,
int p_value)
{
struct strbuf name = STRBUF_INIT;
char *cp;





if (unquote_c_style(&name, line, NULL)) {
strbuf_release(&name);
return NULL;
}

for (cp = name.buf; p_value; p_value--) {
cp = strchr(cp, '/');
if (!cp) {
strbuf_release(&name);
return NULL;
}
cp++;
}

strbuf_remove(&name, 0, cp - name.buf);
if (root->len)
strbuf_insert(&name, 0, root->buf, root->len);
return squash_slash(strbuf_detach(&name, NULL));
}

static size_t sane_tz_len(const char *line, size_t len)
{
const char *tz, *p;

if (len < strlen(" +0500") || line[len-strlen(" +0500")] != ' ')
return 0;
tz = line + len - strlen(" +0500");

if (tz[1] != '+' && tz[1] != '-')
return 0;

for (p = tz + 2; p != line + len; p++)
if (!isdigit(*p))
return 0;

return line + len - tz;
}

static size_t tz_with_colon_len(const char *line, size_t len)
{
const char *tz, *p;

if (len < strlen(" +08:00") || line[len - strlen(":00")] != ':')
return 0;
tz = line + len - strlen(" +08:00");

if (tz[0] != ' ' || (tz[1] != '+' && tz[1] != '-'))
return 0;
p = tz + 2;
if (!isdigit(*p++) || !isdigit(*p++) || *p++ != ':' ||
!isdigit(*p++) || !isdigit(*p++))
return 0;

return line + len - tz;
}

static size_t date_len(const char *line, size_t len)
{
const char *date, *p;

if (len < strlen("72-02-05") || line[len-strlen("-05")] != '-')
return 0;
p = date = line + len - strlen("72-02-05");

if (!isdigit(*p++) || !isdigit(*p++) || *p++ != '-' ||
!isdigit(*p++) || !isdigit(*p++) || *p++ != '-' ||
!isdigit(*p++) || !isdigit(*p++)) 
return 0;

if (date - line >= strlen("19") &&
isdigit(date[-1]) && isdigit(date[-2])) 
date -= strlen("19");

return line + len - date;
}

static size_t short_time_len(const char *line, size_t len)
{
const char *time, *p;

if (len < strlen(" 07:01:32") || line[len-strlen(":32")] != ':')
return 0;
p = time = line + len - strlen(" 07:01:32");


if (*p++ != ' ' ||
!isdigit(*p++) || !isdigit(*p++) || *p++ != ':' ||
!isdigit(*p++) || !isdigit(*p++) || *p++ != ':' ||
!isdigit(*p++) || !isdigit(*p++)) 
return 0;

return line + len - time;
}

static size_t fractional_time_len(const char *line, size_t len)
{
const char *p;
size_t n;


if (!len || !isdigit(line[len - 1]))
return 0;
p = line + len - 1;


while (p > line && isdigit(*p))
p--;
if (*p != '.')
return 0;


n = short_time_len(line, p - line);
if (!n)
return 0;

return line + len - p + n;
}

static size_t trailing_spaces_len(const char *line, size_t len)
{
const char *p;


if (!len || line[len - 1] != ' ')
return 0;

p = line + len;
while (p != line) {
p--;
if (*p != ' ')
return line + len - (p + 1);
}


return len;
}

static size_t diff_timestamp_len(const char *line, size_t len)
{
const char *end = line + len;
size_t n;






if (!isdigit(end[-1]))
return 0;

n = sane_tz_len(line, end - line);
if (!n)
n = tz_with_colon_len(line, end - line);
end -= n;

n = short_time_len(line, end - line);
if (!n)
n = fractional_time_len(line, end - line);
end -= n;

n = date_len(line, end - line);
if (!n) 
return 0;
end -= n;

if (end == line) 
return 0;
if (end[-1] == '\t') { 
end--;
return line + len - end;
}
if (end[-1] != ' ') 
return 0;


end -= trailing_spaces_len(line, end - line);
return line + len - end;
}

static char *find_name_common(struct strbuf *root,
const char *line,
const char *def,
int p_value,
const char *end,
int terminate)
{
int len;
const char *start = NULL;

if (p_value == 0)
start = line;
while (line != end) {
char c = *line;

if (!end && isspace(c)) {
if (c == '\n')
break;
if (name_terminate(c, terminate))
break;
}
line++;
if (c == '/' && !--p_value)
start = line;
}
if (!start)
return squash_slash(xstrdup_or_null(def));
len = line - start;
if (!len)
return squash_slash(xstrdup_or_null(def));







if (def) {
int deflen = strlen(def);
if (deflen < len && !strncmp(start, def, deflen))
return squash_slash(xstrdup(def));
}

if (root->len) {
char *ret = xstrfmt("%s%.*s", root->buf, len, start);
return squash_slash(ret);
}

return squash_slash(xmemdupz(start, len));
}

static char *find_name(struct strbuf *root,
const char *line,
char *def,
int p_value,
int terminate)
{
if (*line == '"') {
char *name = find_name_gnu(root, line, p_value);
if (name)
return name;
}

return find_name_common(root, line, def, p_value, NULL, terminate);
}

static char *find_name_traditional(struct strbuf *root,
const char *line,
char *def,
int p_value)
{
size_t len;
size_t date_len;

if (*line == '"') {
char *name = find_name_gnu(root, line, p_value);
if (name)
return name;
}

len = strchrnul(line, '\n') - line;
date_len = diff_timestamp_len(line, len);
if (!date_len)
return find_name_common(root, line, def, p_value, NULL, TERM_TAB);
len -= date_len;

return find_name_common(root, line, def, p_value, line + len, 0);
}





static int guess_p_value(struct apply_state *state, const char *nameline)
{
char *name, *cp;
int val = -1;

if (is_dev_null(nameline))
return -1;
name = find_name_traditional(&state->root, nameline, NULL, 0);
if (!name)
return -1;
cp = strchr(name, '/');
if (!cp)
val = 0;
else if (state->prefix) {




if (starts_with(name, state->prefix))
val = count_slashes(state->prefix);
else {
cp++;
if (starts_with(cp, state->prefix))
val = count_slashes(state->prefix) + 1;
}
}
free(name);
return val;
}






static int has_epoch_timestamp(const char *nameline)
{






const char stamp_regexp[] =
"^[0-2][0-9]:([0-5][0-9]):00(\\.0+)?"
" "
"([-+][0-2][0-9]:?[0-5][0-9])\n";
const char *timestamp = NULL, *cp, *colon;
static regex_t *stamp;
regmatch_t m[10];
int zoneoffset, epoch_hour, hour, minute;
int status;

for (cp = nameline; *cp != '\n'; cp++) {
if (*cp == '\t')
timestamp = cp + 1;
}
if (!timestamp)
return 0;





if (skip_prefix(timestamp, "1969-12-31 ", &timestamp))
epoch_hour = 24;
else if (skip_prefix(timestamp, "1970-01-01 ", &timestamp))
epoch_hour = 0;
else
return 0;

if (!stamp) {
stamp = xmalloc(sizeof(*stamp));
if (regcomp(stamp, stamp_regexp, REG_EXTENDED)) {
warning(_("Cannot prepare timestamp regexp %s"),
stamp_regexp);
return 0;
}
}

status = regexec(stamp, timestamp, ARRAY_SIZE(m), m, 0);
if (status) {
if (status != REG_NOMATCH)
warning(_("regexec returned %d for input: %s"),
status, timestamp);
return 0;
}

hour = strtol(timestamp, NULL, 10);
minute = strtol(timestamp + m[1].rm_so, NULL, 10);

zoneoffset = strtol(timestamp + m[3].rm_so + 1, (char **) &colon, 10);
if (*colon == ':')
zoneoffset = zoneoffset * 60 + strtol(colon + 1, NULL, 10);
else
zoneoffset = (zoneoffset / 100) * 60 + (zoneoffset % 100);
if (timestamp[m[3].rm_so] == '-')
zoneoffset = -zoneoffset;

return hour * 60 + minute - zoneoffset == epoch_hour * 60;
}








static int parse_traditional_patch(struct apply_state *state,
const char *first,
const char *second,
struct patch *patch)
{
char *name;

first += 4; 
second += 4; 
if (!state->p_value_known) {
int p, q;
p = guess_p_value(state, first);
q = guess_p_value(state, second);
if (p < 0) p = q;
if (0 <= p && p == q) {
state->p_value = p;
state->p_value_known = 1;
}
}
if (is_dev_null(first)) {
patch->is_new = 1;
patch->is_delete = 0;
name = find_name_traditional(&state->root, second, NULL, state->p_value);
patch->new_name = name;
} else if (is_dev_null(second)) {
patch->is_new = 0;
patch->is_delete = 1;
name = find_name_traditional(&state->root, first, NULL, state->p_value);
patch->old_name = name;
} else {
char *first_name;
first_name = find_name_traditional(&state->root, first, NULL, state->p_value);
name = find_name_traditional(&state->root, second, first_name, state->p_value);
free(first_name);
if (has_epoch_timestamp(first)) {
patch->is_new = 1;
patch->is_delete = 0;
patch->new_name = name;
} else if (has_epoch_timestamp(second)) {
patch->is_new = 0;
patch->is_delete = 1;
patch->old_name = name;
} else {
patch->old_name = name;
patch->new_name = xstrdup_or_null(name);
}
}
if (!name)
return error(_("unable to find filename in patch at line %d"), state->linenr);

return 0;
}

static int gitdiff_hdrend(struct gitdiff_data *state,
const char *line,
struct patch *patch)
{
return 1;
}










#define DIFF_OLD_NAME 0
#define DIFF_NEW_NAME 1

static int gitdiff_verify_name(struct gitdiff_data *state,
const char *line,
int isnull,
char **name,
int side)
{
if (!*name && !isnull) {
*name = find_name(state->root, line, NULL, state->p_value, TERM_TAB);
return 0;
}

if (*name) {
char *another;
if (isnull)
return error(_("git apply: bad git-diff - expected /dev/null, got %s on line %d"),
*name, state->linenr);
another = find_name(state->root, line, NULL, state->p_value, TERM_TAB);
if (!another || strcmp(another, *name)) {
free(another);
return error((side == DIFF_NEW_NAME) ?
_("git apply: bad git-diff - inconsistent new filename on line %d") :
_("git apply: bad git-diff - inconsistent old filename on line %d"), state->linenr);
}
free(another);
} else {
if (!is_dev_null(line))
return error(_("git apply: bad git-diff - expected /dev/null on line %d"), state->linenr);
}

return 0;
}

static int gitdiff_oldname(struct gitdiff_data *state,
const char *line,
struct patch *patch)
{
return gitdiff_verify_name(state, line,
patch->is_new, &patch->old_name,
DIFF_OLD_NAME);
}

static int gitdiff_newname(struct gitdiff_data *state,
const char *line,
struct patch *patch)
{
return gitdiff_verify_name(state, line,
patch->is_delete, &patch->new_name,
DIFF_NEW_NAME);
}

static int parse_mode_line(const char *line, int linenr, unsigned int *mode)
{
char *end;
*mode = strtoul(line, &end, 8);
if (end == line || !isspace(*end))
return error(_("invalid mode on line %d: %s"), linenr, line);
return 0;
}

static int gitdiff_oldmode(struct gitdiff_data *state,
const char *line,
struct patch *patch)
{
return parse_mode_line(line, state->linenr, &patch->old_mode);
}

static int gitdiff_newmode(struct gitdiff_data *state,
const char *line,
struct patch *patch)
{
return parse_mode_line(line, state->linenr, &patch->new_mode);
}

static int gitdiff_delete(struct gitdiff_data *state,
const char *line,
struct patch *patch)
{
patch->is_delete = 1;
free(patch->old_name);
patch->old_name = xstrdup_or_null(patch->def_name);
return gitdiff_oldmode(state, line, patch);
}

static int gitdiff_newfile(struct gitdiff_data *state,
const char *line,
struct patch *patch)
{
patch->is_new = 1;
free(patch->new_name);
patch->new_name = xstrdup_or_null(patch->def_name);
return gitdiff_newmode(state, line, patch);
}

static int gitdiff_copysrc(struct gitdiff_data *state,
const char *line,
struct patch *patch)
{
patch->is_copy = 1;
free(patch->old_name);
patch->old_name = find_name(state->root, line, NULL, state->p_value ? state->p_value - 1 : 0, 0);
return 0;
}

static int gitdiff_copydst(struct gitdiff_data *state,
const char *line,
struct patch *patch)
{
patch->is_copy = 1;
free(patch->new_name);
patch->new_name = find_name(state->root, line, NULL, state->p_value ? state->p_value - 1 : 0, 0);
return 0;
}

static int gitdiff_renamesrc(struct gitdiff_data *state,
const char *line,
struct patch *patch)
{
patch->is_rename = 1;
free(patch->old_name);
patch->old_name = find_name(state->root, line, NULL, state->p_value ? state->p_value - 1 : 0, 0);
return 0;
}

static int gitdiff_renamedst(struct gitdiff_data *state,
const char *line,
struct patch *patch)
{
patch->is_rename = 1;
free(patch->new_name);
patch->new_name = find_name(state->root, line, NULL, state->p_value ? state->p_value - 1 : 0, 0);
return 0;
}

static int gitdiff_similarity(struct gitdiff_data *state,
const char *line,
struct patch *patch)
{
unsigned long val = strtoul(line, NULL, 10);
if (val <= 100)
patch->score = val;
return 0;
}

static int gitdiff_dissimilarity(struct gitdiff_data *state,
const char *line,
struct patch *patch)
{
unsigned long val = strtoul(line, NULL, 10);
if (val <= 100)
patch->score = val;
return 0;
}

static int gitdiff_index(struct gitdiff_data *state,
const char *line,
struct patch *patch)
{




const char *ptr, *eol;
int len;
const unsigned hexsz = the_hash_algo->hexsz;

ptr = strchr(line, '.');
if (!ptr || ptr[1] != '.' || hexsz < ptr - line)
return 0;
len = ptr - line;
memcpy(patch->old_oid_prefix, line, len);
patch->old_oid_prefix[len] = 0;

line = ptr + 2;
ptr = strchr(line, ' ');
eol = strchrnul(line, '\n');

if (!ptr || eol < ptr)
ptr = eol;
len = ptr - line;

if (hexsz < len)
return 0;
memcpy(patch->new_oid_prefix, line, len);
patch->new_oid_prefix[len] = 0;
if (*ptr == ' ')
return gitdiff_oldmode(state, ptr + 1, patch);
return 0;
}





static int gitdiff_unrecognized(struct gitdiff_data *state,
const char *line,
struct patch *patch)
{
return 1;
}





static const char *skip_tree_prefix(int p_value,
const char *line,
int llen)
{
int nslash;
int i;

if (!p_value)
return (llen && line[0] == '/') ? NULL : line;

nslash = p_value;
for (i = 0; i < llen; i++) {
int ch = line[i];
if (ch == '/' && --nslash <= 0)
return (i == 0) ? NULL : &line[i + 1];
}
return NULL;
}









static char *git_header_name(int p_value,
const char *line,
int llen)
{
const char *name;
const char *second = NULL;
size_t len, line_len;

line += strlen("diff --git ");
llen -= strlen("diff --git ");

if (*line == '"') {
const char *cp;
struct strbuf first = STRBUF_INIT;
struct strbuf sp = STRBUF_INIT;

if (unquote_c_style(&first, line, &second))
goto free_and_fail1;


cp = skip_tree_prefix(p_value, first.buf, first.len);
if (!cp)
goto free_and_fail1;
strbuf_remove(&first, 0, cp - first.buf);





while ((second < line + llen) && isspace(*second))
second++;

if (line + llen <= second)
goto free_and_fail1;
if (*second == '"') {
if (unquote_c_style(&sp, second, NULL))
goto free_and_fail1;
cp = skip_tree_prefix(p_value, sp.buf, sp.len);
if (!cp)
goto free_and_fail1;

if (strcmp(cp, first.buf))
goto free_and_fail1;
strbuf_release(&sp);
return strbuf_detach(&first, NULL);
}


cp = skip_tree_prefix(p_value, second, line + llen - second);
if (!cp)
goto free_and_fail1;
if (line + llen - cp != first.len ||
memcmp(first.buf, cp, first.len))
goto free_and_fail1;
return strbuf_detach(&first, NULL);

free_and_fail1:
strbuf_release(&first);
strbuf_release(&sp);
return NULL;
}


name = skip_tree_prefix(p_value, line, llen);
if (!name)
return NULL;





for (second = name; second < line + llen; second++) {
if (*second == '"') {
struct strbuf sp = STRBUF_INIT;
const char *np;

if (unquote_c_style(&sp, second, NULL))
goto free_and_fail2;

np = skip_tree_prefix(p_value, sp.buf, sp.len);
if (!np)
goto free_and_fail2;

len = sp.buf + sp.len - np;
if (len < second - name &&
!strncmp(np, name, len) &&
isspace(name[len])) {

strbuf_remove(&sp, 0, np - sp.buf);
return strbuf_detach(&sp, NULL);
}

free_and_fail2:
strbuf_release(&sp);
return NULL;
}
}





second = strchr(name, '\n');
if (!second)
return NULL;
line_len = second - name;
for (len = 0 ; ; len++) {
switch (name[len]) {
default:
continue;
case '\n':
return NULL;
case '\t': case ' ':








if (!name[len + 1])
return NULL; 
second = skip_tree_prefix(p_value, name + len + 1,
line_len - (len + 1));
if (!second)
return NULL;





if (second[len] == '\n' && !strncmp(name, second, len))
return xmemdupz(name, len);
}
}
}

static int check_header_line(int linenr, struct patch *patch)
{
int extensions = (patch->is_delete == 1) + (patch->is_new == 1) +
(patch->is_rename == 1) + (patch->is_copy == 1);
if (extensions > 1)
return error(_("inconsistent header lines %d and %d"),
patch->extension_linenr, linenr);
if (extensions && !patch->extension_linenr)
patch->extension_linenr = linenr;
return 0;
}

int parse_git_diff_header(struct strbuf *root,
int *linenr,
int p_value,
const char *line,
int len,
unsigned int size,
struct patch *patch)
{
unsigned long offset;
struct gitdiff_data parse_hdr_state;


patch->is_new = 0;
patch->is_delete = 0;







patch->def_name = git_header_name(p_value, line, len);
if (patch->def_name && root->len) {
char *s = xstrfmt("%s%s", root->buf, patch->def_name);
free(patch->def_name);
patch->def_name = s;
}

line += len;
size -= len;
(*linenr)++;
parse_hdr_state.root = root;
parse_hdr_state.linenr = *linenr;
parse_hdr_state.p_value = p_value;

for (offset = len ; size > 0 ; offset += len, size -= len, line += len, (*linenr)++) {
static const struct opentry {
const char *str;
int (*fn)(struct gitdiff_data *, const char *, struct patch *);
} optable[] = {
{ "@@ -", gitdiff_hdrend },
{ "--- ", gitdiff_oldname },
{ "+++ ", gitdiff_newname },
{ "old mode ", gitdiff_oldmode },
{ "new mode ", gitdiff_newmode },
{ "deleted file mode ", gitdiff_delete },
{ "new file mode ", gitdiff_newfile },
{ "copy from ", gitdiff_copysrc },
{ "copy to ", gitdiff_copydst },
{ "rename old ", gitdiff_renamesrc },
{ "rename new ", gitdiff_renamedst },
{ "rename from ", gitdiff_renamesrc },
{ "rename to ", gitdiff_renamedst },
{ "similarity index ", gitdiff_similarity },
{ "dissimilarity index ", gitdiff_dissimilarity },
{ "index ", gitdiff_index },
{ "", gitdiff_unrecognized },
};
int i;

len = linelen(line, size);
if (!len || line[len-1] != '\n')
break;
for (i = 0; i < ARRAY_SIZE(optable); i++) {
const struct opentry *p = optable + i;
int oplen = strlen(p->str);
int res;
if (len < oplen || memcmp(p->str, line, oplen))
continue;
res = p->fn(&parse_hdr_state, line + oplen, patch);
if (res < 0)
return -1;
if (check_header_line(*linenr, patch))
return -1;
if (res > 0)
goto done;
break;
}
}

done:
if (!patch->old_name && !patch->new_name) {
if (!patch->def_name) {
error(Q_("git diff header lacks filename information when removing "
"%d leading pathname component (line %d)",
"git diff header lacks filename information when removing "
"%d leading pathname components (line %d)",
parse_hdr_state.p_value),
parse_hdr_state.p_value, *linenr);
return -128;
}
patch->old_name = xstrdup(patch->def_name);
patch->new_name = xstrdup(patch->def_name);
}
if ((!patch->new_name && !patch->is_delete) ||
(!patch->old_name && !patch->is_new)) {
error(_("git diff header lacks filename information "
"(line %d)"), *linenr);
return -128;
}
patch->is_toplevel_relative = 1;
return offset;
}

static int parse_num(const char *line, unsigned long *p)
{
char *ptr;

if (!isdigit(*line))
return 0;
*p = strtoul(line, &ptr, 10);
return ptr - line;
}

static int parse_range(const char *line, int len, int offset, const char *expect,
unsigned long *p1, unsigned long *p2)
{
int digits, ex;

if (offset < 0 || offset >= len)
return -1;
line += offset;
len -= offset;

digits = parse_num(line, p1);
if (!digits)
return -1;

offset += digits;
line += digits;
len -= digits;

*p2 = 1;
if (*line == ',') {
digits = parse_num(line+1, p2);
if (!digits)
return -1;

offset += digits+1;
line += digits+1;
len -= digits+1;
}

ex = strlen(expect);
if (ex > len)
return -1;
if (memcmp(line, expect, ex))
return -1;

return offset + ex;
}

static void recount_diff(const char *line, int size, struct fragment *fragment)
{
int oldlines = 0, newlines = 0, ret = 0;

if (size < 1) {
warning("recount: ignore empty hunk");
return;
}

for (;;) {
int len = linelen(line, size);
size -= len;
line += len;

if (size < 1)
break;

switch (*line) {
case ' ': case '\n':
newlines++;

case '-':
oldlines++;
continue;
case '+':
newlines++;
continue;
case '\\':
continue;
case '@':
ret = size < 3 || !starts_with(line, "@@ ");
break;
case 'd':
ret = size < 5 || !starts_with(line, "diff ");
break;
default:
ret = -1;
break;
}
if (ret) {
warning(_("recount: unexpected line: %.*s"),
(int)linelen(line, size), line);
return;
}
break;
}
fragment->oldlines = oldlines;
fragment->newlines = newlines;
}





static int parse_fragment_header(const char *line, int len, struct fragment *fragment)
{
int offset;

if (!len || line[len-1] != '\n')
return -1;


offset = parse_range(line, len, 4, " +", &fragment->oldpos, &fragment->oldlines);
offset = parse_range(line, len, offset, " @@", &fragment->newpos, &fragment->newlines);

return offset;
}









static int find_header(struct apply_state *state,
const char *line,
unsigned long size,
int *hdrsize,
struct patch *patch)
{
unsigned long offset, len;

patch->is_toplevel_relative = 0;
patch->is_rename = patch->is_copy = 0;
patch->is_new = patch->is_delete = -1;
patch->old_mode = patch->new_mode = 0;
patch->old_name = patch->new_name = NULL;
for (offset = 0; size > 0; offset += len, size -= len, line += len, state->linenr++) {
unsigned long nextlen;

len = linelen(line, size);
if (!len)
break;


if (len < 6)
continue;






if (!memcmp("@@ -", line, 4)) {
struct fragment dummy;
if (parse_fragment_header(line, len, &dummy) < 0)
continue;
error(_("patch fragment without header at line %d: %.*s"),
state->linenr, (int)len-1, line);
return -128;
}

if (size < len + 6)
break;





if (!memcmp("diff --git ", line, 11)) {
int git_hdr_len = parse_git_diff_header(&state->root, &state->linenr,
state->p_value, line, len,
size, patch);
if (git_hdr_len < 0)
return -128;
if (git_hdr_len <= len)
continue;
*hdrsize = git_hdr_len;
return offset;
}


if (memcmp("--- ", line, 4) || memcmp("+++ ", line + len, 4))
continue;






nextlen = linelen(line + len, size - len);
if (size < nextlen + 14 || memcmp("@@ -", line + len + nextlen, 4))
continue;


if (parse_traditional_patch(state, line, line+len, patch))
return -128;
*hdrsize = len + nextlen;
state->linenr += 2;
return offset;
}
return -1;
}

static void record_ws_error(struct apply_state *state,
unsigned result,
const char *line,
int len,
int linenr)
{
char *err;

if (!result)
return;

state->whitespace_error++;
if (state->squelch_whitespace_errors &&
state->squelch_whitespace_errors < state->whitespace_error)
return;

err = whitespace_error_string(result);
if (state->apply_verbosity > verbosity_silent)
fprintf(stderr, "%s:%d: %s.\n%.*s\n",
state->patch_input_file, linenr, err, len, line);
free(err);
}

static void check_whitespace(struct apply_state *state,
const char *line,
int len,
unsigned ws_rule)
{
unsigned result = ws_check(line + 1, len - 1, ws_rule);

record_ws_error(state, result, line + 1, len - 2, state->linenr);
}





static void check_old_for_crlf(struct patch *patch, const char *line, int len)
{
if (len >= 2 && line[len-1] == '\n' && line[len-2] == '\r') {
patch->ws_rule |= WS_CR_AT_EOL;
patch->crlf_in_old = 1;
}
}








static int parse_fragment(struct apply_state *state,
const char *line,
unsigned long size,
struct patch *patch,
struct fragment *fragment)
{
int added, deleted;
int len = linelen(line, size), offset;
unsigned long oldlines, newlines;
unsigned long leading, trailing;

offset = parse_fragment_header(line, len, fragment);
if (offset < 0)
return -1;
if (offset > 0 && patch->recount)
recount_diff(line + offset, size - offset, fragment);
oldlines = fragment->oldlines;
newlines = fragment->newlines;
leading = 0;
trailing = 0;


line += len;
size -= len;
state->linenr++;
added = deleted = 0;
for (offset = len;
0 < size;
offset += len, size -= len, line += len, state->linenr++) {
if (!oldlines && !newlines)
break;
len = linelen(line, size);
if (!len || line[len-1] != '\n')
return -1;
switch (*line) {
default:
return -1;
case '\n': 
case ' ':
oldlines--;
newlines--;
if (!deleted && !added)
leading++;
trailing++;
check_old_for_crlf(patch, line, len);
if (!state->apply_in_reverse &&
state->ws_error_action == correct_ws_error)
check_whitespace(state, line, len, patch->ws_rule);
break;
case '-':
if (!state->apply_in_reverse)
check_old_for_crlf(patch, line, len);
if (state->apply_in_reverse &&
state->ws_error_action != nowarn_ws_error)
check_whitespace(state, line, len, patch->ws_rule);
deleted++;
oldlines--;
trailing = 0;
break;
case '+':
if (state->apply_in_reverse)
check_old_for_crlf(patch, line, len);
if (!state->apply_in_reverse &&
state->ws_error_action != nowarn_ws_error)
check_whitespace(state, line, len, patch->ws_rule);
added++;
newlines--;
trailing = 0;
break;









case '\\':
if (len < 12 || memcmp(line, "\\ ", 2))
return -1;
break;
}
}
if (oldlines || newlines)
return -1;
if (!patch->recount && !deleted && !added)
return -1;

fragment->leading = leading;
fragment->trailing = trailing;






if (12 < size && !memcmp(line, "\\ ", 2))
offset += linelen(line, size);

patch->lines_added += added;
patch->lines_deleted += deleted;

if (0 < patch->is_new && oldlines)
return error(_("new file depends on old contents"));
if (0 < patch->is_delete && newlines)
return error(_("deleted file still has contents"));
return offset;
}













static int parse_single_patch(struct apply_state *state,
const char *line,
unsigned long size,
struct patch *patch)
{
unsigned long offset = 0;
unsigned long oldlines = 0, newlines = 0, context = 0;
struct fragment **fragp = &patch->fragments;

while (size > 4 && !memcmp(line, "@@ -", 4)) {
struct fragment *fragment;
int len;

fragment = xcalloc(1, sizeof(*fragment));
fragment->linenr = state->linenr;
len = parse_fragment(state, line, size, patch, fragment);
if (len <= 0) {
free(fragment);
return error(_("corrupt patch at line %d"), state->linenr);
}
fragment->patch = line;
fragment->size = len;
oldlines += fragment->oldlines;
newlines += fragment->newlines;
context += fragment->leading + fragment->trailing;

*fragp = fragment;
fragp = &fragment->next;

offset += len;
line += len;
size -= len;
}














if (patch->is_new < 0 &&
(oldlines || (patch->fragments && patch->fragments->next)))
patch->is_new = 0;
if (patch->is_delete < 0 &&
(newlines || (patch->fragments && patch->fragments->next)))
patch->is_delete = 0;

if (0 < patch->is_new && oldlines)
return error(_("new file %s depends on old contents"), patch->new_name);
if (0 < patch->is_delete && newlines)
return error(_("deleted file %s still has contents"), patch->old_name);
if (!patch->is_delete && !newlines && context && state->apply_verbosity > verbosity_silent)
fprintf_ln(stderr,
_("** warning: "
"file %s becomes empty but is not deleted"),
patch->new_name);

return offset;
}

static inline int metadata_changes(struct patch *patch)
{
return patch->is_rename > 0 ||
patch->is_copy > 0 ||
patch->is_new > 0 ||
patch->is_delete ||
(patch->old_mode && patch->new_mode &&
patch->old_mode != patch->new_mode);
}

static char *inflate_it(const void *data, unsigned long size,
unsigned long inflated_size)
{
git_zstream stream;
void *out;
int st;

memset(&stream, 0, sizeof(stream));

stream.next_in = (unsigned char *)data;
stream.avail_in = size;
stream.next_out = out = xmalloc(inflated_size);
stream.avail_out = inflated_size;
git_inflate_init(&stream);
st = git_inflate(&stream, Z_FINISH);
git_inflate_end(&stream);
if ((st != Z_STREAM_END) || stream.total_out != inflated_size) {
free(out);
return NULL;
}
return out;
}






static struct fragment *parse_binary_hunk(struct apply_state *state,
char **buf_p,
unsigned long *sz_p,
int *status_p,
int *used_p)
{












int llen, used;
unsigned long size = *sz_p;
char *buffer = *buf_p;
int patch_method;
unsigned long origlen;
char *data = NULL;
int hunk_size = 0;
struct fragment *frag;

llen = linelen(buffer, size);
used = llen;

*status_p = 0;

if (starts_with(buffer, "delta ")) {
patch_method = BINARY_DELTA_DEFLATED;
origlen = strtoul(buffer + 6, NULL, 10);
}
else if (starts_with(buffer, "literal ")) {
patch_method = BINARY_LITERAL_DEFLATED;
origlen = strtoul(buffer + 8, NULL, 10);
}
else
return NULL;

state->linenr++;
buffer += llen;
while (1) {
int byte_length, max_byte_length, newsize;
llen = linelen(buffer, size);
used += llen;
state->linenr++;
if (llen == 1) {

buffer++;
size--;
break;
}




if ((llen < 7) || (llen-2) % 5)
goto corrupt;
max_byte_length = (llen - 2) / 5 * 4;
byte_length = *buffer;
if ('A' <= byte_length && byte_length <= 'Z')
byte_length = byte_length - 'A' + 1;
else if ('a' <= byte_length && byte_length <= 'z')
byte_length = byte_length - 'a' + 27;
else
goto corrupt;




if (max_byte_length < byte_length ||
byte_length <= max_byte_length - 4)
goto corrupt;
newsize = hunk_size + byte_length;
data = xrealloc(data, newsize);
if (decode_85(data + hunk_size, buffer + 1, byte_length))
goto corrupt;
hunk_size = newsize;
buffer += llen;
size -= llen;
}

frag = xcalloc(1, sizeof(*frag));
frag->patch = inflate_it(data, hunk_size, origlen);
frag->free_patch = 1;
if (!frag->patch)
goto corrupt;
free(data);
frag->size = origlen;
*buf_p = buffer;
*sz_p = size;
*used_p = used;
frag->binary_patch_method = patch_method;
return frag;

corrupt:
free(data);
*status_p = -1;
error(_("corrupt binary patch at line %d: %.*s"),
state->linenr-1, llen-1, buffer);
return NULL;
}






static int parse_binary(struct apply_state *state,
char *buffer,
unsigned long size,
struct patch *patch)
{














struct fragment *forward;
struct fragment *reverse;
int status;
int used, used_1;

forward = parse_binary_hunk(state, &buffer, &size, &status, &used);
if (!forward && !status)

return error(_("unrecognized binary patch at line %d"), state->linenr-1);
if (status)

return status;

reverse = parse_binary_hunk(state, &buffer, &size, &status, &used_1);
if (reverse)
used += used_1;
else if (status) {




free((void*) forward->patch);
free(forward);
return status;
}
forward->next = reverse;
patch->fragments = forward;
patch->is_binary = 1;
return used;
}

static void prefix_one(struct apply_state *state, char **name)
{
char *old_name = *name;
if (!old_name)
return;
*name = prefix_filename(state->prefix, *name);
free(old_name);
}

static void prefix_patch(struct apply_state *state, struct patch *p)
{
if (!state->prefix || p->is_toplevel_relative)
return;
prefix_one(state, &p->new_name);
prefix_one(state, &p->old_name);
}





static void add_name_limit(struct apply_state *state,
const char *name,
int exclude)
{
struct string_list_item *it;

it = string_list_append(&state->limit_by_name, name);
it->util = exclude ? NULL : (void *) 1;
}

static int use_patch(struct apply_state *state, struct patch *p)
{
const char *pathname = p->new_name ? p->new_name : p->old_name;
int i;


if (state->prefix && *state->prefix) {
const char *rest;
if (!skip_prefix(pathname, state->prefix, &rest) || !*rest)
return 0;
}


for (i = 0; i < state->limit_by_name.nr; i++) {
struct string_list_item *it = &state->limit_by_name.items[i];
if (!wildmatch(it->string, pathname, 0))
return (it->util != NULL);
}






return !state->has_include;
}












static int parse_chunk(struct apply_state *state, char *buffer, unsigned long size, struct patch *patch)
{
int hdrsize, patchsize;
int offset = find_header(state, buffer, size, &hdrsize, patch);

if (offset < 0)
return offset;

prefix_patch(state, patch);

if (!use_patch(state, patch))
patch->ws_rule = 0;
else if (patch->new_name)
patch->ws_rule = whitespace_rule(state->repo->index,
patch->new_name);
else
patch->ws_rule = whitespace_rule(state->repo->index,
patch->old_name);

patchsize = parse_single_patch(state,
buffer + offset + hdrsize,
size - offset - hdrsize,
patch);

if (patchsize < 0)
return -128;

if (!patchsize) {
static const char git_binary[] = "GIT binary patch\n";
int hd = hdrsize + offset;
unsigned long llen = linelen(buffer + hd, size - hd);

if (llen == sizeof(git_binary) - 1 &&
!memcmp(git_binary, buffer + hd, llen)) {
int used;
state->linenr++;
used = parse_binary(state, buffer + hd + llen,
size - hd - llen, patch);
if (used < 0)
return -1;
if (used)
patchsize = used + llen;
else
patchsize = 0;
}
else if (!memcmp(" differ\n", buffer + hd + llen - 8, 8)) {
static const char *binhdr[] = {
"Binary files ",
"Files ",
NULL,
};
int i;
for (i = 0; binhdr[i]; i++) {
int len = strlen(binhdr[i]);
if (len < size - hd &&
!memcmp(binhdr[i], buffer + hd, len)) {
state->linenr++;
patch->is_binary = 1;
patchsize = llen;
break;
}
}
}





if ((state->apply || state->check) &&
(!patch->is_binary && !metadata_changes(patch))) {
error(_("patch with only garbage at line %d"), state->linenr);
return -128;
}
}

return offset + hdrsize + patchsize;
}

static void reverse_patches(struct patch *p)
{
for (; p; p = p->next) {
struct fragment *frag = p->fragments;

SWAP(p->new_name, p->old_name);
SWAP(p->new_mode, p->old_mode);
SWAP(p->is_new, p->is_delete);
SWAP(p->lines_added, p->lines_deleted);
SWAP(p->old_oid_prefix, p->new_oid_prefix);

for (; frag; frag = frag->next) {
SWAP(frag->newpos, frag->oldpos);
SWAP(frag->newlines, frag->oldlines);
}
}
}

static const char pluses[] =
"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
static const char minuses[]=
"----------------------------------------------------------------------";

static void show_stats(struct apply_state *state, struct patch *patch)
{
struct strbuf qname = STRBUF_INIT;
char *cp = patch->new_name ? patch->new_name : patch->old_name;
int max, add, del;

quote_c_style(cp, &qname, NULL, 0);




max = state->max_len;
if (max > 50)
max = 50;

if (qname.len > max) {
cp = strchr(qname.buf + qname.len + 3 - max, '/');
if (!cp)
cp = qname.buf + qname.len + 3 - max;
strbuf_splice(&qname, 0, cp - qname.buf, "...", 3);
}

if (patch->is_binary) {
printf(" %-*s | Bin\n", max, qname.buf);
strbuf_release(&qname);
return;
}

printf(" %-*s |", max, qname.buf);
strbuf_release(&qname);




max = max + state->max_change > 70 ? 70 - max : state->max_change;
add = patch->lines_added;
del = patch->lines_deleted;

if (state->max_change > 0) {
int total = ((add + del) * max + state->max_change / 2) / state->max_change;
add = (add * max + state->max_change / 2) / state->max_change;
del = total - add;
}
printf("%5d %.*s%.*s\n", patch->lines_added + patch->lines_deleted,
add, pluses, del, minuses);
}

static int read_old_data(struct stat *st, struct patch *patch,
const char *path, struct strbuf *buf)
{
int conv_flags = patch->crlf_in_old ?
CONV_EOL_KEEP_CRLF : CONV_EOL_RENORMALIZE;
switch (st->st_mode & S_IFMT) {
case S_IFLNK:
if (strbuf_readlink(buf, path, st->st_size) < 0)
return error(_("unable to read symlink %s"), path);
return 0;
case S_IFREG:
if (strbuf_read_file(buf, path, st->st_size) != st->st_size)
return error(_("unable to open or read %s"), path);








convert_to_git(NULL, path, buf->buf, buf->len, buf, conv_flags);
return 0;
default:
return -1;
}
}








static void update_pre_post_images(struct image *preimage,
struct image *postimage,
char *buf,
size_t len, size_t postlen)
{
int i, ctx, reduced;
char *new_buf, *old_buf, *fixed;
struct image fixed_preimage;






prepare_image(&fixed_preimage, buf, len, 1);
assert(postlen
? fixed_preimage.nr == preimage->nr
: fixed_preimage.nr <= preimage->nr);
for (i = 0; i < fixed_preimage.nr; i++)
fixed_preimage.line[i].flag = preimage->line[i].flag;
free(preimage->line_allocated);
*preimage = fixed_preimage;










old_buf = postimage->buf;
if (postlen)
new_buf = postimage->buf = xmalloc(postlen);
else
new_buf = old_buf;
fixed = preimage->buf;

for (i = reduced = ctx = 0; i < postimage->nr; i++) {
size_t l_len = postimage->line[i].len;
if (!(postimage->line[i].flag & LINE_COMMON)) {

memmove(new_buf, old_buf, l_len);
old_buf += l_len;
new_buf += l_len;
continue;
}


old_buf += l_len;


while (ctx < preimage->nr &&
!(preimage->line[ctx].flag & LINE_COMMON)) {
fixed += preimage->line[ctx].len;
ctx++;
}





if (preimage->nr <= ctx) {
reduced++;
continue;
}


l_len = preimage->line[ctx].len;
memcpy(new_buf, fixed, l_len);
new_buf += l_len;
fixed += l_len;
postimage->line[i].len = l_len;
ctx++;
}

if (postlen
? postlen < new_buf - postimage->buf
: postimage->len < new_buf - postimage->buf)
BUG("caller miscounted postlen: asked %d, orig = %d, used = %d",
(int)postlen, (int) postimage->len, (int)(new_buf - postimage->buf));


postimage->len = new_buf - postimage->buf;
postimage->nr -= reduced;
}

static int line_by_line_fuzzy_match(struct image *img,
struct image *preimage,
struct image *postimage,
unsigned long current,
int current_lno,
int preimage_limit)
{
int i;
size_t imgoff = 0;
size_t preoff = 0;
size_t postlen = postimage->len;
size_t extra_chars;
char *buf;
char *preimage_eof;
char *preimage_end;
struct strbuf fixed;
char *fixed_buf;
size_t fixed_len;

for (i = 0; i < preimage_limit; i++) {
size_t prelen = preimage->line[i].len;
size_t imglen = img->line[current_lno+i].len;

if (!fuzzy_matchlines(img->buf + current + imgoff, imglen,
preimage->buf + preoff, prelen))
return 0;
if (preimage->line[i].flag & LINE_COMMON)
postlen += imglen - prelen;
imgoff += imglen;
preoff += prelen;
}












buf = preimage_eof = preimage->buf + preoff;
for ( ; i < preimage->nr; i++)
preoff += preimage->line[i].len;
preimage_end = preimage->buf + preoff;
for ( ; buf < preimage_end; buf++)
if (!isspace(*buf))
return 0;








extra_chars = preimage_end - preimage_eof;
strbuf_init(&fixed, imgoff + extra_chars);
strbuf_add(&fixed, img->buf + current, imgoff);
strbuf_add(&fixed, preimage_eof, extra_chars);
fixed_buf = strbuf_detach(&fixed, &fixed_len);
update_pre_post_images(preimage, postimage,
fixed_buf, fixed_len, postlen);
return 1;
}

static int match_fragment(struct apply_state *state,
struct image *img,
struct image *preimage,
struct image *postimage,
unsigned long current,
int current_lno,
unsigned ws_rule,
int match_beginning, int match_end)
{
int i;
char *fixed_buf, *buf, *orig, *target;
struct strbuf fixed;
size_t fixed_len, postlen;
int preimage_limit;

if (preimage->nr + current_lno <= img->nr) {



preimage_limit = preimage->nr;
if (match_end && (preimage->nr + current_lno != img->nr))
return 0;
} else if (state->ws_error_action == correct_ws_error &&
(ws_rule & WS_BLANK_AT_EOF)) {







preimage_limit = img->nr - current_lno;
} else {





return 0;
}

if (match_beginning && current_lno)
return 0;


for (i = 0; i < preimage_limit; i++)
if ((img->line[current_lno + i].flag & LINE_PATCHED) ||
(preimage->line[i].hash != img->line[current_lno + i].hash))
return 0;

if (preimage_limit == preimage->nr) {







if ((match_end
? (current + preimage->len == img->len)
: (current + preimage->len <= img->len)) &&
!memcmp(img->buf + current, preimage->buf, preimage->len))
return 1;
} else {







char *buf_end;

buf = preimage->buf;
buf_end = buf;
for (i = 0; i < preimage_limit; i++)
buf_end += preimage->line[i].len;

for ( ; buf < buf_end; buf++)
if (!isspace(*buf))
break;
if (buf == buf_end)
return 0;
}






if (state->ws_ignore_action == ignore_ws_change)
return line_by_line_fuzzy_match(img, preimage, postimage,
current, current_lno, preimage_limit);

if (state->ws_error_action != correct_ws_error)
return 0;

















postlen = 0;
for (i = 0; i < postimage->nr; i++) {
if (!(postimage->line[i].flag & LINE_COMMON))
postlen += postimage->line[i].len;
}






strbuf_init(&fixed, preimage->len + 1);
orig = preimage->buf;
target = img->buf + current;
for (i = 0; i < preimage_limit; i++) {
size_t oldlen = preimage->line[i].len;
size_t tgtlen = img->line[current_lno + i].len;
size_t fixstart = fixed.len;
struct strbuf tgtfix;
int match;


ws_fix_copy(&fixed, orig, oldlen, ws_rule, NULL);


strbuf_init(&tgtfix, tgtlen);
ws_fix_copy(&tgtfix, target, tgtlen, ws_rule, NULL);











match = (tgtfix.len == fixed.len - fixstart &&
!memcmp(tgtfix.buf, fixed.buf + fixstart,
fixed.len - fixstart));


if (preimage->line[i].flag & LINE_COMMON)
postlen += tgtfix.len;

strbuf_release(&tgtfix);
if (!match)
goto unmatch_exit;

orig += oldlen;
target += tgtlen;
}








for ( ; i < preimage->nr; i++) {
size_t fixstart = fixed.len; 
size_t oldlen = preimage->line[i].len;
int j;


ws_fix_copy(&fixed, orig, oldlen, ws_rule, NULL);

for (j = fixstart; j < fixed.len; j++)
if (!isspace(fixed.buf[j]))
goto unmatch_exit;

orig += oldlen;
}






fixed_buf = strbuf_detach(&fixed, &fixed_len);
if (postlen < postimage->len)
postlen = 0;
update_pre_post_images(preimage, postimage,
fixed_buf, fixed_len, postlen);
return 1;

unmatch_exit:
strbuf_release(&fixed);
return 0;
}

static int find_pos(struct apply_state *state,
struct image *img,
struct image *preimage,
struct image *postimage,
int line,
unsigned ws_rule,
int match_beginning, int match_end)
{
int i;
unsigned long backwards, forwards, current;
int backwards_lno, forwards_lno, current_lno;







if (state->allow_overlap && match_beginning && match_end &&
img->nr - preimage->nr != 0)
match_beginning = 0;






if (match_beginning)
line = 0;
else if (match_end)
line = img->nr - preimage->nr;






if ((size_t) line > img->nr)
line = img->nr;

current = 0;
for (i = 0; i < line; i++)
current += img->line[i].len;





backwards = current;
backwards_lno = line;
forwards = current;
forwards_lno = line;
current_lno = line;

for (i = 0; ; i++) {
if (match_fragment(state, img, preimage, postimage,
current, current_lno, ws_rule,
match_beginning, match_end))
return current_lno;

again:
if (backwards_lno == 0 && forwards_lno == img->nr)
break;

if (i & 1) {
if (backwards_lno == 0) {
i++;
goto again;
}
backwards_lno--;
backwards -= img->line[backwards_lno].len;
current = backwards;
current_lno = backwards_lno;
} else {
if (forwards_lno == img->nr) {
i++;
goto again;
}
forwards += img->line[forwards_lno].len;
forwards_lno++;
current = forwards;
current_lno = forwards_lno;
}

}
return -1;
}

static void remove_first_line(struct image *img)
{
img->buf += img->line[0].len;
img->len -= img->line[0].len;
img->line++;
img->nr--;
}

static void remove_last_line(struct image *img)
{
img->len -= img->line[--img->nr].len;
}






static void update_image(struct apply_state *state,
struct image *img,
int applied_pos,
struct image *preimage,
struct image *postimage)
{




int i, nr;
size_t remove_count, insert_count, applied_at = 0;
char *result;
int preimage_limit;










preimage_limit = preimage->nr;
if (preimage_limit > img->nr - applied_pos)
preimage_limit = img->nr - applied_pos;

for (i = 0; i < applied_pos; i++)
applied_at += img->line[i].len;

remove_count = 0;
for (i = 0; i < preimage_limit; i++)
remove_count += img->line[applied_pos + i].len;
insert_count = postimage->len;


result = xmalloc(st_add3(st_sub(img->len, remove_count), insert_count, 1));
memcpy(result, img->buf, applied_at);
memcpy(result + applied_at, postimage->buf, postimage->len);
memcpy(result + applied_at + postimage->len,
img->buf + (applied_at + remove_count),
img->len - (applied_at + remove_count));
free(img->buf);
img->buf = result;
img->len += insert_count - remove_count;
result[img->len] = '\0';


nr = img->nr + postimage->nr - preimage_limit;
if (preimage_limit < postimage->nr) {




REALLOC_ARRAY(img->line, nr);
img->line_allocated = img->line;
}
if (preimage_limit != postimage->nr)
MOVE_ARRAY(img->line + applied_pos + postimage->nr,
img->line + applied_pos + preimage_limit,
img->nr - (applied_pos + preimage_limit));
COPY_ARRAY(img->line + applied_pos, postimage->line, postimage->nr);
if (!state->allow_overlap)
for (i = 0; i < postimage->nr; i++)
img->line[applied_pos + i].flag |= LINE_PATCHED;
img->nr = nr;
}






static int apply_one_fragment(struct apply_state *state,
struct image *img, struct fragment *frag,
int inaccurate_eof, unsigned ws_rule,
int nth_fragment)
{
int match_beginning, match_end;
const char *patch = frag->patch;
int size = frag->size;
char *old, *oldlines;
struct strbuf newlines;
int new_blank_lines_at_end = 0;
int found_new_blank_lines_at_end = 0;
int hunk_linenr = frag->linenr;
unsigned long leading, trailing;
int pos, applied_pos;
struct image preimage;
struct image postimage;

memset(&preimage, 0, sizeof(preimage));
memset(&postimage, 0, sizeof(postimage));
oldlines = xmalloc(size);
strbuf_init(&newlines, size);

old = oldlines;
while (size > 0) {
char first;
int len = linelen(patch, size);
int plen;
int added_blank_line = 0;
int is_blank_context = 0;
size_t start;

if (!len)
break;








plen = len - 1;
if (len < size && patch[len] == '\\')
plen--;
first = *patch;
if (state->apply_in_reverse) {
if (first == '-')
first = '+';
else if (first == '+')
first = '-';
}

switch (first) {
case '\n':

if (plen < 0)

break;
*old++ = '\n';
strbuf_addch(&newlines, '\n');
add_line_info(&preimage, "\n", 1, LINE_COMMON);
add_line_info(&postimage, "\n", 1, LINE_COMMON);
is_blank_context = 1;
break;
case ' ':
if (plen && (ws_rule & WS_BLANK_AT_EOF) &&
ws_blank_line(patch + 1, plen, ws_rule))
is_blank_context = 1;

case '-':
memcpy(old, patch + 1, plen);
add_line_info(&preimage, old, plen,
(first == ' ' ? LINE_COMMON : 0));
old += plen;
if (first == '-')
break;

case '+':

if (first == '+' && state->no_add)
break;

start = newlines.len;
if (first != '+' ||
!state->whitespace_error ||
state->ws_error_action != correct_ws_error) {
strbuf_add(&newlines, patch + 1, plen);
}
else {
ws_fix_copy(&newlines, patch + 1, plen, ws_rule, &state->applied_after_fixing_ws);
}
add_line_info(&postimage, newlines.buf + start, newlines.len - start,
(first == '+' ? 0 : LINE_COMMON));
if (first == '+' &&
(ws_rule & WS_BLANK_AT_EOF) &&
ws_blank_line(patch + 1, plen, ws_rule))
added_blank_line = 1;
break;
case '@': case '\\':

break;
default:
if (state->apply_verbosity > verbosity_normal)
error(_("invalid start of line: '%c'"), first);
applied_pos = -1;
goto out;
}
if (added_blank_line) {
if (!new_blank_lines_at_end)
found_new_blank_lines_at_end = hunk_linenr;
new_blank_lines_at_end++;
}
else if (is_blank_context)
;
else
new_blank_lines_at_end = 0;
patch += len;
size -= len;
hunk_linenr++;
}
if (inaccurate_eof &&
old > oldlines && old[-1] == '\n' &&
newlines.len > 0 && newlines.buf[newlines.len - 1] == '\n') {
old--;
strbuf_setlen(&newlines, newlines.len - 1);
preimage.line_allocated[preimage.nr - 1].len--;
postimage.line_allocated[postimage.nr - 1].len--;
}

leading = frag->leading;
trailing = frag->trailing;













match_beginning = (!frag->oldpos ||
(frag->oldpos == 1 && !state->unidiff_zero));







match_end = !state->unidiff_zero && !trailing;

pos = frag->newpos ? (frag->newpos - 1) : 0;
preimage.buf = oldlines;
preimage.len = old - oldlines;
postimage.buf = newlines.buf;
postimage.len = newlines.len;
preimage.line = preimage.line_allocated;
postimage.line = postimage.line_allocated;

for (;;) {

applied_pos = find_pos(state, img, &preimage, &postimage, pos,
ws_rule, match_beginning, match_end);

if (applied_pos >= 0)
break;


if ((leading <= state->p_context) && (trailing <= state->p_context))
break;
if (match_beginning || match_end) {
match_beginning = match_end = 0;
continue;
}






if (leading >= trailing) {
remove_first_line(&preimage);
remove_first_line(&postimage);
pos--;
leading--;
}
if (trailing > leading) {
remove_last_line(&preimage);
remove_last_line(&postimage);
trailing--;
}
}

if (applied_pos >= 0) {
if (new_blank_lines_at_end &&
preimage.nr + applied_pos >= img->nr &&
(ws_rule & WS_BLANK_AT_EOF) &&
state->ws_error_action != nowarn_ws_error) {
record_ws_error(state, WS_BLANK_AT_EOF, "+", 1,
found_new_blank_lines_at_end);
if (state->ws_error_action == correct_ws_error) {
while (new_blank_lines_at_end--)
remove_last_line(&postimage);
}







if (state->ws_error_action == die_on_ws_error)
state->apply = 0;
}

if (state->apply_verbosity > verbosity_normal && applied_pos != pos) {
int offset = applied_pos - pos;
if (state->apply_in_reverse)
offset = 0 - offset;
fprintf_ln(stderr,
Q_("Hunk #%d succeeded at %d (offset %d line).",
"Hunk #%d succeeded at %d (offset %d lines).",
offset),
nth_fragment, applied_pos + 1, offset);
}





if ((leading != frag->leading ||
trailing != frag->trailing) && state->apply_verbosity > verbosity_silent)
fprintf_ln(stderr, _("Context reduced to (%ld/%ld)"
" to apply fragment at %d"),
leading, trailing, applied_pos+1);
update_image(state, img, applied_pos, &preimage, &postimage);
} else {
if (state->apply_verbosity > verbosity_normal)
error(_("while searching for:\n%.*s"),
(int)(old - oldlines), oldlines);
}

out:
free(oldlines);
strbuf_release(&newlines);
free(preimage.line_allocated);
free(postimage.line_allocated);

return (applied_pos < 0);
}

static int apply_binary_fragment(struct apply_state *state,
struct image *img,
struct patch *patch)
{
struct fragment *fragment = patch->fragments;
unsigned long len;
void *dst;

if (!fragment)
return error(_("missing binary patch data for '%s'"),
patch->new_name ?
patch->new_name :
patch->old_name);


if (state->apply_in_reverse) {
if (!fragment->next)
return error(_("cannot reverse-apply a binary patch "
"without the reverse hunk to '%s'"),
patch->new_name
? patch->new_name : patch->old_name);
fragment = fragment->next;
}
switch (fragment->binary_patch_method) {
case BINARY_DELTA_DEFLATED:
dst = patch_delta(img->buf, img->len, fragment->patch,
fragment->size, &len);
if (!dst)
return -1;
clear_image(img);
img->buf = dst;
img->len = len;
return 0;
case BINARY_LITERAL_DEFLATED:
clear_image(img);
img->len = fragment->size;
img->buf = xmemdupz(fragment->patch, img->len);
return 0;
}
return -1;
}







static int apply_binary(struct apply_state *state,
struct image *img,
struct patch *patch)
{
const char *name = patch->old_name ? patch->old_name : patch->new_name;
struct object_id oid;
const unsigned hexsz = the_hash_algo->hexsz;





if (strlen(patch->old_oid_prefix) != hexsz ||
strlen(patch->new_oid_prefix) != hexsz ||
get_oid_hex(patch->old_oid_prefix, &oid) ||
get_oid_hex(patch->new_oid_prefix, &oid))
return error(_("cannot apply binary patch to '%s' "
"without full index line"), name);

if (patch->old_name) {




hash_object_file(the_hash_algo, img->buf, img->len, blob_type,
&oid);
if (strcmp(oid_to_hex(&oid), patch->old_oid_prefix))
return error(_("the patch applies to '%s' (%s), "
"which does not match the "
"current contents."),
name, oid_to_hex(&oid));
}
else {

if (img->len)
return error(_("the patch applies to an empty "
"'%s' but it is not empty"), name);
}

get_oid_hex(patch->new_oid_prefix, &oid);
if (is_null_oid(&oid)) {
clear_image(img);
return 0; 
}

if (has_object_file(&oid)) {

enum object_type type;
unsigned long size;
char *result;

result = read_object_file(&oid, &type, &size);
if (!result)
return error(_("the necessary postimage %s for "
"'%s' cannot be read"),
patch->new_oid_prefix, name);
clear_image(img);
img->buf = result;
img->len = size;
} else {





if (apply_binary_fragment(state, img, patch))
return error(_("binary patch does not apply to '%s'"),
name);


hash_object_file(the_hash_algo, img->buf, img->len, blob_type,
&oid);
if (strcmp(oid_to_hex(&oid), patch->new_oid_prefix))
return error(_("binary patch to '%s' creates incorrect result (expecting %s, got %s)"),
name, patch->new_oid_prefix, oid_to_hex(&oid));
}

return 0;
}

static int apply_fragments(struct apply_state *state, struct image *img, struct patch *patch)
{
struct fragment *frag = patch->fragments;
const char *name = patch->old_name ? patch->old_name : patch->new_name;
unsigned ws_rule = patch->ws_rule;
unsigned inaccurate_eof = patch->inaccurate_eof;
int nth = 0;

if (patch->is_binary)
return apply_binary(state, img, patch);

while (frag) {
nth++;
if (apply_one_fragment(state, img, frag, inaccurate_eof, ws_rule, nth)) {
error(_("patch failed: %s:%ld"), name, frag->oldpos);
if (!state->apply_with_reject)
return -1;
frag->rejected = 1;
}
frag = frag->next;
}
return 0;
}

static int read_blob_object(struct strbuf *buf, const struct object_id *oid, unsigned mode)
{
if (S_ISGITLINK(mode)) {
strbuf_grow(buf, 100);
strbuf_addf(buf, "Subproject commit %s\n", oid_to_hex(oid));
} else {
enum object_type type;
unsigned long sz;
char *result;

result = read_object_file(oid, &type, &sz);
if (!result)
return -1;

strbuf_attach(buf, result, sz, sz + 1);
}
return 0;
}

static int read_file_or_gitlink(const struct cache_entry *ce, struct strbuf *buf)
{
if (!ce)
return 0;
return read_blob_object(buf, &ce->oid, ce->ce_mode);
}

static struct patch *in_fn_table(struct apply_state *state, const char *name)
{
struct string_list_item *item;

if (name == NULL)
return NULL;

item = string_list_lookup(&state->fn_table, name);
if (item != NULL)
return (struct patch *)item->util;

return NULL;
}













#define PATH_TO_BE_DELETED ((struct patch *) -2)
#define PATH_WAS_DELETED ((struct patch *) -1)

static int to_be_deleted(struct patch *patch)
{
return patch == PATH_TO_BE_DELETED;
}

static int was_deleted(struct patch *patch)
{
return patch == PATH_WAS_DELETED;
}

static void add_to_fn_table(struct apply_state *state, struct patch *patch)
{
struct string_list_item *item;






if (patch->new_name != NULL) {
item = string_list_insert(&state->fn_table, patch->new_name);
item->util = patch;
}





if ((patch->new_name == NULL) || (patch->is_rename)) {
item = string_list_insert(&state->fn_table, patch->old_name);
item->util = PATH_WAS_DELETED;
}
}

static void prepare_fn_table(struct apply_state *state, struct patch *patch)
{



while (patch) {
if ((patch->new_name == NULL) || (patch->is_rename)) {
struct string_list_item *item;
item = string_list_insert(&state->fn_table, patch->old_name);
item->util = PATH_TO_BE_DELETED;
}
patch = patch->next;
}
}

static int checkout_target(struct index_state *istate,
struct cache_entry *ce, struct stat *st)
{
struct checkout costate = CHECKOUT_INIT;

costate.refresh_cache = 1;
costate.istate = istate;
if (checkout_entry(ce, &costate, NULL, NULL) ||
lstat(ce->name, st))
return error(_("cannot checkout %s"), ce->name);
return 0;
}

static struct patch *previous_patch(struct apply_state *state,
struct patch *patch,
int *gone)
{
struct patch *previous;

*gone = 0;
if (patch->is_copy || patch->is_rename)
return NULL; 

previous = in_fn_table(state, patch->old_name);
if (!previous)
return NULL;

if (to_be_deleted(previous))
return NULL; 

if (was_deleted(previous))
*gone = 1;

return previous;
}

static int verify_index_match(struct apply_state *state,
const struct cache_entry *ce,
struct stat *st)
{
if (S_ISGITLINK(ce->ce_mode)) {
if (!S_ISDIR(st->st_mode))
return -1;
return 0;
}
return ie_match_stat(state->repo->index, ce, st,
CE_MATCH_IGNORE_VALID | CE_MATCH_IGNORE_SKIP_WORKTREE);
}

#define SUBMODULE_PATCH_WITHOUT_INDEX 1

static int load_patch_target(struct apply_state *state,
struct strbuf *buf,
const struct cache_entry *ce,
struct stat *st,
struct patch *patch,
const char *name,
unsigned expected_mode)
{
if (state->cached || state->check_index) {
if (read_file_or_gitlink(ce, buf))
return error(_("failed to read %s"), name);
} else if (name) {
if (S_ISGITLINK(expected_mode)) {
if (ce)
return read_file_or_gitlink(ce, buf);
else
return SUBMODULE_PATCH_WITHOUT_INDEX;
} else if (has_symlink_leading_path(name, strlen(name))) {
return error(_("reading from '%s' beyond a symbolic link"), name);
} else {
if (read_old_data(st, patch, name, buf))
return error(_("failed to read %s"), name);
}
}
return 0;
}








static int load_preimage(struct apply_state *state,
struct image *image,
struct patch *patch, struct stat *st,
const struct cache_entry *ce)
{
struct strbuf buf = STRBUF_INIT;
size_t len;
char *img;
struct patch *previous;
int status;

previous = previous_patch(state, patch, &status);
if (status)
return error(_("path %s has been renamed/deleted"),
patch->old_name);
if (previous) {

strbuf_add(&buf, previous->result, previous->resultsize);
} else {
status = load_patch_target(state, &buf, ce, st, patch,
patch->old_name, patch->old_mode);
if (status < 0)
return status;
else if (status == SUBMODULE_PATCH_WITHOUT_INDEX) {






free_fragment_list(patch->fragments);
patch->fragments = NULL;
} else if (status) {
return error(_("failed to read %s"), patch->old_name);
}
}

img = strbuf_detach(&buf, &len);
prepare_image(image, img, len, !patch->is_binary);
return 0;
}

static int three_way_merge(struct apply_state *state,
struct image *image,
char *path,
const struct object_id *base,
const struct object_id *ours,
const struct object_id *theirs)
{
mmfile_t base_file, our_file, their_file;
mmbuffer_t result = { NULL };
int status;

read_mmblob(&base_file, base);
read_mmblob(&our_file, ours);
read_mmblob(&their_file, theirs);
status = ll_merge(&result, path,
&base_file, "base",
&our_file, "ours",
&their_file, "theirs",
state->repo->index,
NULL);
free(base_file.ptr);
free(our_file.ptr);
free(their_file.ptr);
if (status < 0 || !result.ptr) {
free(result.ptr);
return -1;
}
clear_image(image);
image->buf = result.ptr;
image->len = result.size;

return status;
}






static int load_current(struct apply_state *state,
struct image *image,
struct patch *patch)
{
struct strbuf buf = STRBUF_INIT;
int status, pos;
size_t len;
char *img;
struct stat st;
struct cache_entry *ce;
char *name = patch->new_name;
unsigned mode = patch->new_mode;

if (!patch->is_new)
BUG("patch to %s is not a creation", patch->old_name);

pos = index_name_pos(state->repo->index, name, strlen(name));
if (pos < 0)
return error(_("%s: does not exist in index"), name);
ce = state->repo->index->cache[pos];
if (lstat(name, &st)) {
if (errno != ENOENT)
return error_errno("%s", name);
if (checkout_target(state->repo->index, ce, &st))
return -1;
}
if (verify_index_match(state, ce, &st))
return error(_("%s: does not match index"), name);

status = load_patch_target(state, &buf, ce, &st, patch, name, mode);
if (status < 0)
return status;
else if (status)
return -1;
img = strbuf_detach(&buf, &len);
prepare_image(image, img, len, !patch->is_binary);
return 0;
}

static int try_threeway(struct apply_state *state,
struct image *image,
struct patch *patch,
struct stat *st,
const struct cache_entry *ce)
{
struct object_id pre_oid, post_oid, our_oid;
struct strbuf buf = STRBUF_INIT;
size_t len;
int status;
char *img;
struct image tmp_image;


if (patch->is_delete ||
S_ISGITLINK(patch->old_mode) || S_ISGITLINK(patch->new_mode))
return -1;


if (patch->is_new)
write_object_file("", 0, blob_type, &pre_oid);
else if (get_oid(patch->old_oid_prefix, &pre_oid) ||
read_blob_object(&buf, &pre_oid, patch->old_mode))
return error(_("repository lacks the necessary blob to fall back on 3-way merge."));

if (state->apply_verbosity > verbosity_silent)
fprintf(stderr, _("Falling back to three-way merge...\n"));

img = strbuf_detach(&buf, &len);
prepare_image(&tmp_image, img, len, 1);

if (apply_fragments(state, &tmp_image, patch) < 0) {
clear_image(&tmp_image);
return -1;
}

write_object_file(tmp_image.buf, tmp_image.len, blob_type, &post_oid);
clear_image(&tmp_image);


if (patch->is_new) {
if (load_current(state, &tmp_image, patch))
return error(_("cannot read the current contents of '%s'"),
patch->new_name);
} else {
if (load_preimage(state, &tmp_image, patch, st, ce))
return error(_("cannot read the current contents of '%s'"),
patch->old_name);
}
write_object_file(tmp_image.buf, tmp_image.len, blob_type, &our_oid);
clear_image(&tmp_image);


status = three_way_merge(state, image, patch->new_name,
&pre_oid, &our_oid, &post_oid);
if (status < 0) {
if (state->apply_verbosity > verbosity_silent)
fprintf(stderr,
_("Failed to fall back on three-way merge...\n"));
return status;
}

if (status) {
patch->conflicted_threeway = 1;
if (patch->is_new)
oidclr(&patch->threeway_stage[0]);
else
oidcpy(&patch->threeway_stage[0], &pre_oid);
oidcpy(&patch->threeway_stage[1], &our_oid);
oidcpy(&patch->threeway_stage[2], &post_oid);
if (state->apply_verbosity > verbosity_silent)
fprintf(stderr,
_("Applied patch to '%s' with conflicts.\n"),
patch->new_name);
} else {
if (state->apply_verbosity > verbosity_silent)
fprintf(stderr,
_("Applied patch to '%s' cleanly.\n"),
patch->new_name);
}
return 0;
}

static int apply_data(struct apply_state *state, struct patch *patch,
struct stat *st, const struct cache_entry *ce)
{
struct image image;

if (load_preimage(state, &image, patch, st, ce) < 0)
return -1;

if (patch->direct_to_threeway ||
apply_fragments(state, &image, patch) < 0) {

if (!state->threeway || try_threeway(state, &image, patch, st, ce) < 0)
return -1;
}
patch->result = image.buf;
patch->resultsize = image.len;
add_to_fn_table(state, patch);
free(image.line_allocated);

if (0 < patch->is_delete && patch->resultsize)
return error(_("removal patch leaves file contents"));

return 0;
}












static int check_preimage(struct apply_state *state,
struct patch *patch,
struct cache_entry **ce,
struct stat *st)
{
const char *old_name = patch->old_name;
struct patch *previous = NULL;
int stat_ret = 0, status;
unsigned st_mode = 0;

if (!old_name)
return 0;

assert(patch->is_new <= 0);
previous = previous_patch(state, patch, &status);

if (status)
return error(_("path %s has been renamed/deleted"), old_name);
if (previous) {
st_mode = previous->new_mode;
} else if (!state->cached) {
stat_ret = lstat(old_name, st);
if (stat_ret && errno != ENOENT)
return error_errno("%s", old_name);
}

if (state->check_index && !previous) {
int pos = index_name_pos(state->repo->index, old_name,
strlen(old_name));
if (pos < 0) {
if (patch->is_new < 0)
goto is_new;
return error(_("%s: does not exist in index"), old_name);
}
*ce = state->repo->index->cache[pos];
if (stat_ret < 0) {
if (checkout_target(state->repo->index, *ce, st))
return -1;
}
if (!state->cached && verify_index_match(state, *ce, st))
return error(_("%s: does not match index"), old_name);
if (state->cached)
st_mode = (*ce)->ce_mode;
} else if (stat_ret < 0) {
if (patch->is_new < 0)
goto is_new;
return error_errno("%s", old_name);
}

if (!state->cached && !previous)
st_mode = ce_mode_from_stat(*ce, st->st_mode);

if (patch->is_new < 0)
patch->is_new = 0;
if (!patch->old_mode)
patch->old_mode = st_mode;
if ((st_mode ^ patch->old_mode) & S_IFMT)
return error(_("%s: wrong type"), old_name);
if (st_mode != patch->old_mode)
warning(_("%s has type %o, expected %o"),
old_name, st_mode, patch->old_mode);
if (!patch->new_mode && !patch->is_delete)
patch->new_mode = st_mode;
return 0;

is_new:
patch->is_new = 1;
patch->is_delete = 0;
FREE_AND_NULL(patch->old_name);
return 0;
}


#define EXISTS_IN_INDEX 1
#define EXISTS_IN_WORKTREE 2

static int check_to_create(struct apply_state *state,
const char *new_name,
int ok_if_exists)
{
struct stat nst;

if (state->check_index &&
index_name_pos(state->repo->index, new_name, strlen(new_name)) >= 0 &&
!ok_if_exists)
return EXISTS_IN_INDEX;
if (state->cached)
return 0;

if (!lstat(new_name, &nst)) {
if (S_ISDIR(nst.st_mode) || ok_if_exists)
return 0;







if (has_symlink_leading_path(new_name, strlen(new_name)))
return 0;

return EXISTS_IN_WORKTREE;
} else if (!is_missing_file_error(errno)) {
return error_errno("%s", new_name);
}
return 0;
}

static uintptr_t register_symlink_changes(struct apply_state *state,
const char *path,
uintptr_t what)
{
struct string_list_item *ent;

ent = string_list_lookup(&state->symlink_changes, path);
if (!ent) {
ent = string_list_insert(&state->symlink_changes, path);
ent->util = (void *)0;
}
ent->util = (void *)(what | ((uintptr_t)ent->util));
return (uintptr_t)ent->util;
}

static uintptr_t check_symlink_changes(struct apply_state *state, const char *path)
{
struct string_list_item *ent;

ent = string_list_lookup(&state->symlink_changes, path);
if (!ent)
return 0;
return (uintptr_t)ent->util;
}

static void prepare_symlink_changes(struct apply_state *state, struct patch *patch)
{
for ( ; patch; patch = patch->next) {
if ((patch->old_name && S_ISLNK(patch->old_mode)) &&
(patch->is_rename || patch->is_delete))

register_symlink_changes(state, patch->old_name, APPLY_SYMLINK_GOES_AWAY);

if (patch->new_name && S_ISLNK(patch->new_mode))

register_symlink_changes(state, patch->new_name, APPLY_SYMLINK_IN_RESULT);
}
}

static int path_is_beyond_symlink_1(struct apply_state *state, struct strbuf *name)
{
do {
unsigned int change;

while (--name->len && name->buf[name->len] != '/')
; 
if (!name->len)
break;
name->buf[name->len] = '\0';
change = check_symlink_changes(state, name->buf);
if (change & APPLY_SYMLINK_IN_RESULT)
return 1;
if (change & APPLY_SYMLINK_GOES_AWAY)




continue;


if (state->check_index) {
struct cache_entry *ce;

ce = index_file_exists(state->repo->index, name->buf,
name->len, ignore_case);
if (ce && S_ISLNK(ce->ce_mode))
return 1;
} else {
struct stat st;
if (!lstat(name->buf, &st) && S_ISLNK(st.st_mode))
return 1;
}
} while (1);
return 0;
}

static int path_is_beyond_symlink(struct apply_state *state, const char *name_)
{
int ret;
struct strbuf name = STRBUF_INIT;

assert(*name_ != '\0');
strbuf_addstr(&name, name_);
ret = path_is_beyond_symlink_1(state, &name);
strbuf_release(&name);

return ret;
}

static int check_unsafe_path(struct patch *patch)
{
const char *old_name = NULL;
const char *new_name = NULL;
if (patch->is_delete)
old_name = patch->old_name;
else if (!patch->is_new && !patch->is_copy)
old_name = patch->old_name;
if (!patch->is_delete)
new_name = patch->new_name;

if (old_name && !verify_path(old_name, patch->old_mode))
return error(_("invalid path '%s'"), old_name);
if (new_name && !verify_path(new_name, patch->new_mode))
return error(_("invalid path '%s'"), new_name);
return 0;
}





static int check_patch(struct apply_state *state, struct patch *patch)
{
struct stat st;
const char *old_name = patch->old_name;
const char *new_name = patch->new_name;
const char *name = old_name ? old_name : new_name;
struct cache_entry *ce = NULL;
struct patch *tpatch;
int ok_if_exists;
int status;

patch->rejected = 1; 

status = check_preimage(state, patch, &ce, &st);
if (status)
return status;
old_name = patch->old_name;















if ((tpatch = in_fn_table(state, new_name)) &&
(was_deleted(tpatch) || to_be_deleted(tpatch)))
ok_if_exists = 1;
else
ok_if_exists = 0;

if (new_name &&
((0 < patch->is_new) || patch->is_rename || patch->is_copy)) {
int err = check_to_create(state, new_name, ok_if_exists);

if (err && state->threeway) {
patch->direct_to_threeway = 1;
} else switch (err) {
case 0:
break; 
case EXISTS_IN_INDEX:
return error(_("%s: already exists in index"), new_name);
break;
case EXISTS_IN_WORKTREE:
return error(_("%s: already exists in working directory"),
new_name);
default:
return err;
}

if (!patch->new_mode) {
if (0 < patch->is_new)
patch->new_mode = S_IFREG | 0644;
else
patch->new_mode = patch->old_mode;
}
}

if (new_name && old_name) {
int same = !strcmp(old_name, new_name);
if (!patch->new_mode)
patch->new_mode = patch->old_mode;
if ((patch->old_mode ^ patch->new_mode) & S_IFMT) {
if (same)
return error(_("new mode (%o) of %s does not "
"match old mode (%o)"),
patch->new_mode, new_name,
patch->old_mode);
else
return error(_("new mode (%o) of %s does not "
"match old mode (%o) of %s"),
patch->new_mode, new_name,
patch->old_mode, old_name);
}
}

if (!state->unsafe_paths && check_unsafe_path(patch))
return -128;










if (!patch->is_delete && path_is_beyond_symlink(state, patch->new_name))
return error(_("affected file '%s' is beyond a symbolic link"),
patch->new_name);

if (apply_data(state, patch, &st, ce) < 0)
return error(_("%s: patch does not apply"), name);
patch->rejected = 0;
return 0;
}

static int check_patch_list(struct apply_state *state, struct patch *patch)
{
int err = 0;

prepare_symlink_changes(state, patch);
prepare_fn_table(state, patch);
while (patch) {
int res;
if (state->apply_verbosity > verbosity_normal)
say_patch_name(stderr,
_("Checking patch %s..."), patch);
res = check_patch(state, patch);
if (res == -128)
return -128;
err |= res;
patch = patch->next;
}
return err;
}

static int read_apply_cache(struct apply_state *state)
{
if (state->index_file)
return read_index_from(state->repo->index, state->index_file,
get_git_dir());
else
return repo_read_index(state->repo);
}


static int get_current_oid(struct apply_state *state, const char *path,
struct object_id *oid)
{
int pos;

if (read_apply_cache(state) < 0)
return -1;
pos = index_name_pos(state->repo->index, path, strlen(path));
if (pos < 0)
return -1;
oidcpy(oid, &state->repo->index->cache[pos]->oid);
return 0;
}

static int preimage_oid_in_gitlink_patch(struct patch *p, struct object_id *oid)
{










struct fragment *hunk = p->fragments;
static const char heading[] = "-Subproject commit ";
char *preimage;

if (
hunk && !hunk->next &&

hunk->oldpos == 1 && hunk->oldlines == 1 &&

(preimage = memchr(hunk->patch, '\n', hunk->size)) != NULL &&
starts_with(++preimage, heading) &&

!get_oid_hex(preimage + sizeof(heading) - 1, oid) &&
preimage[sizeof(heading) + the_hash_algo->hexsz - 1] == '\n' &&

starts_with(preimage + sizeof(heading) - 1, p->old_oid_prefix))
return 0; 


return get_oid_hex(p->old_oid_prefix, oid);
}


static int build_fake_ancestor(struct apply_state *state, struct patch *list)
{
struct patch *patch;
struct index_state result = { NULL };
struct lock_file lock = LOCK_INIT;
int res;




for (patch = list; patch; patch = patch->next) {
struct object_id oid;
struct cache_entry *ce;
const char *name;

name = patch->old_name ? patch->old_name : patch->new_name;
if (0 < patch->is_new)
continue;

if (S_ISGITLINK(patch->old_mode)) {
if (!preimage_oid_in_gitlink_patch(patch, &oid))
; 
else
return error(_("sha1 information is lacking or "
"useless for submodule %s"), name);
} else if (!get_oid_blob(patch->old_oid_prefix, &oid)) {
; 
} else if (!patch->lines_added && !patch->lines_deleted) {

if (get_current_oid(state, patch->old_name, &oid))
return error(_("mode change for %s, which is not "
"in current HEAD"), name);
} else
return error(_("sha1 information is lacking or useless "
"(%s)."), name);

ce = make_cache_entry(&result, patch->old_mode, &oid, name, 0, 0);
if (!ce)
return error(_("make_cache_entry failed for path '%s'"),
name);
if (add_index_entry(&result, ce, ADD_CACHE_OK_TO_ADD)) {
discard_cache_entry(ce);
return error(_("could not add %s to temporary index"),
name);
}
}

hold_lock_file_for_update(&lock, state->fake_ancestor, LOCK_DIE_ON_ERROR);
res = write_locked_index(&result, &lock, COMMIT_LOCK);
discard_index(&result);

if (res)
return error(_("could not write temporary index to %s"),
state->fake_ancestor);

return 0;
}

static void stat_patch_list(struct apply_state *state, struct patch *patch)
{
int files, adds, dels;

for (files = adds = dels = 0 ; patch ; patch = patch->next) {
files++;
adds += patch->lines_added;
dels += patch->lines_deleted;
show_stats(state, patch);
}

print_stat_summary(stdout, files, adds, dels);
}

static void numstat_patch_list(struct apply_state *state,
struct patch *patch)
{
for ( ; patch; patch = patch->next) {
const char *name;
name = patch->new_name ? patch->new_name : patch->old_name;
if (patch->is_binary)
printf("-\t-\t");
else
printf("%d\t%d\t", patch->lines_added, patch->lines_deleted);
write_name_quoted(name, stdout, state->line_termination);
}
}

static void show_file_mode_name(const char *newdelete, unsigned int mode, const char *name)
{
if (mode)
printf(" %s mode %06o %s\n", newdelete, mode, name);
else
printf(" %s %s\n", newdelete, name);
}

static void show_mode_change(struct patch *p, int show_name)
{
if (p->old_mode && p->new_mode && p->old_mode != p->new_mode) {
if (show_name)
printf(" mode change %06o => %06o %s\n",
p->old_mode, p->new_mode, p->new_name);
else
printf(" mode change %06o => %06o\n",
p->old_mode, p->new_mode);
}
}

static void show_rename_copy(struct patch *p)
{
const char *renamecopy = p->is_rename ? "rename" : "copy";
const char *old_name, *new_name;


old_name = p->old_name;
new_name = p->new_name;
while (1) {
const char *slash_old, *slash_new;
slash_old = strchr(old_name, '/');
slash_new = strchr(new_name, '/');
if (!slash_old ||
!slash_new ||
slash_old - old_name != slash_new - new_name ||
memcmp(old_name, new_name, slash_new - new_name))
break;
old_name = slash_old + 1;
new_name = slash_new + 1;
}



if (old_name != p->old_name)
printf(" %s %.*s{%s => %s} (%d%%)\n", renamecopy,
(int)(old_name - p->old_name), p->old_name,
old_name, new_name, p->score);
else
printf(" %s %s => %s (%d%%)\n", renamecopy,
p->old_name, p->new_name, p->score);
show_mode_change(p, 0);
}

static void summary_patch_list(struct patch *patch)
{
struct patch *p;

for (p = patch; p; p = p->next) {
if (p->is_new)
show_file_mode_name("create", p->new_mode, p->new_name);
else if (p->is_delete)
show_file_mode_name("delete", p->old_mode, p->old_name);
else {
if (p->is_rename || p->is_copy)
show_rename_copy(p);
else {
if (p->score) {
printf(" rewrite %s (%d%%)\n",
p->new_name, p->score);
show_mode_change(p, 0);
}
else
show_mode_change(p, 1);
}
}
}
}

static void patch_stats(struct apply_state *state, struct patch *patch)
{
int lines = patch->lines_added + patch->lines_deleted;

if (lines > state->max_change)
state->max_change = lines;
if (patch->old_name) {
int len = quote_c_style(patch->old_name, NULL, NULL, 0);
if (!len)
len = strlen(patch->old_name);
if (len > state->max_len)
state->max_len = len;
}
if (patch->new_name) {
int len = quote_c_style(patch->new_name, NULL, NULL, 0);
if (!len)
len = strlen(patch->new_name);
if (len > state->max_len)
state->max_len = len;
}
}

static int remove_file(struct apply_state *state, struct patch *patch, int rmdir_empty)
{
if (state->update_index && !state->ita_only) {
if (remove_file_from_index(state->repo->index, patch->old_name) < 0)
return error(_("unable to remove %s from index"), patch->old_name);
}
if (!state->cached) {
if (!remove_or_warn(patch->old_mode, patch->old_name) && rmdir_empty) {
remove_path(patch->old_name);
}
}
return 0;
}

static int add_index_file(struct apply_state *state,
const char *path,
unsigned mode,
void *buf,
unsigned long size)
{
struct stat st;
struct cache_entry *ce;
int namelen = strlen(path);

ce = make_empty_cache_entry(state->repo->index, namelen);
memcpy(ce->name, path, namelen);
ce->ce_mode = create_ce_mode(mode);
ce->ce_flags = create_ce_flags(0);
ce->ce_namelen = namelen;
if (state->ita_only) {
ce->ce_flags |= CE_INTENT_TO_ADD;
set_object_name_for_intent_to_add_entry(ce);
} else if (S_ISGITLINK(mode)) {
const char *s;

if (!skip_prefix(buf, "Subproject commit ", &s) ||
get_oid_hex(s, &ce->oid)) {
discard_cache_entry(ce);
return error(_("corrupt patch for submodule %s"), path);
}
} else {
if (!state->cached) {
if (lstat(path, &st) < 0) {
discard_cache_entry(ce);
return error_errno(_("unable to stat newly "
"created file '%s'"),
path);
}
fill_stat_cache_info(state->repo->index, ce, &st);
}
if (write_object_file(buf, size, blob_type, &ce->oid) < 0) {
discard_cache_entry(ce);
return error(_("unable to create backing store "
"for newly created file %s"), path);
}
}
if (add_index_entry(state->repo->index, ce, ADD_CACHE_OK_TO_ADD) < 0) {
discard_cache_entry(ce);
return error(_("unable to add cache entry for %s"), path);
}

return 0;
}







static int try_create_file(struct apply_state *state, const char *path,
unsigned int mode, const char *buf,
unsigned long size)
{
int fd, res;
struct strbuf nbuf = STRBUF_INIT;

if (S_ISGITLINK(mode)) {
struct stat st;
if (!lstat(path, &st) && S_ISDIR(st.st_mode))
return 0;
return !!mkdir(path, 0777);
}

if (has_symlinks && S_ISLNK(mode))



return !!symlink(buf, path);

fd = open(path, O_CREAT | O_EXCL | O_WRONLY, (mode & 0100) ? 0777 : 0666);
if (fd < 0)
return 1;

if (convert_to_working_tree(state->repo->index, path, buf, size, &nbuf, NULL)) {
size = nbuf.len;
buf = nbuf.buf;
}

res = write_in_full(fd, buf, size) < 0;
if (res)
error_errno(_("failed to write to '%s'"), path);
strbuf_release(&nbuf);

if (close(fd) < 0 && !res)
return error_errno(_("closing file '%s'"), path);

return res ? -1 : 0;
}










static int create_one_file(struct apply_state *state,
char *path,
unsigned mode,
const char *buf,
unsigned long size)
{
int res;

if (state->cached)
return 0;

res = try_create_file(state, path, mode, buf, size);
if (res < 0)
return -1;
if (!res)
return 0;

if (errno == ENOENT) {
if (safe_create_leading_directories(path))
return 0;
res = try_create_file(state, path, mode, buf, size);
if (res < 0)
return -1;
if (!res)
return 0;
}

if (errno == EEXIST || errno == EACCES) {



struct stat st;
if (!lstat(path, &st) && (!S_ISDIR(st.st_mode) || !rmdir(path)))
errno = EEXIST;
}

if (errno == EEXIST) {
unsigned int nr = getpid();

for (;;) {
char newpath[PATH_MAX];
mksnpath(newpath, sizeof(newpath), "%s~%u", path, nr);
res = try_create_file(state, newpath, mode, buf, size);
if (res < 0)
return -1;
if (!res) {
if (!rename(newpath, path))
return 0;
unlink_or_warn(newpath);
break;
}
if (errno != EEXIST)
break;
++nr;
}
}
return error_errno(_("unable to write file '%s' mode %o"),
path, mode);
}

static int add_conflicted_stages_file(struct apply_state *state,
struct patch *patch)
{
int stage, namelen;
unsigned mode;
struct cache_entry *ce;

if (!state->update_index)
return 0;
namelen = strlen(patch->new_name);
mode = patch->new_mode ? patch->new_mode : (S_IFREG | 0644);

remove_file_from_index(state->repo->index, patch->new_name);
for (stage = 1; stage < 4; stage++) {
if (is_null_oid(&patch->threeway_stage[stage - 1]))
continue;
ce = make_empty_cache_entry(state->repo->index, namelen);
memcpy(ce->name, patch->new_name, namelen);
ce->ce_mode = create_ce_mode(mode);
ce->ce_flags = create_ce_flags(stage);
ce->ce_namelen = namelen;
oidcpy(&ce->oid, &patch->threeway_stage[stage - 1]);
if (add_index_entry(state->repo->index, ce, ADD_CACHE_OK_TO_ADD) < 0) {
discard_cache_entry(ce);
return error(_("unable to add cache entry for %s"),
patch->new_name);
}
}

return 0;
}

static int create_file(struct apply_state *state, struct patch *patch)
{
char *path = patch->new_name;
unsigned mode = patch->new_mode;
unsigned long size = patch->resultsize;
char *buf = patch->result;

if (!mode)
mode = S_IFREG | 0644;
if (create_one_file(state, path, mode, buf, size))
return -1;

if (patch->conflicted_threeway)
return add_conflicted_stages_file(state, patch);
else if (state->update_index)
return add_index_file(state, path, mode, buf, size);
return 0;
}


static int write_out_one_result(struct apply_state *state,
struct patch *patch,
int phase)
{
if (patch->is_delete > 0) {
if (phase == 0)
return remove_file(state, patch, 1);
return 0;
}
if (patch->is_new > 0 || patch->is_copy) {
if (phase == 1)
return create_file(state, patch);
return 0;
}




if (phase == 0)
return remove_file(state, patch, patch->is_rename);
if (phase == 1)
return create_file(state, patch);
return 0;
}

static int write_out_one_reject(struct apply_state *state, struct patch *patch)
{
FILE *rej;
char namebuf[PATH_MAX];
struct fragment *frag;
int cnt = 0;
struct strbuf sb = STRBUF_INIT;

for (cnt = 0, frag = patch->fragments; frag; frag = frag->next) {
if (!frag->rejected)
continue;
cnt++;
}

if (!cnt) {
if (state->apply_verbosity > verbosity_normal)
say_patch_name(stderr,
_("Applied patch %s cleanly."), patch);
return 0;
}




if (!patch->new_name)
die(_("internal error"));


strbuf_addf(&sb, Q_("Applying patch %%s with %d reject...",
"Applying patch %%s with %d rejects...",
cnt),
cnt);
if (state->apply_verbosity > verbosity_silent)
say_patch_name(stderr, sb.buf, patch);
strbuf_release(&sb);

cnt = strlen(patch->new_name);
if (ARRAY_SIZE(namebuf) <= cnt + 5) {
cnt = ARRAY_SIZE(namebuf) - 5;
warning(_("truncating .rej filename to %.*s.rej"),
cnt - 1, patch->new_name);
}
memcpy(namebuf, patch->new_name, cnt);
memcpy(namebuf + cnt, ".rej", 5);

rej = fopen(namebuf, "w");
if (!rej)
return error_errno(_("cannot open %s"), namebuf);






fprintf(rej, "diff a/%s b/%s\t(rejected hunks)\n",
patch->new_name, patch->new_name);
for (cnt = 1, frag = patch->fragments;
frag;
cnt++, frag = frag->next) {
if (!frag->rejected) {
if (state->apply_verbosity > verbosity_silent)
fprintf_ln(stderr, _("Hunk #%d applied cleanly."), cnt);
continue;
}
if (state->apply_verbosity > verbosity_silent)
fprintf_ln(stderr, _("Rejected hunk #%d."), cnt);
fprintf(rej, "%.*s", frag->size, frag->patch);
if (frag->patch[frag->size-1] != '\n')
fputc('\n', rej);
}
fclose(rej);
return -1;
}







static int write_out_results(struct apply_state *state, struct patch *list)
{
int phase;
int errs = 0;
struct patch *l;
struct string_list cpath = STRING_LIST_INIT_DUP;

for (phase = 0; phase < 2; phase++) {
l = list;
while (l) {
if (l->rejected)
errs = 1;
else {
if (write_out_one_result(state, l, phase)) {
string_list_clear(&cpath, 0);
return -1;
}
if (phase == 1) {
if (write_out_one_reject(state, l))
errs = 1;
if (l->conflicted_threeway) {
string_list_append(&cpath, l->new_name);
errs = 1;
}
}
}
l = l->next;
}
}

if (cpath.nr) {
struct string_list_item *item;

string_list_sort(&cpath);
if (state->apply_verbosity > verbosity_silent) {
for_each_string_list_item(item, &cpath)
fprintf(stderr, "U %s\n", item->string);
}
string_list_clear(&cpath, 0);

repo_rerere(state->repo, 0);
}

return errs;
}










static int apply_patch(struct apply_state *state,
int fd,
const char *filename,
int options)
{
size_t offset;
struct strbuf buf = STRBUF_INIT; 
struct patch *list = NULL, **listp = &list;
int skipped_patch = 0;
int res = 0;
int flush_attributes = 0;

state->patch_input_file = filename;
if (read_patch_file(&buf, fd) < 0)
return -128;
offset = 0;
while (offset < buf.len) {
struct patch *patch;
int nr;

patch = xcalloc(1, sizeof(*patch));
patch->inaccurate_eof = !!(options & APPLY_OPT_INACCURATE_EOF);
patch->recount = !!(options & APPLY_OPT_RECOUNT);
nr = parse_chunk(state, buf.buf + offset, buf.len - offset, patch);
if (nr < 0) {
free_patch(patch);
if (nr == -128) {
res = -128;
goto end;
}
break;
}
if (state->apply_in_reverse)
reverse_patches(patch);
if (use_patch(state, patch)) {
patch_stats(state, patch);
*listp = patch;
listp = &patch->next;

if ((patch->new_name &&
ends_with_path_components(patch->new_name,
GITATTRIBUTES_FILE)) ||
(patch->old_name &&
ends_with_path_components(patch->old_name,
GITATTRIBUTES_FILE)))
flush_attributes = 1;
}
else {
if (state->apply_verbosity > verbosity_normal)
say_patch_name(stderr, _("Skipped patch '%s'."), patch);
free_patch(patch);
skipped_patch++;
}
offset += nr;
}

if (!list && !skipped_patch) {
error(_("unrecognized input"));
res = -128;
goto end;
}

if (state->whitespace_error && (state->ws_error_action == die_on_ws_error))
state->apply = 0;

state->update_index = (state->check_index || state->ita_only) && state->apply;
if (state->update_index && !is_lock_file_locked(&state->lock_file)) {
if (state->index_file)
hold_lock_file_for_update(&state->lock_file,
state->index_file,
LOCK_DIE_ON_ERROR);
else
repo_hold_locked_index(state->repo, &state->lock_file,
LOCK_DIE_ON_ERROR);
}

if (state->check_index && read_apply_cache(state) < 0) {
error(_("unable to read index file"));
res = -128;
goto end;
}

if (state->check || state->apply) {
int r = check_patch_list(state, list);
if (r == -128) {
res = -128;
goto end;
}
if (r < 0 && !state->apply_with_reject) {
res = -1;
goto end;
}
}

if (state->apply) {
int write_res = write_out_results(state, list);
if (write_res < 0) {
res = -128;
goto end;
}
if (write_res > 0) {

res = state->apply_with_reject ? -1 : 1;
goto end;
}
}

if (state->fake_ancestor &&
build_fake_ancestor(state, list)) {
res = -128;
goto end;
}

if (state->diffstat && state->apply_verbosity > verbosity_silent)
stat_patch_list(state, list);

if (state->numstat && state->apply_verbosity > verbosity_silent)
numstat_patch_list(state, list);

if (state->summary && state->apply_verbosity > verbosity_silent)
summary_patch_list(list);

if (flush_attributes)
reset_parsed_attributes();
end:
free_patch_list(list);
strbuf_release(&buf);
string_list_clear(&state->fn_table, 0);
return res;
}

static int apply_option_parse_exclude(const struct option *opt,
const char *arg, int unset)
{
struct apply_state *state = opt->value;

BUG_ON_OPT_NEG(unset);

add_name_limit(state, arg, 1);
return 0;
}

static int apply_option_parse_include(const struct option *opt,
const char *arg, int unset)
{
struct apply_state *state = opt->value;

BUG_ON_OPT_NEG(unset);

add_name_limit(state, arg, 0);
state->has_include = 1;
return 0;
}

static int apply_option_parse_p(const struct option *opt,
const char *arg,
int unset)
{
struct apply_state *state = opt->value;

BUG_ON_OPT_NEG(unset);

state->p_value = atoi(arg);
state->p_value_known = 1;
return 0;
}

static int apply_option_parse_space_change(const struct option *opt,
const char *arg, int unset)
{
struct apply_state *state = opt->value;

BUG_ON_OPT_ARG(arg);

if (unset)
state->ws_ignore_action = ignore_ws_none;
else
state->ws_ignore_action = ignore_ws_change;
return 0;
}

static int apply_option_parse_whitespace(const struct option *opt,
const char *arg, int unset)
{
struct apply_state *state = opt->value;

BUG_ON_OPT_NEG(unset);

state->whitespace_option = arg;
if (parse_whitespace_option(state, arg))
return -1;
return 0;
}

static int apply_option_parse_directory(const struct option *opt,
const char *arg, int unset)
{
struct apply_state *state = opt->value;

BUG_ON_OPT_NEG(unset);

strbuf_reset(&state->root);
strbuf_addstr(&state->root, arg);
strbuf_complete(&state->root, '/');
return 0;
}

int apply_all_patches(struct apply_state *state,
int argc,
const char **argv,
int options)
{
int i;
int res;
int errs = 0;
int read_stdin = 1;

for (i = 0; i < argc; i++) {
const char *arg = argv[i];
char *to_free = NULL;
int fd;

if (!strcmp(arg, "-")) {
res = apply_patch(state, 0, "<stdin>", options);
if (res < 0)
goto end;
errs |= res;
read_stdin = 0;
continue;
} else
arg = to_free = prefix_filename(state->prefix, arg);

fd = open(arg, O_RDONLY);
if (fd < 0) {
error(_("can't open patch '%s': %s"), arg, strerror(errno));
res = -128;
free(to_free);
goto end;
}
read_stdin = 0;
set_default_whitespace_mode(state);
res = apply_patch(state, fd, arg, options);
close(fd);
free(to_free);
if (res < 0)
goto end;
errs |= res;
}
set_default_whitespace_mode(state);
if (read_stdin) {
res = apply_patch(state, 0, "<stdin>", options);
if (res < 0)
goto end;
errs |= res;
}

if (state->whitespace_error) {
if (state->squelch_whitespace_errors &&
state->squelch_whitespace_errors < state->whitespace_error) {
int squelched =
state->whitespace_error - state->squelch_whitespace_errors;
warning(Q_("squelched %d whitespace error",
"squelched %d whitespace errors",
squelched),
squelched);
}
if (state->ws_error_action == die_on_ws_error) {
error(Q_("%d line adds whitespace errors.",
"%d lines add whitespace errors.",
state->whitespace_error),
state->whitespace_error);
res = -128;
goto end;
}
if (state->applied_after_fixing_ws && state->apply)
warning(Q_("%d line applied after"
" fixing whitespace errors.",
"%d lines applied after"
" fixing whitespace errors.",
state->applied_after_fixing_ws),
state->applied_after_fixing_ws);
else if (state->whitespace_error)
warning(Q_("%d line adds whitespace errors.",
"%d lines add whitespace errors.",
state->whitespace_error),
state->whitespace_error);
}

if (state->update_index) {
res = write_locked_index(state->repo->index, &state->lock_file, COMMIT_LOCK);
if (res) {
error(_("Unable to write new index file"));
res = -128;
goto end;
}
}

res = !!errs;

end:
rollback_lock_file(&state->lock_file);

if (state->apply_verbosity <= verbosity_silent) {
set_error_routine(state->saved_error_routine);
set_warn_routine(state->saved_warn_routine);
}

if (res > -1)
return res;
return (res == -1 ? 1 : 128);
}

int apply_parse_options(int argc, const char **argv,
struct apply_state *state,
int *force_apply, int *options,
const char * const *apply_usage)
{
struct option builtin_apply_options[] = {
{ OPTION_CALLBACK, 0, "exclude", state, N_("path"),
N_("don't apply changes matching the given path"),
PARSE_OPT_NONEG, apply_option_parse_exclude },
{ OPTION_CALLBACK, 0, "include", state, N_("path"),
N_("apply changes matching the given path"),
PARSE_OPT_NONEG, apply_option_parse_include },
{ OPTION_CALLBACK, 'p', NULL, state, N_("num"),
N_("remove <num> leading slashes from traditional diff paths"),
0, apply_option_parse_p },
OPT_BOOL(0, "no-add", &state->no_add,
N_("ignore additions made by the patch")),
OPT_BOOL(0, "stat", &state->diffstat,
N_("instead of applying the patch, output diffstat for the input")),
OPT_NOOP_NOARG(0, "allow-binary-replacement"),
OPT_NOOP_NOARG(0, "binary"),
OPT_BOOL(0, "numstat", &state->numstat,
N_("show number of added and deleted lines in decimal notation")),
OPT_BOOL(0, "summary", &state->summary,
N_("instead of applying the patch, output a summary for the input")),
OPT_BOOL(0, "check", &state->check,
N_("instead of applying the patch, see if the patch is applicable")),
OPT_BOOL(0, "index", &state->check_index,
N_("make sure the patch is applicable to the current index")),
OPT_BOOL('N', "intent-to-add", &state->ita_only,
N_("mark new files with `git add --intent-to-add`")),
OPT_BOOL(0, "cached", &state->cached,
N_("apply a patch without touching the working tree")),
OPT_BOOL_F(0, "unsafe-paths", &state->unsafe_paths,
N_("accept a patch that touches outside the working area"),
PARSE_OPT_NOCOMPLETE),
OPT_BOOL(0, "apply", force_apply,
N_("also apply the patch (use with --stat/--summary/--check)")),
OPT_BOOL('3', "3way", &state->threeway,
N_( "attempt three-way merge if a patch does not apply")),
OPT_FILENAME(0, "build-fake-ancestor", &state->fake_ancestor,
N_("build a temporary index based on embedded index information")),

OPT_SET_INT('z', NULL, &state->line_termination,
N_("paths are separated with NUL character"), '\0'),
OPT_INTEGER('C', NULL, &state->p_context,
N_("ensure at least <n> lines of context match")),
{ OPTION_CALLBACK, 0, "whitespace", state, N_("action"),
N_("detect new or modified lines that have whitespace errors"),
0, apply_option_parse_whitespace },
{ OPTION_CALLBACK, 0, "ignore-space-change", state, NULL,
N_("ignore changes in whitespace when finding context"),
PARSE_OPT_NOARG, apply_option_parse_space_change },
{ OPTION_CALLBACK, 0, "ignore-whitespace", state, NULL,
N_("ignore changes in whitespace when finding context"),
PARSE_OPT_NOARG, apply_option_parse_space_change },
OPT_BOOL('R', "reverse", &state->apply_in_reverse,
N_("apply the patch in reverse")),
OPT_BOOL(0, "unidiff-zero", &state->unidiff_zero,
N_("don't expect at least one line of context")),
OPT_BOOL(0, "reject", &state->apply_with_reject,
N_("leave the rejected hunks in corresponding *.rej files")),
OPT_BOOL(0, "allow-overlap", &state->allow_overlap,
N_("allow overlapping hunks")),
OPT__VERBOSE(&state->apply_verbosity, N_("be verbose")),
OPT_BIT(0, "inaccurate-eof", options,
N_("tolerate incorrectly detected missing new-line at the end of file"),
APPLY_OPT_INACCURATE_EOF),
OPT_BIT(0, "recount", options,
N_("do not trust the line counts in the hunk headers"),
APPLY_OPT_RECOUNT),
{ OPTION_CALLBACK, 0, "directory", state, N_("root"),
N_("prepend <root> to all filenames"),
0, apply_option_parse_directory },
OPT_END()
};

return parse_options(argc, argv, state->prefix, builtin_apply_options, apply_usage, 0);
}
