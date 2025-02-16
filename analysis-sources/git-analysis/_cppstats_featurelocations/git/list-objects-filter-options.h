#if !defined(LIST_OBJECTS_FILTER_OPTIONS_H)
#define LIST_OBJECTS_FILTER_OPTIONS_H

#include "parse-options.h"
#include "string-list.h"




enum list_objects_filter_choice {
LOFC_DISABLED = 0,
LOFC_BLOB_NONE,
LOFC_BLOB_LIMIT,
LOFC_TREE_DEPTH,
LOFC_SPARSE_OID,
LOFC_COMBINE,
LOFC__COUNT 
};

struct list_objects_filter_options {









struct string_list filter_spec;





enum list_objects_filter_choice choice;




unsigned int no_filter : 1;






char *sparse_oid_name;
unsigned long blob_limit_value;
unsigned long tree_exclude_depth;




size_t sub_nr, sub_alloc;
struct list_objects_filter_options *sub;




};


#define CL_ARG__FILTER "filter"

void list_objects_filter_die_if_populated(
struct list_objects_filter_options *filter_options);









void parse_list_objects_filter(
struct list_objects_filter_options *filter_options,
const char *arg);

int opt_parse_list_objects_filter(const struct option *opt,
const char *arg, int unset);

#define OPT_PARSE_LIST_OBJECTS_FILTER(fo) { OPTION_CALLBACK, 0, CL_ARG__FILTER, fo, N_("args"), N_("object filtering"), 0, opt_parse_list_objects_filter }












const char *expand_list_objects_filter_spec(
struct list_objects_filter_options *filter);







const char *list_objects_filter_spec(
struct list_objects_filter_options *filter);

void list_objects_filter_release(
struct list_objects_filter_options *filter_options);

static inline void list_objects_filter_set_no_filter(
struct list_objects_filter_options *filter_options)
{
list_objects_filter_release(filter_options);
filter_options->no_filter = 1;
}

void partial_clone_register(
const char *remote,
struct list_objects_filter_options *filter_options);
void partial_clone_get_default_filter_spec(
struct list_objects_filter_options *filter_options,
const char *remote);

#endif 
