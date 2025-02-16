






#define USE_THE_INDEX_COMPATIBILITY_MACROS
#include "cache.h"
#include "config.h"
#include "builtin.h"
#include "parse-options.h"
#include "exec-cmd.h"
#include "run-command.h"
#include "sha1-array.h"
#include "remote.h"
#include "dir.h"
#include "rebase.h"
#include "refs.h"
#include "refspec.h"
#include "revision.h"
#include "submodule.h"
#include "submodule-config.h"
#include "tempfile.h"
#include "lockfile.h"
#include "wt-status.h"
#include "commit-reach.h"
#include "sequencer.h"








static enum rebase_type parse_config_rebase(const char *key, const char *value,
int fatal)
{
enum rebase_type v = rebase_parse_value(value);
if (v != REBASE_INVALID)
return v;

if (fatal)
die(_("Invalid value for %s: %s"), key, value);
else
error(_("Invalid value for %s: %s"), key, value);

return REBASE_INVALID;
}




static int parse_opt_rebase(const struct option *opt, const char *arg, int unset)
{
enum rebase_type *value = opt->value;

if (arg)
*value = parse_config_rebase("--rebase", arg, 0);
else
*value = unset ? REBASE_FALSE : REBASE_TRUE;
return *value == REBASE_INVALID ? -1 : 0;
}

static const char * const pull_usage[] = {
N_("git pull [<options>] [<repository> [<refspec>...]]"),
NULL
};


static int opt_verbosity;
static char *opt_progress;
static int recurse_submodules = RECURSE_SUBMODULES_DEFAULT;


static enum rebase_type opt_rebase = -1;
static char *opt_diffstat;
static char *opt_log;
static char *opt_signoff;
static char *opt_squash;
static char *opt_commit;
static char *opt_edit;
static char *cleanup_arg;
static char *opt_ff;
static char *opt_verify_signatures;
static int opt_autostash = -1;
static int config_autostash;
static int check_trust_level = 1;
static struct argv_array opt_strategies = ARGV_ARRAY_INIT;
static struct argv_array opt_strategy_opts = ARGV_ARRAY_INIT;
static char *opt_gpg_sign;
static int opt_allow_unrelated_histories;


static char *opt_all;
static char *opt_append;
static char *opt_upload_pack;
static int opt_force;
static char *opt_tags;
static char *opt_prune;
static char *max_children;
static int opt_dry_run;
static char *opt_keep;
static char *opt_depth;
static char *opt_unshallow;
static char *opt_update_shallow;
static char *opt_refmap;
static char *opt_ipv4;
static char *opt_ipv6;
static int opt_show_forced_updates = -1;
static char *set_upstream;

static struct option pull_options[] = {

OPT__VERBOSITY(&opt_verbosity),
OPT_PASSTHRU(0, "progress", &opt_progress, NULL,
N_("force progress reporting"),
PARSE_OPT_NOARG),
{ OPTION_CALLBACK, 0, "recurse-submodules",
&recurse_submodules, N_("on-demand"),
N_("control for recursive fetching of submodules"),
PARSE_OPT_OPTARG, option_fetch_parse_recurse_submodules },


OPT_GROUP(N_("Options related to merging")),
{ OPTION_CALLBACK, 'r', "rebase", &opt_rebase,
"(false|true|merges|preserve|interactive)",
N_("incorporate changes by rebasing rather than merging"),
PARSE_OPT_OPTARG, parse_opt_rebase },
OPT_PASSTHRU('n', NULL, &opt_diffstat, NULL,
N_("do not show a diffstat at the end of the merge"),
PARSE_OPT_NOARG | PARSE_OPT_NONEG),
OPT_PASSTHRU(0, "stat", &opt_diffstat, NULL,
N_("show a diffstat at the end of the merge"),
PARSE_OPT_NOARG),
OPT_PASSTHRU(0, "summary", &opt_diffstat, NULL,
N_("(synonym to --stat)"),
PARSE_OPT_NOARG | PARSE_OPT_HIDDEN),
OPT_PASSTHRU(0, "log", &opt_log, N_("n"),
N_("add (at most <n>) entries from shortlog to merge commit message"),
PARSE_OPT_OPTARG),
OPT_PASSTHRU(0, "signoff", &opt_signoff, NULL,
N_("add Signed-off-by:"),
PARSE_OPT_OPTARG),
OPT_PASSTHRU(0, "squash", &opt_squash, NULL,
N_("create a single commit instead of doing a merge"),
PARSE_OPT_NOARG),
OPT_PASSTHRU(0, "commit", &opt_commit, NULL,
N_("perform a commit if the merge succeeds (default)"),
PARSE_OPT_NOARG),
OPT_PASSTHRU(0, "edit", &opt_edit, NULL,
N_("edit message before committing"),
PARSE_OPT_NOARG),
OPT_CLEANUP(&cleanup_arg),
OPT_PASSTHRU(0, "ff", &opt_ff, NULL,
N_("allow fast-forward"),
PARSE_OPT_NOARG),
OPT_PASSTHRU(0, "ff-only", &opt_ff, NULL,
N_("abort if fast-forward is not possible"),
PARSE_OPT_NOARG | PARSE_OPT_NONEG),
OPT_PASSTHRU(0, "verify-signatures", &opt_verify_signatures, NULL,
N_("verify that the named commit has a valid GPG signature"),
PARSE_OPT_NOARG),
OPT_BOOL(0, "autostash", &opt_autostash,
N_("automatically stash/stash pop before and after rebase")),
OPT_PASSTHRU_ARGV('s', "strategy", &opt_strategies, N_("strategy"),
N_("merge strategy to use"),
0),
OPT_PASSTHRU_ARGV('X', "strategy-option", &opt_strategy_opts,
N_("option=value"),
N_("option for selected merge strategy"),
0),
OPT_PASSTHRU('S', "gpg-sign", &opt_gpg_sign, N_("key-id"),
N_("GPG sign commit"),
PARSE_OPT_OPTARG),
OPT_SET_INT(0, "allow-unrelated-histories",
&opt_allow_unrelated_histories,
N_("allow merging unrelated histories"), 1),


OPT_GROUP(N_("Options related to fetching")),
OPT_PASSTHRU(0, "all", &opt_all, NULL,
N_("fetch from all remotes"),
PARSE_OPT_NOARG),
OPT_PASSTHRU('a', "append", &opt_append, NULL,
N_("append to .git/FETCH_HEAD instead of overwriting"),
PARSE_OPT_NOARG),
OPT_PASSTHRU(0, "upload-pack", &opt_upload_pack, N_("path"),
N_("path to upload pack on remote end"),
0),
OPT__FORCE(&opt_force, N_("force overwrite of local branch"), 0),
OPT_PASSTHRU('t', "tags", &opt_tags, NULL,
N_("fetch all tags and associated objects"),
PARSE_OPT_NOARG),
OPT_PASSTHRU('p', "prune", &opt_prune, NULL,
N_("prune remote-tracking branches no longer on remote"),
PARSE_OPT_NOARG),
OPT_PASSTHRU('j', "jobs", &max_children, N_("n"),
N_("number of submodules pulled in parallel"),
PARSE_OPT_OPTARG),
OPT_BOOL(0, "dry-run", &opt_dry_run,
N_("dry run")),
OPT_PASSTHRU('k', "keep", &opt_keep, NULL,
N_("keep downloaded pack"),
PARSE_OPT_NOARG),
OPT_PASSTHRU(0, "depth", &opt_depth, N_("depth"),
N_("deepen history of shallow clone"),
0),
OPT_PASSTHRU(0, "unshallow", &opt_unshallow, NULL,
N_("convert to a complete repository"),
PARSE_OPT_NONEG | PARSE_OPT_NOARG),
OPT_PASSTHRU(0, "update-shallow", &opt_update_shallow, NULL,
N_("accept refs that update .git/shallow"),
PARSE_OPT_NOARG),
OPT_PASSTHRU(0, "refmap", &opt_refmap, N_("refmap"),
N_("specify fetch refmap"),
PARSE_OPT_NONEG),
OPT_PASSTHRU('4', "ipv4", &opt_ipv4, NULL,
N_("use IPv4 addresses only"),
PARSE_OPT_NOARG),
OPT_PASSTHRU('6', "ipv6", &opt_ipv6, NULL,
N_("use IPv6 addresses only"),
PARSE_OPT_NOARG),
OPT_BOOL(0, "show-forced-updates", &opt_show_forced_updates,
N_("check for forced-updates on all updated branches")),
OPT_PASSTHRU(0, "set-upstream", &set_upstream, NULL,
N_("set upstream for git pull/fetch"),
PARSE_OPT_NOARG),

OPT_END()
};




static void argv_push_verbosity(struct argv_array *arr)
{
int verbosity;

for (verbosity = opt_verbosity; verbosity > 0; verbosity--)
argv_array_push(arr, "-v");

for (verbosity = opt_verbosity; verbosity < 0; verbosity++)
argv_array_push(arr, "-q");
}




static void argv_push_force(struct argv_array *arr)
{
int force = opt_force;
while (force-- > 0)
argv_array_push(arr, "-f");
}




static void set_reflog_message(int argc, const char **argv)
{
int i;
struct strbuf msg = STRBUF_INIT;

for (i = 0; i < argc; i++) {
if (i)
strbuf_addch(&msg, ' ');
strbuf_addstr(&msg, argv[i]);
}

setenv("GIT_REFLOG_ACTION", msg.buf, 0);

strbuf_release(&msg);
}







static const char *config_get_ff(void)
{
const char *value;

if (git_config_get_value("pull.ff", &value))
return NULL;

switch (git_parse_maybe_bool(value)) {
case 0:
return "--no-ff";
case 1:
return "--ff";
}

if (!strcmp(value, "only"))
return "--ff-only";

die(_("Invalid value for pull.ff: %s"), value);
}








static enum rebase_type config_get_rebase(void)
{
struct branch *curr_branch = branch_get("HEAD");
const char *value;

if (curr_branch) {
char *key = xstrfmt("branch.%s.rebase", curr_branch->name);

if (!git_config_get_value(key, &value)) {
enum rebase_type ret = parse_config_rebase(key, value, 1);
free(key);
return ret;
}

free(key);
}

if (!git_config_get_value("pull.rebase", &value))
return parse_config_rebase("pull.rebase", value, 1);

if (opt_verbosity >= 0 &&
(!opt_ff || strcmp(opt_ff, "--ff-only"))) {
warning(_("Pulling without specifying how to reconcile divergent branches is\n"
"discouraged. You can squelch this message by running one of the following\n"
"commands sometime before your next pull:\n"
"\n"
" git config pull.rebase false #merge (the default strategy)\n"
" git config pull.rebase true #rebase\n"
" git config pull.ff only #fast-forward only\n"
"\n"
"You can replace \"git config\" with \"git config --global\" to set a default\n"
"preference for all repositories. You can also pass --rebase, --no-rebase,\n"
"or --ff-only on the command line to override the configured default per\n"
"invocation.\n"));
}

return REBASE_FALSE;
}




static int git_pull_config(const char *var, const char *value, void *cb)
{
int status;

if (!strcmp(var, "rebase.autostash")) {
config_autostash = git_config_bool(var, value);
return 0;
} else if (!strcmp(var, "submodule.recurse")) {
recurse_submodules = git_config_bool(var, value) ?
RECURSE_SUBMODULES_ON : RECURSE_SUBMODULES_OFF;
return 0;
} else if (!strcmp(var, "gpg.mintrustlevel")) {
check_trust_level = 0;
}

status = git_gpg_config(var, value, cb);
if (status)
return status;

return git_default_config(var, value, cb);
}





static void get_merge_heads(struct oid_array *merge_heads)
{
const char *filename = git_path_fetch_head(the_repository);
FILE *fp;
struct strbuf sb = STRBUF_INIT;
struct object_id oid;

fp = xfopen(filename, "r");
while (strbuf_getline_lf(&sb, fp) != EOF) {
const char *p;
if (parse_oid_hex(sb.buf, &oid, &p))
continue; 
if (starts_with(p, "\tnot-for-merge\t"))
continue; 
oid_array_append(merge_heads, &oid);
}
fclose(fp);
strbuf_release(&sb);
}





static int get_only_remote(struct remote *remote, void *cb_data)
{
const char **remote_name = cb_data;

if (*remote_name)
return -1;

*remote_name = remote->name;
return 0;
}























static void NORETURN die_no_merge_candidates(const char *repo, const char **refspecs)
{
struct branch *curr_branch = branch_get("HEAD");
const char *remote = curr_branch ? curr_branch->remote_name : NULL;

if (*refspecs) {
if (opt_rebase)
fprintf_ln(stderr, _("There is no candidate for rebasing against among the refs that you just fetched."));
else
fprintf_ln(stderr, _("There are no candidates for merging among the refs that you just fetched."));
fprintf_ln(stderr, _("Generally this means that you provided a wildcard refspec which had no\n"
"matches on the remote end."));
} else if (repo && curr_branch && (!remote || strcmp(repo, remote))) {
fprintf_ln(stderr, _("You asked to pull from the remote '%s', but did not specify\n"
"a branch. Because this is not the default configured remote\n"
"for your current branch, you must specify a branch on the command line."),
repo);
} else if (!curr_branch) {
fprintf_ln(stderr, _("You are not currently on a branch."));
if (opt_rebase)
fprintf_ln(stderr, _("Please specify which branch you want to rebase against."));
else
fprintf_ln(stderr, _("Please specify which branch you want to merge with."));
fprintf_ln(stderr, _("See git-pull(1) for details."));
fprintf(stderr, "\n");
fprintf_ln(stderr, " git pull %s %s", _("<remote>"), _("<branch>"));
fprintf(stderr, "\n");
} else if (!curr_branch->merge_nr) {
const char *remote_name = NULL;

if (for_each_remote(get_only_remote, &remote_name) || !remote_name)
remote_name = _("<remote>");

fprintf_ln(stderr, _("There is no tracking information for the current branch."));
if (opt_rebase)
fprintf_ln(stderr, _("Please specify which branch you want to rebase against."));
else
fprintf_ln(stderr, _("Please specify which branch you want to merge with."));
fprintf_ln(stderr, _("See git-pull(1) for details."));
fprintf(stderr, "\n");
fprintf_ln(stderr, " git pull %s %s", _("<remote>"), _("<branch>"));
fprintf(stderr, "\n");
fprintf_ln(stderr, _("If you wish to set tracking information for this branch you can do so with:"));
fprintf(stderr, "\n");
fprintf_ln(stderr, " git branch --set-upstream-to=%s/%s %s\n",
remote_name, _("<branch>"), curr_branch->name);
} else
fprintf_ln(stderr, _("Your configuration specifies to merge with the ref '%s'\n"
"from the remote, but no such ref was fetched."),
*curr_branch->merge_name);
exit(1);
}






static void parse_repo_refspecs(int argc, const char **argv, const char **repo,
const char ***refspecs)
{
if (argc > 0) {
*repo = *argv++;
argc--;
} else
*repo = NULL;
*refspecs = argv;
}





static int run_fetch(const char *repo, const char **refspecs)
{
struct argv_array args = ARGV_ARRAY_INIT;
int ret;

argv_array_pushl(&args, "fetch", "--update-head-ok", NULL);


argv_push_verbosity(&args);
if (opt_progress)
argv_array_push(&args, opt_progress);


if (opt_all)
argv_array_push(&args, opt_all);
if (opt_append)
argv_array_push(&args, opt_append);
if (opt_upload_pack)
argv_array_push(&args, opt_upload_pack);
argv_push_force(&args);
if (opt_tags)
argv_array_push(&args, opt_tags);
if (opt_prune)
argv_array_push(&args, opt_prune);
if (recurse_submodules != RECURSE_SUBMODULES_DEFAULT)
switch (recurse_submodules) {
case RECURSE_SUBMODULES_ON:
argv_array_push(&args, "--recurse-submodules=on");
break;
case RECURSE_SUBMODULES_OFF:
argv_array_push(&args, "--recurse-submodules=no");
break;
case RECURSE_SUBMODULES_ON_DEMAND:
argv_array_push(&args, "--recurse-submodules=on-demand");
break;
default:
BUG("submodule recursion option not understood");
}
if (max_children)
argv_array_push(&args, max_children);
if (opt_dry_run)
argv_array_push(&args, "--dry-run");
if (opt_keep)
argv_array_push(&args, opt_keep);
if (opt_depth)
argv_array_push(&args, opt_depth);
if (opt_unshallow)
argv_array_push(&args, opt_unshallow);
if (opt_update_shallow)
argv_array_push(&args, opt_update_shallow);
if (opt_refmap)
argv_array_push(&args, opt_refmap);
if (opt_ipv4)
argv_array_push(&args, opt_ipv4);
if (opt_ipv6)
argv_array_push(&args, opt_ipv6);
if (opt_show_forced_updates > 0)
argv_array_push(&args, "--show-forced-updates");
else if (opt_show_forced_updates == 0)
argv_array_push(&args, "--no-show-forced-updates");
if (set_upstream)
argv_array_push(&args, set_upstream);

if (repo) {
argv_array_push(&args, repo);
argv_array_pushv(&args, refspecs);
} else if (*refspecs)
BUG("refspecs without repo?");
ret = run_command_v_opt(args.argv, RUN_GIT_CMD);
argv_array_clear(&args);
return ret;
}




static int pull_into_void(const struct object_id *merge_head,
const struct object_id *curr_head)
{
if (opt_verify_signatures) {
struct commit *commit;

commit = lookup_commit(the_repository, merge_head);
if (!commit)
die(_("unable to access commit %s"),
oid_to_hex(merge_head));

verify_merge_signature(commit, opt_verbosity,
check_trust_level);
}







if (checkout_fast_forward(the_repository,
the_hash_algo->empty_tree,
merge_head, 0))
return 1;

if (update_ref("initial pull", "HEAD", merge_head, curr_head, 0, UPDATE_REFS_DIE_ON_ERR))
return 1;

return 0;
}

static int rebase_submodules(void)
{
struct child_process cp = CHILD_PROCESS_INIT;

cp.git_cmd = 1;
cp.no_stdin = 1;
argv_array_pushl(&cp.args, "submodule", "update",
"--recursive", "--rebase", NULL);
argv_push_verbosity(&cp.args);

return run_command(&cp);
}

static int update_submodules(void)
{
struct child_process cp = CHILD_PROCESS_INIT;

cp.git_cmd = 1;
cp.no_stdin = 1;
argv_array_pushl(&cp.args, "submodule", "update",
"--recursive", "--checkout", NULL);
argv_push_verbosity(&cp.args);

return run_command(&cp);
}




static int run_merge(void)
{
int ret;
struct argv_array args = ARGV_ARRAY_INIT;

argv_array_pushl(&args, "merge", NULL);


argv_push_verbosity(&args);
if (opt_progress)
argv_array_push(&args, opt_progress);


if (opt_diffstat)
argv_array_push(&args, opt_diffstat);
if (opt_log)
argv_array_push(&args, opt_log);
if (opt_signoff)
argv_array_push(&args, opt_signoff);
if (opt_squash)
argv_array_push(&args, opt_squash);
if (opt_commit)
argv_array_push(&args, opt_commit);
if (opt_edit)
argv_array_push(&args, opt_edit);
if (cleanup_arg)
argv_array_pushf(&args, "--cleanup=%s", cleanup_arg);
if (opt_ff)
argv_array_push(&args, opt_ff);
if (opt_verify_signatures)
argv_array_push(&args, opt_verify_signatures);
argv_array_pushv(&args, opt_strategies.argv);
argv_array_pushv(&args, opt_strategy_opts.argv);
if (opt_gpg_sign)
argv_array_push(&args, opt_gpg_sign);
if (opt_allow_unrelated_histories > 0)
argv_array_push(&args, "--allow-unrelated-histories");

argv_array_push(&args, "FETCH_HEAD");
ret = run_command_v_opt(args.argv, RUN_GIT_CMD);
argv_array_clear(&args);
return ret;
}








static const char *get_upstream_branch(const char *remote)
{
struct remote *rm;
struct branch *curr_branch;
const char *curr_branch_remote;

rm = remote_get(remote);
if (!rm)
return NULL;

curr_branch = branch_get("HEAD");
if (!curr_branch)
return NULL;

curr_branch_remote = remote_for_branch(curr_branch, NULL);
assert(curr_branch_remote);

if (strcmp(curr_branch_remote, rm->name))
return NULL;

return branch_get_upstream(curr_branch, NULL);
}







static const char *get_tracking_branch(const char *remote, const char *refspec)
{
struct refspec_item spec;
const char *spec_src;
const char *merge_branch;

refspec_item_init_or_die(&spec, refspec, REFSPEC_FETCH);
spec_src = spec.src;
if (!*spec_src || !strcmp(spec_src, "HEAD"))
spec_src = "HEAD";
else if (skip_prefix(spec_src, "heads/", &spec_src))
;
else if (skip_prefix(spec_src, "refs/heads/", &spec_src))
;
else if (starts_with(spec_src, "refs/") ||
starts_with(spec_src, "tags/") ||
starts_with(spec_src, "remotes/"))
spec_src = "";

if (*spec_src) {
if (!strcmp(remote, "."))
merge_branch = mkpath("refs/heads/%s", spec_src);
else
merge_branch = mkpath("refs/remotes/%s/%s", remote, spec_src);
} else
merge_branch = NULL;

refspec_item_clear(&spec);
return merge_branch;
}






static int get_rebase_fork_point(struct object_id *fork_point, const char *repo,
const char *refspec)
{
int ret;
struct branch *curr_branch;
const char *remote_branch;
struct child_process cp = CHILD_PROCESS_INIT;
struct strbuf sb = STRBUF_INIT;

curr_branch = branch_get("HEAD");
if (!curr_branch)
return -1;

if (refspec)
remote_branch = get_tracking_branch(repo, refspec);
else
remote_branch = get_upstream_branch(repo);

if (!remote_branch)
return -1;

argv_array_pushl(&cp.args, "merge-base", "--fork-point",
remote_branch, curr_branch->name, NULL);
cp.no_stdin = 1;
cp.no_stderr = 1;
cp.git_cmd = 1;

ret = capture_command(&cp, &sb, GIT_MAX_HEXSZ);
if (ret)
goto cleanup;

ret = get_oid_hex(sb.buf, fork_point);
if (ret)
goto cleanup;

cleanup:
strbuf_release(&sb);
return ret ? -1 : 0;
}





static int get_octopus_merge_base(struct object_id *merge_base,
const struct object_id *curr_head,
const struct object_id *merge_head,
const struct object_id *fork_point)
{
struct commit_list *revs = NULL, *result;

commit_list_insert(lookup_commit_reference(the_repository, curr_head),
&revs);
commit_list_insert(lookup_commit_reference(the_repository, merge_head),
&revs);
if (!is_null_oid(fork_point))
commit_list_insert(lookup_commit_reference(the_repository, fork_point),
&revs);

result = get_octopus_merge_bases(revs);
free_commit_list(revs);
reduce_heads_replace(&result);

if (!result)
return 1;

oidcpy(merge_base, &result->item->object.oid);
free_commit_list(result);
return 0;
}






static int run_rebase(const struct object_id *curr_head,
const struct object_id *merge_head,
const struct object_id *fork_point)
{
int ret;
struct object_id oct_merge_base;
struct argv_array args = ARGV_ARRAY_INIT;

if (!get_octopus_merge_base(&oct_merge_base, curr_head, merge_head, fork_point))
if (!is_null_oid(fork_point) && oideq(&oct_merge_base, fork_point))
fork_point = NULL;

argv_array_push(&args, "rebase");


argv_push_verbosity(&args);


if (opt_rebase == REBASE_MERGES)
argv_array_push(&args, "--rebase-merges");
else if (opt_rebase == REBASE_PRESERVE)
argv_array_push(&args, "--preserve-merges");
else if (opt_rebase == REBASE_INTERACTIVE)
argv_array_push(&args, "--interactive");
if (opt_diffstat)
argv_array_push(&args, opt_diffstat);
argv_array_pushv(&args, opt_strategies.argv);
argv_array_pushv(&args, opt_strategy_opts.argv);
if (opt_gpg_sign)
argv_array_push(&args, opt_gpg_sign);
if (opt_autostash == 0)
argv_array_push(&args, "--no-autostash");
else if (opt_autostash == 1)
argv_array_push(&args, "--autostash");
if (opt_verify_signatures &&
!strcmp(opt_verify_signatures, "--verify-signatures"))
warning(_("ignoring --verify-signatures for rebase"));

argv_array_push(&args, "--onto");
argv_array_push(&args, oid_to_hex(merge_head));

if (fork_point && !is_null_oid(fork_point))
argv_array_push(&args, oid_to_hex(fork_point));
else
argv_array_push(&args, oid_to_hex(merge_head));

ret = run_command_v_opt(args.argv, RUN_GIT_CMD);
argv_array_clear(&args);
return ret;
}

int cmd_pull(int argc, const char **argv, const char *prefix)
{
const char *repo, **refspecs;
struct oid_array merge_heads = OID_ARRAY_INIT;
struct object_id orig_head, curr_head;
struct object_id rebase_fork_point;
int autostash;

if (!getenv("GIT_REFLOG_ACTION"))
set_reflog_message(argc, argv);

git_config(git_pull_config, NULL);

argc = parse_options(argc, argv, prefix, pull_options, pull_usage, 0);

if (cleanup_arg)




get_cleanup_mode(cleanup_arg, 0);

parse_repo_refspecs(argc, argv, &repo, &refspecs);

if (!opt_ff)
opt_ff = xstrdup_or_null(config_get_ff());

if (opt_rebase < 0)
opt_rebase = config_get_rebase();

if (read_cache_unmerged())
die_resolve_conflict("pull");

if (file_exists(git_path_merge_head(the_repository)))
die_conclude_merge();

if (get_oid("HEAD", &orig_head))
oidclr(&orig_head);

if (!opt_rebase && opt_autostash != -1)
die(_("--[no-]autostash option is only valid with --rebase."));

autostash = config_autostash;
if (opt_rebase) {
if (opt_autostash != -1)
autostash = opt_autostash;

if (is_null_oid(&orig_head) && !is_cache_unborn())
die(_("Updating an unborn branch with changes added to the index."));

if (!autostash)
require_clean_work_tree(the_repository,
N_("pull with rebase"),
_("please commit or stash them."), 1, 0);

if (get_rebase_fork_point(&rebase_fork_point, repo, *refspecs))
oidclr(&rebase_fork_point);
}

if (run_fetch(repo, refspecs))
return 1;

if (opt_dry_run)
return 0;

if (get_oid("HEAD", &curr_head))
oidclr(&curr_head);

if (!is_null_oid(&orig_head) && !is_null_oid(&curr_head) &&
!oideq(&orig_head, &curr_head)) {








warning(_("fetch updated the current branch head.\n"
"fast-forwarding your working tree from\n"
"commit %s."), oid_to_hex(&orig_head));

if (checkout_fast_forward(the_repository, &orig_head,
&curr_head, 0))
die(_("Cannot fast-forward your working tree.\n"
"After making sure that you saved anything precious from\n"
"$ git diff %s\n"
"output, run\n"
"$ git reset --hard\n"
"to recover."), oid_to_hex(&orig_head));
}

get_merge_heads(&merge_heads);

if (!merge_heads.nr)
die_no_merge_candidates(repo, refspecs);

if (is_null_oid(&orig_head)) {
if (merge_heads.nr > 1)
die(_("Cannot merge multiple branches into empty head."));
return pull_into_void(merge_heads.oid, &curr_head);
}
if (opt_rebase && merge_heads.nr > 1)
die(_("Cannot rebase onto multiple branches."));

if (opt_rebase) {
int ret = 0;
if ((recurse_submodules == RECURSE_SUBMODULES_ON ||
recurse_submodules == RECURSE_SUBMODULES_ON_DEMAND) &&
submodule_touches_in_range(the_repository, &rebase_fork_point, &curr_head))
die(_("cannot rebase with locally recorded submodule modifications"));
if (!autostash) {
struct commit_list *list = NULL;
struct commit *merge_head, *head;

head = lookup_commit_reference(the_repository,
&orig_head);
commit_list_insert(head, &list);
merge_head = lookup_commit_reference(the_repository,
&merge_heads.oid[0]);
if (is_descendant_of(merge_head, list)) {

opt_ff = "--ff-only";
ret = run_merge();
}
}
ret = run_rebase(&curr_head, merge_heads.oid, &rebase_fork_point);

if (!ret && (recurse_submodules == RECURSE_SUBMODULES_ON ||
recurse_submodules == RECURSE_SUBMODULES_ON_DEMAND))
ret = rebase_submodules();

return ret;
} else {
int ret = run_merge();
if (!ret && (recurse_submodules == RECURSE_SUBMODULES_ON ||
recurse_submodules == RECURSE_SUBMODULES_ON_DEMAND))
ret = update_submodules();
return ret;
}
}
