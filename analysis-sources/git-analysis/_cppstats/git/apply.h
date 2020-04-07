#include "hash.h"
#include "lockfile.h"
#include "string-list.h"
struct repository;
enum apply_ws_error_action {
nowarn_ws_error,
warn_on_ws_error,
die_on_ws_error,
correct_ws_error
};
enum apply_ws_ignore {
ignore_ws_none,
ignore_ws_change
};
enum apply_verbosity {
verbosity_silent = -1,
verbosity_normal = 0,
verbosity_verbose = 1
};
#define APPLY_SYMLINK_GOES_AWAY 01
#define APPLY_SYMLINK_IN_RESULT 02
struct apply_state {
const char *prefix;
struct lock_file lock_file;
int apply; 
int cached; 
int check; 
int check_index; 
int update_index; 
int ita_only; 
int diffstat; 
int numstat; 
int summary; 
int allow_overlap;
int apply_in_reverse;
int apply_with_reject;
int no_add;
int threeway;
int unidiff_zero;
int unsafe_paths;
struct repository *repo;
const char *index_file;
enum apply_verbosity apply_verbosity;
const char *fake_ancestor;
const char *patch_input_file;
int line_termination;
struct strbuf root;
int p_value;
int p_value_known;
unsigned int p_context;
struct string_list limit_by_name;
int has_include;
int linenr; 
struct string_list symlink_changes; 
int max_change;
int max_len;
struct string_list fn_table;
void (*saved_error_routine)(const char *err, va_list params);
void (*saved_warn_routine)(const char *warn, va_list params);
enum apply_ws_error_action ws_error_action;
enum apply_ws_ignore ws_ignore_action;
const char *whitespace_option;
int whitespace_error;
int squelch_whitespace_errors;
int applied_after_fixing_ws;
};
struct patch {
char *new_name, *old_name, *def_name;
unsigned int old_mode, new_mode;
int is_new, is_delete; 
int rejected;
unsigned ws_rule;
int lines_added, lines_deleted;
int score;
int extension_linenr; 
unsigned int is_toplevel_relative:1;
unsigned int inaccurate_eof:1;
unsigned int is_binary:1;
unsigned int is_copy:1;
unsigned int is_rename:1;
unsigned int recount:1;
unsigned int conflicted_threeway:1;
unsigned int direct_to_threeway:1;
unsigned int crlf_in_old:1;
struct fragment *fragments;
char *result;
size_t resultsize;
char old_oid_prefix[GIT_MAX_HEXSZ + 1];
char new_oid_prefix[GIT_MAX_HEXSZ + 1];
struct patch *next;
struct object_id threeway_stage[3];
};
int apply_parse_options(int argc, const char **argv,
struct apply_state *state,
int *force_apply, int *options,
const char * const *apply_usage);
int init_apply_state(struct apply_state *state,
struct repository *repo,
const char *prefix);
void clear_apply_state(struct apply_state *state);
int check_apply_state(struct apply_state *state, int force_apply);
int parse_git_diff_header(struct strbuf *root,
int *linenr,
int p_value,
const char *line,
int len,
unsigned int size,
struct patch *patch);
#define APPLY_OPT_INACCURATE_EOF (1<<0) 
#define APPLY_OPT_RECOUNT (1<<1) 
int apply_all_patches(struct apply_state *state,
int argc, const char **argv,
int options);
