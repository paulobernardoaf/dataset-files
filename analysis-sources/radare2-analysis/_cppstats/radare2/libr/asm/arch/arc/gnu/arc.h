#define ARC_MACH_ARC4 1
#define ARC_MACH_ARC5 2
#define ARC_MACH_ARC6 4
#define ARC_MACH_ARC7 8
#define ARC_MACH_ARC601 16
#define E_ARC_MACH_A4 0x00000000
#define EM_ARCOMPACT 93 
#define ARC_MACH_BIG 32
#define ARCOMPACT (ARC_MACH_ARC5 | ARC_MACH_ARC6 | ARC_MACH_ARC601 | ARC_MACH_ARC7)
#define ARC_MACH_CPU_MASK (ARC_MACH_BIG - 1)
#define ARC_MACH_MASK ((ARC_MACH_BIG << 1) - 1)
typedef unsigned int arc_insn;
struct arc_opcode {
char *syntax; 
unsigned long mask, value; 
int flags; 
#define ARC_OPCODE_CPU(bits) ((bits) & ARC_MACH_CPU_MASK)
#define ARC_OPCODE_MACH(bits) ((bits) & ARC_MACH_MASK)
#define ARC_OPCODE_FLAG_START (ARC_MACH_MASK + 1)
#define ARC_OPCODE_COND_BRANCH (ARC_OPCODE_FLAG_START)
#define SYNTAX_LENGTH (ARC_OPCODE_COND_BRANCH << 1)
#define SYNTAX_3OP (SYNTAX_LENGTH )
#define SYNTAX_2OP (SYNTAX_3OP << 1)
#define SYNTAX_1OP (SYNTAX_2OP << 1)
#define SYNTAX_NOP (SYNTAX_1OP << 1)
#define OP1_DEST_IGNORED (SYNTAX_NOP << 1)
#define OP1_MUST_BE_IMM (OP1_DEST_IGNORED << 1)
#define OP1_IMM_IMPLIED (OP1_MUST_BE_IMM << 1)
#define SUFFIX_NONE (OP1_IMM_IMPLIED << 1)
#define SUFFIX_COND (SUFFIX_NONE << 1)
#define SUFFIX_FLAG (SUFFIX_COND << 1)
#define SYNTAX_VALID (SUFFIX_FLAG << 1)
#define SIMD_LONG_INST (SYNTAX_VALID << 1)
#define AC_SYNTAX_3OP (0x01)
#define AC_SYNTAX_2OP (AC_SYNTAX_3OP << 1)
#define AC_SYNTAX_1OP (AC_SYNTAX_2OP << 1)
#define AC_SYNTAX_NOP (AC_SYNTAX_1OP << 1)
#define AC_SYNTAX_SIMD (AC_SYNTAX_NOP << 1)
#define AC_OP1_DEST_IGNORED (AC_SYNTAX_SIMD << 1)
#define AC_OP1_MUST_BE_IMM (AC_OP1_DEST_IGNORED << 1)
#define AC_OP1_IMM_IMPLIED (AC_OP1_MUST_BE_IMM << 1)
#define AC_SIMD_SYNTAX_DISC (AC_OP1_IMM_IMPLIED << 1)
#define AC_SIMD_IREGA (AC_SIMD_SYNTAX_DISC << 1)
#define AC_SIMD_IREGB (AC_SIMD_IREGA << 1)
#define AC_SIMD_SYNTAX_VVV (AC_SIMD_IREGB << 1)
#define AC_SIMD_SYNTAX_VV0 (AC_SIMD_SYNTAX_VVV << 1)
#define AC_SIMD_SYNTAX_VbI0 (AC_SIMD_SYNTAX_VV0 << 1)
#define AC_SIMD_SYNTAX_Vb00 (AC_SIMD_SYNTAX_VbI0 << 1)
#define AC_SIMD_SYNTAX_VbC0 (AC_SIMD_SYNTAX_Vb00 << 1)
#define AC_SIMD_SYNTAX_V00 (AC_SIMD_SYNTAX_VbC0 << 1)
#define AC_SIMD_SYNTAX_VC0 (AC_SIMD_SYNTAX_V00 << 1)
#define AC_SIMD_SYNTAX_VVC (AC_SIMD_SYNTAX_VC0 << 1)
#define AC_SIMD_SYNTAX_VV (AC_SIMD_SYNTAX_VVC << 1)
#define AC_SIMD_SYNTAX_VVI (AC_SIMD_SYNTAX_VV << 1)
#define AC_SIMD_SYNTAX_C (AC_SIMD_SYNTAX_VVI << 1)
#define AC_SIMD_SYNTAX_0 (AC_SIMD_SYNTAX_C << 1)
#define AC_SIMD_SYNTAX_CC (AC_SIMD_SYNTAX_0 << 1)
#define AC_SIMD_SYNTAX_C0 (AC_SIMD_SYNTAX_CC << 1)
#define AC_SIMD_SYNTAX_DC (AC_SIMD_SYNTAX_C0 << 1)
#define AC_SIMD_SYNTAX_D0 (AC_SIMD_SYNTAX_DC << 1)
#define AC_SIMD_SYNTAX_VD (AC_SIMD_SYNTAX_D0 << 1)
#define AC_SIMD_SYNTAX_VVL (AC_SIMD_SYNTAX_VD << 1)
#define AC_SIMD_SYNTAX_VU0 (AC_SIMD_SYNTAX_VVL << 1)
#define AC_SIMD_SYNTAX_VL0 (AC_SIMD_SYNTAX_VU0 << 1)
#define AC_SIMD_SYNTAX_C00 (AC_SIMD_SYNTAX_VL0 << 1)
#define AC_SUFFIX_NONE (0x1)
#define AC_SUFFIX_DIRECT (AC_SUFFIX_NONE << 1)
#define AC_SUFFIX_COND (AC_SUFFIX_DIRECT << 1)
#define AC_SUFFIX_FLAG (AC_SUFFIX_COND << 1)
#define AC_SIMD_FLAGS_NONE (AC_SUFFIX_FLAG << 1)
#define AC_SIMD_FLAG_SET (AC_SIMD_FLAGS_NONE << 1)
#define AC_SIMD_FLAG1_SET (AC_SIMD_FLAG_SET << 1)
#define AC_SIMD_FLAG2_SET (AC_SIMD_FLAG1_SET << 1)
#define AC_SIMD_ENCODE_U8 (AC_SIMD_FLAG2_SET << 1)
#define AC_SIMD_ENCODE_U6 (AC_SIMD_ENCODE_U8 << 1)
#define AC_SIMD_SCALE_1 (AC_SIMD_ENCODE_U6 << 1)
#define AC_SIMD_SCALE_2 (AC_SIMD_SCALE_1 << 1)
#define AC_SIMD_SCALE_3 (AC_SIMD_SCALE_2 << 1)
#define AC_SIMD_SCALE_4 (AC_SIMD_SCALE_3 << 1)
#define AC_SIMD_SCALE_0 (AC_SIMD_SCALE_4 << 1)
#define AC_SIMD_ENCODE_LIMM (AC_SIMD_SCALE_0 << 1)
#define AC_SIMD_EXTENDED (AC_SIMD_ENCODE_LIMM << 1)
#define AC_SIMD_EXTEND2 (AC_SIMD_EXTENDED << 1)
#define AC_SIMD_EXTEND3 (AC_SIMD_EXTEND2 << 1)
#define AC_SUFFIX_LANEMASK (AC_SIMD_EXTEND3 << 1)
#define AC_SIMD_ENCODE_S12 (AC_SUFFIX_LANEMASK << 1)
#define AC_SIMD_ZERVA (AC_SIMD_ENCODE_S12 << 1)
#define AC_SIMD_ZERVB (AC_SIMD_ZERVA << 1)
#define AC_SIMD_ZERVC (AC_SIMD_ZERVB << 1)
#define AC_SIMD_SETLM (AC_SIMD_ZERVC << 1)
#define AC_SIMD_EXTEND1 (AC_SIMD_SETLM << 1)
#define AC_SIMD_KREG (AC_SIMD_EXTEND1 << 1)
#define AC_SIMD_ENCODE_U16 (AC_SIMD_KREG << 1)
#define AC_SIMD_ENCODE_ZR (AC_SIMD_ENCODE_U16 << 1)
#define AC_EXTENDED_MULTIPLY AC_SIMD_EXTENDED
#define I(x) (((unsigned) (x) & 31) << 27)
#define A(x) (((unsigned) (x) & ARC_MASK_REG) << ARC_SHIFT_REGA)
#define B(x) (((unsigned) (x) & ARC_MASK_REG) << ARC_SHIFT_REGB)
#define C(x) (((unsigned) (x) & ARC_MASK_REG) << ARC_SHIFT_REGC)
#define R(x,b,m) (((unsigned) (x) & (m)) << (b)) 
struct arc_opcode *next_asm; 
struct arc_opcode *next_dis; 
#define ARC_HASH_OPCODE(string) ((string)[0] >= 'a' && (string)[0] <= 'z' ? (string)[0] - 'a' : 26)
#define ARC_HASH_ICODE(insn) ((unsigned int) (insn) >> 27)
#define ARC_OPCODE_NEXT_ASM(op) ((op)->next_asm)
#define ARC_OPCODE_NEXT_DIS(op) ((op)->next_dis)
unsigned long mask2,value2; 
};
struct arc_operand_value {
char *name; 
short value; 
unsigned char type; 
unsigned char flags; 
#define ARC_OPVAL_CPU(bits) ((bits) & ARC_MACH_CPU_MASK)
#define ARC_OPVAL_MACH(bits) ((bits) & ARC_MACH_MASK)
};
struct arc_ext_operand_value {
struct arc_ext_operand_value *next;
struct arc_operand_value operand;
};
extern struct arc_ext_operand_value *arc_ext_operands;
struct arc_operand {
unsigned char fmt;
unsigned char bits;
unsigned char shift;
int flags;
#define ARC_OPERAND_SUFFIX 1
#define ARC_OPERAND_RELATIVE_BRANCH 2
#define ARC_OPERAND_ABSOLUTE_BRANCH 4
#define ARC_OPERAND_ADDRESS 8
#define ARC_OPERAND_LIMM 0x10
#define ARC_OPERAND_SIGNED 0x20
#define ARC_OPERAND_SIGNOPT 0x40
#define ARC_OPERAND_NEGATIVE 0x80
#define ARC_OPERAND_FAKE 0x100
#define ARC_OPERAND_JUMPFLAGS 0x200
#define ARC_OPERAND_WARN 0x400
#define ARC_OPERAND_ERROR 0x800
#define ARC_OPERAND_LOAD 0x8000
#define ARC_OPERAND_STORE 0x10000
#define ARC_OPERAND_UNSIGNED 0x20000
#define ARC_OPERAND_2BYTE_ALIGNED 0x40000
#define ARC_OPERAND_4BYTE_ALIGNED 0x80000
#define ARC_MOD_DOT 0x1000
#define ARC_MOD_REG 0x2000
#define ARC_MOD_AUXREG 0x4000
#define ARC_MOD_SDASYM 0x100000
#define ARC_MOD_BITS 0x107000
#define ARC_MOD_P(X) ((X) & ARC_MOD_BITS)
#define ARC_REGISTER_READONLY 0x01
#define ARC_REGISTER_WRITEONLY 0x02
#define ARC_REGISTER_NOSHORT_CUT 0x04
#define ARC_REGISTER_16 0x8
#define ARC_INCR_U6 0x100000
#define ARC_SIMD_SCALE1 (ARC_INCR_U6 << 0x1)
#define ARC_SIMD_SCALE2 (ARC_SIMD_SCALE1 << 0x1)
#define ARC_SIMD_SCALE3 (ARC_SIMD_SCALE2 << 0x1)
#define ARC_SIMD_SCALE4 (ARC_SIMD_SCALE3 << 0x1)
#define ARC_SIMD_LANEMASK (ARC_SIMD_SCALE4 <<0x1)
#define ARC_SIMD_REGISTER (ARC_SIMD_LANEMASK <<0x1)
#define ARC_SIMD_ZERVA (ARC_SIMD_REGISTER <<0x1)
#define ARC_SIMD_ZERVB (ARC_SIMD_ZERVA <<0x1)
#define ARC_SIMD_ZERVC (ARC_SIMD_ZERVB <<0x1)
#define ARC_SIMD_SETLM (ARC_SIMD_ZERVC <<0x1)
#define ARC_REGISTER_SIMD_VR 0x10
#define ARC_REGISTER_SIMD_I 0x20
#define ARC_REGISTER_SIMD_DR 0x40
#define ARC_REGISTER_SIMD_K 0x80
arc_insn (*insert) (arc_insn insn, long *extend, const struct arc_operand *operand,
int mods, const struct arc_operand_value *reg,
long value, const char **errmsg);
long (*extract) (arc_insn *insn,
const struct arc_operand *operand, int mods,
const struct arc_operand_value **opval, int *invalid);
};
#define ARC_HAVE_CPU(bits) ((bits) & ARC_MACH_CPU_MASK)
#define ARC_HAVE_MACH(bits) ((bits) & ARC_MACH_MASK)
#define ARC_REG_SHIMM_UPDATE 61
#define ARC_REG_SHIMM 63
#define ARC_REG_LIMM 62
#define ARC_REG_CONSTANT_P(REG) ((REG) >= 61)
#define ARC_SHIFT_REGA 21
#define ARC_SHIFT_REGB 15
#define ARC_SHIFT_REGC 9
#define ARC_SHIFT_REGA_AC 0
#define ARC_SHIFT_REGB_LOW_AC 24
#define ARC_SHIFT_REGB_HIGH_AC 12
#define ARC_SHIFT_REGC_AC 6
#define ARC_MASK_REG 63
#define ARC_DELAY_NONE 0 
#define ARC_DELAY_NORMAL 1 
#define ARC_DELAY_JUMP 2 
#define ARC_SHIMM_CONST_P(x) ((long) (x) >= -256 && (long) (x) <= 255)
extern const struct arc_operand *arc_operands;
extern int arc_operand_count;
extern const struct arc_operand_value *arc_suffixes;
extern int arc_suffixes_count;
extern const struct arc_operand_value *arc_reg_names;
extern int arc_reg_names_count;
extern unsigned char *arc_operand_map;
extern int arc_cond_p;
extern int arc_mach_a4;
extern unsigned long arc_ld_ext_mask;
extern int arc_user_mode_only;
int arc_get_opcode_mach (int, int);
void arc_opcode_init_tables (int);
void arc_opcode_init_insert (void);
void arc_opcode_init_extract (void);
const struct arc_opcode *arc_opcode_lookup_asm (const char *);
const struct arc_opcode *arc_opcode_lookup_dis (unsigned int);
int arc_opcode_limm_p (long *);
const struct arc_operand_value *arc_opcode_lookup_suffix (const struct arc_operand *type, int value);
int arc_opcode_supported (const struct arc_opcode *);
int arc_opval_supported (const struct arc_operand_value *);
int arc_insn_not_jl (arc_insn);
extern char *arc_aux_reg_name (int);
extern struct arc_operand_value *get_ext_suffix (char *,char);
extern int ac_branch_or_jump_insn (arc_insn, int);
extern int ac_lpcc_insn (arc_insn);
extern int ac_constant_operand (const struct arc_operand *);
extern int ac_register_operand (const struct arc_operand *);
extern int ac_symbol_operand (const struct arc_operand *);
extern int ARC700_register_simd_operand (char);
extern int arc_operand_type (int);
extern int ac_add_reg_sdasym_insn (arc_insn);
extern int ac_get_load_sdasym_insn_type (arc_insn, int);
extern int ac_get_store_sdasym_insn_type (arc_insn, int);
extern int arc_limm_fixup_adjust (arc_insn);
extern int arc_test_wb (void);
