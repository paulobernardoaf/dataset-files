





















#if !defined(_NIOS2_H_)
#define _NIOS2_H_

#include "mybfd.h"








enum overflow_type
{
call_target_overflow = 0,
branch_target_overflow,
address_offset_overflow,
signed_immed16_overflow,
unsigned_immed16_overflow,
unsigned_immed5_overflow,
custom_opcode_overflow,
no_overflow
};
































struct nios2_opcode
{
const char *name; 
const char *args; 

const char *args_test; 

unsigned long num_args; 

unsigned long match; 
unsigned long mask; 

unsigned long pinfo; 

enum overflow_type overflow_msg; 

};





#define NIOS2_INSN_MACRO 0x80000000
#define NIOS2_INSN_MACRO_MOV 0x80000001
#define NIOS2_INSN_MACRO_MOVI 0x80000002
#define NIOS2_INSN_MACRO_MOVIA 0x80000004

#define NIOS2_INSN_RELAXABLE 0x40000000
#define NIOS2_INSN_UBRANCH 0x00000010
#define NIOS2_INSN_CBRANCH 0x00000020
#define NIOS2_INSN_CALL 0x00000040

#define NIOS2_INSN_ADDI 0x00000080
#define NIOS2_INSN_ANDI 0x00000100
#define NIOS2_INSN_ORI 0x00000200
#define NIOS2_INSN_XORI 0x00000400



struct nios2_reg
{
const char *name;
const int index;
};






#define GET_INSN_FIELD(X, i) (((i) & OP_MASK_##X) >> OP_SH_##X)

#define SET_INSN_FIELD(X, i, j) ((i) = (((i) & ~OP_MASK_##X) | (((j) << OP_SH_##X) & OP_MASK_##X)))



#define IW_A_LSB 27
#define IW_A_MSB 31
#define IW_A_SZ 5
#define IW_A_MASK 0x1f

#define IW_B_LSB 22
#define IW_B_MSB 26
#define IW_B_SZ 5
#define IW_B_MASK 0x1f

#define IW_C_LSB 17
#define IW_C_MSB 21
#define IW_C_SZ 5
#define IW_C_MASK 0x1f

#define IW_IMM16_LSB 6
#define IW_IMM16_MSB 21
#define IW_IMM16_SZ 16
#define IW_IMM16_MASK 0xffff

#define IW_IMM26_LSB 6
#define IW_IMM26_MSB 31
#define IW_IMM26_SZ 26
#define IW_IMM26_MASK 0x3ffffff

#define IW_OP_LSB 0
#define IW_OP_MSB 5
#define IW_OP_SZ 6
#define IW_OP_MASK 0x3f

#define IW_OPX_LSB 11
#define IW_OPX_MSB 16
#define IW_OPX_SZ 6
#define IW_OPX_MASK 0x3f

#define IW_SHIFT_IMM5_LSB 6
#define IW_SHIFT_IMM5_MSB 10
#define IW_SHIFT_IMM5_SZ 5
#define IW_SHIFT_IMM5_MASK 0x1f

#define IW_CONTROL_REGNUM_LSB 6
#define IW_CONTROL_REGNUM_MSB 9
#define IW_CONTROL_REGNUM_SZ 4
#define IW_CONTROL_REGNUM_MASK 0xf


#define OP_MASK_OP (IW_OP_MASK << IW_OP_LSB)
#define OP_SH_OP IW_OP_LSB


#define OP_MASK_IOP (IW_OP_MASK << IW_OP_LSB)
#define OP_SH_IOP IW_OP_LSB

#define OP_MASK_IMM16 (IW_IMM16_MASK << IW_IMM16_LSB)
#define OP_SH_IMM16 IW_IMM16_LSB

#define OP_MASK_IRD (IW_B_MASK << IW_B_LSB)
#define OP_SH_IRD IW_B_LSB 

#define OP_MASK_IRT (IW_B_MASK << IW_B_LSB)
#define OP_SH_IRT IW_B_LSB

#define OP_MASK_IRS (IW_A_MASK << IW_A_LSB)
#define OP_SH_IRS IW_A_LSB


#define OP_MASK_ROP (IW_OP_MASK << IW_OP_LSB)
#define OP_SH_ROP IW_OP_LSB

#define OP_MASK_ROPX (IW_OPX_MASK << IW_OPX_LSB)
#define OP_SH_ROPX IW_OPX_LSB

#define OP_MASK_RRD (IW_C_MASK << IW_C_LSB)
#define OP_SH_RRD IW_C_LSB

#define OP_MASK_RRT (IW_B_MASK << IW_B_LSB)
#define OP_SH_RRT IW_B_LSB

#define OP_MASK_RRS (IW_A_MASK << IW_A_LSB)
#define OP_SH_RRS IW_A_LSB


#define OP_MASK_JOP (IW_OP_MASK << IW_OP_LSB)
#define OP_SH_JOP IW_OP_LSB

#define OP_MASK_IMM26 (IW_IMM26_MASK << IW_IMM26_LSB)
#define OP_SH_IMM26 IW_IMM26_LSB


#define OP_MASK_RCTL 0x000007c0
#define OP_SH_RCTL 6


#define OP_MASK_TRAP_IMM5 0x000007c0
#define OP_SH_TRAP_IMM5 6


#define OP_MASK_IMM5 (IW_SHIFT_IMM5_MASK << IW_SHIFT_IMM5_LSB)
#define OP_SH_IMM5 IW_SHIFT_IMM5_LSB


#define OP_MASK_CACHE_OPX (IW_B_MASK << IW_B_LSB)
#define OP_SH_CACHE_OPX IW_B_LSB
#define OP_MASK_CACHE_RRS (IW_A_MASK << IW_A_LSB)
#define OP_SH_CACHE_RRS IW_A_LSB


#define OP_MASK_CUSTOM_A 0x00010000
#define OP_SH_CUSTOM_A 16

#define OP_MASK_CUSTOM_B 0x00008000
#define OP_SH_CUSTOM_B 15

#define OP_MASK_CUSTOM_C 0x00004000
#define OP_SH_CUSTOM_C 14

#define OP_MASK_CUSTOM_N 0x00003fc0
#define OP_SH_CUSTOM_N 6
#define OP_MAX_CUSTOM_N 255


#define OP_ADDI 4
#define OP_ANDHI 44
#define OP_ANDI 12
#define OP_BEQ 38
#define OP_BGE 14
#define OP_BGEU 46
#define OP_BLT 22
#define OP_BLTU 54
#define OP_BNE 30
#define OP_BR 6
#define OP_CALL 0
#define OP_CMPEQI 32
#define OP_CMPGEI 8
#define OP_CMPGEUI 40
#define OP_CMPLTI 16
#define OP_CMPLTUI 48
#define OP_CMPNEI 24
#define OP_CUSTOM 50
#define OP_FLUSHD 59
#define OP_FLUSHDA 27
#define OP_INITD 51
#define OP_INITDA 19
#define OP_JMPI 1
#define OP_LDB 7
#define OP_LDBIO 39
#define OP_LDBU 3
#define OP_LDBUIO 35
#define OP_LDH 15
#define OP_LDHIO 47
#define OP_LDHU 11
#define OP_LDHUIO 43
#define OP_LDL 31
#define OP_LDW 23
#define OP_LDWIO 55
#define OP_MULI 36
#define OP_OPX 58
#define OP_ORHI 52
#define OP_ORI 20
#define OP_RDPRS 56
#define OP_STB 5
#define OP_STBIO 37
#define OP_STC 29
#define OP_STH 13
#define OP_STHIO 45
#define OP_STW 21
#define OP_STWIO 53
#define OP_XORHI 60
#define OP_XORI 28


#define OPX_ADD 49
#define OPX_AND 14
#define OPX_BREAK 52
#define OPX_BRET 9
#define OPX_CALLR 29
#define OPX_CMPEQ 32
#define OPX_CMPGE 8
#define OPX_CMPGEU 40
#define OPX_CMPLT 16
#define OPX_CMPLTU 48
#define OPX_CMPNE 24
#define OPX_CRST 62
#define OPX_DIV 37
#define OPX_DIVU 36
#define OPX_ERET 1
#define OPX_FLUSHI 12
#define OPX_FLUSHP 4
#define OPX_HBREAK 53
#define OPX_INITI 41
#define OPX_INTR 61
#define OPX_JMP 13
#define OPX_MUL 39
#define OPX_MULXSS 31
#define OPX_MULXSU 23
#define OPX_MULXUU 7
#define OPX_NEXTPC 28
#define OPX_NOR 6
#define OPX_OR 22
#define OPX_RDCTL 38
#define OPX_RET 5
#define OPX_ROL 3
#define OPX_ROLI 2
#define OPX_ROR 11
#define OPX_SLL 19
#define OPX_SLLI 18
#define OPX_SRA 59
#define OPX_SRAI 58
#define OPX_SRL 27
#define OPX_SRLI 26
#define OPX_SUB 57
#define OPX_SYNC 54
#define OPX_TRAP 45
#define OPX_WRCTL 46
#define OPX_WRPRS 20
#define OPX_XOR 30





#define OP_MATCH_ADDI OP_ADDI
#define OP_MATCH_ANDHI OP_ANDHI
#define OP_MATCH_ANDI OP_ANDI
#define OP_MATCH_BEQ OP_BEQ
#define OP_MATCH_BGE OP_BGE
#define OP_MATCH_BGEU OP_BGEU
#define OP_MATCH_BLT OP_BLT
#define OP_MATCH_BLTU OP_BLTU
#define OP_MATCH_BNE OP_BNE
#define OP_MATCH_BR OP_BR
#define OP_MATCH_FLUSHD OP_FLUSHD
#define OP_MATCH_FLUSHDA OP_FLUSHDA
#define OP_MATCH_INITD OP_INITD
#define OP_MATCH_INITDA OP_INITDA
#define OP_MATCH_CALL OP_CALL
#define OP_MATCH_CMPEQI OP_CMPEQI
#define OP_MATCH_CMPGEI OP_CMPGEI
#define OP_MATCH_CMPGEUI OP_CMPGEUI
#define OP_MATCH_CMPLTI OP_CMPLTI
#define OP_MATCH_CMPLTUI OP_CMPLTUI
#define OP_MATCH_CMPNEI OP_CMPNEI
#define OP_MATCH_JMPI OP_JMPI
#define OP_MATCH_LDB OP_LDB
#define OP_MATCH_LDBIO OP_LDBIO
#define OP_MATCH_LDBU OP_LDBU
#define OP_MATCH_LDBUIO OP_LDBUIO
#define OP_MATCH_LDH OP_LDH
#define OP_MATCH_LDHIO OP_LDHIO
#define OP_MATCH_LDHU OP_LDHU
#define OP_MATCH_LDHUIO OP_LDHUIO
#define OP_MATCH_LDL OP_LDL
#define OP_MATCH_LDW OP_LDW
#define OP_MATCH_LDWIO OP_LDWIO
#define OP_MATCH_MULI OP_MULI
#define OP_MATCH_OPX OP_OPX
#define OP_MATCH_ORHI OP_ORHI
#define OP_MATCH_ORI OP_ORI
#define OP_MATCH_RDPRS OP_RDPRS
#define OP_MATCH_STB OP_STB
#define OP_MATCH_STBIO OP_STBIO
#define OP_MATCH_STC OP_STC
#define OP_MATCH_STH OP_STH
#define OP_MATCH_STHIO OP_STHIO
#define OP_MATCH_STW OP_STW
#define OP_MATCH_STWIO OP_STWIO
#define OP_MATCH_CUSTOM OP_CUSTOM
#define OP_MATCH_XORHI OP_XORHI
#define OP_MATCH_XORI OP_XORI
#define OP_MATCH_OPX OP_OPX


#define OPX_MATCH(code) ((code << IW_OPX_LSB) | OP_OPX)

#define OP_MATCH_ADD OPX_MATCH (OPX_ADD)
#define OP_MATCH_AND OPX_MATCH (OPX_AND)
#define OP_MATCH_BREAK ((0x1e << 17) | OPX_MATCH (OPX_BREAK))
#define OP_MATCH_BRET (0xf0000000 | OPX_MATCH (OPX_BRET))
#define OP_MATCH_CALLR ((0x1f << 17) | OPX_MATCH (OPX_CALLR))
#define OP_MATCH_CMPEQ OPX_MATCH (OPX_CMPEQ)
#define OP_MATCH_CMPGE OPX_MATCH (OPX_CMPGE)
#define OP_MATCH_CMPGEU OPX_MATCH (OPX_CMPGEU)
#define OP_MATCH_CMPLT OPX_MATCH (OPX_CMPLT)
#define OP_MATCH_CMPLTU OPX_MATCH (OPX_CMPLTU)
#define OP_MATCH_CMPNE OPX_MATCH (OPX_CMPNE)
#define OP_MATCH_DIV OPX_MATCH (OPX_DIV)
#define OP_MATCH_DIVU OPX_MATCH (OPX_DIVU)
#define OP_MATCH_JMP OPX_MATCH (OPX_JMP)
#define OP_MATCH_MUL OPX_MATCH (OPX_MUL)
#define OP_MATCH_MULXSS OPX_MATCH (OPX_MULXSS)
#define OP_MATCH_MULXSU OPX_MATCH (OPX_MULXSU)
#define OP_MATCH_MULXUU OPX_MATCH (OPX_MULXUU)
#define OP_MATCH_NEXTPC OPX_MATCH (OPX_NEXTPC)
#define OP_MATCH_NOR OPX_MATCH (OPX_NOR)
#define OP_MATCH_OR OPX_MATCH (OPX_OR)
#define OP_MATCH_RDCTL OPX_MATCH (OPX_RDCTL)
#define OP_MATCH_RET (0xf8000000 | OPX_MATCH (OPX_RET))
#define OP_MATCH_ROL OPX_MATCH (OPX_ROL)
#define OP_MATCH_ROLI OPX_MATCH (OPX_ROLI)
#define OP_MATCH_ROR OPX_MATCH (OPX_ROR)
#define OP_MATCH_SLL OPX_MATCH (OPX_SLL)
#define OP_MATCH_SLLI OPX_MATCH (OPX_SLLI)
#define OP_MATCH_SRA OPX_MATCH (OPX_SRA)
#define OP_MATCH_SRAI OPX_MATCH (OPX_SRAI)
#define OP_MATCH_SRL OPX_MATCH (OPX_SRL)
#define OP_MATCH_SRLI OPX_MATCH (OPX_SRLI)
#define OP_MATCH_SUB OPX_MATCH (OPX_SUB)
#define OP_MATCH_SYNC OPX_MATCH (OPX_SYNC)
#define OP_MATCH_TRAP ((0x1d << 17) | OPX_MATCH (OPX_TRAP))
#define OP_MATCH_ERET (0xef800000 | OPX_MATCH (OPX_ERET))
#define OP_MATCH_WRCTL OPX_MATCH (OPX_WRCTL)
#define OP_MATCH_WRPRS OPX_MATCH (OPX_WRPRS)
#define OP_MATCH_XOR OPX_MATCH (OPX_XOR)
#define OP_MATCH_FLUSHI OPX_MATCH (OPX_FLUSHI)
#define OP_MATCH_FLUSHP OPX_MATCH (OPX_FLUSHP)
#define OP_MATCH_INITI OPX_MATCH (OPX_INITI)


#define OP_MASK_BREAK ((OP_MASK_RRS | OP_MASK_RRT | OP_MASK_RRD | OP_MASK_ROPX | OP_MASK_OP) & 0xfffff03f)


#define OP_MASK_CALLR ((OP_MASK_RRT | OP_MASK_RRD | OP_MASK_ROPX | OP_MASK_OP))

#define OP_MASK_JMP ((OP_MASK_RRT | OP_MASK_RRD | OP_MASK_ROPX | OP_MASK_OP))

#define OP_MASK_SYNC ((OP_MASK_RRT | OP_MASK_RRD | OP_MASK_ROPX | OP_MASK_OP))

#define OP_MASK_TRAP ((OP_MASK_RRS | OP_MASK_RRT | OP_MASK_RRD | OP_MASK_ROPX | OP_MASK_OP) & 0xfffff83f)


#define OP_MASK_WRCTL ((OP_MASK_RRT | OP_MASK_RRD | OP_MASK_ROPX | OP_MASK_OP)) 

#define OP_MASK_NEXTPC ((OP_MASK_RRS | OP_MASK_RRT | OP_MASK_ROPX | OP_MASK_OP))

#define OP_MASK_FLUSHI ((OP_MASK_RRT | OP_MASK_RRD | OP_MASK_ROPX | OP_MASK_OP))

#define OP_MASK_INITI ((OP_MASK_RRT | OP_MASK_RRD | OP_MASK_ROPX | OP_MASK_OP))


#define OP_MASK_ROLI ((OP_MASK_RRT | OP_MASK_ROPX | OP_MASK_OP))
#define OP_MASK_SLLI ((OP_MASK_RRT | OP_MASK_ROPX | OP_MASK_OP))
#define OP_MASK_SRAI ((OP_MASK_RRT | OP_MASK_ROPX | OP_MASK_OP))
#define OP_MASK_SRLI ((OP_MASK_RRT | OP_MASK_ROPX | OP_MASK_OP))
#define OP_MASK_RDCTL ((OP_MASK_RRS | OP_MASK_RRT | OP_MASK_ROPX | OP_MASK_OP)) 


#if !defined(OP_MASK)
#define OP_MASK 0xffffffff
#endif


#define GET_IW_A(Iw) (((Iw) >> IW_A_LSB) & IW_A_MASK)

#define GET_IW_B(Iw) (((Iw) >> IW_B_LSB) & IW_B_MASK)

#define GET_IW_C(Iw) (((Iw) >> IW_C_LSB) & IW_C_MASK)

#define GET_IW_CONTROL_REGNUM(Iw) (((Iw) >> IW_CONTROL_REGNUM_LSB) & IW_CONTROL_REGNUM_MASK)

#define GET_IW_IMM16(Iw) (((Iw) >> IW_IMM16_LSB) & IW_IMM16_MASK)

#define GET_IW_IMM26(Iw) (((Iw) >> IW_IMM26_LSB) & IW_IMM26_MASK)

#define GET_IW_OP(Iw) (((Iw) >> IW_OP_LSB) & IW_OP_MASK)

#define GET_IW_OPX(Iw) (((Iw) >> IW_OPX_LSB) & IW_OPX_MASK)



extern const struct nios2_opcode nios2_builtin_opcodes[];
extern const int bfd_nios2_num_builtin_opcodes;
extern struct nios2_opcode *nios2_opcodes;
extern int bfd_nios2_num_opcodes;


extern const struct nios2_reg nios2_builtin_regs[];
extern struct nios2_reg *nios2_regs;
extern const int nios2_num_builtin_regs;
extern int nios2_num_regs;


#define NUMOPCODES bfd_nios2_num_opcodes
#define NUMREGISTERS nios2_num_regs;



extern const struct nios2_opcode *nios2_find_opcode_hash (unsigned long);

#endif 
