enum parse_opt_type {
OPTION_END,
OPTION_ARGUMENT,
OPTION_GROUP,
OPTION_NUMBER,
OPTION_ALIAS,
OPTION_BIT,
OPTION_NEGBIT,
OPTION_BITOP,
OPTION_COUNTUP,
OPTION_SET_INT,
OPTION_STRING,
OPTION_INTEGER,
OPTION_MAGNITUDE,
OPTION_CALLBACK,
OPTION_LOWLEVEL_CALLBACK,
OPTION_FILENAME
};
enum parse_opt_flags {
PARSE_OPT_KEEP_DASHDASH = 1,
PARSE_OPT_STOP_AT_NON_OPTION = 2,
PARSE_OPT_KEEP_ARGV0 = 4,
PARSE_OPT_KEEP_UNKNOWN = 8,
PARSE_OPT_NO_INTERNAL_HELP = 16,
PARSE_OPT_ONE_SHOT = 32
};
enum parse_opt_option_flags {
PARSE_OPT_OPTARG = 1,
PARSE_OPT_NOARG = 2,
PARSE_OPT_NONEG = 4,
PARSE_OPT_HIDDEN = 8,
PARSE_OPT_LASTARG_DEFAULT = 16,
PARSE_OPT_NODASH = 32,
PARSE_OPT_LITERAL_ARGHELP = 64,
PARSE_OPT_SHELL_EVAL = 256,
PARSE_OPT_NOCOMPLETE = 512,
PARSE_OPT_COMP_ARG = 1024,
PARSE_OPT_CMDMODE = 2048
};
enum parse_opt_result {
PARSE_OPT_COMPLETE = -3,
PARSE_OPT_HELP = -2,
PARSE_OPT_ERROR = -1, 
PARSE_OPT_DONE = 0, 
PARSE_OPT_NON_OPTION,
PARSE_OPT_UNKNOWN
};
struct option;
typedef int parse_opt_cb(const struct option *, const char *arg, int unset);
struct parse_opt_ctx_t;
typedef enum parse_opt_result parse_opt_ll_cb(struct parse_opt_ctx_t *ctx,
const struct option *opt,
const char *arg, int unset);
struct option {
enum parse_opt_type type;
int short_name;
const char *long_name;
void *value;
const char *argh;
const char *help;
int flags;
parse_opt_cb *callback;
intptr_t defval;
parse_opt_ll_cb *ll_callback;
intptr_t extra;
};
#define OPT_BIT_F(s, l, v, h, b, f) { OPTION_BIT, (s), (l), (v), NULL, (h), PARSE_OPT_NOARG|(f), NULL, (b) }
#define OPT_COUNTUP_F(s, l, v, h, f) { OPTION_COUNTUP, (s), (l), (v), NULL, (h), PARSE_OPT_NOARG|(f) }
#define OPT_SET_INT_F(s, l, v, h, i, f) { OPTION_SET_INT, (s), (l), (v), NULL, (h), PARSE_OPT_NOARG | (f), NULL, (i) }
#define OPT_BOOL_F(s, l, v, h, f) OPT_SET_INT_F(s, l, v, h, 1, f)
#define OPT_CALLBACK_F(s, l, v, a, h, f, cb) { OPTION_CALLBACK, (s), (l), (v), (a), (h), (f), (cb) }
#define OPT_STRING_F(s, l, v, a, h, f) { OPTION_STRING, (s), (l), (v), (a), (h), (f) }
#define OPT_INTEGER_F(s, l, v, h, f) { OPTION_INTEGER, (s), (l), (v), N_("n"), (h), (f) }
#define OPT_END() { OPTION_END }
#define OPT_ARGUMENT(l, v, h) { OPTION_ARGUMENT, 0, (l), (v), NULL, (h), PARSE_OPT_NOARG, NULL, 1 }
#define OPT_GROUP(h) { OPTION_GROUP, 0, NULL, NULL, NULL, (h) }
#define OPT_BIT(s, l, v, h, b) OPT_BIT_F(s, l, v, h, b, 0)
#define OPT_BITOP(s, l, v, h, set, clear) { OPTION_BITOP, (s), (l), (v), NULL, (h), PARSE_OPT_NOARG|PARSE_OPT_NONEG, NULL, (set), NULL, (clear) }
#define OPT_NEGBIT(s, l, v, h, b) { OPTION_NEGBIT, (s), (l), (v), NULL, (h), PARSE_OPT_NOARG, NULL, (b) }
#define OPT_COUNTUP(s, l, v, h) OPT_COUNTUP_F(s, l, v, h, 0)
#define OPT_SET_INT(s, l, v, h, i) OPT_SET_INT_F(s, l, v, h, i, 0)
#define OPT_BOOL(s, l, v, h) OPT_BOOL_F(s, l, v, h, 0)
#define OPT_HIDDEN_BOOL(s, l, v, h) { OPTION_SET_INT, (s), (l), (v), NULL, (h), PARSE_OPT_NOARG | PARSE_OPT_HIDDEN, NULL, 1}
#define OPT_CMDMODE(s, l, v, h, i) { OPTION_SET_INT, (s), (l), (v), NULL, (h), PARSE_OPT_CMDMODE|PARSE_OPT_NOARG|PARSE_OPT_NONEG, NULL, (i) }
#define OPT_INTEGER(s, l, v, h) OPT_INTEGER_F(s, l, v, h, 0)
#define OPT_MAGNITUDE(s, l, v, h) { OPTION_MAGNITUDE, (s), (l), (v), N_("n"), (h), PARSE_OPT_NONEG }
#define OPT_STRING(s, l, v, a, h) OPT_STRING_F(s, l, v, a, h, 0)
#define OPT_STRING_LIST(s, l, v, a, h) { OPTION_CALLBACK, (s), (l), (v), (a), (h), 0, &parse_opt_string_list }
#define OPT_UYN(s, l, v, h) { OPTION_CALLBACK, (s), (l), (v), NULL, (h), PARSE_OPT_NOARG, &parse_opt_tertiary }
#define OPT_EXPIRY_DATE(s, l, v, h) { OPTION_CALLBACK, (s), (l), (v), N_("expiry-date"),(h), 0, parse_opt_expiry_date_cb }
#define OPT_CALLBACK(s, l, v, a, h, f) OPT_CALLBACK_F(s, l, v, a, h, 0, f)
#define OPT_NUMBER_CALLBACK(v, h, f) { OPTION_NUMBER, 0, NULL, (v), NULL, (h), PARSE_OPT_NOARG | PARSE_OPT_NONEG, (f) }
#define OPT_FILENAME(s, l, v, h) { OPTION_FILENAME, (s), (l), (v), N_("file"), (h) }
#define OPT_COLOR_FLAG(s, l, v, h) { OPTION_CALLBACK, (s), (l), (v), N_("when"), (h), PARSE_OPT_OPTARG, parse_opt_color_flag_cb, (intptr_t)"always" }
#define OPT_NOOP_NOARG(s, l) { OPTION_CALLBACK, (s), (l), NULL, NULL, N_("no-op (backward compatibility)"), PARSE_OPT_HIDDEN | PARSE_OPT_NOARG, parse_opt_noop_cb }
#define OPT_ALIAS(s, l, source_long_name) { OPTION_ALIAS, (s), (l), (source_long_name) }
int parse_options(int argc, const char **argv, const char *prefix,
const struct option *options,
const char * const usagestr[], int flags);
NORETURN void usage_with_options(const char * const *usagestr,
const struct option *options);
NORETURN void usage_msg_opt(const char *msg,
const char * const *usagestr,
const struct option *options);
int optbug(const struct option *opt, const char *reason);
const char *optname(const struct option *opt, int flags);
#define BUG_ON_OPT_NEG(unset) do { if ((unset)) BUG("option callback does not expect negation"); } while (0)
#define BUG_ON_OPT_ARG(arg) do { if ((arg)) BUG("option callback does not expect an argument"); } while (0)
#define BUG_ON_OPT_NEG_NOARG(unset, arg) do { BUG_ON_OPT_NEG(unset); if(!(arg)) BUG("option callback expects an argument"); } while(0)
struct parse_opt_ctx_t {
const char **argv;
const char **out;
int argc, cpidx, total;
const char *opt;
int flags;
const char *prefix;
const char **alias_groups; 
struct option *updated_options;
};
void parse_options_start(struct parse_opt_ctx_t *ctx,
int argc, const char **argv, const char *prefix,
const struct option *options, int flags);
int parse_options_step(struct parse_opt_ctx_t *ctx,
const struct option *options,
const char * const usagestr[]);
int parse_options_end(struct parse_opt_ctx_t *ctx);
struct option *parse_options_dup(const struct option *a);
struct option *parse_options_concat(const struct option *a, const struct option *b);
int parse_opt_abbrev_cb(const struct option *, const char *, int);
int parse_opt_expiry_date_cb(const struct option *, const char *, int);
int parse_opt_color_flag_cb(const struct option *, const char *, int);
int parse_opt_verbosity_cb(const struct option *, const char *, int);
int parse_opt_object_name(const struct option *, const char *, int);
int parse_opt_object_id(const struct option *, const char *, int);
int parse_opt_commits(const struct option *, const char *, int);
int parse_opt_commit(const struct option *, const char *, int);
int parse_opt_tertiary(const struct option *, const char *, int);
int parse_opt_string_list(const struct option *, const char *, int);
int parse_opt_noop_cb(const struct option *, const char *, int);
enum parse_opt_result parse_opt_unknown_cb(struct parse_opt_ctx_t *ctx,
const struct option *,
const char *, int);
int parse_opt_passthru(const struct option *, const char *, int);
int parse_opt_passthru_argv(const struct option *, const char *, int);
#define OPT__VERBOSE(var, h) OPT_COUNTUP('v', "verbose", (var), (h))
#define OPT__QUIET(var, h) OPT_COUNTUP('q', "quiet", (var), (h))
#define OPT__VERBOSITY(var) { OPTION_CALLBACK, 'v', "verbose", (var), NULL, N_("be more verbose"), PARSE_OPT_NOARG, &parse_opt_verbosity_cb, 0 }, { OPTION_CALLBACK, 'q', "quiet", (var), NULL, N_("be more quiet"), PARSE_OPT_NOARG, &parse_opt_verbosity_cb, 0 }
#define OPT__DRY_RUN(var, h) OPT_BOOL('n', "dry-run", (var), (h))
#define OPT__FORCE(var, h, f) OPT_COUNTUP_F('f', "force", (var), (h), (f))
#define OPT__ABBREV(var) { OPTION_CALLBACK, 0, "abbrev", (var), N_("n"), N_("use <n> digits to display SHA-1s"), PARSE_OPT_OPTARG, &parse_opt_abbrev_cb, 0 }
#define OPT__COLOR(var, h) OPT_COLOR_FLAG(0, "color", (var), (h))
#define OPT_COLUMN(s, l, v, h) { OPTION_CALLBACK, (s), (l), (v), N_("style"), (h), PARSE_OPT_OPTARG, parseopt_column_callback }
#define OPT_PASSTHRU(s, l, v, a, h, f) { OPTION_CALLBACK, (s), (l), (v), (a), (h), (f), parse_opt_passthru }
#define OPT_PASSTHRU_ARGV(s, l, v, a, h, f) { OPTION_CALLBACK, (s), (l), (v), (a), (h), (f), parse_opt_passthru_argv }
#define _OPT_CONTAINS_OR_WITH(name, variable, help, flag) { OPTION_CALLBACK, 0, name, (variable), N_("commit"), (help), PARSE_OPT_LASTARG_DEFAULT | flag, parse_opt_commits, (intptr_t) "HEAD" }
#define OPT_CONTAINS(v, h) _OPT_CONTAINS_OR_WITH("contains", v, h, PARSE_OPT_NONEG)
#define OPT_NO_CONTAINS(v, h) _OPT_CONTAINS_OR_WITH("no-contains", v, h, PARSE_OPT_NONEG)
#define OPT_WITH(v, h) _OPT_CONTAINS_OR_WITH("with", v, h, PARSE_OPT_HIDDEN | PARSE_OPT_NONEG)
#define OPT_WITHOUT(v, h) _OPT_CONTAINS_OR_WITH("without", v, h, PARSE_OPT_HIDDEN | PARSE_OPT_NONEG)
#define OPT_CLEANUP(v) OPT_STRING(0, "cleanup", v, N_("mode"), N_("how to strip spaces and #comments from message"))
#define OPT_PATHSPEC_FROM_FILE(v) OPT_FILENAME(0, "pathspec-from-file", v, N_("read pathspec from file"))
#define OPT_PATHSPEC_FILE_NUL(v) OPT_BOOL(0, "pathspec-file-nul", v, N_("with --pathspec-from-file, pathspec elements are separated with NUL character"))
