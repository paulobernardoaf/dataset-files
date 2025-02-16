

#include <r_asm.h>
#include <r_lib.h>
#include <capstone/capstone.h>
static csh cd = 0;
#include "cs_mnemonics.c"

static int disassemble(RAsm *a, RAsmOp *op, const ut8 *buf, int len) {
cs_insn* insn;
int n = -1, ret = -1;
int mode = CS_MODE_BIG_ENDIAN;
if (a->cpu && *a->cpu) {
if (!strcmp (a->cpu, "v9")) {
mode |= CS_MODE_V9;
}
}
if (op) {
memset (op, 0, sizeof (RAsmOp));
op->size = 4;
}
if (cd != 0) {
cs_close (&cd);
}
ret = cs_open (CS_ARCH_SPARC, mode, &cd);
if (ret) {
goto fin;
}
cs_option (cd, CS_OPT_DETAIL, CS_OPT_OFF);
if (!op) {
return 0;
}
if (a->big_endian) {
n = cs_disasm (cd, buf, len, a->pc, 1, &insn);
}
if (n < 1) {
r_asm_op_set_asm (op, "invalid");
op->size = 4;
ret = -1;
goto beach;
} else {
ret = 4;
}
if (insn->size < 1) {
goto beach;
}
op->size = insn->size;
char *buf_asm = sdb_fmt ("%s%s%s",
insn->mnemonic, insn->op_str[0]? " ": "",
insn->op_str);
r_str_replace_char (buf_asm, '%', 0);
r_asm_op_set_asm (op, buf_asm);

cs_free (insn, n);
beach:

fin:
return ret;
}

RAsmPlugin r_asm_plugin_sparc_cs = {
.name = "sparc",
.desc = "Capstone SPARC disassembler",
.license = "BSD",
.arch = "sparc",
.cpus = "v9",
.bits = 32|64,
.endian = R_SYS_ENDIAN_BIG | R_SYS_ENDIAN_LITTLE,
.disassemble = &disassemble,
.mnemonics = mnemonics
};

#if !defined(R2_PLUGIN_INCORE)
R_API RLibStruct radare_plugin = {
.type = R_LIB_TYPE_ASM,
.data = &r_asm_plugin_sparc_cs,
.version = R2_VERSION
};
#endif
