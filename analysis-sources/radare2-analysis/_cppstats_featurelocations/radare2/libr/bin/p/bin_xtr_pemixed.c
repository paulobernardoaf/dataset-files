

#include <r_types.h>
#include <r_util.h>
#include <r_lib.h>
#include <r_bin.h>
#include "pe/pemixed.h"

static RList * oneshotall(RBin *bin, const ut8 *buf, ut64 size);
static RBinXtrData * oneshot(RBin *bin, const ut8 *buf, ut64 size, int subbin_type);

static void free_xtr (void *xtr_obj) {
r_bin_pemixed_free ((struct r_bin_pemixed_obj_t*) xtr_obj);
}

static void destroy(RBin *bin) {
free_xtr (bin->cur->xtr_obj);
}

static bool check_buffer(RBuffer *b) {
return false;
#if 0
if (!bytes) {
return false;
}
if (sz <= 0x3d) { 
return false;
}
ut32 idx = (bytes[0x3c] | (bytes[0x3d]<<8));
if (sz > idx + 0x18 + 2) {



if (!memcmp (bytes, "MZ", 2)) {
if (!memcmp (bytes+idx, "PE", 2) &&
!memcmp (bytes+idx+0x18, "\x0b\x01", 2)) {
return true;
}

if (!memcmp (bytes+idx, "PL", 2)) {
return true;
}
}
}
return false;
#endif
}


static RList * oneshotall(RBin *bin, const ut8 *buf, ut64 size) {

RBinXtrData *data = oneshot (bin, buf, size, SUB_BIN_DOS);

if (!data) {
return NULL;
}

RList * res = r_list_newf (r_bin_xtrdata_free);
r_list_append (res, data);

if ((data = oneshot (bin, buf, size, SUB_BIN_NATIVE))){
r_list_append (res, data);
}

if ((data = oneshot (bin, buf, size, SUB_BIN_NET))){
r_list_append (res, data);
}

return res;
}


static void fill_metadata_info_from_hdr(RBinXtrMetadata *meta, void *foo) {
meta->arch = NULL;
meta->bits = 0;
meta->machine = NULL;
meta->type = NULL;
meta->libname = NULL;
meta->xtr_type = "net";

}


static RBinXtrData * oneshot(RBin *bin, const ut8 *buf, ut64 size, int sub_bin_type) {
r_return_val_if_fail (bin && bin->cur && buf, false);

if (!bin->cur->xtr_obj) {
bin->cur->xtr_obj = r_bin_pemixed_from_bytes_new (buf, size);
}

struct r_bin_pemixed_obj_t* fb = bin->cur->xtr_obj;

struct PE_(r_bin_pe_obj_t)* pe = r_bin_pemixed_extract (fb, sub_bin_type);
if (!pe) {
return NULL;
}
RBinXtrMetadata *metadata = R_NEW0 (RBinXtrMetadata);
if (!metadata) {
return NULL;
}
fill_metadata_info_from_hdr (metadata, pe);
return r_bin_xtrdata_new (pe->b, 0, pe->size, 3, metadata);
}

RBinXtrPlugin r_bin_xtr_plugin_xtr_pemixed = {
.name = "xtr.pemixed",
.desc = "Extract sub-binaries in PE files",
.load = NULL, 
.extract = NULL, 
.extractall = NULL, 
.destroy = &destroy,
.extract_from_bytes = &oneshot,
.extractall_from_bytes = &oneshotall,
.free_xtr = &free_xtr,
.check_buffer = &check_buffer,
};

#if !defined(R2_PLUGIN_INCORE)
R_API RLibStruct radare_plugin = {
.type = R_LIB_TYPE_BIN_XTR,
.data = &r_bin_xtr_plugin_pemixed,
.version = R2_VERSION
};
#endif
