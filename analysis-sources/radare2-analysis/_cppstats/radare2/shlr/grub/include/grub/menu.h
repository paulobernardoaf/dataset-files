struct grub_menu_entry_class
{
char *name;
struct grub_menu_entry_class *next;
};
struct grub_menu_entry
{
const char *title;
int restricted;
const char *users;
struct grub_menu_entry_class *classes;
const char *sourcecode;
int hotkey;
const char *group;
struct grub_menu_entry *next;
};
typedef struct grub_menu_entry *grub_menu_entry_t;
struct grub_menu
{
int size;
grub_menu_entry_t entry_list;
};
typedef struct grub_menu *grub_menu_t;
grub_err_t grub_menu_entry_add (int argc, const char **args,
const char *sourcecode);
void grub_menu_execute (const char *config, int nested, int batch);
const char *grub_menu_key2name (int key);
int grub_menu_name2key (const char *name);
