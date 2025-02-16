



















#if !defined(_ELF_ARM_H)
#define _ELF_ARM_H

#include "elf/reloc-macros.h"


#define EF_ARM_RELEXEC 0x01
#define EF_ARM_HASENTRY 0x02
#define EF_ARM_INTERWORK 0x04
#define EF_ARM_APCS_26 0x08
#define EF_ARM_APCS_FLOAT 0x10
#define EF_ARM_PIC 0x20
#define EF_ARM_ALIGN8 0x40 
#define EF_ARM_NEW_ABI 0x80
#define EF_ARM_OLD_ABI 0x100
#define EF_ARM_SOFT_FLOAT 0x200
#define EF_ARM_VFP_FLOAT 0x400
#define EF_ARM_MAVERICK_FLOAT 0x800


#define PT_ARM_EXIDX (PT_LOPROC + 1)


#define EF_ARM_SYMSARESORTED 0x04 
#define EF_ARM_DYNSYMSUSESEGIDX 0x08 
#define EF_ARM_MAPSYMSFIRST 0x10 
#define EF_ARM_EABIMASK 0xFF000000


#define EF_ARM_BE8 0x00800000
#define EF_ARM_LE8 0x00400000

#define EF_ARM_EABI_VERSION(flags) ((flags) & EF_ARM_EABIMASK)
#define EF_ARM_EABI_UNKNOWN 0x00000000
#define EF_ARM_EABI_VER1 0x01000000
#define EF_ARM_EABI_VER2 0x02000000
#define EF_ARM_EABI_VER3 0x03000000
#define EF_ARM_EABI_VER4 0x04000000
#define EF_ARM_EABI_VER5 0x05000000


#define F_INTERWORK EF_ARM_INTERWORK
#define F_APCS26 EF_ARM_APCS_26
#define F_APCS_FLOAT EF_ARM_APCS_FLOAT
#define F_PIC EF_ARM_PIC
#define F_SOFT_FLOAT EF_ARM_SOFT_FLOAT
#define F_VFP_FLOAT EF_ARM_VFP_FLOAT


#define STT_ARM_TFUNC STT_LOPROC 
#define STT_ARM_16BIT STT_HIPROC 


#define SHT_ARM_EXIDX 0x70000001 
#define SHT_ARM_PREEMPTMAP 0x70000002 
#define SHT_ARM_ATTRIBUTES 0x70000003 
#define SHT_ARM_DEBUGOVERLAY 0x70000004 
#define SHT_ARM_OVERLAYSECTION 0x70000005 


#define SHF_ENTRYSECT 0x10000000 
#define SHF_COMDEF 0x80000000 


#define PF_ARM_SB 0x10000000 
#define PF_ARM_PI 0x20000000 
#define PF_ARM_ABS 0x40000000 


#define TAG_CPU_ARCH_PRE_V4 0
#define TAG_CPU_ARCH_V4 1
#define TAG_CPU_ARCH_V4T 2
#define TAG_CPU_ARCH_V5T 3
#define TAG_CPU_ARCH_V5TE 4
#define TAG_CPU_ARCH_V5TEJ 5
#define TAG_CPU_ARCH_V6 6
#define TAG_CPU_ARCH_V6KZ 7
#define TAG_CPU_ARCH_V6T2 8
#define TAG_CPU_ARCH_V6K 9
#define TAG_CPU_ARCH_V7 10
#define TAG_CPU_ARCH_V6_M 11
#define TAG_CPU_ARCH_V6S_M 12
#define TAG_CPU_ARCH_V7E_M 13
#define TAG_CPU_ARCH_V8 14
#define MAX_TAG_CPU_ARCH 14


#define TAG_CPU_ARCH_V4T_PLUS_V6_M (MAX_TAG_CPU_ARCH + 1)



START_RELOC_NUMBERS (elf_arm_reloc_type)

RELOC_NUMBER (R_ARM_NONE, 0)
RELOC_NUMBER (R_ARM_PC24, 1) 
RELOC_NUMBER (R_ARM_ABS32, 2)
RELOC_NUMBER (R_ARM_REL32, 3)
RELOC_NUMBER (R_ARM_LDR_PC_G0, 4)
RELOC_NUMBER (R_ARM_ABS16, 5)
RELOC_NUMBER (R_ARM_ABS12, 6)
RELOC_NUMBER (R_ARM_THM_ABS5, 7)
RELOC_NUMBER (R_ARM_ABS8, 8)
RELOC_NUMBER (R_ARM_SBREL32, 9)
RELOC_NUMBER (R_ARM_THM_CALL, 10)
RELOC_NUMBER (R_ARM_THM_PC8, 11)
RELOC_NUMBER (R_ARM_BREL_ADJ, 12)
RELOC_NUMBER (R_ARM_TLS_DESC, 13)
RELOC_NUMBER (R_ARM_THM_SWI8, 14) 
RELOC_NUMBER (R_ARM_XPC25, 15) 
RELOC_NUMBER (R_ARM_THM_XPC22, 16) 
RELOC_NUMBER (R_ARM_TLS_DTPMOD32, 17)
RELOC_NUMBER (R_ARM_TLS_DTPOFF32, 18)
RELOC_NUMBER (R_ARM_TLS_TPOFF32, 19)
RELOC_NUMBER (R_ARM_COPY, 20) 
RELOC_NUMBER (R_ARM_GLOB_DAT, 21) 
RELOC_NUMBER (R_ARM_JUMP_SLOT, 22) 
RELOC_NUMBER (R_ARM_RELATIVE, 23) 
RELOC_NUMBER (R_ARM_GOTOFF32, 24) 
RELOC_NUMBER (R_ARM_BASE_PREL, 25) 
RELOC_NUMBER (R_ARM_GOT_BREL, 26) 
RELOC_NUMBER (R_ARM_PLT32, 27) 
RELOC_NUMBER (R_ARM_CALL, 28)
RELOC_NUMBER (R_ARM_JUMP24, 29)
RELOC_NUMBER (R_ARM_THM_JUMP24, 30)
RELOC_NUMBER (R_ARM_BASE_ABS, 31)
RELOC_NUMBER (R_ARM_ALU_PCREL7_0, 32) 
RELOC_NUMBER (R_ARM_ALU_PCREL15_8, 33) 
RELOC_NUMBER (R_ARM_ALU_PCREL23_15, 34) 
RELOC_NUMBER (R_ARM_LDR_SBREL_11_0, 35) 
RELOC_NUMBER (R_ARM_ALU_SBREL_19_12, 36) 
RELOC_NUMBER (R_ARM_ALU_SBREL_27_20, 37) 
RELOC_NUMBER (R_ARM_TARGET1, 38)
RELOC_NUMBER (R_ARM_SBREL31, 39) 
RELOC_NUMBER (R_ARM_V4BX, 40)
RELOC_NUMBER (R_ARM_TARGET2, 41)
RELOC_NUMBER (R_ARM_PREL31, 42)
RELOC_NUMBER (R_ARM_MOVW_ABS_NC, 43)
RELOC_NUMBER (R_ARM_MOVT_ABS, 44)
RELOC_NUMBER (R_ARM_MOVW_PREL_NC, 45)
RELOC_NUMBER (R_ARM_MOVT_PREL, 46)
RELOC_NUMBER (R_ARM_THM_MOVW_ABS_NC, 47)
RELOC_NUMBER (R_ARM_THM_MOVT_ABS, 48)
RELOC_NUMBER (R_ARM_THM_MOVW_PREL_NC, 49)
RELOC_NUMBER (R_ARM_THM_MOVT_PREL, 50)
RELOC_NUMBER (R_ARM_THM_JUMP19, 51)
RELOC_NUMBER (R_ARM_THM_JUMP6, 52)
RELOC_NUMBER (R_ARM_THM_ALU_PREL_11_0, 53)
RELOC_NUMBER (R_ARM_THM_PC12, 54)
RELOC_NUMBER (R_ARM_ABS32_NOI, 55)
RELOC_NUMBER (R_ARM_REL32_NOI, 56)
RELOC_NUMBER (R_ARM_ALU_PC_G0_NC, 57)
RELOC_NUMBER (R_ARM_ALU_PC_G0, 58)
RELOC_NUMBER (R_ARM_ALU_PC_G1_NC, 59)
RELOC_NUMBER (R_ARM_ALU_PC_G1, 60)
RELOC_NUMBER (R_ARM_ALU_PC_G2, 61)
RELOC_NUMBER (R_ARM_LDR_PC_G1, 62)
RELOC_NUMBER (R_ARM_LDR_PC_G2, 63)
RELOC_NUMBER (R_ARM_LDRS_PC_G0, 64)
RELOC_NUMBER (R_ARM_LDRS_PC_G1, 65)
RELOC_NUMBER (R_ARM_LDRS_PC_G2, 66)
RELOC_NUMBER (R_ARM_LDC_PC_G0, 67)
RELOC_NUMBER (R_ARM_LDC_PC_G1, 68)
RELOC_NUMBER (R_ARM_LDC_PC_G2, 69)
RELOC_NUMBER (R_ARM_ALU_SB_G0_NC, 70)
RELOC_NUMBER (R_ARM_ALU_SB_G0, 71)
RELOC_NUMBER (R_ARM_ALU_SB_G1_NC, 72)
RELOC_NUMBER (R_ARM_ALU_SB_G1, 73)
RELOC_NUMBER (R_ARM_ALU_SB_G2, 74)
RELOC_NUMBER (R_ARM_LDR_SB_G0, 75)
RELOC_NUMBER (R_ARM_LDR_SB_G1, 76)
RELOC_NUMBER (R_ARM_LDR_SB_G2, 77)
RELOC_NUMBER (R_ARM_LDRS_SB_G0, 78)
RELOC_NUMBER (R_ARM_LDRS_SB_G1, 79)
RELOC_NUMBER (R_ARM_LDRS_SB_G2, 80)
RELOC_NUMBER (R_ARM_LDC_SB_G0, 81)
RELOC_NUMBER (R_ARM_LDC_SB_G1, 82)
RELOC_NUMBER (R_ARM_LDC_SB_G2, 83)
RELOC_NUMBER (R_ARM_MOVW_BREL_NC, 84)
RELOC_NUMBER (R_ARM_MOVT_BREL, 85)
RELOC_NUMBER (R_ARM_MOVW_BREL, 86)
RELOC_NUMBER (R_ARM_THM_MOVW_BREL_NC, 87)
RELOC_NUMBER (R_ARM_THM_MOVT_BREL, 88)
RELOC_NUMBER (R_ARM_THM_MOVW_BREL, 89)
RELOC_NUMBER (R_ARM_TLS_GOTDESC, 90)
RELOC_NUMBER (R_ARM_TLS_CALL, 91)
RELOC_NUMBER (R_ARM_TLS_DESCSEQ, 92)
RELOC_NUMBER (R_ARM_THM_TLS_CALL, 93)
RELOC_NUMBER (R_ARM_PLT32_ABS, 94)
RELOC_NUMBER (R_ARM_GOT_ABS, 95)
RELOC_NUMBER (R_ARM_GOT_PREL, 96)
RELOC_NUMBER (R_ARM_GOT_BREL12, 97)
RELOC_NUMBER (R_ARM_GOTOFF12, 98)
RELOC_NUMBER (R_ARM_GOTRELAX, 99)
RELOC_NUMBER (R_ARM_GNU_VTENTRY, 100) 
RELOC_NUMBER (R_ARM_GNU_VTINHERIT, 101) 
RELOC_NUMBER (R_ARM_THM_JUMP11, 102)
RELOC_NUMBER (R_ARM_THM_JUMP8, 103)
RELOC_NUMBER (R_ARM_TLS_GD32, 104)
RELOC_NUMBER (R_ARM_TLS_LDM32, 105)
RELOC_NUMBER (R_ARM_TLS_LDO32, 106)
RELOC_NUMBER (R_ARM_TLS_IE32, 107)
RELOC_NUMBER (R_ARM_TLS_LE32, 108)
RELOC_NUMBER (R_ARM_TLS_LDO12, 109)
RELOC_NUMBER (R_ARM_TLS_LE12, 110)
RELOC_NUMBER (R_ARM_TLS_IE12GP, 111)

RELOC_NUMBER (R_ARM_ME_TOO, 128) 
RELOC_NUMBER (R_ARM_THM_TLS_DESCSEQ ,129)

RELOC_NUMBER (R_ARM_IRELATIVE, 160)


RELOC_NUMBER (R_ARM_RXPC25, 249)
RELOC_NUMBER (R_ARM_RSBREL32, 250)
RELOC_NUMBER (R_ARM_THM_RPC22, 251)
RELOC_NUMBER (R_ARM_RREL32, 252)
RELOC_NUMBER (R_ARM_RABS32, 253)
RELOC_NUMBER (R_ARM_RPC24, 254)
RELOC_NUMBER (R_ARM_RBASE, 255)


FAKE_RELOC (R_ARM_GOTOFF, R_ARM_GOTOFF32) 
FAKE_RELOC (R_ARM_THM_PC22, R_ARM_THM_CALL)
FAKE_RELOC (R_ARM_THM_PC11, R_ARM_THM_JUMP11)
FAKE_RELOC (R_ARM_THM_PC9, R_ARM_THM_JUMP8)



FAKE_RELOC (R_ARM_GOTPC, R_ARM_BASE_PREL) 
FAKE_RELOC (R_ARM_GOT32, R_ARM_GOT_BREL) 
FAKE_RELOC (R_ARM_ROSEGREL32, R_ARM_SBREL31) 
FAKE_RELOC (R_ARM_AMP_VCALL9, R_ARM_BREL_ADJ) 

END_RELOC_NUMBERS (R_ARM_max = 256)

#if defined(BFD_ARCH_SIZE)


enum
{

Tag_CPU_raw_name = 4,
Tag_CPU_name,
Tag_CPU_arch,
Tag_CPU_arch_profile,
Tag_ARM_ISA_use,
Tag_THUMB_ISA_use,
Tag_FP_arch,
Tag_WMMX_arch,
Tag_Advanced_SIMD_arch,
Tag_PCS_config,
Tag_ABI_PCS_R9_use,
Tag_ABI_PCS_RW_data,
Tag_ABI_PCS_RO_data,
Tag_ABI_PCS_GOT_use,
Tag_ABI_PCS_wchar_t,
Tag_ABI_FP_rounding,
Tag_ABI_FP_denormal,
Tag_ABI_FP_exceptions,
Tag_ABI_FP_user_exceptions,
Tag_ABI_FP_number_model,
Tag_ABI_align_needed,
Tag_ABI_align_preserved,
Tag_ABI_enum_size,
Tag_ABI_HardFP_use,
Tag_ABI_VFP_args,
Tag_ABI_WMMX_args,
Tag_ABI_optimization_goals,
Tag_ABI_FP_optimization_goals,

Tag_undefined33 = 33,
Tag_CPU_unaligned_access,
Tag_undefined35,
Tag_FP_HP_extension,
Tag_undefined37,
Tag_ABI_FP_16bit_format,
Tag_undefined39,
Tag_undefined40,
Tag_undefined41,
Tag_MPextension_use,
Tag_undefined_43,
Tag_DIV_use,
Tag_nodefaults = 64,
Tag_also_compatible_with,
Tag_T2EE_use,
Tag_conformance,
Tag_Virtualization_use,
Tag_undefined69,
Tag_MPextension_use_legacy,


Tag_VFP_arch = Tag_FP_arch,
Tag_ABI_align8_needed = Tag_ABI_align_needed,
Tag_ABI_align8_preserved = Tag_ABI_align_preserved,
Tag_VFP_HP_extension = Tag_FP_HP_extension
};

#endif


#define ARM_NOTE_SECTION ".note.gnu.arm.ident"


#define ELF_STRING_ARM_unwind ".ARM.exidx"
#define ELF_STRING_ARM_unwind_info ".ARM.extab"
#define ELF_STRING_ARM_unwind_once ".gnu.linkonce.armexidx."
#define ELF_STRING_ARM_unwind_info_once ".gnu.linkonce.armextab."

enum arm_st_branch_type {
ST_BRANCH_TO_ARM,
ST_BRANCH_TO_THUMB,
ST_BRANCH_LONG,
ST_BRANCH_UNKNOWN
};

#define ARM_SYM_BRANCH_TYPE(SYM) ((enum arm_st_branch_type) (SYM)->st_target_internal)


#endif 
