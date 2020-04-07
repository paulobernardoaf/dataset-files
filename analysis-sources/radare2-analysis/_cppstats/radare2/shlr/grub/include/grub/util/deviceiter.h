#include <config.h>
void grub_util_iterate_devices (int (*hook) (const char *, int, void *),
void *closure, int floppy_disks);
void grub_util_emit_devicemap_entry (FILE *fp, char *name, int is_floppy,
int *num_fd, int *num_hd);
