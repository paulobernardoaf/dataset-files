#if !defined(WORKTREE_H)
#define WORKTREE_H

#include "cache.h"
#include "refs.h"

struct strbuf;

struct worktree {
char *path;
char *id;
char *head_ref; 
char *lock_reason; 
struct object_id head_oid;
int is_detached;
int is_bare;
int is_current;
int lock_reason_valid; 
};



#define GWT_SORT_LINKED (1 << 0) 









struct worktree **get_worktrees(unsigned flags);




int submodule_uses_worktrees(const char *path);





const char *get_worktree_git_dir(const struct worktree *wt);















struct worktree *find_worktree(struct worktree **list,
const char *prefix,
const char *arg);





struct worktree *find_worktree_by_path(struct worktree **, const char *path);




int is_main_worktree(const struct worktree *wt);





const char *worktree_lock_reason(struct worktree *wt);

#define WT_VALIDATE_WORKTREE_MISSING_OK (1 << 0)





int validate_worktree(const struct worktree *wt,
struct strbuf *errmsg,
unsigned flags);




void update_worktree_location(struct worktree *wt,
const char *path_);




void free_worktrees(struct worktree **);






const struct worktree *find_shared_symref(const char *symref,
const char *target);





int other_head_refs(each_ref_fn fn, void *cb_data);

int is_worktree_being_rebased(const struct worktree *wt, const char *target);
int is_worktree_being_bisected(const struct worktree *wt, const char *target);





const char *worktree_git_path(const struct worktree *wt,
const char *fmt, ...)
__attribute__((format (printf, 2, 3)));








int parse_worktree_ref(const char *worktree_ref, const char **name,
int *name_length, const char **ref);




void strbuf_worktree_ref(const struct worktree *wt,
struct strbuf *sb,
const char *refname);





const char *worktree_ref(const struct worktree *wt,
const char *refname);

#endif
