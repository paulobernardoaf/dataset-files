#if !defined(REF_FILTER_H)
#define REF_FILTER_H

#include "sha1-array.h"
#include "refs.h"
#include "commit.h"
#include "parse-options.h"


#define QUOTE_NONE 0
#define QUOTE_SHELL 1
#define QUOTE_PERL 2
#define QUOTE_PYTHON 4
#define QUOTE_TCL 8

#define FILTER_REFS_INCLUDE_BROKEN 0x0001
#define FILTER_REFS_TAGS 0x0002
#define FILTER_REFS_BRANCHES 0x0004
#define FILTER_REFS_REMOTES 0x0008
#define FILTER_REFS_OTHERS 0x0010
#define FILTER_REFS_ALL (FILTER_REFS_TAGS | FILTER_REFS_BRANCHES | FILTER_REFS_REMOTES | FILTER_REFS_OTHERS)

#define FILTER_REFS_DETACHED_HEAD 0x0020
#define FILTER_REFS_KIND_MASK (FILTER_REFS_ALL | FILTER_REFS_DETACHED_HEAD)

struct atom_value;

struct ref_sorting {
struct ref_sorting *next;
int atom; 
unsigned reverse : 1,
ignore_case : 1,
version : 1;
};

struct ref_array_item {
struct object_id objectname;
int flag;
unsigned int kind;
const char *symref;
struct commit *commit;
struct atom_value *value;
char refname[FLEX_ARRAY];
};

struct ref_array {
int nr, alloc;
struct ref_array_item **items;
struct rev_info *revs;
};

struct ref_filter {
const char **name_patterns;
struct oid_array points_at;
struct commit_list *with_commit;
struct commit_list *no_commit;

enum {
REF_FILTER_MERGED_NONE = 0,
REF_FILTER_MERGED_INCLUDE,
REF_FILTER_MERGED_OMIT
} merge;
struct commit *merge_commit;

unsigned int with_commit_tag_algo : 1,
match_as_path : 1,
ignore_case : 1,
detached : 1;
unsigned int kind,
lines;
int abbrev,
verbose;
};

struct ref_format {




const char *format;
int quote_style;
int use_color;


int need_color_reset_at_eol;
};

#define REF_FORMAT_INIT { NULL, 0, -1 }


#define _OPT_MERGED_NO_MERGED(option, filter, h) { OPTION_CALLBACK, 0, option, (filter), N_("commit"), (h), PARSE_OPT_LASTARG_DEFAULT | PARSE_OPT_NONEG, parse_opt_merge_filter, (intptr_t) "HEAD" }




#define OPT_MERGED(f, h) _OPT_MERGED_NO_MERGED("merged", f, h)
#define OPT_NO_MERGED(f, h) _OPT_MERGED_NO_MERGED("no-merged", f, h)

#define OPT_REF_SORT(var) OPT_CALLBACK_F(0, "sort", (var), N_("key"), N_("field name to sort on"), PARSE_OPT_NONEG, parse_opt_ref_sorting)










int filter_refs(struct ref_array *array, struct ref_filter *filter, unsigned int type);

void ref_array_clear(struct ref_array *array);

int verify_ref_format(struct ref_format *format);

void ref_array_sort(struct ref_sorting *sort, struct ref_array *array);

int format_ref_array_item(struct ref_array_item *info,
const struct ref_format *format,
struct strbuf *final_buf,
struct strbuf *error_buf);

void show_ref_array_item(struct ref_array_item *info, const struct ref_format *format);

void parse_ref_sorting(struct ref_sorting **sorting_tail, const char *atom);

int parse_opt_ref_sorting(const struct option *opt, const char *arg, int unset);

struct ref_sorting *ref_default_sorting(void);

int parse_opt_merge_filter(const struct option *opt, const char *arg, int unset);

char *get_head_description(void);

void setup_ref_filter_porcelain_msg(void);





void pretty_print_ref(const char *name, const struct object_id *oid,
const struct ref_format *format);





struct ref_array_item *ref_array_push(struct ref_array *array,
const char *refname,
const struct object_id *oid);

#endif 
