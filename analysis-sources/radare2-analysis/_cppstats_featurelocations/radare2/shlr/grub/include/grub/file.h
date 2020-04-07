

















#if !defined(GRUB_FILE_HEADER)
#define GRUB_FILE_HEADER 1

#include <grub/types.h>
#include <grub/err.h>
#include <grub/device.h>
#include <grub/fs.h>


struct grub_file
{

grub_device_t device;


grub_fs_t fs;


grub_off_t offset;


grub_off_t size;


void *data;


void (*read_hook) (grub_disk_addr_t sector,
unsigned offset, unsigned length, void *closure);
void *closure;
int flags;
};
typedef struct grub_file *grub_file_t;


char *grub_file_get_device_name (const char *name);

grub_file_t grub_file_open (const char *name);
grub_ssize_t grub_file_read (grub_file_t file, void *buf,
grub_size_t len);
grub_off_t grub_file_seek (grub_file_t file, grub_off_t offset);
grub_err_t grub_file_close (grub_file_t file);

static inline grub_off_t
grub_file_size (const grub_file_t file)
{
return file->size;
}

static inline grub_off_t
grub_file_tell (const grub_file_t file)
{
return file->offset;
}

void grub_blocklist_convert (grub_file_t file);
grub_ssize_t grub_blocklist_write (grub_file_t file, const char *buf,
grub_size_t len);

#define GRUB_FILE_PB_MIN_SIZE (1 << 20)

extern void (*grub_file_pb_init) (void);
extern void (*grub_file_pb_fini) (void);
extern void (*grub_file_pb_show) (int num, int total);
grub_ssize_t grub_file_pb_read (grub_file_t file, void *buf, grub_size_t len,
int total);

#endif 
