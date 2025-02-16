#include <grub/lib/arg.h>
#include <grub/command.h>
struct grub_extcmd;
typedef grub_err_t (*grub_extcmd_func_t) (struct grub_extcmd *cmd,
int argc, char **args);
struct grub_extcmd
{
grub_command_t cmd;
grub_extcmd_func_t func;
const struct grub_arg_option *options;
void *data;
struct grub_arg_list *state;
};
typedef struct grub_extcmd *grub_extcmd_t;
grub_extcmd_t grub_reg_ecmd (const char *name,
grub_extcmd_func_t func,
unsigned flags,
const char *summary,
const char *description,
const struct grub_arg_option *parser);
#define grub_register_extcmd(name, func, flags, s, d, p) grub_reg_ecmd (name, func, flags, s, d, p); GRUB_MODATTR ("command", "*" name);
void grub_unregister_extcmd (grub_extcmd_t cmd);
