#include <grub/symbol.h>
#include <grub/err.h>
#include <grub/list.h>
#define GRUB_COMMAND_FLAG_CMDLINE 0x1
#define GRUB_COMMAND_FLAG_MENU 0x2
#define GRUB_COMMAND_FLAG_BOTH 0x3
#define GRUB_COMMAND_FLAG_TITLE 0x4
#define GRUB_COMMAND_FLAG_NO_ECHO 0x8
#define GRUB_COMMAND_FLAG_EXTCMD 0x10
#define GRUB_COMMAND_FLAG_DYNCMD 0x20
struct grub_command;
typedef grub_err_t (*grub_command_func_t) (struct grub_command *cmd,
int argc, char **argv);
struct grub_command
{
struct grub_command *next;
const char *name;
int prio;
grub_command_func_t func;
unsigned flags;
const char *summary;
const char *description;
void *data;
};
typedef struct grub_command *grub_command_t;
extern grub_command_t grub_command_list;
grub_command_t grub_reg_cmd (const char *name,
grub_command_func_t func,
const char *summary,
const char *description,
int prio);
void grub_unregister_command (grub_command_t cmd);
#define grub_register_command(name, func, summary, description) grub_reg_cmd (name, func, summary, description, 0); GRUB_MODATTR ("command", name);
#define grub_register_command_p1(name, func, summary, description) grub_reg_cmd (name, func, summary, description, 1); GRUB_MODATTR ("command", "*" name);
static inline grub_command_t
grub_command_find (const char *name)
{
return grub_named_list_find (GRUB_AS_NAMED_LIST (grub_command_list), name);
}
static inline grub_err_t
grub_command_execute (const char *name, int argc, char **argv)
{
grub_command_t cmd;
cmd = grub_command_find (name);
return (cmd) ? cmd->func (cmd, argc, argv) : GRUB_ERR_FILE_NOT_FOUND;
}
static inline int
grub_command_iterate (int (*func) (grub_command_t, void *closure),
void *closure)
{
return grub_list_iterate (GRUB_AS_LIST (grub_command_list),
(grub_list_hook_t) func, closure);
}
void grub_register_core_commands (void);
