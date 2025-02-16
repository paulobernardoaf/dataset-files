

#define R_BIN_ELF64 1
#include "bin_elf.inc"


static bool check_buffer(RBuffer *b) {
ut8 buf[5] = {0};
if (r_buf_size (b) > 4) {
r_buf_read_at (b, 0, buf, sizeof (buf));
if (!memcmp (buf, "\x7F\x45\x4c\x46\x02", 5)) {
return true;
}
}
return false;
}

extern struct r_bin_dbginfo_t r_bin_dbginfo_elf64;
extern struct r_bin_write_t r_bin_write_elf64;

static ut64 get_elf_vaddr64 (RBinFile *bf, ut64 baddr, ut64 paddr, ut64 vaddr) {

struct Elf_(r_bin_elf_obj_t)* obj = bf->o->bin_obj;
return obj->baddr - obj->boffset + vaddr;
}

static void headers64(RBinFile *bf) {
#define p bf->rbin->cb_printf
p ("0x00000000 ELF64 0x%08x\n", r_buf_read_le32_at (bf->buf, 0));
p ("0x00000010 Type 0x%04x\n", r_buf_read_le16_at (bf->buf, 0x10));
p ("0x00000012 Machine 0x%04x\n", r_buf_read_le16_at (bf->buf, 0x12));
p ("0x00000014 Version 0x%08x\n", r_buf_read_le32_at (bf->buf, 0x14));
p ("0x00000018 Entrypoint 0x%08"PFMT64x"\n", r_buf_read_le64_at (bf->buf, 0x18));
p ("0x00000020 PhOff 0x%08"PFMT64x"\n", r_buf_read_le64_at (bf->buf, 0x20));
p ("0x00000028 ShOff 0x%08"PFMT64x"\n", r_buf_read_le64_at (bf->buf, 0x28));
}

static RBuffer* create(RBin* bin, const ut8 *code, int codelen, const ut8 *data, int datalen, RBinArchOptions *opt) {
ut32 p_start, p_phoff, p_phdr;
ut32 p_vaddr, p_paddr, p_fs, p_fs2;
ut32 p_ehdrsz, p_phdrsz;
ut64 filesize, code_va, code_pa, phoff;
ut16 ehdrsz, phdrsz;
ut64 baddr = 0x400000LL;
RBuffer *buf = r_buf_new ();

#define B(x,y) r_buf_append_bytes(buf,(const ut8*)(x),y)
#define Q(x) r_buf_append_ut64(buf,x)
#define D(x) r_buf_append_ut32(buf,x)
#define H(x) r_buf_append_ut16(buf,x)
#define Z(x) r_buf_append_nbytes(buf,x)
#define W(x,y,z) r_buf_write_at(buf,x,(const ut8*)(y),z)


B ("\x7F" "ELF" "\x02\x01\x01\x00", 8); 
Z (8);
H (2); 
H (62); 
D (1); 
p_start = r_buf_size (buf);
Q (-1); 
p_phoff = r_buf_size (buf);
Q (-1); 
Q (0); 
D (0); 
p_ehdrsz = r_buf_size (buf);
H (-1); 
p_phdrsz = r_buf_size (buf);
H (-1); 
H (1); 
H (0); 
H (0); 
H (0); 


p_phdr = r_buf_size (buf);
D (1); 
D (5); 
Q (0); 
p_vaddr = r_buf_size (buf);
Q (-1); 
p_paddr = r_buf_size (buf);
Q (-1); 
p_fs = r_buf_size (buf);
Q (-1); 
p_fs2 = r_buf_size (buf);
Q (-1); 
Q (0x200000); 


ehdrsz = p_phdr;
phdrsz = r_buf_size (buf) - p_phdr;
code_pa = r_buf_size (buf);
code_va = code_pa + baddr;
phoff = p_phdr;
filesize = code_pa + codelen + datalen;


W (p_start, &code_va, 8);
W (p_phoff, &phoff, 8);
W (p_ehdrsz, &ehdrsz, 2);
W (p_phdrsz, &phdrsz, 2);
W (p_fs, &filesize, 8);
W (p_fs2, &filesize, 8);

W (p_vaddr, &baddr, 8);
W (p_paddr, &baddr, 8);


B (code, codelen);

if (data && datalen>0) {
eprintf ("Warning: DATA section not support for ELF yet\n");
B (data, datalen);
}
return buf;
}

RBinPlugin r_bin_plugin_elf64 = {
.name = "elf64",
.desc = "elf64 bin plugin",
.license = "LGPL3",
.get_sdb = &get_sdb,
.check_buffer = &check_buffer,
.load_buffer= &load_buffer,
.destroy = &destroy,
.baddr = &baddr,
.boffset = &boffset,
.binsym = &binsym,
.entries = &entries,
.sections = &sections,
.symbols = &symbols,
.imports = &imports,
.minstrlen = 4,
.info = &info,
.fields = &fields,
.header = &headers64,
.size = &size,
.libs = &libs,
.relocs = &relocs,
.patch_relocs = &patch_relocs,
.dbginfo = &r_bin_dbginfo_elf64,
.create = &create,
.write = &r_bin_write_elf64,
.get_vaddr = &get_elf_vaddr64,
.file_type = &get_file_type,
.regstate = &regstate,
.maps = &maps,
};

#if !defined(R2_PLUGIN_INCORE)
R_API RLibStruct radare_plugin = {
.type = R_LIB_TYPE_BIN,
.data = &r_bin_plugin_elf64,
.version = R2_VERSION
};
#endif
