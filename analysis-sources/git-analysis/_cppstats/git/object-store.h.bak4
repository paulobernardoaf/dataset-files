#if !defined(OBJECT_STORE_H)
#define OBJECT_STORE_H

#include "cache.h"
#include "oidmap.h"
#include "list.h"
#include "sha1-array.h"
#include "strbuf.h"
#include "thread-utils.h"

struct object_directory {
struct object_directory *next;










char loose_objects_subdir_seen[256];
struct oid_array loose_objects_cache[256];





char *path;
};

void prepare_alt_odb(struct repository *r);
char *compute_alternate_path(const char *path, struct strbuf *err);
typedef int alt_odb_fn(struct object_directory *, void *);
int foreach_alt_odb(alt_odb_fn, void*);
typedef void alternate_ref_fn(const struct object_id *oid, void *);
void for_each_alternate_ref(alternate_ref_fn, void *);





void add_to_alternates_file(const char *dir);






void add_to_alternates_memory(const char *dir);





struct oid_array *odb_loose_cache(struct object_directory *odb,
const struct object_id *oid);


void odb_clear_loose_cache(struct object_directory *odb);

struct packed_git {
struct hashmap_entry packmap_ent;
struct packed_git *next;
struct list_head mru;
struct pack_window *windows;
off_t pack_size;
const void *index_data;
size_t index_size;
uint32_t num_objects;
uint32_t num_bad_objects;
unsigned char *bad_object_sha1;
int index_version;
time_t mtime;
int pack_fd;
int index; 
unsigned pack_local:1,
pack_keep:1,
pack_keep_in_core:1,
freshened:1,
do_not_close:1,
pack_promisor:1,
multi_pack_index:1;
unsigned char hash[GIT_MAX_RAWSZ];
struct revindex_entry *revindex;

char pack_name[FLEX_ARRAY]; 
};

struct multi_pack_index;

static inline int pack_map_entry_cmp(const void *unused_cmp_data,
const struct hashmap_entry *entry,
const struct hashmap_entry *entry2,
const void *keydata)
{
const char *key = keydata;
const struct packed_git *pg1, *pg2;

pg1 = container_of(entry, const struct packed_git, packmap_ent);
pg2 = container_of(entry2, const struct packed_git, packmap_ent);

return strcmp(pg1->pack_name, key ? key : pg2->pack_name);
}

struct raw_object_store {





struct object_directory *odb;
struct object_directory **odb_tail;
int loaded_alternates;






char *alternate_db;





struct oidmap *replace_map;
unsigned replace_map_initialized : 1;
pthread_mutex_t replace_mutex; 

struct commit_graph *commit_graph;
unsigned commit_graph_attempted : 1; 






struct multi_pack_index *multi_pack_index;







struct packed_git *packed_git;

struct list_head packed_git_mru;





struct hashmap pack_map;






unsigned long approximate_object_count;
unsigned approximate_object_count_valid : 1;





unsigned packed_git_initialized : 1;
};

struct raw_object_store *raw_object_store_new(void);
void raw_object_store_clear(struct raw_object_store *o);





const char *loose_object_path(struct repository *r, struct strbuf *buf,
const struct object_id *oid);

void *map_loose_object(struct repository *r, const struct object_id *oid,
unsigned long *size);

void *read_object_file_extended(struct repository *r,
const struct object_id *oid,
enum object_type *type,
unsigned long *size, int lookup_replace);
static inline void *repo_read_object_file(struct repository *r,
const struct object_id *oid,
enum object_type *type,
unsigned long *size)
{
return read_object_file_extended(r, oid, type, size, 1);
}
#if !defined(NO_THE_REPOSITORY_COMPATIBILITY_MACROS)
#define read_object_file(oid, type, size) repo_read_object_file(the_repository, oid, type, size)
#endif


int oid_object_info(struct repository *r, const struct object_id *, unsigned long *);

int hash_object_file(const struct git_hash_algo *algo, const void *buf,
unsigned long len, const char *type,
struct object_id *oid);

int write_object_file(const void *buf, unsigned long len,
const char *type, struct object_id *oid);

int hash_object_file_literally(const void *buf, unsigned long len,
const char *type, struct object_id *oid,
unsigned flags);









int pretend_object_file(void *, unsigned long, enum object_type,
struct object_id *oid);

int force_object_loose(const struct object_id *oid, time_t mtime);








int read_loose_object(const char *path,
const struct object_id *expected_oid,
enum object_type *type,
unsigned long *size,
void **contents);

#if !defined(NO_THE_REPOSITORY_COMPATIBILITY_MACROS)
#define has_sha1_file_with_flags(sha1, flags) repo_has_sha1_file_with_flags(the_repository, sha1, flags)
#define has_sha1_file(sha1) repo_has_sha1_file(the_repository, sha1)
#endif


int repo_has_object_file(struct repository *r, const struct object_id *oid);
int repo_has_object_file_with_flags(struct repository *r,
const struct object_id *oid, int flags);
#if !defined(NO_THE_REPOSITORY_COMPATIBILITY_MACROS)
#define has_object_file(oid) repo_has_object_file(the_repository, oid)
#define has_object_file_with_flags(oid, flags) repo_has_object_file_with_flags(the_repository, oid, flags)
#endif






int has_loose_object_nonlocal(const struct object_id *);

void assert_oid_type(const struct object_id *oid, enum object_type expect);

















void enable_obj_read_lock(void);
void disable_obj_read_lock(void);

extern int obj_read_use_lock;
extern pthread_mutex_t obj_read_mutex;

static inline void obj_read_lock(void)
{
if(obj_read_use_lock)
pthread_mutex_lock(&obj_read_mutex);
}

static inline void obj_read_unlock(void)
{
if(obj_read_use_lock)
pthread_mutex_unlock(&obj_read_mutex);
}

struct object_info {

enum object_type *typep;
unsigned long *sizep;
off_t *disk_sizep;
struct object_id *delta_base_oid;
struct strbuf *type_name;
void **contentp;


enum {
OI_CACHED,
OI_LOOSE,
OI_PACKED,
OI_DBCACHED
} whence;
union {








struct {
struct packed_git *pack;
off_t offset;
unsigned int is_delta;
} packed;
} u;
};





#define OBJECT_INFO_INIT {NULL}


#define OBJECT_INFO_LOOKUP_REPLACE 1

#define OBJECT_INFO_ALLOW_UNKNOWN_TYPE 2

#define OBJECT_INFO_QUICK 8

#define OBJECT_INFO_IGNORE_LOOSE 16




#define OBJECT_INFO_SKIP_FETCH_OBJECT 32




#define OBJECT_INFO_FOR_PREFETCH (OBJECT_INFO_SKIP_FETCH_OBJECT | OBJECT_INFO_QUICK)

int oid_object_info_extended(struct repository *r,
const struct object_id *,
struct object_info *, unsigned flags);























typedef int each_loose_object_fn(const struct object_id *oid,
const char *path,
void *data);
typedef int each_loose_cruft_fn(const char *basename,
const char *path,
void *data);
typedef int each_loose_subdir_fn(unsigned int nr,
const char *path,
void *data);
int for_each_file_in_obj_subdir(unsigned int subdir_nr,
struct strbuf *path,
each_loose_object_fn obj_cb,
each_loose_cruft_fn cruft_cb,
each_loose_subdir_fn subdir_cb,
void *data);
int for_each_loose_file_in_objdir(const char *path,
each_loose_object_fn obj_cb,
each_loose_cruft_fn cruft_cb,
each_loose_subdir_fn subdir_cb,
void *data);
int for_each_loose_file_in_objdir_buf(struct strbuf *path,
each_loose_object_fn obj_cb,
each_loose_cruft_fn cruft_cb,
each_loose_subdir_fn subdir_cb,
void *data);


enum for_each_object_flags {

FOR_EACH_OBJECT_LOCAL_ONLY = (1<<0),


FOR_EACH_OBJECT_PROMISOR_ONLY = (1<<1),




FOR_EACH_OBJECT_PACK_ORDER = (1<<2),
};








int for_each_loose_object(each_loose_object_fn, void *,
enum for_each_object_flags flags);









typedef int each_packed_object_fn(const struct object_id *oid,
struct packed_git *pack,
uint32_t pos,
void *data);
int for_each_object_in_pack(struct packed_git *p,
each_packed_object_fn, void *data,
enum for_each_object_flags flags);
int for_each_packed_object(each_packed_object_fn, void *,
enum for_each_object_flags flags);

#endif 
