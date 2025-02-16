#include "test-tool.h"
#include "cache.h"
#include "string-list.h"







static void parse_string_list(struct string_list *list, const char *arg)
{
if (!strcmp(arg, "-"))
return;

(void)string_list_split(list, arg, ':', -1);
}

static void write_list(const struct string_list *list)
{
int i;
for (i = 0; i < list->nr; i++)
printf("[%d]: \"%s\"\n", i, list->items[i].string);
}

static void write_list_compact(const struct string_list *list)
{
int i;
if (!list->nr)
printf("-\n");
else {
printf("%s", list->items[0].string);
for (i = 1; i < list->nr; i++)
printf(":%s", list->items[i].string);
printf("\n");
}
}

static int prefix_cb(struct string_list_item *item, void *cb_data)
{
const char *prefix = (const char *)cb_data;
return starts_with(item->string, prefix);
}

int cmd__string_list(int argc, const char **argv)
{
if (argc == 5 && !strcmp(argv[1], "split")) {
struct string_list list = STRING_LIST_INIT_DUP;
int i;
const char *s = argv[2];
int delim = *argv[3];
int maxsplit = atoi(argv[4]);

i = string_list_split(&list, s, delim, maxsplit);
printf("%d\n", i);
write_list(&list);
string_list_clear(&list, 0);
return 0;
}

if (argc == 5 && !strcmp(argv[1], "split_in_place")) {
struct string_list list = STRING_LIST_INIT_NODUP;
int i;
char *s = xstrdup(argv[2]);
int delim = *argv[3];
int maxsplit = atoi(argv[4]);

i = string_list_split_in_place(&list, s, delim, maxsplit);
printf("%d\n", i);
write_list(&list);
string_list_clear(&list, 0);
free(s);
return 0;
}

if (argc == 4 && !strcmp(argv[1], "filter")) {




struct string_list list = STRING_LIST_INIT_DUP;
const char *prefix = argv[3];

parse_string_list(&list, argv[2]);
filter_string_list(&list, 0, prefix_cb, (void *)prefix);
write_list_compact(&list);
string_list_clear(&list, 0);
return 0;
}

if (argc == 3 && !strcmp(argv[1], "remove_duplicates")) {
struct string_list list = STRING_LIST_INIT_DUP;

parse_string_list(&list, argv[2]);
string_list_remove_duplicates(&list, 0);
write_list_compact(&list);
string_list_clear(&list, 0);
return 0;
}

if (argc == 2 && !strcmp(argv[1], "sort")) {
struct string_list list = STRING_LIST_INIT_NODUP;
struct strbuf sb = STRBUF_INIT;
struct string_list_item *item;

strbuf_read(&sb, 0, 0);





if (sb.len && sb.buf[sb.len - 1] == '\n')
strbuf_setlen(&sb, sb.len - 1);
string_list_split_in_place(&list, sb.buf, '\n', -1);

string_list_sort(&list);

for_each_string_list_item(item, &list)
puts(item->string);

string_list_clear(&list, 0);
strbuf_release(&sb);
return 0;
}

fprintf(stderr, "%s: unknown function name: %s\n", argv[0],
argv[1] ? argv[1] : "(there was none)");
return 1;
}
