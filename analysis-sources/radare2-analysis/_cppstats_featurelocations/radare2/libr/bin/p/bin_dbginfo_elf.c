

#include <r_types.h>
#include <r_bin.h>


static int get_line(RBinFile *bf, ut64 addr, char *file, int len, int *line) {
if (bf->sdb_addrinfo) {
char offset[64];
char *offset_ptr = sdb_itoa (addr, offset, 16);
char *ret = sdb_get (bf->sdb_addrinfo, offset_ptr, 0);
if (ret) {
char *p = strchr (ret, '|');
if (p) {
*p = '\0';
strncpy (file, ret, len);
*line = atoi (p + 1);
return true;
}
}
}
return false;
}

#if !R_BIN_ELF64
struct r_bin_dbginfo_t r_bin_dbginfo_elf = {
.get_line = &get_line,
};
#endif
