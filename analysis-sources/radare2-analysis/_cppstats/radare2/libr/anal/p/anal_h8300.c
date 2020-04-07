#include <string.h>
#include <r_types.h>
#include <r_lib.h>
#include <r_asm.h>
#include <r_anal.h>
#include <r_util.h>
#include <h8300_disas.h>
#define emit(frag) r_strbuf_appendf(&op->esil, frag)
#define emitf(...) r_strbuf_appendf(&op->esil, __VA_ARGS__)
#define setZ r_strbuf_appendf(&op->esil, ",$z,Z,:=") 
#define setN r_strbuf_appendf(&op->esil, ",15,$s,N,=") 
#define setV(val) r_strbuf_appendf(&op->esil, ",%s,V,=", val) 
#define setC_B r_strbuf_appendf(&op->esil, ",7,$c,C,:=") 
#define setC_W r_strbuf_appendf(&op->esil, ",15,$c,C,:=") 
#define setCb_B r_strbuf_appendf(&op->esil, ",7,$b,C,:=") 
#define setCb_W r_strbuf_appendf(&op->esil, ",15,$b,C,:=") 
#define setH_B r_strbuf_appendf(&op->esil, ",3,$c,H,:=") 
#define setH_W r_strbuf_appendf(&op->esil, ",11,$c,H,:=") 
#define setHb_B r_strbuf_appendf(&op->esil, ",3,$b,H,:=") 
#define setHb_W r_strbuf_appendf(&op->esil, ",11,$b,H,:=") 
#define rs() (buf[1]&0x70)>>4 
#define rsB() (buf[1]&0x70)>>4,buf[1]&0x80?'l':'h' 
#define rd() buf[1]&0x07
#define rdB(a) buf[a]&0x07,buf[a]&0x8?'l':'h'
#define mask() r_strbuf_appendf(&op->esil, ",0xffff,r%u,&=",rd());
#define maskB(a) r_strbuf_appendf(&op->esil, ",0xff,r%u%c,&=",rdB(a));
#define imm buf[1]
static void h8300_anal_jmp(RAnalOp *op, ut64 addr, const ut8 *buf) {
ut16 ad;
switch (buf[0]) {
case H8300_JMP_1:
op->type = R_ANAL_OP_TYPE_UJMP;
break;
case H8300_JMP_2:
op->type = R_ANAL_OP_TYPE_JMP;
r_mem_swapendian ((ut8*)&ad, buf + 2, sizeof (ut16));
op->jump = ad;
break;
case H8300_JMP_3:
op->type = R_ANAL_OP_TYPE_UJMP;
op->jump = buf[1];
break;
}
}
static void h8300_anal_jsr(RAnalOp *op, ut64 addr, const ut8 *buf) {
ut16 ad;
switch (buf[0]) {
case H8300_JSR_1:
op->type = R_ANAL_OP_TYPE_UCALL;
break;
case H8300_JSR_2:
op->type = R_ANAL_OP_TYPE_CALL;
r_mem_swapendian ((ut8*)&ad, buf + 2, sizeof (ut16));
op->jump = ad;
op->fail = addr + 4;
break;
case H8300_JSR_3:
op->type = R_ANAL_OP_TYPE_UCALL;
op->jump = buf[1];
break;
}
}
static int analop_esil(RAnal *a, RAnalOp *op, ut64 addr, const ut8 *buf) {
int ret = -1;
ut8 opcode = buf[0];
if (!op) {
return 2;
}
r_strbuf_init (&op->esil);
r_strbuf_set (&op->esil, "");
switch (opcode >> 4) {
case H8300_CMP_4BIT:
r_strbuf_appendf(&op->esil, "0x%02x,r%u%c,-", imm, rdB(0));
setV("%o");
setN;
setHb_B;
setCb_B;
maskB(0);
setZ;
return 0;
case H8300_OR_4BIT:
r_strbuf_appendf(&op->esil, "0x%02x,r%u%c,|=", imm, rdB(0));
setV("0");
setN;
maskB(0);
setZ;
return 0;
case H8300_XOR_4BIT:
r_strbuf_appendf(&op->esil, "0x%02x,r%u%c,^=", imm, rdB(0));
setN;
setV("0");
maskB(0);
setZ;
return 0;
case H8300_AND_4BIT:
r_strbuf_appendf(&op->esil, "0x%02x,r%u%c,&=", imm, rdB(0));
setN;
setV("0");
maskB(0);
setZ;
return 0;
case H8300_ADD_4BIT:
r_strbuf_appendf(&op->esil, "0x%02x,r%u%c,+=", imm, rdB(0));
setV("%o");
setN;
setH_B;
setC_B;
maskB(0);
setZ;
return 0;
case H8300_ADDX_4BIT:
r_strbuf_appendf(&op->esil, "0x%02x,C,+,r%u%c,+= ", imm,
rdB(0), rdB(0));
setV("%o");
setN;
setH_B;
setC_B;
maskB(0);
setZ;
return 0;
case H8300_SUBX_4BIT:
r_strbuf_appendf(&op->esil, "0x%02x,r%u%c,-=,C,r%u%c,-=", imm, rdB(0), rdB(0));
setV("%o");
setN;
setHb_B;
setCb_B;
maskB(0);
setZ;
return 0;
case H8300_MOV_4BIT_2: 
case H8300_MOV_4BIT_3: 
case H8300_MOV_4BIT: 
return 0;
default:
break;
};
switch (opcode) {
case H8300_NOP:
r_strbuf_set (&op->esil, ",");
return 0;
case H8300_SLEEP: 
return 0;
case H8300_STC:
r_strbuf_appendf(&op->esil, "ccr,r%u%c,=", rdB(1));
return 0;
case H8300_LDC:
r_strbuf_appendf(&op->esil, "r%u%c,ccr,=", rdB(1));
return 0;
case H8300_ORC:
r_strbuf_appendf(&op->esil, "0x%02x,ccr,|=", imm);
return 0;
case H8300_XORC:
r_strbuf_appendf(&op->esil, "0x%02x,ccr,^=", imm);
return 0;
case H8300_ANDC:
r_strbuf_appendf(&op->esil, "0x%02x,ccr,&=", imm);
return 0;
case H8300_LDC_2:
r_strbuf_appendf(&op->esil, "0x%02x,ccr,=", imm);
return 0;
case H8300_ADDB_DIRECT:
r_strbuf_appendf(&op->esil, "r%u%c,r%u%c,+=", rsB(), rdB(1));
setH_B;
setV("%o");
setC_B ;
setN;
maskB(1);
setZ;
return 0;
case H8300_ADDW_DIRECT:
r_strbuf_appendf (&op->esil, "r%u,r%u,+=", rs(), rd());
setH_W;
setV("%o");
setC_W;
setN;
mask();
setZ;
return 0;
case H8300_INC:
r_strbuf_appendf(&op->esil, "1,r%u%c,+=", rdB(1));
setV("%o") ;
setN;
maskB(1);
setZ;
return 0;
case H8300_ADDS:
r_strbuf_appendf (&op->esil, "%d,r%u,+=",
((buf[1] & 0xf0) == 0x80) ? 2 : 1, rd());
return 0;
case H8300_MOV_1:
r_strbuf_appendf (&op->esil, "r%u%c,r%u%c,=", rsB(), rdB(1));
setN;
maskB(1);
setZ;
return 0;
case H8300_MOV_2:
r_strbuf_appendf(&op->esil, "r%u,r%u,=", rs(), rd());
setN;
mask();
setZ;
return 0;
case H8300_ADDX:
r_strbuf_appendf (&op->esil, "r%u%c,C,+,r%u%c,+=",
rsB(), rdB(1), rdB(1));
setV("%o");
setN;
setH_B ;
setC_B;
maskB(1);
setZ;
return 0;
case H8300_DAA: 
return 0;
case H8300_SHL: 
return 0;
case H8300_SHR: 
return 0;
case H8300_ROTL: 
return 0;
case H8300_ROTR: 
return 0;
case H8300_OR:
r_strbuf_appendf(&op->esil, "r%u%c,r%u%c,|=", rsB(), rdB(1));
setV("0");
setN;
maskB(1);
setZ;
return 0;
case H8300_XOR:
r_strbuf_appendf(&op->esil, "r%u%c,r%u%c,^=", rsB(), rdB(1));
setV("0") ;
setN;
maskB(1);
setZ;
return 0;
case H8300_AND:
r_strbuf_appendf(&op->esil, "r%u%c,r%u%c,&=", rsB(), rdB(1));
setV("0");
setN;
maskB(1);
setZ;
return 0;
case H8300_NOT_NEG:
if ((buf[1] & 0xf0) == 0x80) { 
r_strbuf_appendf(&op->esil, "r%u%c,0,-,r%u%c,=", rdB(1), rdB(1));
setHb_B;
setV("%o") ;
setCb_B ;
setN;
maskB(1);
setZ;
} else if ((buf[1] & 0xf0) == 0x00) { 
r_strbuf_appendf(&op->esil, "r%u%c,!=", rdB(1));
setV("0");
setN;
maskB(1);
setZ;
}
return 0;
case H8300_SUB_1:
r_strbuf_appendf(&op->esil, "r%u%c,r%u%c,-=", rsB(), rdB(1));
setHb_B;
setV("%o");
setCb_B;
setN;
maskB(1);
setZ;
return 0;
case H8300_SUBW:
r_strbuf_appendf (&op->esil, "r%u,r%u,-=", rs(), rd());
setHb_W;
setV ("%o");
setCb_W;
setN;
mask();
setZ;
return 0;
case H8300_DEC:
r_strbuf_appendf (&op->esil, "1,r%u%c,-=", rdB(1));
setV("%o");
setN;
maskB(1);
setZ;
return 0;
case H8300_SUBS:
r_strbuf_appendf(&op->esil, "%d,r%u,-=",
( (buf[1] & 0xf0) == 0x80) ? 2 : 1, rd());
return 0;
case H8300_CMP_1:
r_strbuf_appendf(&op->esil, "r%u%c,r%u%c,-", rsB(), rdB(1));
setHb_B;
setV("%o");
setCb_B;
setN;
maskB(1);
setZ;
return 0;
case H8300_CMP_2:
r_strbuf_appendf(&op->esil, "r%u,r%u,-", rs(), rd());
setHb_W;
setV("%o");
setCb_W;
setN;
mask();
setZ;
return 0;
case H8300_SUBX:
r_strbuf_appendf(&op->esil, "r%u%c,r%u%c,-=,C,r%u%c,-=",
rsB(), rdB(1), rdB(1));
setHb_B;
setV("%o");
setCb_B;
setN;
maskB(1);
setZ;
return 0;
case H8300_DAS: 
return 0;
case H8300_BRA:
r_strbuf_appendf(&op->esil, "0x%02x,pc,+=", buf[1]);
return 0;
case H8300_BRN:
r_strbuf_appendf(&op->esil,",");
return 0;
case H8300_BHI:
r_strbuf_appendf(&op->esil, "C,Z,|,!,?{0x%02x,pc,+=}", buf[1]);
return 0;
case H8300_BLS:
r_strbuf_appendf(&op->esil, "C,Z,|,?{0x%02x,pc,+=}", buf[1]);
return 0;
case H8300_BCC:
r_strbuf_appendf(&op->esil, "C,!,?{0x%02x,pc,+=}", buf[1]);
return 0;
case H8300_BCS:
r_strbuf_appendf(&op->esil, "C,?{0x%02x,pc,+=}", buf[1]);
return 0;
case H8300_BNE:
r_strbuf_appendf(&op->esil, "Z,!,?{0x%02x,pc,+=}", buf[1]);
return 0;
case H8300_BEQ:
r_strbuf_appendf(&op->esil, "Z,?{0x%02x,pc,+=}", buf[1]);
return 0;
case H8300_BVC:
r_strbuf_appendf(&op->esil, "V,!,?{0x%02x,pc,+=}", buf[1]);
return 0;
case H8300_BVS:
r_strbuf_appendf(&op->esil, "V,?{0x%02x,pc,+=}", buf[1]);
return 0;
case H8300_BPL:
r_strbuf_appendf(&op->esil, "N,!,?{0x%02x,pc,+=}", buf[1]);
return 0;
case H8300_BMI:
r_strbuf_appendf(&op->esil, "N,?{0x%02x,pc,+=}", buf[1]);
return 0;
case H8300_BGE:
r_strbuf_appendf(&op->esil, "N,V,^,!,?{0x%02x,pc,+=}", buf[1]);
return 0;
case H8300_BLT:
r_strbuf_appendf(&op->esil, "N,V,^,?{0x%02x,pc,+=}", buf[1]);
return 0;
case H8300_BGT:
r_strbuf_appendf(&op->esil, "Z,N,V,^,|,!,?{0x%02x,pc,+=}", buf[1]);
return 0;
case H8300_BLE:
r_strbuf_appendf(&op->esil, "Z,N,V,^,|,?{0x%02x,pc,+=}", buf[1]);
return 0;
case H8300_MULXU:
r_strbuf_appendf(&op->esil, "r%u%c,r%ul,*,r%u,=",
rsB(), rd(), rd());
return 0;
case H8300_DIVXU: return 0;
case H8300_RTS: return 0;
case H8300_BSR: return 0;
case H8300_RTE: return 0;
case H8300_JMP_1: return 0;
case H8300_JMP_2: return 0;
case H8300_JMP_3: return 0;
case H8300_JSR_1: return 0;
case H8300_JSR_2: return 0;
case H8300_JSR_3: 
return 0;
case H8300_BSET_1: 
r_strbuf_appendf(&op->esil, "0x7,r%u%c,&,1,<<,r%u%c,|=", rsB(), rdB(1));
return 0;
case H8300_BNOT_1: 
r_strbuf_appendf(&op->esil,"0x07,r%u%c,&,1,<<,r%u%c,^=", rsB(), rdB(1));
return 0;
case H8300_BCLR_R2R8: 
r_strbuf_appendf(&op->esil, "0x7,r%u%c,&,1,<<,!,r%u%c,&=", rsB(), rdB(1));
return 0;
case H8300_BTST_R2R8: 
r_strbuf_appendf(&op->esil, "0x7,r%u%c,&,0x7,r%u%c,&,1,<<,r%u%c,&,>>,!,Z,=",
rsB(), rsB(), rdB(1));
return 0;
case H8300_BST_BIST: 
if (!(buf[1] & 0x80)) { 
r_strbuf_appendf(&op->esil,"%d,C,<<,r%u%c,|=",rs(),rdB(1));
} else { 
r_strbuf_appendf (&op->esil, "%d,C,!,<<,r%u%c,|=", rs (), rdB (1));
}
return 0;
case H8300_MOV_R82IND16: return 0;
case H8300_MOV_IND162R16: return 0;
case H8300_MOV_R82ABS16: return 0;
case H8300_MOV_ABS162R16: return 0;
case H8300_MOV_R82RDEC16: return 0;
case H8300_MOV_INDINC162R16: return 0;
case H8300_MOV_R82DISPR16: return 0;
case H8300_MOV_DISP162R16: 
return 0;
case H8300_BSET_2: 
r_strbuf_appendf(&op->esil, "%d,1,<<,r%u%c,|=", rs(), rdB(1));
return 0;
case H8300_BNOT_2: 
r_strbuf_appendf(&op->esil,"%d,1,<<,r%u%c,^=",rs(),rdB(1));
return 0;
case H8300_BCLR_IMM2R8:
r_strbuf_appendf(&op->esil, "%d,1,<<,!,r%u%c,&=", rs(), rdB(1));
return 0;
case H8300_BTST: 
r_strbuf_appendf(&op->esil, "%d,%d,1,<<,r%u%c,&,>>,!,Z,=",
rs(), rs(), rdB(1));
return 0;
case H8300_BOR_BIOR: 
if (!(buf[1] & 0x80)) { 
r_strbuf_appendf(&op->esil, "%d,%d,1,<<,r%u%c,&,>>,C,|=",
rs(), rs(), rdB(1));
} else { 
r_strbuf_appendf (&op->esil, "%d,%d,1,<<,r%u%c,&,>>,!,C,|=",
rs (), rs (), rdB (1));
}
return 0;
case H8300_BXOR_BIXOR: 
if (!(buf[1] & 0x80)) { 
r_strbuf_appendf(&op->esil, "%d,%d,1,<<,r%u%c,&,>>,C,^=",
rs(), rs(), rdB(1));
} else { 
r_strbuf_appendf (&op->esil, "%d,%d,1,<<,r%u%c,&,>>,!,C,^=",
rs (), rs (), rdB (1));
}
return 0;
case H8300_BAND_BIAND:
if (!(buf[1] & 0x80)) { 
r_strbuf_appendf(&op->esil, "%d,%d,1,<<,r%u%c,&,>>,C,&=",
rs(), rs(), rdB(1));
} else { 
r_strbuf_appendf (&op->esil, "%d,%d,1,<<,r%u%c,&,>>,!,C,&=",
rs (), rs (), rdB (1));
}
return 0;
case H8300_BILD_IMM2R8:
if (!(buf[1] & 0x80)) { 
r_strbuf_appendf(&op->esil, "%d,%d,1,<<,r%u%c,&,>>,C,=",
rs(), rs(), rdB(1));
} else { 
r_strbuf_appendf (&op->esil, "%d,%d,1,<<,r%u%c,&,>>,!,C,=",
rs (), rs (), rdB (1));
}
return 0;
case H8300_MOV_IMM162R16: return 0;
case H8300_EEPMOV: return 0;
case H8300_BIAND_IMM2IND16: return 0;
case H8300_BCLR_R2IND16: return 0;
case H8300_BIAND_IMM2ABS8: return 0;
case H8300_BCLR_R2ABS8: return 0;
default:
break;
};
return ret;
}
static int h8300_op(RAnal *anal, RAnalOp *op, ut64 addr,
const ut8 *buf, int len, RAnalOpMask mask)
{
int ret;
ut8 opcode = buf[0];
struct h8300_cmd cmd;
if (!op) {
return 2;
}
memset(op, 0, sizeof (RAnalOp));
op->addr = addr;
op->jump = op->fail = -1;
op->ptr = op->val = -1;
ret = op->size = h8300_decode_command(buf, &cmd);
if (ret < 0) {
return ret;
}
switch (opcode >> 4) {
case H8300_MOV_4BIT_2:
case H8300_MOV_4BIT_3:
case H8300_MOV_4BIT:
op->type = R_ANAL_OP_TYPE_MOV;
break;
case H8300_CMP_4BIT:
op->type = R_ANAL_OP_TYPE_CMP;
break;
case H8300_XOR_4BIT:
op->type = R_ANAL_OP_TYPE_XOR;
break;
case H8300_AND_4BIT:
op->type = R_ANAL_OP_TYPE_AND;
break;
case H8300_ADD_4BIT:
case H8300_ADDX_4BIT:
op->type = R_ANAL_OP_TYPE_AND;
break;
case H8300_SUBX_4BIT:
op->type = R_ANAL_OP_TYPE_SUB;
break;
default:
op->type = R_ANAL_OP_TYPE_UNK;
break;
};
if (op->type != R_ANAL_OP_TYPE_UNK) {
analop_esil(anal, op, addr, buf);
return ret;
}
switch (opcode) {
case H8300_MOV_R82IND16:
case H8300_MOV_IND162R16:
case H8300_MOV_R82ABS16:
case H8300_MOV_ABS162R16:
case H8300_MOV_R82RDEC16:
case H8300_MOV_INDINC162R16:
case H8300_MOV_R82DISPR16:
case H8300_MOV_DISP162R16:
case H8300_MOV_IMM162R16:
case H8300_MOV_1:
case H8300_MOV_2:
case H8300_EEPMOV:
op->type = R_ANAL_OP_TYPE_MOV;
break;
case H8300_RTS:
op->type = R_ANAL_OP_TYPE_RET;
break;
case H8300_CMP_1:
case H8300_CMP_2:
case H8300_BTST_R2R8:
case H8300_BTST:
op->type = R_ANAL_OP_TYPE_CMP;
break;
case H8300_SHL:
op->type = R_ANAL_OP_TYPE_SHL;
break;
case H8300_SHR:
op->type = R_ANAL_OP_TYPE_SHR;
break;
case H8300_XOR:
case H8300_XORC:
op->type = R_ANAL_OP_TYPE_XOR;
break;
case H8300_MULXU:
op->type = R_ANAL_OP_TYPE_MUL;
break;
case H8300_ANDC:
op->type = R_ANAL_OP_TYPE_AND;
break;
case H8300_ADDB_DIRECT:
case H8300_ADDW_DIRECT:
case H8300_ADDS:
case H8300_ADDX:
op->type = R_ANAL_OP_TYPE_ADD;
break;
case H8300_SUB_1:
case H8300_SUBW:
case H8300_SUBS:
case H8300_SUBX:
op->type = R_ANAL_OP_TYPE_SUB;
break;
case H8300_NOP:
op->type = R_ANAL_OP_TYPE_NOP;
break;
case H8300_JSR_1:
case H8300_JSR_2:
case H8300_JSR_3:
h8300_anal_jsr(op, addr, buf);
break;
case H8300_JMP_1:
case H8300_JMP_2:
case H8300_JMP_3:
h8300_anal_jmp(op, addr, buf);
break;
case H8300_BRA:
case H8300_BRN:
case H8300_BHI:
case H8300_BLS:
case H8300_BCC:
case H8300_BCS:
case H8300_BNE:
case H8300_BEQ:
case H8300_BVC:
case H8300_BVS:
case H8300_BPL:
case H8300_BMI:
case H8300_BGE:
case H8300_BLT:
case H8300_BGT:
case H8300_BLE:
op->type = R_ANAL_OP_TYPE_CJMP;
op->jump = addr + 2 + (st8)(buf[1]);
op->fail = addr + 2;
break;
default:
op->type = R_ANAL_OP_TYPE_UNK;
break;
};
if (mask & R_ANAL_OP_MASK_ESIL) {
analop_esil(anal, op, addr, buf);
}
return ret;
}
static int set_reg_profile(RAnal *anal) {
char *p =
"=PC pc\n"
"=SP r7\n"
"gpr r0 .16 0 0\n"
"gpr r0h .8 0 0\n"
"gpr r0l .8 1 0\n"
"gpr r1 .16 2 0\n"
"gpr r1h .8 2 0\n"
"gpr r1l .8 3 0\n"
"gpr r2 .16 4 0\n"
"gpr r2h .8 4 0\n"
"gpr r2l .8 5 0\n"
"gpr r3 .16 6 0\n"
"gpr r3h .8 6 0\n"
"gpr r3l .8 7 0\n"
"gpr r4 .16 8 0\n"
"gpr r4h .8 8 0\n"
"gpr r4l .8 9 0\n"
"gpr r5 .16 10 0\n"
"gpr r5h .8 10 0\n"
"gpr r5l .8 11 0\n"
"gpr r6 .16 12 0\n"
"gpr r6h .8 12 0\n"
"gpr r6l .8 13 0\n"
"gpr r7 .16 14 0\n"
"gpr r7h .8 14 0\n"
"gpr r7l .8 15 0\n"
"gpr pc .16 16 0\n"
"gpr ccr .8 18 0\n"
"gpr I .1 .151 0\n"
"gpr U1 .1 .150 0\n"
"gpr H .1 .149 0\n"
"gpr U2 .1 .148 0\n"
"gpr N .1 .147 0\n"
"gpr Z .1 .146 0\n"
"gpr V .1 .145 0\n"
"gpr C .1 .144 0\n";
return r_reg_set_profile_string (anal->reg, p);
}
RAnalPlugin r_anal_plugin_h8300 = {
.name = "h8300",
.desc = "H8300 code analysis plugin",
.license = "LGPL3",
.arch = "h8300",
.bits = 16,
.op = &h8300_op,
.esil = true,
.set_reg_profile = set_reg_profile,
};
#if !defined(R2_PLUGIN_INCORE)
struct r_lib_struct_t radare_plugin = {
.type = R_LIB_TYPE_ANAL,
.data = &r_anal_plugin_h8300,
.version = R2_VERSION
};
#endif
