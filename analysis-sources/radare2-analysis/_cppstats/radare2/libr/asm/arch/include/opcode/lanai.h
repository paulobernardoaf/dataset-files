#define lanai_architecture bfd_lanai_architecture
#define architecture_pname bfd_lanai_architecture_pname
#define lanai_opcode bfd_lanai_opcode
#define lanai_opcodes bfd_lanai_opcodes
enum lanai_architecture {
v0 = 0, v1
};
extern const char *architecture_pname[];
struct lanai_opcode {
const char *name;
unsigned long match; 
unsigned long lose; 
const char *args;
unsigned int flags;
enum lanai_architecture architecture;
};
#define F_ALIAS 1 
#define F_JSR 2 
#define F_RI 0x10 
#define F_RR 0x20 
#define F_RRR 0x40 
#define F_RM 0x80
#define F_RRM 0x100
#define F_BR 0x200
#define F_SLS 0x400
#define F_SLI 0x800
#define F_SPLS 0x1000
#define F_PUNT 0x2000
#define F_SBR 0x4000
#define F_SCC 0x8000
#define F_BYTE 0x20000
#define F_HALF 0x10000
#define F_FULL 0x00000
#define F_DATA_SIZE(X) (4>>((X)&0x30000))
#define F_CONDITIONAL 0x40000
#define F_REL 0x80000
#define F_LEADZ 0x100000
#define F_POPC 0x200000
#define F_CONDBR (F_BR|F_CONDITIONAL) 
#define F_UNBR (F_BR) 
#define F_RELCONDBR (F_REL|F_BR|F_CONDITIONAL) 
#define F_RELUNBR (F_REL|F_BR) 
#define L3_USE_SI
#define L3_USE_SPLS
#define L3_USE_SLS
#define L3_USE_SLI
#define L3_USE_SBR
#define L3_T 0
#define L3_F 1
#define L3_HI 2
#define L3_LS 3
#define L3_CC 4
#define L3_CS 5
#define L3_NE 6
#define L3_EQ 7
#define L3_VC 8
#define L3_VS 9
#define L3_PL 10
#define L3_MI 11
#define L3_GE 12
#define L3_LT 13
#define L3_GT 14
#define L3_LE 15
#define L3_UGE L3_CC
#define L3_ULT L3_CS
#define L3_UGT L3_GT
#define L3_ULE L3_LE
#define L3_RI (0x00000000)
#define L3_RI_OPCODE_MASK (0x80000000)
#define L3_RI_MASK (0xf0030000)
#define L3_RR (0xc0000000)
#define L3_RR_OPCODE_MASK (0xf0000003)
#define L3_RR_MASK (0xf00207fb)
#define L3_LEADZ (0xc0000002)
#define L3_LEADZ_OPCODE_MASK L3_RR_OPCODE_MASK
#define L3_LEADZ_MASK (0xf00207fb)
#define L3_POPC (0xc0000003)
#define L3_POPC_OPCODE_MASK L3_RR_OPCODE_MASK
#define L3_POPC_MASK (0xf00207fb)
#define L3_RRR (0xd0000000)
#define L3_RRR_OPCODE_MASK (0xf0000000)
#define L3_RRR_MASK (0xf0000000)
#define L3_RM (0x80000000)
#define L3_RM_OPCODE_MASK (0xe0000000)
#define L3_RM_MASK (0xf0030000)
#define L3_RRM (0xa0000000)
#define L3_RRM_OPCODE_MASK (0xe0000000)
#define L3_RRM_MASK (0xe0030007)
#define L3_BR (0xe0000000)
#define L3_BR_OPCODE_MASK (0xf0000002)
#define L3_BR_MASK (0xfe000003)
#define L3_BRR (0xe1000002)
#define L3_BRR_OPCODE_MASK (0xf1000002)
#define L3_BRR_MASK (0xff000003)
#define L3_SCC (0xe0000002)
#define L3_SCC_OPCODE_MASK (0xf1000002)
#define L3_SCC_MASK (0xff000003)
#define L3_SLS (0xf0000000)
#define L3_SLS_OPCODE_MASK (0xf0020000)
#define L3_SLS_MASK (0xf0030000)
#define L3_SLI (0xf0020000)
#define L3_SLI_OPCODE_MASK (0xf0030000)
#define L3_SLI_MASK (0xf0030000)
#define L3_SPLS (0xf0030000)
#define L3_SPLS_OPCODE_MASK (0xf0038000)
#define L3_SPLS_MASK (0xf003fc00)
#if defined(BAD)
#define L3_SI (0xf0038000)
#define L3_SI_OPCODE_MASK ___bogus___
#define L3_SI_MASK (0xf003cf47)
#endif
#define L3_PUNT (0xf003ff47)
#define L3_PUNT_OPCODE_MASK (0xf003ff47)
#define L3_PUNT_MASK (0xf003ff47)
#define L3_SBR (0xf003c000)
#define L3_SBR_OPCODE_MASK (0xf003f806)
#define L3_SBR_MASK (0xfe03f807)
#define L3_ADD (0x00)
#define L3_ADDC (0x01)
#define L3_SUB (0x02)
#define L3_SUBB (0x03)
#define L3_AND (0x04)
#define L3_OR (0x05)
#define L3_XOR (0x06)
#define L3_SH (0x07)
#define L3_OP_MASK (0x07)
#define L3_FLAGS (0x08)
#define L3_ARITH (0x10)
#define L3_HALFWORD 0
#define L3_BYTE 4 
#define L3_FULLWORD 2
#define L3_SIGNED 0
#define L3_UNSIGNED 1 
#define L3_SIGNED_HALFWORD ( L3_SIGNED | L3_HALFWORD )
#define L3_SIGNED_BYTE ( L3_SIGNED | L3_BYTE )
#define L3_SIGNED_FULLWORD ( L3_SIGNED | L3_FULLWORD )
#define L3_UNSIGNED_HALFWORD ( L3_UNSIGNED | L3_HALFWORD )
#define L3_UNSIGNED_BYTE ( L3_UNSIGNED | L3_BYTE )
#define L3_UNSIGNED_FULLWORD ( L3_UNSIGNED | L3_FULLWORD )
#define L3_RI_F (0x00020000)
#define L3_RI_H (0x00010000)
#define L3_RR_F (0x00020000)
#define L3_LEADZ_F L3_RR_F
#define L3_POPC_F L3_RR_F
#define L3_RRR_F (0x00020000)
#define L3_RRR_H (0x00010000)
#define L3_RM_P (0x00020000)
#define L3_RM_Q (0x00010000)
#define L3_RM_S (0x10000000)
#define L3_RRM_P (0x00020000)
#define L3_RRM_Q (0x00010000)
#define L3_RRM_S (0x10000000)
#define L3_RRM_Y (0x00000004)
#define L3_RRM_L (0x00000002)
#define L3_RRM_E (0x00000001)
#define L3_BR_R (0x00000002)
#define L3_SLS_S (0x00010000)
#define L3_SPLS_Y (0x00004000)
#define L3_SPLS_S (0x00002000)
#define L3_SPLS_E (0x00001000)
#define L3_SPLS_P (0x00000800)
#define L3_SPLS_Q (0x00000400)
#define L3_SI_F (0x00002000)
#define L3_SBR_H (0x00000004)
#define L3_SBR_R (0x00000002)
#define L3_SBR_N (0x00000001)
#define L3_CONST_MASK (0x0000ffff)
#define L3_BR_CONST_MASK (0x01fffffc)
#define L3_SPLS_CONST_MASK (0x000003ff)
#define L3_RD(x) (((x)&0x1f) << 23)
#define L3_RS1(x) (((x)&0x1f) << 18)
#define L3_RS2(x) (((x)&0x1f) << 11)
#define L3_RS3(x) (((x)&0x1f) << 3)
#define L3_RI_OP(x) (((x)&L3_OP_MASK) << 28)
#define L3_RR_OP(x) (((x)&L3_OP_MASK) << 8)
#define L3_RRR_OP1(x) (((x)&L3_OP_MASK) << 0)
#define L3_RRR_OP2(x) (((x)&L3_OP_MASK) << 8)
#define L3_RRM_OP(x) (((x)&L3_OP_MASK) << 8)
#define L3_RRM_MODE(x) (((x)&0x7) << 0)
#define L3_BR_COND(x) ((((x)&0xe) << 24) | ((x)&1) )
#define L3_SBR_COND(x) ((((x)&0xe) << 24) | ((x)&1) )
#define L3_SLS_HIBITS(x) (((x)&0x1f) << 18)
#define L3_SLS_CONST(x) ((((x)&0x1f) << 18) | ((x)&0xffff))
#define L3_SLI_CONST(x) ((((x)&0x1f) << 18) | ((x)&0xffff))
#define L3_SPLS_MODE(x) (((x)&0x5) << 12)
#define L3_SBR_OP(x) (((x)&0x7) << 8)
#define L3_OP1(x) (((x)&0x7) << 0)
#define L3_OP2(x) (((x)&0x7) << 8)
#define SEX(value, bits) ((((int)(value)) << ((8 * sizeof (int)) - bits) ) >> ((8 * sizeof (int)) - bits) )
#define X_RD(i) (((i) >> 23) & 0x1f)
#define X_RS1(i) (((i) >> 18) & 0x1f)
#define X_RS2(i) (((i) >> 11) & 0x1f)
#define X_RS3(i) (((i) >> 3) & 0x1f)
#define X_OP1(i) (((i) >> 0) & 0x07)
#define X_OP2(i) (((i) >> 8) & 0x07)
#define X_RI_OP(i) (((i) >> 28) & 0x07)
#define X_RR_OP(i) X_OP2(i)
#define X_RRM_OP(i) X_OP2(i)
#define X_RRR_OP1(i) X_OP1(i)
#define X_RRR_OP2(i) X_OP2(i)
#define X_C10(i) ((i) & 0x3ff)
#define X_C16(i) ((i) & 0xffff)
#define X_C21(i) (((i) & 0xffff) | (((i) & 0x7c0000)>>2))
#define X_C25(i) ((i) & 0x1fffffc)
extern struct lanai_opcode lanai_opcodes[];
extern const int bfd_lanai_num_opcodes;
#define NUMOPCODES bfd_lanai_num_opcodes
