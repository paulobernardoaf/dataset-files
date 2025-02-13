#include <grub/file.h>
#include <grub/symbol.h>
#include <grub/err.h>
#include <grub/types.h>
int grub_loader_is_loaded (void);
void grub_loader_set (grub_err_t (*boot) (void),
grub_err_t (*unload) (void),
int noreturn);
void grub_loader_unset (void);
grub_err_t grub_loader_boot (void);
typedef enum {
GRUB_LOADER_PREBOOT_HOOK_PRIO_NORMAL = 400,
GRUB_LOADER_PREBOOT_HOOK_PRIO_DISK = 300,
GRUB_LOADER_PREBOOT_HOOK_PRIO_CONSOLE = 200,
GRUB_LOADER_PREBOOT_HOOK_PRIO_MEMORY = 100,
} grub_loader_preboot_hook_prio_t;
void *grub_loader_register_preboot_hook (grub_err_t (*preboot_func) (int noret),
grub_err_t (*preboot_rest_func) (void),
grub_loader_preboot_hook_prio_t prio);
void grub_loader_unregister_preboot_hook (void *hnd);
