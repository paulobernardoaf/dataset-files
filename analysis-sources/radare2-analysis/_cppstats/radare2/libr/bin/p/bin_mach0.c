#include <r_types.h>
#include <r_util.h>
#include <r_lib.h>
#include <r_bin.h>
#include "../i/private.h"
#include "mach0/mach0.h"
#include "objc/mach0_classes.h"
#include <sdb/ht_uu.h>
extern RBinWrite r_bin_write_mach0;
static RBinInfo *info(RBinFile *bf);
static Sdb *get_sdb (RBinFile *bf) {
RBinObject *o = bf->o;
if (!o) {
return NULL;
}
struct MACH0_(obj_t) *bin = (struct MACH0_(obj_t) *) o->bin_obj;
return bin? bin->kv: NULL;
}
static char *entitlements(RBinFile *bf, bool json) {
r_return_val_if_fail (bf && bf->o && bf->o->bin_obj, NULL);
struct MACH0_(obj_t) *bin = bf->o->bin_obj;
return r_str_dup (NULL, (const char*)bin->signature);
}
static bool load_buffer(RBinFile *bf, void **bin_obj, RBuffer *buf, ut64 loadaddr, Sdb *sdb){
r_return_val_if_fail (bf && bin_obj && buf, false);
struct MACH0_(opts_t) opts;
MACH0_(opts_set_default) (&opts, bf);
struct MACH0_(obj_t) *res = MACH0_(new_buf) (buf, &opts);
if (res) {
sdb_ns_set (sdb, "info", res->kv);
*bin_obj = res;
return true;
}
return false;
}
static void destroy(RBinFile *bf) {
MACH0_(mach0_free) (bf->o->bin_obj);
}
static ut64 baddr(RBinFile *bf) {
r_return_val_if_fail (bf && bf->o && bf->o->bin_obj, UT64_MAX);
struct MACH0_(obj_t) *bin = bf->o->bin_obj;
return MACH0_(get_baddr)(bin);
}
static RList *sections(RBinFile *bf) {
return MACH0_(get_segments) (bf);
}
static RBinAddr *newEntry(ut64 hpaddr, ut64 paddr, int type, int bits) {
RBinAddr *ptr = R_NEW0 (RBinAddr);
if (ptr) {
ptr->paddr = paddr;
ptr->vaddr = paddr;
ptr->hpaddr = hpaddr;
ptr->bits = bits;
ptr->type = type;
if (bits == 16 && ptr->vaddr & 1) {
ptr->paddr--;
ptr->vaddr--;
}
}
return ptr;
}
static void process_constructors(RBinFile *bf, RList *ret, int bits) {
RList *secs = sections (bf);
RListIter *iter;
RBinSection *sec;
int i, type;
r_list_foreach (secs, iter, sec) {
type = -1;
if (strstr (sec->name, "_mod_fini_func")) {
type = R_BIN_ENTRY_TYPE_FINI;
} else if (strstr (sec->name, "_mod_init_func")) {
type = R_BIN_ENTRY_TYPE_INIT;
}
if (type != -1) {
ut8 *buf = calloc (sec->size, 1);
if (!buf) {
continue;
}
int read = r_buf_read_at (bf->buf, sec->paddr, buf, sec->size);
if (read < sec->size) {
eprintf ("process_constructors: cannot process section %s\n", sec->name);
continue;
}
if (bits == 32) {
for (i = 0; i + 3 < sec->size; i += 4) {
ut32 addr32 = r_read_le32 (buf + i);
RBinAddr *ba = newEntry (sec->paddr + i, (ut64)addr32, type, bits);
if (ba) {
r_list_append (ret, ba);
}
}
} else {
for (i = 0; i + 7 < sec->size; i += 8) {
ut64 addr64 = r_read_le64 (buf + i);
RBinAddr *ba = newEntry (sec->paddr + i, addr64, type, bits);
if (ba) {
r_list_append (ret, ba);
}
}
}
free (buf);
}
}
r_list_free (secs);
}
static RList *entries(RBinFile *bf) {
r_return_val_if_fail (bf && bf->o, NULL);
RBinAddr *ptr = NULL;
struct addr_t *entry = NULL;
RList *ret = r_list_newf (free);
if (!ret) {
return NULL;
}
int bits = MACH0_(get_bits) (bf->o->bin_obj);
if (!(entry = MACH0_(get_entrypoint) (bf->o->bin_obj))) {
return ret;
}
if ((ptr = R_NEW0 (RBinAddr))) {
ptr->paddr = entry->offset + bf->o->boffset;
ptr->vaddr = entry->addr;
ptr->hpaddr = entry->haddr;
ptr->bits = bits;
if (bits == 16) {
if (ptr->vaddr & 1) {
ptr->paddr--;
ptr->vaddr--;
}
}
r_list_append (ret, ptr);
}
process_constructors (bf, ret, bits);
free (entry);
return ret;
}
static void _handle_arm_thumb(struct MACH0_(obj_t) *bin, RBinSymbol **p) {
RBinSymbol *ptr = *p;
ptr->bits = 32;
if (bin) {
if (ptr->paddr & 1) {
ptr->paddr--;
ptr->vaddr--;
ptr->bits = 16;
}
}
}
#if FEATURE_SYMLIST
static RList *symbols(RBinFile *bf) {
RBinObject *obj = bf? bf->o: NULL;
return (RList *)MACH0_(get_symbols_list) (obj->bin_obj);
}
#else
static RList *symbols(RBinFile *bf) {
struct MACH0_(obj_t) *bin;
int i;
const struct symbol_t *syms = NULL;
RBinSymbol *ptr = NULL;
RBinObject *obj = bf? bf->o: NULL;
RList *ret = r_list_newf (free);
#if 0
const char *lang = "c"; 
#endif
int wordsize = 0;
if (!ret) {
return NULL;
}
if (!obj || !obj->bin_obj) {
free (ret);
return NULL;
}
bool isStripped = false;
wordsize = MACH0_(get_bits) (obj->bin_obj);
if (!(syms = MACH0_(get_symbols) (obj->bin_obj))) {
return ret;
}
Sdb *symcache = sdb_new0 ();
bin = (struct MACH0_(obj_t) *) obj->bin_obj;
for (i = 0; !syms[i].last; i++) {
if (!syms[i].name[0] || syms[i].addr < 100) {
continue;
}
if (!(ptr = R_NEW0 (RBinSymbol))) {
break;
}
ptr->name = strdup ((char*)syms[i].name);
ptr->is_imported = syms[i].is_imported;
if (ptr->name[0] == '_' && !ptr->is_imported) {
char *dn = r_bin_demangle (bf, ptr->name, ptr->name, ptr->vaddr, false);
if (dn) {
ptr->dname = dn;
char *p = strchr (dn, '.');
if (p) {
if (IS_UPPER (ptr->name[0])) {
ptr->classname = strdup (ptr->name);
ptr->classname[p - ptr->name] = 0;
} else if (IS_UPPER (p[1])) {
ptr->classname = strdup (p + 1);
p = strchr (ptr->classname, '.');
if (p) {
*p = 0;
}
}
}
}
}
ptr->forwarder = "NONE";
ptr->bind = (syms[i].type == R_BIN_MACH0_SYMBOL_TYPE_LOCAL)? R_BIN_BIND_LOCAL_STR: R_BIN_BIND_GLOBAL_STR;
ptr->type = R_BIN_TYPE_FUNC_STR;
ptr->vaddr = syms[i].addr;
ptr->paddr = syms[i].offset + obj->boffset;
ptr->size = syms[i].size;
if (bin->hdr.cputype == CPU_TYPE_ARM && wordsize < 64) {
_handle_arm_thumb (bin, &ptr);
}
ptr->ordinal = i;
bin->dbg_info = strncmp (ptr->name, "radr://", 7)? 0: 1;
sdb_set (symcache, sdb_fmt ("sym0x%"PFMT64x, ptr->vaddr), "found", 0);
#if 0
if (!strncmp (ptr->name, "__Z", 3)) {
lang = "c++";
}
if (!strncmp (ptr->name, "type.", 5)) {
lang = "go";
} else if (!strcmp (ptr->name, "_rust_oom")) {
lang = "rust";
}
#endif
r_list_append (ret, ptr);
}
if (bin->func_start) {
char symstr[128];
ut64 value = 0, address = 0;
const ut8 *temp = bin->func_start;
const ut8 *temp_end = bin->func_start + bin->func_size;
strcpy (symstr, "sym0x");
while (temp + 3 < temp_end && *temp) {
temp = r_uleb128_decode (temp, NULL, &value);
address += value;
ptr = R_NEW0 (RBinSymbol);
if (!ptr) {
break;
}
ptr->vaddr = bin->baddr + address;
ptr->paddr = address;
ptr->size = 0;
ptr->name = r_str_newf ("func.%08"PFMT64x, ptr->vaddr);
ptr->type = R_BIN_TYPE_FUNC_STR;
ptr->forwarder = "NONE";
ptr->bind = R_BIN_BIND_LOCAL_STR;
ptr->ordinal = i++;
if (bin->hdr.cputype == CPU_TYPE_ARM && wordsize < 64) {
_handle_arm_thumb (bin, &ptr);
}
r_list_append (ret, ptr);
if (!isStripped) {
snprintf (symstr + 5, sizeof (symstr) - 5 , "%" PFMT64x, ptr->vaddr);
if (!sdb_const_get (symcache, symstr, 0)) {
isStripped = true;
}
}
}
}
#if 0
if (bin->has_blocks_ext) {
lang = !strcmp (lang, "c++") ? "c++ blocks ext." : "c blocks ext.";
}
bin->lang = lang;
#endif
if (isStripped) {
bin->dbg_info |= R_BIN_DBG_STRIPPED;
}
sdb_free (symcache);
return ret;
}
#endif 
static RBinImport *import_from_name(RBin *rbin, const char *orig_name, HtPP *imports_by_name) {
if (imports_by_name) {
bool found = false;
RBinImport *ptr = ht_pp_find (imports_by_name, orig_name, &found);
if (found) {
return ptr;
}
}
RBinImport *ptr = NULL;
if (!(ptr = R_NEW0 (RBinImport))) {
return NULL;
}
char *name = (char*) orig_name;
const char *_objc_class = "_OBJC_CLASS_$";
const int _objc_class_len = strlen (_objc_class);
const char *_objc_metaclass = "_OBJC_METACLASS_$";
const int _objc_metaclass_len = strlen (_objc_metaclass);
char *type = "FUNC";
if (!strncmp (name, _objc_class, _objc_class_len)) {
name += _objc_class_len;
type = "OBJC_CLASS";
} else if (!strncmp (name, _objc_metaclass, _objc_metaclass_len)) {
name += _objc_metaclass_len;
type = "OBJC_METACLASS";
}
if (*name == '_') {
name++;
}
ptr->name = strdup (name);
ptr->bind = "NONE";
ptr->type = r_str_constpool_get (&rbin->constpool, type);
if (imports_by_name) {
ht_pp_insert (imports_by_name, orig_name, ptr);
}
return ptr;
}
static RList *imports(RBinFile *bf) {
RBinObject *obj = bf ? bf->o : NULL;
struct MACH0_(obj_t) *bin = bf ? bf->o->bin_obj : NULL;
struct import_t *imports = NULL;
const char *name;
RBinImport *ptr = NULL;
RList *ret = NULL;
int i;
if (!obj || !bin || !obj->bin_obj || !(ret = r_list_newf (free))) {
return NULL;
}
if (!(imports = MACH0_(get_imports) (bf->o->bin_obj))) {
return ret;
}
bin->has_canary = false;
bin->has_retguard = -1;
bin->has_sanitizers = false;
bin->has_blocks_ext = false;
for (i = 0; !imports[i].last; i++) {
if (!(ptr = import_from_name (bf->rbin, imports[i].name, NULL))) {
break;
}
name = ptr->name;
ptr->ordinal = imports[i].ord;
if (bin->imports_by_ord && ptr->ordinal < bin->imports_by_ord_size) {
bin->imports_by_ord[ptr->ordinal] = ptr;
}
if (!strcmp (name, "__stack_chk_fail") ) {
bin->has_canary = true;
}
if (!strcmp (name, "__asan_init") ||
!strcmp (name, "__tsan_init")) {
bin->has_sanitizers = true;
}
if (!strcmp (name, "_NSConcreteGlobalBlock")) {
bin->has_blocks_ext = true;
}
r_list_append (ret, ptr);
}
free (imports);
return ret;
}
static RList *relocs(RBinFile *bf) {
RList *ret = NULL;
struct MACH0_(obj_t) *bin = NULL;
RBinObject *obj = bf ? bf->o : NULL;
if (bf && bf->o) {
bin = bf->o->bin_obj;
}
if (!obj || !obj->bin_obj || !(ret = r_list_newf (free))) {
return NULL;
}
ret->free = free;
RSkipList *relocs;
if (!(relocs = MACH0_(get_relocs) (bf->o->bin_obj))) {
return ret;
}
RSkipListNode *it;
struct reloc_t *reloc;
r_skiplist_foreach (relocs, it, reloc) {
if (reloc->external) {
continue;
}
RBinReloc *ptr = NULL;
if (!(ptr = R_NEW0 (RBinReloc))) {
break;
}
ptr->type = reloc->type;
ptr->additive = 0;
if (reloc->ord >= 0 && bin->imports_by_ord && reloc->ord < bin->imports_by_ord_size) {
ptr->import = bin->imports_by_ord[reloc->ord];
} else if (reloc->name[0]) {
RBinImport *imp;
if (!(imp = import_from_name (bf->rbin, (char*) reloc->name, bin->imports_by_name))) {
break;
}
ptr->import = imp;
} else {
ptr->import = NULL;
}
ptr->addend = reloc->addend;
ptr->vaddr = reloc->addr;
ptr->paddr = reloc->offset;
r_list_append (ret, ptr);
}
r_skiplist_free (relocs);
return ret;
}
static RList *libs(RBinFile *bf) {
int i;
char *ptr = NULL;
struct lib_t *libs;
RList *ret = NULL;
RBinObject *obj = bf ? bf->o : NULL;
if (!obj || !obj->bin_obj || !(ret = r_list_newf (free))) {
return NULL;
}
if ((libs = MACH0_(get_libs) (obj->bin_obj))) {
for (i = 0; !libs[i].last; i++) {
ptr = strdup (libs[i].name);
r_list_append (ret, ptr);
}
free (libs);
}
return ret;
}
static RBinInfo *info(RBinFile *bf) {
struct MACH0_(obj_t) *bin = NULL;
char *str;
r_return_val_if_fail (bf && bf->o, NULL);
RBinInfo *ret = R_NEW0 (RBinInfo);
if (!ret) {
return NULL;
}
bin = bf->o->bin_obj;
if (bf->file) {
ret->file = strdup (bf->file);
}
if ((str = MACH0_(get_class) (bf->o->bin_obj))) {
ret->bclass = str;
}
if (bin) {
ret->has_canary = bin->has_canary;
ret->has_retguard = -1;
ret->has_sanitizers = bin->has_sanitizers;
ret->dbg_info = bin->dbg_info;
ret->lang = bin->lang;
}
ret->intrp = r_str_dup (NULL, MACH0_(get_intrp)(bf->o->bin_obj));
ret->compiler = r_str_dup (NULL, "");
ret->rclass = strdup ("mach0");
ret->os = strdup (MACH0_(get_os)(bf->o->bin_obj));
ret->subsystem = strdup ("darwin");
ret->arch = strdup (MACH0_(get_cputype) (bf->o->bin_obj));
ret->machine = MACH0_(get_cpusubtype) (bf->o->bin_obj);
ret->has_lit = true;
ret->type = MACH0_(get_filetype) (bf->o->bin_obj);
ret->big_endian = MACH0_(is_big_endian) (bf->o->bin_obj);
ret->bits = 32;
if (bf && bf->o && bf->o->bin_obj) {
ret->has_crypto = ((struct MACH0_(obj_t)*)
bf->o->bin_obj)->has_crypto;
ret->bits = MACH0_(get_bits) (bf->o->bin_obj);
}
ret->has_va = true;
ret->has_pi = MACH0_(is_pie) (bf->o->bin_obj);
ret->has_nx = MACH0_(has_nx) (bf->o->bin_obj);
return ret;
}
static bool _patch_reloc(struct MACH0_(obj_t) *bin, RIOBind *iob, struct reloc_t * reloc, ut64 symbol_at) {
ut64 pc = reloc->addr;
ut64 ins_len = 0;
switch (bin->hdr.cputype) {
case CPU_TYPE_X86_64: {
switch (reloc->type) {
case X86_64_RELOC_UNSIGNED:
break;
case X86_64_RELOC_BRANCH:
pc -= 1;
ins_len = 5;
break;
default:
eprintf ("Warning: unsupported reloc type for X86_64 (%d), please file a bug.\n", reloc->type);
return false;
}
break;
}
case CPU_TYPE_ARM64:
case CPU_TYPE_ARM64_32:
pc = reloc->addr & ~3;
ins_len = 4;
break;
case CPU_TYPE_ARM:
break;
default:
eprintf ("Warning: unsupported architecture for patching relocs, please file a bug. %s\n", MACH0_(get_cputype_from_hdr)(&bin->hdr));
return false;
}
ut64 val = symbol_at;
if (reloc->pc_relative) {
val = symbol_at - pc - ins_len;
}
ut8 buf[8];
r_write_ble (buf, val, false, reloc->size * 8);
iob->write_at (iob->io, reloc->addr, buf, reloc->size);
return true;
}
static RList* patch_relocs(RBin *b) {
RList *ret = NULL;
RIO *io = NULL;
RBinObject *obj = NULL;
struct MACH0_(obj_t) *bin = NULL;
RIOMap *g = NULL, *s = NULL;
HtUU *relocs_by_sym = NULL;
RIODesc *gotr2desc = NULL;
r_return_val_if_fail (b, NULL);
io = b->iob.io;
if (!io || !io->desc) {
return NULL;
}
obj = r_bin_cur_object (b);
if (!obj) {
return NULL;
}
bin = obj->bin_obj;
RSkipList * all_relocs = MACH0_(get_relocs)(bin);
if (!all_relocs) {
return NULL;
}
RList * ext_relocs = r_list_new ();
if (!ext_relocs) {
goto beach;
}
RSkipListNode *it;
struct reloc_t * reloc;
r_skiplist_foreach (all_relocs, it, reloc) {
if (!reloc->external) {
continue;
}
r_list_append (ext_relocs, reloc);
}
ut64 num_ext_relocs = r_list_length (ext_relocs);
if (!num_ext_relocs) {
goto beach;
}
if (!io->cached) {
eprintf ("Warning: run r2 with -e io.cache=true to fix relocations in disassembly\n");
goto beach;
}
int cdsz = obj->info ? obj->info->bits / 8 : 8;
SdbListIter *iter;
ut64 offset = 0;
ls_foreach (io->maps, iter, s) {
if (s->itv.addr > offset) {
offset = s->itv.addr;
g = s;
}
}
if (!g) {
goto beach;
}
ut64 n_vaddr = g->itv.addr + g->itv.size;
ut64 size = num_ext_relocs * cdsz;
char *muri = r_str_newf ("malloc://%" PFMT64u, size);
gotr2desc = b->iob.open_at (io, muri, R_PERM_R, 0664, n_vaddr);
free (muri);
if (!gotr2desc) {
goto beach;
}
RIOMap *gotr2map = b->iob.map_get (io, n_vaddr);
if (!gotr2map) {
goto beach;
}
gotr2map->name = strdup (".got.r2");
if (!(ret = r_list_newf ((RListFree)free))) {
goto beach;
}
if (!(relocs_by_sym = ht_uu_new0 ())) {
goto beach;
}
ut64 vaddr = n_vaddr;
RListIter *liter;
r_list_foreach (ext_relocs, liter, reloc) {
ut64 sym_addr = 0;
sym_addr = ht_uu_find (relocs_by_sym, reloc->ord, NULL);
if (!sym_addr) {
sym_addr = vaddr;
ht_uu_insert (relocs_by_sym, reloc->ord, vaddr);
vaddr += cdsz;
}
if (!_patch_reloc (bin, &b->iob, reloc, sym_addr)) {
continue;
}
RBinReloc *ptr = NULL;
if (!(ptr = R_NEW0 (RBinReloc))) {
goto beach;
}
ptr->type = reloc->type;
ptr->additive = 0;
RBinImport *imp;
if (!(imp = import_from_name (b, (char*) reloc->name, bin->imports_by_name))) {
R_FREE (ptr);
goto beach;
}
ptr->vaddr = sym_addr;
ptr->import = imp;
r_list_append (ret, ptr);
}
if (r_list_empty (ret)) {
goto beach;
}
ht_uu_free (relocs_by_sym);
r_list_free (ext_relocs);
r_skiplist_free (all_relocs);
return ret;
beach:
r_list_free (ext_relocs);
r_skiplist_free (all_relocs);
r_io_desc_free (gotr2desc);
r_list_free (ret);
ht_uu_free (relocs_by_sym);
return NULL;
}
#if !R_BIN_MACH064
static bool check_buffer(RBuffer *b) {
if (r_buf_size (b) >= 4) {
ut8 buf[4] = {0};
if (r_buf_read_at (b, 0, buf, 4)) {
if (!memcmp (buf, "\xce\xfa\xed\xfe", 4) ||
!memcmp (buf, "\xfe\xed\xfa\xce", 4)) {
return true;
}
}
}
return false;
}
static RBuffer *create(RBin *bin, const ut8 *code, int clen, const ut8 *data, int dlen, RBinArchOptions *opt) {
const bool use_pagezero = true;
const bool use_main = true;
const bool use_dylinker = true;
const bool use_libsystem = true;
const bool use_linkedit = true;
ut32 filesize, codeva, datava;
ut32 ncmds, cmdsize, magiclen;
ut32 p_codefsz = 0, p_codeva = 0, p_codesz = 0, p_codepa = 0;
ut32 p_datafsz = 0, p_datava = 0, p_datasz = 0, p_datapa = 0;
ut32 p_cmdsize = 0, p_entry = 0, p_tmp = 0;
ut32 baddr = 0x1000;
r_return_val_if_fail (bin && opt, NULL);
bool is_arm = strstr (opt->arch, "arm");
RBuffer *buf = r_buf_new ();
#if !defined(R_BIN_MACH064)
if (opt->bits == 64) {
eprintf ("TODO: Please use mach064 instead of mach0\n");
free (buf);
return NULL;
}
#endif
#define B(x,y) r_buf_append_bytes(buf,(const ut8*)(x),y)
#define D(x) r_buf_append_ut32(buf,x)
#define Z(x) r_buf_append_nbytes(buf,x)
#define W(x,y,z) r_buf_write_at(buf,x,(const ut8*)(y),z)
#define WZ(x,y) p_tmp=r_buf_size (buf);Z(x);W(p_tmp,y,strlen(y))
B ("\xce\xfa\xed\xfe", 4); 
if (is_arm) {
D (12); 
D (3); 
} else {
D (7); 
D (3); 
}
D (2); 
if (data && dlen > 0) {
ncmds = 3;
cmdsize = 0;
} else {
ncmds = 2;
cmdsize = 0;
}
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
magiclen = r_buf_size (buf);
if (use_pagezero) {
D (1); 
D (56); 
WZ (16, "__PAGEZERO");
D (0); 
D (0x00001000); 
D (0); 
D (0); 
D (0); 
D (0); 
D (0); 
D (0); 
}
D (1); 
D (124); 
WZ (16, "__TEXT");
D (baddr); 
D (0x1000); 
D (0); 
p_codefsz = r_buf_size (buf);
D (-1); 
D (7); 
D (5); 
D (1); 
D (0); 
WZ (16, "__text");
WZ (16, "__TEXT");
p_codeva = r_buf_size (buf); 
D (-1);
p_codesz = r_buf_size (buf); 
D (-1);
p_codepa = r_buf_size (buf); 
D (-1); 
D (0); 
D (0); 
D (0); 
D (0); 
D (0); 
D (0); 
if (data && dlen > 0) {
D (1); 
D (124); 
p_tmp = r_buf_size (buf);
Z (16);
W (p_tmp, "__TEXT", 6); 
D (0x2000); 
D (0x1000); 
D (0); 
p_datafsz = r_buf_size (buf);
D (-1); 
D (6); 
D (6); 
D (1); 
D (0); 
WZ (16, "__data");
WZ (16, "__DATA");
p_datava = r_buf_size (buf);
D (-1);
p_datasz = r_buf_size (buf);
D (-1);
p_datapa = r_buf_size (buf);
D (-1); 
D (2); 
D (0); 
D (0); 
D (0); 
D (0); 
D (0);
}
if (use_dylinker) {
if (use_linkedit) {
D (1); 
D (56); 
WZ (16, "__LINKEDIT");
D (0x3000); 
D (0x00001000); 
D (0x1000); 
D (0); 
D (7); 
D (1); 
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
D (5); 
D (80); 
if (is_arm) {
D (1); 
D (17); 
p_entry = r_buf_size (buf) + (16 * sizeof (ut32));
Z (17 * sizeof (ut32));
} else {
D (1); 
D (16); 
p_entry = r_buf_size (buf) + (10 * sizeof (ut32));
Z (16 * sizeof (ut32));
}
}
WZ (4096 - r_buf_size (buf), "");
cmdsize = r_buf_size (buf) - magiclen;
codeva = r_buf_size (buf) + baddr;
datava = r_buf_size (buf) + clen + baddr;
if (p_entry != 0) {
W (p_entry, &codeva, 4); 
}
W (p_cmdsize, &cmdsize, 4);
filesize = magiclen + cmdsize + clen + dlen;
W (p_codefsz, &filesize, 4);
W (p_codefsz-8, &filesize, 4); 
W (p_codeva, &codeva, 4);
W (p_codesz, &clen, 4);
p_tmp = codeva - baddr;
W (p_codepa, &p_tmp, 4);
B (code, clen);
if (data && dlen > 0) {
W (p_datafsz, &filesize, 4);
W (p_datava, &datava, 4);
W (p_datasz, &dlen, 4);
p_tmp = datava - baddr;
W (p_datapa, &p_tmp, 4);
B (data, dlen);
}
return buf;
}
static RBinAddr *binsym(RBinFile *bf, int sym) {
ut64 addr;
RBinAddr *ret = NULL;
switch (sym) {
case R_BIN_SYM_MAIN:
addr = MACH0_(get_main) (bf->o->bin_obj);
if (addr == UT64_MAX || !(ret = R_NEW0 (RBinAddr))) {
return NULL;
}
ret->vaddr = ((addr >> 1) << 1);
ret->paddr = ret->vaddr;
break;
}
return ret;
}
static ut64 size(RBinFile *bf) {
ut64 off = 0;
ut64 len = 0;
if (!bf->o->sections) {
RListIter *iter;
RBinSection *section;
bf->o->sections = sections (bf);
r_list_foreach (bf->o->sections, iter, section) {
if (section->paddr > off) {
off = section->paddr;
len = section->size;
}
}
}
return off + len;
}
RBinPlugin r_bin_plugin_mach0 = {
.name = "mach0",
.desc = "mach0 bin plugin",
.license = "LGPL3",
.get_sdb = &get_sdb,
.load_buffer = &load_buffer,
.destroy = &destroy,
.check_buffer = &check_buffer,
.baddr = &baddr,
.binsym = &binsym,
.entries = &entries,
.signature = &entitlements,
.sections = &sections,
.symbols = &symbols,
.imports = &imports,
.size = &size,
.info = &info,
.header = MACH0_(mach_headerfields),
.fields = MACH0_(mach_fields),
.libs = &libs,
.relocs = &relocs,
.patch_relocs = &patch_relocs,
.create = &create,
.classes = &MACH0_(parse_classes),
.write = &r_bin_write_mach0,
};
#if !defined(R2_PLUGIN_INCORE)
R_API RLibStruct radare_plugin = {
.type = R_LIB_TYPE_BIN,
.data = &r_bin_plugin_mach0,
.version = R2_VERSION
};
#endif
#endif
