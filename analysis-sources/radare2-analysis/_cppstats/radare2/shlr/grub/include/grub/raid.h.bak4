


















#if !defined(GRUB_RAID_H)
#define GRUB_RAID_H 1

#include <grub/types.h>

#define GRUB_RAID_MAX_DEVICES 32

#define GRUB_RAID_LAYOUT_LEFT_ASYMMETRIC 0
#define GRUB_RAID_LAYOUT_RIGHT_ASYMMETRIC 1
#define GRUB_RAID_LAYOUT_LEFT_SYMMETRIC 2
#define GRUB_RAID_LAYOUT_RIGHT_SYMMETRIC 3

#define GRUB_RAID_LAYOUT_RIGHT_MASK 1
#define GRUB_RAID_LAYOUT_SYMMETRIC_MASK 2

struct grub_raid_array
{
int number; 


int level; 
int layout; 
unsigned int total_devs; 
grub_size_t chunk_size; 
grub_uint64_t disk_size; 

grub_uint64_t disk_offset;
int index; 
int uuid_len; 
char *uuid; 


char *name; 
unsigned int nr_devs; 
grub_disk_t device[GRUB_RAID_MAX_DEVICES]; 
grub_uint64_t offset[GRUB_RAID_MAX_DEVICES];

struct grub_raid_array *next;
};

struct grub_raid
{
const char *name;

grub_err_t (*detect) (grub_disk_t disk, struct grub_raid_array *array);

struct grub_raid *next;
};
typedef struct grub_raid *grub_raid_t;

void grub_raid_register (grub_raid_t raid);
void grub_raid_unregister (grub_raid_t raid);

void grub_raid_block_xor (char *buf1, const char *buf2, int size);

typedef grub_err_t (*grub_raid5_recover_func_t) (struct grub_raid_array *array,
int disknr, char *buf,
grub_disk_addr_t sector,
int size);

typedef grub_err_t (*grub_raid6_recover_func_t) (struct grub_raid_array *array,
int disknr, int p, char *buf,
grub_disk_addr_t sector,
int size);

extern grub_raid5_recover_func_t grub_raid5_recover_func;
extern grub_raid6_recover_func_t grub_raid6_recover_func;

#endif 
