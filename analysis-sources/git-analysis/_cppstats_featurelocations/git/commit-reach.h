#if !defined(COMMIT_REACH_H)
#define COMMIT_REACH_H

#include "commit.h"
#include "commit-slab.h"

struct commit_list;
struct ref_filter;
struct object_id;
struct object_array;

struct commit_list *repo_get_merge_bases(struct repository *r,
struct commit *rev1,
struct commit *rev2);
struct commit_list *repo_get_merge_bases_many(struct repository *r,
struct commit *one, int n,
struct commit **twos);

struct commit_list *repo_get_merge_bases_many_dirty(struct repository *r,
struct commit *one, int n,
struct commit **twos);
#if !defined(NO_THE_REPOSITORY_COMPATIBILITY_MACROS)
#define get_merge_bases(r1, r2) repo_get_merge_bases(the_repository, r1, r2)
#define get_merge_bases_many(one, n, two) repo_get_merge_bases_many(the_repository, one, n, two)
#define get_merge_bases_many_dirty(one, n, twos) repo_get_merge_bases_many_dirty(the_repository, one, n, twos)
#endif

struct commit_list *get_octopus_merge_bases(struct commit_list *in);

int is_descendant_of(struct commit *commit, struct commit_list *with_commit);
int repo_in_merge_bases(struct repository *r,
struct commit *commit,
struct commit *reference);
int repo_in_merge_bases_many(struct repository *r,
struct commit *commit,
int nr_reference, struct commit **reference);
#if !defined(NO_THE_REPOSITORY_COMPATIBILITY_MACROS)
#define in_merge_bases(c1, c2) repo_in_merge_bases(the_repository, c1, c2)
#define in_merge_bases_many(c1, n, cs) repo_in_merge_bases_many(the_repository, c1, n, cs)
#endif











struct commit_list *reduce_heads(struct commit_list *heads);





void reduce_heads_replace(struct commit_list **heads);

int ref_newer(const struct object_id *new_oid, const struct object_id *old_oid);





enum contains_result {
CONTAINS_UNKNOWN = 0,
CONTAINS_NO,
CONTAINS_YES
};

define_commit_slab(contains_cache, enum contains_result);

int commit_contains(struct ref_filter *filter, struct commit *commit,
struct commit_list *list, struct contains_cache *cache);








int can_all_from_reach_with_flag(struct object_array *from,
unsigned int with_flag,
unsigned int assign_flag,
time_t min_commit_date,
uint32_t min_generation);
int can_all_from_reach(struct commit_list *from, struct commit_list *to,
int commit_date_cutoff);










struct commit_list *get_reachable_subset(struct commit **from, int nr_from,
struct commit **to, int nr_to,
unsigned int reachable_flag);

#endif
