#include <string.h>
#include <r_types.h>
#include <r_lib.h>
#include <r_asm.h>
#include <r_anal.h>
static const char* mips_reg_decode(unsigned reg_num) {
static const char *REGISTERS[32] = {
"zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
"t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
"t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"
};
if (reg_num < 32) {
return REGISTERS[reg_num];
}
return NULL;
}
static int mips_op(RAnal *anal, RAnalOp *op, ut64 addr, const ut8 *b, int len, RAnalOpMask mask) {
ut32 opcode;
int optype, oplen = (anal->bits==16)?2:4;
if (!op) {
return oplen;
}
memset (op, 0, sizeof (RAnalOp));
op->type = R_ANAL_OP_TYPE_UNK;
op->size = oplen;
op->delay = 0;
op->addr = addr;
r_strbuf_init (&op->esil);
opcode = r_read_ble32 (b, anal->big_endian);
if (opcode == 0) {
op->type = R_ANAL_OP_TYPE_NOP;
return oplen;
}
optype = (b[0]>>2);
if (optype == 0) {
#if WIP
int rs = ((b[0]&3)<<3) + (b[1]>>5);
int rt = b[1]&31;
int rd = b[2]>>3;
int sa = (b[2]&7)+(b[3]>>6);
#endif
int fun = b[3]&63;
switch (fun) {
case 0: 
break;
case 2: 
break;
case 3: 
break;
case 4: 
break;
case 6: 
break;
case 7: 
break;
case 8: 
if (((b[0]&3)<<3) + (b[1]>>5) == 31) {
op->type = R_ANAL_OP_TYPE_RET;
} else {
op->type = R_ANAL_OP_TYPE_JMP;
}
op->delay = 1;
break;
case 9: 
op->type = R_ANAL_OP_TYPE_UCALL;
op->delay = 1;
break;
case 12: 
op->type = R_ANAL_OP_TYPE_SWI;
break;
case 13: 
op->type = R_ANAL_OP_TYPE_TRAP;
break;
case 16: 
case 18: 
case 17: 
case 19: 
case 24: 
case 25: 
case 26: 
case 27: 
op->type = R_ANAL_OP_TYPE_DIV;
break;
case 32: 
case 33: 
op->type = R_ANAL_OP_TYPE_ADD;
break;
case 34: 
case 35: 
op->type = R_ANAL_OP_TYPE_SUB;
break;
case 36: 
op->type = R_ANAL_OP_TYPE_AND;
break;
case 37: 
op->type = R_ANAL_OP_TYPE_OR;
break;
case 38: 
op->type = R_ANAL_OP_TYPE_XOR;
break;
case 39: 
case 42: 
case 43: 
break;
default:
break;
}
} else
if ((optype & 0x3e) == 2) {
int address = (((b[0]&3)<<24)+(b[1]<<16)+(b[2]<<8)+b[3]) << 2;
ut64 page_hack = addr & 0xf0000000;
switch (optype) {
case 2: 
op->type = R_ANAL_OP_TYPE_JMP;
op->jump = page_hack + address;
op->delay = 1;
r_strbuf_setf (&op->esil, "0x%08x,pc,=", address);
break;
case 3: 
op->type = R_ANAL_OP_TYPE_CALL;
op->jump = page_hack + address;
op->fail = addr+8;
op->delay = 1;
r_strbuf_setf (&op->esil, "pc,lr,=,0x%08x,pc,=", (ut32)address);
break;
}
} else if ((optype & 0x1c) == 0x1c) {
#if WIP
int fmt = ((b[0]&3)<<3) + (b[1]>>5);
int ft = (b[1]&31);
int fs = (b[2]>>3);
int fd = (b[2]&7)+(b[3]>>6);
#endif
int fun = (b[3]&63);
switch (fun) {
case 0: 
break;
case 1: 
break;
case 2: 
break;
case 3: 
break;
}
} else {
int rs = ((b[0]&3)<<3)+(b[1]>>5);
int rt = b[1]&31;
int imm = (b[2]<<8)+b[3];
if (((optype >> 2) ^ 0x3) && (imm & 0x8000)) {
imm = 0 - (0x10000 - imm);
}
switch (optype) {
case 1: 
case 4: 
case 5: 
case 6: 
case 7: 
op->type = R_ANAL_OP_TYPE_CJMP;
op->jump = addr+(imm<<2)+4;
op->fail = addr+8;
op->delay = 1;
break;
case 15: 
op->dst = r_anal_value_new ();
op->dst->reg = r_reg_get (anal->reg, mips_reg_decode(rt), R_REG_TYPE_GPR);
op->val = imm;
break;
case 9: 
op->dst = r_anal_value_new ();
op->dst->reg = r_reg_get (anal->reg, mips_reg_decode(rt), R_REG_TYPE_GPR);
op->src[0] = r_anal_value_new ();
op->src[0]->reg = r_reg_get (anal->reg, mips_reg_decode(rs), R_REG_TYPE_GPR);
op->val = imm; 
break;
case 8: 
case 10: 
case 11: 
case 12: 
case 13: 
case 14: 
case 32: 
case 33: 
case 35: 
case 36: 
case 37: 
case 40: 
case 41: 
case 43: 
case 49: 
case 57: 
break;
case 29: 
op->type = R_ANAL_OP_TYPE_CALL;
op->jump = addr + 4*((b[3] | b[2]<<8 | b[1]<<16));
op->fail = addr + 8;
op->delay = 1;
break;
}
}
#if 0
switch (optype) {
case 'R': 
op->type = R_ANAL_OP_TYPE_ADD;
break;
case 'I': 
op->type = R_ANAL_OP_TYPE_JMP;
break;
case 'J': 
op->type = R_ANAL_OP_TYPE_CALL;
break;
case 'C': 
op->type = R_ANAL_OP_TYPE_RET;
break;
}
#endif
return oplen;
return op->size;
}
static int mips_set_reg_profile(RAnal* anal){
const char *p =
#if 0
"=PC pc\n"
"=SP sp\n"
"=A0 a0\n"
"=A1 a1\n"
"=A2 a2\n"
"=A3 a3\n"
"gpr zero .32 0 0\n"
"gpr at .32 4 0\n"
"gpr v0 .32 8 0\n"
"gpr v1 .32 12 0\n"
"gpr a0 .32 16 0\n"
"gpr a1 .32 20 0\n"
"gpr a2 .32 24 0\n"
"gpr a3 .32 28 0\n"
"gpr t0 .32 32 0\n"
"gpr t1 .32 36 0\n"
"gpr t2 .32 40 0\n"
"gpr t3 .32 44 0\n"
"gpr t4 .32 48 0\n"
"gpr t5 .32 52 0\n"
"gpr t6 .32 56 0\n"
"gpr t7 .32 60 0\n"
"gpr s0 .32 64 0\n"
"gpr s1 .32 68 0\n"
"gpr s2 .32 72 0\n"
"gpr s3 .32 76 0\n"
"gpr s4 .32 80 0\n"
"gpr s5 .32 84 0\n"
"gpr s6 .32 88 0\n"
"gpr s7 .32 92 0\n"
"gpr t8 .32 96 0\n"
"gpr t9 .32 100 0\n"
"gpr k0 .32 104 0\n"
"gpr k1 .32 108 0\n"
"gpr gp .32 112 0\n"
"gpr sp .32 116 0\n"
"gpr fp .32 120 0\n"
"gpr ra .32 124 0\n"
"gpr pc .32 128 0\n";
#else
"=PC pc\n"
"=SP sp\n"
"=BP fp\n"
"=A0 a0\n"
"=A1 a1\n"
"=A2 a2\n"
"=A3 a3\n"
"gpr zero .64 0 0\n"
"gpr at .64 8 0\n"
"gpr v0 .64 16 0\n"
"gpr v1 .64 24 0\n"
"gpr a0 .64 32 0\n"
"gpr a1 .64 40 0\n"
"gpr a2 .64 48 0\n"
"gpr a3 .64 56 0\n"
"gpr t0 .64 64 0\n"
"gpr t1 .64 72 0\n"
"gpr t2 .64 80 0\n"
"gpr t3 .64 88 0\n"
"gpr t4 .64 96 0\n"
"gpr t5 .64 104 0\n"
"gpr t6 .64 112 0\n"
"gpr t7 .64 120 0\n"
"gpr s0 .64 128 0\n"
"gpr s1 .64 136 0\n"
"gpr s2 .64 144 0\n"
"gpr s3 .64 152 0\n"
"gpr s4 .64 160 0\n"
"gpr s5 .64 168 0\n"
"gpr s6 .64 176 0\n"
"gpr s7 .64 184 0\n"
"gpr t8 .64 192 0\n"
"gpr t9 .64 200 0\n"
"gpr k0 .64 208 0\n"
"gpr k1 .64 216 0\n"
"gpr gp .64 224 0\n"
"gpr sp .64 232 0\n"
"gpr fp .64 240 0\n"
"gpr ra .64 248 0\n"
"gpr pc .64 272 0\n"
;
#endif
return r_reg_set_profile_string (anal->reg, p);
}
static int archinfo(RAnal *anal, int q) {
return 4;
}
RAnalPlugin r_anal_plugin_mips_gnu = {
.name = "mips.gnu",
.desc = "MIPS code analysis plugin",
.license = "LGPL3",
.arch = "mips",
.bits = 32,
.esil = true,
.archinfo = archinfo,
.op = &mips_op,
.set_reg_profile = mips_set_reg_profile,
};
#if !defined(R2_PLUGIN_INCORE)
R_API RLibStruct radare_plugin = {
.type = R_LIB_TYPE_ANAL,
.data = &r_anal_plugin_mips_gnu
};
#endif
