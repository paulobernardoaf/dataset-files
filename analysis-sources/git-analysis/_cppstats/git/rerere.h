#include "string-list.h"
struct pathspec;
struct repository;
#define RERERE_AUTOUPDATE 01
#define RERERE_NOAUTOUPDATE 02
#define RERERE_READONLY 04
extern void *RERERE_RESOLVED;
struct rerere_dir;
struct rerere_id {
struct rerere_dir *collection;
int variant;
};
int setup_rerere(struct repository *,struct string_list *, int);
#if !defined(NO_THE_REPOSITORY_COMPATIBILITY_MACROS)
#define rerere(flags) repo_rerere(the_repository, flags)
#endif
int repo_rerere(struct repository *, int);
const char *rerere_path(const struct rerere_id *, const char *file);
int rerere_forget(struct repository *, struct pathspec *);
int rerere_remaining(struct repository *, struct string_list *);
void rerere_clear(struct repository *, struct string_list *);
void rerere_gc(struct repository *, struct string_list *);
#define OPT_RERERE_AUTOUPDATE(v) OPT_UYN(0, "rerere-autoupdate", (v), N_("update the index with reused conflict resolution if possible"))
