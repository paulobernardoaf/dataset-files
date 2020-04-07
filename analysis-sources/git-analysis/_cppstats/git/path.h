struct repository;
struct strbuf;
const char *mkpath(const char *fmt, ...)
__attribute__((format (printf, 1, 2)));
char *mkpathdup(const char *fmt, ...)
__attribute__((format (printf, 1, 2)));
char *mksnpath(char *buf, size_t n, const char *fmt, ...)
__attribute__((format (printf, 3, 4)));
void strbuf_git_common_path(struct strbuf *sb,
const struct repository *repo,
const char *fmt, ...)
__attribute__((format (printf, 3, 4)));
const char *git_common_path(const char *fmt, ...)
__attribute__((format (printf, 1, 2)));
char *repo_git_path(const struct repository *repo,
const char *fmt, ...)
__attribute__((format (printf, 2, 3)));
void strbuf_repo_git_path(struct strbuf *sb,
const struct repository *repo,
const char *fmt, ...)
__attribute__((format (printf, 3, 4)));
const char *git_path(const char *fmt, ...)
__attribute__((format (printf, 1, 2)));
char *git_pathdup(const char *fmt, ...)
__attribute__((format (printf, 1, 2)));
char *git_path_buf(struct strbuf *buf, const char *fmt, ...)
__attribute__((format (printf, 2, 3)));
void strbuf_git_path(struct strbuf *sb, const char *fmt, ...)
__attribute__((format (printf, 2, 3)));
char *repo_worktree_path(const struct repository *repo,
const char *fmt, ...)
__attribute__((format (printf, 2, 3)));
void strbuf_repo_worktree_path(struct strbuf *sb,
const struct repository *repo,
const char *fmt, ...)
__attribute__((format (printf, 3, 4)));
char *git_pathdup_submodule(const char *path, const char *fmt, ...)
__attribute__((format (printf, 2, 3)));
int strbuf_git_path_submodule(struct strbuf *sb, const char *path,
const char *fmt, ...)
__attribute__((format (printf, 3, 4)));
void report_linked_checkout_garbage(void);
#define GIT_PATH_FUNC(func, filename) const char *func(void) { static char *ret; if (!ret) ret = git_pathdup(filename); return ret; }
#define REPO_GIT_PATH_FUNC(var, filename) const char *git_path_##var(struct repository *r) { if (!r->cached_paths.var) r->cached_paths.var = repo_git_path(r, filename); return r->cached_paths.var; }
struct path_cache {
const char *cherry_pick_head;
const char *revert_head;
const char *squash_msg;
const char *merge_msg;
const char *merge_rr;
const char *merge_mode;
const char *merge_head;
const char *fetch_head;
const char *shallow;
};
#define PATH_CACHE_INIT { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
const char *git_path_cherry_pick_head(struct repository *r);
const char *git_path_revert_head(struct repository *r);
const char *git_path_squash_msg(struct repository *r);
const char *git_path_merge_msg(struct repository *r);
const char *git_path_merge_rr(struct repository *r);
const char *git_path_merge_mode(struct repository *r);
const char *git_path_merge_head(struct repository *r);
const char *git_path_fetch_head(struct repository *r);
const char *git_path_shallow(struct repository *r);
int ends_with_path_components(const char *path, const char *components);
