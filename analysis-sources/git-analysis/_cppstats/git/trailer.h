#include "list.h"
struct strbuf;
enum trailer_where {
WHERE_DEFAULT,
WHERE_END,
WHERE_AFTER,
WHERE_BEFORE,
WHERE_START
};
enum trailer_if_exists {
EXISTS_DEFAULT,
EXISTS_ADD_IF_DIFFERENT_NEIGHBOR,
EXISTS_ADD_IF_DIFFERENT,
EXISTS_ADD,
EXISTS_REPLACE,
EXISTS_DO_NOTHING
};
enum trailer_if_missing {
MISSING_DEFAULT,
MISSING_ADD,
MISSING_DO_NOTHING
};
int trailer_set_where(enum trailer_where *item, const char *value);
int trailer_set_if_exists(enum trailer_if_exists *item, const char *value);
int trailer_set_if_missing(enum trailer_if_missing *item, const char *value);
struct trailer_info {
int blank_line_before_trailer;
const char *trailer_start, *trailer_end;
char **trailers;
size_t trailer_nr;
};
struct new_trailer_item {
struct list_head list;
const char *text;
enum trailer_where where;
enum trailer_if_exists if_exists;
enum trailer_if_missing if_missing;
};
struct process_trailer_options {
int in_place;
int trim_empty;
int only_trailers;
int only_input;
int unfold;
int no_divider;
int value_only;
const struct strbuf *separator;
int (*filter)(const struct strbuf *, void *);
void *filter_data;
};
#define PROCESS_TRAILER_OPTIONS_INIT {0}
void process_trailers(const char *file,
const struct process_trailer_options *opts,
struct list_head *new_trailer_head);
void trailer_info_get(struct trailer_info *info, const char *str,
const struct process_trailer_options *opts);
void trailer_info_release(struct trailer_info *info);
void format_trailers_from_commit(struct strbuf *out, const char *msg,
const struct process_trailer_options *opts);
