





















#if !defined(__CRIS_H_INCLUDED_)
#define __CRIS_H_INCLUDED_

#if !defined(__STDC__) && !defined(const)
#define const
#endif



#define MAX_REG (15)
#define REG_SP (14)
#define REG_PC (15)




enum cris_insn_version_usage
{

cris_ver_version_all=0,


cris_ver_warning,


cris_ver_v0_3,


cris_ver_v3p,


cris_ver_v8,


cris_ver_v8p,


cris_ver_sim_v0_10,


cris_ver_v0_10,


cris_ver_v3_10,


cris_ver_v8_10,


cris_ver_v10,


cris_ver_v10p,




cris_ver_v32p
};



struct cris_spec_reg
{
const char *const name;
unsigned int number;


unsigned int reg_size;



enum cris_insn_version_usage applicable_version;



const char *const warning;
};
extern const struct cris_spec_reg cris_spec_regs[];



struct cris_support_reg
{
const char *const name;
unsigned int number;
};
extern const struct cris_support_reg cris_support_regs[];

struct cris_cond15
{

const char *const name;


enum cris_insn_version_usage applicable_version;
};
extern const struct cris_cond15 cris_conds15[];


#define AUTOINCR_BIT (0x04)


#define BDAP_QUICK_OPCODE (0x0100)
#define BDAP_QUICK_Z_BITS (0x0e00)

#define BIAP_OPCODE (0x0540)
#define BIAP_Z_BITS (0x0a80)

#define DIP_OPCODE (0x0970)
#define DIP_Z_BITS (0xf280)

#define BDAP_INDIR_LOW (0x40)
#define BDAP_INDIR_LOW_Z (0x80)
#define BDAP_INDIR_HIGH (0x09)
#define BDAP_INDIR_HIGH_Z (0x02)

#define BDAP_INDIR_OPCODE (BDAP_INDIR_HIGH * 0x0100 + BDAP_INDIR_LOW)
#define BDAP_INDIR_Z_BITS (BDAP_INDIR_HIGH_Z * 0x100 + BDAP_INDIR_LOW_Z)
#define BDAP_PC_LOW (BDAP_INDIR_LOW + REG_PC)
#define BDAP_INCR_HIGH (BDAP_INDIR_HIGH + AUTOINCR_BIT)



#define NO_CRIS_PREFIX 0


#define CC_CC 0x0
#define CC_HS 0x0
#define CC_CS 0x1
#define CC_LO 0x1
#define CC_NE 0x2
#define CC_EQ 0x3
#define CC_VC 0x4
#define CC_VS 0x5
#define CC_PL 0x6
#define CC_MI 0x7
#define CC_LS 0x8
#define CC_HI 0x9
#define CC_GE 0xA
#define CC_LT 0xB
#define CC_GT 0xC
#define CC_LE 0xD
#define CC_A 0xE
#define CC_EXT 0xF



extern const char *const cris_cc_strings[];


#define BRANCH_QUICK_LOW (0)
#define BRANCH_QUICK_HIGH (0)
#define BRANCH_QUICK_OPCODE (BRANCH_QUICK_HIGH * 0x0100 + BRANCH_QUICK_LOW)
#define BRANCH_QUICK_Z_BITS (0x0F00)


#define BA_QUICK_HIGH (BRANCH_QUICK_HIGH + CC_A * 0x10)
#define BA_QUICK_OPCODE (BA_QUICK_HIGH * 0x100 + BRANCH_QUICK_LOW)


#define BRANCH_PC_LOW (0xFF)
#define BRANCH_INCR_HIGH (0x0D)
#define BA_PC_INCR_OPCODE ((BRANCH_INCR_HIGH + CC_A * 0x10) * 0x0100 + BRANCH_PC_LOW)






#define JUMP_INDIR_OPCODE (0x0930)
#define JUMP_INDIR_Z_BITS (0xf2c0)
#define JUMP_PC_INCR_OPCODE (JUMP_INDIR_OPCODE + AUTOINCR_BIT * 0x0100 + REG_PC)


#define MOVE_M_TO_PREG_OPCODE 0x0a30
#define MOVE_M_TO_PREG_ZBITS 0x01c0


#define MOVE_PC_INCR_OPCODE_PREFIX (((BDAP_INCR_HIGH | (REG_PC << 4)) << 8) | BDAP_PC_LOW | (2 << 4))

#define MOVE_PC_INCR_OPCODE_SUFFIX (MOVE_M_TO_PREG_OPCODE | REG_PC | (AUTOINCR_BIT << 8))


#define JUMP_PC_INCR_OPCODE_V32 (0x0DBF)


#define BA_DWORD_OPCODE (0x0EBF)


#define NOP_OPCODE (0x050F)
#define NOP_Z_BITS (0xFFFF ^ NOP_OPCODE)

#define NOP_OPCODE_V32 (0x05B0)
#define NOP_Z_BITS_V32 (0xFFFF ^ NOP_OPCODE_V32)





#define NOP_OPCODE_COMMON (0x630)
#define NOP_OPCODE_ZBITS_COMMON (0xffff & ~NOP_OPCODE_COMMON)


#define LAPC_DWORD_OPCODE (0x0D7F)
#define LAPC_DWORD_Z_BITS (0x0fff & ~LAPC_DWORD_OPCODE)


enum cris_imm_oprnd_size_type
{

SIZE_NONE,


SIZE_FIX_32,


SIZE_SPEC_REG,


SIZE_FIELD_SIGNED,


SIZE_FIELD_UNSIGNED,


SIZE_FIELD
};



enum cris_op_type
{
cris_not_implemented_op = 0,
cris_abs_op,
cris_addi_op,
cris_asr_op,
cris_asrq_op,
cris_ax_ei_setf_op,
cris_bdap_prefix,
cris_biap_prefix,
cris_break_op,
cris_btst_nop_op,
cris_clearf_di_op,
cris_dip_prefix,
cris_dstep_logshift_mstep_neg_not_op,
cris_eight_bit_offset_branch_op,
cris_move_mem_to_reg_movem_op,
cris_move_reg_to_mem_movem_op,
cris_move_to_preg_op,
cris_muls_op,
cris_mulu_op,
cris_none_reg_mode_add_sub_cmp_and_or_move_op,
cris_none_reg_mode_clear_test_op,
cris_none_reg_mode_jump_op,
cris_none_reg_mode_move_from_preg_op,
cris_quick_mode_add_sub_op,
cris_quick_mode_and_cmp_move_or_op,
cris_quick_mode_bdap_prefix,
cris_reg_mode_add_sub_cmp_and_or_move_op,
cris_reg_mode_clear_op,
cris_reg_mode_jump_op,
cris_reg_mode_move_from_preg_op,
cris_reg_mode_test_op,
cris_scc_op,
cris_sixteen_bit_offset_branch_op,
cris_three_operand_add_sub_cmp_and_or_op,
cris_three_operand_bound_op,
cris_two_operand_bound_op,
cris_xor_op
};

struct cris_opcode
{

const char *name;


unsigned int match;


unsigned int lose;


const char *args;


char delayed;


enum cris_imm_oprnd_size_type imm_oprnd_size;


enum cris_insn_version_usage applicable_version;


enum cris_op_type op;
};
extern const struct cris_opcode cris_opcodes[];








#define CRIS_DIS_FLAG_MEMREF (1 << 0)


#define CRIS_DIS_FLAG_MEM_TARGET_IS_REG (1 << 1)


#define CRIS_DIS_FLAG_MEM_TARGET2_IS_REG (1 << 2)



#define CRIS_DIS_FLAG_MEM_TARGET2_MULT2 (1 << 3)



#define CRIS_DIS_FLAG_MEM_TARGET2_MULT4 (1 << 4)




#define CRIS_DIS_FLAG_MEM_TARGET2_MEM (1 << 5)



#define CRIS_DIS_FLAG_MEM_TARGET2_MEM_BYTE (1 << 6)



#define CRIS_DIS_FLAG_MEM_TARGET2_MEM_WORD (1 << 7)

#endif 







