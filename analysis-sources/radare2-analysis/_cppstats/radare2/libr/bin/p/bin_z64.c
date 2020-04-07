#include <r_types.h>
#include <r_util.h>
#include <r_lib.h>
#include <r_bin.h>
#include <r_io.h>
#include <r_cons.h>
#define N64_ROM_START 0x1000
typedef struct {
ut8 x1; 
ut8 x2; 
ut8 x3; 
ut8 x4; 
ut32 ClockRate;
ut32 BootAddress;
ut32 Release;
ut32 CRC1;
ut32 CRC2;
ut64 UNK1;
char Name[20];
ut32 UNK2;
ut16 UNK3;
ut8 UNK4;
ut8 ManufacturerID; 
ut16 CartridgeID;
char CountryCode;
ut8 UNK5;
} N64Header;
static N64Header n64_header;
static ut64 baddr(RBinFile *bf) {
return (ut64) r_read_be32(&n64_header.BootAddress);
}
static bool check_buffer(RBuffer *b) {
ut8 magic[4];
if (r_buf_size (b) < N64_ROM_START) {
return false;
}
(void)r_buf_read_at (b, 0, magic, sizeof (magic));
return !memcmp (magic, "\x80\x37\x12\x40", 4);
}
static bool load_buffer(RBinFile *bf, void **bin_obj, RBuffer *b, ut64 loadaddr, Sdb *sdb) {
if (check_buffer (b)) {
ut8 buf[sizeof (N64Header)] = {0};
r_buf_read_at (b, 0, buf, sizeof (buf));
*bin_obj = memcpy (&n64_header, buf, sizeof (N64Header));
return true;
}
return false;
}
static RList *entries(RBinFile *bf) {
RList *ret = r_list_newf (free);
if (!ret) {
return NULL;
}
RBinAddr *ptr = R_NEW0 (RBinAddr);
if (ptr) {
ptr->paddr = N64_ROM_START;
ptr->vaddr = baddr (bf);
r_list_append (ret, ptr);
}
return ret;
}
static RList *sections(RBinFile *bf) {
RList *ret = r_list_new ();
if (!ret) {
return NULL;
}
RBinSection *text = R_NEW0 (RBinSection);
if (!text) {
r_list_free (ret);
return NULL;
}
text->name = strdup ("text");
text->size = r_buf_size (bf->buf) - N64_ROM_START;
text->vsize = text->size;
text->paddr = N64_ROM_START;
text->vaddr = baddr (bf);
text->perm = R_PERM_RX;
text->add = true;
r_list_append (ret, text);
return ret;
}
static ut64 boffset(RBinFile *bf) {
return 0LL;
}
static RBinInfo *info(RBinFile *bf) {
char GameName[21] = {0};
RBinInfo *ret = R_NEW0 (RBinInfo);
if (!ret) {
return NULL;
}
memcpy (GameName, n64_header.Name, sizeof (n64_header.Name));
ret->file = r_str_newf ("%s (%c)", GameName, n64_header.CountryCode);
ret->os = strdup ("n64");
ret->arch = strdup ("mips");
ret->machine = strdup ("Nintendo 64");
ret->type = strdup ("ROM");
ret->bits = 64;
ret->has_va = true;
ret->big_endian = true;
return ret;
}
#if !R_BIN_Z64
RBinPlugin r_bin_plugin_z64 = {
.name = "z64",
.desc = "Nintendo 64 binaries big endian r_bin plugin",
.license = "LGPL3",
.load_buffer = &load_buffer,
.check_buffer = &check_buffer,
.baddr = baddr,
.boffset = &boffset,
.entries = &entries,
.sections = &sections,
.info = &info
};
#if !defined(R2_PLUGIN_INCORE)
R_API RLibStruct radare_plugin = {
.type = R_LIB_TYPE_BIN,
.data = &r_bin_plugin_z64,
.version = R2_VERSION
};
#endif
#endif
