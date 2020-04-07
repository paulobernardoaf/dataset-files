#include <grub/symbol.h>
struct grub_parttool_argdesc
{
char *name;
char *desc;
enum {GRUB_PARTTOOL_ARG_END, GRUB_PARTTOOL_ARG_BOOL, GRUB_PARTTOOL_ARG_VAL}
type;
};
struct grub_parttool_args
{
int set;
union
{
int bool;
char *str;
};
};
typedef grub_err_t (*grub_parttool_function_t) (const grub_device_t dev,
const struct grub_parttool_args *args);
struct grub_parttool
{
struct grub_parttool *next;
char *name;
int handle;
int nargs;
struct grub_parttool_argdesc *args;
grub_parttool_function_t func;
};
#define grub_parttool_register(name, func, args) grub_parttool_reg (name, func, args); GRUB_MODATTR ("parttool", name);
int grub_parttool_reg (const char *part_name,
const grub_parttool_function_t func,
const struct grub_parttool_argdesc *args);
void grub_parttool_unregister (int handle);
