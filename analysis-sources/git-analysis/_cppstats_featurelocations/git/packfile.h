#if !defined(PACKFILE_H)
#define PACKFILE_H

#include "cache.h"
#include "oidset.h"


struct packed_git;
struct object_info;








char *odb_pack_name(struct strbuf *buf, const unsigned char *sha1, const char *ext);






char *sha1_pack_name(const unsigned char *sha1);






char *sha1_pack_index_name(const unsigned char *sha1);





const char *pack_basename(struct packed_git *p);

struct packed_git *parse_pack_index(unsigned char *sha1, const char *idx_path);

typedef void each_file_in_pack_dir_fn(const char *full_path, size_t full_path_len,
const char *file_pach, void *data);
void for_each_file_in_pack_dir(const char *objdir,
each_file_in_pack_dir_fn fn,
void *data);


#define PACKDIR_FILE_PACK 1
#define PACKDIR_FILE_IDX 2
#define PACKDIR_FILE_GARBAGE 4
extern void (*report_garbage)(unsigned seen_bits, const char *path);

void reprepare_packed_git(struct repository *r);
void install_packed_git(struct repository *r, struct packed_git *pack);

struct packed_git *get_packed_git(struct repository *r);
struct list_head *get_packed_git_mru(struct repository *r);
struct multi_pack_index *get_multi_pack_index(struct repository *r);
struct packed_git *get_all_packs(struct repository *r);





unsigned long repo_approximate_object_count(struct repository *r);
#define approximate_object_count() repo_approximate_object_count(the_repository)

struct packed_git *find_sha1_pack(const unsigned char *sha1,
struct packed_git *packs);

void pack_report(void);





int open_pack_index(struct packed_git *);





void close_pack_index(struct packed_git *);

int close_pack_fd(struct packed_git *p);

uint32_t get_pack_fanout(struct packed_git *p, uint32_t value);

unsigned char *use_pack(struct packed_git *, struct pack_window **, off_t, unsigned long *);
void close_pack_windows(struct packed_git *);
void close_pack(struct packed_git *);
void close_object_store(struct raw_object_store *o);
void unuse_pack(struct pack_window **);
void clear_delta_base_cache(void);
struct packed_git *add_packed_git(const char *path, size_t path_len, int local);






void unlink_pack_path(const char *pack_name, int force_delete);









void check_pack_index_ptr(const struct packed_git *p, const void *ptr);







int bsearch_pack(const struct object_id *oid, const struct packed_git *p, uint32_t *result);






int nth_packed_object_id(struct object_id *, struct packed_git *, uint32_t n);





off_t nth_packed_object_offset(const struct packed_git *, uint32_t n);





off_t find_pack_entry_one(const unsigned char *sha1, struct packed_git *);

int is_pack_valid(struct packed_git *);
void *unpack_entry(struct repository *r, struct packed_git *, off_t, enum object_type *, unsigned long *);
unsigned long unpack_object_header_buffer(const unsigned char *buf, unsigned long len, enum object_type *type, unsigned long *sizep);
unsigned long get_size_from_delta(struct packed_git *, struct pack_window **, off_t);
int unpack_object_header(struct packed_git *, struct pack_window **, off_t *, unsigned long *);
off_t get_delta_base(struct packed_git *p, struct pack_window **w_curs,
off_t *curpos, enum object_type type,
off_t delta_obj_offset);

void release_pack_memory(size_t);


extern int do_check_packed_object_crc;

int packed_object_info(struct repository *r,
struct packed_git *pack,
off_t offset, struct object_info *);

void mark_bad_packed_object(struct packed_git *p, const unsigned char *sha1);
const struct packed_git *has_packed_and_bad(struct repository *r, const unsigned char *sha1);





int find_pack_entry(struct repository *r, const struct object_id *oid, struct pack_entry *e);

int has_object_pack(const struct object_id *oid);

int has_pack_index(const unsigned char *sha1);





int is_promisor_object(const struct object_id *oid);











int load_idx(const char *path, const unsigned int hashsz, void *idx_map,
size_t idx_size, struct packed_git *p);

#endif
