#include "string-list.h"
struct commit;
struct shortlog {
struct string_list list;
int summary;
int wrap_lines;
int sort_by_number;
int wrap;
int in1;
int in2;
int user_format;
int abbrev;
int committer;
char *common_repo_prefix;
int email;
struct string_list mailmap;
FILE *file;
};
void shortlog_init(struct shortlog *log);
void shortlog_add_commit(struct shortlog *log, struct commit *commit);
void shortlog_output(struct shortlog *log);
