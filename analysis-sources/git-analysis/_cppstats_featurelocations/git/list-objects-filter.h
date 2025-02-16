#if !defined(LIST_OBJECTS_FILTER_H)
#define LIST_OBJECTS_FILTER_H

struct list_objects_filter_options;
struct object;
struct oidset;
struct repository;










































enum list_objects_filter_result {
LOFR_ZERO = 0,
LOFR_MARK_SEEN = 1<<0,
LOFR_DO_SHOW = 1<<1,
LOFR_SKIP_TREE = 1<<2,
};

enum list_objects_filter_situation {
LOFS_BEGIN_TREE,
LOFS_END_TREE,
LOFS_BLOB
};

struct filter;








struct filter *list_objects_filter__init(
struct oidset *omitted,
struct list_objects_filter_options *filter_options);






enum list_objects_filter_result list_objects_filter__filter_object(
struct repository *r,
enum list_objects_filter_situation filter_situation,
struct object *obj,
const char *pathname,
const char *filename,
struct filter *filter);





void list_objects_filter__free(struct filter *filter);

#endif 
