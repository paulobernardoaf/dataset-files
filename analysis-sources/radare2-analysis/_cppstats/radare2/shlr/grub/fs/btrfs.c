#include <grub/err.h>
#include <grub/file.h>
#include <grub/mm.h>
#include <grub/misc.h>
#include <grub/disk.h>
#include <grub/dl.h>
#include <grub/types.h>
#include <r_types.h>
#define BTRFS_SIGNATURE "_BHRfS_M"
R_PACKED(
struct btrfs_superblock
{ 
grub_uint8_t dummy1[32];
grub_uint16_t uuid[8];
grub_uint8_t dummy2[16];
grub_uint8_t signature[sizeof (BTRFS_SIGNATURE) - 1];
});
struct grub_btrfs_data
{
struct btrfs_superblock sblock;
};
static struct grub_btrfs_data *
grub_btrfs_mount (grub_disk_t disk)
{
struct grub_btrfs_data *data = grub_malloc (sizeof (*data));
if (! data)
return NULL;
if (grub_disk_read (disk, 128, 0, sizeof (data->sblock),
&data->sblock) != GRUB_ERR_NONE)
goto fail;
if (grub_memcmp ((char *) data->sblock.signature, BTRFS_SIGNATURE, sizeof (BTRFS_SIGNATURE) - 1))
{
grub_error (GRUB_ERR_BAD_FS, "not a Btrfs filesystem");
goto fail;
}
return data;
fail:
grub_free (data);
return NULL;
}
static grub_err_t
grub_btrfs_open (struct grub_file *file ,
const char *name )
{
return grub_error (GRUB_ERR_NOT_IMPLEMENTED_YET, "only detection is supported for Btrfs");
}
static grub_err_t
grub_btrfs_dir (grub_device_t device,
const char *path ,
int (*hook) (const char *filename,
const struct grub_dirhook_info *info))
{
struct grub_btrfs_data *data = grub_btrfs_mount (device->disk);
if (grub_errno)
return grub_errno;
grub_free (data);
return GRUB_ERR_NONE;
}
static grub_err_t
grub_btrfs_uuid (grub_device_t device, char **uuid)
{
struct grub_btrfs_data *data;
*uuid = NULL;
data = grub_btrfs_mount (device->disk);
if (! data)
return grub_errno;
*uuid = grub_xasprintf ("%04x%04x-%04x-%04x-%04x-%04x%04x%04x",
grub_be_to_cpu16 (data->sblock.uuid[0]),
grub_be_to_cpu16 (data->sblock.uuid[1]),
grub_be_to_cpu16 (data->sblock.uuid[2]),
grub_be_to_cpu16 (data->sblock.uuid[3]),
grub_be_to_cpu16 (data->sblock.uuid[4]),
grub_be_to_cpu16 (data->sblock.uuid[5]),
grub_be_to_cpu16 (data->sblock.uuid[6]),
grub_be_to_cpu16 (data->sblock.uuid[7]));
grub_free (data);
return grub_errno;
}
struct grub_fs grub_btrfs_fs =
{
.name = "btrfs",
.dir = grub_btrfs_dir,
.open = grub_btrfs_open,
.uuid = grub_btrfs_uuid,
};
