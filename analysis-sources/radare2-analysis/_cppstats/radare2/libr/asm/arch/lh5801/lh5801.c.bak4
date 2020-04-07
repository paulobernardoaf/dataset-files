












#include "lh5801.h"
#include <stdio.h>
#include <string.h>
#include <r_types.h>

#define ARRAY_LENGTH(a) (sizeof(a)/sizeof((a)[0]))

const struct lh5801_insn_class_desc
lh5801_insn_class_descs[LH5801_INSNC_NUMBER] = {
[LH5801_INSNC_ADC] = { "adc", "add with carry" },
[LH5801_INSNC_ADI] = { "adi", "add immediate" },
[LH5801_INSNC_DCA] = { "dca", "decimal add" },
[LH5801_INSNC_ADR] = { "adr", "add Rreg" },
[LH5801_INSNC_SBC] = { "sbc", "subtract with carry" },
[LH5801_INSNC_SBI] = { "sbi", "subtract immediate" },
[LH5801_INSNC_DCS] = { "dcs", "decimal subtract" },
[LH5801_INSNC_AND] = { "and", "and accumulator" },
[LH5801_INSNC_ANI] = { "ani", "and immediate" },
[LH5801_INSNC_ORA] = { "ora", "or accumulator" },
[LH5801_INSNC_ORI] = { "ori", "or immediate" },
[LH5801_INSNC_EOR] = { "eor", "exclusive or" },
[LH5801_INSNC_EAI] = { "eai", "exclusive or accumulator, immediate" },
[LH5801_INSNC_INC] = { "inc", "increment" },
[LH5801_INSNC_DEC] = { "dec", "decrement" },
[LH5801_INSNC_CPA] = { "cpa", "compare accumulator" },
[LH5801_INSNC_CPI] = { "cpi", "compare immediate" },
[LH5801_INSNC_BIT] = { "bit", "bit test" },
[LH5801_INSNC_BII] = { "bii", "bit test immediate" },
[LH5801_INSNC_LDA] = { "lda", "load accumulator" },
[LH5801_INSNC_LDE] = { "lde", "load and decrement" },
[LH5801_INSNC_LIN] = { "lin", "load and increment" },
[LH5801_INSNC_LDI] = { "ldi", "load immediate" },
[LH5801_INSNC_LDX] = { "ldx", "load Xreg" },
[LH5801_INSNC_STA] = { "sta", "store accumulator" },
[LH5801_INSNC_SDE] = { "sde", "store and decrement" },
[LH5801_INSNC_SIN] = { "sin", "store and increment" },
[LH5801_INSNC_STX] = { "stx", "store Xreg" },
[LH5801_INSNC_PSH] = { "psh", "push" },
[LH5801_INSNC_POP] = { "pop", "pop" },
[LH5801_INSNC_ATT] = { "att", "accumulator to t (status register)" },
[LH5801_INSNC_TTA] = { "tta", "t (status register) to accumulator" },
[LH5801_INSNC_TIN] = { "tin", "transfer and increment" },
[LH5801_INSNC_CIN] = { "cin", "compare and increment" },
[LH5801_INSNC_ROL] = { "rol", "rotate left" },
[LH5801_INSNC_ROR] = { "ror", "rotate right" },
[LH5801_INSNC_SHL] = { "shl", "shift left" },
[LH5801_INSNC_SHR] = { "shr", "shift right" },
[LH5801_INSNC_DRL] = { "drl", "digit rotate left" },
[LH5801_INSNC_DRR] = { "drr", "digit rotate right" },
[LH5801_INSNC_AEX] = { "aex", "exchange accumulator" },
[LH5801_INSNC_SEC] = { "sec", "set carry flag" },
[LH5801_INSNC_REC] = { "rec", "reset carry flag" },
[LH5801_INSNC_CDV] = { "cdv", "clear divider" },
[LH5801_INSNC_ATP] = { "atp", "accumulator to port" },
[LH5801_INSNC_SPU] = { "spu", "set PU" },
[LH5801_INSNC_RPU] = { "rpu", "reset PU" },
[LH5801_INSNC_SPV] = { "spv", "set PV" },
[LH5801_INSNC_RPV] = { "rpv", "reset PV" },
[LH5801_INSNC_SDP] = { "sdp", "set display" },
[LH5801_INSNC_RDP] = { "rdp", "reset display" },
[LH5801_INSNC_ITA] = { "ita", "IN to accumulator" },
[LH5801_INSNC_SIE] = { "sie", "set interrupt enable" },
[LH5801_INSNC_RIE] = { "rie", "reset interrupt enable" },
[LH5801_INSNC_AM0] = { "am0", "accumulator to tm and 0" },
[LH5801_INSNC_AM1] = { "am1", "accumulator to tm and 1" },
[LH5801_INSNC_NOP] = { "nop", "no operation" },
[LH5801_INSNC_HLT] = { "hlt", "halt" },
[LH5801_INSNC_OFF] = { "off", "\"off\", reset BF" },
[LH5801_INSNC_JMP] = { "jmp", "jump" },
[LH5801_INSNC_BCH] = { "bch", "unconditional branch" },
[LH5801_INSNC_BCC] = { "bcc", "conditional branch" },
[LH5801_INSNC_LOP] = { "lop", "loop" },
[LH5801_INSNC_SJP] = { "sjp", "subroutine jump (aka. call)" },
[LH5801_INSNC_VEJ] = { "vej", "vector subroutine jump, short format" },
[LH5801_INSNC_VMJ] = { "vmj", "vector subroutine jump, long format" },
[LH5801_INSNC_VCC] = { "vcc", "conditional vector subroutine jump" },
[LH5801_INSNC_RTN] = { "rtn", "return from subroutine" },
[LH5801_INSNC_RTI] = { "rti", "return from interrupt" }
};



enum lh5801_insn_format {

LH5801_IFMT_IMM0 = 0,
LH5801_IFMT_IMM1,
LH5801_IFMT_IMM2,
LH5801_IFMT_IMM3,
LH5801_IFMT_IMM_MASK = 3,



LH5801_IFMT_FD_NO = 0,
LH5801_IFMT_FD_YES = 1 << 2,
LH5801_IFMT_FD_MOD = 2 << 2, 
LH5801_IFMT_FD_MASK = 3 << 2, 


LH5801_IFMT_RREG = 1 << 4, 
LH5801_IFMT_AREG = 2 << 4, 
LH5801_IFMT_SREG = 3 << 4, 
LH5801_IFMT_PREG = 4 << 4, 
LH5801_IFMT_REG_MASK = 7 << 4,



LH5801_IFMT_COND = 1 << 7,


LH5801_IFMT_BCH = 1 << 8,


LH5801_IFMT_VEJ = 1 << 9,


LH5801_IFMT_RFULL = 0,
LH5801_IFMT_RLOW = 1 << 10,
LH5801_IFMT_RHIGH = 2 << 10,
LH5801_IFMT_RMEM = 3 << 10, 
LH5801_IFMT_RMODE_MASK = 3 << 10,
};

#define LH5801_IFMT_IMMS(f) ((f)&LH5801_IFMT_IMM_MASK)
#define LH5801_IFMT_RMODE(f) ((f)&LH5801_IFMT_RMODE_MASK)

static bool lh5801_ifmt_fd_matches(enum lh5801_insn_format fmt, int fd) {
switch (fmt & LH5801_IFMT_FD_MASK) {
case LH5801_IFMT_FD_NO: return !fd;
case LH5801_IFMT_FD_YES: return fd;
case LH5801_IFMT_FD_MOD: return true;
default: return false;
}
}


struct lh5801_insn_desc {
ut8 iclass; 


ut8 opcode;

ut16 format; 
};

const struct lh5801_insn_desc lh5801_insn_descs[] = {
{ 
.iclass = LH5801_INSNC_ADC,
.format = LH5801_IFMT_RREG|LH5801_IFMT_RLOW,
.opcode = 0x02,
},
{ 
.iclass = LH5801_INSNC_ADC,
.format = LH5801_IFMT_RREG|LH5801_IFMT_RHIGH,
.opcode = 0x82,
},
{ 
.iclass = LH5801_INSNC_ADC,
.format = LH5801_IFMT_RREG|LH5801_IFMT_RMEM|LH5801_IFMT_FD_MOD,
.opcode = 0x03,
},
{ 
.iclass = LH5801_INSNC_ADC,
.format = LH5801_IFMT_IMM2|LH5801_IFMT_FD_MOD,
.opcode = 0xa3,
},
{ 
.iclass = LH5801_INSNC_ADI,
.format = LH5801_IFMT_IMM1|LH5801_IFMT_AREG,
.opcode = 0xb3
},
{ 
.iclass = LH5801_INSNC_ADI,
.format = LH5801_IFMT_IMM1|LH5801_IFMT_RREG|LH5801_IFMT_FD_MOD|LH5801_IFMT_RMEM,
.opcode = 0x4f,
},
{ 
.iclass = LH5801_INSNC_ADI,
.format = LH5801_IFMT_IMM3|LH5801_IFMT_FD_MOD,
.opcode = 0xef
},
{ 
.iclass = LH5801_INSNC_DCA,
.format = LH5801_IFMT_RREG|LH5801_IFMT_RMEM|LH5801_IFMT_FD_MOD,
.opcode = 0x8c,
},
{ 
.iclass = LH5801_INSNC_ADR,
.format = LH5801_IFMT_RREG|LH5801_IFMT_FD_YES,
.opcode = 0xca,
},
{ 
.iclass = LH5801_INSNC_SBC,
.format = LH5801_IFMT_RREG|LH5801_IFMT_RLOW,
.opcode = 0x00
},
{ 
.iclass = LH5801_INSNC_SBC,
.format = LH5801_IFMT_RREG|LH5801_IFMT_RHIGH,
.opcode = 0x80
},
{ 
.iclass = LH5801_INSNC_SBC,
.format = LH5801_IFMT_RREG|LH5801_IFMT_RMEM|LH5801_IFMT_FD_MOD,
.opcode = 0x01
},
{ 
.iclass = LH5801_INSNC_SBC,
.format = LH5801_IFMT_IMM2|LH5801_IFMT_FD_MOD,
.opcode = 0xa1
},
{ 
.iclass = LH5801_INSNC_SBI,
.format = LH5801_IFMT_AREG|LH5801_IFMT_IMM1,
.opcode = 0xb1
},
{ 
.iclass = LH5801_INSNC_DCS,
.format = LH5801_IFMT_RREG|LH5801_IFMT_RMEM|LH5801_IFMT_FD_MOD,
.opcode = 0x0c
},
{ 
.iclass = LH5801_INSNC_AND,
.format = LH5801_IFMT_RREG|LH5801_IFMT_RMEM|LH5801_IFMT_FD_MOD,
.opcode = 0x09
},
{ 
.iclass = LH5801_INSNC_AND,
.format = LH5801_IFMT_IMM2|LH5801_IFMT_FD_MOD,
.opcode = 0xa9
},
{ 
.iclass = LH5801_INSNC_ANI,
.format = LH5801_IFMT_IMM1|LH5801_IFMT_AREG,
.opcode = 0xb9
},
{ 
.iclass = LH5801_INSNC_ANI,
.format = LH5801_IFMT_IMM1|LH5801_IFMT_RREG|LH5801_IFMT_RMEM|LH5801_IFMT_FD_MOD,
.opcode = 0x49
},
{ 
.iclass = LH5801_INSNC_ANI,
.format = LH5801_IFMT_IMM3|LH5801_IFMT_FD_MOD,
.opcode = 0xe9
},
{ 
.iclass = LH5801_INSNC_ORA,
.format = LH5801_IFMT_RREG|LH5801_IFMT_RREG|LH5801_IFMT_RMEM|LH5801_IFMT_FD_MOD,
.opcode = 0x0b
},
{ 
.iclass = LH5801_INSNC_ORA,
.format = LH5801_IFMT_IMM2|LH5801_IFMT_FD_MOD,
.opcode = 0xab
},
{ 
.iclass = LH5801_INSNC_ORI,
.format = LH5801_IFMT_AREG|LH5801_IFMT_IMM1,
.opcode = 0xbb
},
{ 
.iclass = LH5801_INSNC_ORI,
.format = LH5801_IFMT_RREG|LH5801_IFMT_IMM1|LH5801_IFMT_RMEM|LH5801_IFMT_FD_MOD,
.opcode = 0x4b
},
{ 
.iclass = LH5801_INSNC_ORI,
.format = LH5801_IFMT_IMM3|LH5801_IFMT_FD_MOD,
.opcode = 0xeb
},
{ 
.iclass = LH5801_INSNC_EOR,
.format = LH5801_IFMT_RREG|LH5801_IFMT_RMEM|LH5801_IFMT_FD_MOD,
.opcode = 0x0d
},
{ 
.iclass = LH5801_INSNC_EOR,
.format = LH5801_IFMT_IMM2|LH5801_IFMT_FD_MOD,
.opcode = 0xad
},
{ 
.iclass = LH5801_INSNC_EAI,
.format = LH5801_IFMT_IMM1,
.opcode = 0xbd
},
{ 
.iclass = LH5801_INSNC_INC,
.format = LH5801_IFMT_AREG,
.opcode = 0xdd,
},
{ 
.iclass = LH5801_INSNC_INC,
.format = LH5801_IFMT_RREG|LH5801_IFMT_RLOW,
.opcode = 0x40,
},
{ 
.iclass = LH5801_INSNC_INC,
.format = LH5801_IFMT_RREG|LH5801_IFMT_RHIGH|LH5801_IFMT_FD_YES,
.opcode = 0x40,
},
{ 
.iclass = LH5801_INSNC_INC,
.format = LH5801_IFMT_RREG,
.opcode = 0x44
},
{ 
.iclass = LH5801_INSNC_DEC,
.format = LH5801_IFMT_AREG,
.opcode = 0xdf,
},
{ 
.iclass = LH5801_INSNC_DEC,
.format = LH5801_IFMT_RREG|LH5801_IFMT_RLOW,
.opcode = 0x42,
},
{ 
.iclass = LH5801_INSNC_DEC,
.format = LH5801_IFMT_RREG|LH5801_IFMT_RHIGH|LH5801_IFMT_FD_YES,
.opcode = 0x42,
},
{ 
.iclass = LH5801_INSNC_DEC,
.format = LH5801_IFMT_RREG,
.opcode = 0x46
},
{ 
.iclass = LH5801_INSNC_CPA,
.format = LH5801_IFMT_RREG|LH5801_IFMT_RLOW,
.opcode = 0x06
},
{ 
.iclass = LH5801_INSNC_CPA,
.format = LH5801_IFMT_RREG|LH5801_IFMT_RHIGH,
.opcode = 0x86
},
{ 
.iclass = LH5801_INSNC_CPA,
.format = LH5801_IFMT_RREG|LH5801_IFMT_RMEM|LH5801_IFMT_FD_MOD,
.opcode = 0x07
},
{ 
.iclass = LH5801_INSNC_CPA,
.format = LH5801_IFMT_IMM2|LH5801_IFMT_FD_MOD,
.opcode = 0xa7
},
{ 
.iclass = LH5801_INSNC_CPI,
.format = LH5801_IFMT_RREG|LH5801_IFMT_RLOW|LH5801_IFMT_IMM1,
.opcode = 0x4e
},
{ 
.iclass = LH5801_INSNC_CPI,
.format = LH5801_IFMT_RREG|LH5801_IFMT_RHIGH|LH5801_IFMT_IMM1,
.opcode = 0x4c
},
{ 
.iclass = LH5801_INSNC_CPI,
.format = LH5801_IFMT_AREG|LH5801_IFMT_IMM1,
.opcode = 0xb7
},
{ 
.iclass = LH5801_INSNC_BIT,
.format = LH5801_IFMT_RREG|LH5801_IFMT_RMEM|LH5801_IFMT_FD_MOD,
.opcode = 0x0f
},
{ 
.iclass = LH5801_INSNC_BIT,
.format = LH5801_IFMT_IMM2|LH5801_IFMT_FD_MOD,
.opcode = 0xaf
},
{ 
.iclass = LH5801_INSNC_BII,
.format = LH5801_IFMT_AREG|LH5801_IFMT_IMM1,
.opcode = 0xbf
},
{ 
.iclass = LH5801_INSNC_BII,
.format = LH5801_IFMT_RREG|LH5801_IFMT_IMM1|LH5801_IFMT_RMEM|LH5801_IFMT_FD_MOD,
.opcode = 0x4d
},
{ 
.iclass = LH5801_INSNC_BII,
.format = LH5801_IFMT_IMM3|LH5801_IFMT_FD_MOD,
.opcode = 0xed
},
{ 
.iclass = LH5801_INSNC_LDA,
.format = LH5801_IFMT_RREG|LH5801_IFMT_RLOW,
.opcode = 0x04,
},
{ 
.iclass = LH5801_INSNC_LDA,
.format = LH5801_IFMT_RREG|LH5801_IFMT_RHIGH,
.opcode = 0x84,
},
{ 
.iclass = LH5801_INSNC_LDA,
.format = LH5801_IFMT_RREG|LH5801_IFMT_RMEM|LH5801_IFMT_FD_MOD,
.opcode = 0x05,
},
{ 
.iclass = LH5801_INSNC_LDA,
.format = LH5801_IFMT_IMM2|LH5801_IFMT_FD_MOD,
.opcode = 0xa5,
},
{ 
.iclass = LH5801_INSNC_LDE,
.format = LH5801_IFMT_RREG,
.opcode = 0x47,
},
{ 
.iclass = LH5801_INSNC_LIN,
.format = LH5801_IFMT_RREG,
.opcode = 0x45,
},
{ 
.iclass = LH5801_INSNC_LDI,
.format = LH5801_IFMT_RREG|LH5801_IFMT_RLOW|LH5801_IFMT_IMM1,
.opcode = 0x4a,
},
{ 
.iclass = LH5801_INSNC_LDI,
.format = LH5801_IFMT_RREG|LH5801_IFMT_RHIGH|LH5801_IFMT_IMM1,
.opcode = 0x48,
},
{ 
.iclass = LH5801_INSNC_LDI,
.format = LH5801_IFMT_AREG|LH5801_IFMT_IMM1,
.opcode = 0xb5
},
{ 
.iclass = LH5801_INSNC_LDI,
.format = LH5801_IFMT_SREG|LH5801_IFMT_IMM2,
.opcode = 0xaa
},
{ 
.iclass = LH5801_INSNC_LDX,
.format = LH5801_IFMT_FD_YES|LH5801_IFMT_RREG,
.opcode = 0x08,
},
{ 
.iclass = LH5801_INSNC_LDX,
.format = LH5801_IFMT_FD_YES|LH5801_IFMT_SREG,
.opcode = 0x48,
},
{ 
.iclass = LH5801_INSNC_LDX,
.format = LH5801_IFMT_FD_YES|LH5801_IFMT_PREG,
.opcode = 0x58
},
{ 
.iclass = LH5801_INSNC_STA,
.format = LH5801_IFMT_RREG|LH5801_IFMT_RLOW,
.opcode = 0x0a
},
{ 
.iclass = LH5801_INSNC_STA,
.format = LH5801_IFMT_RREG|LH5801_IFMT_RHIGH,
.opcode = 0x08
},
{ 
.iclass = LH5801_INSNC_STA,
.format = LH5801_IFMT_RREG|LH5801_IFMT_RMEM|LH5801_IFMT_FD_MOD,
.opcode = 0x0e
},
{ 
.iclass = LH5801_INSNC_STA,
.format = LH5801_IFMT_IMM2|LH5801_IFMT_FD_MOD,
.opcode = 0xae
},
{ 
.iclass = LH5801_INSNC_SDE,
.format = LH5801_IFMT_RREG,
.opcode = 0x43
},
{ 
.iclass = LH5801_INSNC_SIN,
.format = LH5801_IFMT_RREG,
.opcode = 0x41
},
{ 
.iclass = LH5801_INSNC_STX,
.format = LH5801_IFMT_FD_YES|LH5801_IFMT_RREG,
.opcode = 0x4a
},
{ 
.iclass = LH5801_INSNC_STX,
.format = LH5801_IFMT_FD_YES|LH5801_IFMT_SREG,
.opcode = 0x4e
},
{ 
.iclass = LH5801_INSNC_STX,
.format = LH5801_IFMT_FD_YES|LH5801_IFMT_PREG,
.opcode = 0x5e
},
{ 
.iclass = LH5801_INSNC_PSH,
.format = LH5801_IFMT_FD_YES|LH5801_IFMT_AREG,
.opcode = 0xc8
},
{ 
.iclass = LH5801_INSNC_PSH,
.format = LH5801_IFMT_FD_YES|LH5801_IFMT_RREG,
.opcode = 0x88
},
{ 
.iclass = LH5801_INSNC_POP,
.format = LH5801_IFMT_FD_YES|LH5801_IFMT_AREG,
.opcode = 0x8a
},
{ 
.iclass = LH5801_INSNC_POP,
.format = LH5801_IFMT_FD_YES|LH5801_IFMT_RREG,
.opcode = 0x0a
},
{ 
.iclass = LH5801_INSNC_ATT,
.format = LH5801_IFMT_FD_YES,
.opcode = 0xec
},
{ 
.iclass = LH5801_INSNC_TTA,
.format = LH5801_IFMT_FD_YES,
.opcode = 0xaa
},
{ 
.iclass = LH5801_INSNC_TIN,
.format = 0,
.opcode = 0xf5
},
{ 
.iclass = LH5801_INSNC_CIN,
.format = 0,
.opcode = 0xf7
},
{ 





.iclass = LH5801_INSNC_ROL,
.format = 0,
.opcode = 0xdd
},
{ 
.iclass = LH5801_INSNC_ROR,
.format = 0,
.opcode = 0xd1
},
{ 
.iclass = LH5801_INSNC_SHL,
.format = 0,
.opcode = 0xd9
},
{ 
.iclass = LH5801_INSNC_SHR,
.format = 0,
.opcode = 0xd5
},
{ 
.iclass = LH5801_INSNC_DRL,
.format = 0, 
.opcode = 0xd7
},
{ 
.iclass = LH5801_INSNC_DRR,
.format = 0, 
.opcode = 0xd3
},
{ 
.iclass = LH5801_INSNC_AEX,
.format = 0,
.opcode = 0xf1
},
{ 
.iclass = LH5801_INSNC_AM0,
.format = LH5801_IFMT_FD_YES,
.opcode = 0xce
},
{ 
.iclass = LH5801_INSNC_AM1,
.format = LH5801_IFMT_FD_YES,
.opcode = 0xde
},
{ 
.iclass = LH5801_INSNC_CDV,
.format = LH5801_IFMT_FD_YES,
.opcode = 0x8e
},
{ 
.iclass = LH5801_INSNC_ATP,
.format = LH5801_IFMT_FD_YES,
.opcode = 0xcc
},
{ 
.iclass = LH5801_INSNC_SDP,
.format = LH5801_IFMT_FD_YES,
.opcode = 0xc1
},
{ 
.iclass = LH5801_INSNC_RDP,
.format = LH5801_IFMT_FD_YES,
.opcode = 0xc0
},
{ 
.iclass = LH5801_INSNC_SPU,
.format = 0,
.opcode = 0xe1
},
{ 
.iclass = LH5801_INSNC_RPU,
.format = 0,
.opcode = 0xe3
},
{ 
.iclass = LH5801_INSNC_SPV,
.format = 0,
.opcode = 0xa8
},
{ 
.iclass = LH5801_INSNC_RPV,
.format = 0,
.opcode = 0xb8
},
{ 
.iclass = LH5801_INSNC_ITA,
.format = LH5801_IFMT_FD_YES,
.opcode = 0xba
},
{ 
.iclass = LH5801_INSNC_RIE,
.format = LH5801_IFMT_FD_YES,
.opcode = 0xbe
},
{ 
.iclass = LH5801_INSNC_SIE,
.format = LH5801_IFMT_FD_YES,
.opcode = 0x81
},
{ 
.iclass = LH5801_INSNC_HLT,
.format = LH5801_IFMT_FD_YES,
.opcode = 0xb1
},
{ 
.iclass = LH5801_INSNC_OFF,
.format = LH5801_IFMT_FD_YES,
.opcode = 0x4c
},
{ 
.iclass = LH5801_INSNC_NOP,
.format = 0,
.opcode = 0x38
},
{ 
.iclass = LH5801_INSNC_SEC,
.format = 0,
.opcode = 0xfb
},
{ 
.iclass = LH5801_INSNC_REC,
.format = 0,
.opcode = 0xf9
},
{ 
.iclass = LH5801_INSNC_JMP,
.format = LH5801_IFMT_IMM2,
.opcode = 0xba
},
{ 
.iclass = LH5801_INSNC_BCH,
.format = LH5801_IFMT_BCH|LH5801_IFMT_IMM1,
.opcode = 0x8e
},
{ 
.iclass = LH5801_INSNC_BCC,
.format = LH5801_IFMT_BCH|LH5801_IFMT_COND|LH5801_IFMT_IMM1,
.opcode = 0x81
},
{ 
.iclass = LH5801_INSNC_LOP,
.format = LH5801_IFMT_IMM1,
.opcode = 0x88
},
{ 
.iclass = LH5801_INSNC_SJP,
.format = LH5801_IFMT_IMM2,
.opcode = 0xbe,
},
{ 
.iclass = LH5801_INSNC_VEJ,
.format = LH5801_IFMT_VEJ,
.opcode = 0xc0
},
{ 
.iclass = LH5801_INSNC_VCC,
.format = LH5801_IFMT_IMM1|LH5801_IFMT_COND,
.opcode = 0xc1
},
{ 







.iclass = LH5801_INSNC_VMJ,
.format = LH5801_IFMT_IMM1,
.opcode = 0xcd
},
{ 
.iclass = LH5801_INSNC_RTN,
.format = 0,
.opcode = 0x9a
},
{ 
.iclass = LH5801_INSNC_RTI,
.format = 0,
.opcode = 0x8a
}
};





int lh5801_decode(struct lh5801_insn *insn, const ut8 *buf, int len) {
int fd = (buf[0] == 0xfd);
int type = -1;
unsigned i;
struct lh5801_insn_desc desc;

if (fd) {
buf++;
len--;
}

if (len == 0)
return 0;


for (i = 0; i < ARRAY_LENGTH(lh5801_insn_descs); i++) {
ut8 byte = *buf;
unsigned fmt;
unsigned ifmt_reg;

desc = lh5801_insn_descs[i];
fmt = desc.format;
ifmt_reg = fmt & LH5801_IFMT_REG_MASK;

if(!lh5801_ifmt_fd_matches(fmt, fd))
continue;


if (ifmt_reg == LH5801_IFMT_RREG && (byte >> 4) % 4 == 3)
continue;


if (ifmt_reg == LH5801_IFMT_RREG)
byte &= 0xcf; 
if (fmt & LH5801_IFMT_COND)
byte &= 0xf1; 
if (fmt & LH5801_IFMT_BCH)
byte &= 0xef; 

if (byte == desc.opcode) {
type = i;
break;
}



if (fmt & LH5801_IFMT_VEJ) {
if (!(byte & 1) && byte >= 0xc0 && byte <= 0xf6) {
type = i;
break;
}
}
}
if (type == -1)
return -1;


insn->iclass = desc.iclass;
insn->type = type;
insn->fd = fd;
insn->opcode = buf[0];
switch (LH5801_IFMT_IMMS(desc.format)) {
case 3: insn->imm[2] = buf[3];
case 2: insn->imm[1] = buf[2];
case 1: insn->imm[0] = buf[1];
}


return fd + 1 + LH5801_IFMT_IMMS(desc.format);
}



static char *print_reg(char *buf, const struct lh5801_insn *insn) {
const struct lh5801_insn_desc desc = lh5801_insn_descs[insn->type];
unsigned regnr = (insn->opcode >> 4) & 3;
const char names[] = "xyu";
char *saved_buf = buf;


switch(desc.format & LH5801_IFMT_REG_MASK) {
case LH5801_IFMT_AREG: return "a";
case LH5801_IFMT_SREG: return "s";
case LH5801_IFMT_PREG: return "p";
}

if (regnr == 3)
return "invalid";
else switch (LH5801_IFMT_RMODE(desc.format)) {
case LH5801_IFMT_RFULL:
buf[0] = names[regnr];
buf[1] = '\0';
break;
case LH5801_IFMT_RLOW:
case LH5801_IFMT_RHIGH:
buf[0] = names[regnr];
buf[1] = (desc.format & LH5801_IFMT_RLOW)? 'l':'h';
buf[2] = '\0';
break;
case LH5801_IFMT_RMEM:
if (desc.format & LH5801_IFMT_FD_MOD) {
if (insn->fd)
*(buf++) = '#';
buf[0] = '(';
buf[1] = names[regnr];
buf[2] = ')';
buf[3] = '\0';
} else {
return NULL;
}
break;
default:
return NULL;
}
return saved_buf;
}

void lh5801_print_insn(char *out, int size, const struct lh5801_insn *insn) {
const struct lh5801_insn_class_desc *iclass =
&lh5801_insn_class_descs[insn->iclass];
const struct lh5801_insn_desc desc = lh5801_insn_descs[insn->type];
const char *mnem = iclass->mnem;
char mnembuf[4];
char regbuf[8];


if (desc.format & LH5801_IFMT_COND) {
mnembuf[0] = mnem[0]; 
mnembuf[1] = "chzv"[(insn->opcode >> 2) % 4]; 
mnembuf[2] = (insn->opcode & 2)? 's':'r'; 
mnembuf[3] = '\0';
mnem = mnembuf;
}
















switch (desc.format & ~LH5801_IFMT_RMODE_MASK & ~LH5801_IFMT_COND
& ~LH5801_IFMT_FD_MASK) {
case LH5801_IFMT_VEJ:
snprintf(out, size, "%s %02xh", mnem, insn->opcode);
break;
case LH5801_IFMT_IMM0:
snprintf(out, size, "%s", mnem);
break;
case LH5801_IFMT_IMM0|LH5801_IFMT_RREG:
case LH5801_IFMT_IMM0|LH5801_IFMT_AREG:
case LH5801_IFMT_IMM0|LH5801_IFMT_SREG:
case LH5801_IFMT_IMM0|LH5801_IFMT_PREG:
snprintf(out, size, "%s %s", mnem, print_reg(regbuf, insn));
break;
case LH5801_IFMT_IMM1:
snprintf(out, size, "%s %02xh", mnem, insn->imm[0]);
break;
case LH5801_IFMT_IMM1|LH5801_IFMT_RREG:
case LH5801_IFMT_IMM1|LH5801_IFMT_AREG:
case LH5801_IFMT_IMM1|LH5801_IFMT_SREG:
case LH5801_IFMT_IMM1|LH5801_IFMT_PREG:
snprintf(out, size, "%s %s, %02xh", mnem,
print_reg(regbuf, insn), insn->imm[0]);
break;
case LH5801_IFMT_IMM1|LH5801_IFMT_BCH:
snprintf(out, size, "%s %c%02xh", mnem,
(insn->opcode & 0x10)? '-':'+', insn->imm[0]);
break;
case LH5801_IFMT_IMM2:
if (desc.format & LH5801_IFMT_FD_MOD) {
snprintf(out, size, "%s %s(%02x%02xh)", mnem,
insn->fd? "#":"",
insn->imm[0], insn->imm[1]);
} else {
snprintf(out, size, "%s %02x%02xh", mnem,
insn->imm[0], insn->imm[1]);
}
break;
case LH5801_IFMT_IMM3:
if (desc.format & LH5801_IFMT_FD_MOD) {
snprintf(out, size, "%s %s(%02x%02xh), %02xh", mnem,
insn->fd? "#":"",
insn->imm[0], insn->imm[1], insn->imm[2]);
} else {
snprintf(out,size, "imm3 invalid format");
}
break;
default:
snprintf(out, size, "%s, BUG: unknown format 0x%x -> 0x%x",
mnem, desc.format,
desc.format & ~LH5801_IFMT_RMODE_MASK &
~LH5801_IFMT_COND & ~LH5801_IFMT_FD_MASK);
}
}
