#include <grub/device.h>
#include <grub/symbol.h>
#include <grub/types.h>
#include <grub/list.h>
struct grub_file;
struct grub_dirhook_info
{
unsigned int dir:1;
unsigned int mtimeset:1;
unsigned int case_insensitive:1;
grub_int32_t mtime;
};
struct grub_fs
{
struct grub_fs *next;
const char *name;
grub_err_t (*dir) (grub_device_t device, const char *path,
int (*hook) (const char *filename,
const struct grub_dirhook_info *info,
void *closure),
void *closure);
grub_err_t (*open) (struct grub_file *file, const char *name);
grub_ssize_t (*read) (struct grub_file *file, char *buf, grub_size_t len);
grub_err_t (*close) (struct grub_file *file);
grub_err_t (*label) (grub_device_t device, char **label);
grub_err_t (*uuid) (grub_device_t device, char **uuid);
grub_err_t (*mtime) (grub_device_t device, grub_int32_t *timebuf);
#if defined(GRUB_UTIL)
int reserved_first_sector;
#endif
};
typedef struct grub_fs *grub_fs_t;
extern struct grub_fs grub_fs_blocklist;
typedef int (*grub_fs_autoload_hook_t) (void);
extern grub_fs_autoload_hook_t grub_fs_autoload_hook;
extern grub_fs_t grub_fs_list;
static inline void
grub_fs_iterate (int (*hook) (const grub_fs_t fs, void *closure),
void *closure)
{
grub_list_iterate (GRUB_AS_LIST (grub_fs_list), (grub_list_hook_t) hook,
closure);
}
grub_fs_t grub_fs_probe (grub_device_t device);
