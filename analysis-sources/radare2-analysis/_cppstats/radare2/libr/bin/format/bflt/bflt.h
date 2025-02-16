#include <r_types.h>
#include <r_util.h>
#include <r_lib.h>
#include <r_bin.h>
#define FLAT_VERSION 0x00000004L
#define FLAT_FLAG_RAM 0x1 
#define FLAT_FLAG_GOTPIC 0x2 
#define FLAT_FLAG_GZIP 0x4 
#define FLAT_FLAG_GZDATA 0x8 
#define FLAT_FLAG_KTRACE 0x10 
struct bflt_hdr {
char magic[4];
ut32 rev;
ut32 entry;
ut32 data_start;
ut32 data_end;
ut32 bss_end;
ut32 stack_size;
ut32 reloc_start;
ut32 reloc_count;
ut32 flags;
ut32 build_date;
ut32 filler[5];
};
struct reloc_struct_t {
ut32 addr_to_patch;
ut32 data_offset;
};
struct r_bin_bflt_obj {
struct bflt_hdr *hdr;
struct reloc_struct_t *reloc_table;
struct reloc_struct_t *got_table;
RBuffer *b;
ut8 endian;
size_t size;
uint32_t n_got;
};
#define BFLT_HDR_SIZE sizeof (struct bflt_hdr)
#define VALID_GOT_ENTRY(x) (x != 0xFFFFFFFF)
RBinAddr *r_bflt_get_entry(struct r_bin_bflt_obj *bin);
struct r_bin_bflt_obj *r_bin_bflt_new_buf(RBuffer *buf);
void r_bin_bflt_free (struct r_bin_bflt_obj *obj);
