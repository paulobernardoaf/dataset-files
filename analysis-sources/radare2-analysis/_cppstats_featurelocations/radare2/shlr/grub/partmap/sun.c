


















#include <grub/partition.h>
#include <grub/disk.h>
#include <grub/mm.h>
#include <grub/misc.h>
#include <grub/dl.h>
#include <grub/symbol.h>
#include <grub/types.h>
#include <grub/err.h>
#include <r_types.h>

#define GRUB_PARTMAP_SUN_MAGIC 0xDABE
#define GRUB_PARTMAP_SUN_MAX_PARTS 8
#define GRUB_PARTMAP_SUN_WHOLE_DISK_ID 0x05

R_PACKED (
struct grub_sun_partition_info
{
grub_uint8_t spare1;
grub_uint8_t id;
grub_uint8_t spare2;
grub_uint8_t flags;
});

R_PACKED (
struct grub_sun_partition_descriptor
{
grub_uint32_t start_cylinder;
grub_uint32_t num_sectors;
});

R_PACKED (
struct grub_sun_block
{
grub_uint8_t info[128]; 
grub_uint8_t spare0[14];
struct grub_sun_partition_info infos[8];
grub_uint8_t spare1[246]; 
grub_uint16_t rspeed; 
grub_uint16_t pcylcount; 
grub_uint16_t sparecyl; 
grub_uint8_t spare2[4]; 
grub_uint16_t ilfact; 
grub_uint16_t ncyl; 
grub_uint16_t nacyl; 
grub_uint16_t ntrks; 
grub_uint16_t nsect; 
grub_uint8_t spare3[4]; 
struct grub_sun_partition_descriptor partitions[8];
grub_uint16_t magic; 
grub_uint16_t csum; 
});

struct grub_partition_map grub_sun_partition_map;


static int
grub_sun_is_valid (struct grub_sun_block *label)
{
grub_unaligned_uint16_t *pos;
grub_uint16_t sum = 0;

for (pos = (grub_uint16_t *) label;
pos < (grub_uint16_t *) (label + 1);
pos++)
sum ^= *pos;

return ! sum;
}

static grub_err_t
sun_partition_map_iterate (grub_disk_t disk,
int (*hook) (grub_disk_t disk,
const grub_partition_t partition,
void *closure),
void *closure)
{
grub_partition_t p;
struct grub_sun_block block;
int partnum;
grub_err_t err;

p = (grub_partition_t) grub_zalloc (sizeof (struct grub_partition));
if (! p)
return grub_errno;

p->partmap = &grub_sun_partition_map;
err = grub_disk_read (disk, 0, 0, sizeof (struct grub_sun_block),
&block);
if (err)
{
grub_free (p);
return err;
}

if (GRUB_PARTMAP_SUN_MAGIC != grub_be_to_cpu16 (block.magic))
{
grub_free (p);
return grub_error (GRUB_ERR_BAD_PART_TABLE, "not a sun partition table");
}

if (! grub_sun_is_valid (&block))
{
grub_free (p);
return grub_error (GRUB_ERR_BAD_PART_TABLE, "invalid checksum");
}



for (partnum = 0; partnum < GRUB_PARTMAP_SUN_MAX_PARTS; partnum++)
{
struct grub_sun_partition_descriptor *desc;

if (block.infos[partnum].id == 0
|| block.infos[partnum].id == GRUB_PARTMAP_SUN_WHOLE_DISK_ID)
continue;

desc = &block.partitions[partnum];
p->start = ((grub_uint64_t) grub_be_to_cpu32 (desc->start_cylinder)
* grub_be_to_cpu16 (block.ntrks)
* grub_be_to_cpu16 (block.nsect));
p->len = grub_be_to_cpu32 (desc->num_sectors);
p->number = p->index = partnum;
if (p->len)
{
if (hook (disk, p, closure))
partnum = GRUB_PARTMAP_SUN_MAX_PARTS;
}
}

grub_free (p);

return grub_errno;
}


struct grub_partition_map grub_sun_partition_map =
{
.name = "sun",
.iterate = sun_partition_map_iterate,
};

