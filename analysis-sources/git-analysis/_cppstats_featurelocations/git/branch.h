#if !defined(BRANCH_H)
#define BRANCH_H

struct repository;
struct strbuf;

enum branch_track {
BRANCH_TRACK_UNSPECIFIED = -1,
BRANCH_TRACK_NEVER = 0,
BRANCH_TRACK_REMOTE,
BRANCH_TRACK_ALWAYS,
BRANCH_TRACK_EXPLICIT,
BRANCH_TRACK_OVERRIDE
};

extern enum branch_track git_branch_track;


























void create_branch(struct repository *r,
const char *name, const char *start_name,
int force, int clobber_head_ok,
int reflog, int quiet, enum branch_track track);






int validate_branchname(const char *name, struct strbuf *ref);







int validate_new_branchname(const char *name, struct strbuf *ref, int force);





void remove_merge_branch_state(struct repository *r);





void remove_branch_state(struct repository *r, int verbose);






#define BRANCH_CONFIG_VERBOSE 01
int install_branch_config(int flag, const char *local, const char *origin, const char *remote);




int read_branch_desc(struct strbuf *, const char *branch_name);






void die_if_checked_out(const char *branch, int ignore_current_worktree);






int replace_each_worktree_head_symref(const char *oldref, const char *newref,
const char *logmsg);

#endif
