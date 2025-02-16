#if !defined(LINE_LOG_H)
#define LINE_LOG_H

#include "diffcore.h"

struct rev_info;
struct commit;



struct range {
long start, end;
};


struct range_set {
unsigned int alloc, nr;
struct range *ranges;
};



struct diff_ranges {
struct range_set parent;
struct range_set target;
};

void range_set_init(struct range_set *, size_t prealloc);
void range_set_release(struct range_set *);

void range_set_append_unsafe(struct range_set *, long start, long end);

void range_set_append(struct range_set *, long start, long end);




void sort_and_merge_range_set(struct range_set *);







struct line_log_data {
struct line_log_data *next;
char *path;
char status;
struct range_set ranges;
int arg_alloc, arg_nr;
const char **args;
struct diff_filepair *pair;
struct diff_ranges diff;
};

void line_log_init(struct rev_info *rev, const char *prefix, struct string_list *args);

int line_log_filter(struct rev_info *rev);

int line_log_print(struct rev_info *rev, struct commit *commit);

#endif 
