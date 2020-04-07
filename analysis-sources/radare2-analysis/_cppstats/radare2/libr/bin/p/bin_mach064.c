#define R_BIN_MACH064 1
#include "bin_mach0.c"
#include "objc/mach064_classes.h"
#include "../format/mach0/mach064_is_kernelcache.c"
static bool check_buffer(RBuffer *b) {
ut8 buf[4] = {0};
if (r_buf_size (b) > 4) {
r_buf_read_at (b, 0, buf, sizeof (buf));
if (!memcmp (buf, "\xfe\xed\xfa\xcf", 4)) {
return true;
}
if (!memcmp (buf, "\xcf\xfa\xed\xfe", 4)) {
return !is_kernelcache_buffer (b);
}
}
return false;
}
static RBuffer* create(RBin* bin, const ut8 *code, int codelen, const ut8 *data, int datalen, RBinArchOptions *opt) {
const bool use_pagezero = true;
const bool use_main = true;
const bool use_dylinker = true;
const bool use_libsystem = true;
const bool use_linkedit = true;
ut64 filesize, codeva, datava;
ut32 ncmds, magiclen, headerlen;
ut64 p_codefsz=0, p_codeva=0, p_codesz=0, p_codepa=0;
ut64 p_datafsz=0, p_datava=0, p_datasz=0, p_datapa=0;
ut64 p_cmdsize=0, p_entry=0, p_tmp=0;
ut64 baddr = 0x100001000LL;
RBuffer *buf = r_buf_new ();
#define B(x,y) r_buf_append_bytes(buf,(const ut8*)(x),y)
#define D(x) r_buf_append_ut32(buf,x)
#define Q(x) r_buf_append_ut64(buf,x)
#define Z(x) r_buf_append_nbytes(buf,x)
#define W(x,y,z) r_buf_write_at(buf,x,(const ut8*)(y),z)
#define WZ(x,y) p_tmp=r_buf_size (buf);Z(x);W(p_tmp,y,strlen(y))
B ("\xcf\xfa\xed\xfe", 4); 
D (7 | 0x01000000); 
D(0x80000003); 
D (2); 
ncmds = (data && datalen>0)? 3: 2;
if (use_pagezero) {
ncmds++;
}
if (use_dylinker) {
ncmds++;
if (use_linkedit) {
ncmds += 3;
}
if (use_libsystem) {
ncmds++;
}
}
D (ncmds); 
p_cmdsize = r_buf_size (buf);
D (-1); 
D (0);
D (0); 
magiclen = r_buf_size (buf);
if (use_pagezero) {
D (0x19); 
D (72); 
WZ (16, "__PAGEZERO");
Q (0); 
Q (0x1000); 
Q (0); 
Q (0); 
D (0); 
D (0); 
D (0); 
D (0); 
}
D (0x19); 
D (124+28); 
WZ (16, "__TEXT");
Q (baddr); 
Q (0x1000); 
Q (0); 
p_codefsz = r_buf_size (buf);
Q (-1); 
D (7); 
D (5); 
D (1); 
D (0); 
WZ (16, "__text");
WZ (16, "__TEXT");
p_codeva = r_buf_size (buf); 
Q (-1);
p_codesz = r_buf_size (buf); 
Q (-1);
p_codepa = r_buf_size (buf); 
D (-1); 
D (2); 
D (0); 
D (0); 
D (0); 
D (0); 
D (0); 
D (0); 
if (data && datalen > 0) {
D (0x19); 
D (124+28); 
p_tmp = r_buf_size (buf);
Z (16);
W (p_tmp, "__TEXT", 6); 
Q (0x2000); 
Q (0x1000); 
Q (0); 
p_datafsz = r_buf_size (buf);
Q (-1); 
D (6); 
D (6); 
D (1); 
D (0); 
WZ (16, "__data");
WZ (16, "__DATA");
p_datava = r_buf_size (buf);
Q (-1);
p_datasz = r_buf_size (buf);
Q (-1);
p_datapa = r_buf_size (buf);
D (-1); 
D (2); 
D (0); 
D (0); 
D (0); 
D (0); 
D (0); 
D (0); 
}
if (use_dylinker) {
if (use_linkedit) {
D (0x19); 
D (72); 
WZ (16, "__LINKEDIT");
Q (0x3000); 
Q (0x00001000); 
Q (0x1000); 
Q (0); 
D (7); 
D (3); 
D (0); 
D (0); 
D (2); 
D (24); 
D (0x1000); 
D (0); 
D (0x1000); 
D (0); 
D (0xb); 
D (80); 
Z (18 * sizeof (ut32)); 
}
const char *dyld = "/usr/lib/dyld";
const int dyld_len = strlen (dyld) + 1;
D(0xe); 
D((4 * 3) + dyld_len);
D(dyld_len - 2);
WZ(dyld_len, dyld); 
if (use_libsystem) {
const char *lib = "/usr/lib/libSystem.B.dylib";
const int lib_len = strlen (lib) + 1;
D (0xc); 
D (24 + lib_len); 
D (24); 
D (0x2);
D (0x1);
D (0x1);
WZ (lib_len, lib);
}
}
if (use_main) {
D (0x80000028); 
D (24); 
D (baddr); 
D (0); 
D (0); 
D (0); 
} else {
#define STATESIZE (21*sizeof (ut64))
D (5); 
D (184); 
D (4); 
D (42); 
p_entry = r_buf_size (buf) + (16*sizeof (ut64));
Z (STATESIZE);
}
WZ (4096 - r_buf_size (buf), "");
headerlen = r_buf_size (buf) - magiclen;
codeva = r_buf_size (buf) + baddr;
datava = r_buf_size (buf) + codelen + baddr;
if (p_entry != 0) {
W (p_entry, &codeva, 8); 
}
W (p_cmdsize, &headerlen, 4);
filesize = magiclen + headerlen + codelen + datalen;
W (p_codefsz, &filesize, 8);
W (p_codefsz-16, &filesize, 8); 
W (p_codeva, &codeva, 8);
{
ut64 clen = codelen;
W (p_codesz, &clen, 8);
}
p_tmp = codeva - baddr;
W (p_codepa, &p_tmp, 8);
B (code, codelen);
if (data && datalen>0) {
W (p_datafsz, &filesize, 8);
W (p_datava, &datava, 8);
W (p_datasz, &datalen, 8);
p_tmp = datava - baddr;
W (p_datapa, &p_tmp, 8);
B (data, datalen);
}
return buf;
}
static RBinAddr* binsym(RBinFile *bf, int sym) {
ut64 addr;
RBinAddr *ret = NULL;
switch (sym) {
case R_BIN_SYM_MAIN:
addr = MACH0_(get_main) (bf->o->bin_obj);
if (!addr || !(ret = R_NEW0 (RBinAddr))) {
return NULL;
}
ret->paddr = ret->vaddr = addr;
break;
}
return ret;
}
RBinPlugin r_bin_plugin_mach064 = {
.name = "mach064",
.desc = "mach064 bin plugin",
.license = "LGPL3",
.get_sdb = &get_sdb,
.load_buffer = &load_buffer,
.destroy = &destroy,
.check_buffer = &check_buffer,
.baddr = &baddr,
.binsym = binsym,
.entries = &entries,
.sections = &sections,
.signature = &entitlements,
.symbols = &symbols,
.imports = &imports,
.info = &info,
.libs = &libs,
.header = &MACH0_(mach_headerfields),
.relocs = &relocs,
.patch_relocs = &patch_relocs,
.fields = &MACH0_(mach_fields),
.create = &create,
.classes = &MACH0_(parse_classes),
.write = &r_bin_write_mach0,
};
#if !defined(R2_PLUGIN_INCORE)
R_API RLibStruct radare_plugin = {
.type = R_LIB_TYPE_BIN,
.data = &r_bin_plugin_mach064,
.version = R2_VERSION
};
#endif
