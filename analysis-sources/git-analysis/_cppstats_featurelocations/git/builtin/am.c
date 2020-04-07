




#define USE_THE_INDEX_COMPATIBILITY_MACROS
#include "cache.h"
#include "config.h"
#include "builtin.h"
#include "exec-cmd.h"
#include "parse-options.h"
#include "dir.h"
#include "run-command.h"
#include "quote.h"
#include "tempfile.h"
#include "lockfile.h"
#include "cache-tree.h"
#include "refs.h"
#include "commit.h"
#include "diff.h"
#include "diffcore.h"
#include "unpack-trees.h"
#include "branch.h"
#include "sequencer.h"
#include "revision.h"
#include "merge-recursive.h"
#include "log-tree.h"
#include "notes-utils.h"
#include "rerere.h"
#include "prompt.h"
#include "mailinfo.h"
#include "apply.h"
#include "string-list.h"
#include "packfile.h"
#include "repository.h"




static int linelen(const char *msg)
{
return strchrnul(msg, '\n') - msg;
}




static int str_isspace(const char *str)
{
for (; *str; str++)
if (!isspace(*str))
return 0;

return 1;
}

enum patch_format {
PATCH_FORMAT_UNKNOWN = 0,
PATCH_FORMAT_MBOX,
PATCH_FORMAT_STGIT,
PATCH_FORMAT_STGIT_SERIES,
PATCH_FORMAT_HG,
PATCH_FORMAT_MBOXRD
};

enum keep_type {
KEEP_FALSE = 0,
KEEP_TRUE, 
KEEP_NON_PATCH 
};

enum scissors_type {
SCISSORS_UNSET = -1,
SCISSORS_FALSE = 0, 
SCISSORS_TRUE 
};

enum signoff_type {
SIGNOFF_FALSE = 0,
SIGNOFF_TRUE = 1,
SIGNOFF_EXPLICIT 
};

enum show_patch_type {
SHOW_PATCH_RAW = 0,
SHOW_PATCH_DIFF = 1,
};

struct am_state {

char *dir;


int cur;
int last;


char *author_name;
char *author_email;
char *author_date;
char *msg;
size_t msg_len;


struct object_id orig_commit;


int prec;


int interactive;
int threeway;
int quiet;
int signoff; 
int utf8;
int keep; 
int message_id;
int scissors; 
struct argv_array git_apply_opts;
const char *resolvemsg;
int committer_date_is_author_date;
int ignore_date;
int allow_rerere_autoupdate;
const char *sign_commit;
int rebasing;
};




static void am_state_init(struct am_state *state)
{
int gpgsign;

memset(state, 0, sizeof(*state));

state->dir = git_pathdup("rebase-apply");

state->prec = 4;

git_config_get_bool("am.threeway", &state->threeway);

state->utf8 = 1;

git_config_get_bool("am.messageid", &state->message_id);

state->scissors = SCISSORS_UNSET;

argv_array_init(&state->git_apply_opts);

if (!git_config_get_bool("commit.gpgsign", &gpgsign))
state->sign_commit = gpgsign ? "" : NULL;
}




static void am_state_release(struct am_state *state)
{
free(state->dir);
free(state->author_name);
free(state->author_email);
free(state->author_date);
free(state->msg);
argv_array_clear(&state->git_apply_opts);
}




static inline const char *am_path(const struct am_state *state, const char *path)
{
return mkpath("%s/%s", state->dir, path);
}




static void write_state_text(const struct am_state *state,
const char *name, const char *string)
{
write_file(am_path(state, name), "%s", string);
}

static void write_state_count(const struct am_state *state,
const char *name, int value)
{
write_file(am_path(state, name), "%d", value);
}

static void write_state_bool(const struct am_state *state,
const char *name, int value)
{
write_state_text(state, name, value ? "t" : "f");
}





static void say(const struct am_state *state, FILE *fp, const char *fmt, ...)
{
va_list ap;

va_start(ap, fmt);
if (!state->quiet) {
vfprintf(fp, fmt, ap);
putc('\n', fp);
}
va_end(ap);
}




static int am_in_progress(const struct am_state *state)
{
struct stat st;

if (lstat(state->dir, &st) < 0 || !S_ISDIR(st.st_mode))
return 0;
if (lstat(am_path(state, "last"), &st) || !S_ISREG(st.st_mode))
return 0;
if (lstat(am_path(state, "next"), &st) || !S_ISREG(st.st_mode))
return 0;
return 1;
}






static int read_state_file(struct strbuf *sb, const struct am_state *state,
const char *file, int trim)
{
strbuf_reset(sb);

if (strbuf_read_file(sb, am_path(state, file), 0) >= 0) {
if (trim)
strbuf_trim(sb);

return sb->len;
}

if (errno == ENOENT)
return -1;

die_errno(_("could not read '%s'"), am_path(state, file));
}

















static int read_am_author_script(struct am_state *state)
{
const char *filename = am_path(state, "author-script");

assert(!state->author_name);
assert(!state->author_email);
assert(!state->author_date);

return read_author_script(filename, &state->author_name,
&state->author_email, &state->author_date, 1);
}





static void write_author_script(const struct am_state *state)
{
struct strbuf sb = STRBUF_INIT;

strbuf_addstr(&sb, "GIT_AUTHOR_NAME=");
sq_quote_buf(&sb, state->author_name);
strbuf_addch(&sb, '\n');

strbuf_addstr(&sb, "GIT_AUTHOR_EMAIL=");
sq_quote_buf(&sb, state->author_email);
strbuf_addch(&sb, '\n');

strbuf_addstr(&sb, "GIT_AUTHOR_DATE=");
sq_quote_buf(&sb, state->author_date);
strbuf_addch(&sb, '\n');

write_state_text(state, "author-script", sb.buf);

strbuf_release(&sb);
}








static int read_commit_msg(struct am_state *state)
{
struct strbuf sb = STRBUF_INIT;

assert(!state->msg);

if (read_state_file(&sb, state, "final-commit", 0) < 0) {
strbuf_release(&sb);
return -1;
}

state->msg = strbuf_detach(&sb, &state->msg_len);
return 0;
}




static void write_commit_msg(const struct am_state *state)
{
const char *filename = am_path(state, "final-commit");
write_file_buf(filename, state->msg, state->msg_len);
}




static void am_load(struct am_state *state)
{
struct strbuf sb = STRBUF_INIT;

if (read_state_file(&sb, state, "next", 1) < 0)
BUG("state file 'next' does not exist");
state->cur = strtol(sb.buf, NULL, 10);

if (read_state_file(&sb, state, "last", 1) < 0)
BUG("state file 'last' does not exist");
state->last = strtol(sb.buf, NULL, 10);

if (read_am_author_script(state) < 0)
die(_("could not parse author script"));

read_commit_msg(state);

if (read_state_file(&sb, state, "original-commit", 1) < 0)
oidclr(&state->orig_commit);
else if (get_oid_hex(sb.buf, &state->orig_commit) < 0)
die(_("could not parse %s"), am_path(state, "original-commit"));

read_state_file(&sb, state, "threeway", 1);
state->threeway = !strcmp(sb.buf, "t");

read_state_file(&sb, state, "quiet", 1);
state->quiet = !strcmp(sb.buf, "t");

read_state_file(&sb, state, "sign", 1);
state->signoff = !strcmp(sb.buf, "t");

read_state_file(&sb, state, "utf8", 1);
state->utf8 = !strcmp(sb.buf, "t");

if (file_exists(am_path(state, "rerere-autoupdate"))) {
read_state_file(&sb, state, "rerere-autoupdate", 1);
state->allow_rerere_autoupdate = strcmp(sb.buf, "t") ?
RERERE_NOAUTOUPDATE : RERERE_AUTOUPDATE;
} else {
state->allow_rerere_autoupdate = 0;
}

read_state_file(&sb, state, "keep", 1);
if (!strcmp(sb.buf, "t"))
state->keep = KEEP_TRUE;
else if (!strcmp(sb.buf, "b"))
state->keep = KEEP_NON_PATCH;
else
state->keep = KEEP_FALSE;

read_state_file(&sb, state, "messageid", 1);
state->message_id = !strcmp(sb.buf, "t");

read_state_file(&sb, state, "scissors", 1);
if (!strcmp(sb.buf, "t"))
state->scissors = SCISSORS_TRUE;
else if (!strcmp(sb.buf, "f"))
state->scissors = SCISSORS_FALSE;
else
state->scissors = SCISSORS_UNSET;

read_state_file(&sb, state, "apply-opt", 1);
argv_array_clear(&state->git_apply_opts);
if (sq_dequote_to_argv_array(sb.buf, &state->git_apply_opts) < 0)
die(_("could not parse %s"), am_path(state, "apply-opt"));

state->rebasing = !!file_exists(am_path(state, "rebasing"));

strbuf_release(&sb);
}





static void am_destroy(const struct am_state *state)
{
struct strbuf sb = STRBUF_INIT;

strbuf_addstr(&sb, state->dir);
remove_dir_recursively(&sb, 0);
strbuf_release(&sb);
}




static int run_applypatch_msg_hook(struct am_state *state)
{
int ret;

assert(state->msg);
ret = run_hook_le(NULL, "applypatch-msg", am_path(state, "final-commit"), NULL);

if (!ret) {
FREE_AND_NULL(state->msg);
if (read_commit_msg(state) < 0)
die(_("'%s' was deleted by the applypatch-msg hook"),
am_path(state, "final-commit"));
}

return ret;
}




static int run_post_rewrite_hook(const struct am_state *state)
{
struct child_process cp = CHILD_PROCESS_INIT;
const char *hook = find_hook("post-rewrite");
int ret;

if (!hook)
return 0;

argv_array_push(&cp.args, hook);
argv_array_push(&cp.args, "rebase");

cp.in = xopen(am_path(state, "rewritten"), O_RDONLY);
cp.stdout_to_stderr = 1;
cp.trace2_hook_name = "post-rewrite";

ret = run_command(&cp);

close(cp.in);
return ret;
}







static int copy_notes_for_rebase(const struct am_state *state)
{
struct notes_rewrite_cfg *c;
struct strbuf sb = STRBUF_INIT;
const char *invalid_line = _("Malformed input line: '%s'.");
const char *msg = "Notes added by 'git rebase'";
FILE *fp;
int ret = 0;

assert(state->rebasing);

c = init_copy_notes_for_rewrite("rebase");
if (!c)
return 0;

fp = xfopen(am_path(state, "rewritten"), "r");

while (!strbuf_getline_lf(&sb, fp)) {
struct object_id from_obj, to_obj;
const char *p;

if (sb.len != the_hash_algo->hexsz * 2 + 1) {
ret = error(invalid_line, sb.buf);
goto finish;
}

if (parse_oid_hex(sb.buf, &from_obj, &p)) {
ret = error(invalid_line, sb.buf);
goto finish;
}

if (*p != ' ') {
ret = error(invalid_line, sb.buf);
goto finish;
}

if (get_oid_hex(p + 1, &to_obj)) {
ret = error(invalid_line, sb.buf);
goto finish;
}

if (copy_note_for_rewrite(c, &from_obj, &to_obj))
ret = error(_("Failed to copy notes from '%s' to '%s'"),
oid_to_hex(&from_obj), oid_to_hex(&to_obj));
}

finish:
finish_copy_notes_for_rewrite(the_repository, c, msg);
fclose(fp);
strbuf_release(&sb);
return ret;
}








static int is_mail(FILE *fp)
{
const char *header_regex = "^[!-9;-~]+:";
struct strbuf sb = STRBUF_INIT;
regex_t regex;
int ret = 1;

if (fseek(fp, 0L, SEEK_SET))
die_errno(_("fseek failed"));

if (regcomp(&regex, header_regex, REG_NOSUB | REG_EXTENDED))
die("invalid pattern: %s", header_regex);

while (!strbuf_getline(&sb, fp)) {
if (!sb.len)
break; 


if (*sb.buf == '\t' || *sb.buf == ' ')
continue;


if (regexec(&regex, sb.buf, 0, NULL, 0)) {
ret = 0;
goto done;
}
}

done:
regfree(&regex);
strbuf_release(&sb);
return ret;
}






static int detect_patch_format(const char **paths)
{
enum patch_format ret = PATCH_FORMAT_UNKNOWN;
struct strbuf l1 = STRBUF_INIT;
struct strbuf l2 = STRBUF_INIT;
struct strbuf l3 = STRBUF_INIT;
FILE *fp;




if (!*paths || !strcmp(*paths, "-") || is_directory(*paths))
return PATCH_FORMAT_MBOX;






fp = xfopen(*paths, "r");

while (!strbuf_getline(&l1, fp)) {
if (l1.len)
break;
}

if (starts_with(l1.buf, "From ") || starts_with(l1.buf, "From: ")) {
ret = PATCH_FORMAT_MBOX;
goto done;
}

if (starts_with(l1.buf, "#This series applies on GIT commit")) {
ret = PATCH_FORMAT_STGIT_SERIES;
goto done;
}

if (!strcmp(l1.buf, "#HG changeset patch")) {
ret = PATCH_FORMAT_HG;
goto done;
}

strbuf_getline(&l2, fp);
strbuf_getline(&l3, fp);





if (l1.len && !l2.len &&
(starts_with(l3.buf, "From:") ||
starts_with(l3.buf, "Author:") ||
starts_with(l3.buf, "Date:"))) {
ret = PATCH_FORMAT_STGIT;
goto done;
}

if (l1.len && is_mail(fp)) {
ret = PATCH_FORMAT_MBOX;
goto done;
}

done:
fclose(fp);
strbuf_release(&l1);
strbuf_release(&l2);
strbuf_release(&l3);
return ret;
}





static int split_mail_mbox(struct am_state *state, const char **paths,
int keep_cr, int mboxrd)
{
struct child_process cp = CHILD_PROCESS_INIT;
struct strbuf last = STRBUF_INIT;
int ret;

cp.git_cmd = 1;
argv_array_push(&cp.args, "mailsplit");
argv_array_pushf(&cp.args, "-d%d", state->prec);
argv_array_pushf(&cp.args, "-o%s", state->dir);
argv_array_push(&cp.args, "-b");
if (keep_cr)
argv_array_push(&cp.args, "--keep-cr");
if (mboxrd)
argv_array_push(&cp.args, "--mboxrd");
argv_array_push(&cp.args, "--");
argv_array_pushv(&cp.args, paths);

ret = capture_command(&cp, &last, 8);
if (ret)
goto exit;

state->cur = 1;
state->last = strtol(last.buf, NULL, 10);

exit:
strbuf_release(&last);
return ret ? -1 : 0;
}






typedef int (*mail_conv_fn)(FILE *out, FILE *in, int keep_cr);







static int split_mail_conv(mail_conv_fn fn, struct am_state *state,
const char **paths, int keep_cr)
{
static const char *stdin_only[] = {"-", NULL};
int i;

if (!*paths)
paths = stdin_only;

for (i = 0; *paths; paths++, i++) {
FILE *in, *out;
const char *mail;
int ret;

if (!strcmp(*paths, "-"))
in = stdin;
else
in = fopen(*paths, "r");

if (!in)
return error_errno(_("could not open '%s' for reading"),
*paths);

mail = mkpath("%s/%0*d", state->dir, state->prec, i + 1);

out = fopen(mail, "w");
if (!out) {
if (in != stdin)
fclose(in);
return error_errno(_("could not open '%s' for writing"),
mail);
}

ret = fn(out, in, keep_cr);

fclose(out);
if (in != stdin)
fclose(in);

if (ret)
return error(_("could not parse patch '%s'"), *paths);
}

state->cur = 1;
state->last = i;
return 0;
}





static int stgit_patch_to_mail(FILE *out, FILE *in, int keep_cr)
{
struct strbuf sb = STRBUF_INIT;
int subject_printed = 0;

while (!strbuf_getline_lf(&sb, in)) {
const char *str;

if (str_isspace(sb.buf))
continue;
else if (skip_prefix(sb.buf, "Author:", &str))
fprintf(out, "From:%s\n", str);
else if (starts_with(sb.buf, "From") || starts_with(sb.buf, "Date"))
fprintf(out, "%s\n", sb.buf);
else if (!subject_printed) {
fprintf(out, "Subject: %s\n", sb.buf);
subject_printed = 1;
} else {
fprintf(out, "\n%s\n", sb.buf);
break;
}
}

strbuf_reset(&sb);
while (strbuf_fread(&sb, 8192, in) > 0) {
fwrite(sb.buf, 1, sb.len, out);
strbuf_reset(&sb);
}

strbuf_release(&sb);
return 0;
}










static int split_mail_stgit_series(struct am_state *state, const char **paths,
int keep_cr)
{
const char *series_dir;
char *series_dir_buf;
FILE *fp;
struct argv_array patches = ARGV_ARRAY_INIT;
struct strbuf sb = STRBUF_INIT;
int ret;

if (!paths[0] || paths[1])
return error(_("Only one StGIT patch series can be applied at once"));

series_dir_buf = xstrdup(*paths);
series_dir = dirname(series_dir_buf);

fp = fopen(*paths, "r");
if (!fp)
return error_errno(_("could not open '%s' for reading"), *paths);

while (!strbuf_getline_lf(&sb, fp)) {
if (*sb.buf == '#')
continue; 

argv_array_push(&patches, mkpath("%s/%s", series_dir, sb.buf));
}

fclose(fp);
strbuf_release(&sb);
free(series_dir_buf);

ret = split_mail_conv(stgit_patch_to_mail, state, patches.argv, keep_cr);

argv_array_clear(&patches);
return ret;
}





static int hg_patch_to_mail(FILE *out, FILE *in, int keep_cr)
{
struct strbuf sb = STRBUF_INIT;
int rc = 0;

while (!strbuf_getline_lf(&sb, in)) {
const char *str;

if (skip_prefix(sb.buf, "#User ", &str))
fprintf(out, "From: %s\n", str);
else if (skip_prefix(sb.buf, "#Date ", &str)) {
timestamp_t timestamp;
long tz, tz2;
char *end;

errno = 0;
timestamp = parse_timestamp(str, &end, 10);
if (errno) {
rc = error(_("invalid timestamp"));
goto exit;
}

if (!skip_prefix(end, " ", &str)) {
rc = error(_("invalid Date line"));
goto exit;
}

errno = 0;
tz = strtol(str, &end, 10);
if (errno) {
rc = error(_("invalid timezone offset"));
goto exit;
}

if (*end) {
rc = error(_("invalid Date line"));
goto exit;
}






tz2 = labs(tz) / 3600 * 100 + labs(tz) % 3600 / 60;
if (tz > 0)
tz2 = -tz2;

fprintf(out, "Date: %s\n", show_date(timestamp, tz2, DATE_MODE(RFC2822)));
} else if (starts_with(sb.buf, "#")) {
continue;
} else {
fprintf(out, "\n%s\n", sb.buf);
break;
}
}

strbuf_reset(&sb);
while (strbuf_fread(&sb, 8192, in) > 0) {
fwrite(sb.buf, 1, sb.len, out);
strbuf_reset(&sb);
}
exit:
strbuf_release(&sb);
return rc;
}


















static int split_mail(struct am_state *state, enum patch_format patch_format,
const char **paths, int keep_cr)
{
if (keep_cr < 0) {
keep_cr = 0;
git_config_get_bool("am.keepcr", &keep_cr);
}

switch (patch_format) {
case PATCH_FORMAT_MBOX:
return split_mail_mbox(state, paths, keep_cr, 0);
case PATCH_FORMAT_STGIT:
return split_mail_conv(stgit_patch_to_mail, state, paths, keep_cr);
case PATCH_FORMAT_STGIT_SERIES:
return split_mail_stgit_series(state, paths, keep_cr);
case PATCH_FORMAT_HG:
return split_mail_conv(hg_patch_to_mail, state, paths, keep_cr);
case PATCH_FORMAT_MBOXRD:
return split_mail_mbox(state, paths, keep_cr, 1);
default:
BUG("invalid patch_format");
}
return -1;
}




static void am_setup(struct am_state *state, enum patch_format patch_format,
const char **paths, int keep_cr)
{
struct object_id curr_head;
const char *str;
struct strbuf sb = STRBUF_INIT;

if (!patch_format)
patch_format = detect_patch_format(paths);

if (!patch_format) {
fprintf_ln(stderr, _("Patch format detection failed."));
exit(128);
}

if (mkdir(state->dir, 0777) < 0 && errno != EEXIST)
die_errno(_("failed to create directory '%s'"), state->dir);
delete_ref(NULL, "REBASE_HEAD", NULL, REF_NO_DEREF);

if (split_mail(state, patch_format, paths, keep_cr) < 0) {
am_destroy(state);
die(_("Failed to split patches."));
}

if (state->rebasing)
state->threeway = 1;

write_state_bool(state, "threeway", state->threeway);
write_state_bool(state, "quiet", state->quiet);
write_state_bool(state, "sign", state->signoff);
write_state_bool(state, "utf8", state->utf8);

if (state->allow_rerere_autoupdate)
write_state_bool(state, "rerere-autoupdate",
state->allow_rerere_autoupdate == RERERE_AUTOUPDATE);

switch (state->keep) {
case KEEP_FALSE:
str = "f";
break;
case KEEP_TRUE:
str = "t";
break;
case KEEP_NON_PATCH:
str = "b";
break;
default:
BUG("invalid value for state->keep");
}

write_state_text(state, "keep", str);
write_state_bool(state, "messageid", state->message_id);

switch (state->scissors) {
case SCISSORS_UNSET:
str = "";
break;
case SCISSORS_FALSE:
str = "f";
break;
case SCISSORS_TRUE:
str = "t";
break;
default:
BUG("invalid value for state->scissors");
}
write_state_text(state, "scissors", str);

sq_quote_argv(&sb, state->git_apply_opts.argv);
write_state_text(state, "apply-opt", sb.buf);

if (state->rebasing)
write_state_text(state, "rebasing", "");
else
write_state_text(state, "applying", "");

if (!get_oid("HEAD", &curr_head)) {
write_state_text(state, "abort-safety", oid_to_hex(&curr_head));
if (!state->rebasing)
update_ref("am", "ORIG_HEAD", &curr_head, NULL, 0,
UPDATE_REFS_DIE_ON_ERR);
} else {
write_state_text(state, "abort-safety", "");
if (!state->rebasing)
delete_ref(NULL, "ORIG_HEAD", NULL, 0);
}






write_state_count(state, "next", state->cur);
write_state_count(state, "last", state->last);

strbuf_release(&sb);
}





static void am_next(struct am_state *state)
{
struct object_id head;

FREE_AND_NULL(state->author_name);
FREE_AND_NULL(state->author_email);
FREE_AND_NULL(state->author_date);
FREE_AND_NULL(state->msg);
state->msg_len = 0;

unlink(am_path(state, "author-script"));
unlink(am_path(state, "final-commit"));

oidclr(&state->orig_commit);
unlink(am_path(state, "original-commit"));
delete_ref(NULL, "REBASE_HEAD", NULL, REF_NO_DEREF);

if (!get_oid("HEAD", &head))
write_state_text(state, "abort-safety", oid_to_hex(&head));
else
write_state_text(state, "abort-safety", "");

state->cur++;
write_state_count(state, "next", state->cur);
}




static const char *msgnum(const struct am_state *state)
{
static struct strbuf sb = STRBUF_INIT;

strbuf_reset(&sb);
strbuf_addf(&sb, "%0*d", state->prec, state->cur);

return sb.buf;
}





static void NORETURN die_user_resolve(const struct am_state *state)
{
if (state->resolvemsg) {
printf_ln("%s", state->resolvemsg);
} else {
const char *cmdline = state->interactive ? "git am -i" : "git am";

printf_ln(_("When you have resolved this problem, run \"%s --continue\"."), cmdline);
printf_ln(_("If you prefer to skip this patch, run \"%s --skip\" instead."), cmdline);
printf_ln(_("To restore the original branch and stop patching, run \"%s --abort\"."), cmdline);
}

exit(128);
}




static void am_append_signoff(struct am_state *state)
{
struct strbuf sb = STRBUF_INIT;

strbuf_attach(&sb, state->msg, state->msg_len, state->msg_len);
append_signoff(&sb, 0, 0);
state->msg = strbuf_detach(&sb, &state->msg_len);
}










static int parse_mail(struct am_state *state, const char *mail)
{
FILE *fp;
struct strbuf sb = STRBUF_INIT;
struct strbuf msg = STRBUF_INIT;
struct strbuf author_name = STRBUF_INIT;
struct strbuf author_date = STRBUF_INIT;
struct strbuf author_email = STRBUF_INIT;
int ret = 0;
struct mailinfo mi;

setup_mailinfo(&mi);

if (state->utf8)
mi.metainfo_charset = get_commit_output_encoding();
else
mi.metainfo_charset = NULL;

switch (state->keep) {
case KEEP_FALSE:
break;
case KEEP_TRUE:
mi.keep_subject = 1;
break;
case KEEP_NON_PATCH:
mi.keep_non_patch_brackets_in_subject = 1;
break;
default:
BUG("invalid value for state->keep");
}

if (state->message_id)
mi.add_message_id = 1;

switch (state->scissors) {
case SCISSORS_UNSET:
break;
case SCISSORS_FALSE:
mi.use_scissors = 0;
break;
case SCISSORS_TRUE:
mi.use_scissors = 1;
break;
default:
BUG("invalid value for state->scissors");
}

mi.input = xfopen(mail, "r");
mi.output = xfopen(am_path(state, "info"), "w");
if (mailinfo(&mi, am_path(state, "msg"), am_path(state, "patch")))
die("could not parse patch");

fclose(mi.input);
fclose(mi.output);

if (mi.format_flowed)
warning(_("Patch sent with format=flowed; "
"space at the end of lines might be lost."));


fp = xfopen(am_path(state, "info"), "r");
while (!strbuf_getline_lf(&sb, fp)) {
const char *x;

if (skip_prefix(sb.buf, "Subject: ", &x)) {
if (msg.len)
strbuf_addch(&msg, '\n');
strbuf_addstr(&msg, x);
} else if (skip_prefix(sb.buf, "Author: ", &x))
strbuf_addstr(&author_name, x);
else if (skip_prefix(sb.buf, "Email: ", &x))
strbuf_addstr(&author_email, x);
else if (skip_prefix(sb.buf, "Date: ", &x))
strbuf_addstr(&author_date, x);
}
fclose(fp);


if (!strcmp(author_name.buf, "Mail System Internal Data")) {
ret = 1;
goto finish;
}

if (is_empty_or_missing_file(am_path(state, "patch"))) {
printf_ln(_("Patch is empty."));
die_user_resolve(state);
}

strbuf_addstr(&msg, "\n\n");
strbuf_addbuf(&msg, &mi.log_message);
strbuf_stripspace(&msg, 0);

assert(!state->author_name);
state->author_name = strbuf_detach(&author_name, NULL);

assert(!state->author_email);
state->author_email = strbuf_detach(&author_email, NULL);

assert(!state->author_date);
state->author_date = strbuf_detach(&author_date, NULL);

assert(!state->msg);
state->msg = strbuf_detach(&msg, &state->msg_len);

finish:
strbuf_release(&msg);
strbuf_release(&author_date);
strbuf_release(&author_email);
strbuf_release(&author_name);
strbuf_release(&sb);
clear_mailinfo(&mi);
return ret;
}





static int get_mail_commit_oid(struct object_id *commit_id, const char *mail)
{
struct strbuf sb = STRBUF_INIT;
FILE *fp = xfopen(mail, "r");
const char *x;
int ret = 0;

if (strbuf_getline_lf(&sb, fp) ||
!skip_prefix(sb.buf, "From ", &x) ||
get_oid_hex(x, commit_id) < 0)
ret = -1;

strbuf_release(&sb);
fclose(fp);
return ret;
}





static void get_commit_info(struct am_state *state, struct commit *commit)
{
const char *buffer, *ident_line, *msg;
size_t ident_len;
struct ident_split id;

buffer = logmsg_reencode(commit, NULL, get_commit_output_encoding());

ident_line = find_commit_header(buffer, "author", &ident_len);
if (!ident_line)
die(_("missing author line in commit %s"),
oid_to_hex(&commit->object.oid));
if (split_ident_line(&id, ident_line, ident_len) < 0)
die(_("invalid ident line: %.*s"), (int)ident_len, ident_line);

assert(!state->author_name);
if (id.name_begin)
state->author_name =
xmemdupz(id.name_begin, id.name_end - id.name_begin);
else
state->author_name = xstrdup("");

assert(!state->author_email);
if (id.mail_begin)
state->author_email =
xmemdupz(id.mail_begin, id.mail_end - id.mail_begin);
else
state->author_email = xstrdup("");

assert(!state->author_date);
state->author_date = xstrdup(show_ident_date(&id, DATE_MODE(NORMAL)));

assert(!state->msg);
msg = strstr(buffer, "\n\n");
if (!msg)
die(_("unable to parse commit %s"), oid_to_hex(&commit->object.oid));
state->msg = xstrdup(msg + 2);
state->msg_len = strlen(state->msg);
unuse_commit_buffer(commit, buffer);
}




static void write_commit_patch(const struct am_state *state, struct commit *commit)
{
struct rev_info rev_info;
FILE *fp;

fp = xfopen(am_path(state, "patch"), "w");
repo_init_revisions(the_repository, &rev_info, NULL);
rev_info.diff = 1;
rev_info.abbrev = 0;
rev_info.disable_stdin = 1;
rev_info.show_root_diff = 1;
rev_info.diffopt.output_format = DIFF_FORMAT_PATCH;
rev_info.no_commit_id = 1;
rev_info.diffopt.flags.binary = 1;
rev_info.diffopt.flags.full_index = 1;
rev_info.diffopt.use_color = 0;
rev_info.diffopt.file = fp;
rev_info.diffopt.close_file = 1;
add_pending_object(&rev_info, &commit->object, "");
diff_setup_done(&rev_info.diffopt);
log_tree_commit(&rev_info, commit);
}





static void write_index_patch(const struct am_state *state)
{
struct tree *tree;
struct object_id head;
struct rev_info rev_info;
FILE *fp;

if (!get_oid("HEAD", &head)) {
struct commit *commit = lookup_commit_or_die(&head, "HEAD");
tree = get_commit_tree(commit);
} else
tree = lookup_tree(the_repository,
the_repository->hash_algo->empty_tree);

fp = xfopen(am_path(state, "patch"), "w");
repo_init_revisions(the_repository, &rev_info, NULL);
rev_info.diff = 1;
rev_info.disable_stdin = 1;
rev_info.no_commit_id = 1;
rev_info.diffopt.output_format = DIFF_FORMAT_PATCH;
rev_info.diffopt.use_color = 0;
rev_info.diffopt.file = fp;
rev_info.diffopt.close_file = 1;
add_pending_object(&rev_info, &tree->object, "");
diff_setup_done(&rev_info.diffopt);
run_diff_index(&rev_info, 1);
}










static int parse_mail_rebase(struct am_state *state, const char *mail)
{
struct commit *commit;
struct object_id commit_oid;

if (get_mail_commit_oid(&commit_oid, mail) < 0)
die(_("could not parse %s"), mail);

commit = lookup_commit_or_die(&commit_oid, mail);

get_commit_info(state, commit);

write_commit_patch(state, commit);

oidcpy(&state->orig_commit, &commit_oid);
write_state_text(state, "original-commit", oid_to_hex(&commit_oid));
update_ref("am", "REBASE_HEAD", &commit_oid,
NULL, REF_NO_DEREF, UPDATE_REFS_DIE_ON_ERR);

return 0;
}





static int run_apply(const struct am_state *state, const char *index_file)
{
struct argv_array apply_paths = ARGV_ARRAY_INIT;
struct argv_array apply_opts = ARGV_ARRAY_INIT;
struct apply_state apply_state;
int res, opts_left;
int force_apply = 0;
int options = 0;

if (init_apply_state(&apply_state, the_repository, NULL))
BUG("init_apply_state() failed");

argv_array_push(&apply_opts, "apply");
argv_array_pushv(&apply_opts, state->git_apply_opts.argv);

opts_left = apply_parse_options(apply_opts.argc, apply_opts.argv,
&apply_state, &force_apply, &options,
NULL);

if (opts_left != 0)
die("unknown option passed through to git apply");

if (index_file) {
apply_state.index_file = index_file;
apply_state.cached = 1;
} else
apply_state.check_index = 1;





if (state->threeway && !index_file)
apply_state.apply_verbosity = verbosity_silent;

if (check_apply_state(&apply_state, force_apply))
BUG("check_apply_state() failed");

argv_array_push(&apply_paths, am_path(state, "patch"));

res = apply_all_patches(&apply_state, apply_paths.argc, apply_paths.argv, options);

argv_array_clear(&apply_paths);
argv_array_clear(&apply_opts);
clear_apply_state(&apply_state);

if (res)
return res;

if (index_file) {

discard_cache();
read_cache_from(index_file);
}

return 0;
}




static int build_fake_ancestor(const struct am_state *state, const char *index_file)
{
struct child_process cp = CHILD_PROCESS_INIT;

cp.git_cmd = 1;
argv_array_push(&cp.args, "apply");
argv_array_pushv(&cp.args, state->git_apply_opts.argv);
argv_array_pushf(&cp.args, "--build-fake-ancestor=%s", index_file);
argv_array_push(&cp.args, am_path(state, "patch"));

if (run_command(&cp))
return -1;

return 0;
}




static int fall_back_threeway(const struct am_state *state, const char *index_path)
{
struct object_id orig_tree, their_tree, our_tree;
const struct object_id *bases[1] = { &orig_tree };
struct merge_options o;
struct commit *result;
char *their_tree_name;

if (get_oid("HEAD", &our_tree) < 0)
oidcpy(&our_tree, the_hash_algo->empty_tree);

if (build_fake_ancestor(state, index_path))
return error("could not build fake ancestor");

discard_cache();
read_cache_from(index_path);

if (write_index_as_tree(&orig_tree, &the_index, index_path, 0, NULL))
return error(_("Repository lacks necessary blobs to fall back on 3-way merge."));

say(state, stdout, _("Using index info to reconstruct a base tree..."));

if (!state->quiet) {




struct rev_info rev_info;

repo_init_revisions(the_repository, &rev_info, NULL);
rev_info.diffopt.output_format = DIFF_FORMAT_NAME_STATUS;
rev_info.diffopt.filter |= diff_filter_bit('A');
rev_info.diffopt.filter |= diff_filter_bit('M');
add_pending_oid(&rev_info, "HEAD", &our_tree, 0);
diff_setup_done(&rev_info.diffopt);
run_diff_index(&rev_info, 1);
}

if (run_apply(state, index_path))
return error(_("Did you hand edit your patch?\n"
"It does not apply to blobs recorded in its index."));

if (write_index_as_tree(&their_tree, &the_index, index_path, 0, NULL))
return error("could not write tree");

say(state, stdout, _("Falling back to patching base and 3-way merge..."));

discard_cache();
read_cache();









init_merge_options(&o, the_repository);

o.branch1 = "HEAD";
their_tree_name = xstrfmt("%.*s", linelen(state->msg), state->msg);
o.branch2 = their_tree_name;
o.detect_directory_renames = MERGE_DIRECTORY_RENAMES_NONE;

if (state->quiet)
o.verbosity = 0;

if (merge_recursive_generic(&o, &our_tree, &their_tree, 1, bases, &result)) {
repo_rerere(the_repository, state->allow_rerere_autoupdate);
free(their_tree_name);
return error(_("Failed to merge in the changes."));
}

free(their_tree_name);
return 0;
}






static void do_commit(const struct am_state *state)
{
struct object_id tree, parent, commit;
const struct object_id *old_oid;
struct commit_list *parents = NULL;
const char *reflog_msg, *author;
struct strbuf sb = STRBUF_INIT;

if (run_hook_le(NULL, "pre-applypatch", NULL))
exit(1);

if (write_cache_as_tree(&tree, 0, NULL))
die(_("git write-tree failed to write a tree"));

if (!get_oid_commit("HEAD", &parent)) {
old_oid = &parent;
commit_list_insert(lookup_commit(the_repository, &parent),
&parents);
} else {
old_oid = NULL;
say(state, stderr, _("applying to an empty history"));
}

author = fmt_ident(state->author_name, state->author_email,
WANT_AUTHOR_IDENT,
state->ignore_date ? NULL : state->author_date,
IDENT_STRICT);

if (state->committer_date_is_author_date)
setenv("GIT_COMMITTER_DATE",
state->ignore_date ? "" : state->author_date, 1);

if (commit_tree(state->msg, state->msg_len, &tree, parents, &commit,
author, state->sign_commit))
die(_("failed to write commit object"));

reflog_msg = getenv("GIT_REFLOG_ACTION");
if (!reflog_msg)
reflog_msg = "am";

strbuf_addf(&sb, "%s: %.*s", reflog_msg, linelen(state->msg),
state->msg);

update_ref(sb.buf, "HEAD", &commit, old_oid, 0,
UPDATE_REFS_DIE_ON_ERR);

if (state->rebasing) {
FILE *fp = xfopen(am_path(state, "rewritten"), "a");

assert(!is_null_oid(&state->orig_commit));
fprintf(fp, "%s ", oid_to_hex(&state->orig_commit));
fprintf(fp, "%s\n", oid_to_hex(&commit));
fclose(fp);
}

run_hook_le(NULL, "post-applypatch", NULL);

strbuf_release(&sb);
}





static void validate_resume_state(const struct am_state *state)
{
if (!state->msg)
die(_("cannot resume: %s does not exist."),
am_path(state, "final-commit"));

if (!state->author_name || !state->author_email || !state->author_date)
die(_("cannot resume: %s does not exist."),
am_path(state, "author-script"));
}








static int do_interactive(struct am_state *state)
{
assert(state->msg);

for (;;) {
char reply[64];

puts(_("Commit Body is:"));
puts("--------------------------");
printf("%s", state->msg);
puts("--------------------------");






printf(_("Apply? [y]es/[n]o/[e]dit/[v]iew patch/[a]ccept all: "));
if (!fgets(reply, sizeof(reply), stdin))
die("unable to read from stdin; aborting");

if (*reply == 'y' || *reply == 'Y') {
return 0;
} else if (*reply == 'a' || *reply == 'A') {
state->interactive = 0;
return 0;
} else if (*reply == 'n' || *reply == 'N') {
return 1;
} else if (*reply == 'e' || *reply == 'E') {
struct strbuf msg = STRBUF_INIT;

if (!launch_editor(am_path(state, "final-commit"), &msg, NULL)) {
free(state->msg);
state->msg = strbuf_detach(&msg, &state->msg_len);
}
strbuf_release(&msg);
} else if (*reply == 'v' || *reply == 'V') {
const char *pager = git_pager(1);
struct child_process cp = CHILD_PROCESS_INIT;

if (!pager)
pager = "cat";
prepare_pager_args(&cp, pager);
argv_array_push(&cp.args, am_path(state, "patch"));
run_command(&cp);
}
}
}








static void am_run(struct am_state *state, int resume)
{
const char *argv_gc_auto[] = {"gc", "--auto", NULL};
struct strbuf sb = STRBUF_INIT;

unlink(am_path(state, "dirtyindex"));

if (refresh_and_write_cache(REFRESH_QUIET, 0, 0) < 0)
die(_("unable to write index file"));

if (repo_index_has_changes(the_repository, NULL, &sb)) {
write_state_bool(state, "dirtyindex", 1);
die(_("Dirty index: cannot apply patches (dirty: %s)"), sb.buf);
}

strbuf_release(&sb);

while (state->cur <= state->last) {
const char *mail = am_path(state, msgnum(state));
int apply_status;

reset_ident_date();

if (!file_exists(mail))
goto next;

if (resume) {
validate_resume_state(state);
} else {
int skip;

if (state->rebasing)
skip = parse_mail_rebase(state, mail);
else
skip = parse_mail(state, mail);

if (skip)
goto next; 

if (state->signoff)
am_append_signoff(state);

write_author_script(state);
write_commit_msg(state);
}

if (state->interactive && do_interactive(state))
goto next;

if (run_applypatch_msg_hook(state))
exit(1);

say(state, stdout, _("Applying: %.*s"), linelen(state->msg), state->msg);

apply_status = run_apply(state, NULL);

if (apply_status && state->threeway) {
struct strbuf sb = STRBUF_INIT;

strbuf_addstr(&sb, am_path(state, "patch-merge-index"));
apply_status = fall_back_threeway(state, sb.buf);
strbuf_release(&sb);





if (!apply_status &&
!repo_index_has_changes(the_repository, NULL, NULL)) {
say(state, stdout, _("No changes -- Patch already applied."));
goto next;
}
}

if (apply_status) {
printf_ln(_("Patch failed at %s %.*s"), msgnum(state),
linelen(state->msg), state->msg);

if (advice_amworkdir)
advise(_("Use 'git am --show-current-patch=diff' to see the failed patch"));

die_user_resolve(state);
}

do_commit(state);

next:
am_next(state);

if (resume)
am_load(state);
resume = 0;
}

if (!is_empty_or_missing_file(am_path(state, "rewritten"))) {
assert(state->rebasing);
copy_notes_for_rebase(state);
run_post_rewrite_hook(state);
}





if (!state->rebasing) {
am_destroy(state);
close_object_store(the_repository->objects);
run_command_v_opt(argv_gc_auto, RUN_GIT_CMD);
}
}






static void am_resolve(struct am_state *state)
{
validate_resume_state(state);

say(state, stdout, _("Applying: %.*s"), linelen(state->msg), state->msg);

if (!repo_index_has_changes(the_repository, NULL, NULL)) {
printf_ln(_("No changes - did you forget to use 'git add'?\n"
"If there is nothing left to stage, chances are that something else\n"
"already introduced the same changes; you might want to skip this patch."));
die_user_resolve(state);
}

if (unmerged_cache()) {
printf_ln(_("You still have unmerged paths in your index.\n"
"You should 'git add' each file with resolved conflicts to mark them as such.\n"
"You might run `git rm` on a file to accept \"deleted by them\" for it."));
die_user_resolve(state);
}

if (state->interactive) {
write_index_patch(state);
if (do_interactive(state))
goto next;
}

repo_rerere(the_repository, 0);

do_commit(state);

next:
am_next(state);
am_load(state);
am_run(state, 0);
}






static int fast_forward_to(struct tree *head, struct tree *remote, int reset)
{
struct lock_file lock_file = LOCK_INIT;
struct unpack_trees_options opts;
struct tree_desc t[2];

if (parse_tree(head) || parse_tree(remote))
return -1;

hold_locked_index(&lock_file, LOCK_DIE_ON_ERROR);

refresh_cache(REFRESH_QUIET);

memset(&opts, 0, sizeof(opts));
opts.head_idx = 1;
opts.src_index = &the_index;
opts.dst_index = &the_index;
opts.update = 1;
opts.merge = 1;
opts.reset = reset;
opts.fn = twoway_merge;
init_tree_desc(&t[0], head->buffer, head->size);
init_tree_desc(&t[1], remote->buffer, remote->size);

if (unpack_trees(2, t, &opts)) {
rollback_lock_file(&lock_file);
return -1;
}

if (write_locked_index(&the_index, &lock_file, COMMIT_LOCK))
die(_("unable to write new index file"));

return 0;
}





static int merge_tree(struct tree *tree)
{
struct lock_file lock_file = LOCK_INIT;
struct unpack_trees_options opts;
struct tree_desc t[1];

if (parse_tree(tree))
return -1;

hold_locked_index(&lock_file, LOCK_DIE_ON_ERROR);

memset(&opts, 0, sizeof(opts));
opts.head_idx = 1;
opts.src_index = &the_index;
opts.dst_index = &the_index;
opts.merge = 1;
opts.fn = oneway_merge;
init_tree_desc(&t[0], tree->buffer, tree->size);

if (unpack_trees(1, t, &opts)) {
rollback_lock_file(&lock_file);
return -1;
}

if (write_locked_index(&the_index, &lock_file, COMMIT_LOCK))
die(_("unable to write new index file"));

return 0;
}





static int clean_index(const struct object_id *head, const struct object_id *remote)
{
struct tree *head_tree, *remote_tree, *index_tree;
struct object_id index;

head_tree = parse_tree_indirect(head);
if (!head_tree)
return error(_("Could not parse object '%s'."), oid_to_hex(head));

remote_tree = parse_tree_indirect(remote);
if (!remote_tree)
return error(_("Could not parse object '%s'."), oid_to_hex(remote));

read_cache_unmerged();

if (fast_forward_to(head_tree, head_tree, 1))
return -1;

if (write_cache_as_tree(&index, 0, NULL))
return -1;

index_tree = parse_tree_indirect(&index);
if (!index_tree)
return error(_("Could not parse object '%s'."), oid_to_hex(&index));

if (fast_forward_to(index_tree, remote_tree, 0))
return -1;

if (merge_tree(remote_tree))
return -1;

remove_branch_state(the_repository, 0);

return 0;
}




static void am_rerere_clear(void)
{
struct string_list merge_rr = STRING_LIST_INIT_DUP;
rerere_clear(the_repository, &merge_rr);
string_list_clear(&merge_rr, 1);
}




static void am_skip(struct am_state *state)
{
struct object_id head;

am_rerere_clear();

if (get_oid("HEAD", &head))
oidcpy(&head, the_hash_algo->empty_tree);

if (clean_index(&head, &head))
die(_("failed to clean index"));

if (state->rebasing) {
FILE *fp = xfopen(am_path(state, "rewritten"), "a");

assert(!is_null_oid(&state->orig_commit));
fprintf(fp, "%s ", oid_to_hex(&state->orig_commit));
fprintf(fp, "%s\n", oid_to_hex(&head));
fclose(fp);
}

am_next(state);
am_load(state);
am_run(state, 0);
}








static int safe_to_abort(const struct am_state *state)
{
struct strbuf sb = STRBUF_INIT;
struct object_id abort_safety, head;

if (file_exists(am_path(state, "dirtyindex")))
return 0;

if (read_state_file(&sb, state, "abort-safety", 1) > 0) {
if (get_oid_hex(sb.buf, &abort_safety))
die(_("could not parse %s"), am_path(state, "abort-safety"));
} else
oidclr(&abort_safety);
strbuf_release(&sb);

if (get_oid("HEAD", &head))
oidclr(&head);

if (oideq(&head, &abort_safety))
return 1;

warning(_("You seem to have moved HEAD since the last 'am' failure.\n"
"Not rewinding to ORIG_HEAD"));

return 0;
}




static void am_abort(struct am_state *state)
{
struct object_id curr_head, orig_head;
int has_curr_head, has_orig_head;
char *curr_branch;

if (!safe_to_abort(state)) {
am_destroy(state);
return;
}

am_rerere_clear();

curr_branch = resolve_refdup("HEAD", 0, &curr_head, NULL);
has_curr_head = curr_branch && !is_null_oid(&curr_head);
if (!has_curr_head)
oidcpy(&curr_head, the_hash_algo->empty_tree);

has_orig_head = !get_oid("ORIG_HEAD", &orig_head);
if (!has_orig_head)
oidcpy(&orig_head, the_hash_algo->empty_tree);

clean_index(&curr_head, &orig_head);

if (has_orig_head)
update_ref("am --abort", "HEAD", &orig_head,
has_curr_head ? &curr_head : NULL, 0,
UPDATE_REFS_DIE_ON_ERR);
else if (curr_branch)
delete_ref(NULL, curr_branch, NULL, REF_NO_DEREF);

free(curr_branch);
am_destroy(state);
}

static int show_patch(struct am_state *state, enum show_patch_type sub_mode)
{
struct strbuf sb = STRBUF_INIT;
const char *patch_path;
int len;

if (!is_null_oid(&state->orig_commit)) {
const char *av[4] = { "show", NULL, "--", NULL };
char *new_oid_str;
int ret;

av[1] = new_oid_str = xstrdup(oid_to_hex(&state->orig_commit));
ret = run_command_v_opt(av, RUN_GIT_CMD);
free(new_oid_str);
return ret;
}

switch (sub_mode) {
case SHOW_PATCH_RAW:
patch_path = am_path(state, msgnum(state));
break;
case SHOW_PATCH_DIFF:
patch_path = am_path(state, "patch");
break;
default:
BUG("invalid mode for --show-current-patch");
}

len = strbuf_read_file(&sb, patch_path, 0);
if (len < 0)
die_errno(_("failed to read '%s'"), patch_path);

setup_pager();
write_in_full(1, sb.buf, sb.len);
strbuf_release(&sb);
return 0;
}





static int parse_opt_patchformat(const struct option *opt, const char *arg, int unset)
{
int *opt_value = opt->value;

if (unset)
*opt_value = PATCH_FORMAT_UNKNOWN;
else if (!strcmp(arg, "mbox"))
*opt_value = PATCH_FORMAT_MBOX;
else if (!strcmp(arg, "stgit"))
*opt_value = PATCH_FORMAT_STGIT;
else if (!strcmp(arg, "stgit-series"))
*opt_value = PATCH_FORMAT_STGIT_SERIES;
else if (!strcmp(arg, "hg"))
*opt_value = PATCH_FORMAT_HG;
else if (!strcmp(arg, "mboxrd"))
*opt_value = PATCH_FORMAT_MBOXRD;




else
return error(_("Invalid value for --patch-format: %s"), arg);
return 0;
}

enum resume_type {
RESUME_FALSE = 0,
RESUME_APPLY,
RESUME_RESOLVED,
RESUME_SKIP,
RESUME_ABORT,
RESUME_QUIT,
RESUME_SHOW_PATCH
};

struct resume_mode {
enum resume_type mode;
enum show_patch_type sub_mode;
};

static int parse_opt_show_current_patch(const struct option *opt, const char *arg, int unset)
{
int *opt_value = opt->value;
struct resume_mode *resume = container_of(opt_value, struct resume_mode, mode);





const char *valid_modes[] = {
[SHOW_PATCH_DIFF] = "diff",
[SHOW_PATCH_RAW] = "raw"
};
int new_value = SHOW_PATCH_RAW;

if (arg) {
for (new_value = 0; new_value < ARRAY_SIZE(valid_modes); new_value++) {
if (!strcmp(arg, valid_modes[new_value]))
break;
}
if (new_value >= ARRAY_SIZE(valid_modes))
return error(_("Invalid value for --show-current-patch: %s"), arg);
}

if (resume->mode == RESUME_SHOW_PATCH && new_value != resume->sub_mode)
return error(_("--show-current-patch=%s is incompatible with "
"--show-current-patch=%s"),
arg, valid_modes[resume->sub_mode]);

resume->mode = RESUME_SHOW_PATCH;
resume->sub_mode = new_value;
return 0;
}

static int git_am_config(const char *k, const char *v, void *cb)
{
int status;

status = git_gpg_config(k, v, NULL);
if (status)
return status;

return git_default_config(k, v, NULL);
}

int cmd_am(int argc, const char **argv, const char *prefix)
{
struct am_state state;
int binary = -1;
int keep_cr = -1;
int patch_format = PATCH_FORMAT_UNKNOWN;
struct resume_mode resume = { .mode = RESUME_FALSE };
int in_progress;
int ret = 0;

const char * const usage[] = {
N_("git am [<options>] [(<mbox> | <Maildir>)...]"),
N_("git am [<options>] (--continue | --skip | --abort)"),
NULL
};

struct option options[] = {
OPT_BOOL('i', "interactive", &state.interactive,
N_("run interactively")),
OPT_HIDDEN_BOOL('b', "binary", &binary,
N_("historical option -- no-op")),
OPT_BOOL('3', "3way", &state.threeway,
N_("allow fall back on 3way merging if needed")),
OPT__QUIET(&state.quiet, N_("be quiet")),
OPT_SET_INT('s', "signoff", &state.signoff,
N_("add a Signed-off-by line to the commit message"),
SIGNOFF_EXPLICIT),
OPT_BOOL('u', "utf8", &state.utf8,
N_("recode into utf8 (default)")),
OPT_SET_INT('k', "keep", &state.keep,
N_("pass -k flag to git-mailinfo"), KEEP_TRUE),
OPT_SET_INT(0, "keep-non-patch", &state.keep,
N_("pass -b flag to git-mailinfo"), KEEP_NON_PATCH),
OPT_BOOL('m', "message-id", &state.message_id,
N_("pass -m flag to git-mailinfo")),
OPT_SET_INT_F(0, "keep-cr", &keep_cr,
N_("pass --keep-cr flag to git-mailsplit for mbox format"),
1, PARSE_OPT_NONEG),
OPT_SET_INT_F(0, "no-keep-cr", &keep_cr,
N_("do not pass --keep-cr flag to git-mailsplit independent of am.keepcr"),
0, PARSE_OPT_NONEG),
OPT_BOOL('c', "scissors", &state.scissors,
N_("strip everything before a scissors line")),
OPT_PASSTHRU_ARGV(0, "whitespace", &state.git_apply_opts, N_("action"),
N_("pass it through git-apply"),
0),
OPT_PASSTHRU_ARGV(0, "ignore-space-change", &state.git_apply_opts, NULL,
N_("pass it through git-apply"),
PARSE_OPT_NOARG),
OPT_PASSTHRU_ARGV(0, "ignore-whitespace", &state.git_apply_opts, NULL,
N_("pass it through git-apply"),
PARSE_OPT_NOARG),
OPT_PASSTHRU_ARGV(0, "directory", &state.git_apply_opts, N_("root"),
N_("pass it through git-apply"),
0),
OPT_PASSTHRU_ARGV(0, "exclude", &state.git_apply_opts, N_("path"),
N_("pass it through git-apply"),
0),
OPT_PASSTHRU_ARGV(0, "include", &state.git_apply_opts, N_("path"),
N_("pass it through git-apply"),
0),
OPT_PASSTHRU_ARGV('C', NULL, &state.git_apply_opts, N_("n"),
N_("pass it through git-apply"),
0),
OPT_PASSTHRU_ARGV('p', NULL, &state.git_apply_opts, N_("num"),
N_("pass it through git-apply"),
0),
OPT_CALLBACK(0, "patch-format", &patch_format, N_("format"),
N_("format the patch(es) are in"),
parse_opt_patchformat),
OPT_PASSTHRU_ARGV(0, "reject", &state.git_apply_opts, NULL,
N_("pass it through git-apply"),
PARSE_OPT_NOARG),
OPT_STRING(0, "resolvemsg", &state.resolvemsg, NULL,
N_("override error message when patch failure occurs")),
OPT_CMDMODE(0, "continue", &resume.mode,
N_("continue applying patches after resolving a conflict"),
RESUME_RESOLVED),
OPT_CMDMODE('r', "resolved", &resume.mode,
N_("synonyms for --continue"),
RESUME_RESOLVED),
OPT_CMDMODE(0, "skip", &resume.mode,
N_("skip the current patch"),
RESUME_SKIP),
OPT_CMDMODE(0, "abort", &resume.mode,
N_("restore the original branch and abort the patching operation."),
RESUME_ABORT),
OPT_CMDMODE(0, "quit", &resume.mode,
N_("abort the patching operation but keep HEAD where it is."),
RESUME_QUIT),
{ OPTION_CALLBACK, 0, "show-current-patch", &resume.mode,
"(diff|raw)",
N_("show the patch being applied"),
PARSE_OPT_CMDMODE | PARSE_OPT_OPTARG | PARSE_OPT_NONEG | PARSE_OPT_LITERAL_ARGHELP,
parse_opt_show_current_patch, RESUME_SHOW_PATCH },
OPT_BOOL(0, "committer-date-is-author-date",
&state.committer_date_is_author_date,
N_("lie about committer date")),
OPT_BOOL(0, "ignore-date", &state.ignore_date,
N_("use current timestamp for author date")),
OPT_RERERE_AUTOUPDATE(&state.allow_rerere_autoupdate),
{ OPTION_STRING, 'S', "gpg-sign", &state.sign_commit, N_("key-id"),
N_("GPG-sign commits"),
PARSE_OPT_OPTARG, NULL, (intptr_t) "" },
OPT_HIDDEN_BOOL(0, "rebasing", &state.rebasing,
N_("(internal use for git-rebase)")),
OPT_END()
};

if (argc == 2 && !strcmp(argv[1], "-h"))
usage_with_options(usage, options);

git_config(git_am_config, NULL);

am_state_init(&state);

in_progress = am_in_progress(&state);
if (in_progress)
am_load(&state);

argc = parse_options(argc, argv, prefix, options, usage, 0);

if (binary >= 0)
fprintf_ln(stderr, _("The -b/--binary option has been a no-op for long time, and\n"
"it will be removed. Please do not use it anymore."));


git_committer_info(IDENT_STRICT);

if (repo_read_index_preload(the_repository, NULL, 0) < 0)
die(_("failed to read the index"));

if (in_progress) {











if (argc || (resume.mode == RESUME_FALSE && !isatty(0)))
die(_("previous rebase directory %s still exists but mbox given."),
state.dir);

if (resume.mode == RESUME_FALSE)
resume.mode = RESUME_APPLY;

if (state.signoff == SIGNOFF_EXPLICIT)
am_append_signoff(&state);
} else {
struct argv_array paths = ARGV_ARRAY_INIT;
int i;






if (file_exists(state.dir) && !state.rebasing) {
if (resume.mode == RESUME_ABORT || resume.mode == RESUME_QUIT) {
am_destroy(&state);
am_state_release(&state);
return 0;
}

die(_("Stray %s directory found.\n"
"Use \"git am --abort\" to remove it."),
state.dir);
}

if (resume.mode)
die(_("Resolve operation not in progress, we are not resuming."));

for (i = 0; i < argc; i++) {
if (is_absolute_path(argv[i]) || !prefix)
argv_array_push(&paths, argv[i]);
else
argv_array_push(&paths, mkpath("%s/%s", prefix, argv[i]));
}

if (state.interactive && !paths.argc)
die(_("interactive mode requires patches on the command line"));

am_setup(&state, patch_format, paths.argv, keep_cr);

argv_array_clear(&paths);
}

switch (resume.mode) {
case RESUME_FALSE:
am_run(&state, 0);
break;
case RESUME_APPLY:
am_run(&state, 1);
break;
case RESUME_RESOLVED:
am_resolve(&state);
break;
case RESUME_SKIP:
am_skip(&state);
break;
case RESUME_ABORT:
am_abort(&state);
break;
case RESUME_QUIT:
am_rerere_clear();
am_destroy(&state);
break;
case RESUME_SHOW_PATCH:
ret = show_patch(&state, resume.sub_mode);
break;
default:
BUG("invalid resume value");
}

am_state_release(&state);

return ret;
}
