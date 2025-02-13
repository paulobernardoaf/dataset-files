struct string_list_item {
char *string;
void *util;
};
typedef int (*compare_strings_fn)(const char *, const char *);
struct string_list {
struct string_list_item *items;
unsigned int nr, alloc;
unsigned int strdup_strings:1;
compare_strings_fn cmp; 
};
#define STRING_LIST_INIT_NODUP { NULL, 0, 0, 0, NULL }
#define STRING_LIST_INIT_DUP { NULL, 0, 0, 1, NULL }
void string_list_init(struct string_list *list, int strdup_strings);
typedef int (*string_list_each_func_t)(struct string_list_item *, void *);
void filter_string_list(struct string_list *list, int free_util,
string_list_each_func_t want, void *cb_data);
void string_list_clear(struct string_list *list, int free_util);
typedef void (*string_list_clear_func_t)(void *p, const char *str);
void string_list_clear_func(struct string_list *list, string_list_clear_func_t clearfunc);
int for_each_string_list(struct string_list *list,
string_list_each_func_t func, void *cb_data);
#define for_each_string_list_item(item,list) for (item = (list)->items; item && item < (list)->items + (list)->nr; ++item)
void string_list_remove_empty_items(struct string_list *list, int free_util);
int string_list_has_string(const struct string_list *list, const char *string);
int string_list_find_insert_index(const struct string_list *list, const char *string,
int negative_existing_index);
struct string_list_item *string_list_insert(struct string_list *list, const char *string);
void string_list_remove(struct string_list *list, const char *string,
int free_util);
struct string_list_item *string_list_lookup(struct string_list *list, const char *string);
void string_list_remove_duplicates(struct string_list *sorted_list, int free_util);
struct string_list_item *string_list_append(struct string_list *list, const char *string);
struct string_list_item *string_list_append_nodup(struct string_list *list, char *string);
void string_list_sort(struct string_list *list);
int unsorted_string_list_has_string(struct string_list *list, const char *string);
struct string_list_item *unsorted_string_list_lookup(struct string_list *list,
const char *string);
void unsorted_string_list_delete_item(struct string_list *list, int i, int free_util);
int string_list_split(struct string_list *list, const char *string,
int delim, int maxsplit);
int string_list_split_in_place(struct string_list *list, char *string,
int delim, int maxsplit);
