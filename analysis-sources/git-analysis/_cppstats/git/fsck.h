#include "oidset.h"
#define FSCK_ERROR 1
#define FSCK_WARN 2
#define FSCK_IGNORE 3
struct fsck_options;
struct object;
void fsck_set_msg_type(struct fsck_options *options,
const char *msg_id, const char *msg_type);
void fsck_set_msg_types(struct fsck_options *options, const char *values);
int is_valid_msg_type(const char *msg_id, const char *msg_type);
typedef int (*fsck_walk_func)(struct object *obj, int type, void *data, struct fsck_options *options);
typedef int (*fsck_error)(struct fsck_options *o,
const struct object_id *oid, enum object_type object_type,
int msg_type, const char *message);
int fsck_error_function(struct fsck_options *o,
const struct object_id *oid, enum object_type object_type,
int msg_type, const char *message);
struct fsck_options {
fsck_walk_func walk;
fsck_error error_func;
unsigned strict:1;
int *msg_type;
struct oidset skiplist;
kh_oid_map_t *object_names;
};
#define FSCK_OPTIONS_DEFAULT { NULL, fsck_error_function, 0, NULL, OIDSET_INIT }
#define FSCK_OPTIONS_STRICT { NULL, fsck_error_function, 1, NULL, OIDSET_INIT }
int fsck_walk(struct object *obj, void *data, struct fsck_options *options);
int fsck_object(struct object *obj, void *data, unsigned long size,
struct fsck_options *options);
int fsck_finish(struct fsck_options *options);
void fsck_enable_object_names(struct fsck_options *options);
const char *fsck_get_object_name(struct fsck_options *options,
const struct object_id *oid);
__attribute__((format (printf,3,4)))
void fsck_put_object_name(struct fsck_options *options,
const struct object_id *oid,
const char *fmt, ...);
const char *fsck_describe_object(struct fsck_options *options,
const struct object_id *oid);
