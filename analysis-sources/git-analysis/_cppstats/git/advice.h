#include "git-compat-util.h"
extern int advice_fetch_show_forced_updates;
extern int advice_push_update_rejected;
extern int advice_push_non_ff_current;
extern int advice_push_non_ff_matching;
extern int advice_push_already_exists;
extern int advice_push_fetch_first;
extern int advice_push_needs_force;
extern int advice_push_unqualified_ref_name;
extern int advice_status_hints;
extern int advice_status_u_option;
extern int advice_status_ahead_behind_warning;
extern int advice_commit_before_merge;
extern int advice_reset_quiet_warning;
extern int advice_resolve_conflict;
extern int advice_sequencer_in_use;
extern int advice_implicit_identity;
extern int advice_detached_head;
extern int advice_set_upstream_failure;
extern int advice_object_name_warning;
extern int advice_amworkdir;
extern int advice_rm_hints;
extern int advice_add_embedded_repo;
extern int advice_ignored_hook;
extern int advice_waiting_for_editor;
extern int advice_graft_file_deprecated;
extern int advice_checkout_ambiguous_remote_branch_name;
extern int advice_submodule_alternate_error_strategy_die;
extern int advice_add_ignored_file;
extern int advice_add_empty_pathspec;
enum advice_type {
ADVICE_ADD_EMBEDDED_REPO,
ADVICE_AM_WORK_DIR,
ADVICE_CHECKOUT_AMBIGUOUS_REMOTE_BRANCH_NAME,
ADVICE_COMMIT_BEFORE_MERGE,
ADVICE_DETACHED_HEAD,
ADVICE_FETCH_SHOW_FORCED_UPDATES,
ADVICE_GRAFT_FILE_DEPRECATED,
ADVICE_IGNORED_HOOK,
ADVICE_IMPLICIT_IDENTITY,
ADVICE_NESTED_TAG,
ADVICE_OBJECT_NAME_WARNING,
ADVICE_PUSH_ALREADY_EXISTS,
ADVICE_PUSH_FETCH_FIRST,
ADVICE_PUSH_NEEDS_FORCE,
ADVICE_PUSH_NON_FF_CURRENT,
ADVICE_PUSH_NON_FF_MATCHING,
ADVICE_PUSH_UNQUALIFIED_REF_NAME,
ADVICE_PUSH_UPDATE_REJECTED_ALIAS,
ADVICE_PUSH_UPDATE_REJECTED,
ADVICE_RESET_QUIET_WARNING,
ADVICE_RESOLVE_CONFLICT,
ADVICE_RM_HINTS,
ADVICE_SEQUENCER_IN_USE,
ADVICE_SET_UPSTREAM_FAILURE,
ADVICE_STATUS_AHEAD_BEHIND_WARNING,
ADVICE_STATUS_HINTS,
ADVICE_STATUS_U_OPTION,
ADVICE_SUBMODULE_ALTERNATE_ERROR_STRATEGY_DIE,
ADVICE_WAITING_FOR_EDITOR,
};
int git_default_advice_config(const char *var, const char *value);
__attribute__((format (printf, 1, 2)))
void advise(const char *advice, ...);
int advice_enabled(enum advice_type type);
void advise_if_enabled(enum advice_type type, const char *advice, ...);
int error_resolve_conflict(const char *me);
void NORETURN die_resolve_conflict(const char *me);
void NORETURN die_conclude_merge(void);
void detach_advice(const char *new_name);
