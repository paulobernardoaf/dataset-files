#include <grub/types.h>
struct grub_aout32_header
{
grub_uint32_t a_midmag; 
grub_uint32_t a_text; 
grub_uint32_t a_data; 
grub_uint32_t a_bss; 
grub_uint32_t a_syms; 
grub_uint32_t a_entry; 
grub_uint32_t a_trsize; 
grub_uint32_t a_drsize; 
};
struct grub_aout64_header
{
grub_uint32_t a_midmag; 
grub_uint64_t a_text; 
grub_uint64_t a_data; 
grub_uint64_t a_bss; 
grub_uint64_t a_syms; 
grub_uint64_t a_entry; 
grub_uint64_t a_trsize; 
grub_uint64_t a_drsize; 
};
union grub_aout_header
{
struct grub_aout32_header aout32;
struct grub_aout64_header aout64;
};
#define AOUT_TYPE_NONE 0
#define AOUT_TYPE_AOUT32 1
#define AOUT_TYPE_AOUT64 6
#define AOUT32_OMAGIC 0x107 
#define AOUT32_NMAGIC 0x108 
#define AOUT32_ZMAGIC 0x10b 
#define AOUT32_QMAGIC 0xcc 
#define AOUT64_OMAGIC 0x1001
#define AOUT64_ZMAGIC 0x1002
#define AOUT64_NMAGIC 0x1003
#define AOUT_MID_ZERO 0 
#define AOUT_MID_SUN010 1 
#define AOUT_MID_SUN020 2 
#define AOUT_MID_I386 134 
#define AOUT_MID_SPARC 138 
#define AOUT_MID_HP200 200 
#define AOUT_MID_SUN 0x103
#define AOUT_MID_HP300 300 
#define AOUT_MID_HPUX 0x20C 
#define AOUT_MID_HPUX800 0x20B 
#define AOUT_FLAG_PIC 0x10 
#define AOUT_FLAG_DYNAMIC 0x20 
#define AOUT_FLAG_DPMASK 0x30 
#define AOUT_GETMAGIC(header) ((header).a_midmag & 0xffff)
#define AOUT_GETMID(header) ((header).a_midmag >> 16) & 0x03ff)
#define AOUT_GETFLAG(header) ((header).a_midmag >> 26) & 0x3f)
#if !defined(GRUB_UTIL)
int grub_aout_get_type (union grub_aout_header *header);
grub_err_t grub_aout_load (grub_file_t file, int offset,
grub_addr_t load_addr, int load_size,
grub_addr_t bss_end_addr);
#endif
