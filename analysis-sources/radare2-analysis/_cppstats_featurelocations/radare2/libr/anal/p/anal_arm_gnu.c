

#include <string.h>
#include <r_types.h>
#include <r_lib.h>
#include <r_asm.h>
#include <r_anal.h>


#include "wine-arm.h"
#include "../asm/arch/arm/asm-arm.h"
#include "../asm/arch/arm/winedbg/be_arm.h"
#include "./anal_arm_hacks.inc"

static unsigned int disarm_branch_offset(unsigned int pc, unsigned int insoff) {
unsigned int add = insoff << 2;

if ((add & 0x02000000) == 0x02000000) {
add |= 0xFC000000;
}
return add + pc + 8;
}

#define IS_BRANCH(x) (((x) & ARM_BRANCH_I_MASK) == ARM_BRANCH_I)
#define IS_BRANCHL(x) (IS_BRANCH (x) && ((x) & ARM_BRANCH_LINK) == ARM_BRANCH_LINK)
#define IS_RETURN(x) (((x) & (ARM_DTM_I_MASK | ARM_DTM_LOAD | (1 << 15))) == (ARM_DTM_I | ARM_DTM_LOAD | (1 << 15)))

#define IS_UNKJMP(x) ((((ARM_DTX_RD_MASK))) == (ARM_DTX_LOAD | ARM_DTX_I | (ARM_PC << 12)))
#define IS_LOAD(x) (((x) & ARM_DTX_LOAD) == (ARM_DTX_LOAD))
#define IS_CONDAL(x) (((x) & ARM_COND_MASK) == ARM_COND_AL)
#define IS_EXITPOINT(x) (IS_BRANCH (x) || IS_RETURN (x) || IS_UNKJMP (x))

#define API static

static int op_thumb(RAnal *anal, RAnalOp *op, ut64 addr, const ut8 *data, int len) {
int op_code;
ut16 *_ins = (ut16 *) data;
ut16 ins = *_ins;
ut32 *_ins32 = (ut32 *) data;
ut32 ins32 = *_ins32;

struct winedbg_arm_insn *arminsn = arm_new ();
arm_set_thumb (arminsn, true);
arm_set_input_buffer (arminsn, data);
arm_set_pc (arminsn, addr);
op->jump = op->fail = -1;
op->ptr = op->val = -1;
op->delay = 0;
op->size = arm_disasm_one_insn (arminsn);
op->jump = arminsn->jmp;
op->fail = arminsn->fail;
arm_free (arminsn);




if (((ins & B4 (B1110, 0, 0, 0)) == B4 (B0010, 0, 0, 0))
&& (1 == (ins & B4 (1, B1000, 0, 0)) >> 11)) { 
op->type = R_ANAL_OP_TYPE_CMP;
return op->size;
}
if ((ins & B4 (B1111, B1100, 0, 0)) == B4 (B0100, 0, 0, 0)) {
op_code = (ins & B4 (0, B0011, B1100, 0)) >> 6;
if (op_code == 8 || op_code == 10) { 
op->type = R_ANAL_OP_TYPE_CMP;
return op->size;
}
}
if ((ins & B4 (B1111, B1100, 0, 0)) == B4 (B0100, B0100, 0, 0)) {
op_code = (ins & B4 (0, B0011, 0, 0)) >> 8; 
if (op_code == 1) {
op->type = R_ANAL_OP_TYPE_CMP;
return op->size;
}
}
if (ins == 0xbf) {

op->type = R_ANAL_OP_TYPE_NOP;
} else if (((op_code = ((ins & B4 (B1111, B1000, 0, 0)) >> 11)) >= 12 &&
op_code <= 17)) {
if (op_code % 2) {
op->type = R_ANAL_OP_TYPE_LOAD;
} else {
op->type = R_ANAL_OP_TYPE_STORE;
}
} else if ((ins & B4 (B1111, 0, 0, 0)) == B4 (B0101, 0, 0, 0)) {
op_code = (ins & B4 (0, B1110, 0, 0)) >> 9;
if (op_code % 2) {
op->type = R_ANAL_OP_TYPE_LOAD;
} else {
op->type = R_ANAL_OP_TYPE_STORE;
}
} else if ((ins & B4 (B1111, 0, 0, 0)) == B4 (B1101, 0, 0, 0)) {

int delta = (ins & B4 (0, 0, B1111, B1111));
op->type = R_ANAL_OP_TYPE_CJMP;
op->jump = addr + 4 + (delta << 1);
op->fail = addr + 4;
} else if ((ins & B4 (B1111, B1000, 0, 0)) == B4 (B1110, 0, 0, 0)) {

int delta = (ins & B4 (0, 0, B1111, B1111));
op->type = R_ANAL_OP_TYPE_JMP;
op->jump = addr + 4 + (delta << 1);
op->fail = addr + 4;
} else if ((ins & B4 (B1111, B1111, B1000, 0)) ==
B4 (B0100, B0111, B1000, 0)) {

op->type = R_ANAL_OP_TYPE_UCALL;
op->fail = addr + 4;
} else if ((ins & B4 (B1111, B1111, B1000, 0)) ==
B4 (B0100, B0111, 0, 0)) {

op->type = R_ANAL_OP_TYPE_UJMP;
op->fail = addr + 4;
} else if ((ins & B4 (B1111, B1000, 0, 0)) == B4 (B1111, 0, 0, 0)) {



ut16 nextins = (ins32 & 0xFFFF0000) >> 16;
ut32 high = (ins & B4 (0, B0111, B1111, B1111)) << 12;
if (ins & B4 (0, B0100, 0, 0)) {
high |= B4 (B1111, B1000, 0, 0) << 16;
}
int delta = high + ((nextins & B4 (0, B0111, B1111, B1111)) * 2);
op->jump = (int) (addr + 4 + (delta));
op->type = R_ANAL_OP_TYPE_CALL;
op->fail = addr + 4;
} else if ((ins & B4 (B1111, B1111, 0, 0)) == B4 (B1011, B1110, 0, 0)) {
op->type = R_ANAL_OP_TYPE_TRAP;
op->val = (ut64) (ins >> 8);
} else if ((ins & B4 (B1111, B1111, 0, 0)) == B4 (B1101, B1111, 0, 0)) {
op->type = R_ANAL_OP_TYPE_SWI;
op->val = (ut64) (ins >> 8);
}
return op->size;
}

#if 0
"eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc",
"hi", "ls", "ge", "lt", "gt", "le", "al", "nv",
#endif
static int iconds[] = {
R_ANAL_COND_EQ,
R_ANAL_COND_NE,
0, 
0, 
0, 
0, 
0, 
0, 

0, 
0, 
R_ANAL_COND_GE,
R_ANAL_COND_LT,
R_ANAL_COND_GT,
R_ANAL_COND_LE,
R_ANAL_COND_AL,
R_ANAL_COND_NV,
};

static int op_cond(const ut8 *data) {
ut8 b = data[3] >> 4;
if (b == 0xf) {
return 0;
}
return iconds[b];
}

static int arm_op32(RAnal *anal, RAnalOp *op, ut64 addr, const ut8 *data, int len) {
const ut8 *b = (ut8 *) data;
ut8 ndata[4];
ut32 branch_dst_addr, i = 0;
ut32 *code = (ut32 *) data;
struct winedbg_arm_insn *arminsn;

if (!data) {
return 0;
}
memset (op, '\0', sizeof (RAnalOp));
arminsn = arm_new ();
arm_set_thumb (arminsn, false);
arm_set_input_buffer (arminsn, data);
arm_set_pc (arminsn, addr);
op->jump = op->fail = -1;
op->ptr = op->val = -1;
op->addr = addr;
op->type = R_ANAL_OP_TYPE_UNK;

if (anal->big_endian) {
b = data = ndata;
ndata[0] = data[3];
ndata[1] = data[2];
ndata[2] = data[1];
ndata[3] = data[0];
}
if (anal->bits == 16) {
arm_free (arminsn);
return op_thumb (anal, op, addr, data, len);
}
op->size = 4;
op->cond = op_cond (data);
if (b[2] == 0x8f && b[3] == 0xe2) {
op->type = R_ANAL_OP_TYPE_ADD;
#define ROR(x, y) ((int) ((x) >> (y)) | (((x) << (32 - (y)))))
op->ptr = addr + ROR (b[0], (b[1] & 0xf) << 1) + 8;
} else if (b[2] >= 0x9c && b[2] <= 0x9f) { 
char ch = b[3] & 0xf;
switch (ch) {
case 5:
if ((b[3] & 0xf) == 5) {
op->ptr = 8 + addr + b[0] + ((b[1] & 0xf) << 8);


}
case 4:
case 6:
case 7:
case 8:
case 9: op->type = R_ANAL_OP_TYPE_LOAD; break;
}
} else 
if (b[2] == 0xa0 && b[3] == 0xe1) {
int n = (b[0] << 16) + b[1];
op->type = R_ANAL_OP_TYPE_MOV;
switch (n) {
case 0:
case 0x0110: case 0x0220: case 0x0330: case 0x0440:
case 0x0550: case 0x0660: case 0x0770: case 0x0880:
case 0x0990: case 0x0aa0: case 0x0bb0: case 0x0cc0:
op->type = R_ANAL_OP_TYPE_NOP;
break;
}
} else if (b[3] == 0xef) {
op->type = R_ANAL_OP_TYPE_SWI;
op->val = (b[0] | (b[1] << 8) | (b[2] << 2));
} else if ((b[3] & 0xf) == 5) { 
#if 0
0x00000000 a4a09fa4 ldrge sl, [pc], 0xa4
0x00000000 a4a09fa5 ldrge sl, [pc, 0xa4]
0x00000000 a4a09fa6 ldrge sl, [pc], r4, lsr 1
0x00000000 a4a09fa7 ldrge sl, [pc, r4, lsr 1]
0x00000000 a4a09fe8 ldm pc, {
r2, r5, r7, sp, pc
}; < UNPREDICT
#endif
if ((b[1] & 0xf0) == 0xf0) {


op->type = R_ANAL_OP_TYPE_RET; 

op->jump = 1234;


}


} else if (b[3] == 0xe2 && b[2] == 0x8d && b[1] == 0xd0) {

op->type = R_ANAL_OP_TYPE_ADD;
op->stackop = R_ANAL_STACK_INC;
op->val = -b[0];
} else if (b[3] == 0xe2 && b[2] == 0x4d && b[1] == 0xd0) {

op->type = R_ANAL_OP_TYPE_SUB;
op->stackop = R_ANAL_STACK_INC;
op->val = b[0];
} else if (b[3] == 0xe2 && b[2] == 0x4c && b[1] == 0xb0) {

op->type = R_ANAL_OP_TYPE_SUB;
op->stackop = R_ANAL_STACK_INC;
op->val = -b[0];
} else if (b[3] == 0xe2 && b[2] == 0x4b && b[1] == 0xd0) {

op->type = R_ANAL_OP_TYPE_SUB;
op->stackop = R_ANAL_STACK_INC;
op->val = -b[0];
} else if ((code[i] == 0x1eff2fe1) ||
(code[i] == 0xe12fff1e)) { 
op->type = R_ANAL_OP_TYPE_RET;
} else if ((code[i] & ARM_DTX_LOAD)) { 
ut32 ptr = 0;
op->type = R_ANAL_OP_TYPE_MOV;
if (b[2] == 0x1b) {

op->stackop = R_ANAL_STACK_GET;
op->ptr = b[0];

} else {


if (anal->bits == 32) {
b = (ut8 *) &ptr;
op->ptr = b[0] + (b[1] << 8) + (b[2] << 16) + (b[3] << 24);


} else {
op->ptr = 0;
}
}
}

if (IS_LOAD (code[i])) {
op->type = R_ANAL_OP_TYPE_LOAD;
op->refptr = 4;
}
if (((((code[i] & 0xff) >= 0x10 && (code[i] & 0xff) < 0x20)) &&
((code[i] & 0xffffff00) == 0xe12fff00)) ||
IS_EXITPOINT (code[i])) {

b = data;
branch_dst_addr = disarm_branch_offset (
addr, b[0] | (b[1] << 8) |
(b[2] << 16)); 
op->ptr = 0;
if ((((code[i] & 0xff) >= 0x10 && (code[i] & 0xff) < 0x20)) &&
((code[i] & 0xffffff00) == 0xe12fff00)) {
op->type = R_ANAL_OP_TYPE_UJMP;
} else if (IS_BRANCHL (code[i])) {
if (IS_BRANCH (code[i])) {
op->type = R_ANAL_OP_TYPE_CALL;
op->jump = branch_dst_addr;
op->fail = addr + 4;
} else {
op->type = R_ANAL_OP_TYPE_RET;
}
} else if (IS_BRANCH (code[i])) {
if (IS_CONDAL (code[i])) {
op->type = R_ANAL_OP_TYPE_JMP;
op->jump = branch_dst_addr;
op->fail = UT64_MAX;
} else {
op->type = R_ANAL_OP_TYPE_CJMP;
op->jump = branch_dst_addr;
op->fail = addr + 4;
}
} else {



}
}


arm_free (arminsn);
return op->size;
}


static ut64 getaddr(ut64 addr, const ut8 *d) {
if (d[2] >> 7) {

st32 n = (d[0] + (d[1] << 8) + (d[2] << 16) + ((ut64)(0xff) << 24)); 
n = -n;
return addr - (n * 4);
}
return addr + (4 * (d[0] + (d[1] << 8) + (d[2] << 16)));
}

static int arm_op64(RAnal *anal, RAnalOp *op, ut64 addr, const ut8 *d, int len) {
memset (op, 0, sizeof (RAnalOp));
if (d[3] == 0) {
return -1; 
}
int haa = hackyArmAnal (anal, op, d, len);
if (haa > 0) {
return haa;
}
op->size = 4;
op->type = R_ANAL_OP_TYPE_NULL;
if (d[0] == 0xc0 && d[3] == 0xd6) {

op->type = R_ANAL_OP_TYPE_RET;
}
switch (d[3]) {
case 0x71:
case 0xeb:
op->type = R_ANAL_OP_TYPE_CMP;
break;
case 0xb8:
case 0xb9:
case 0xf8:
case 0xa9: 
case 0xf9: 
op->type = R_ANAL_OP_TYPE_LOAD;
break;
case 0x91: 
case 0x52: 
case 0x94: 
case 0x97: 
op->type = R_ANAL_OP_TYPE_CALL;
op->jump = getaddr (addr, d);
op->fail = addr + 4;
break;
case 0x54: 
op->type = R_ANAL_OP_TYPE_CJMP;
op->jump = addr + (4 * ((d[0] >> 4) | (d[1] << 8) | (d[2] << 16)));
op->fail = addr + 4;
break;
case 0x17: 
case 0x14: 
op->type = R_ANAL_OP_TYPE_JMP;
op->jump = getaddr (addr, d);
op->fail = addr + 4;
break;
}
return op->size;
}

static int arm_op(RAnal *anal, RAnalOp *op, ut64 addr, const ut8 *data, int len, RAnalOpMask mask) {
if (anal->bits == 64) {
return arm_op64 (anal, op, addr, data, len);
}
return arm_op32 (anal, op, addr, data, len);
}

static int set_reg_profile(RAnal *anal) {

const char *p32 =
"=PC r15\n"
"=SP r13\n"
"=BP r14\n" 
"=A0 r0\n"
"=A1 r1\n"
"=A2 r2\n"
"=A3 r3\n"
"gpr lr .32 56 0\n" 
"gpr pc .32 60 0\n" 

"gpr r0 .32 0 0\n"
"gpr r1 .32 4 0\n"
"gpr r2 .32 8 0\n"
"gpr r3 .32 12 0\n"
"gpr r4 .32 16 0\n"
"gpr r5 .32 20 0\n"
"gpr r6 .32 24 0\n"
"gpr r7 .32 28 0\n"
"gpr r8 .32 32 0\n"
"gpr r9 .32 36 0\n"
"gpr r10 .32 40 0\n"
"gpr r11 .32 44 0\n"
"gpr r12 .32 48 0\n"
"gpr r13 .32 52 0\n"
"gpr r14 .32 56 0\n"
"gpr r15 .32 60 0\n"
"gpr r16 .32 64 0\n"
"gpr r17 .32 68 0\n"
"gpr cpsr .32 72 0\n";
return r_reg_set_profile_string (anal->reg, p32);
}

static int archinfo(RAnal *anal, int q) {
if (q == R_ANAL_ARCHINFO_ALIGN) {
if (anal && anal->bits == 16) {
return 2;
}
return 4;
}
if (q == R_ANAL_ARCHINFO_MAX_OP_SIZE) {
return 4;
}
if (q == R_ANAL_ARCHINFO_MIN_OP_SIZE) {
if (anal && anal->bits == 16) {
return 2;
}
return 4;
}
return 4; 
}

RAnalPlugin r_anal_plugin_arm_gnu = {
.name = "arm.gnu",
.arch = "arm",
.license = "LGPL3",
.bits = 16 | 32 | 64,
.desc = "ARM code analysis plugin",
.archinfo = archinfo,
.op = &arm_op,
.set_reg_profile = set_reg_profile,
};

#if !defined(R2_PLUGIN_INCORE)
R_API RLibStruct radare_plugin = {
.type = R_LIB_TYPE_ANAL,
.data = &r_anal_plugin_arm_gnu,
.version = R2_VERSION
};
#endif
