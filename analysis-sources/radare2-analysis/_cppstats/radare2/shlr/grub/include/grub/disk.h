#include <grub/symbol.h>
#include <grub/err.h>
#include <grub/types.h>
#include <grub/device.h>
enum grub_disk_dev_id
{
GRUB_DISK_DEVICE_BIOSDISK_ID,
GRUB_DISK_DEVICE_OFDISK_ID,
GRUB_DISK_DEVICE_LOOPBACK_ID,
GRUB_DISK_DEVICE_EFIDISK_ID,
GRUB_DISK_DEVICE_RAID_ID,
GRUB_DISK_DEVICE_LVM_ID,
GRUB_DISK_DEVICE_HOST_ID,
GRUB_DISK_DEVICE_ATA_ID,
GRUB_DISK_DEVICE_MEMDISK_ID,
GRUB_DISK_DEVICE_NAND_ID,
GRUB_DISK_DEVICE_UUID_ID,
GRUB_DISK_DEVICE_PXE_ID,
GRUB_DISK_DEVICE_SCSI_ID,
GRUB_DISK_DEVICE_FILE_ID,
GRUB_DISK_DEVICE_LUKS_ID,
GRUB_DISK_DEVICE_USB_ID,
GRUB_DISK_DEVICE_MAP_ID,
};
struct grub_disk;
#if defined(GRUB_UTIL)
struct grub_disk_memberlist;
#endif
struct grub_disk_dev
{
const char *name;
enum grub_disk_dev_id id;
int (*iterate) (int (*hook) (const char *name, void *closure),
void *closure);
grub_err_t (*open) (const char *name, struct grub_disk *disk);
void (*close) (struct grub_disk *disk);
grub_err_t (*read) (struct grub_disk *disk, grub_disk_addr_t sector,
grub_size_t size, char *buf);
grub_err_t (*write) (struct grub_disk *disk, grub_disk_addr_t sector,
grub_size_t size, const char *buf);
#if defined(GRUB_UTIL)
struct grub_disk_memberlist *(*memberlist) (struct grub_disk *disk);
#endif
struct grub_disk_dev *next;
};
typedef struct grub_disk_dev *grub_disk_dev_t;
struct grub_partition;
struct grub_disk
{
const char *name;
grub_disk_dev_t dev;
grub_uint64_t total_sectors;
int has_partitions;
unsigned long id;
struct grub_partition *partition;
void (*read_hook) (grub_disk_addr_t sector,
unsigned offset, unsigned length, void* closure);
void* closure;
void *data;
};
typedef struct grub_disk *grub_disk_t;
#if defined(GRUB_UTIL)
struct grub_disk_memberlist
{
grub_disk_t disk;
struct grub_disk_memberlist *next;
};
typedef struct grub_disk_memberlist *grub_disk_memberlist_t;
#endif
#define GRUB_DISK_SECTOR_SIZE 0x200
#define GRUB_DISK_SECTOR_BITS 9
#define GRUB_DISK_CACHE_NUM 1021
#define GRUB_DISK_CACHE_SIZE 8
#define GRUB_DISK_CACHE_BITS 3
void grub_disk_cache_invalidate_all (void);
void grub_disk_dev_register (grub_disk_dev_t dev);
void grub_disk_dev_unregister (grub_disk_dev_t dev);
int grub_disk_dev_iterate (int (*hook) (const char *name, void *closure),
void *closure);
grub_disk_t grub_disk_open (const char *name);
void grub_disk_close (grub_disk_t disk);
grub_err_t grub_disk_read (grub_disk_t disk,
grub_disk_addr_t sector,
grub_off_t offset,
grub_size_t size,
void *buf);
grub_err_t grub_disk_read_ex (grub_disk_t disk,
grub_disk_addr_t sector,
grub_off_t offset,
grub_size_t size,
void *buf,
int flags);
grub_err_t grub_disk_write (grub_disk_t disk,
grub_disk_addr_t sector,
grub_off_t offset,
grub_size_t size,
const void *buf);
grub_uint64_t grub_disk_get_size (grub_disk_t disk);
extern void (* grub_disk_firmware_fini) (void);
extern int grub_disk_firmware_is_tainted;
struct grub_disk_ata_pass_through_parms
{
grub_uint8_t taskfile[8];
void * buffer;
int size;
};
extern grub_err_t (* grub_disk_ata_pass_through) (grub_disk_t,
struct grub_disk_ata_pass_through_parms *);
