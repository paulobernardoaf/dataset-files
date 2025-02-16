#if !defined(BISECT_H)
#define BISECT_H

struct commit_list;
struct repository;








void find_bisection(struct commit_list **list, int *reaches, int *all,
int find_all);

struct commit_list *filter_skipped(struct commit_list *list,
struct commit_list **tried,
int show_all,
int *count,
int *skipped_first);

#define BISECT_SHOW_ALL (1<<0)
#define REV_LIST_QUIET (1<<1)

struct rev_list_info {
struct rev_info *revs;
int flags;
int show_timestamp;
int hdr_termination;
const char *header_prefix;
};


















enum bisect_error {
BISECT_OK = 0,
BISECT_FAILED = -1,
BISECT_ONLY_SKIPPED_LEFT = -2,
BISECT_MERGE_BASE_CHECK = -3,
BISECT_NO_TESTABLE_COMMIT = -4,
BISECT_INTERNAL_SUCCESS_1ST_BAD_FOUND = -10,
BISECT_INTERNAL_SUCCESS_MERGE_BASE = -11
};

enum bisect_error bisect_next_all(struct repository *r,
const char *prefix,
int no_checkout);

int estimate_bisect_steps(int all);

void read_bisect_terms(const char **bad, const char **good);

int bisect_clean_state(void);

#endif
