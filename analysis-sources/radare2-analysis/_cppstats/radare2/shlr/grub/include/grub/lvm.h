#include <grub/types.h>
#define GRUB_LVM_ID_STRLEN 38
struct grub_lvm_vg {
char id[GRUB_LVM_ID_STRLEN+1];
char *name;
int extent_size;
struct grub_lvm_pv *pvs;
struct grub_lvm_lv *lvs;
struct grub_lvm_vg *next;
};
struct grub_lvm_pv {
char id[GRUB_LVM_ID_STRLEN+1];
char *name;
grub_disk_t disk;
int start; 
struct grub_lvm_pv *next;
};
struct grub_lvm_lv {
char *name;
unsigned int number;
unsigned int segment_count;
grub_uint64_t size;
struct grub_lvm_segment *segments; 
struct grub_lvm_vg *vg;
struct grub_lvm_lv *next;
};
struct grub_lvm_segment {
unsigned int start_extent;
unsigned int extent_count;
unsigned int stripe_count;
unsigned int stripe_size;
struct grub_lvm_stripe *stripes; 
};
struct grub_lvm_stripe {
int start;
struct grub_lvm_pv *pv;
};
#define GRUB_LVM_LABEL_SIZE GRUB_DISK_SECTOR_SIZE
#define GRUB_LVM_LABEL_SCAN_SECTORS 4L
#define GRUB_LVM_LABEL_ID "LABELONE"
#define GRUB_LVM_LVM2_LABEL "LVM2 001"
#define GRUB_LVM_ID_LEN 32
struct grub_lvm_label_header {
grub_int8_t id[8]; 
grub_uint64_t sector_xl; 
grub_uint32_t crc_xl; 
grub_uint32_t offset_xl; 
grub_int8_t type[8]; 
} __attribute__ ((packed));
struct grub_lvm_disk_locn {
grub_uint64_t offset; 
grub_uint64_t size; 
} __attribute__ ((packed));
struct grub_lvm_pv_header {
grub_int8_t pv_uuid[GRUB_LVM_ID_LEN];
grub_uint64_t device_size_xl; 
struct grub_lvm_disk_locn disk_areas_xl[0]; 
} __attribute__ ((packed));
#define GRUB_LVM_FMTT_MAGIC "\040\114\126\115\062\040\170\133\065\101\045\162\060\116\052\076"
#define GRUB_LVM_FMTT_VERSION 1
#define GRUB_LVM_MDA_HEADER_SIZE 512
struct grub_lvm_raw_locn {
grub_uint64_t offset; 
grub_uint64_t size; 
grub_uint32_t checksum;
grub_uint32_t filler;
} __attribute__ ((packed));
struct grub_lvm_mda_header {
grub_uint32_t checksum_xl; 
grub_int8_t magic[16]; 
grub_uint32_t version;
grub_uint64_t start; 
grub_uint64_t size; 
struct grub_lvm_raw_locn raw_locns[0]; 
} __attribute__ ((packed));
