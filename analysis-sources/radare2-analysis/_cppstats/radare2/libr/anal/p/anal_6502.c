#include <string.h>
#include <r_types.h>
#include <r_lib.h>
#include <r_asm.h>
#include <r_anal.h>
#include "../../asm/arch/snes/snes_op_table.h"
enum {
_6502_FLAGS_C = (1 << 0),
_6502_FLAGS_B = (1 << 1),
_6502_FLAGS_Z = (1 << 2),
_6502_FLAGS_N = (1 << 3),
_6502_FLAGS_NZ = (_6502_FLAGS_Z | _6502_FLAGS_N),
_6502_FLAGS_CNZ = (_6502_FLAGS_C | _6502_FLAGS_Z | _6502_FLAGS_N),
_6502_FLAGS_BNZ = (_6502_FLAGS_B | _6502_FLAGS_Z | _6502_FLAGS_N),
};
static void _6502_anal_update_flags(RAnalOp *op, int flags) {
if (flags & _6502_FLAGS_B) {
r_strbuf_append (&op->esil, ",9,$b,C,:=");
}
if (flags & _6502_FLAGS_C) {
r_strbuf_append (&op->esil, ",7,$c,C,:=");
}
if (flags & _6502_FLAGS_Z) {
r_strbuf_append (&op->esil, ",$z,Z,:=");
}
if (flags & _6502_FLAGS_N) {
r_strbuf_append (&op->esil, ",7,$s,N,:=");
}
}
static void _6502_anal_esil_get_addr_pattern1(RAnalOp *op, const ut8* data, int len, char* addrbuf, int addrsize) {
if (len < 1) {
return;
}
switch (data[0] & 0x1f) { 
case 0x09: 
op->cycles = 2;
snprintf (addrbuf, addrsize,"0x%02x", (len > 1)? data[1]: 0);
break;
case 0x05: 
op->cycles = 3;
snprintf (addrbuf, addrsize,"0x%02x", (len > 1)? data[1]: 0);
break;
case 0x15: 
op->cycles = 4;
snprintf (addrbuf, addrsize,"x,0x%02x,+", (len > 1)? data[1]: 0);
break;
case 0x0d: 
op->cycles = 4;
snprintf (addrbuf, addrsize,"0x%04x", (len > 2) ? (data[1] | data[2] << 8): 0);
break;
case 0x1d: 
op->cycles = 4;
snprintf (addrbuf, addrsize,"x,0x%04x,+", (len > 2) ? data[1] | data[2] << 8: 0);
break;
case 0x19: 
op->cycles = 4;
snprintf (addrbuf, addrsize,"y,0x%04x,+", (len > 2)? data[1] | data[2] << 8: 0);
break;
case 0x01: 
op->cycles = 6;
snprintf (addrbuf, addrsize,"x,0x%02x,+,[2]", (len > 1)? data[1]: 0);
break;
case 0x11: 
op->cycles = 5;
snprintf (addrbuf, addrsize,"y,0x%02x,[2],+", (len > 1) ? data[1]: 0);
break;
}
}
static void _6502_anal_esil_get_addr_pattern2(RAnalOp *op, const ut8* data, int len, char* addrbuf, int addrsize, char reg) {
if (len < 1) {
return;
}
switch (data[0] & 0x1f) { 
case 0x02: 
op->cycles = 2;
snprintf (addrbuf, addrsize, "0x%02x", (len>1)? data[1]: 0);
break;
case 0x0a: 
op->cycles = 2;
snprintf (addrbuf, addrsize, "a");
break;
case 0x06: 
op->cycles = 5;
snprintf (addrbuf, addrsize, "0x%02x", (len>1)?data[1]:0);
break;
case 0x16: 
op->cycles = 6;
snprintf (addrbuf, addrsize, "%c,0x%02x,+", reg, (len >1)? data[1]:0);
break;
case 0x0e: 
op->cycles = 6;
snprintf (addrbuf, addrsize, "0x%04x", (len>2)? data[1] | data[2] << 8: 0);
break;
case 0x1e: 
op->cycles = 7;
snprintf (addrbuf, addrsize, "%c,0x%04x,+", reg, (len>2)? data[1] | data[2] << 8: 0);
break;
}
}
static void _6502_anal_esil_get_addr_pattern3(RAnalOp *op, const ut8* data, int len, char* addrbuf, int addrsize, char reg) {
if (len < 1) {
return;
}
switch (data[0] & 0x1f) { 
case 0x00: 
op->cycles = 2;
snprintf (addrbuf, addrsize, "0x%02x", (len > 1) ? data[1]: 0);
break;
case 0x08: 
op->cycles = 2;
snprintf (addrbuf, addrsize, "a");
break;
case 0x04: 
op->cycles = 5;
snprintf (addrbuf, addrsize, "0x%02x", (len > 1)? data[1]: 0);
break;
case 0x14: 
op->cycles = 6;
snprintf (addrbuf, addrsize, "%c,0x%02x,+", reg, (len>1)? data[1]:0);
break;
case 0x0c: 
op->cycles = 6;
snprintf (addrbuf, addrsize, "0x%04x", (len>2)? data[1] | data[2] << 8: 0);
break;
case 0x1c: 
op->cycles = 7;
snprintf (addrbuf, addrsize, "%c,0x%04x,+", reg, (len>2)? data[1] | data[2] << 8: 0);
break;
}
}
static void _6502_anal_esil_ccall(RAnalOp *op, ut8 data0) {
char *flag;
switch (data0) {
case 0x10: 
flag = "N,!";
break;
case 0x30: 
flag = "N";
break;
case 0x50: 
flag = "V,!";
break;
case 0x70: 
flag = "V";
break;
case 0x90: 
flag = "C,!";
break;
case 0xb0: 
flag = "C";
break;
case 0xd0: 
flag = "Z,!";
break;
case 0xf0: 
flag = "Z";
break;
default:
flag = "unk";
break;
}
r_strbuf_setf (&op->esil, "%s,?{,0x%04x,pc,=,}", flag, (op->jump & 0xffff));
}
static void _6502_anal_esil_inc_reg(RAnalOp *op, ut8 data0, char* sign) {
char* reg = NULL;
switch(data0) {
case 0xe8: 
case 0xca: 
reg = "x";
break;
case 0xc8: 
case 0x88: 
reg = "y";
break;
}
r_strbuf_setf (&op->esil, "%s,%s%s=", reg, sign, sign);
_6502_anal_update_flags (op, _6502_FLAGS_NZ);
}
static void _6502_anal_esil_mov(RAnalOp *op, ut8 data0) {
const char* src="unk";
const char* dst="unk";
switch(data0) {
case 0xaa: 
src="a";
dst="x";
break;
case 0x8a: 
src="x";
dst="a";
break;
case 0xa8: 
src="a";
dst="y";
break;
case 0x98: 
src="y";
dst="a";
break;
case 0x9a: 
src="x";
dst="sp";
break;
case 0xba: 
src="sp";
dst="x";
break;
default:
break;
}
r_strbuf_setf (&op->esil, "%s,%s,=",src,dst);
if (data0 != 0x9a) {
_6502_anal_update_flags (op, _6502_FLAGS_NZ);
}
}
static void _6502_anal_esil_push(RAnalOp *op, ut8 data0) {
char *reg = (data0==0x08) ? "flags" : "a";
r_strbuf_setf (&op->esil, "%s,sp,0x100,+,=[1],sp,--=", reg);
}
static void _6502_anal_esil_pop(RAnalOp *op, ut8 data0) {
char *reg = (data0==0x28) ? "flags" : "a";
r_strbuf_setf (&op->esil, "sp,++=,sp,0x100,+,[1],%s,=", reg);
if (data0 == 0x68) {
_6502_anal_update_flags (op, _6502_FLAGS_NZ);
}
}
static void _6502_anal_esil_flags(RAnalOp *op, ut8 data0) {
int enabled=0;
char flag ='u';
switch(data0) {
case 0x78: 
enabled = 1;
flag = 'I';
break;
case 0x58: 
enabled = 0;
flag = 'I';
break;
case 0x38: 
enabled = 1;
flag = 'C';
break;
case 0x18: 
enabled = 0;
flag = 'C';
break;
case 0xf8: 
enabled = 1;
flag = 'D';
break;
case 0xd8: 
enabled = 0;
flag = 'D';
break;
case 0xb8: 
enabled = 0;
flag = 'V';
break;
break;
}
r_strbuf_setf (&op->esil, "%d,%c,=", enabled, flag);
}
static int _6502_op(RAnal *anal, RAnalOp *op, ut64 addr, const ut8 *data, int len, RAnalOpMask mask) {
char addrbuf[64];
const int buffsize = sizeof (addrbuf) - 1;
if (len < 1) {
return -1;
}
memset (op, '\0', sizeof (RAnalOp));
op->size = snes_op_get_size (1, 1, &snes_op[data[0]]); 
op->addr = addr;
op->type = R_ANAL_OP_TYPE_UNK;
op->id = data[0];
r_strbuf_init (&op->esil);
switch (data[0]) {
case 0x02:
case 0x03:
case 0x04:
case 0x07:
case 0x0b:
case 0x0c:
case 0x0f:
case 0x12:
case 0x13:
case 0x14:
case 0x17:
case 0x1a:
case 0x1b:
case 0x1c:
case 0x1f:
case 0x22:
case 0x23:
case 0x27:
case 0x2b:
case 0x2f:
case 0x32:
case 0x33:
case 0x34:
case 0x37:
case 0x3a:
case 0x3b:
case 0x3c:
case 0x3f:
case 0x42:
case 0x43:
case 0x44:
case 0x47:
case 0x4b:
case 0x4f:
case 0x52:
case 0x53:
case 0x54:
case 0x57:
case 0x5a:
case 0x5b:
case 0x5c:
case 0x5f:
case 0x62:
case 0x63:
case 0x64:
case 0x67:
case 0x6b:
case 0x6f:
case 0x72:
case 0x73:
case 0x74:
case 0x77:
case 0x7a:
case 0x7b:
case 0x7c:
case 0x7f:
case 0x80:
case 0x82:
case 0x83:
case 0x87:
case 0x89:
case 0x8b:
case 0x8f:
case 0x92:
case 0x93:
case 0x97:
case 0x9b:
case 0x9c:
case 0x9e:
case 0x9f:
case 0xa3:
case 0xa7:
case 0xab:
case 0xaf:
case 0xb2:
case 0xb3:
case 0xb7:
case 0xbb:
case 0xbf:
case 0xc2:
case 0xc3:
case 0xc7:
case 0xcb:
case 0xcf:
case 0xd2:
case 0xd3:
case 0xd4:
case 0xd7:
case 0xda:
case 0xdb:
case 0xdc:
case 0xdf:
case 0xe2:
case 0xe3:
case 0xe7:
case 0xeb:
case 0xef:
case 0xf2:
case 0xf3:
case 0xf4:
case 0xf7:
case 0xfa:
case 0xfb:
case 0xfc:
case 0xff:
op->size = 1;
op->type = R_ANAL_OP_TYPE_ILL;
break;
case 0x00: 
op->cycles = 7;
op->type = R_ANAL_OP_TYPE_SWI;
op->size = 1;
r_strbuf_set (&op->esil, ",1,I,=,0,D,=,flags,0x10,|,0x100,sp,+,=[1],pc,1,+,0xfe,sp,+,=[2],3,sp,-=,0xfffe,[2],pc,=");
break;
case 0x78: 
case 0x58: 
case 0x38: 
case 0x18: 
case 0xf8: 
case 0xd8: 
case 0xb8: 
op->cycles = 2;
op->type = R_ANAL_OP_TYPE_NOP;
_6502_anal_esil_flags (op, data[0]);
break;
case 0x24: 
case 0x2c: 
op->type = R_ANAL_OP_TYPE_MOV;
_6502_anal_esil_get_addr_pattern3 (op, data, len, addrbuf, buffsize, 0);
r_strbuf_setf (&op->esil, "%s,[1],0x80,&,!,!,N,=,%s,[1],0x40,&,!,!,V,=,a,%s,[1],&,0xff,&,!,Z,=", addrbuf, addrbuf, addrbuf);
break;
case 0x69: 
case 0x65: 
case 0x75: 
case 0x6d: 
case 0x7d: 
case 0x79: 
case 0x61: 
case 0x71: 
op->type = R_ANAL_OP_TYPE_ADD;
_6502_anal_esil_get_addr_pattern1 (op, data, len, addrbuf, buffsize);
if (data[0] == 0x69) { 
r_strbuf_setf (&op->esil, "%s,a,+=,7,$c,C,a,+=,7,$c,|,C,:=", addrbuf);
} else {
r_strbuf_setf (&op->esil, "%s,[1],a,+=,7,$c,C,a,+=,7,$c,|,C,:=", addrbuf);
}
_6502_anal_update_flags (op, _6502_FLAGS_NZ);
r_strbuf_append (&op->esil, ",a,a,=,$z,Z,:=");
break;
case 0xe9: 
case 0xe5: 
case 0xf5: 
case 0xed: 
case 0xfd: 
case 0xf9: 
case 0xe1: 
case 0xf1: 
op->type = R_ANAL_OP_TYPE_SUB;
_6502_anal_esil_get_addr_pattern1 (op, data, len, addrbuf, buffsize);
if (data[0] == 0xe9) { 
r_strbuf_setf (&op->esil, "C,!,%s,+,a,-=", addrbuf);
} else {
r_strbuf_setf (&op->esil, "C,!,%s,[1],+,a,-=", addrbuf);
}
_6502_anal_update_flags (op, _6502_FLAGS_BNZ);
r_strbuf_append (&op->esil, ",a,a,=,$z,Z,:=,C,!=");
break;
case 0x09: 
case 0x05: 
case 0x15: 
case 0x0d: 
case 0x1d: 
case 0x19: 
case 0x01: 
case 0x11: 
op->type = R_ANAL_OP_TYPE_OR;
_6502_anal_esil_get_addr_pattern1 (op, data, len, addrbuf, buffsize);
if (data[0] == 0x09) { 
r_strbuf_setf (&op->esil, "%s,a,|=", addrbuf);
} else {
r_strbuf_setf (&op->esil, "%s,[1],a,|=", addrbuf);
}
_6502_anal_update_flags (op, _6502_FLAGS_NZ);
break;
case 0x29: 
case 0x25: 
case 0x35: 
case 0x2d: 
case 0x3d: 
case 0x39: 
case 0x21: 
case 0x31: 
op->type = R_ANAL_OP_TYPE_AND;
_6502_anal_esil_get_addr_pattern1 (op, data, len, addrbuf, buffsize);
if (data[0] == 0x29) { 
r_strbuf_setf (&op->esil, "%s,a,&=", addrbuf);
} else {
r_strbuf_setf (&op->esil, "%s,[1],a,&=", addrbuf);
}
_6502_anal_update_flags (op, _6502_FLAGS_NZ);
break;
case 0x49: 
case 0x45: 
case 0x55: 
case 0x4d: 
case 0x5d: 
case 0x59: 
case 0x41: 
case 0x51: 
op->type = R_ANAL_OP_TYPE_XOR;
_6502_anal_esil_get_addr_pattern1 (op, data, len, addrbuf, buffsize);
if (data[0] == 0x49) { 
r_strbuf_setf (&op->esil, "%s,a,^=", addrbuf);
} else {
r_strbuf_setf (&op->esil, "%s,[1],a,^=", addrbuf);
}
_6502_anal_update_flags (op, _6502_FLAGS_NZ);
break;
case 0x0a: 
case 0x06: 
case 0x16: 
case 0x0e: 
case 0x1e: 
op->type = R_ANAL_OP_TYPE_SHL;
if (data[0] == 0x0a) {
r_strbuf_set (&op->esil, "1,a,<<=,7,$c,C,:=,a,a,=");
} else {
_6502_anal_esil_get_addr_pattern2 (op, data, len, addrbuf, buffsize, 'x');
r_strbuf_setf (&op->esil, "1,%s,[1],<<,%s,=[1],7,$c,C,:=", addrbuf, addrbuf);
}
_6502_anal_update_flags (op, _6502_FLAGS_NZ);
break;
case 0x4a: 
case 0x46: 
case 0x56: 
case 0x4e: 
case 0x5e: 
op->type = R_ANAL_OP_TYPE_SHR;
if (data[0] == 0x4a) {
r_strbuf_set (&op->esil, "1,a,&,C,=,1,a,>>=");
} else {
_6502_anal_esil_get_addr_pattern2 (op, data, len, addrbuf, buffsize, 'x');
r_strbuf_setf (&op->esil, "1,%s,[1],&,C,=,1,%s,[1],>>,%s,=[1]", addrbuf, addrbuf, addrbuf);
}
_6502_anal_update_flags (op, _6502_FLAGS_NZ);
break;
case 0x2a: 
case 0x26: 
case 0x36: 
case 0x2e: 
case 0x3e: 
op->type = R_ANAL_OP_TYPE_ROL;
if (data[0] == 0x2a) {
r_strbuf_set (&op->esil, "1,a,<<,C,|,a,=,7,$c,C,:=,a,a,=");
} else {
_6502_anal_esil_get_addr_pattern2 (op, data, len, addrbuf, buffsize, 'x');
r_strbuf_setf (&op->esil, "1,%s,[1],<<,C,|,%s,=[1],7,$c,C,:=", addrbuf, addrbuf);
}
_6502_anal_update_flags (op, _6502_FLAGS_NZ);
break;
case 0x6a: 
case 0x66: 
case 0x76: 
case 0x6e: 
case 0x7e: 
op->type = R_ANAL_OP_TYPE_ROR;
if (data[0] == 0x6a) {
r_strbuf_set (&op->esil, "C,N,=,1,a,&,C,=,1,a,>>,7,N,<<,|,a,=");
} else {
_6502_anal_esil_get_addr_pattern2 (op, data, len, addrbuf, buffsize, 'x');
r_strbuf_setf (&op->esil, "C,N,=,1,%s,[1],&,C,=,1,%s,[1],>>,7,N,<<,|,%s,=[1]", addrbuf, addrbuf, addrbuf);
}
_6502_anal_update_flags (op, _6502_FLAGS_NZ);
break;
case 0xe6: 
case 0xf6: 
case 0xee: 
case 0xfe: 
op->type = R_ANAL_OP_TYPE_STORE;
_6502_anal_esil_get_addr_pattern2 (op, data, len, addrbuf, buffsize, 'x');
r_strbuf_setf (&op->esil, "%s,++=[1]", addrbuf);
_6502_anal_update_flags (op, _6502_FLAGS_NZ);
break;
case 0xc6: 
case 0xd6: 
case 0xce: 
case 0xde: 
op->type = R_ANAL_OP_TYPE_STORE;
_6502_anal_esil_get_addr_pattern2 (op, data, len, addrbuf, buffsize, 'x');
r_strbuf_setf (&op->esil, "%s,--=[1]", addrbuf);
_6502_anal_update_flags (op, _6502_FLAGS_NZ);
break;
case 0xe8: 
case 0xc8: 
op->cycles = 2;
op->type = R_ANAL_OP_TYPE_STORE;
_6502_anal_esil_inc_reg (op, data[0], "+");
break;
case 0xca: 
case 0x88: 
op->cycles = 2;
op->type = R_ANAL_OP_TYPE_STORE;
_6502_anal_esil_inc_reg (op, data[0], "-");
break;
case 0xc9: 
case 0xc5: 
case 0xd5: 
case 0xcd: 
case 0xdd: 
case 0xd9: 
case 0xc1: 
case 0xd1: 
op->type = R_ANAL_OP_TYPE_CMP;
_6502_anal_esil_get_addr_pattern1 (op, data, len, addrbuf, buffsize);
if (data[0] == 0xc9) { 
r_strbuf_setf (&op->esil, "%s,a,==", addrbuf);
} else {
r_strbuf_setf (&op->esil, "%s,[1],a,==", addrbuf);
}
_6502_anal_update_flags (op, _6502_FLAGS_BNZ);
r_strbuf_append (&op->esil, ",C,!,C,=");
break;
case 0xe0: 
case 0xe4: 
case 0xec: 
op->type = R_ANAL_OP_TYPE_CMP;
_6502_anal_esil_get_addr_pattern3 (op, data, len, addrbuf, buffsize, 0);
if (data[0] == 0xe0) { 
r_strbuf_setf (&op->esil, "%s,x,==", addrbuf);
} else {
r_strbuf_setf (&op->esil, "%s,[1],x,==", addrbuf);
}
_6502_anal_update_flags (op, _6502_FLAGS_BNZ);
r_strbuf_append (&op->esil, ",C,!,C,=");
break;
case 0xc0: 
case 0xc4: 
case 0xcc: 
op->type = R_ANAL_OP_TYPE_CMP;
_6502_anal_esil_get_addr_pattern3 (op, data, len, addrbuf, buffsize, 0);
if (data[0] == 0xc0) { 
r_strbuf_setf (&op->esil, "%s,y,==", addrbuf);
} else {
r_strbuf_setf (&op->esil, "%s,[1],y,==", addrbuf);
}
_6502_anal_update_flags (op, _6502_FLAGS_BNZ);
r_strbuf_append (&op->esil, ",C,!,C,=");
break;
case 0x10: 
case 0x30: 
case 0x50: 
case 0x70: 
case 0x90: 
case 0xb0: 
case 0xd0: 
case 0xf0: 
op->cycles = 2;
op->failcycles = 3;
op->type = R_ANAL_OP_TYPE_CJMP;
if (len > 1) {
if (data[1] <= 127) {
op->jump = addr + data[1] + op->size;
} else {
op->jump = addr - (256 - data[1]) + op->size;
}
} else {
op->jump = addr;
}
op->fail = addr + op->size;
_6502_anal_esil_ccall (op, data[0]);
break;
case 0x20: 
op->cycles = 6;
op->type = R_ANAL_OP_TYPE_CALL;
op->jump = (len > 2)? data[1] | data[2] << 8: 0;
op->stackop = R_ANAL_STACK_INC;
op->stackptr = 2;
r_strbuf_setf (&op->esil, "1,pc,-,0xff,sp,+,=[2],0x%04x,pc,=,2,sp,-=", op->jump);
break;
case 0x4c: 
op->cycles = 3;
op->type = R_ANAL_OP_TYPE_JMP;
op->jump = (len > 2)? data[1] | data[2] << 8: 0;
r_strbuf_setf (&op->esil, "0x%04x,pc,=", op->jump);
break;
case 0x6c: 
op->cycles = 5;
op->type = R_ANAL_OP_TYPE_UJMP;
r_strbuf_setf (&op->esil, "0x%04x,[2],pc,=", len > 2? data[1] | data[2] << 8: 0);
break;
case 0x60: 
op->eob = true;
op->type = R_ANAL_OP_TYPE_RET;
op->cycles = 6;
op->stackop = R_ANAL_STACK_INC;
op->stackptr = -2;
r_strbuf_set (&op->esil, "0x101,sp,+,[2],pc,=,pc,++=,2,sp,+=");
break;
case 0x40: 
op->eob = true;
op->type = R_ANAL_OP_TYPE_RET;
op->cycles = 6;
op->stackop = R_ANAL_STACK_INC;
op->stackptr = -3;
r_strbuf_set (&op->esil, "0x101,sp,+,[1],flags,=,0x102,sp,+,[2],pc,=,3,sp,+=");
break;
case 0xea: 
op->type = R_ANAL_OP_TYPE_NOP;
op->cycles = 2;
break;
case 0xa9: 
case 0xa5: 
case 0xb5: 
case 0xad: 
case 0xbd: 
case 0xb9: 
case 0xa1: 
case 0xb1: 
op->type = R_ANAL_OP_TYPE_LOAD;
_6502_anal_esil_get_addr_pattern1 (op, data, len, addrbuf, buffsize);
if (data[0] == 0xa9) { 
r_strbuf_setf (&op->esil, "%s,a,=", addrbuf);
} else {
r_strbuf_setf (&op->esil, "%s,[1],a,=", addrbuf);
}
_6502_anal_update_flags (op, _6502_FLAGS_NZ);
break;
case 0xa2: 
case 0xa6: 
case 0xb6: 
case 0xae: 
case 0xbe: 
op->type = R_ANAL_OP_TYPE_LOAD;
_6502_anal_esil_get_addr_pattern2 (op, data, len, addrbuf, buffsize, 'y');
if (data[0] == 0xa2) { 
r_strbuf_setf (&op->esil, "%s,x,=", addrbuf);
} else {
r_strbuf_setf (&op->esil, "%s,[1],x,=", addrbuf);
}
_6502_anal_update_flags (op, _6502_FLAGS_NZ);
break;
case 0xa0: 
case 0xa4: 
case 0xb4: 
case 0xac: 
case 0xbc: 
op->type = R_ANAL_OP_TYPE_LOAD;
_6502_anal_esil_get_addr_pattern3 (op, data, len, addrbuf, buffsize, 'x');
if (data[0] == 0xa0) { 
r_strbuf_setf (&op->esil, "%s,y,=", addrbuf);
} else {
r_strbuf_setf (&op->esil, "%s,[1],y,=", addrbuf);
}
_6502_anal_update_flags (op, _6502_FLAGS_NZ);
break;
case 0x85: 
case 0x95: 
case 0x8d: 
case 0x9d: 
case 0x99: 
case 0x81: 
case 0x91: 
op->type = R_ANAL_OP_TYPE_STORE;
_6502_anal_esil_get_addr_pattern1 (op, data, len, addrbuf, buffsize);
r_strbuf_setf (&op->esil, "a,%s,=[1]", addrbuf);
break;
case 0x86: 
case 0x96: 
case 0x8e: 
op->type = R_ANAL_OP_TYPE_STORE;
_6502_anal_esil_get_addr_pattern2 (op, data, len, addrbuf, buffsize, 'y');
r_strbuf_setf (&op->esil, "x,%s,=[1]", addrbuf);
break;
case 0x84: 
case 0x94: 
case 0x8c: 
op->type = R_ANAL_OP_TYPE_STORE;
_6502_anal_esil_get_addr_pattern3 (op, data, len, addrbuf, buffsize, 'x');
r_strbuf_setf (&op->esil, "y,%s,=[1]", addrbuf);
break;
case 0x08: 
case 0x48: 
op->type = R_ANAL_OP_TYPE_PUSH;
op->cycles = 3;
op->stackop = R_ANAL_STACK_INC;
op->stackptr = 1;
_6502_anal_esil_push (op, data[0]);
break;
case 0x28: 
case 0x68: 
op->type = R_ANAL_OP_TYPE_POP;
op->cycles = 4;
op->stackop = R_ANAL_STACK_INC;
op->stackptr = -1;
_6502_anal_esil_pop (op, data[0]);
break;
case 0xaa: 
case 0x8a: 
case 0xa8: 
case 0x98: 
op->type = R_ANAL_OP_TYPE_MOV;
op->cycles = 2;
_6502_anal_esil_mov (op, data[0]);
break;
case 0x9a: 
op->type = R_ANAL_OP_TYPE_MOV;
op->cycles = 2;
op->stackop = R_ANAL_STACK_SET;
_6502_anal_esil_mov (op, data[0]);
break;
case 0xba: 
op->type = R_ANAL_OP_TYPE_MOV;
op->cycles = 2;
op->stackop = R_ANAL_STACK_GET;
_6502_anal_esil_mov (op, data[0]);
break;
}
return op->size;
}
static int set_reg_profile(RAnal *anal) {
char *p =
"=PC pc\n"
"=SP sp\n"
"gpr a .8 0 0\n"
"gpr x .8 1 0\n"
"gpr y .8 2 0\n"
"gpr flags .8 3 0\n"
"gpr C .1 .24 0\n"
"gpr Z .1 .25 0\n"
"gpr I .1 .26 0\n"
"gpr D .1 .27 0\n"
"gpr V .1 .30 0\n"
"gpr N .1 .31 0\n"
"gpr sp .8 4 0\n"
"gpr pc .16 5 0\n";
return r_reg_set_profile_string (anal->reg, p);
}
static int esil_6502_init (RAnalEsil *esil) {
if (esil->anal && esil->anal->reg) { 
r_reg_set_value (esil->anal->reg, r_reg_get (esil->anal->reg, "pc", -1), 0x0000);
r_reg_set_value (esil->anal->reg, r_reg_get (esil->anal->reg, "sp", -1), 0xff);
r_reg_set_value (esil->anal->reg, r_reg_get (esil->anal->reg, "a", -1), 0x00);
r_reg_set_value (esil->anal->reg, r_reg_get (esil->anal->reg, "x", -1), 0x00);
r_reg_set_value (esil->anal->reg, r_reg_get (esil->anal->reg, "y", -1), 0x00);
r_reg_set_value (esil->anal->reg, r_reg_get (esil->anal->reg, "flags", -1), 0x00);
}
return true;
}
static int esil_6502_fini (RAnalEsil *esil) {
return true;
}
RAnalPlugin r_anal_plugin_6502 = {
.name = "6502",
.desc = "6502/NES analysis plugin",
.license = "LGPL3",
.arch = "6502",
.bits = 8,
.op = &_6502_op,
.set_reg_profile = &set_reg_profile,
.esil = true,
.esil_init = esil_6502_init,
.esil_fini = esil_6502_fini,
};
#if !defined(R2_PLUGIN_INCORE)
R_API RLibStruct radare_plugin = {
.type = R_LIB_TYPE_ANAL,
.data = &r_anal_plugin_6502,
.version = R2_VERSION
};
#endif
