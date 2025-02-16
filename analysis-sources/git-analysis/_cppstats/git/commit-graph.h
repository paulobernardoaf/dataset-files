#include "git-compat-util.h"
#include "repository.h"
#include "string-list.h"
#include "cache.h"
#include "object-store.h"
#define GIT_TEST_COMMIT_GRAPH "GIT_TEST_COMMIT_GRAPH"
#define GIT_TEST_COMMIT_GRAPH_DIE_ON_LOAD "GIT_TEST_COMMIT_GRAPH_DIE_ON_LOAD"
struct commit;
char *get_commit_graph_filename(struct object_directory *odb);
int open_commit_graph(const char *graph_file, int *fd, struct stat *st);
int parse_commit_in_graph(struct repository *r, struct commit *item);
void load_commit_graph_info(struct repository *r, struct commit *item);
struct tree *get_commit_tree_in_graph(struct repository *r,
const struct commit *c);
struct commit_graph {
int graph_fd;
const unsigned char *data;
size_t data_len;
unsigned char hash_len;
unsigned char num_chunks;
uint32_t num_commits;
struct object_id oid;
char *filename;
struct object_directory *odb;
uint32_t num_commits_in_base;
struct commit_graph *base_graph;
const uint32_t *chunk_oid_fanout;
const unsigned char *chunk_oid_lookup;
const unsigned char *chunk_commit_data;
const unsigned char *chunk_extra_edges;
const unsigned char *chunk_base_graphs;
};
struct commit_graph *load_commit_graph_one_fd_st(int fd, struct stat *st,
struct object_directory *odb);
struct commit_graph *read_commit_graph_one(struct repository *r,
struct object_directory *odb);
struct commit_graph *parse_commit_graph(void *graph_map, int fd,
size_t graph_size);
int generation_numbers_enabled(struct repository *r);
enum commit_graph_write_flags {
COMMIT_GRAPH_WRITE_APPEND = (1 << 0),
COMMIT_GRAPH_WRITE_PROGRESS = (1 << 1),
COMMIT_GRAPH_WRITE_SPLIT = (1 << 2),
COMMIT_GRAPH_WRITE_CHECK_OIDS = (1 << 3)
};
struct split_commit_graph_opts {
int size_multiple;
int max_commits;
timestamp_t expire_time;
};
int write_commit_graph_reachable(struct object_directory *odb,
enum commit_graph_write_flags flags,
const struct split_commit_graph_opts *split_opts);
int write_commit_graph(struct object_directory *odb,
struct string_list *pack_indexes,
struct string_list *commit_hex,
enum commit_graph_write_flags flags,
const struct split_commit_graph_opts *split_opts);
#define COMMIT_GRAPH_VERIFY_SHALLOW (1 << 0)
int verify_commit_graph(struct repository *r, struct commit_graph *g, int flags);
void close_commit_graph(struct raw_object_store *);
void free_commit_graph(struct commit_graph *);
void disable_commit_graph(struct repository *r);
