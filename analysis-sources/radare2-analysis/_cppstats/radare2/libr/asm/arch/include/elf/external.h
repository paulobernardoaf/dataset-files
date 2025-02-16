typedef struct {
unsigned char e_ident[16]; 
unsigned char e_type[2]; 
unsigned char e_machine[2]; 
unsigned char e_version[4]; 
unsigned char e_entry[4]; 
unsigned char e_phoff[4]; 
unsigned char e_shoff[4]; 
unsigned char e_flags[4]; 
unsigned char e_ehsize[2]; 
unsigned char e_phentsize[2]; 
unsigned char e_phnum[2]; 
unsigned char e_shentsize[2]; 
unsigned char e_shnum[2]; 
unsigned char e_shstrndx[2]; 
} Elf32_External_Ehdr;
typedef struct {
unsigned char e_ident[16]; 
unsigned char e_type[2]; 
unsigned char e_machine[2]; 
unsigned char e_version[4]; 
unsigned char e_entry[8]; 
unsigned char e_phoff[8]; 
unsigned char e_shoff[8]; 
unsigned char e_flags[4]; 
unsigned char e_ehsize[2]; 
unsigned char e_phentsize[2]; 
unsigned char e_phnum[2]; 
unsigned char e_shentsize[2]; 
unsigned char e_shnum[2]; 
unsigned char e_shstrndx[2]; 
} Elf64_External_Ehdr;
typedef struct {
unsigned char p_type[4]; 
unsigned char p_offset[4]; 
unsigned char p_vaddr[4]; 
unsigned char p_paddr[4]; 
unsigned char p_filesz[4]; 
unsigned char p_memsz[4]; 
unsigned char p_flags[4]; 
unsigned char p_align[4]; 
} Elf32_External_Phdr;
typedef struct {
unsigned char p_type[4]; 
unsigned char p_flags[4]; 
unsigned char p_offset[8]; 
unsigned char p_vaddr[8]; 
unsigned char p_paddr[8]; 
unsigned char p_filesz[8]; 
unsigned char p_memsz[8]; 
unsigned char p_align[8]; 
} Elf64_External_Phdr;
typedef struct {
unsigned char sh_name[4]; 
unsigned char sh_type[4]; 
unsigned char sh_flags[4]; 
unsigned char sh_addr[4]; 
unsigned char sh_offset[4]; 
unsigned char sh_size[4]; 
unsigned char sh_link[4]; 
unsigned char sh_info[4]; 
unsigned char sh_addralign[4]; 
unsigned char sh_entsize[4]; 
} Elf32_External_Shdr;
typedef struct {
unsigned char sh_name[4]; 
unsigned char sh_type[4]; 
unsigned char sh_flags[8]; 
unsigned char sh_addr[8]; 
unsigned char sh_offset[8]; 
unsigned char sh_size[8]; 
unsigned char sh_link[4]; 
unsigned char sh_info[4]; 
unsigned char sh_addralign[8]; 
unsigned char sh_entsize[8]; 
} Elf64_External_Shdr;
typedef struct {
unsigned char st_name[4]; 
unsigned char st_value[4]; 
unsigned char st_size[4]; 
unsigned char st_info[1]; 
unsigned char st_other[1]; 
unsigned char st_shndx[2]; 
} Elf32_External_Sym;
typedef struct {
unsigned char st_name[4]; 
unsigned char st_info[1]; 
unsigned char st_other[1]; 
unsigned char st_shndx[2]; 
unsigned char st_value[8]; 
unsigned char st_size[8]; 
} Elf64_External_Sym;
typedef struct {
unsigned char est_shndx[4]; 
} Elf_External_Sym_Shndx;
typedef struct {
unsigned char namesz[4]; 
unsigned char descsz[4]; 
unsigned char type[4]; 
char name[1]; 
} Elf_External_Note;
typedef struct {
unsigned char r_offset[4]; 
unsigned char r_info[4]; 
} Elf32_External_Rel;
typedef struct {
unsigned char r_offset[4]; 
unsigned char r_info[4]; 
unsigned char r_addend[4]; 
} Elf32_External_Rela;
typedef struct {
unsigned char r_offset[8]; 
unsigned char r_info[8]; 
} Elf64_External_Rel;
typedef struct {
unsigned char r_offset[8]; 
unsigned char r_info[8]; 
unsigned char r_addend[8]; 
} Elf64_External_Rela;
typedef struct {
unsigned char d_tag[4]; 
union {
unsigned char d_val[4];
unsigned char d_ptr[4];
} d_un;
} Elf32_External_Dyn;
typedef struct {
unsigned char d_tag[8]; 
union {
unsigned char d_val[8];
unsigned char d_ptr[8];
} d_un;
} Elf64_External_Dyn;
typedef struct {
unsigned char vd_version[2];
unsigned char vd_flags[2];
unsigned char vd_ndx[2];
unsigned char vd_cnt[2];
unsigned char vd_hash[4];
unsigned char vd_aux[4];
unsigned char vd_next[4];
} Elf_External_Verdef;
typedef struct {
unsigned char vda_name[4];
unsigned char vda_next[4];
} Elf_External_Verdaux;
typedef struct {
unsigned char vn_version[2];
unsigned char vn_cnt[2];
unsigned char vn_file[4];
unsigned char vn_aux[4];
unsigned char vn_next[4];
} Elf_External_Verneed;
typedef struct {
unsigned char vna_hash[4];
unsigned char vna_flags[2];
unsigned char vna_other[2];
unsigned char vna_name[4];
unsigned char vna_next[4];
} Elf_External_Vernaux;
typedef struct {
unsigned char vs_vers[2];
} ATTRIBUTE_PACKED Elf_External_Versym;
typedef struct
{
unsigned char si_boundto[2];
unsigned char si_flags[2];
} Elf_External_Syminfo;
typedef struct
{
unsigned char a_type[4];
unsigned char a_val[4];
} Elf32_External_Auxv;
typedef struct
{
unsigned char a_type[8];
unsigned char a_val[8];
} Elf64_External_Auxv;
#define GRP_ENTRY_SIZE 4
