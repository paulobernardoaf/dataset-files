typedef enum _tricore_opcode_arch_val
{
TRICORE_GENERIC = 0x00000000,
TRICORE_RIDER_A = 0x00000001,
TRICORE_RIDER_B = 0x00000002,
TRICORE_RIDER_D = TRICORE_RIDER_B,
TRICORE_V2 = 0x00000004,
TRICORE_PCP = 0x00000010,
TRICORE_PCP2 = 0x00000020
} tricore_isa;
#define bfd_mach_rider_a 0x0001
#define bfd_mach_rider_b 0x0002
#define bfd_mach_rider_c 0x0003
#define bfd_mach_rider_2 0x0004
#define bfd_mach_rider_d 0x0002
#define bfd_mach_rider_mask 0x000f
#define SEC_ARCH_BIT_0 0x008
#define TRICORE_V2_UP TRICORE_V2
#define TRICORE_RIDER_D_UP (TRICORE_RIDER_D | TRICORE_V2_UP)
#define TRICORE_RIDER_B_UP (TRICORE_RIDER_B | TRICORE_RIDER_D_UP)
#define TRICORE_RIDER_B_DN TRICORE_RIDER_B
#define TRICORE_RIDER_D_DN (TRICORE_RIDER_D | TRICORE_RIDER_B_DN)
#define TRICORE_V2_DN (TRICORE_V2 | TRICORE_RIDER_D_DN)
typedef enum _tricore_fmt
{
TRICORE_FMT_ABS,
TRICORE_FMT_ABSB,
TRICORE_FMT_B,
TRICORE_FMT_BIT,
TRICORE_FMT_BO,
TRICORE_FMT_BOL,
TRICORE_FMT_BRC,
TRICORE_FMT_BRN,
TRICORE_FMT_BRR,
TRICORE_FMT_RC,
TRICORE_FMT_RCPW,
TRICORE_FMT_RCR,
TRICORE_FMT_RCRR,
TRICORE_FMT_RCRW,
TRICORE_FMT_RLC,
TRICORE_FMT_RR,
TRICORE_FMT_RR1,
TRICORE_FMT_RR2,
TRICORE_FMT_RRPW,
TRICORE_FMT_RRR,
TRICORE_FMT_RRR1,
TRICORE_FMT_RRR2,
TRICORE_FMT_RRRR,
TRICORE_FMT_RRRW,
TRICORE_FMT_SYS,
TRICORE_FMT_SB,
TRICORE_FMT_SBC,
TRICORE_FMT_SBR,
TRICORE_FMT_SBRN,
TRICORE_FMT_SC,
TRICORE_FMT_SLR,
TRICORE_FMT_SLRO,
TRICORE_FMT_SR,
TRICORE_FMT_SRC,
TRICORE_FMT_SRO,
TRICORE_FMT_SRR,
TRICORE_FMT_SRRS,
TRICORE_FMT_SSR,
TRICORE_FMT_SSRO,
TRICORE_FMT_MAX 
} tricore_fmt;
#if defined(__STDC__) || defined(ALMOST_STDC)
#define F(x) TRICORE_FMT_ ##x
#elif defined(_MSC_VER)
#define F(x) TRICORE_FMT_ ##x
#else
#define F(x) TRICORE_FMT_x
#endif
extern unsigned long tricore_mask_abs;
extern unsigned long tricore_mask_absb;
extern unsigned long tricore_mask_b;
extern unsigned long tricore_mask_bit;
extern unsigned long tricore_mask_bo;
extern unsigned long tricore_mask_bol;
extern unsigned long tricore_mask_brc;
extern unsigned long tricore_mask_brn;
extern unsigned long tricore_mask_brr;
extern unsigned long tricore_mask_rc;
extern unsigned long tricore_mask_rcpw;
extern unsigned long tricore_mask_rcr;
extern unsigned long tricore_mask_rcrr;
extern unsigned long tricore_mask_rcrw;
extern unsigned long tricore_mask_rlc;
extern unsigned long tricore_mask_rr;
extern unsigned long tricore_mask_rr1;
extern unsigned long tricore_mask_rr2;
extern unsigned long tricore_mask_rrpw;
extern unsigned long tricore_mask_rrr;
extern unsigned long tricore_mask_rrr1;
extern unsigned long tricore_mask_rrr2;
extern unsigned long tricore_mask_rrrr;
extern unsigned long tricore_mask_rrrw;
extern unsigned long tricore_mask_sys;
extern unsigned long tricore_mask_sb;
extern unsigned long tricore_mask_sbc;
extern unsigned long tricore_mask_sbr;
extern unsigned long tricore_mask_sbrn;
extern unsigned long tricore_mask_sc;
extern unsigned long tricore_mask_slr;
extern unsigned long tricore_mask_slro;
extern unsigned long tricore_mask_sr;
extern unsigned long tricore_mask_src;
extern unsigned long tricore_mask_sro;
extern unsigned long tricore_mask_srr;
extern unsigned long tricore_mask_srrs;
extern unsigned long tricore_mask_ssr;
extern unsigned long tricore_mask_ssro;
extern unsigned long tricore_opmask[];
extern void tricore_init_arch_vars PARAMS ((unsigned long));
struct tricore_opcode
{
const char *name; 
const int len32; 
const unsigned long opcode; 
const unsigned long lose; 
const tricore_fmt format; 
const int nr_operands; 
const char *args; 
const char *fields; 
const tricore_isa isa; 
int insind; 
int inslast; 
};
extern struct tricore_opcode tricore_opcodes[];
extern const int tricore_numopcodes;
extern unsigned long tricore_opmask[];
struct pcp_opcode
{
const char *name; 
const int len32; 
const unsigned long opcode; 
const unsigned long lose; 
const int fmt_group; 
const int ooo; 
const int nr_operands; 
const char *args; 
const tricore_isa isa; 
int insind; 
int inslast; 
};
extern struct pcp_opcode pcp_opcodes[];
extern const int pcp_numopcodes;
struct tricore_core_register
{
const char *name; 
const unsigned long addr; 
const tricore_isa isa; 
};
extern const struct tricore_core_register tricore_sfrs[];
extern const int tricore_numsfrs;
#define FMT_ABS_NONE '0'
#define FMT_ABS_OFF18 '1'
#define FMT_ABS_S1_D '2'
#define FMT_ABSB_NONE '0'
#define FMT_ABSB_OFF18 '1'
#define FMT_ABSB_B '2'
#define FMT_ABSB_BPOS3 '3'
#define FMT_B_NONE '0'
#define FMT_B_DISP24 '1'
#define FMT_BIT_NONE '0'
#define FMT_BIT_D '1'
#define FMT_BIT_P2 '2'
#define FMT_BIT_P1 '3'
#define FMT_BIT_S2 '4'
#define FMT_BIT_S1 '5'
#define FMT_BO_NONE '0'
#define FMT_BO_OFF10 '1'
#define FMT_BO_S2 '2'
#define FMT_BO_S1_D '3'
#define FMT_BOL_NONE '0'
#define FMT_BOL_OFF16 '1'
#define FMT_BOL_S2 '2'
#define FMT_BOL_S1_D '3'
#define FMT_BRC_NONE '0'
#define FMT_BRC_DISP15 '1'
#define FMT_BRC_CONST4 '2'
#define FMT_BRC_S1 '3'
#define FMT_BRN_NONE '0'
#define FMT_BRN_DISP15 '1'
#define FMT_BRN_N '2'
#define FMT_BRN_S1 '3'
#define FMT_BRR_NONE '0'
#define FMT_BRR_DISP15 '1'
#define FMT_BRR_S2 '2'
#define FMT_BRR_S1 '3'
#define FMT_RC_NONE '0'
#define FMT_RC_D '1'
#define FMT_RC_CONST9 '2'
#define FMT_RC_S1 '3'
#define FMT_RCPW_NONE '0'
#define FMT_RCPW_D '1'
#define FMT_RCPW_P '2'
#define FMT_RCPW_W '3'
#define FMT_RCPW_CONST4 '4'
#define FMT_RCPW_S1 '5'
#define FMT_RCR_NONE '0'
#define FMT_RCR_D '1'
#define FMT_RCR_S3 '2'
#define FMT_RCR_CONST9 '3'
#define FMT_RCR_S1 '4'
#define FMT_RCRR_NONE '0'
#define FMT_RCRR_D '1'
#define FMT_RCRR_S3 '2'
#define FMT_RCRR_CONST4 '3'
#define FMT_RCRR_S1 '4'
#define FMT_RCRW_NONE '0'
#define FMT_RCRW_D '1'
#define FMT_RCRW_S3 '2'
#define FMT_RCRW_W '3'
#define FMT_RCRW_CONST4 '4'
#define FMT_RCRW_S1 '5'
#define FMT_RLC_NONE '0'
#define FMT_RLC_D '1'
#define FMT_RLC_CONST16 '2'
#define FMT_RLC_S1 '3'
#define FMT_RR_NONE '0'
#define FMT_RR_D '1'
#define FMT_RR_N '2'
#define FMT_RR_S2 '3'
#define FMT_RR_S1 '4'
#define FMT_RR1_NONE '0'
#define FMT_RR1_D '1'
#define FMT_RR1_N '2'
#define FMT_RR1_S2 '3'
#define FMT_RR1_S1 '4'
#define FMT_RR2_NONE '0'
#define FMT_RR2_D '1'
#define FMT_RR2_S2 '2'
#define FMT_RR2_S1 '3'
#define FMT_RRPW_NONE '0'
#define FMT_RRPW_D '1'
#define FMT_RRPW_P '2'
#define FMT_RRPW_W '3'
#define FMT_RRPW_S2 '4'
#define FMT_RRPW_S1 '5'
#define FMT_RRR_NONE '0'
#define FMT_RRR_D '1'
#define FMT_RRR_S3 '2'
#define FMT_RRR_N '3'
#define FMT_RRR_S2 '4'
#define FMT_RRR_S1 '5'
#define FMT_RRR1_NONE '0'
#define FMT_RRR1_D '1'
#define FMT_RRR1_S3 '2'
#define FMT_RRR1_N '3'
#define FMT_RRR1_S2 '4'
#define FMT_RRR1_S1 '5'
#define FMT_RRR2_NONE '0'
#define FMT_RRR2_D '1'
#define FMT_RRR2_S3 '2'
#define FMT_RRR2_S2 '3'
#define FMT_RRR2_S1 '4'
#define FMT_RRRR_NONE '0'
#define FMT_RRRR_D '1'
#define FMT_RRRR_S3 '2'
#define FMT_RRRR_S2 '3'
#define FMT_RRRR_S1 '4'
#define FMT_RRRW_NONE '0'
#define FMT_RRRW_D '1'
#define FMT_RRRW_S3 '2'
#define FMT_RRRW_W '3'
#define FMT_RRRW_S2 '4'
#define FMT_RRRW_S1 '5'
#define FMT_SYS_NONE '0'
#define FMT_SYS_S1_D '1'
#define FMT_SB_NONE '0'
#define FMT_SB_DISP8 '1'
#define FMT_SBC_NONE '0'
#define FMT_SBC_CONST4 '1'
#define FMT_SBC_DISP4 '2'
#define FMT_SBR_NONE '0'
#define FMT_SBR_S2 '1'
#define FMT_SBR_DISP4 '2'
#define FMT_SBRN_NONE '0'
#define FMT_SBRN_N '1'
#define FMT_SBRN_DISP4 '2'
#define FMT_SC_NONE '0'
#define FMT_SC_CONST8 '1'
#define FMT_SLR_NONE '0'
#define FMT_SLR_S2 '1'
#define FMT_SLR_D '2'
#define FMT_SLRO_NONE '0'
#define FMT_SLRO_OFF4 '1'
#define FMT_SLRO_D '2'
#define FMT_SR_NONE '0'
#define FMT_SR_S1_D '1'
#define FMT_SRC_NONE '0'
#define FMT_SRC_CONST4 '1'
#define FMT_SRC_S1_D '2'
#define FMT_SRO_NONE '0'
#define FMT_SRO_S2 '1'
#define FMT_SRO_OFF4 '2'
#define FMT_SRR_NONE '0'
#define FMT_SRR_S2 '1'
#define FMT_SRR_S1_D '2'
#define FMT_SRRS_NONE '0'
#define FMT_SRRS_S2 '1'
#define FMT_SRRS_S1_D '2'
#define FMT_SRRS_N '3'
#define FMT_SSR_NONE '0'
#define FMT_SSR_S2 '1'
#define FMT_SSR_S1 '2'
#define FMT_SSRO_NONE '0'
#define FMT_SSRO_OFF4 '1'
#define FMT_SSRO_S1 '2'
