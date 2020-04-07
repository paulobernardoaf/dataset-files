

#include <string.h>
#include <r_types.h>
#include <r_lib.h>
#include <r_asm.h>
#include <r_anal.h>
#include "../../asm/arch/snes/snes_op_table.h"
#include "../../asm/p/asm_snes.h"

static struct snes_asm_flags* snesflags = NULL;

static int snes_anop(RAnal *anal, RAnalOp *op, ut64 addr, const ut8 *data, int len, RAnalOpMask mask) {
memset (op, '\0', sizeof (RAnalOp));
op->size = snes_op_get_size(snesflags->M, snesflags->X, &snes_op[data[0]]);
if (op->size > len) {
return op->size = 0;
}
op->nopcode = 1;
op->addr = addr;
op->type = R_ANAL_OP_TYPE_UNK;
switch (data[0]) {
case 0xea: 
op->type = R_ANAL_OP_TYPE_NOP;
break;
case 0xfb: 
op->type = R_ANAL_OP_TYPE_XCHG;
break;
case 0x00: 
case 0x02: 
op->type = R_ANAL_OP_TYPE_SWI;
break;
case 0x1b: 
case 0x3b: 
case 0x5b: 
case 0x7b: 
case 0x8a: 
case 0x98: 
case 0x9a: 
case 0x9b: 
case 0xa8: 
case 0xaa: 
case 0xba: 
case 0xbb: 
op->type = R_ANAL_OP_TYPE_MOV;
break;
case 0x48: 
case 0x8b: 
case 0x0b: 
case 0x4b: 
case 0x08: 
case 0xda: 
case 0x5a: 
op->type = R_ANAL_OP_TYPE_PUSH;
break;
case 0x68: 
case 0xab: 
case 0x2b: 
case 0x28: 
case 0xfa: 
case 0x7a: 
op->type = R_ANAL_OP_TYPE_POP;
break;

case 0x61: case 0x63: case 0x65: case 0x67: case 0x69: case 0x6d:
case 0x6f: case 0x71: case 0x72: case 0x73: case 0x75: case 0x77:
case 0x79: case 0x7d: case 0x7f:

case 0x1a: case 0xe6: case 0xee: case 0xf6: case 0xfe:
case 0xe8: 
case 0xc8: 
op->type = R_ANAL_OP_TYPE_ADD;
break;

case 0x23: case 0x25: case 0x27: case 0x29: case 0x2d: case 0x2f:
case 0x31: case 0x32: case 0x33: case 0x35: case 0x37: case 0x39:
case 0x3d: case 0x3f:
op->type = R_ANAL_OP_TYPE_AND;
break;

case 0x24: case 0x2c: case 0x34: case 0x3c: case 0x89:
op->type = R_ANAL_OP_TYPE_ACMP;
break;

case 0xc1: case 0xc3: case 0xc5: case 0xc7: case 0xc9: case 0xcd:
case 0xcf: case 0xd1: case 0xd2: case 0xd3: case 0xd5: case 0xd7:
case 0xd9: case 0xdd: case 0xdf:

case 0xe0: case 0xe4: case 0xec:

case 0xc0: case 0xc4: case 0xcc:
op->type = R_ANAL_OP_TYPE_CMP;
break;

case 0x01: case 0x03: case 0x05: case 0x07: case 0x09: case 0x0d:
case 0x0f: case 0x11: case 0x12: case 0x13: case 0x15: case 0x17:
case 0x19: case 0x1d: case 0x1f:
op->type = R_ANAL_OP_TYPE_OR;
break;

case 0x41: case 0x43: case 0x45: case 0x47: case 0x49: case 0x4d:
case 0x4f: case 0x51: case 0x52: case 0x53: case 0x55: case 0x57:
case 0x59: case 0x5d: case 0x5f:
op->type = R_ANAL_OP_TYPE_XOR;
break;

case 0x06: case 0x0a: case 0x0e: case 0x16: case 0x1e:
op->type = R_ANAL_OP_TYPE_SAL;
break;

case 0x46: case 0x4a: case 0x4e: case 0x56: case 0x5e:
op->type = R_ANAL_OP_TYPE_SHR;
break;

case 0x26: case 0x2a: case 0x2e: case 0x36: case 0x3e:
op->type = R_ANAL_OP_TYPE_ROL;
break;

case 0x66: case 0x6a: case 0x6e: case 0x76: case 0x7e:
op->type = R_ANAL_OP_TYPE_ROR;
break;

case 0xe1: case 0xe3: case 0xe5: case 0xe7: case 0xe9: case 0xed:
case 0xef: case 0xf1: case 0xf2: case 0xf3: case 0xf5: case 0xf7:
case 0xf9: case 0xfd: case 0xff:

case 0x3a: case 0xc6: case 0xce: case 0xd6: case 0xde:
case 0xca: 
case 0x88: 
op->type = R_ANAL_OP_TYPE_SUB;
break;

case 0x81: case 0x83: case 0x85: case 0x87: case 0x8d: case 0x8f:
case 0x91: case 0x92: case 0x93: case 0x95: case 0x97: case 0x99:
case 0x9d: case 0x9f:

case 0x86: case 0x8e: case 0x96:

case 0x84: case 0x8c: case 0x94:

case 0x64: case 0x74: case 0x9c: case 0x9e:
op->type = R_ANAL_OP_TYPE_STORE;
break;

case 0xa1: case 0xa3: case 0xa5: case 0xa7: case 0xa9: case 0xad:
case 0xaf: case 0xb1: case 0xb2: case 0xb3: case 0xb5: case 0xb7:
case 0xb9: case 0xbd: case 0xbf:

case 0xa2: case 0xa6: case 0xae: case 0xb6: case 0xbe:

case 0xa0: case 0xa4: case 0xac: case 0xb4: case 0xbc:
op->type = R_ANAL_OP_TYPE_LOAD;
break;
case 0x4c: 
op->eob = true;
op->jump = (addr & 0xFF0000) | ut8p_bw (data + 1);
op->type = R_ANAL_OP_TYPE_JMP;
break;
case 0x5c: 
op->eob = true;
op->jump = data[1] | data[2] << 8 | data[3] << 16;
op->type = R_ANAL_OP_TYPE_JMP;
break;
case 0x80: 
op->eob = true;
op->jump = addr + 2 + (st8)data[1];
op->type = R_ANAL_OP_TYPE_JMP;
break;
case 0x82: 
op->eob = true;
op->jump = addr + 3 + (st16)ut8p_bw (data + 1);
op->type = R_ANAL_OP_TYPE_JMP;
break;
case 0x6c: 
case 0x7c: 
case 0xdc: 
op->eob = true;
op->type = R_ANAL_OP_TYPE_UJMP;
break;
case 0x10: 
case 0x30: 
case 0x50: 
case 0x70: 
case 0x90: 
case 0xb0: 
op->eob = true;
op->jump = addr + 2 + (st8)data[1];
op->fail = addr + 2;
op->type = R_ANAL_OP_TYPE_CJMP;
break;
case 0xd0: 
op->eob = true;
op->cond = R_ANAL_COND_NE;
op->jump = addr + 2 + (st8)data[1];
op->fail = addr + 2;
op->type = R_ANAL_OP_TYPE_CJMP;
break;
case 0xf0: 
op->eob = true;
op->cond = R_ANAL_COND_EQ;
op->jump = addr + 2 + (st8)data[1];
op->fail = addr + 2;
op->type = R_ANAL_OP_TYPE_CJMP;
break;
case 0x20: 
op->jump = (addr & 0xFF0000) | ut8p_bw (data+1);
op->type = R_ANAL_OP_TYPE_CALL;
break;
case 0x22: 
op->jump = data[1] | data[2] << 8 | data[3] << 16;
op->type = R_ANAL_OP_TYPE_CALL;
break;
case 0xfc: 
op->type = R_ANAL_OP_TYPE_UCALL;
break;
case 0x40: 
case 0x60: 
case 0x6b: 
op->eob = true;
op->type = R_ANAL_OP_TYPE_RET;
break;
case 0xc2: 
if (((st8)data[1]) & 0x10) {
snesflags->X = 0;
}
if (((st8)data[1]) & 0x20) {
snesflags->M = 0;
}
break;
case 0xe2: 
if (((st8)data[1]) & 0x10) {
snesflags->X = 1;
}
if (((st8)data[1]) & 0x20) {
snesflags->M = 1;
}
break;
}
return op->size;
}

static int snes_anal_init (void* user) {
if (!snesflags) {
snesflags = malloc (sizeof (struct snes_asm_flags));
}
memset(snesflags,0,sizeof (struct snes_asm_flags));
return 0;
}

static int snes_anal_fini (void* user) {
free(snesflags);
snesflags = NULL;
return 0;
}

RAnalPlugin r_anal_plugin_snes = {
.name = "snes",
.desc = "SNES analysis plugin",
.license = "LGPL3",
.arch = "snes",
.bits = 16,
.init = snes_anal_init,
.fini = snes_anal_fini,
.op = &snes_anop,
};

#if !defined(R2_PLUGIN_INCORE)
R_API RLibStruct radare_plugin = {
.type = R_LIB_TYPE_ANAL,
.data = &r_anal_plugin_snes,
.version = R2_VERSION
};
#endif
