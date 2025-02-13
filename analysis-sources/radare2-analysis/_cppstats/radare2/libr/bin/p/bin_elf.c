#include "bin_elf.inc"
static void headers32(RBinFile *bf) {
#define p bf->rbin->cb_printf
p ("0x00000000 ELF MAGIC 0x%08x\n", r_buf_read_le32_at (bf->buf, 0));
p ("0x00000010 Type 0x%04x\n", r_buf_read_le16_at (bf->buf, 0x10));
p ("0x00000012 Machine 0x%04x\n", r_buf_read_le16_at (bf->buf, 0x12));
p ("0x00000014 Version 0x%08x\n", r_buf_read_le32_at (bf->buf, 0x14));
p ("0x00000018 Entrypoint 0x%08x\n", r_buf_read_le32_at (bf->buf, 0x18));
p ("0x0000001c PhOff 0x%08x\n", r_buf_read_le32_at (bf->buf, 0x1c));
p ("0x00000020 ShOff 0x%08x\n", r_buf_read_le32_at (bf->buf, 0x20));
}
static bool check_buffer(RBuffer *buf) {
ut8 b[5] = {0};
r_buf_read_at (buf, 0, b, sizeof (b));
return !memcmp (b, ELFMAG, SELFMAG) && b[4] != 2;
}
extern struct r_bin_dbginfo_t r_bin_dbginfo_elf;
extern struct r_bin_write_t r_bin_write_elf;
static RBuffer* create(RBin* bin, const ut8 *code, int codelen, const ut8 *data, int datalen, RBinArchOptions *opt) {
r_return_val_if_fail (bin && opt && opt->arch, NULL);
ut32 filesize, code_va, code_pa, phoff;
ut32 p_start, p_phoff, p_phdr;
ut32 p_ehdrsz, p_phdrsz;
ut16 ehdrsz, phdrsz;
ut32 p_vaddr, p_paddr, p_fs, p_fs2;
ut32 baddr;
RBuffer *buf = r_buf_new ();
bool is_arm = !strcmp (opt->arch, "arm");
if (is_arm) {
baddr = 0x40000;
} else {
baddr = 0x8048000;
}
#define B(x,y) r_buf_append_bytes(buf,(const ut8*)(x),y)
#define D(x) r_buf_append_ut32(buf,x)
#define H(x) r_buf_append_ut16(buf,x)
#define Z(x) r_buf_append_nbytes(buf,x)
#define W(x,y,z) r_buf_write_at(buf,x,(const ut8*)(y),z)
#define WZ(x,y) p_tmp=r_buf_size (buf);Z(x);W(p_tmp,y,strlen(y))
B ("\x7F" "ELF" "\x01\x01\x01\x00", 8);
Z (8);
H (2); 
if (is_arm) {
H (40); 
} else {
H (3); 
}
D (1);
p_start = r_buf_size (buf);
D (-1); 
p_phoff = r_buf_size (buf);
D (-1); 
D (0); 
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
D (0);
p_vaddr = r_buf_size (buf);
D (-1); 
p_paddr = r_buf_size (buf);
D (-1); 
p_fs = r_buf_size (buf);
D (-1); 
p_fs2 = r_buf_size (buf);
D (-1); 
D (5); 
D (0x1000); 
ehdrsz = p_phdr;
phdrsz = r_buf_size (buf) - p_phdr;
code_pa = r_buf_size (buf);
code_va = code_pa + baddr;
phoff = 0x34;
filesize = code_pa + codelen + datalen;
W (p_start, &code_va, 4);
W (p_phoff, &phoff, 4);
W (p_ehdrsz, &ehdrsz, 2);
W (p_phdrsz, &phdrsz, 2);
code_va = baddr; 
W (p_vaddr, &code_va, 4);
code_pa = baddr; 
W (p_paddr, &code_pa, 4);
W (p_fs, &filesize, 4);
W (p_fs2, &filesize, 4);
B (code, codelen);
if (data && datalen > 0) {
eprintf ("Warning: DATA section not support for ELF yet\n");
B (data, datalen);
}
return buf;
}
RBinPlugin r_bin_plugin_elf = {
.name = "elf",
.desc = "ELF format r2 plugin",
.license = "LGPL3",
.get_sdb = &get_sdb,
.load_buffer = &load_buffer,
.destroy = &destroy,
.check_buffer = &check_buffer,
.baddr = &baddr,
.boffset = &boffset,
.binsym = &binsym,
.entries = &entries,
.sections = &sections,
.symbols = &symbols,
.minstrlen = 4,
.imports = &imports,
.info = &info,
.fields = &fields,
.header = &headers32,
.size = &size,
.libs = &libs,
.relocs = &relocs,
.patch_relocs = &patch_relocs,
.dbginfo = &r_bin_dbginfo_elf,
.create = &create,
.write = &r_bin_write_elf,
.file_type = &get_file_type,
.regstate = &regstate,
.maps = &maps,
};
#if !defined(R2_PLUGIN_INCORE)
R_API RLibStruct radare_plugin = {
.type = R_LIB_TYPE_BIN,
.data = &r_bin_plugin_elf,
.version = R2_VERSION
};
#endif
