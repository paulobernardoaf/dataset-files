#include <r_types.h>
#include <r_util.h>
#include <r_lib.h>
#include <r_bin.h>

#include "elf_specs.h"

#if !defined(_INCLUDE_ELF_H_)
#define _INCLUDE_ELF_H_

#define R_BIN_ELF_SCN_IS_EXECUTABLE(x) x & SHF_EXECINSTR
#define R_BIN_ELF_SCN_IS_READABLE(x) x & SHF_ALLOC
#define R_BIN_ELF_SCN_IS_WRITABLE(x) x & SHF_WRITE

#define R_BIN_ELF_SYMTAB_SYMBOLS 1 << 0
#define R_BIN_ELF_DYNSYM_SYMBOLS 1 << 1
#define R_BIN_ELF_IMPORT_SYMBOLS (1 << 2 | (bin->ehdr.e_type == ET_REL ? R_BIN_ELF_SYMTAB_SYMBOLS : R_BIN_ELF_DYNSYM_SYMBOLS))
#define R_BIN_ELF_ALL_SYMBOLS (R_BIN_ELF_SYMTAB_SYMBOLS | R_BIN_ELF_DYNSYM_SYMBOLS)
#define ELFOBJ struct Elf_(r_bin_elf_obj_t)

typedef struct r_bin_elf_section_t {
ut64 offset;
ut64 rva;
ut64 size;
ut64 align;
ut32 flags;
ut32 link;
ut32 info;
char name[ELF_STRING_LENGTH];
int last;
int type;
} RBinElfSection;

typedef struct r_bin_elf_symbol_t {
ut64 offset;
ut64 size;
ut32 ordinal;
const char *bind;
const char *type;
char name[ELF_STRING_LENGTH];
char libname[ELF_STRING_LENGTH];
int last;
bool in_shdr;
bool is_sht_null;
bool is_vaddr; 
bool is_imported;
} RBinElfSymbol;

typedef struct r_bin_elf_reloc_t {
int sym;
int type;
int is_rela;
st64 addend;
ut64 offset;
ut64 rva;
ut16 section;
int last;
ut64 sto;
} RBinElfReloc;

typedef struct r_bin_elf_field_t {
ut64 offset;
char name[ELF_STRING_LENGTH];
int last;
} RBinElfField;

typedef struct r_bin_elf_string_t {
ut64 offset;
ut64 size;
char type;
char string[ELF_STRING_LENGTH];
int last;
} RBinElfString;

typedef struct r_bin_elf_lib_t {
char name[ELF_STRING_LENGTH];
int last;
} RBinElfLib;

struct Elf_(r_bin_elf_obj_t) {
Elf_(Ehdr) ehdr;
Elf_(Phdr)* phdr;
Elf_(Shdr)* shdr;

Elf_(Shdr) *strtab_section;
ut64 strtab_size;
char* strtab;

Elf_(Shdr) *shstrtab_section;
ut64 shstrtab_size;
char* shstrtab;

Elf_(Dyn) *dyn_buf;
int dyn_entries;
int is_rela;
ut32 reloc_num;

ut64 version_info[DT_VERSIONTAGNUM];

char *dynstr;
ut32 dynstr_size;

RBinImport **imports_by_ord;
size_t imports_by_ord_size;
RBinSymbol **symbols_by_ord;
size_t symbols_by_ord_size;

int bss;
ut64 size;
ut64 baddr;
ut64 boffset;
int endian;
bool verbose;
const char* file;
RBuffer *b;
Sdb *kv;

RBinElfSection *g_sections;
RBinElfSymbol *g_symbols;
RBinElfSymbol *g_imports;
RBinElfSymbol *phdr_symbols;
RBinElfSymbol *phdr_imports;
HtUP *rel_cache;
};

int Elf_(r_bin_elf_has_va)(struct Elf_(r_bin_elf_obj_t) *bin);
ut64 Elf_(r_bin_elf_get_section_addr)(struct Elf_(r_bin_elf_obj_t) *bin, const char *section_name);
ut64 Elf_(r_bin_elf_get_section_offset)(struct Elf_(r_bin_elf_obj_t) *bin, const char *section_name);
ut64 Elf_(r_bin_elf_get_baddr)(struct Elf_(r_bin_elf_obj_t) *bin);
ut64 Elf_(r_bin_elf_p2v)(struct Elf_(r_bin_elf_obj_t) *bin, ut64 paddr);
ut64 Elf_(r_bin_elf_v2p)(struct Elf_(r_bin_elf_obj_t) *bin, ut64 vaddr);
ut64 Elf_(r_bin_elf_p2v_new)(struct Elf_(r_bin_elf_obj_t) *bin, ut64 paddr);
ut64 Elf_(r_bin_elf_v2p_new)(struct Elf_(r_bin_elf_obj_t) *bin, ut64 vaddr);
ut64 Elf_(r_bin_elf_get_boffset)(struct Elf_(r_bin_elf_obj_t) *bin);
ut64 Elf_(r_bin_elf_get_entry_offset)(struct Elf_(r_bin_elf_obj_t) *bin);
ut64 Elf_(r_bin_elf_get_main_offset)(struct Elf_(r_bin_elf_obj_t) *bin);
ut64 Elf_(r_bin_elf_get_init_offset)(struct Elf_(r_bin_elf_obj_t) *bin);
ut64 Elf_(r_bin_elf_get_fini_offset)(struct Elf_(r_bin_elf_obj_t) *bin);
char *Elf_(r_bin_elf_intrp)(struct Elf_(r_bin_elf_obj_t) *bin);
char *Elf_(r_bin_elf_compiler)(ELFOBJ *bin);
bool Elf_(r_bin_elf_get_stripped)(struct Elf_(r_bin_elf_obj_t) *bin);
bool Elf_(r_bin_elf_is_static)(struct Elf_(r_bin_elf_obj_t) *bin);
char* Elf_(r_bin_elf_get_data_encoding)(struct Elf_(r_bin_elf_obj_t) *bin);
char* Elf_(r_bin_elf_get_arch)(struct Elf_(r_bin_elf_obj_t) *bin);
char* Elf_(r_bin_elf_get_machine_name)(struct Elf_(r_bin_elf_obj_t) *bin);
char* Elf_(r_bin_elf_get_file_type)(struct Elf_(r_bin_elf_obj_t) *bin);
char* Elf_(r_bin_elf_get_elf_class)(struct Elf_(r_bin_elf_obj_t) *bin);
int Elf_(r_bin_elf_get_bits)(struct Elf_(r_bin_elf_obj_t) *bin);
char* Elf_(r_bin_elf_get_osabi_name)(struct Elf_(r_bin_elf_obj_t) *bin);
int Elf_(r_bin_elf_is_big_endian)(struct Elf_(r_bin_elf_obj_t) *bin);
RBinElfReloc* Elf_(r_bin_elf_get_relocs)(struct Elf_(r_bin_elf_obj_t) *bin);
RBinElfLib* Elf_(r_bin_elf_get_libs)(struct Elf_(r_bin_elf_obj_t) *bin);
RBinElfSection* Elf_(r_bin_elf_get_sections)(struct Elf_(r_bin_elf_obj_t) *bin);
RBinElfSymbol* Elf_(r_bin_elf_get_symbols)(struct Elf_(r_bin_elf_obj_t) *bin);
RBinElfSymbol* Elf_(r_bin_elf_get_imports)(struct Elf_(r_bin_elf_obj_t) *bin);
struct r_bin_elf_field_t* Elf_(r_bin_elf_get_fields)(struct Elf_(r_bin_elf_obj_t) *bin);
char *Elf_(r_bin_elf_get_rpath)(struct Elf_(r_bin_elf_obj_t) *bin);

struct Elf_(r_bin_elf_obj_t)* Elf_(r_bin_elf_new)(const char* file, bool verbose);
struct Elf_(r_bin_elf_obj_t)* Elf_(r_bin_elf_new_buf)(RBuffer *buf, bool verbose);
void Elf_(r_bin_elf_free)(struct Elf_(r_bin_elf_obj_t)* bin);

ut64 Elf_(r_bin_elf_resize_section)(RBinFile *bf, const char *name, ut64 size);
bool Elf_(r_bin_elf_section_perms)(RBinFile *bf, const char *name, int perms);
bool Elf_(r_bin_elf_entry_write)(RBinFile *bf, ut64 addr);
bool Elf_(r_bin_elf_del_rpath)(RBinFile *bf);

int Elf_(r_bin_elf_has_relro)(struct Elf_(r_bin_elf_obj_t) *bin);
int Elf_(r_bin_elf_has_nx)(struct Elf_(r_bin_elf_obj_t) *bin);
ut8 *Elf_(r_bin_elf_grab_regstate)(struct Elf_(r_bin_elf_obj_t) *bin, int *len);
RList *Elf_(r_bin_elf_get_maps)(ELFOBJ *bin);
RBinSymbol *Elf_(_r_bin_elf_convert_symbol)(struct Elf_(r_bin_elf_obj_t) *bin,
struct r_bin_elf_symbol_t *symbol,
const char *namefmt);
#endif
