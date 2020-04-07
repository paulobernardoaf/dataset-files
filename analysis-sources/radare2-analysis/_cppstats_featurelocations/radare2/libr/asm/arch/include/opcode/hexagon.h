





















#if !defined(OPCODES_HEXAGON_H)
#define OPCODES_HEXAGON_H 1









#define HEXAGON_MACH 0
#define HEXAGON_MACH_V2 2
#define HEXAGON_MACH_V3 3
#define HEXAGON_MACH_V4 4
#define HEXAGON_MACH_V5 5

#define HEXAGON_MACH_BIG 16


#define HEXAGON_MACH_CPU_MASK (HEXAGON_MACH_BIG - 1)

#define HEXAGON_MACH_MASK ((HEXAGON_MACH_BIG << 1) - 1)


#define HEXAGON_IS_V2 (1 << 31)
#define HEXAGON_IS_V3 (1 << 30)
#define HEXAGON_IS_V4 (1 << 29)
#define HEXAGON_IS_V5 (1 << 28)
#define HEXAGON_V2_AND_V3 (HEXAGON_IS_V3 | HEXAGON_IS_V2)
#define HEXAGON_V2_AND_UP (HEXAGON_IS_V5 | HEXAGON_IS_V4 | HEXAGON_IS_V3 | HEXAGON_IS_V2)
#define HEXAGON_V3_AND_UP (HEXAGON_IS_V5 | HEXAGON_IS_V4 | HEXAGON_IS_V3)
#define HEXAGON_V4_AND_UP (HEXAGON_IS_V5 | HEXAGON_IS_V4)


#define HEXAGON_INSN_LEN (4)



#define MAX_PACKET_INSNS 4


#define HEXAGON_MAPPED_LEN (256)


#define HEXAGON_IS16BITS(num) 0 


#define HEXAGON_IS16INSN(num) 0 


#define HEXAGON_LO16(num) ((num) & ~(-1 << 16))


#define HEXAGON_HI16(num) (HEXAGON_LO16 ((num) >> 16))


#define HEXAGON_KXER_MASK(num) ((num) & (-1 << 6))


#define HEXAGON_KXED_MASK(num) ((num) & ~(-1 << 6))


#define HEXAGON_NUM_GENERAL_PURPOSE_REGS 32
#define HEXAGON_NUM_CONTROL_REGS 32
#define HEXAGON_NUM_SYS_CTRL_REGS 64
#define HEXAGON_NUM_PREDICATE_REGS 4
#define HEXAGON_NUM_GUEST_REGS 32


#define HEXAGON_SUBREGS (16)
#define HEXAGON_SUBREGS_LO (0)
#define HEXAGON_SUBREGS_HI (16)
#define HEXAGON_SUBREGS_TO(r, p) (((r) < HEXAGON_SUBREGS_HI ? (r) : (r) - HEXAGON_SUBREGS / 2) / ((p)? 2: 1))



#define HEXAGON_SUBREGS_FROM(r, p) ((r) * ((p)? 2: 1) < HEXAGON_SUBREGS / 2 ? (r) * ((p)? 2: 1) : (r) * ((p)? 2: 1) - HEXAGON_SUBREGS / 2 + HEXAGON_SUBREGS_HI)





#define HEXAGON_SLOTS_DUPLEX (0x3) 
#define HEXAGON_SLOTS_STORES (0x2) 
#define HEXAGON_SLOTS_1 (0x2) 
#define HEXAGON_SLOTS_MEM1 (0x1) 



#define HEXAGON_INSN_TYPE_A7 (0x70000000) 
#define HEXAGON_INSN_TYPE_X8 (0x80000000) 
#define HEXAGON_INSN_TYPE_A11 (0xb0000000) 
#define HEXAGON_INSN_TYPE_X12 (0xc0000000) 
#define HEXAGON_INSN_TYPE_X13 (0xd0000000) 
#define HEXAGON_INSN_TYPE_X14 (0xe0000000) 
#define HEXAGON_INSN_TYPE_A15 (0xf0000000) 
#define HEXAGON_INSN_TYPE_MASK(i) ((i) & 0xf0000000) 
#define HEXAGON_INSN_TYPE_A(i) ((HEXAGON_INSN_TYPE_MASK (i) == HEXAGON_INSN_TYPE_A7) || (HEXAGON_INSN_TYPE_MASK (i) == HEXAGON_INSN_TYPE_A11) || (HEXAGON_INSN_TYPE_MASK (i) == HEXAGON_INSN_TYPE_A15))


#define HEXAGON_INSN_TYPE_X(i) ((HEXAGON_INSN_TYPE_MASK (i) == HEXAGON_INSN_TYPE_X8) || (HEXAGON_INSN_TYPE_MASK (i) == HEXAGON_INSN_TYPE_X12) || (HEXAGON_INSN_TYPE_MASK (i) == HEXAGON_INSN_TYPE_X13) || (HEXAGON_INSN_TYPE_MASK (i) == HEXAGON_INSN_TYPE_X14))





typedef unsigned int hexagon_insn;

typedef struct _hexagon_opcode
{
char *syntax; 
char *enc; 
int flags; 




#define HEXAGON_CODE_CPU(b) ((b) & HEXAGON_MACH_CPU_MASK)

#define HEXAGON_CODE_MACH(b) ((b) & HEXAGON_MACH_MASK)



#define HEXAGON_CODE_FLAG(f) ((f) << 8)

#define HEXAGON_CODE_IS_PREFIX (HEXAGON_CODE_FLAG (0x0001))
#define HEXAGON_CODE_IS_DUPLEX (HEXAGON_CODE_FLAG (0x0002))
#define HEXAGON_CODE_IS_COMPND (HEXAGON_CODE_FLAG (0x0004))
#define HEXAGON_CODE_IS_BRANCH (HEXAGON_CODE_FLAG (0x0008))
#define HEXAGON_CODE_IS_MEMORY (HEXAGON_CODE_FLAG (0x0010))
#define HEXAGON_CODE_IS_LOAD (HEXAGON_CODE_FLAG (0x0020))
#define HEXAGON_CODE_IS_STORE (HEXAGON_CODE_FLAG (0x0040))





struct _hexagon_opcode *next_asm; 
struct _hexagon_opcode *next_dis; 


#define HEXAGON_HASH_OPCODE(string) hexagon_hash_opcode (string)

#define HEXAGON_HASH_ICODE(insn) hexagon_hash_icode (insn)




#define HEXAGON_CODE_NEXT_ASM(op) ((op)? (op)->next_asm: (op))
#define HEXAGON_CODE_NEXT_DIS(op) ((op)? (op)->next_dis: (op))

unsigned int slots; 

unsigned int implicit; 


#define IMPLICIT_LR 0x0001
#define IMPLICIT_SP 0x0002
#define IMPLICIT_FP 0x0004
#define IMPLICIT_PC 0x0008
#define IMPLICIT_LC0 0x0010
#define IMPLICIT_SA0 0x0020
#define IMPLICIT_LC1 0x0040
#define IMPLICIT_SA1 0x0080
#define IMPLICIT_SR_OVF 0x0100
#define IMPLICIT_P3 0x0200

#define IMPLICIT_P0 0x0400

#define IMPLICIT_P1 0x0800

unsigned int attributes;


#define A_IT_NOP 0x00000001
#define A_RESTRICT_NOSRMOVE 0x00000002
#define A_RESTRICT_LOOP_LA 0x00000004
#define A_NOTE_LA_RESTRICT A_RESTRICT_LOOP_LA
#define A_RESTRICT_COF_MAX1 0x00000008
#define A_RESTRICT_NOPACKET 0x00000010
#define A_NOTE_NOPACKET A_RESTRICT_NOPACKET
#define A_RESTRICT_NOSLOT1 0x00000020
#define A_NOTE_NOSLOT1 A_RESTRICT_NOSLOT1
#define A_RESTRICT_NOCOF 0x00000040
#define A_COF 0x00000080
#define A_RESTRICT_BRANCHADDER_MAX1 0x00000100
#define A_NOTE_BRANCHADDER_MAX1 A_RESTRICT_BRANCHADDER_MAX1
#define A_BRANCHADDER 0x00000200
#define A_RESTRICT_SINGLE_MEM_FIRST 0x00000400
#define CONDITIONAL_EXEC 0x00000800
#define A_CONDEXEC CONDITIONAL_EXEC
#define A_NOTE_CONDITIONAL CONDITIONAL_EXEC
#define CONDITION_SENSE_INVERTED 0x00001000
#define CONDITION_DOTNEW 0x00002000
#define A_DOTNEW CONDITION_DOTNEW
#define A_RESTRICT_PREFERSLOT0 0x00004000
#define A_RESTRICT_LATEPRED 0x00008000
#define A_NOTE_LATEPRED A_RESTRICT_LATEPRED

#define A_RESTRICT_PACKET_AXOK 0x00010000
#define A_NOTE_AXOK A_RESTRICT_PACKET_AXOK
#define A_RESTRICT_PACKET_SOMEREGS_OK 0x00020000
#define A_RELAX_COF_1ST 0x00040000
#define A_RELAX_COF_2ND 0x00080000

#define PACKED 0x00100000
#define A_IT_EXTENDER 0x00200000
#define EXTENDABLE_LOWER_CASE_IMMEDIATE 0x00400000
#define EXTENDABLE_UPPER_CASE_IMMEDIATE 0x00800000
#define A_RESTRICT_SLOT0ONLY 0x01000000
#define A_STORE 0x02000000
#define A_STOREIMMED 0x04000000
#define A_RESTRICT_NOSLOT1_STORE 0x08000000
#define MUST_EXTEND 0x10000000
#define A_MUST_EXTEND MUST_EXTEND
#define A_IT_HWLOOP 0x20000000
#define A_RESTRICT_NOLOOPSETUP 0x40000000


#define A_GUEST 0x00000000
#define A_NOTE_GUEST 0x00000000
#define A_EXTENDABLE 0x00000000
#define A_EXT_LOWER_IMMED 0x00000000
#define A_EXT_UPPER_IMMED 0x00000000
#define A_ARCHV2 0x00000000
#define A_ARCHV3 0x00000000
#define A_CRSLOT23 0x00000000
#define A_NOTE_CRSLOT23 0x00000000
#define A_MEMSIZE_1B 0x00000000
#define A_MEMSIZE_2B 0x00000000
#define A_MEMSIZE_4B 0x00000000
#define A_MEMSIZE_8B 0x00000000
#define A_MEMLIKE 0x00000000
#define A_IMPLICIT_WRITES_SP 0x00000000
#define A_IMPLICIT_READS_SP 0x00000000
#define A_IMPLICIT_READS_LR 0x00000000
#define A_IMPLICIT_WRITES_LR 0x00000000
#define A_IMPLICIT_WRITES_FP 0x00000000
#define A_IMPLICIT_READS_FP 0x00000000
#define A_IMPLICIT_READS_PC 0x00000000
#define A_IMPLICIT_WRITES_PC 0x00000000
#define A_IMPLICIT_READS_GP 0x00000000
#define A_IMPLICIT_READS_CS 0x00000000
#define A_IMPLICIT_READS_P0 0x00000000
#define A_IMPLICIT_WRITES_P0 0x00000000
#define A_IMPLICIT_READS_P1 0x00000000
#define A_IMPLICIT_WRITES_P1 0x00000000
#define A_IMPLICIT_WRITES_P3 0x00000000
#define A_IMPLICIT_WRITES_SRBIT 0x00000000
#define A_IMPLICIT_WRITES_LC0 0x00000000
#define A_IMPLICIT_WRITES_LC1 0x00000000
#define A_IMPLICIT_WRITES_SA0 0x00000000
#define A_IMPLICIT_WRITES_SA1 0x00000000
#define A_JUMP 0x00000000
#define A_CJUMP 0x00000000
#define A_NEWCMPJUMP 0x00000000
#define A_DIRECT 0x00000000
#define A_INDIRECT 0x00000000
#define A_CALL 0x00000000
#define A_ROPS_2 0x00000000
#define A_ROPS_3 0x00000000
#define A_MEMOP 0x00000000
#define A_LOAD 0x00000000
#define A_NVSTORE 0x00000000
#define A_DOTOLD 0x00000000
#define A_COMMUTES 0x00000000
#define A_PRIV 0x00000000
#define A_NOTE_PRIV 0x00000000
#define A_SATURATE 0x00000000
#define A_USATURATE 0x00000000
#define A_NOTE_SR_OVF_WHEN_SATURATING 0x00000000
#define A_BIDIRSHIFTL 0x00000000
#define A_BIDIRSHIFTR 0x00000000
#define A_NOTE_OOBVSHIFT 0X00000000
#define A_ICOP 0x00000000
#define A_INTRINSIC_RETURNS_UNSIGNED 0x00000000
#define A_CIRCADDR 0x00000000
#define A_BREVADDR 0x00000000
#define A_IT_MPY 0x00000000
#define A_IT_MPY_32 0x00000000
#define A_NOTE_SPECIALGROUPING 0x00000000
#define A_NOTE_PACKET_PC 0x00000000
#define A_NOTE_PACKET_NPC 0x00000000
#define A_NOTE_RELATIVE_ADDRESS 0x00000000
#define A_EXCEPTION_SWI 0x00000000
#define A_NOTE_NEWVAL_SLOT0 0x00000000
#define A_DOTNEWVALUE 0x00000000
#define A_FPOP 0x00000000
#define A_FPDOUBLE 0x00000000
#define A_FPSINGLE 0x00000000
#define A_RET_TYPE 0x00000000
#define A_PRED_BIT_4 0x00000000
#define A_NOTE_COMPAT_ACCURACY 0x00000000
#define A_MPY 0x00000000
#define A_CACHEOP 0x00000000
#define A_RESTRICT_SLOT1_AOK 0x00000000
#define A_NOTE_SLOT1_AOK A_RESTRICT_SLOT1_AOK
#define A_NOTE_DEALLOCRET 0x00000000
#define A_HINTJR 0x00000000
#define A_PRED_BIT_7 0x00000000
#define A_PRED_BIT_12 0x00000000
#define A_PRED_BIT_13 0x00000000
#define A_NOTE_NVSLOT0 0x00000000
#define A_NOTE_DEPRECATED 0x00000000
#define A_HWLOOP0_SETUP 0x00000000
#define A_HWLOOP1_SETUP 0x00000000
#define A_DOTNEW_LOAD 0x00000000


void *map;
} hexagon_opcode;

typedef size_t hexagon_hash;

typedef struct _hexagon_operand
{

char *fmt;


unsigned char bits;


char enc_letter;


unsigned int shift_count;


bfd_reloc_code_real_type reloc_type, reloc_kxer, reloc_kxed;


int flags;


#define HEXAGON_OPERAND_IS_IMMEDIATE 0x00000001
#define HEXAGON_OPERAND_PC_RELATIVE 0x00000002
#define HEXAGON_OPERAND_IS_SIGNED 0x00000004
#define HEXAGON_OPERAND_IS_PAIR 0x00000008
#define HEXAGON_OPERAND_IS_SUBSET 0x00000010
#define HEXAGON_OPERAND_IS_MODIFIER 0x00000020
#define HEXAGON_OPERAND_IS_PREDICATE 0x00000040
#define HEXAGON_OPERAND_IS_CONTROL 0x00000080
#define HEXAGON_OPERAND_IS_SYSTEM 0x00000100
#define HEXAGON_OPERAND_IS_LO16 0x00000200
#define HEXAGON_OPERAND_IS_HI16 0x00000400
#define HEXAGON_OPERAND_IS_READ 0x00000800
#define HEXAGON_OPERAND_IS_WRITE 0x00001000
#define HEXAGON_OPERAND_IS_MODIFIED 0x00002000
#define HEXAGON_OPERAND_IS_NEGATIVE 0x00004000
#define HEXAGON_OPERAND_IS_CONSTANT 0x00008000
#define HEXAGON_OPERAND_IS_NEW 0x00010000
#define HEXAGON_OPERAND_IS_GUEST 0x00020000
#define HEXAGON_OPERAND_IS_REGISTER 0x00040000
#define HEXAGON_OPERAND_IS_RNEW 0x00080000

#define HEXAGON_OPERAND_IS_RELAX 0x10000000
#define HEXAGON_OPERAND_IS_KXER 0x20000000
#define HEXAGON_OPERAND_IS_KXED 0x40000000
#define HEXAGON_OPERAND_IS_INVALID 0x80000000


char *dis_fmt, *alt_fmt;


char *(*parse) (const struct _hexagon_operand *, hexagon_insn *,
const hexagon_opcode *, char *, long *, int *, char **);
} hexagon_operand;

typedef struct hexagon_operand_arg
{
const hexagon_operand *operand;
long value;
char string [HEXAGON_MAPPED_LEN];
} hexagon_operand_arg;

typedef void (*hexagon_mapping) (char *, size_t, const hexagon_operand_arg []);

typedef struct hexagon_reg
{
const char *name;
int reg_num;
int flags;


#define HEXAGON_REG_IS_READONLY (1 << 0)
#define HEXAGON_REG_IS_WRITEONLY (1 << 1)
#define HEXAGON_REG_IS_READWRITE (1 << 2)
} hexagon_reg;

typedef struct hexagon_reg_score
{
char used, letter;
#define HEXAGON_PRED_LEN (3)
#define HEXAGON_PRED_MSK (~(-1 << HEXAGON_PRED_LEN))
#define HEXAGON_PRED_YES (0x01)
#define HEXAGON_PRED_NOT (0x02)
#define HEXAGON_PRED_NEW (0x04)
#define HEXAGON_PRED_GET(P, I) (((P) >> ((I) * HEXAGON_PRED_LEN)) & HEXAGON_PRED_MSK)
#define HEXAGON_PRED_SET(P, I, N) ((HEXAGON_PRED_GET (P, I) | ((N) & HEXAGON_PRED_MSK)) << ((I) * HEXAGON_PRED_LEN))

long pred: (HEXAGON_NUM_PREDICATE_REGS * HEXAGON_PRED_LEN);
size_t ndx;
} hexagon_reg_score;





#define HEXAGON_HAVE_CPU(bits) ((bits) & HEXAGON_MACH_CPU_MASK)

#define HEXAGON_HAVE_MACH(bits) ((bits) & HEXAGON_MACH_MASK)

extern const hexagon_operand hexagon_operands [];
extern const size_t hexagon_operand_count;
extern hexagon_opcode *hexagon_opcodes;
extern size_t hexagon_opcodes_count;
extern int hexagon_verify_hw;
extern hexagon_insn hexagon_nop, hexagon_kext;



#define PACKET_BEGIN '{' 
#define PACKET_END '}' 
#define PACKET_END_INNER ":endloop0" 
#define PACKET_END_OUTER ":endloop1" 
#define PACKET_PAIR ';' 



#define HEXAGON_END_PACKET_POS (14)
#define HEXAGON_END_PACKET_MASK (3 << HEXAGON_END_PACKET_POS)
#define HEXAGON_END_PACKET (3 << HEXAGON_END_PACKET_POS) 
#define HEXAGON_END_LOOP (2 << HEXAGON_END_PACKET_POS) 
#define HEXAGON_END_NOT (1 << HEXAGON_END_PACKET_POS) 
#define HEXAGON_END_PAIR (0 << HEXAGON_END_PACKET_POS) 



#define HEXAGON_END_PACKET_GET(insn) ((insn) & HEXAGON_END_PACKET_MASK) 

#define HEXAGON_END_PACKET_SET(insn, bits) (((insn) & ~HEXAGON_END_PACKET_MASK) | (bits)) 

#define HEXAGON_END_PACKET_RESET(insn) (HEXAGON_END_PACKET_SET ((insn), HEXAGON_END_PACKET_GET (insn) == HEXAGON_END_PAIR ? HEXAGON_END_PAIR: HEXAGON_END_NOT)) 






#define hexagon_if_arch_v1() (FALSE) 
#define hexagon_if_arch_v2() (hexagon_if_arch (HEXAGON_MACH_V2)) 
#define hexagon_if_arch_v3() (hexagon_if_arch (HEXAGON_MACH_V3)) 
#define hexagon_if_arch_v4() (hexagon_if_arch (HEXAGON_MACH_V4)) 
#define hexagon_if_arch_v5() (hexagon_if_arch (HEXAGON_MACH_V5)) 

extern int hexagon_arch (void);
extern int hexagon_if_arch (int);
extern int hexagon_if_arch_kext (void);
extern int hexagon_if_arch_pairs (void);
extern int hexagon_if_arch_autoand (void);
extern int hexagon_get_opcode_mach (int, int);
extern hexagon_hash hexagon_hash_opcode (const char *);
extern hexagon_hash hexagon_hash_icode (hexagon_insn);
extern hexagon_insn hexagon_encode_opcode (const char *);
extern hexagon_insn hexagon_encode_mask (char *);

extern void hexagon_opcode_init_tables (int);
extern const hexagon_opcode *hexagon_opcode_next_asm (const hexagon_opcode *);
extern const hexagon_opcode *hexagon_opcode_lookup_asm (const char *);
extern const hexagon_opcode *hexagon_opcode_lookup_dis (hexagon_insn);
extern const hexagon_opcode *hexagon_lookup_insn (hexagon_insn);
extern int hexagon_opcode_supported (const hexagon_opcode *);
extern int hexagon_encode_operand
(const hexagon_operand *, hexagon_insn *, const hexagon_opcode *,
long, long *, int, int, char **);
extern const hexagon_operand *hexagon_lookup_operand (const char *);
extern const hexagon_operand *hexagon_lookup_reloc
(bfd_reloc_code_real_type, int, const hexagon_opcode *);
extern int hexagon_extract_operand
(const hexagon_operand *, hexagon_insn, bfd_vma, char *, int *, char **);
extern int hexagon_extract_predicate_operand
(const hexagon_operand *, hexagon_insn, char *, int *, char **);
extern int hexagon_extract_modifier_operand
(const hexagon_operand *, hexagon_insn, char *, int *, char **);
extern char *hexagon_dis_operand
(const hexagon_operand *, hexagon_insn, bfd_vma, bfd_vma, char *, char *, char **);
extern int hexagon_dis_opcode
(char *, hexagon_insn, bfd_vma, const hexagon_opcode *, char **);
extern const hexagon_operand *hexagon_operand_find_lo16 (const hexagon_operand *);
extern const hexagon_operand *hexagon_operand_find_hi16 (const hexagon_operand *);
extern const hexagon_operand *hexagon_operand_find
(const hexagon_operand *, const char *);


extern const hexagon_opcode hexagon_packet_header_opcode;

#endif 
