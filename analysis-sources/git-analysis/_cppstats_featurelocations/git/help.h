#if !defined(HELP_H)
#define HELP_H

#include "string-list.h"
#include "strbuf.h"

struct cmdnames {
int alloc;
int cnt;
struct cmdname {
size_t len; 
char name[FLEX_ARRAY];
} **names;
};

static inline void mput_char(char c, unsigned int num)
{
while (num--)
putchar(c);
}

void list_common_cmds_help(void);
void list_all_cmds_help(void);
void list_common_guides_help(void);
void list_config_help(int for_human);

void list_all_main_cmds(struct string_list *list);
void list_all_other_cmds(struct string_list *list);
void list_cmds_by_category(struct string_list *list,
const char *category);
void list_cmds_by_config(struct string_list *list);
const char *help_unknown_cmd(const char *cmd);
void load_command_list(const char *prefix,
struct cmdnames *main_cmds,
struct cmdnames *other_cmds);
void add_cmdname(struct cmdnames *cmds, const char *name, int len);

void exclude_cmds(struct cmdnames *cmds, struct cmdnames *excludes);
int is_in_cmdlist(struct cmdnames *cmds, const char *name);
void list_commands(unsigned int colopts, struct cmdnames *main_cmds, struct cmdnames *other_cmds);





NORETURN void help_unknown_ref(const char *ref, const char *cmd, const char *error);

static inline void list_config_item(struct string_list *list,
const char *prefix,
const char *str)
{
string_list_append_nodup(list, xstrfmt("%s.%s", prefix, str));
}

#define define_list_config_array(array) void list_config_##array(struct string_list *list, const char *prefix) { int i; for (i = 0; i < ARRAY_SIZE(array); i++) if (array[i]) list_config_item(list, prefix, array[i]); } struct string_list









#define define_list_config_array_extra(array, values) void list_config_##array(struct string_list *list, const char *prefix) { int i; static const char *extra[] = values; for (i = 0; i < ARRAY_SIZE(extra); i++) list_config_item(list, prefix, extra[i]); for (i = 0; i < ARRAY_SIZE(array); i++) if (array[i]) list_config_item(list, prefix, array[i]); } struct string_list













void list_config_advices(struct string_list *list, const char *prefix);
void list_config_color_branch_slots(struct string_list *list, const char *prefix);
void list_config_color_decorate_slots(struct string_list *list, const char *prefix);
void list_config_color_diff_slots(struct string_list *list, const char *prefix);
void list_config_color_grep_slots(struct string_list *list, const char *prefix);
void list_config_color_interactive_slots(struct string_list *list, const char *prefix);
void list_config_color_status_slots(struct string_list *list, const char *prefix);
void list_config_color_sideband_slots(struct string_list *list, const char *prefix);
void list_config_fsck_msg_ids(struct string_list *list, const char *prefix);

#endif 
