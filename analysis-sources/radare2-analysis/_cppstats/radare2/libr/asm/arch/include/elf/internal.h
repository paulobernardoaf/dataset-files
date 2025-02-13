#define EI_NIDENT 16 
typedef struct elf_internal_ehdr {
unsigned char e_ident[EI_NIDENT]; 
bfd_vma e_entry; 
bfd_size_type e_phoff; 
bfd_size_type e_shoff; 
unsigned long e_version; 
unsigned long e_flags; 
unsigned short e_type; 
unsigned short e_machine; 
unsigned int e_ehsize; 
unsigned int e_phentsize; 
unsigned int e_phnum; 
unsigned int e_shentsize; 
unsigned int e_shnum; 
unsigned int e_shstrndx; 
} Elf_Internal_Ehdr;
struct elf_internal_phdr {
unsigned long p_type; 
unsigned long p_flags; 
bfd_vma p_offset; 
bfd_vma p_vaddr; 
bfd_vma p_paddr; 
bfd_vma p_filesz; 
bfd_vma p_memsz; 
bfd_vma p_align; 
};
typedef struct elf_internal_phdr Elf_Internal_Phdr;
typedef struct elf_internal_shdr {
unsigned int sh_name; 
unsigned int sh_type; 
bfd_vma sh_flags; 
bfd_vma sh_addr; 
bfd_size_type sh_size; 
bfd_size_type sh_entsize; 
unsigned long sh_link; 
unsigned long sh_info; 
file_ptr sh_offset; 
unsigned int sh_addralign; 
asection * bfd_section; 
unsigned char *contents; 
} Elf_Internal_Shdr;
struct elf_internal_sym {
bfd_vma st_value; 
bfd_vma st_size; 
unsigned long st_name; 
unsigned char st_info; 
unsigned char st_other; 
unsigned int st_shndx; 
};
typedef struct elf_internal_sym Elf_Internal_Sym;
typedef struct elf_internal_note {
unsigned long namesz; 
unsigned long descsz; 
unsigned long type; 
char * namedata; 
char * descdata; 
bfd_vma descpos; 
} Elf_Internal_Note;
typedef struct elf_internal_rela {
bfd_vma r_offset; 
bfd_vma r_info; 
bfd_vma r_addend; 
} Elf_Internal_Rela;
typedef struct elf_internal_dyn {
bfd_vma d_tag; 
union {
bfd_vma d_val;
bfd_vma d_ptr;
} d_un;
} Elf_Internal_Dyn;
typedef struct elf_internal_verdef {
unsigned short vd_version; 
unsigned short vd_flags; 
unsigned short vd_ndx; 
unsigned short vd_cnt; 
unsigned long vd_hash; 
unsigned long vd_aux; 
unsigned long vd_next; 
bfd *vd_bfd; 
const char *vd_nodename; 
struct elf_internal_verdef *vd_nextdef; 
struct elf_internal_verdaux *vd_auxptr; 
unsigned int vd_exp_refno; 
} Elf_Internal_Verdef;
typedef struct elf_internal_verdaux {
unsigned long vda_name; 
unsigned long vda_next; 
const char *vda_nodename; 
struct elf_internal_verdaux *vda_nextptr; 
} Elf_Internal_Verdaux;
typedef struct elf_internal_verneed {
unsigned short vn_version; 
unsigned short vn_cnt; 
unsigned long vn_file; 
unsigned long vn_aux; 
unsigned long vn_next; 
bfd *vn_bfd; 
const char *vn_filename; 
struct elf_internal_vernaux *vn_auxptr; 
struct elf_internal_verneed *vn_nextref; 
} Elf_Internal_Verneed;
typedef struct elf_internal_vernaux {
unsigned long vna_hash; 
unsigned short vna_flags; 
unsigned short vna_other; 
unsigned long vna_name; 
unsigned long vna_next; 
const char *vna_nodename; 
struct elf_internal_vernaux *vna_nextptr; 
} Elf_Internal_Vernaux;
typedef struct elf_internal_versym {
unsigned short vs_vers;
} Elf_Internal_Versym;
typedef struct
{
unsigned short int si_boundto;
unsigned short int si_flags;
} Elf_Internal_Syminfo;
typedef struct
{
bfd_vma a_type;
bfd_vma a_val;
} Elf_Internal_Auxv;
struct elf_segment_map
{
struct elf_segment_map *next;
unsigned long p_type;
unsigned long p_flags;
bfd_vma p_paddr;
bfd_vma p_vaddr_offset;
bfd_vma p_align;
unsigned int p_flags_valid : 1;
unsigned int p_paddr_valid : 1;
unsigned int p_align_valid : 1;
unsigned int includes_filehdr : 1;
unsigned int includes_phdrs : 1;
unsigned int count;
asection *sections[1];
};
#define ELF_SECTION_SIZE(sec_hdr, segment) (((sec_hdr->sh_flags & SHF_TLS) == 0 || sec_hdr->sh_type != SHT_NOBITS || segment->p_type == PT_TLS) ? sec_hdr->sh_size : 0)
#define ELF_IS_SECTION_IN_SEGMENT(sec_hdr, segment) (((((sec_hdr->sh_flags & SHF_TLS) != 0) && (segment->p_type == PT_TLS || segment->p_type == PT_LOAD)) || ((sec_hdr->sh_flags & SHF_TLS) == 0 && segment->p_type != PT_TLS)) && (sec_hdr->sh_type == SHT_NOBITS || ((bfd_vma) sec_hdr->sh_offset >= segment->p_offset && (sec_hdr->sh_offset + ELF_SECTION_SIZE(sec_hdr, segment) <= segment->p_offset + segment->p_filesz))) && ((sec_hdr->sh_flags & SHF_ALLOC) == 0 || (sec_hdr->sh_addr >= segment->p_vaddr && (sec_hdr->sh_addr + ELF_SECTION_SIZE(sec_hdr, segment) <= segment->p_vaddr + segment->p_memsz))))
#define ELF_IS_SECTION_IN_SEGMENT_FILE(sec_hdr, segment) (sec_hdr->sh_size > 0 && ELF_IS_SECTION_IN_SEGMENT (sec_hdr, segment))
#define ELF_IS_SECTION_IN_SEGMENT_MEMORY(sec_hdr, segment) (ELF_SECTION_SIZE(sec_hdr, segment) > 0 && ELF_IS_SECTION_IN_SEGMENT (sec_hdr, segment))
