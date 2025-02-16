

#include <r_types.h>
#include <r_util.h>
#include <r_lib.h>
#include <r_bin.h>



#define APP_NAME_BYTES 32
#define COMPANY_NAME_BYTES 32

R_PACKED (
typedef struct {
ut8 major; 
ut8 minor;
}) Version;

R_PACKED (
typedef struct {
char header[8]; 
Version struct_version; 
Version sdk_version; 
Version app_version; 
ut16 size; 
ut32 offset; 
ut32 crc; 
char name[APP_NAME_BYTES]; 
char company[COMPANY_NAME_BYTES]; 
ut32 icon_resource_id; 
ut32 sym_table_addr; 
ut32 flags; 
ut32 reloc_list_start; 
ut32 num_reloc_entries; 
ut8 uuid[16];
}) PebbleAppInfo;

static bool check_buffer(RBuffer *b) {
ut8 magic[8];
if (r_buf_read_at (b, 0, magic, sizeof (magic)) != sizeof (magic)) {
return false;
}
return !memcmp (magic, "PBLAPP\x00\x00", 8);
}

static bool load_buffer(RBinFile *bf, void **bin_obj, RBuffer *b, ut64 loadaddr, Sdb *sdb){
return check_buffer (b);
}

static ut64 baddr(RBinFile *bf) {
return 0LL;
}


static RList *strings(RBinFile *bf) {
return NULL;
}

static RBinInfo* info(RBinFile *bf) {
RBinInfo *ret = NULL;
PebbleAppInfo pai;
memset (&pai, 0, sizeof (pai));
int reat = r_buf_read_at (bf->buf, 0, (ut8*)&pai, sizeof (pai));
if (reat != sizeof (pai)) {
eprintf ("Truncated Header\n");
return NULL;
}
if (!(ret = R_NEW0 (RBinInfo))) {
return NULL;
}
ret->lang = NULL;
ret->file = strdup (bf->file);
ret->type = strdup ("pebble");
ret->bclass = r_str_ndup (pai.name, 32);
ret->rclass = r_str_ndup (pai.company, 32);
ret->os = strdup ("rtos");
ret->subsystem = strdup ("pebble");
ret->machine = strdup ("watch");
ret->arch = strdup ("arm"); 
ret->has_va = 1;
ret->bits = 16;
ret->big_endian = 0;
ret->dbg_info = 0;
return ret;
}

static RList* sections(RBinFile *bf) {
ut64 textsize = UT64_MAX;
RList *ret = NULL;
RBinSection *ptr = NULL;
PebbleAppInfo pai = {{0}};
if (!r_buf_read_at (bf->buf, 0, (ut8*)&pai, sizeof(pai))) {
eprintf ("Truncated Header\n");
return NULL;
}
if (!(ret = r_list_new ())) {
return NULL;
}
ret->free = free;

if (!(ptr = R_NEW0 (RBinSection))) {
return ret;
}
ptr->name = strdup ("relocs");
ptr->vsize = ptr->size = pai.num_reloc_entries * sizeof (ut32);
ptr->vaddr = ptr->paddr = pai.reloc_list_start;
ptr->perm = R_PERM_RW;
ptr->add = true;
r_list_append (ret, ptr);
if (ptr->vaddr < textsize) {
textsize = ptr->vaddr;
}


if (!(ptr = R_NEW0 (RBinSection))) {
return ret;
}
ptr->name = strdup ("symtab");
ptr->vsize = ptr->size = 0;
ptr->vaddr = ptr->paddr = pai.sym_table_addr;
ptr->perm = R_PERM_R;
ptr->add = true;
r_list_append (ret, ptr);
if (ptr->vaddr < textsize) {
textsize = ptr->vaddr;
}

if (!(ptr = R_NEW0 (RBinSection))) {
return ret;
}
ptr->name = strdup ("text");
ptr->vaddr = ptr->paddr = 0x80;
ptr->vsize = ptr->size = textsize - ptr->paddr;
ptr->perm = R_PERM_RWX;
ptr->add = true;
r_list_append (ret, ptr);

if (!(ptr = R_NEW0 (RBinSection))) {
return ret;
}
ptr->name = strdup ("header");
ptr->vsize = ptr->size = sizeof (PebbleAppInfo);
ptr->vaddr = ptr->paddr = 0;
ptr->perm = R_PERM_R;
ptr->add = true;
r_list_append (ret, ptr);

return ret;
}

#if 0
static RList* relocs(RBinFile *bf) {
RList *ret = NULL;
RBinReloc *ptr = NULL;
ut64 got_addr;
int i;

if (!(ret = r_list_new ()))
return NULL;
ret->free = free;
return ret;
}
#endif

static RList* entries(RBinFile *bf) {
RBinAddr *ptr = NULL;
RList *ret;
PebbleAppInfo pai;
if (!r_buf_read_at (bf->buf, 0, (ut8*)&pai, sizeof(pai))) {
eprintf ("Truncated Header\n");
return NULL;
}
if (!(ret = r_list_new ())) {
return NULL;
}
ret->free = free;
if (!(ptr = R_NEW0 (RBinAddr))) {
return ret;
}
ptr->paddr = pai.offset;
ptr->vaddr = pai.offset;
r_list_append (ret, ptr);
return ret;
}

RBinPlugin r_bin_plugin_pebble = {
.name = "pebble",
.desc = "Pebble Watch App",
.license = "LGPL",
.load_buffer = &load_buffer,
.check_buffer = &check_buffer,
.baddr = &baddr,
.entries = entries,
.sections = sections,
.strings = &strings,
.info = &info,

};

#if !defined(R2_PLUGIN_INCORE)
R_API RLibStruct radare_plugin = {
.type = R_LIB_TYPE_BIN,
.data = &r_bin_plugin_pebble,
.version = R2_VERSION
};
#endif
