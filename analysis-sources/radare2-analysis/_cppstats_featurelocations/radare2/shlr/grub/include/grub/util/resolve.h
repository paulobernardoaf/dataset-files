

















#if !defined(GRUB_UTIL_RESOLVE_HEADER)
#define GRUB_UTIL_RESOLVE_HEADER 1

struct grub_util_path_list
{
const char *name;
struct grub_util_path_list *next;
};



struct grub_util_path_list *
grub_util_resolve_dependencies (const char *prefix,
const char *dep_list_file,
char *modules[]);

#endif 
