#include <r_types.h>
#include <r_lib.h>
#include <r_asm.h>
#include <r_anal.h>
#include "../../asm/arch/dalvik/opcode.h"
#include "../../bin/format/dex/dex.h"
static const char *getCond(ut8 cond) {
switch (cond) {
case 0x32: 
return "$z";
case 0x33: 
return "$z,!";
case 0x34: 
return "63,$c,!";
case 0x35: 
return "63,$c,$z,|";
case 0x36: 
return "63,$c";
case 0x37: 
return "63,$c,!,$z,|";
}
return "";
}
static const char *getCondz(ut8 cond) {
switch (cond) {
case 0x38: 
return "NOP";
case 0x39: 
return "!";
case 0x3a: 
return "0,==,63,$c,!";
case 0x3b: 
return "0,==,63,$c,$z,|";
case 0x3c: 
return "0,==,63,$c";
case 0x3d: 
return "0,==,63,$c,!";
}
return "";
}
static int dalvik_op(RAnal *anal, RAnalOp *op, ut64 addr, const ut8 *data, int len, RAnalOpMask mask) {
int sz = dalvik_opcodes[data[0]].len;
if (!op || sz >= len) {
if (op && (mask & R_ANAL_OP_MASK_DISASM)) {
op->mnemonic = strdup ("invalid");
}
return -1;
}
op->size = sz;
op->nopcode = 1; 
op->id = data[0];
ut32 vA = 0;
ut32 vB = 0;
ut32 vC = 0;
if (len > 3) {
vA = data[1];
vB = data[2];
vC = data[3];
}
switch (data[0]) {
case 0xca: 
op->family = R_ANAL_OP_FAMILY_FPU;
case 0x1b: 
case 0x14: 
case 0x15: 
case 0x16: 
case 0x17: 
case 0x42: 
case 0x12: 
{
op->type = R_ANAL_OP_TYPE_MOV;
ut32 vB = (data[1] & 0x0f);
ut32 vA = (data[1] & 0xf0) >> 4;
ut32 vC = (len > 4)? r_read_le32 (data + 2): 0x22;
ut32 val = vC?vC:vA;
op->val = val;
op->nopcode = 2;
if (mask & R_ANAL_OP_MASK_ESIL) {
esilprintf (op, "0x%"PFMT64x",v%d,=", val, vB);
}
}
break;
case 0x01: 
case 0x07: 
case 0x04: 
{
ut32 vB = (data[1] & 0x0f);
ut32 vA = (data[1] & 0xf0) >> 4;
if (vA == vB) {
op->type = R_ANAL_OP_TYPE_NOP;
if (mask & R_ANAL_OP_MASK_ESIL) {
esilprintf (op, ",");
}
} else {
op->type = R_ANAL_OP_TYPE_MOV;
if (mask & R_ANAL_OP_MASK_ESIL) {
esilprintf (op, "v%d,v%d,=", vA, vB);
}
}
}
break;
case 0x02: 
case 0x03: 
case 0x05: 
case 0x06: 
case 0x08: 
case 0x09: 
case 0x13: 
op->type = R_ANAL_OP_TYPE_MOV;
if (len > 2) {
int vA = (int) data[1];
ut32 vB = (data[3] << 8) | data[2];
if (mask & R_ANAL_OP_MASK_ESIL) {
esilprintf (op, "v%d,v%d,=", vA, vB);
}
op->val = vB;
}
break;
case 0x18: 
case 0x19: 
op->type = R_ANAL_OP_TYPE_MOV;
break;
case 0x0a: 
case 0x0d: 
case 0x0c: 
case 0x0b: 
op->type = R_ANAL_OP_TYPE_MOV;
if (mask & R_ANAL_OP_MASK_ESIL) {
ut32 vA = data[1];
esilprintf (op, "sp,v%d,=[8],8,sp,+=,8", vA);
}
break;
case 0x1a: 
op->type = R_ANAL_OP_TYPE_MOV;
op->datatype = R_ANAL_DATATYPE_STRING;
if (len > 2) {
ut32 vA = data[1];
ut32 vB = (data[3]<<8) | data[2];
ut64 offset = R_ANAL_GET_OFFSET (anal, 's', vB);
op->ptr = offset;
if (mask & R_ANAL_OP_MASK_ESIL) {
esilprintf (op, "0x%"PFMT64x",v%d,=", offset, vA);
}
}
break;
case 0x1c: 
op->type = R_ANAL_OP_TYPE_MOV;
op->datatype = R_ANAL_DATATYPE_CLASS;
break;
case 0x89: 
case 0x8a: 
case 0x87: 
case 0x8c: 
case 0x8b: 
case 0x88: 
case 0x86: 
op->family = R_ANAL_OP_FAMILY_FPU;
case 0x81: 
case 0x82: 
case 0x85: 
case 0x83: 
case 0x8d: 
case 0x8e: 
op->type = R_ANAL_OP_TYPE_CAST;
if (mask & R_ANAL_OP_MASK_ESIL) {
ut32 vA = (data[1] & 0x0f);
ut32 vB = (data[1] & 0xf0) >> 4;
esilprintf (op, "v%d,0xff,&,v%d,=", vB, vA);
}
break;
case 0x8f: 
op->type = R_ANAL_OP_TYPE_CAST;
if (mask & R_ANAL_OP_MASK_ESIL) {
ut32 vA = (data[1] & 0x0f);
ut32 vB = (data[1] & 0xf0) >> 4;
esilprintf (op, "v%d,0xffff,&,v%d,=", vB, vA);
}
break;
case 0x84: 
op->type = R_ANAL_OP_TYPE_CAST;
if (mask & R_ANAL_OP_MASK_ESIL) {
ut32 vA = (data[1] & 0x0f);
ut32 vB = (data[1] & 0xf0) >> 4;
esilprintf (op, "v%d,0xffffffff,&,v%d,=", vB, vA);
}
break;
case 0x20: 
op->type = R_ANAL_OP_TYPE_CMP;
if (mask & R_ANAL_OP_MASK_ESIL) {
esilprintf (op, "%d,instanceof,%d,-,!,v%d,=", vC, vB, vA);
}
break;
case 0x21: 
op->type = R_ANAL_OP_TYPE_LENGTH;
op->datatype = R_ANAL_DATATYPE_ARRAY;
break;
case 0x44: 
case 0x45: 
case 0x46:
case 0x47: 
case 0x48: 
case 0x49: 
case 0x4a: 
case 0x52: 
case 0x58: 
case 0x53: 
case 0x56: 
case 0x57: 
case 0xea: 
case 0xf4: 
case 0x66: 
case 0xfd: 
case 0x55: 
case 0x60: 
case 0x61: 
case 0x64: 
case 0x65: 
case 0xe3: 
case 0xe4: 
case 0xe5: 
case 0xe6: 
case 0xe7: 
case 0xe8: 
case 0xf3: 
case 0xf8: 
case 0xf2: 
op->type = R_ANAL_OP_TYPE_LOAD;
break;
case 0x54: 
op->type = R_ANAL_OP_TYPE_LOAD;
if (mask & R_ANAL_OP_MASK_ESIL) {
ut32 vA = (data[1] & 0x0f);
ut32 vB = (data[1] & 0xf0) >> 4;
ut32 vC = (data[2] & 0x0f);
esilprintf (op, "%d,v%d,iget,v%d,=", vC, vB, vA);
}
break;
case 0x63: 
op->datatype = R_ANAL_DATATYPE_BOOLEAN;
op->type = R_ANAL_OP_TYPE_LOAD;
if (mask & R_ANAL_OP_MASK_ESIL) {
ut32 vA = (data[1] & 0x0f);
ut32 vB = (data[1] & 0xf0) >> 4;
ut32 vC = (data[2] & 0x0f);
const char *vT = "-boolean";
esilprintf (op, "%d,%d,sget%s,v%d,=", vC, vB, vT, vA);
}
break;
case 0x62: 
{
op->datatype = R_ANAL_DATATYPE_OBJECT;
op->type = R_ANAL_OP_TYPE_LOAD;
ut32 vC = len > 3?(data[3] << 8) | data[2] : 0;
op->ptr = anal->binb.get_offset (anal->binb.bin, 'f', vC);
if (mask & R_ANAL_OP_MASK_ESIL) {
ut32 vA = (data[1] & 0x0f);
esilprintf (op, "%d,v%d,=", op->ptr, vA);
}
}
break;
case 0x6b: 
case 0x6d: 
case 0xeb: 
case 0x4b: 
case 0x4c: 
case 0x4d: 
case 0x4e: 
case 0x4f: 
case 0x5e: 
case 0xfc: 
case 0xf5: 
case 0x5c: 
case 0x69: 
case 0x5f: 
case 0xe9: 
case 0xf6: 
case 0xf7: 
case 0x67: 
case 0x59: 
case 0x5a: 
case 0x5b: 
case 0x5d: 
case 0x50: 
case 0x51: 
case 0x68: 
case 0x6a: 
case 0x6c: 
case 0xfe: 
{
op->type = R_ANAL_OP_TYPE_STORE;
ut32 vC = len > 3?(data[3] << 8) | data[2] : 0;
op->ptr = anal->binb.get_offset (anal->binb.bin, 'f', vC);
}
if (mask & R_ANAL_OP_MASK_ESIL) {
ut32 vA = (data[1] & 0x0f);
esilprintf (op, "%d,v%d,=", op->ptr, vA);
}
break;
case 0xad: 
op->family = R_ANAL_OP_FAMILY_FPU;
op->type = R_ANAL_OP_TYPE_MUL;
if (mask & R_ANAL_OP_MASK_ESIL) {
esilprintf (op, "v%d,v%d,*,v%d,=", vC, vB, vA);
}
break;
case 0x9d:
case 0xc8: 
op->family = R_ANAL_OP_FAMILY_FPU;
case 0xcd:
case 0xd2: 
case 0x92:
case 0xb2:
op->type = R_ANAL_OP_TYPE_MUL;
if (mask & R_ANAL_OP_MASK_ESIL) {
ut32 vA = (data[1] & 0x0f);
ut32 vB = (data[1] & 0xf0) >> 4;
ut32 vC = (data[2] << 8) | data[3];
esilprintf (op, "%d,v%d,*,v%d,=", vC, vB, vA);
op->val = vC;
}
break;
case 0x7c: 
case 0x7e: 
op->type = R_ANAL_OP_TYPE_NOT;
break;
case 0xa4: 
case 0xba: 
case 0xe2: 
case 0xa5: 
case 0x9a: 
case 0xc5: 
case 0xc4: 
case 0xe1: 
case 0x99: 
op->type = R_ANAL_OP_TYPE_SHR;
break;
case 0xaa: 
case 0xcf: 
case 0xaf: 
op->family = R_ANAL_OP_FAMILY_FPU;
case 0xb4: 
case 0xdc: 
case 0xd4: 
case 0xbf: 
case 0x9f: 
case 0x94: 
op->type = R_ANAL_OP_TYPE_MOD; 
break;
case 0xd7:
case 0xd9:
case 0xda:
case 0xde:
case 0x95: 
case 0x96: 
op->type = R_ANAL_OP_TYPE_OR;
break;
case 0xc2: 
case 0x97: 
case 0xdf: 
case 0xa2: 
op->type = R_ANAL_OP_TYPE_XOR;
break;
case 0xc9: 
op->family = R_ANAL_OP_FAMILY_FPU;
case 0x93: 
case 0xd3: 
case 0xdb: 
case 0xce: 
case 0x9e: 
case 0xbe: 
case 0xae: 
case 0xa9: 
case 0xb3: 
op->type = R_ANAL_OP_TYPE_DIV;
break;
case 0x0e: 
case 0x0f: 
case 0x10: 
case 0x11: 
case 0xf1: 
op->type = R_ANAL_OP_TYPE_RET;
op->eob = true;
if (mask & R_ANAL_OP_MASK_ESIL) {
if (data[0] == 0x0e) {
esilprintf (op, "sp,[8],ip,=,8,sp,+=");
} else {
ut32 vA = data[1];
esilprintf (op, "sp,[8],ip,=,8,sp,+=,8,sp,-=,v%d,sp,=[8]", vA);
}
}
break;
case 0x28: 
op->jump = addr + ((char)data[1])*2;
op->type = R_ANAL_OP_TYPE_JMP;
op->eob = true;
if (mask & R_ANAL_OP_MASK_ESIL) {
esilprintf (op, "0x%"PFMT64x",ip,=", op->jump);
}
break;
case 0x29: 
if (len > 3) {
op->jump = addr + (short)(data[2]|data[3]<<8)*2;
op->type = R_ANAL_OP_TYPE_JMP;
op->eob = true;
if (mask & R_ANAL_OP_MASK_ESIL) {
esilprintf (op, "0x%"PFMT64x",ip,=", op->jump);
}
}
break;
case 0x2a: 
if (len > 5) {
st64 dst = (st64)(data[2]|(data[3]<<8)|(data[4]<<16)|(data[5]<<24));
op->jump = addr + (dst * 2);
op->type = R_ANAL_OP_TYPE_JMP;
op->eob = true;
if (mask & R_ANAL_OP_MASK_ESIL) {
esilprintf (op, "0x%"PFMT64x",ip,=", op->jump);
}
}
break;
case 0x2c:
case 0x2b:
op->type = R_ANAL_OP_TYPE_SWITCH;
break;
case 0x3e: 
case 0x43:
op->type = R_ANAL_OP_TYPE_ILL;
op->size = 1;
op->eob = true;
if (mask & R_ANAL_OP_MASK_ESIL) {
esilprintf (op, ",");
}
break;
case 0x2d: 
case 0x2e: 
case 0x3f: 
case 0x2f: 
case 0x30: 
case 0x31: 
case 0x1f: 
op->type = R_ANAL_OP_TYPE_CMP;
break;
case 0x32: 
case 0x33: 
case 0x34: 
case 0x35: 
case 0x36: 
case 0x37: 
op->type = R_ANAL_OP_TYPE_CJMP;
if (len > 2) {
op->jump = addr + (len>3?(short)(data[2]|data[3]<<8)*2 : 0);
op->fail = addr + sz;
op->eob = true;
if (mask & R_ANAL_OP_MASK_ESIL) {
ut32 vA = data[1];
ut32 vB = data[2];
const char *cond = getCond (data[0]);
esilprintf (op, "v%d,v%d,==,%s,?{,%"PFMT64d",ip,=}", vB, vA, cond, op->jump);
}
}
break;
case 0x38: 
case 0x39: 
case 0x3a: 
case 0x3b: 
case 0x3c: 
case 0x3d: 
op->type = R_ANAL_OP_TYPE_CJMP;
if (len > 2) {
op->jump = addr + (len>3?(short)(data[2]|data[3]<<8)*2 : 0);
op->fail = addr + sz;
op->eob = true;
if (mask & R_ANAL_OP_MASK_ESIL) {
ut32 vA = data[1];
const char *cond = getCondz (data[0]);
esilprintf (op, "v%d,%s,?{,%"PFMT64d",ip,=}", vA, cond, op->jump);
}
}
break;
case 0xec: 
op->type = R_ANAL_OP_TYPE_TRAP;
if (mask & R_ANAL_OP_MASK_ESIL) {
esilprintf (op, "TRAP");
}
break;
case 0x1d: 
op->type = R_ANAL_OP_TYPE_PUSH;
op->stackop = R_ANAL_STACK_INC;
op->stackptr = 1;
if (mask & R_ANAL_OP_MASK_ESIL) {
esilprintf (op, ",");
}
break;
case 0x1e: 
op->type = R_ANAL_OP_TYPE_POP;
op->stackop = R_ANAL_STACK_INC;
op->stackptr = -1;
if (mask & R_ANAL_OP_MASK_ESIL) {
esilprintf (op, ",");
}
break;
case 0x73: 
break;
case 0x6f: 
case 0x70: 
case 0x71: 
case 0x72: 
case 0x77: 
case 0xb9: 
case 0xb7: 
case 0xb6: 
case 0x6e: 
if (len > 2) {
ut32 vB = len > 3?(data[3] << 8) | data[2] : 0;
ut64 dst = anal->binb.get_offset (anal->binb.bin, 'm', vB);
if (dst == 0) {
op->type = R_ANAL_OP_TYPE_UCALL;
} else {
op->type = R_ANAL_OP_TYPE_CALL;
op->jump = dst;
}
op->fail = addr + sz;
if (mask & R_ANAL_OP_MASK_ESIL) {
esilprintf (op, "8,sp,-=,0x%"PFMT64x",sp,=[8],0x%"PFMT64x",ip,=", op->fail, op->jump);
}
}
break;
case 0x78: 
case 0xf0: 
case 0xf9: 
case 0xfb: 
case 0x74: 
case 0x75: 
case 0x76: 
case 0xfa: 
if (len > 2) {
op->fail = addr + sz;
op->type = R_ANAL_OP_TYPE_UCALL;
}
if (mask & R_ANAL_OP_MASK_ESIL) {
esilprintf (op, "8,sp,-=,0x%"PFMT64x",sp,=[8],0x%"PFMT64x",ip,=", op->fail, op->jump);
}
break;
case 0x27: 
{
op->type = R_ANAL_OP_TYPE_TRAP;
if (mask & R_ANAL_OP_MASK_ESIL) {
ut32 vA = data[1];
esilprintf (op, "v%d,TRAP", vA);
}
}
break;
case 0xee: 
case 0xef: 
op->type = R_ANAL_OP_TYPE_SWI;
break;
case 0xed: 
op->type = R_ANAL_OP_TYPE_TRAP;
break;
case 0x22: 
op->type = R_ANAL_OP_TYPE_NEW;
if (len > 2) {
int vB = (data[3] << 8) | data[2];
ut64 off = R_ANAL_GET_OFFSET (anal, 't', vB);
op->ptr = off;
if (mask & R_ANAL_OP_MASK_ESIL) {
int vA = (int) data[1];
esilprintf (op, "%d,new,v%d,=", off, vA);
}
}
break;
case 0x23: 
op->type = R_ANAL_OP_TYPE_NEW;
if (len > 2 && mask & R_ANAL_OP_MASK_ESIL) {
ut32 vA = (data[1] & 0x0f);
ut32 vB = (data[1] & 0xf0) >> 4;
ut32 vC = (int) data[2] | (data[3]<<8);
esilprintf (op, "%d,%d,new-array,v%d,=",vC, vB, vA);
}
break;
case 0x24: 
case 0x25: 
case 0x26: 
op->type = R_ANAL_OP_TYPE_NEW;
if (len > 2) {
int vB = (data[3] << 8) | data[2];
ut64 off = R_ANAL_GET_OFFSET (anal, 't', vB);
op->ptr = off;
}
break;
case 0x00: 
op->type = R_ANAL_OP_TYPE_NOP;
if (mask & R_ANAL_OP_MASK_ESIL) {
esilprintf (op, ",");
}
break;
case 0x90: 
case 0x9b: 
case 0xa6: 
case 0xac: 
case 0xb0: 
case 0xbb: 
case 0xc6: 
case 0xcb: 
case 0xd0: 
case 0xd8: 
op->type = R_ANAL_OP_TYPE_ADD;
if (mask & R_ANAL_OP_MASK_ESIL) {
ut32 vB = (data[1] & 0x0f);
ut32 vA = (data[1] & 0xf0) >> 4;
esilprintf (op, "v%d,v%d,+=", vB, vA);
}
break;
case 0xa7: 
case 0xcc: 
op->family = R_ANAL_OP_FAMILY_FPU;
case 0xc7:
case 0xbc:
case 0x91:
case 0xb1: 
case 0xd1: 
case 0x9c: 
op->type = R_ANAL_OP_TYPE_SUB;
if (mask & R_ANAL_OP_MASK_ESIL) {
esilprintf (op, "v%d,v%d,-,v%d,=", vC, vB, vA);
}
break;
case 0x7b: 
case 0x7d: 
case 0x7f: 
case 0x80: 
op->type = R_ANAL_OP_TYPE_NOT;
break;
case 0xa0: 
case 0xc0: 
case 0xdd: 
case 0xd5: 
case 0xb5: 
op->type = R_ANAL_OP_TYPE_AND;
break;
case 0xd6: 
case 0xc1: 
case 0xa1: 
op->type = R_ANAL_OP_TYPE_OR;
if (mask & R_ANAL_OP_MASK_ESIL) {
ut32 vA = (data[1] & 0x0f);
ut32 vB = (data[1] & 0xf0) >> 4;
ut32 vC = (data[2] << 8) | data[3];
esilprintf (op, "%d,v%d,|,v%d,=", vC, vB, vA);
op->val = vC;
}
break;
case 0xe0: 
case 0xc3: 
case 0xa3: 
case 0x98: 
case 0xb8: 
op->type = R_ANAL_OP_TYPE_SHL;
break;
}
return sz;
}
static int set_reg_profile(RAnal *anal) {
const char *p =
"=PC ip\n"
"=SP sp\n"
"=BP bp\n"
"=A0 v0\n"
"=A1 v1\n"
"=A2 v2\n"
"=A3 v3\n"
"=SN v0\n"
"gpr v0 .32 0 0\n"
"gpr v1 .32 4 0\n"
"gpr v2 .32 8 0\n"
"gpr v3 .32 12 0\n"
"gpr v4 .32 16 0\n"
"gpr v5 .32 20 0\n"
"gpr v6 .32 24 0\n"
"gpr v7 .32 28 0\n"
"gpr v8 .32 32 0\n"
"gpr v9 .32 36 0\n"
"gpr v10 .32 40 0\n"
"gpr v11 .32 44 0\n"
"gpr v12 .32 48 0\n"
"gpr v13 .32 52 0\n"
"gpr v14 .32 56 0\n"
"gpr v15 .32 60 0\n"
"gpr v16 .32 40 0\n"
"gpr v17 .32 44 0\n"
"gpr v18 .32 48 0\n"
"gpr v19 .32 52 0\n"
"gpr v20 .32 56 0\n"
"gpr v21 .32 60 0\n"
"gpr v22 .32 64 0\n"
"gpr v23 .32 68 0\n"
"gpr v24 .32 72 0\n"
"gpr v25 .32 76 0\n"
"gpr v26 .32 80 0\n"
"gpr v27 .32 84 0\n"
"gpr v28 .32 88 0\n"
"gpr v29 .32 92 0\n"
"gpr v30 .32 96 0\n"
"gpr v31 .32 100 0\n"
"gpr v32 .32 104 0\n"
"gpr v33 .32 108 0\n"
"gpr v34 .32 112 0\n"
"gpr ip .32 116 0\n"
"gpr sp .32 120 0\n"
"gpr bp .32 124 0\n"
;
return r_reg_set_profile_string (anal->reg, p);
}
RAnalPlugin r_anal_plugin_dalvik = {
.name = "dalvik",
.arch = "dalvik",
.set_reg_profile = &set_reg_profile,
.license = "LGPL3",
.bits = 32,
.desc = "Dalvik (Android VM) bytecode analysis plugin",
.op = &dalvik_op,
};
#if !defined(R2_PLUGIN_INCORE)
R_API RLibStruct radare_plugin = {
.type = R_LIB_TYPE_ANAL,
.data = &r_anal_plugin_dalvik,
.version = R2_VERSION
};
#endif
