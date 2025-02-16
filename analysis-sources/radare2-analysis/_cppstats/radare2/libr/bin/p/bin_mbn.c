#include <r_types.h>
#include <r_util.h>
#include <r_lib.h>
#include <r_bin.h>
typedef struct sbl_header {
ut32 load_index;
ut32 version; 
ut32 paddr; 
ut32 vaddr; 
ut32 psize; 
ut32 code_pa; 
ut32 sign_va;
ut32 sign_sz;
ut32 cert_va; 
ut32 cert_sz;
} SblHeader;
static SblHeader sb = {0};
static bool check_buffer(RBuffer *b) {
r_return_val_if_fail (b, false);
ut64 bufsz = r_buf_size (b);
if (sizeof (SblHeader) < bufsz) {
int ret = r_buf_fread_at (b, 0, (ut8*)&sb, "10i", 1);
if (!ret) {
return false;
}
#if 0
eprintf ("V=%d\n", sb.version);
eprintf ("PA=0x%08x sz=0x%x\n", sb.paddr, sb.psize);
eprintf ("VA=0x%08x sz=0x%x\n", sb.vaddr, sb.psize);
eprintf ("CODE=0x%08x\n", sb.code_pa + sb.vaddr + 40);
eprintf ("SIGN=0x%08x sz=0x%x\n", sb.sign_va, sb.sign_sz);
if (sb.cert_sz > 0) {
eprintf ("CERT=0x%08x sz=0x%x\n", sb.cert_va, sb.cert_sz);
} else {
eprintf ("No certificate found.\n");
}
#endif
if (sb.version != 3) { 
return false;
}
if (sb.paddr + sizeof (SblHeader) > bufsz) { 
return false;
}
if (sb.vaddr < 0x100 || sb.psize > bufsz) { 
return false;
}
if (sb.cert_va < sb.vaddr) {
return false;
}
if (sb.cert_sz >= 0xf0000) {
return false;
}
if (sb.sign_va < sb.vaddr) {
return false;
}
if (sb.sign_sz >= 0xf0000) {
return false;
}
if (sb.load_index < 1 || sb.load_index > 0x40) {
return false; 
}
return true;
}
return false;
}
static bool load_buffer(RBinFile *bf, void **bin_obj, RBuffer *b, ut64 loadaddr, Sdb *sdb){
return check_buffer (b);
}
static ut64 baddr(RBinFile *bf) {
return sb.vaddr; 
}
static RList* entries(RBinFile *bf) {
RList* ret = r_list_newf (free);;
if (ret) {
RBinAddr *ptr = R_NEW0 (RBinAddr);
if (ptr) {
ptr->paddr = 40 + sb.code_pa;
ptr->vaddr = 40 + sb.code_pa + sb.vaddr;
r_list_append (ret, ptr);
}
}
return ret;
}
static RList* sections(RBinFile *bf) {
RBinSection *ptr = NULL;
RList *ret = NULL;
int rc;
if (!(ret = r_list_new ())) {
return NULL;
}
ret->free = free;
rc = r_buf_fread_at (bf->buf, 0, (ut8*)&sb, "10i", 1);
if (!rc) {
r_list_free (ret);
return false;
}
if (!(ptr = R_NEW0 (RBinSection))) {
return ret;
}
ptr->name = strdup ("text");
ptr->size = sb.psize;
ptr->vsize = sb.psize;
ptr->paddr = sb.paddr + 40;
ptr->vaddr = sb.vaddr;
ptr->perm = R_PERM_RX; 
ptr->add = true;
ptr->has_strings = true;
r_list_append (ret, ptr);
if (!(ptr = R_NEW0 (RBinSection))) {
return ret;
}
ptr->name = strdup ("sign");
ptr->size = sb.sign_sz;
ptr->vsize = sb.sign_sz;
ptr->paddr = sb.sign_va - sb.vaddr;
ptr->vaddr = sb.sign_va;
ptr->perm = R_PERM_R; 
ptr->has_strings = true;
ptr->add = true;
r_list_append (ret, ptr);
if (sb.cert_sz && sb.cert_va > sb.vaddr) {
if (!(ptr = R_NEW0 (RBinSection))) {
return ret;
}
ptr->name = strdup ("cert");
ptr->size = sb.cert_sz;
ptr->vsize = sb.cert_sz;
ptr->paddr = sb.cert_va - sb.vaddr;
ptr->vaddr = sb.cert_va;
ptr->perm = R_PERM_R; 
ptr->has_strings = true;
ptr->add = true;
r_list_append (ret, ptr);
}
return ret;
}
static RBinInfo* info(RBinFile *bf) {
RBinInfo *ret = NULL;
const int bits = 16;
if (!(ret = R_NEW0 (RBinInfo))) {
return NULL;
}
ret->file = strdup (bf->file);
ret->bclass = strdup ("bootloader");
ret->rclass = strdup ("mbn");
ret->os = strdup ("MBN");
ret->arch = strdup ("arm");
ret->machine = strdup (ret->arch);
ret->subsystem = strdup ("mbn");
ret->type = strdup ("sbl"); 
ret->bits = bits;
ret->has_va = true;
ret->has_crypto = true; 
ret->has_pi = false;
ret->has_nx = false;
ret->big_endian = false;
ret->dbg_info = false;
return ret;
}
static ut64 size(RBinFile *bf) {
return sizeof (SblHeader) + sb.psize;
}
RBinPlugin r_bin_plugin_mbn = {
.name = "mbn",
.desc = "MBN/SBL bootloader things",
.license = "LGPL3",
.minstrlen = 10,
.load_buffer = &load_buffer,
.size = &size,
.check_buffer = &check_buffer,
.baddr = &baddr,
.entries = &entries,
.sections = &sections,
.info = &info,
};
#if !defined(R2_PLUGIN_INCORE)
R_API RLibStruct radare_plugin = {
.type = R_LIB_TYPE_BIN,
.data = &r_bin_plugin_mbn,
.version = R2_VERSION
};
#endif
