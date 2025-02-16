#include <r_anal.h>
#include <r_lib.h>
#include <capstone/capstone.h>
#include <capstone/arm.h>
#include "./anal_arm_hacks.inc"
#define esilprintf(op, fmt, ...) r_strbuf_setf (&op->esil, fmt, ##__VA_ARGS__)
#define IMM64(x) (ut64)(insn->detail->arm64.operands[x].imm)
#define INSOP64(x) insn->detail->arm64.operands[x]
#define REG(x) r_str_get (cs_reg_name (*handle, insn->detail->arm.operands[x].reg))
#define REG64(x) r_str_get (cs_reg_name (*handle, insn->detail->arm64.operands[x].reg))
#define REGID64(x) insn->detail->arm64.operands[x].reg
#define REGID(x) insn->detail->arm.operands[x].reg
#define IMM(x) (ut32)(insn->detail->arm.operands[x].imm)
#define INSOP(x) insn->detail->arm.operands[x]
#define MEMBASE(x) r_str_get (cs_reg_name (*handle, insn->detail->arm.operands[x].mem.base))
#define MEMBASE64(x) r_str_get (cs_reg_name (*handle, insn->detail->arm64.operands[x].mem.base))
#define REGBASE(x) insn->detail->arm.operands[x].mem.base
#define REGBASE64(x) insn->detail->arm64.operands[x].mem.base
#define MEMINDEX(x) r_str_get (cs_reg_name (*handle, insn->detail->arm.operands[x].mem.index))
#define HASMEMINDEX(x) (insn->detail->arm.operands[x].mem.index != ARM_REG_INVALID)
#define MEMINDEX64(x) r_str_get (cs_reg_name (*handle, insn->detail->arm64.operands[x].mem.index))
#define HASMEMINDEX64(x) (insn->detail->arm64.operands[x].mem.index != ARM64_REG_INVALID)
#define MEMDISP(x) insn->detail->arm.operands[x].mem.disp
#define MEMDISP64(x) (ut64)insn->detail->arm64.operands[x].mem.disp
#define ISIMM(x) (insn->detail->arm.operands[x].type == ARM_OP_IMM)
#define ISIMM64(x) (insn->detail->arm64.operands[x].type == ARM64_OP_IMM)
#define ISREG(x) (insn->detail->arm.operands[x].type == ARM_OP_REG)
#define ISREG64(x) (insn->detail->arm64.operands[x].type == ARM64_OP_REG)
#define ISMEM(x) (insn->detail->arm.operands[x].type == ARM_OP_MEM)
#define ISMEM64(x) (insn->detail->arm64.operands[x].type == ARM64_OP_MEM)
#if CS_API_MAJOR > 3
#define LSHIFT(x) insn->detail->arm.operands[x].mem.lshift
#define LSHIFT2(x) insn->detail->arm.operands[x].shift.value 
#define LSHIFT2_64(x) insn->detail->arm64.operands[x].shift.value
#else
#define LSHIFT(x) 0
#define LSHIFT2(x) 0
#define LSHIFT2_64(x) 0
#endif
#define OPCOUNT() insn->detail->arm.op_count
#define OPCOUNT64() insn->detail->arm64.op_count
#define ISSHIFTED(x) (insn->detail->arm.operands[x].shift.type != ARM_SFT_INVALID && insn->detail->arm.operands[x].shift.value != 0)
#define SHIFTTYPE(x) insn->detail->arm.operands[x].shift.type
#define SHIFTVALUE(x) insn->detail->arm.operands[x].shift.value
#define ISWRITEBACK64() (insn->detail->arm64.writeback == true)
#define ISPREINDEX32() ((OPCOUNT64() == 2) && (ISMEM64(1)) && (ISWRITEBACK64()))
#define ISPOSTINDEX32() ((OPCOUNT64() == 3) && (ISIMM64(2)) && (ISWRITEBACK64()))
#define ISPREINDEX64() ((OPCOUNT64() == 3) && (ISMEM64(2)) && (ISWRITEBACK64()))
#define ISPOSTINDEX64() ((OPCOUNT64() == 4) && (ISIMM64(3)) && (ISWRITEBACK64()))
static RRegItem base_regs[4];
static RRegItem regdelta_regs[4];
static const ut64 bitmask_by_width[] = {
0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff, 0x1ff, 0x3ff, 0x7ff,
0xfff, 0x1fff, 0x3fff, 0x7fff, 0xffff, 0x1ffff, 0x3ffff, 0x7ffff,
0xfffff, 0x1fffff, 0x3fffff, 0x7fffff, 0xffffff, 0x1ffffffLL, 0x3ffffffLL,
0x7ffffffLL, 0xfffffffLL, 0x1fffffffLL, 0x3fffffffLL, 0x7fffffffLL, 0xffffffffLL,
0x1ffffffffLL, 0x3ffffffffLL, 0x7ffffffffLL, 0xfffffffffLL, 0x1fffffffffLL,
0x3fffffffffLL, 0x7fffffffffLL, 0xffffffffffLL, 0x1ffffffffffLL, 0x3ffffffffffLL,
0x7ffffffffffLL, 0xfffffffffffLL, 0x1fffffffffffLL, 0x3fffffffffffLL, 0x7fffffffffffLL,
0xffffffffffffLL, 0x1ffffffffffffLL, 0x3ffffffffffffLL, 0x7ffffffffffffLL,
0xfffffffffffffLL, 0x1fffffffffffffLL, 0x3fffffffffffffLL, 0x7fffffffffffffLL,
0xffffffffffffffLL, 0x1ffffffffffffffLL, 0x3ffffffffffffffLL, 0x7ffffffffffffffLL,
0xfffffffffffffffLL, 0x1fffffffffffffffLL, 0x3fffffffffffffffLL, 0x7fffffffffffffffLL, 0xffffffffffffffffLL
};
static const char *shift_type_name(arm_shifter type) {
switch (type) {
case ARM_SFT_ASR:
return "asr";
case ARM_SFT_LSL:
return "lsl";
case ARM_SFT_LSR:
return "lsr";
case ARM_SFT_ROR:
return "ror";
case ARM_SFT_RRX:
return "rrx";
case ARM_SFT_ASR_REG:
return "asr_reg";
case ARM_SFT_LSL_REG:
return "lsl_reg";
case ARM_SFT_LSR_REG:
return "lsr_reg";
case ARM_SFT_ROR_REG:
return "ror_reg";
case ARM_SFT_RRX_REG:
return "rrx_reg";
default:
return "";
}
}
static const char *vector_data_type_name(arm_vectordata_type type) {
switch (type) {
case ARM_VECTORDATA_I8:
return "i8";
case ARM_VECTORDATA_I16:
return "i16";
case ARM_VECTORDATA_I32:
return "i32";
case ARM_VECTORDATA_I64:
return "i64";
case ARM_VECTORDATA_S8:
return "s8";
case ARM_VECTORDATA_S16:
return "s16";
case ARM_VECTORDATA_S32:
return "s32";
case ARM_VECTORDATA_S64:
return "s64";
case ARM_VECTORDATA_U8:
return "u8";
case ARM_VECTORDATA_U16:
return "u16";
case ARM_VECTORDATA_U32:
return "u32";
case ARM_VECTORDATA_U64:
return "u64";
case ARM_VECTORDATA_P8:
return "p8";
case ARM_VECTORDATA_F32:
return "f32";
case ARM_VECTORDATA_F64:
return "f64";
case ARM_VECTORDATA_F16F64:
return "f16.f64";
case ARM_VECTORDATA_F64F16:
return "f64.f16";
case ARM_VECTORDATA_F32F16:
return "f32.f16";
case ARM_VECTORDATA_F16F32:
return "f16.f32";
case ARM_VECTORDATA_F64F32:
return "f64.f32";
case ARM_VECTORDATA_F32F64:
return "f32.f64";
case ARM_VECTORDATA_S32F32:
return "s32.f32";
case ARM_VECTORDATA_U32F32:
return "u32.f32";
case ARM_VECTORDATA_F32S32:
return "f32.s32";
case ARM_VECTORDATA_F32U32:
return "f32.u32";
case ARM_VECTORDATA_F64S16:
return "f64.s16";
case ARM_VECTORDATA_F32S16:
return "f32.s16";
case ARM_VECTORDATA_F64S32:
return "f64.s32";
case ARM_VECTORDATA_S16F64:
return "s16.f64";
case ARM_VECTORDATA_S16F32:
return "s16.f64";
case ARM_VECTORDATA_S32F64:
return "s32.f64";
case ARM_VECTORDATA_U16F64:
return "u16.f64";
case ARM_VECTORDATA_U16F32:
return "u16.f32";
case ARM_VECTORDATA_U32F64:
return "u32.f64";
case ARM_VECTORDATA_F64U16:
return "f64.u16";
case ARM_VECTORDATA_F32U16:
return "f32.u16";
case ARM_VECTORDATA_F64U32:
return "f64.u32";
default:
return "";
}
}
static const char *cc_name(arm_cc cc) {
switch (cc) {
case ARM_CC_EQ: 
return "eq";
case ARM_CC_NE: 
return "ne";
case ARM_CC_HS: 
return "hs";
case ARM_CC_LO: 
return "lo";
case ARM_CC_MI: 
return "mi";
case ARM_CC_PL: 
return "pl";
case ARM_CC_VS: 
return "vs";
case ARM_CC_VC: 
return "vc";
case ARM_CC_HI: 
return "hi";
case ARM_CC_LS: 
return "ls";
case ARM_CC_GE: 
return "ge";
case ARM_CC_LT: 
return "lt";
case ARM_CC_GT: 
return "gt";
case ARM_CC_LE: 
return "le";
default:
return "";
}
}
static void opex(RStrBuf *buf, csh handle, cs_insn *insn) {
int i;
r_strbuf_init (buf);
r_strbuf_append (buf, "{");
cs_arm *x = &insn->detail->arm;
r_strbuf_append (buf, "\"operands\":[");
for (i = 0; i < x->op_count; i++) {
cs_arm_op *op = &x->operands[i];
if (i > 0) {
r_strbuf_append (buf, ",");
}
r_strbuf_append (buf, "{");
switch (op->type) {
case ARM_OP_REG:
r_strbuf_append (buf, "\"type\":\"reg\"");
r_strbuf_appendf (buf, ",\"value\":\"%s\"", cs_reg_name (handle, op->reg));
break;
case ARM_OP_IMM:
r_strbuf_append (buf, "\"type\":\"imm\"");
r_strbuf_appendf (buf, ",\"value\":%d", (st32)op->imm);
break;
case ARM_OP_MEM:
r_strbuf_append (buf, "\"type\":\"mem\"");
if (op->mem.base != ARM_REG_INVALID) {
r_strbuf_appendf (buf, ",\"base\":\"%s\"", cs_reg_name (handle, op->mem.base));
}
if (op->mem.index != ARM_REG_INVALID) {
r_strbuf_appendf (buf, ",\"index\":\"%s\"", cs_reg_name (handle, op->mem.index));
}
r_strbuf_appendf (buf, ",\"scale\":%d", op->mem.scale);
r_strbuf_appendf (buf, ",\"disp\":%d", op->mem.disp);
break;
case ARM_OP_FP:
r_strbuf_append (buf, "\"type\":\"fp\"");
r_strbuf_appendf (buf, ",\"value\":%lf", op->fp);
break;
case ARM_OP_CIMM:
r_strbuf_append (buf, "\"type\":\"cimm\"");
r_strbuf_appendf (buf, ",\"value\":%d", (st32)op->imm);
break;
case ARM_OP_PIMM:
r_strbuf_append (buf, "\"type\":\"pimm\"");
r_strbuf_appendf (buf, ",\"value\":%d", (st32)op->imm);
break;
case ARM_OP_SETEND:
r_strbuf_append (buf, "\"type\":\"setend\"");
switch (op->setend) {
case ARM_SETEND_BE:
r_strbuf_append (buf, ",\"value\":\"be\"");
break;
case ARM_SETEND_LE:
r_strbuf_append (buf, ",\"value\":\"le\"");
break;
default:
r_strbuf_append (buf, ",\"value\":\"invalid\"");
break;
}
break;
case ARM_OP_SYSREG:
r_strbuf_append (buf, "\"type\":\"sysreg\"");
r_strbuf_appendf (buf, ",\"value\":\"%s\"", cs_reg_name (handle, op->reg));
break;
default:
r_strbuf_append (buf, ",\"type\":\"invalid\"");
break;
}
if (op->shift.type != ARM_SFT_INVALID) {
r_strbuf_append (buf, ",\"shift\":{");
switch (op->shift.type) {
case ARM_SFT_ASR:
case ARM_SFT_LSL:
case ARM_SFT_LSR:
case ARM_SFT_ROR:
case ARM_SFT_RRX:
r_strbuf_appendf (buf, "\"type\":\"%s\"", shift_type_name (op->shift.type));
r_strbuf_appendf (buf, ",\"value\":\"%u\"", op->shift.value);
break;
case ARM_SFT_ASR_REG:
case ARM_SFT_LSL_REG:
case ARM_SFT_LSR_REG:
case ARM_SFT_ROR_REG:
case ARM_SFT_RRX_REG:
r_strbuf_appendf (buf, "\"type\":\"%s\"", shift_type_name (op->shift.type));
r_strbuf_appendf (buf, ",\"value\":\"%d\"", cs_reg_name (handle, op->shift.value));
break;
default:
break;
}
r_strbuf_append (buf, "}");
}
if (op->vector_index != -1) {
r_strbuf_appendf (buf, ",\"vector_index\":\"%d\"", op->vector_index);
}
if (op->subtracted) {
r_strbuf_append (buf, ",\"subtracted\":true");
}
r_strbuf_append (buf, "}");
}
r_strbuf_append (buf, "]");
if (x->usermode) {
r_strbuf_append (buf, ",\"usermode\":true");
}
if (x->update_flags) {
r_strbuf_append (buf, ",\"update_flags\":true");
}
if (x->writeback) {
r_strbuf_append (buf, ",\"writeback\":true");
}
if (x->vector_size) {
r_strbuf_appendf (buf, ",\"vector_size\":%d", x->vector_size);
}
if (x->vector_data != ARM_VECTORDATA_INVALID) {
r_strbuf_appendf (buf, ",\"vector_data\":\"%s\"", vector_data_type_name (x->vector_data));
}
if (x->cps_mode != ARM_CPSMODE_INVALID) {
r_strbuf_appendf (buf, ",\"cps_mode\":%d", x->cps_mode);
}
if (x->cps_flag != ARM_CPSFLAG_INVALID) {
r_strbuf_appendf (buf, ",\"cps_flag\":%d", x->cps_flag);
}
if (x->cc != ARM_CC_INVALID && x->cc != ARM_CC_AL) {
r_strbuf_appendf (buf, ",\"cc\":\"%s\"", cc_name (x->cc));
}
if (x->mem_barrier != ARM_MB_INVALID) {
r_strbuf_appendf (buf, ",\"mem_barrier\":%d", x->mem_barrier - 1);
}
r_strbuf_append (buf, "}");
}
static int arm64_reg_width(int reg) {
switch (reg) {
case ARM64_REG_W0:
case ARM64_REG_W1:
case ARM64_REG_W2:
case ARM64_REG_W3:
case ARM64_REG_W4:
case ARM64_REG_W5:
case ARM64_REG_W6:
case ARM64_REG_W7:
case ARM64_REG_W8:
case ARM64_REG_W9:
case ARM64_REG_W10:
case ARM64_REG_W11:
case ARM64_REG_W12:
case ARM64_REG_W13:
case ARM64_REG_W14:
case ARM64_REG_W15:
case ARM64_REG_W16:
case ARM64_REG_W17:
case ARM64_REG_W18:
case ARM64_REG_W19:
case ARM64_REG_W20:
case ARM64_REG_W21:
case ARM64_REG_W22:
case ARM64_REG_W23:
case ARM64_REG_W24:
case ARM64_REG_W25:
case ARM64_REG_W26:
case ARM64_REG_W27:
case ARM64_REG_W28:
case ARM64_REG_W29:
case ARM64_REG_W30:
return 32;
break;
default:
break;
}
return 64;
}
static const char *cc_name64(arm64_cc cc) {
switch (cc) {
case ARM64_CC_EQ: 
return "eq";
case ARM64_CC_NE: 
return "ne";
case ARM64_CC_HS: 
return "hs";
case ARM64_CC_LO: 
return "lo";
case ARM64_CC_MI: 
return "mi";
case ARM64_CC_PL: 
return "pl";
case ARM64_CC_VS: 
return "vs";
case ARM64_CC_VC: 
return "vc";
case ARM64_CC_HI: 
return "hi";
case ARM64_CC_LS: 
return "ls";
case ARM64_CC_GE: 
return "ge";
case ARM64_CC_LT: 
return "lt";
case ARM64_CC_GT: 
return "gt";
case ARM64_CC_LE: 
return "le";
default:
return "";
}
}
static const char *extender_name(arm64_extender extender) {
switch (extender) {
case ARM64_EXT_UXTB:
return "uxtb";
case ARM64_EXT_UXTH:
return "uxth";
case ARM64_EXT_UXTW:
return "uxtw";
case ARM64_EXT_UXTX:
return "uxtx";
case ARM64_EXT_SXTB:
return "sxtb";
case ARM64_EXT_SXTH:
return "sxth";
case ARM64_EXT_SXTW:
return "sxtw";
case ARM64_EXT_SXTX:
return "sxtx";
default:
return "";
}
}
static const char *vas_name(arm64_vas vas) {
switch (vas) {
case ARM64_VAS_8B:
return "8b";
case ARM64_VAS_16B:
return "16b";
case ARM64_VAS_4H:
return "4h";
case ARM64_VAS_8H:
return "8h";
case ARM64_VAS_2S:
return "2s";
case ARM64_VAS_4S:
return "4s";
case ARM64_VAS_2D:
return "2d";
case ARM64_VAS_1D:
return "1d";
case ARM64_VAS_1Q:
return "1q";
#if CS_API_MAJOR > 4
case ARM64_VAS_1B:
return "8b";
case ARM64_VAS_4B:
return "8b";
case ARM64_VAS_2H:
return "2h";
case ARM64_VAS_1H:
return "1h";
case ARM64_VAS_1S:
return "1s";
#endif
default:
return "";
}
}
#if CS_API_MAJOR == 4
static const char *vess_name(arm64_vess vess) {
switch (vess) {
case ARM64_VESS_B:
return "b";
case ARM64_VESS_H:
return "h";
case ARM64_VESS_S:
return "s";
case ARM64_VESS_D:
return "d";
default:
return "";
}
}
#endif
static void opex64(RStrBuf *buf, csh handle, cs_insn *insn) {
int i;
r_strbuf_init (buf);
r_strbuf_append (buf, "{");
cs_arm64 *x = &insn->detail->arm64;
r_strbuf_append (buf, "\"operands\":[");
for (i = 0; i < x->op_count; i++) {
cs_arm64_op *op = &x->operands[i];
if (i > 0) {
r_strbuf_append (buf, ",");
}
r_strbuf_append (buf, "{");
switch (op->type) {
case ARM64_OP_REG:
r_strbuf_append (buf, "\"type\":\"reg\"");
r_strbuf_appendf (buf, ",\"value\":\"%s\"", cs_reg_name (handle, op->reg));
break;
case ARM64_OP_REG_MRS:
r_strbuf_append (buf, "\"type\":\"reg_mrs\"");
break;
case ARM64_OP_REG_MSR:
r_strbuf_append (buf, "\"type\":\"reg_msr\"");
break;
case ARM64_OP_IMM:
r_strbuf_append (buf, "\"type\":\"imm\"");
r_strbuf_appendf (buf, ",\"value\":%"PFMT64d, op->imm);
break;
case ARM64_OP_MEM:
r_strbuf_append (buf, "\"type\":\"mem\"");
if (op->mem.base != ARM64_REG_INVALID) {
r_strbuf_appendf (buf, ",\"base\":\"%s\"", cs_reg_name (handle, op->mem.base));
}
if (op->mem.index != ARM64_REG_INVALID) {
r_strbuf_appendf (buf, ",\"index\":\"%s\"", cs_reg_name (handle, op->mem.index));
}
r_strbuf_appendf (buf, ",\"disp\":%d", op->mem.disp);
break;
case ARM64_OP_FP:
r_strbuf_append (buf, "\"type\":\"fp\"");
r_strbuf_appendf (buf, ",\"value\":%lf", op->fp);
break;
case ARM64_OP_CIMM:
r_strbuf_append (buf, "\"type\":\"cimm\"");
r_strbuf_appendf (buf, ",\"value\":%"PFMT64d, op->imm);
break;
case ARM64_OP_PSTATE:
r_strbuf_append (buf, "\"type\":\"pstate\"");
switch (op->pstate) {
case ARM64_PSTATE_SPSEL:
r_strbuf_append (buf, ",\"value\":\"spsel\"");
break;
case ARM64_PSTATE_DAIFSET:
r_strbuf_append (buf, ",\"value\":\"daifset\"");
break;
case ARM64_PSTATE_DAIFCLR:
r_strbuf_append (buf, ",\"value\":\"daifclr\"");
break;
default:
r_strbuf_appendf (buf, ",\"value\":%d", op->pstate);
}
break;
case ARM64_OP_SYS:
r_strbuf_append (buf, "\"type\":\"sys\"");
r_strbuf_appendf (buf, ",\"value\":%u", op->sys);
break;
case ARM64_OP_PREFETCH:
r_strbuf_append (buf, "\"type\":\"prefetch\"");
r_strbuf_appendf (buf, ",\"value\":%d", op->prefetch - 1);
break;
case ARM64_OP_BARRIER:
r_strbuf_append (buf, "\"type\":\"prefetch\"");
r_strbuf_appendf (buf, ",\"value\":%d", op->barrier - 1);
break;
default:
r_strbuf_append (buf, ",\"type\":\"invalid\"");
break;
}
if (op->shift.type != ARM64_SFT_INVALID) {
r_strbuf_append (buf, ",\"shift\":{");
switch (op->shift.type) {
case ARM64_SFT_LSL:
r_strbuf_append (buf, "\"type\":\"lsl\"");
break;
case ARM64_SFT_MSL:
r_strbuf_append (buf, "\"type\":\"msl\"");
break;
case ARM64_SFT_LSR:
r_strbuf_append (buf, "\"type\":\"lsr\"");
break;
case ARM64_SFT_ASR:
r_strbuf_append (buf, "\"type\":\"asr\"");
break;
case ARM64_SFT_ROR:
r_strbuf_append (buf, "\"type\":\"ror\"");
break;
default:
break;
}
r_strbuf_appendf (buf, ",\"value\":\"%u\"", op->shift.value);
r_strbuf_append (buf, "}");
}
if (op->ext != ARM64_EXT_INVALID) {
r_strbuf_appendf (buf, ",\"ext\":\"%s\"", extender_name (op->ext));
}
if (op->vector_index != -1) {
r_strbuf_appendf (buf, ",\"vector_index\":\"%d\"", op->vector_index);
}
if (op->vas != ARM64_VAS_INVALID) {
r_strbuf_appendf (buf, ",\"vas\":\"%s\"", vas_name (op->vas));
}
#if CS_API_MAJOR == 4
if (op->vess != ARM64_VESS_INVALID) {
r_strbuf_appendf (buf, ",\"vess\":\"%s\"", vess_name (op->vess));
}
#endif
r_strbuf_append (buf, "}");
}
r_strbuf_append (buf, "]");
if (x->update_flags) {
r_strbuf_append (buf, ",\"update_flags\":true");
}
if (x->writeback) {
r_strbuf_append (buf, ",\"writeback\":true");
}
if (x->cc != ARM64_CC_INVALID && x->cc != ARM64_CC_AL && x->cc != ARM64_CC_NV) {
r_strbuf_appendf (buf, ",\"cc\":\"%s\"", cc_name64 (x->cc));
}
r_strbuf_append (buf, "}");
}
static const char *decode_shift(arm_shifter shift) {
static const char *E_OP_SR = ">>";
static const char *E_OP_SL = "<<";
static const char *E_OP_RR = ">>>";
static const char *E_OP_ASR = ">>>>";
static const char *E_OP_VOID = "";
switch (shift) {
case ARM_SFT_ASR:
case ARM_SFT_ASR_REG:
return E_OP_ASR;
case ARM_SFT_LSR:
case ARM_SFT_LSR_REG:
return E_OP_SR;
case ARM_SFT_LSL:
case ARM_SFT_LSL_REG:
return E_OP_SL;
case ARM_SFT_ROR:
case ARM_SFT_RRX:
case ARM_SFT_ROR_REG:
case ARM_SFT_RRX_REG:
return E_OP_RR;
default:
break;
}
return E_OP_VOID;
}
static const char *decode_shift_64(arm64_shifter shift) {
static const char *E_OP_SR = ">>";
static const char *E_OP_SL = "<<";
static const char *E_OP_RR = ">>>";
static const char *E_OP_VOID = "";
switch (shift) {
case ARM64_SFT_ASR:
case ARM64_SFT_LSR:
return E_OP_SR;
case ARM64_SFT_LSL:
case ARM64_SFT_MSL:
return E_OP_SL;
case ARM64_SFT_ROR:
return E_OP_RR;
default:
break;
}
return E_OP_VOID;
}
#define DECODE_SHIFT(x) decode_shift(insn->detail->arm.operands[x].shift.type)
#define DECODE_SHIFT64(x) decode_shift_64(insn->detail->arm64.operands[x].shift.type)
static int regsize64(cs_insn *insn, int n) {
unsigned int reg = insn->detail->arm64.operands[n].reg;
if ( (reg >= ARM64_REG_S0 && reg <= ARM64_REG_S31) ||
(reg >= ARM64_REG_W0 && reg <= ARM64_REG_W30) ||
reg == ARM64_REG_WZR) {
return 4;
}
if (reg >= ARM64_REG_B0 && reg <= ARM64_REG_B31) {
return 1;
}
if (reg >= ARM64_REG_H0 && reg <= ARM64_REG_H31) {
return 2;
}
if (reg >= ARM64_REG_Q0 && reg <= ARM64_REG_Q31) {
return 16;
}
return 8;
}
#define REGSIZE64(x) regsize64 (insn, x)
const char* arm_prefix_cond(RAnalOp *op, int cond_type) {
const char *close_cond[2];
close_cond[0] = "\0";
close_cond[1] = ",}\0";
int close_type = 0;
switch (cond_type) {
case ARM_CC_EQ:
close_type = 1;
r_strbuf_setf (&op->esil, "zf,?{,");
break;
case ARM_CC_NE:
close_type = 1;
r_strbuf_setf (&op->esil, "zf,!,?{,");
break;
case ARM_CC_HS:
close_type = 1;
r_strbuf_setf (&op->esil, "cf,?{,");
break;
case ARM_CC_LO:
close_type = 1;
r_strbuf_setf (&op->esil, "cf,!,?{,");
break;
case ARM_CC_MI:
close_type = 1;
r_strbuf_setf (&op->esil, "nf,?{,");
break;
case ARM_CC_PL:
close_type = 1;
r_strbuf_setf (&op->esil, "nf,!,?{,");
break;
case ARM_CC_VS:
close_type = 1;
r_strbuf_setf (&op->esil, "vf,?{,");
break;
case ARM_CC_VC:
close_type = 1;
r_strbuf_setf (&op->esil, "vf,!,?{,");
break;
case ARM_CC_HI:
close_type = 1;
r_strbuf_setf (&op->esil, "cf,zf,!,&,?{,");
break;
case ARM_CC_LS:
close_type = 1;
r_strbuf_setf (&op->esil, "cf,!,zf,!,|,?{,");
break;
case ARM_CC_GE:
close_type = 1;
r_strbuf_setf (&op->esil, "nf,vf,^,!,?{,");
break;
case ARM_CC_LT:
close_type = 1;
r_strbuf_setf (&op->esil, "nf,vf,^,?{,");
break;
case ARM_CC_GT:
close_type = 1;
r_strbuf_setf (&op->esil, "zf,!,nf,vf,^,!,&,?{,");
break;
case ARM_CC_LE:
close_type = 1;
r_strbuf_setf (&op->esil, "zf,nf,vf,^,|,?{,");
break;
case ARM_CC_AL:
break;
default:
break;
}
return close_cond[close_type];
}
static const char *arg(RAnal *a, csh *handle, cs_insn *insn, char *buf, int n) {
buf[0] = 0;
switch (insn->detail->arm.operands[n].type) {
case ARM_OP_REG:
if (ISSHIFTED (n)) {
sprintf (buf, "%u,%s,%s",
LSHIFT2 (n),
r_str_get (cs_reg_name (*handle,
insn->detail->arm.operands[n].reg)),
DECODE_SHIFT (n));
} else {
sprintf (buf, "%s",
r_str_get (cs_reg_name (*handle,
insn->detail->arm.operands[n].reg)));
}
break;
case ARM_OP_IMM:
if (a->bits == 64) {
sprintf (buf, "%"PFMT64d, (ut64)
insn->detail->arm.operands[n].imm);
} else {
sprintf (buf, "%"PFMT64d, (ut64)(ut32)
insn->detail->arm.operands[n].imm);
}
break;
case ARM_OP_MEM:
break;
case ARM_OP_FP:
sprintf (buf, "%lf", insn->detail->arm.operands[n].fp);
break;
default:
break;
}
return buf;
}
#define ARG(x) arg(a, handle, insn, str[x], x)
#define SHIFTED_REG64_APPEND(sb, n) shifted_reg64_append(sb, handle, insn, n)
static void shifted_reg64_append(RStrBuf *sb, csh *handle, cs_insn *insn, int n) {
if (insn->detail->arm64.operands[n].shift.type != ARM64_SFT_ASR) {
r_strbuf_appendf (sb, "%d,%s,%s", LSHIFT2_64(n), REG64(n), DECODE_SHIFT64(n));
} else {
int index = LSHIFT2_64(n) - 1;
if (index < 0) {
return;
}
ut64 missing_ones = bitmask_by_width[index] << (REGSIZE64(n)*8 - LSHIFT2_64(n));
r_strbuf_appendf (sb, "%d,%s,%s,1,%s,<<<,1,&,?{,%"PFMT64u",}{,0,},|",
LSHIFT2_64(n), REG64(n), DECODE_SHIFT64(n), REG64(n), (ut64)missing_ones);
}
}
#define OPCALL(opchar) arm64math(a, op, addr, buf, len, handle, insn, opchar, 0)
#define OPCALL_NEG(opchar) arm64math(a, op, addr, buf, len, handle, insn, opchar, 1)
static void arm64math(RAnal *a, RAnalOp *op, ut64 addr, const ut8 *buf, int len, csh *handle, cs_insn *insn, const char *opchar, int negate) {
const char *r0 = REG64(0);
const char *r1 = REG64(1);
if (ISREG64(2)) {
if (LSHIFT2_64 (2)) {
SHIFTED_REG64_APPEND(&op->esil, 2);
if (negate) {
r_strbuf_appendf (&op->esil, ",-1,^");
}
if (REGID64(0) == REGID64(1)) {
r_strbuf_appendf (&op->esil, ",%s,%s=", r0, opchar);
} else {
r_strbuf_appendf (&op->esil, ",%s,%s,%s,=", r1, opchar, r0);
}
} else {
const char *r2 = REG64(2);
if (negate) {
if (REGID64(0) == REGID64(1)) {
r_strbuf_setf (&op->esil, "%s,-1,^,%s,%s=", r2, r0, opchar);
} else {
r_strbuf_setf (&op->esil, "%s,-1,^,%s,%s,%s,=", r2, r1, opchar, r0);
}
} else {
if (REGID64(0) == REGID64(1)) {
r_strbuf_setf (&op->esil, "%s,%s,%s=", r2, r0, opchar);
} else {
r_strbuf_setf (&op->esil, "%s,%s,%s,%s,=", r2, r1, opchar, r0);
}
}
}
} else {
ut64 i2 = IMM64(2);
if (negate) {
if (REGID64(0) == REGID64(1)) {
r_strbuf_setf (&op->esil, "%"PFMT64d",-1,^,%s,%s=", i2, r0, opchar);
} else {
r_strbuf_setf (&op->esil, "%"PFMT64d",-1,^,%s,%s,%s,=", i2, r1, opchar, r0);
}
} else {
if (REGID64(0) == REGID64(1)) {
r_strbuf_setf (&op->esil, "%"PFMT64d",%s,%s=", i2, r0, opchar);
} else {
r_strbuf_setf (&op->esil, "%"PFMT64d",%s,%s,%s,=", i2, r1, opchar, r0);
}
}
}
}
static int analop64_esil(RAnal *a, RAnalOp *op, ut64 addr, const ut8 *buf, int len, csh *handle, cs_insn *insn) {
const char *postfix = NULL;
r_strbuf_init (&op->esil);
r_strbuf_set (&op->esil, "");
postfix = arm_prefix_cond (op, insn->detail->arm64.cc);
switch (insn->id) {
case ARM64_INS_REV:
{
const char *r0 = REG64(0);
const char *r1 = REG64(1);
int size = REGSIZE64(1);
#if 0
r_strbuf_setf (&op->esil,
"0,%s,=," 
"%d," 
"DUP," 
"DUP,1,SWAP,-,8,*," 
"DUP,0xff,<<,%s,&,>>," 
"SWAP,%d,-,8,*," 
"SWAP,<<,%s,|=," 
"4,REPEAT", 
r0, size, r1, size, r0);
#endif
if (size == 8) {
r_strbuf_setf (&op->esil,
"56,0xff,%s,&,<<,%s,=,"
"48,0xff,8,%s,>>,&,<<,%s,|=,"
"40,0xff,16,%s,>>,&,<<,%s,|=,"
"32,0xff,24,%s,>>,&,<<,%s,|=,"
"24,0xff,32,%s,>>,&,<<,%s,|=,"
"16,0xff,40,%s,>>,&,<<,%s,|=,"
"8,0xff,48,%s,>>,&,<<,%s,|=,"
"0xff,56,%s,>>,&,%s,|=,",
r1, r0, r1, r0, r1, r0, r1, r0,
r1, r0, r1, r0, r1, r0, r1, r0);
} else {
r_strbuf_setf (&op->esil,
"24,0xff,%s,&,<<,%s,=,"
"16,0xff,8,%s,>>,&,<<,%s,|=,"
"8,0xff,16,%s,>>,&,<<,%s,|=,"
"0xff,24,%s,>>,&,%s,|=,",
r1, r0, r1, r0, r1, r0, r1, r0);
}
break;
}
case ARM64_INS_REV32:
{
const char *r0 = REG64(0);
const char *r1 = REG64(1);
r_strbuf_setf (&op->esil,
"24,0x000000ff000000ff,%s,&,<<,%s,=,"
"16,0x000000ff000000ff,8,%s,>>,&,<<,%s,|=,"
"8,0x000000ff000000ff,16,%s,>>,&,<<,%s,|=,"
"0x000000ff000000ff,24,%s,>>,&,%s,|=,",
r1, r0, r1, r0, r1, r0, r1, r0);
break;
}
case ARM64_INS_REV16:
{
const char *r0 = REG64(0);
const char *r1 = REG64(1);
r_strbuf_setf (&op->esil,
"8,0xff00ff00ff00ff00,%s,&,>>,%s,=,"
"8,0x00ff00ff00ff00ff,%s,&,<<,%s,|=,",
r1, r0, r1, r0);
break;
}
case ARM64_INS_ADR:
r_strbuf_setf (&op->esil,
"%"PFMT64d",%s,=", IMM64(1), REG64(0));
break;
case ARM64_INS_UMADDL:
case ARM64_INS_SMADDL:
case ARM64_INS_FMADD:
case ARM64_INS_MADD:
r_strbuf_setf (&op->esil, "%s,%s,*,%s,+,%s,=",
REG64 (2), REG64 (1), REG64 (3), REG64 (0));
break;
case ARM64_INS_MSUB:
r_strbuf_setf (&op->esil, "%s,%s,*,%s,-,%s,=",
REG64 (2), REG64 (1), REG64 (3), REG64 (0));
break;
case ARM64_INS_ADD:
case ARM64_INS_ADC: 
OPCALL("+");
break;
case ARM64_INS_SUB:
OPCALL("-");
break;
case ARM64_INS_SMULL:
case ARM64_INS_MUL:
OPCALL("*");
break;
case ARM64_INS_AND:
OPCALL("&");
break;
case ARM64_INS_ORR:
OPCALL("|");
break;
case ARM64_INS_EOR:
OPCALL("^");
break;
case ARM64_INS_ORN:
OPCALL_NEG("|");
break;
case ARM64_INS_EON:
OPCALL_NEG("^");
break;
case ARM64_INS_LSR:
OPCALL(">>");
break;
case ARM64_INS_LSL:
OPCALL("<<");
break;
case ARM64_INS_ROR:
OPCALL(">>>");
break;
case ARM64_INS_STURB: 
break;
case ARM64_INS_NOP:
r_strbuf_setf (&op->esil, ",");
break;
case ARM64_INS_FDIV:
case ARM64_INS_SDIV:
case ARM64_INS_UDIV:
r_strbuf_setf (&op->esil, "%s,%s,/=", REG64 (1), REG64 (0));
break;
case ARM64_INS_BR:
r_strbuf_setf (&op->esil, "%s,pc,=", REG64(0));
break;
case ARM64_INS_B:
r_strbuf_appendf (&op->esil, "%"PFMT64d",pc,=", IMM64 (0));
break;
case ARM64_INS_BL:
r_strbuf_setf (&op->esil, "pc,lr,=,%"PFMT64d",pc,=", IMM64 (0));
break;
case ARM64_INS_BLR:
r_strbuf_setf (&op->esil, "pc,lr,=,%s,pc,=", REG64 (0));
break;
case ARM64_INS_LDRH:
case ARM64_INS_LDUR:
case ARM64_INS_LDR:
case ARM64_INS_LDRSB:
case ARM64_INS_LDRB:
case ARM64_INS_LDRSW:
case ARM64_INS_LDURSW:
{
int size = REGSIZE64 (0);
switch (insn->id) {
case ARM64_INS_LDRSB:
case ARM64_INS_LDRB:
size = 1;
break;
case ARM64_INS_LDRH:
size = 2;
break;
case ARM64_INS_LDRSW:
case ARM64_INS_LDURSW:
size = 4;
break;
default:
break;
}
if (ISMEM64(1)) {
if (HASMEMINDEX64(1)) {
if (LSHIFT2_64(1)) {
r_strbuf_appendf (&op->esil, "%s,%d,%s,%s,+,[%d],%s,=",
MEMBASE64(1), LSHIFT2_64(1), MEMINDEX64(1), DECODE_SHIFT64(1), size, REG64(0));
} else {
r_strbuf_appendf (&op->esil, "%s,%s,+,[%d],%s,=",
MEMBASE64(1), MEMINDEX64(1), size, REG64(0));
}
} else {
if (LSHIFT2_64(1)) {
r_strbuf_appendf (&op->esil, "%s,%d,%"PFMT64d",%s,+,[%d],%s,=",
MEMBASE64(1), LSHIFT2_64(1), MEMDISP64(1), DECODE_SHIFT64(1), size, REG64(0));
} else if ((int)MEMDISP64(1) < 0){
r_strbuf_appendf (&op->esil, "%"PFMT64d",%s,-,DUP,tmp,=,[%d],%s,=,",
-(int)MEMDISP64(1), MEMBASE64(1), size, REG64(0));
} else {
r_strbuf_appendf (&op->esil, "%"PFMT64d",%s,+,DUP,tmp,=,[%d],%s,=,",
MEMDISP64(1), MEMBASE64(1), size, REG64(0));
}
}
op->refptr = 4;
} else {
if (ISREG64(1)) {
if (OPCOUNT64() == 2) {
r_strbuf_setf (&op->esil, "%s,[%d],%s,=",
REG64(1), size, REG64(0));
} else if (OPCOUNT64() == 3) {
if (ISREG64(2)) {
r_strbuf_setf (&op->esil, "%s,%s,+,[%d],%s,=",
REG64(1), REG64(2), size, REG64(0));
}
}
} else {
r_strbuf_setf (&op->esil, "%"PFMT64d",[%d],%s,=",
IMM64(1), size, REG64(0));
}
}
break;
}
case ARM64_INS_FCMP:
case ARM64_INS_CCMP:
case ARM64_INS_CCMN:
case ARM64_INS_TST: 
case ARM64_INS_CMP: 
case ARM64_INS_CMN: 
{
int bits = arm64_reg_width(REGID64(0));
if (ISIMM64(1)) {
r_strbuf_setf (&op->esil, "%"PFMT64d",%s,==,$z,zf,:=,%d,$s,nf,:=,%d,$b,!,cf,:=,%d,$o,vf,:=", IMM64(1), REG64(0), bits - 1, bits, bits - 1);
} else {
r_strbuf_setf (&op->esil, "%s,%s,==,$z,zf,:=,%d,$s,nf,:=,%d,$b,!,cf,:=,%d,$o,vf,:=", REG64(1), REG64(0), bits - 1, bits, bits -1);
}
break;
}
case ARM64_INS_FCSEL:
case ARM64_INS_CSEL: 
r_strbuf_appendf (&op->esil, "%s,}{,%s,},%s,=", REG64(1), REG64(2), REG64(0));
postfix = "";
break;
case ARM64_INS_CSET: 
r_strbuf_appendf (&op->esil, "1,}{,0,},%s,=", REG64(0));
postfix = "";
break;
case ARM64_INS_CINC: 
r_strbuf_appendf (&op->esil, "1,%s,+,}{,%s,},%s,=", REG64(1), REG64(1), REG64(0));
postfix = "";
break;
case ARM64_INS_CSINC: 
r_strbuf_appendf (&op->esil, "%s,}{,1,%s,+,},%s,=", REG64(1), REG64(2), REG64(0));
postfix = "";
break;
case ARM64_INS_STXRB:
case ARM64_INS_STXRH:
case ARM64_INS_STXR:
{
int size = REGSIZE64(1);
if (insn->id == ARM64_INS_STXRB) {
size = 1;
} else if (insn->id == ARM64_INS_STXRH) {
size = 2;
}
r_strbuf_setf (&op->esil, "%s,%s,%"PFMT64d",+,=[%d]",
REG64(1), MEMBASE64(1), MEMDISP64(1), size);
break;
}
case ARM64_INS_STRB:
case ARM64_INS_STRH:
case ARM64_INS_STUR:
case ARM64_INS_STR: 
{
int size = REGSIZE64(0);
int disp = MEMDISP64(1);
char sign = disp>=0?'+':'-';
ut64 abs = disp>=0? MEMDISP64(1): -MEMDISP64(1);
if (insn->id == ARM64_INS_STRB) {
size = 1;
} else if (insn->id == ARM64_INS_STRH) {
size = 2;
}
if (ISPREINDEX32()) {
r_strbuf_setf (&op->esil, "%"PFMT64d",%s,%c=,%s,0x%"PFMT64x",%s,%c,=[%d]",
abs, MEMBASE64(1), sign,
REG64(0), abs, MEMBASE64(1), sign, size);
} else if (ISPOSTINDEX32()) {
int val = IMM64(2);
sign = val>=0?'+':'-';
abs = val>=0? val: -val;
r_strbuf_setf (&op->esil, "%s,%s,=[%d],%"PFMT64d",%s,%c=",
REG64(0), MEMBASE64(1), size,
abs, MEMBASE64(1), sign);
} else {
r_strbuf_setf (&op->esil, "%s,0x%"PFMT64x",%s,%c,=[%d]",
REG64(0), abs, MEMBASE64(1), sign, size);
}
break;
}
case ARM64_INS_BIC:
if (OPCOUNT64 () == 2) {
if (REGSIZE64(0) == 4) {
r_strbuf_appendf (&op->esil, "%s,0xffffffff,^,%s,&=",REG64 (1),REG64 (0));
} else {
r_strbuf_appendf (&op->esil, "%s,0xffffffffffffffff,^,%s,&=",REG64 (1),REG64 (0));
}
} else {
if (REGSIZE64(0) == 4) {
r_strbuf_appendf (&op->esil, "%s,0xffffffff,^,%s,&,%s,=",REG64 (2),REG64 (1),REG64 (0));
} else {
r_strbuf_appendf (&op->esil, "%s,0xffffffffffffffff,^,%s,&,%s,=",REG64 (2),REG64 (1),REG64 (0));
}
}
break;
case ARM64_INS_CBZ:
r_strbuf_setf (&op->esil, "%s,!,?{,%"PFMT64d",pc,=,}",
REG64(0), IMM64(1));
break;
case ARM64_INS_CBNZ:
r_strbuf_setf (&op->esil, "%s,?{,%"PFMT64d",pc,=,}",
REG64(0), IMM64(1));
break;
case ARM64_INS_TBZ:
r_strbuf_setf (&op->esil, "%d,1,<<,%s,&,!,?{,%"PFMT64d",pc,=,}",
IMM64(1), REG64(0), IMM64(2));
break;
case ARM64_INS_TBNZ:
r_strbuf_setf (&op->esil, "%d,1,<<,%s,&,?{,%"PFMT64d",pc,=,}",
IMM64(1), REG64(0), IMM64(2));
break;
case ARM64_INS_STP: 
{
int disp = (int)MEMDISP64(2);
char sign = disp>=0?'+':'-';
ut64 abs = disp>=0? MEMDISP64(2): -MEMDISP64(2);
int size = REGSIZE64(0);
if (ISPREINDEX64()) {
r_strbuf_setf(&op->esil,
"%"PFMT64d",%s,%c=,%s,%s,=[%d],%s,%s,%d,+,=[%d]",
abs, MEMBASE64(2), sign,
REG64(0), MEMBASE64(2), size,
REG64(1), MEMBASE64(2), size, size);
} else if (ISPOSTINDEX64()) {
int val = IMM64(3);
sign = val>=0?'+':'-';
abs = val>=0? val: -val;
r_strbuf_setf(&op->esil,
"%s,%s,=[%d],%s,%s,%d,+,=[%d],%d,%s,%c=",
REG64(0), MEMBASE64(2), size,
REG64(1), MEMBASE64(2), size, size,
abs, MEMBASE64(2), sign);
} else {
r_strbuf_setf (&op->esil,
"%s,%s,%"PFMT64d",%c,=[%d],"
"%s,%s,%"PFMT64d",%c,%d,+,=[%d]",
REG64(0), MEMBASE64(2), abs, sign, size,
REG64(1), MEMBASE64(2), abs, sign, size, size);
}
}
break;
case ARM64_INS_LDP: 
{
int disp = (int)MEMDISP64(2);
char sign = disp>=0?'+':'-';
ut64 abs = disp>=0? MEMDISP64(2): -MEMDISP64(2);
int size = REGSIZE64(0);
if (ISPREINDEX64()) {
r_strbuf_setf (&op->esil,
"%"PFMT64d",%s,%c=,"
"%s,[%d],%s,=,"
"%s,%d,+,[%d],%s,=",
abs, MEMBASE64(2), sign,
MEMBASE64(2), size, REG64(0),
MEMBASE64(2), size, size, REG64(1));
} else if (ISPOSTINDEX64()) {
int val = IMM64(3);
sign = val>=0?'+':'-';
abs = val>=0? val: -val;
r_strbuf_setf (&op->esil,
"%s,[%d],%s,=,"
"%s,%d,+,[%d],%s,=,"
"%d,%s,%c=",
MEMBASE64(2), size, REG64(0),
MEMBASE64(2), size, size, REG64(1),
abs, MEMBASE64(2), sign);
} else {
r_strbuf_setf (&op->esil,
"%s,%"PFMT64d",%c,[%d],%s,=,"
"%s,%"PFMT64d",%c,%d,%c,[%d],%s,=",
MEMBASE64(2), abs, sign, size, REG64(0),
MEMBASE64(2), abs, sign, size, sign, size, REG64(1));
}
}
break;
case ARM64_INS_ADRP:
r_strbuf_setf (&op->esil, "%"PFMT64d",%s,=",
IMM64 (1), REG64 (0));
break;
case ARM64_INS_MOV:
if (ISREG64(1)) {
r_strbuf_setf (&op->esil, "%s,%s,=", REG64 (1), REG64 (0));
} else {
r_strbuf_setf (&op->esil, "%"PFMT64d",%s,=", IMM64 (1), REG64 (0));
}
break;
case ARM64_INS_MOVN:
if (ISREG64(1)) {
r_strbuf_setf (&op->esil, "%s,-1,^,%s,=", REG64 (1), REG64 (0));
} else {
r_strbuf_setf (&op->esil, "%"PFMT64d",-1,^,%s,=", IMM64 (1), REG64 (0));
}
break;
case ARM64_INS_MOVK: 
{
ut64 shift = LSHIFT2_64 (1);
if (shift < 0) {
shift = 0;
} else if (shift > 48) {
shift = 48;
}
ut64 shifted_imm = IMM64(1) << shift;
ut64 mask = ~(0xffffLL << shift);
r_strbuf_setf (&op->esil, "%"PFMT64u",%s,&,%"PFMT64u",|,%s,=",
mask,
REG64(0),
shifted_imm,
REG64(0));
break;
}
case ARM64_INS_MOVZ:
r_strbuf_setf (&op->esil, "%"PFMT64u",%s,=",
IMM64(1) << LSHIFT2_64(1),
REG64 (0));
break;
case ARM64_INS_ASR:
OPCALL(">>>>");
break;
case ARM64_INS_SXTB:
if (arm64_reg_width(REGID64(0)) == 32) {
r_strbuf_setf (&op->esil, "%s,%s,=,8,%s,>>,%s,%s,=,%s,%s,&=,31,$c,?{,0xffffff00,%s,|=}",
REG64(1), REG64(0), REG64(1), REG64(1), REG64(0),
"0xff", REG64(0), REG64(0));
} else {
r_strbuf_setf (&op->esil, "%s,%s,=,8,%s,>>,%s,%s,=,%s,%s,&=,63,$c,?{,0xffffffffffffff00,%s,|=}",
REG64(1), REG64(0), REG64(1), REG64(1), REG64(0),
"0xff", REG64(0), REG64(0));
}
break;
case ARM64_INS_SXTH: 
if (arm64_reg_width(REGID64(0)) == 32) {
r_strbuf_setf (&op->esil, "%s,%s,=,16,%s,>>,%s,%s,=,%s,%s,&=,31,$c,?{,0xffff0000,%s,|=}",
REG64(1), REG64(0), REG64(1), REG64(1), REG64(0),
"0xffff", REG64(0), REG64(0));
} else {
r_strbuf_setf (&op->esil, "%s,%s,=,16,%s,>>,%s,%s,=,%s,%s,&=,63,$c,?{,0xffffffffffffff00,%s,|=}",
REG64(1), REG64(0), REG64(1), REG64(1), REG64(0),
"0xffff", REG64(0), REG64(0));
}
break;
case ARM64_INS_SXTW: 
r_strbuf_setf (&op->esil, "%s,%s,=,32,%s,>>,%s,%s,=,%s,%s,&=,63,$c,?{,0xffffffffffffff00,%s,|=}",
REG64(1), REG64(0), REG64(1), REG64(1), REG64(0),
"0xffffffff", REG64(0), REG64(0));
break;
case ARM64_INS_UXTB:
r_strbuf_setf (&op->esil, "%s,0xff,&,%s,=", REG64(1),REG64(0));
break;
case ARM64_INS_UXTH:
r_strbuf_setf (&op->esil, "%s,0xffff,&,%s,=", REG64(1),REG64(0));
break;
case ARM64_INS_RET:
r_strbuf_setf (&op->esil, "lr,pc,=");
break;
case ARM64_INS_ERET:
r_strbuf_setf (&op->esil, "lr,pc,=");
break;
case ARM64_INS_BFI: 
case ARM64_INS_BFXIL:
{
if (OPCOUNT64() >= 3 && ISIMM64 (3) && IMM64 (3) > 0) {
ut64 mask = bitmask_by_width[IMM64 (3) - 1];
ut64 shift = IMM64 (2);
ut64 notmask = ~(mask << shift);
r_strbuf_setf (&op->esil, "%"PFMT64u",%s,&,%"PFMT64u",%"PFMT64u",%s,&,<<,|,%s,=",
notmask, REG64 (0), shift, mask, REG64 (1), REG64 (0));
}
break;
}
case ARM64_INS_UBFX:
if (IMM64 (3) > 0 && IMM64 (3) <= 64 - IMM64 (2)) {
r_strbuf_appendf (&op->esil, "%d,%s,%d,%"PFMT64u",<<,&,>>,%s,=",
IMM64 (2), REG64 (1), IMM64 (2) , (ut64)bitmask_by_width[IMM64 (3) - 1], REG64 (0));
}
break;
case ARM64_INS_NEG:
#if CS_API_MAJOR > 3
case ARM64_INS_NEGS:
#endif
if (LSHIFT2_64 (1)) {
SHIFTED_REG64_APPEND (&op->esil, 1);
} else {
r_strbuf_appendf (&op->esil, "%s", REG64 (1));
}
r_strbuf_appendf (&op->esil, ",0,-,%s,=", REG64 (0));
break;
case ARM64_INS_SVC:
r_strbuf_setf (&op->esil, "%u,$", IMM64 (0));
break;
}
r_strbuf_append (&op->esil, postfix);
return 0;
}
#define MATH32(opchar) arm32math(a, op, addr, buf, len, handle, insn, pcdelta, str, opchar, 0)
#define MATH32_NEG(opchar) arm32math(a, op, addr, buf, len, handle, insn, pcdelta, str, opchar, 1)
#define MATH32AS(opchar) arm32mathaddsub(a, op, addr, buf, len, handle, insn, pcdelta, str, opchar)
static void arm32math(RAnal *a, RAnalOp *op, ut64 addr, const ut8 *buf, int len, csh *handle, cs_insn *insn, int pcdelta, char (*str)[32], const char *opchar, int negate) {
const char *dest = ARG(0);
const char *op1;
const char *op2;
bool rotate_imm = OPCOUNT() > 3;
if (OPCOUNT() > 2) {
op1 = ARG(1);
op2 = ARG(2);
} else {
op1 = dest;
op2 = ARG(1);
}
if (rotate_imm) {
r_strbuf_appendf (&op->esil, "%s,", ARG(3));
}
if (!strcmp (op2, "pc")) {
r_strbuf_appendf (&op->esil, "%d,$$,+", pcdelta);
} else {
r_strbuf_appendf (&op->esil, "%s", op2);
}
if (rotate_imm) {
r_strbuf_appendf (&op->esil, ",>>>");
}
if (negate) {
r_strbuf_appendf (&op->esil, ",-1,^");
}
if (!strcmp (op1, "pc")) {
r_strbuf_appendf (&op->esil, ",%d,$$,+,%s,0xffffffff,&,%s,=", pcdelta, opchar, dest);
} else {
r_strbuf_appendf (&op->esil, ",%s,%s,0xffffffff,&,%s,=", op1, opchar, dest);
}
}
static void arm32mathaddsub(RAnal *a, RAnalOp *op, ut64 addr, const ut8 *buf, int len, csh *handle, cs_insn *insn, int pcdelta, char (*str)[32], const char *opchar) {
const char *dst = ARG(0);
const char *src;
bool noflags = false;
if (!strcmp (dst, "pc")) { 
noflags = true;
}
if (OPCOUNT() == 3) {
r_strbuf_appendf (&op->esil, "%s,0xffffffff,&,%s,=,", ARG (1), dst);
src = ARG (2);
} else {
src = ARG (1);
}
r_strbuf_appendf (&op->esil, "%s,%s,%s,0xffffffff,&,%s,=", src, dst, opchar, dst);
if (noflags) {
return;
}
r_strbuf_appendf (&op->esil, ",$z,zf,:=,%s,cf,:=,vf,=,0,nf,=",
(!strcmp (opchar, "+") ? "30,$c,31,$c,^,31,$c" : "30,$c,31,$c,^,32,$b"));
}
static int analop_esil(RAnal *a, RAnalOp *op, ut64 addr, const ut8 *buf, int len, csh *handle, cs_insn *insn, bool thumb) {
int i;
const char *postfix = NULL;
char str[32][32];
int msr_flags;
int pcdelta = (thumb ? 4 : 8);
ut32 mask = UT32_MAX;
int str_ldr_bytes = 4;
r_strbuf_init (&op->esil);
r_strbuf_set (&op->esil, "");
postfix = arm_prefix_cond (op, insn->detail->arm.cc);
switch (insn->id) {
case ARM_INS_IT:
r_strbuf_appendf (&op->esil, "%d,pc,+=", op->fail);
break;
case ARM_INS_BKPT:
r_strbuf_setf (&op->esil, "%d,%d,TRAP", IMM (0), IMM (0));
break;
case ARM_INS_NOP:
r_strbuf_setf (&op->esil, ",");
break;
case ARM_INS_BL:
case ARM_INS_BLX:
r_strbuf_append (&op->esil, "pc,lr,=,");
case ARM_INS_BX:
case ARM_INS_BXJ:
case ARM_INS_B:
if (ISREG(0) && REGID(0) == ARM_REG_PC) {
r_strbuf_appendf (&op->esil, "0x%"PFMT64x",pc,=", (addr & ~3LL) + pcdelta);
} else {
r_strbuf_appendf (&op->esil, "%s,pc,=", ARG(0));
}
break;
case ARM_INS_UDF:
r_strbuf_setf (&op->esil, "%s,TRAP", ARG(0));
break;
case ARM_INS_SADD16:
case ARM_INS_SADD8:
MATH32AS("+");
break;
case ARM_INS_ADDW:
case ARM_INS_ADD:
case ARM_INS_ADC:
MATH32("+");
break;
case ARM_INS_SSUB16:
case ARM_INS_SSUB8:
MATH32AS("-");
break;
case ARM_INS_SUBW:
case ARM_INS_SUB:
MATH32("-");
break;
case ARM_INS_MUL:
MATH32("*");
break;
case ARM_INS_AND:
MATH32("&");
break;
case ARM_INS_ORR:
MATH32("|");
break;
case ARM_INS_EOR:
MATH32("^");
break;
case ARM_INS_ORN:
MATH32_NEG("|");
break;
case ARM_INS_LSR:
MATH32(">>");
break;
case ARM_INS_LSL:
MATH32("<<");
break;
case ARM_INS_SVC:
r_strbuf_setf (&op->esil, "%s,$", ARG(0));
break;
case ARM_INS_PUSH:
#if 0
PUSH { r4, r5, r6, r7, lr }
4,sp,-=,lr,sp,=[4],
4,sp,-=,r7,sp,=[4],
4,sp,-=,r6,sp,=[4],
4,sp,-=,r5,sp,=[4],
4,sp,-=,r4,sp,=[4]
20,sp,-=,lr,r7,r6,r5,r4,5,sp,=[*]
#endif
r_strbuf_appendf (&op->esil, "%d,sp,-=,",
4 * insn->detail->arm.op_count);
for (i=insn->detail->arm.op_count; i>0; i--) {
r_strbuf_appendf (&op->esil, "%s,", REG (i-1));
}
r_strbuf_appendf (&op->esil, "%d,sp,=[*]",
insn->detail->arm.op_count);
break;
case ARM_INS_STM:
r_strbuf_setf (&op->esil, "");
for (i=1; i<insn->detail->arm.op_count; i++) {
r_strbuf_appendf (&op->esil, "%s,%s,%d,%c,=[4],",
REG (i), ARG (0), R_ABS(i*4), i > 0? '+' : '-');
}
if (insn->detail->arm.writeback == true) { 
r_strbuf_appendf (&op->esil, "%d,%s,+=,",
(insn->detail->arm.op_count -1)*4, ARG (0));
}
break;
case ARM_INS_ASR:
if (OPCOUNT() == 2) {
r_strbuf_appendf (&op->esil, "%s,%s,>>>>,%s,=", ARG(1), ARG(0), ARG(0));
} else if (OPCOUNT() == 3) {
r_strbuf_appendf (&op->esil, "%s,%s,>>>>,%s,=", ARG(2), ARG(1), ARG(0));
}
break;
case ARM_INS_POP:
#if 0
POP { r4,r5, r6}
r6,r5,r4,3,sp,[*],12,sp,+=
#endif
for (i=insn->detail->arm.op_count; i>0; i--) {
r_strbuf_appendf (&op->esil, "%s,", REG (i-1));
}
r_strbuf_appendf (&op->esil, "%d,sp,[*],",
insn->detail->arm.op_count);
r_strbuf_appendf (&op->esil, "%d,sp,+=",
4 * insn->detail->arm.op_count);
break;
case ARM_INS_LDM:
{
const char *comma = "";
for (i=1; i<insn->detail->arm.op_count; i++) {
r_strbuf_appendf (&op->esil, "%s%s,%d,+,[4],%s,=",
comma, ARG (0), (i - 1) * 4, REG (i));
comma = ",";
}
}
break;
case ARM_INS_CMP:
r_strbuf_appendf (&op->esil, "%s,%s,==,$z,zf,:=,31,$s,nf,:=,32,$b,!,cf,:=,31,$o,vf,:=", ARG(1), ARG(0));
break;
case ARM_INS_CMN:
r_strbuf_appendf (&op->esil, "%s,%s,^,!,!,zf,=", ARG(1), ARG(0));
break;
case ARM_INS_MOVT:
r_strbuf_appendf (&op->esil, "16,%s,<<,%s,|=", ARG(1), REG(0));
break;
case ARM_INS_ADR:
r_strbuf_appendf (&op->esil, "%d,$$,+,%s,+,0xfffffffc,&,%s,=",
pcdelta, ARG(1), REG(0));
break;
case ARM_INS_MOV:
case ARM_INS_VMOV:
case ARM_INS_MOVW:
r_strbuf_appendf (&op->esil, "%s,%s,=", ARG(1), REG(0));
break;
case ARM_INS_CBZ:
r_strbuf_appendf (&op->esil, "%s,!,?{,%"PFMT32u",pc,=,}",
REG(0), IMM(1));
break;
case ARM_INS_CBNZ:
r_strbuf_appendf (&op->esil, "%s,?{,%"PFMT32u",pc,=,}",
REG(0), IMM(1));
break;
case ARM_INS_STRT:
case ARM_INS_STR:
case ARM_INS_STRHT:
case ARM_INS_STRH:
case ARM_INS_STRBT:
case ARM_INS_STRB:
case ARM_INS_STRD:
switch(insn->id) {
case ARM_INS_STRD:
str_ldr_bytes = 8; 
break;
case ARM_INS_STRHT:
case ARM_INS_STRH:
str_ldr_bytes = 2;
break;
case ARM_INS_STRBT:
case ARM_INS_STRB:
str_ldr_bytes = 1;
break;
default:
str_ldr_bytes = 4;
}
if (OPCOUNT() == 2) {
if (ISMEM(1) && !HASMEMINDEX(1)) {
int disp = MEMDISP(1);
char sign = disp>=0?'+':'-';
disp = disp>=0?disp:-disp;
r_strbuf_appendf (&op->esil, "%s,0x%x,%s,%c,0xffffffff,&,=[%d]",
REG(0), disp, MEMBASE(1), sign, str_ldr_bytes);
if (insn->detail->arm.writeback) {
r_strbuf_appendf (&op->esil, ",%d,%s,%c,%s,=",
disp, MEMBASE(1), sign, MEMBASE(1));
}
}
if (HASMEMINDEX(1)) { 
if (ISSHIFTED(1)) { 
switch (SHIFTTYPE(1)) {
case ARM_SFT_LSL:
r_strbuf_appendf (&op->esil, "%s,%s,%d,%s,<<,+,0xffffffff,&,=[%d]",
REG(0), MEMBASE(1), SHIFTVALUE(1), MEMINDEX(1), str_ldr_bytes);
if (insn->detail->arm.writeback) { 
r_strbuf_appendf (&op->esil, ",%s,%d,%s,<<,+,%s,=",
MEMBASE(1), SHIFTVALUE(1), MEMINDEX(1), MEMBASE(1));
}
break;
case ARM_SFT_LSR:
r_strbuf_appendf (&op->esil, "%s,%s,%d,%s,>>,+,0xffffffff,&,=[%d]",
REG(0), MEMBASE(1), SHIFTVALUE(1), MEMINDEX(1), str_ldr_bytes);
if (insn->detail->arm.writeback) {
r_strbuf_appendf (&op->esil, ",%s,%d,%s,>>,+,%s,=",
MEMBASE(1), SHIFTVALUE(1), MEMINDEX(1), MEMBASE(1));
}
break;
case ARM_SFT_ASR:
r_strbuf_appendf (&op->esil, "%s,%s,%d,%s,>>>>,+,0xffffffff,&,=[%d]",
REG(0), MEMBASE(1), SHIFTVALUE(1), MEMINDEX(1), str_ldr_bytes);
if (insn->detail->arm.writeback) {
r_strbuf_appendf (&op->esil, ",%s,%d,%s,>>>>,+,%s,=",
MEMBASE(1), SHIFTVALUE(1), MEMINDEX(1), MEMBASE(1));
}
break;
case ARM_SFT_ROR:
r_strbuf_appendf (&op->esil, "%s,%s,%d,%s,>>>,+,0xffffffff,&,=[%d]",
REG(0), MEMBASE(1), SHIFTVALUE(1), MEMINDEX(1), str_ldr_bytes);
if (insn->detail->arm.writeback) {
r_strbuf_appendf (&op->esil, ",%s,%d,%s,>>>,+,%s,=",
MEMBASE(1), SHIFTVALUE(1), MEMINDEX(1), MEMBASE(1));
}
break;
case ARM_SFT_RRX: 
break;
default:
break;
}
} else { 
r_strbuf_appendf (&op->esil, "%s,%s,%s,+,0xffffffff,&,=[%d]",
REG(0), MEMINDEX(1), MEMBASE(1), str_ldr_bytes);
if (insn->detail->arm.writeback) {
r_strbuf_appendf (&op->esil, ",%s,%s,+,%s,=",
MEMINDEX(1), MEMBASE(1), MEMBASE(1));
}
}
}
}
if (OPCOUNT() == 3) { 
if (ISIMM(2)) { 
r_strbuf_appendf (&op->esil, "%s,%s,0xffffffff,&,=[%d],%d,%s,+=",
REG(0), MEMBASE(1), str_ldr_bytes, IMM(2), MEMBASE(1));
}
if (ISREG(2)) { 
if (ISSHIFTED(2)) { 
switch (SHIFTTYPE(2)) {
case ARM_SFT_LSL:
r_strbuf_appendf (&op->esil, "%s,%s,0xffffffff,&,=[%d],%s,%d,%s,<<,+,%s,=",
REG(0), MEMBASE(1), str_ldr_bytes, MEMBASE(1), SHIFTVALUE(2), REG(2), MEMBASE(1));
break;
case ARM_SFT_LSR:
r_strbuf_appendf (&op->esil, "%s,%s,0xffffffff,&,=[%d],%s,%d,%s,>>,+,%s,=",
REG(0), MEMBASE(1), str_ldr_bytes, MEMBASE(1), SHIFTVALUE(2), REG(2), MEMBASE(1));
break;
case ARM_SFT_ASR:
r_strbuf_appendf (&op->esil, "%s,%s,0xffffffff,&,=[%d],%s,%d,%s,>>>>,+,%s,=",
REG(0), MEMBASE(1), str_ldr_bytes, MEMBASE(1), SHIFTVALUE(2), REG(2), MEMBASE(1));
break;
case ARM_SFT_ROR:
r_strbuf_appendf (&op->esil, "%s,%s,0xffffffff,&,=[%d],%s,%d,%s,>>>,+,%s,=",
REG(0), MEMBASE(1), str_ldr_bytes, MEMBASE(1), SHIFTVALUE(2), REG(2), MEMBASE(1));
break;
case ARM_SFT_RRX:
break;
default:
break;
}
} else { 
r_strbuf_appendf (&op->esil, "%s,%s,0xffffffff,&,=[%d],%s,%s,+=",
REG(0), MEMBASE(1), str_ldr_bytes, REG(2), MEMBASE(1));
}
}
if (ISREG(1) && str_ldr_bytes==8) { 
if (!HASMEMINDEX(2)) {
int disp = MEMDISP(2);
char sign = disp>=0?'+':'-';
disp = disp>=0?disp:-disp;
r_strbuf_appendf (&op->esil, "%s,%d,%s,%c,0xffffffff,&,=[4],%s,4,%d,+,%s,%c,0xffffffff,&,=[4]",
REG(0), disp, MEMBASE(2), sign, REG(1), disp, MEMBASE(2), sign);
if (insn->detail->arm.writeback) {
r_strbuf_appendf (&op->esil, ",%d,%s,%c,%s,=",
disp, MEMBASE(2), sign, MEMBASE(2));
}
} else {
if (ISSHIFTED(2)) {
} else {
r_strbuf_appendf (&op->esil, "%s,%s,%s,+,0xffffffff,&,=[4],%s,4,%s,+,%s,+,0xffffffff,&,=[4]",
REG(0), MEMINDEX(2), MEMBASE(2), REG(1), MEMINDEX(2), MEMBASE(2));
if (insn->detail->arm.writeback) {
r_strbuf_appendf (&op->esil, ",%s,%s,+,%s,=",
MEMINDEX(2), MEMBASE(2), MEMBASE(2));
}
}
}
}
}
if (OPCOUNT() == 4) { 
if (ISIMM(3)) { 
r_strbuf_appendf (&op->esil, "%s,%s,0xffffffff,&,=[%d],%s,4,%s,+,0xffffffff,&,=[%d],%d,%s,+=,",
REG(0), MEMBASE(2), str_ldr_bytes, REG(1), MEMBASE(2), str_ldr_bytes, IMM(3), MEMBASE(2));
}
if (ISREG(3)) { 
if (ISSHIFTED(3)) {
} else {
r_strbuf_appendf (&op->esil, "%s,%s,0xffffffff,&,=[%d],%s,4,%s,+,0xffffffff,&,=[%d],%s,%s,+=",
REG(0), MEMBASE(2), str_ldr_bytes, REG(1), MEMBASE(2), str_ldr_bytes, REG(3), MEMBASE(2));
}
}
}
break;
case ARM_INS_TST:
r_strbuf_appendf (&op->esil, "%s,%s,==,$z,zf,:=", ARG(1), ARG(0));
break;
case ARM_INS_LDRD:
case ARM_INS_LDRB:
r_strbuf_appendf (&op->esil, "%s,%d,+,[1],%s,=",
MEMBASE(1), MEMDISP(1), REG(0));
if (insn->detail->arm.writeback) {
r_strbuf_appendf (&op->esil, ",%s,%d,+,%s,=",
MEMBASE(1), MEMDISP(1), MEMBASE(1) );
}
break;
case ARM_INS_SXTH:
r_strbuf_appendf (&op->esil,
"15,%s,>>,1,&,?{,15,-1,<<,%s,0xffff,&,|,%s,:=,}{,%s,0xffff,%s,:=,}",
REG(1), REG(1), REG(0), REG(1), REG(0));
break;
case ARM_INS_SXTB:
r_strbuf_appendf (&op->esil,
"7,%s,>>,1,&,?{,7,-1,<<,%s,0xff,&,|,%s,:=,}{,%s,0xff,&,%s,:=,}",
REG(1), REG(1), REG(0), REG(1), REG(0));
break;
case ARM_INS_LDREX:
case ARM_INS_LDREXB:
case ARM_INS_LDREXD:
case ARM_INS_LDREXH:
op->family = R_ANAL_OP_FAMILY_THREAD;
case ARM_INS_LDRHT:
case ARM_INS_LDRH:
case ARM_INS_LDRT:
case ARM_INS_LDRBT:
case ARM_INS_LDRSB:
case ARM_INS_LDRSBT:
case ARM_INS_LDRSH:
case ARM_INS_LDRSHT:
case ARM_INS_LDR:
switch (insn->id) {
case ARM_INS_LDRHT:
case ARM_INS_LDRH:
case ARM_INS_LDRSH:
case ARM_INS_LDRSHT:
mask = UT16_MAX;
break;
default:
mask = UT32_MAX;
break;
}
addr &= ~3LL;
if (MEMDISP(1) < 0) {
const char *pc = "$$";
if (REGBASE(1) == ARM_REG_PC) {
op->refptr = 4;
op->ptr = addr + pcdelta + MEMDISP(1);
r_strbuf_appendf (&op->esil, "0x%"PFMT64x",2,2,%s,>>,<<,+,0xffffffff,&,[4],0x%x,&,%s,=",
(ut64)MEMDISP(1), pc, mask, REG(0));
} else {
int disp = MEMDISP(1);
if (disp < 0) {
r_strbuf_appendf (&op->esil, "0x%"PFMT64x",%s,-,0xffffffff,&,[4],0x%x,&,%s,=",
(ut64)-disp, MEMBASE(1), mask, REG(0));
} else {
r_strbuf_appendf (&op->esil, "0x%"PFMT64x",%s,+,0xffffffff,&,[4],0x%x,&,%s,=",
(ut64)disp, MEMBASE(1), mask, REG(0));
}
}
} else {
if (REGBASE(1) == ARM_REG_PC) {
const char *pc = "$$";
op->refptr = 4;
op->ptr = addr + pcdelta + MEMDISP(1);
if (ISMEM(1) && LSHIFT2(1)) {
r_strbuf_appendf (&op->esil, "2,2,%d,%s,+,>>,<<,%d,%s,<<,+,0xffffffff,&,[4],0x%x,&,%s,=",
pcdelta, pc, LSHIFT2(1), MEMINDEX(1), mask, REG(0));
} else {
if (ISREG(1)) {
r_strbuf_appendf (&op->esil, "2,2,%d,%s,+,>>,<<,%s,+,0xffffffff,&,[4],0x%x,&,%s,=",
pcdelta, pc, MEMINDEX(1), mask, REG(0));
} else {
r_strbuf_appendf (&op->esil, "2,2,%d,%s,+,>>,<<,%d,+,0xffffffff,&,[4],0x%x,&,%s,=",
pcdelta, pc, MEMDISP(1), mask, REG(0));
}
}
} else {
if (ISMEM(1) && LSHIFT2(1)) {
r_strbuf_appendf (&op->esil, "%s,%d,%s,<<,+,0xffffffff,&,[4],0x%x,&,%s,=",
MEMBASE(1), LSHIFT2(1), MEMINDEX(1), mask, REG(0));
} else {
if (ISREG(1)) {
r_strbuf_appendf (&op->esil, "%s,%s,+,0xffffffff,&,[4],0x%x,&,%s,=",
MEMBASE(1), MEMINDEX(1), mask, REG(0));
} else if (HASMEMINDEX(1)) { 
r_strbuf_appendf (&op->esil, "%s,%s,+,0xffffffff,&,[4],0x%x,&,%s,=",
MEMINDEX(1), MEMBASE(1), mask, REG(0));
} else {
int disp = MEMDISP(1);
if (disp < 0) {
r_strbuf_appendf (&op->esil, "%d,%s,-,0xffffffff,&,[4],0x%x,&,%s,=",
-disp, MEMBASE(1), mask, REG(0));
if (insn->detail->arm.writeback) {
r_strbuf_appendf (&op->esil, ",%d,%s,+,%s,=",
-disp, MEMBASE(1), MEMBASE(1));
}
} else {
r_strbuf_appendf (&op->esil, "%d,%s,+,0xffffffff,&,[4],0x%x,&,%s,=",
disp, MEMBASE(1), mask, REG(0));
if (insn->detail->arm.writeback) {
r_strbuf_appendf (&op->esil, ",%d,%s,+,%s,=",
disp, MEMBASE(1), MEMBASE(1));
}
}
}
}
}
}
break;
case ARM_INS_MRS:
break;
case ARM_INS_MSR:
msr_flags = insn->detail->arm.operands[0].reg >> 4;
r_strbuf_appendf (&op->esil, "0,",REG(1));
if (msr_flags & 1) {
r_strbuf_appendf (&op->esil, "0xFF,|,");
}
if (msr_flags & 2) {
r_strbuf_appendf (&op->esil, "0xFF00,|,");
}
if (msr_flags & 4) {
r_strbuf_appendf (&op->esil, "0xFF0000,|,");
}
if (msr_flags & 8) {
r_strbuf_appendf (&op->esil, "0xFF000000,|,");
}
r_strbuf_appendf (&op->esil, "DUP,!,SWAP,&,%s,SWAP,cpsr,&,|,cpsr,=",REG(1));
break;
case ARM_INS_UBFX:
if (IMM (3)>0 && IMM (3)<=32-IMM (2)) {
r_strbuf_appendf (&op->esil, "%d,%s,%d,%"PFMT64u",<<,&,>>,%s,=",
IMM (2),REG (1),IMM (2),(ut64)bitmask_by_width[IMM (3)-1],REG (0));
}
break;
case ARM_INS_UXTB:
r_strbuf_appendf (&op->esil, "%s,0xff,&,%s,=",ARG (1),REG (0));
break;
case ARM_INS_RSB:
if (OPCOUNT () == 2) {
r_strbuf_appendf (&op->esil, "%s,%s,-=",ARG (0),ARG (1));
} else if (OPCOUNT () == 3) {
r_strbuf_appendf (&op->esil, "%s,%s,-,%s,=",ARG (1),ARG (2),ARG (0));
}
break;
case ARM_INS_BIC:
if (OPCOUNT () == 2) {
r_strbuf_appendf (&op->esil, "%s,0xffffffff,^,%s,&=",ARG (1),ARG (0));
} else {
r_strbuf_appendf (&op->esil, "%s,0xffffffff,^,%s,&,%s,=",ARG (2),ARG (1),ARG (0));
}
break;
case ARM_INS_SMMLA:
r_strbuf_appendf (&op->esil, "32,%s,%s,*,>>,%s,+,0xffffffff,&,%s,=",
REG (1), REG(2), REG(3), REG (0));
break;
case ARM_INS_SMMLAR:
r_strbuf_appendf (&op->esil, "32,0x80000000,%s,%s,*,+,>>,%s,+,0xffffffff,&,%s,=",
REG (1), REG(2), REG(3), REG (0));
break;
case ARM_INS_UMULL:
r_strbuf_appendf (&op->esil, "32,%s,%s,*,DUP,0xffffffff,&,%s,=,>>,%s,=",
REG(2), REG(3), REG (0), REG(1));
break;
case ARM_INS_MLS:
r_strbuf_appendf (&op->esil, "%s,%s,*,%s,-,0xffffffff,&,%s,=",
REG (1), REG(2), REG(3), REG (0));
break;
case ARM_INS_MLA:
r_strbuf_appendf (&op->esil, "%s,%s,*,%s,+,0xffffffff,&,%s,=",
REG (1), REG(2), REG(3), REG (0));
break;
case ARM_INS_MVN:
r_strbuf_appendf (&op->esil, "-1,%s,^,0xffffffff,&,%s,=",
ARG (1), REG (0));
break;
case ARM_INS_BFI:
{
if (OPCOUNT() >= 3 && ISIMM (3) && IMM (3) > 0 && IMM (3) < 64) {
ut64 mask = bitmask_by_width[IMM (3) - 1];
ut64 shift = IMM (2);
ut64 notmask = ~(mask << shift);
r_strbuf_setf (&op->esil, "%"PFMT64u",%s,&,%"PFMT64u",%"PFMT64u",%s,&,<<,|,0xffffffff,&,%s,=",
notmask, REG (0), shift, mask, REG (1), REG (0));
}
break;
}
case ARM_INS_BFC:
{
if (OPCOUNT() >= 2 && ISIMM (2) && IMM (2) > 0 && IMM (2) < 64) {
ut64 mask = bitmask_by_width[IMM (2) - 1];
ut64 shift = IMM (1);
ut64 notmask = ~(mask << shift);
r_strbuf_setf (&op->esil, "%"PFMT64u",%s,&,0xffffffff,&,%s,=",
notmask, REG (0), REG (0));
}
break;
}
case ARM_INS_REV:
{
const char *r0 = REG (0);
const char *r1 = REG (1);
r_strbuf_setf (&op->esil,
"24,0xff,%s,&,<<,%s,=,"
"16,0xff,8,%s,>>,&,<<,%s,|=,"
"8,0xff,16,%s,>>,&,<<,%s,|=,"
"0xff,24,%s,>>,&,%s,|=,",
r1, r0, r1, r0, r1, r0, r1, r0);
break;
}
case ARM_INS_REV16:
{
const char *r0 = REG (0);
const char *r1 = REG (1);
r_strbuf_setf (&op->esil,
"8,0xff00ff00,%s,&,>>,%s,=,"
"8,0x00ff00ff,%s,&,<<,%s,|=,",
r1, r0, r1, r0);
break;
}
case ARM_INS_REVSH:
{
const char *r0 = REG (0);
const char *r1 = REG (1);
r_strbuf_setf (&op->esil,
"8,0xff00,%s,&,>>,%s,=,"
"8,0x00ff,%s,&,<<,%s,|=,"
"0x8000,%s,&,?{,"
"0xffff0000,%s,|=,"
"}",
r1, r0, r1, r0, r0, r0);
break;
}
default:
break;
}
if (insn->detail->arm.update_flags) {
switch(insn->id) {
case ARM_INS_CMP:
case ARM_INS_CMN:
break;
default:
r_strbuf_appendf (&op->esil, ",$z,zf,:=,31,$s,nf,:=");
}
}
r_strbuf_append (&op->esil, postfix);
return 0;
}
static void anop64 (csh handle, RAnalOp *op, cs_insn *insn) {
ut64 addr = op->addr;
if (cs_insn_group (handle, insn, ARM64_GRP_CRYPTO)) {
op->family = R_ANAL_OP_FAMILY_CRYPTO;
} else if (cs_insn_group (handle, insn, ARM64_GRP_CRC)) {
op->family = R_ANAL_OP_FAMILY_CRYPTO;
#if CS_API_MAJOR >= 4
} else if (cs_insn_group (handle, insn, ARM64_GRP_PRIVILEGE)) {
op->family = R_ANAL_OP_FAMILY_PRIV;
#endif
} else if (cs_insn_group (handle, insn, ARM64_GRP_NEON)) {
op->family = R_ANAL_OP_FAMILY_MMX;
} else if (cs_insn_group (handle, insn, ARM64_GRP_FPARMV8)) {
op->family = R_ANAL_OP_FAMILY_FPU;
} else {
op->family = R_ANAL_OP_FAMILY_CPU;
}
switch (insn->detail->arm64.cc) {
case ARM64_CC_GE:
case ARM64_CC_GT:
case ARM64_CC_LE:
case ARM64_CC_LT:
op->sign = true;
break;
default:
break;
}
switch (insn->id) {
#if CS_API_MAJOR > 4
case ARM64_INS_PACDA:
case ARM64_INS_PACDB:
case ARM64_INS_PACDZA:
case ARM64_INS_PACDZB:
case ARM64_INS_PACGA:
case ARM64_INS_PACIA:
case ARM64_INS_PACIA1716:
case ARM64_INS_PACIASP:
case ARM64_INS_PACIAZ:
case ARM64_INS_PACIB:
case ARM64_INS_PACIB1716:
case ARM64_INS_PACIBSP:
case ARM64_INS_PACIBZ:
case ARM64_INS_PACIZA:
case ARM64_INS_PACIZB:
case ARM64_INS_AUTDA:
case ARM64_INS_AUTDB:
case ARM64_INS_AUTDZA:
case ARM64_INS_AUTDZB:
case ARM64_INS_AUTIA:
case ARM64_INS_AUTIA1716:
case ARM64_INS_AUTIASP:
case ARM64_INS_AUTIAZ:
case ARM64_INS_AUTIB:
case ARM64_INS_AUTIB1716:
case ARM64_INS_AUTIBSP:
case ARM64_INS_AUTIBZ:
case ARM64_INS_AUTIZA:
case ARM64_INS_AUTIZB:
case ARM64_INS_XPACD:
case ARM64_INS_XPACI:
case ARM64_INS_XPACLRI:
op->type = R_ANAL_OP_TYPE_CMP;
op->family = R_ANAL_OP_FAMILY_PAC;
break;
#endif
case ARM64_INS_SVC:
op->type = R_ANAL_OP_TYPE_SWI;
op->val = IMM64(0);
break;
case ARM64_INS_ADRP:
case ARM64_INS_ADR:
op->type = R_ANAL_OP_TYPE_LEA;
op->ptr = IMM64(1);
break;
case ARM64_INS_NOP:
op->type = R_ANAL_OP_TYPE_NOP;
op->cycles = 1;
break;
case ARM64_INS_SUB:
if (ISREG64(0) && REGID64(0) == ARM64_REG_SP) {
op->stackop = R_ANAL_STACK_INC;
if (ISIMM64(1)) {
op->stackptr = IMM(1);
} else if (ISIMM64(2) && ISREG64(1) && REGID64(1) == ARM64_REG_SP) {
op->stackptr = IMM64(2);
}
op->val = op->stackptr;
} else {
op->stackop = R_ANAL_STACK_RESET;
op->stackptr = 0;
}
op->cycles = 1;
case ARM64_INS_MSUB:
op->type = R_ANAL_OP_TYPE_SUB;
break;
case ARM64_INS_FDIV:
case ARM64_INS_SDIV:
case ARM64_INS_UDIV:
op->cycles = 4;
op->type = R_ANAL_OP_TYPE_DIV;
break;
case ARM64_INS_MUL:
case ARM64_INS_SMULL:
case ARM64_INS_FMUL:
case ARM64_INS_UMULL:
op->cycles = 4;
op->type = R_ANAL_OP_TYPE_MUL;
break;
case ARM64_INS_ADD:
op->cycles = 1;
case ARM64_INS_ADC:
case ARM64_INS_UMADDL:
case ARM64_INS_SMADDL:
case ARM64_INS_FMADD:
case ARM64_INS_MADD:
op->type = R_ANAL_OP_TYPE_ADD;
break;
case ARM64_INS_CSEL:
case ARM64_INS_FCSEL:
case ARM64_INS_CSET:
case ARM64_INS_CINC:
op->type = R_ANAL_OP_TYPE_CMOV;
break;
case ARM64_INS_MOV:
if (REGID64(0) == ARM64_REG_SP) {
op->stackop = R_ANAL_STACK_RESET;
op->stackptr = 0;
}
op->cycles = 1;
case ARM64_INS_MOVI:
case ARM64_INS_MOVK:
case ARM64_INS_MOVN:
case ARM64_INS_SMOV:
case ARM64_INS_UMOV:
case ARM64_INS_FMOV:
case ARM64_INS_SBFX:
case ARM64_INS_UBFX:
case ARM64_INS_UBFM:
case ARM64_INS_SBFIZ:
case ARM64_INS_UBFIZ:
case ARM64_INS_BIC:
case ARM64_INS_BFI:
case ARM64_INS_BFXIL:
op->type = R_ANAL_OP_TYPE_MOV;
break;
case ARM64_INS_MOVZ:
op->type = R_ANAL_OP_TYPE_MOV;
op->ptr = 0LL;
op->ptrsize = 8;
op->val = IMM64(1);
break;
case ARM64_INS_UXTB:
case ARM64_INS_SXTB:
op->type = R_ANAL_OP_TYPE_CAST;
op->ptr = 0LL;
op->ptrsize = 1;
break;
case ARM64_INS_UXTH:
case ARM64_INS_SXTH:
op->type = R_ANAL_OP_TYPE_MOV;
op->ptr = 0LL;
op->ptrsize = 2;
break;
case ARM64_INS_UXTW:
case ARM64_INS_SXTW:
op->type = R_ANAL_OP_TYPE_MOV;
op->ptr = 0LL;
op->ptrsize = 4;
break;
case ARM64_INS_BRK:
case ARM64_INS_HLT:
op->type = R_ANAL_OP_TYPE_TRAP;
break;
case ARM64_INS_DMB:
case ARM64_INS_DSB:
case ARM64_INS_ISB:
op->family = R_ANAL_OP_FAMILY_THREAD;
case ARM64_INS_IC: 
case ARM64_INS_DC: 
op->type = R_ANAL_OP_TYPE_SYNC; 
break;
case ARM64_INS_DUP:
case ARM64_INS_XTN:
case ARM64_INS_XTN2:
case ARM64_INS_REV64:
case ARM64_INS_EXT:
case ARM64_INS_INS:
op->type = R_ANAL_OP_TYPE_MOV;
break;
case ARM64_INS_LSL:
op->cycles = 1;
case ARM64_INS_SHL:
case ARM64_INS_USHLL:
op->type = R_ANAL_OP_TYPE_SHL;
break;
case ARM64_INS_LSR:
op->cycles = 1;
op->type = R_ANAL_OP_TYPE_SHR;
break;
case ARM64_INS_ASR:
op->cycles = 1;
op->type = R_ANAL_OP_TYPE_SAR;
break;
case ARM64_INS_NEG:
#if CS_API_MAJOR > 3
case ARM64_INS_NEGS:
#endif
op->type = R_ANAL_OP_TYPE_NOT;
break;
case ARM64_INS_FCMP:
case ARM64_INS_CCMP:
case ARM64_INS_CCMN:
case ARM64_INS_CMP:
case ARM64_INS_CMN:
case ARM64_INS_TST:
op->type = R_ANAL_OP_TYPE_CMP;
break;
case ARM64_INS_ROR:
op->cycles = 1;
op->type = R_ANAL_OP_TYPE_ROR;
break;
case ARM64_INS_AND:
op->type = R_ANAL_OP_TYPE_AND;
break;
case ARM64_INS_ORR:
case ARM64_INS_ORN:
op->type = R_ANAL_OP_TYPE_OR;
break;
case ARM64_INS_EOR:
case ARM64_INS_EON:
op->type = R_ANAL_OP_TYPE_XOR;
break;
case ARM64_INS_STRB:
case ARM64_INS_STURB:
case ARM64_INS_STUR:
case ARM64_INS_STR:
case ARM64_INS_STP:
case ARM64_INS_STNP:
case ARM64_INS_STXR:
case ARM64_INS_STXRH:
case ARM64_INS_STLXRH:
case ARM64_INS_STXRB:
op->type = R_ANAL_OP_TYPE_STORE;
if (REGBASE64(1) == ARM64_REG_X29) {
op->stackop = R_ANAL_STACK_SET;
op->stackptr = 0;
op->ptr = -MEMDISP64(1);
}
break;
case ARM64_INS_LDUR:
case ARM64_INS_LDURB:
case ARM64_INS_LDRSW:
case ARM64_INS_LDRSB:
case ARM64_INS_LDRSH:
case ARM64_INS_LDR:
case ARM64_INS_LDURSW:
case ARM64_INS_LDP:
case ARM64_INS_LDNP:
case ARM64_INS_LDPSW:
case ARM64_INS_LDRH:
case ARM64_INS_LDRB:
if (REGID(0) == ARM_REG_PC) {
op->type = R_ANAL_OP_TYPE_UJMP;
if (insn->detail->arm.cc != ARM_CC_AL) {
op->type = R_ANAL_OP_TYPE_UCJMP;
}
} else {
op->type = R_ANAL_OP_TYPE_LOAD;
}
switch (insn->id) {
case ARM64_INS_LDPSW:
case ARM64_INS_LDRSW:
case ARM64_INS_LDRSH:
case ARM64_INS_LDRSB:
op->sign = true;
break;
}
if (REGBASE64(1) == ARM64_REG_X29) {
op->stackop = R_ANAL_STACK_GET;
op->stackptr = 0;
op->ptr = MEMDISP64(1);
} else {
if (ISIMM64(1)) {
op->type = R_ANAL_OP_TYPE_LEA;
op->ptr = IMM64(1);
op->refptr = 8;
} else {
int d = (int)MEMDISP64(1);
op->ptr = (d < 0)? -d: d;
op->refptr = 4;
}
}
break;
#if CS_API_MAJOR > 4
case ARM64_INS_BLRAA:
case ARM64_INS_BLRAAZ:
case ARM64_INS_BLRAB:
case ARM64_INS_BLRABZ:
op->family = R_ANAL_OP_FAMILY_PAC;
op->type = R_ANAL_OP_TYPE_RCALL;
break;
case ARM64_INS_BRAA:
case ARM64_INS_BRAAZ:
case ARM64_INS_BRAB:
case ARM64_INS_BRABZ:
op->family = R_ANAL_OP_FAMILY_PAC;
op->type = R_ANAL_OP_TYPE_RJMP;
break;
case ARM64_INS_LDRAA:
case ARM64_INS_LDRAB:
op->family = R_ANAL_OP_FAMILY_PAC;
op->type = R_ANAL_OP_TYPE_LOAD;
break;
case ARM64_INS_RETAA:
case ARM64_INS_RETAB:
case ARM64_INS_ERETAA:
case ARM64_INS_ERETAB:
op->family = R_ANAL_OP_FAMILY_PAC;
op->type = R_ANAL_OP_TYPE_RET;
break;
#endif
case ARM64_INS_ERET:
op->family = R_ANAL_OP_FAMILY_PRIV;
op->type = R_ANAL_OP_TYPE_RET;
break;
case ARM64_INS_RET:
op->type = R_ANAL_OP_TYPE_RET;
break;
case ARM64_INS_BL: 
op->type = R_ANAL_OP_TYPE_CALL;
op->jump = IMM64(0);
op->fail = addr + 4;
break;
case ARM64_INS_BLR: 
op->type = R_ANAL_OP_TYPE_UCALL;
op->fail = addr + 4;
break;
case ARM64_INS_CBZ:
case ARM64_INS_CBNZ:
op->type = R_ANAL_OP_TYPE_CJMP;
op->jump = IMM64(1);
op->fail = addr+op->size;
break;
case ARM64_INS_TBZ:
case ARM64_INS_TBNZ:
op->type = R_ANAL_OP_TYPE_CJMP;
op->jump = IMM64(2);
op->fail = addr+op->size;
break;
case ARM64_INS_BR:
op->type = R_ANAL_OP_TYPE_UJMP; 
op->eob = true;
break;
case ARM64_INS_B:
if (insn->detail->arm64.operands[0].reg == ARM64_REG_LR) {
op->type = R_ANAL_OP_TYPE_RET;
} else if (insn->detail->arm64.cc) {
op->type = R_ANAL_OP_TYPE_CJMP;
op->jump = IMM64(0);
op->fail = addr + op->size;
} else {
op->type = R_ANAL_OP_TYPE_JMP;
op->jump = IMM64(0);
}
break;
default:
break;
}
}
static int cond_cs2r2(int cc) {
if (cc == ARM_CC_AL || cc < 0) {
cc = R_ANAL_COND_AL;
} else {
switch (cc) {
case ARM_CC_EQ: cc = R_ANAL_COND_EQ; break;
case ARM_CC_NE: cc = R_ANAL_COND_NE; break;
case ARM_CC_HS: cc = R_ANAL_COND_HS; break;
case ARM_CC_LO: cc = R_ANAL_COND_LO; break;
case ARM_CC_MI: cc = R_ANAL_COND_MI; break;
case ARM_CC_PL: cc = R_ANAL_COND_PL; break;
case ARM_CC_VS: cc = R_ANAL_COND_VS; break;
case ARM_CC_VC: cc = R_ANAL_COND_VC; break;
case ARM_CC_HI: cc = R_ANAL_COND_HI; break;
case ARM_CC_LS: cc = R_ANAL_COND_LS; break;
case ARM_CC_GE: cc = R_ANAL_COND_GE; break;
case ARM_CC_LT: cc = R_ANAL_COND_LT; break;
case ARM_CC_GT: cc = R_ANAL_COND_GT; break;
case ARM_CC_LE: cc = R_ANAL_COND_LE; break;
}
}
return cc;
}
static ut64 lookahead(csh handle, const ut64 addr, const ut8 *buf, int len, int distance) {
cs_insn *insn = NULL;
int n = cs_disasm (handle, buf, len, addr, distance, &insn);
if (n < 1) {
return UT64_MAX;
}
ut64 result = insn[n - 1].address;
cs_free (insn, n);
return result;
}
static void anop32(RAnal *a, csh handle, RAnalOp *op, cs_insn *insn, bool thumb, const ut8 *buf, int len) {
const ut64 addr = op->addr;
const int pcdelta = thumb? 4 : 8;
int i;
op->cond = cond_cs2r2 (insn->detail->arm.cc);
if (op->cond == R_ANAL_COND_NV) {
op->type = R_ANAL_OP_TYPE_NOP;
return;
}
op->cycles = 1;
if (cs_insn_group (handle, insn, ARM_GRP_CRYPTO)) {
op->family = R_ANAL_OP_FAMILY_CRYPTO;
} else if (cs_insn_group (handle, insn, ARM_GRP_CRC)) {
op->family = R_ANAL_OP_FAMILY_CRYPTO;
#if CS_API_MAJOR >= 4
} else if (cs_insn_group (handle, insn, ARM_GRP_PRIVILEGE)) {
op->family = R_ANAL_OP_FAMILY_PRIV;
} else if (cs_insn_group (handle, insn, ARM_GRP_VIRTUALIZATION)) {
op->family = R_ANAL_OP_FAMILY_VIRT;
#endif
} else if (cs_insn_group (handle, insn, ARM_GRP_NEON)) {
op->family = R_ANAL_OP_FAMILY_MMX;
} else if (cs_insn_group (handle, insn, ARM_GRP_FPARMV8)) {
op->family = R_ANAL_OP_FAMILY_FPU;
} else if (cs_insn_group (handle, insn, ARM_GRP_THUMB2DSP)) {
op->family = R_ANAL_OP_FAMILY_MMX;
} else {
op->family = R_ANAL_OP_FAMILY_CPU;
}
switch (insn->id) {
#if 0
If PC is specified for Rn, the value used is the address of the instruction plus 4.
These instructions cause a PC-relative forward branch using a table of single byte offsets (TBB) or halfword offsets (TBH). Rn provides a pointer to the table, and Rm supplies an index into the table. The branch length is twice the value of the byte (TBB) or the halfword (TBH) returned from the table. The target of the branch table must be in the same execution state.
jmp $$ + 4 + ( [delta] * 2 )
#endif
case ARM_INS_TBH: 
op->type = R_ANAL_OP_TYPE_UJMP;
op->cycles = 2;
op->ptrsize = 2;
op->ireg = r_str_get (cs_reg_name (handle, INSOP (0).mem.index));
break;
case ARM_INS_TBB: 
op->type = R_ANAL_OP_TYPE_UJMP;
op->cycles = 2;
op->ptrsize = 1;
op->ireg = r_str_get (cs_reg_name (handle, INSOP (0).mem.index));
break;
case ARM_INS_PLD:
op->type = R_ANAL_OP_TYPE_LEA; 
if (ISMEM (0)) {
int regBase = REGBASE(0);
int delta = MEMDISP(0);
if (regBase == ARM_REG_PC) {
op->ptr = addr + 4 + delta;
} else {
}
}
break;
case ARM_INS_IT:
{
op->type = R_ANAL_OP_TYPE_CJMP;
op->jump = addr + insn->size;
int distance = r_str_nlen (insn->mnemonic, 5);
op->cycles = 2;
op->fail = lookahead (handle, addr + insn->size, buf + insn->size, len - insn->size, distance);
break;
}
case ARM_INS_BKPT:
op->type = R_ANAL_OP_TYPE_TRAP;
op->cycles = 4;
break;
case ARM_INS_NOP:
op->type = R_ANAL_OP_TYPE_NOP;
op->cycles = 1;
break;
case ARM_INS_POP:
case ARM_INS_FLDMDBX:
case ARM_INS_FLDMIAX:
case ARM_INS_LDMDA:
case ARM_INS_LDMDB:
case ARM_INS_LDMIB:
case ARM_INS_LDM:
op->type = R_ANAL_OP_TYPE_POP;
op->cycles = 2;
for (i = 0; i < insn->detail->arm.op_count; i++) {
if (insn->detail->arm.operands[i].type == ARM_OP_REG &&
insn->detail->arm.operands[i].reg == ARM_REG_PC) {
if (insn->detail->arm.cc == ARM_CC_AL) {
op->type = R_ANAL_OP_TYPE_RET;
} else {
op->type = R_ANAL_OP_TYPE_CRET;
}
break;
}
}
break;
case ARM_INS_SUB:
if (ISREG(0) && REGID(0) == ARM_REG_SP) {
op->stackop = R_ANAL_STACK_INC;
if (ISIMM(1)) {
op->stackptr = IMM (1);
} else if ( ISIMM(2) && ISREG(1) && REGID(1) == ARM_REG_SP) {
op->stackptr = IMM (2);
}
op->val = op->stackptr;
}
op->cycles = 1;
case ARM_INS_SUBW:
case ARM_INS_SSUB8:
case ARM_INS_SSUB16:
op->type = R_ANAL_OP_TYPE_SUB;
break;
case ARM_INS_ADD:
case ARM_INS_ADC:
op->type = R_ANAL_OP_TYPE_ADD;
if (REGID(0) == ARM_REG_PC) {
op->type = R_ANAL_OP_TYPE_UJMP;
if (REGID(1) == ARM_REG_PC && insn->detail->arm.cc != ARM_CC_AL) {
op->type = R_ANAL_OP_TYPE_UCJMP;
op->fail = addr+op->size;
op->jump = ((addr & ~3LL) + (thumb? 4: 8) + MEMDISP(1)) & UT64_MAX;
op->ptr = (addr & ~3LL) + (thumb? 4: 8) + MEMDISP(1);
op->refptr = 4;
op->reg = r_str_get (cs_reg_name (handle, INSOP (2).reg));
break;
}
}
op->cycles = 1;
break;
case ARM_INS_ADDW:
case ARM_INS_SADD8:
case ARM_INS_SADD16:
op->type = R_ANAL_OP_TYPE_ADD;
break;
case ARM_INS_SDIV:
case ARM_INS_UDIV:
op->cycles = 4;
case ARM_INS_VDIV:
op->type = R_ANAL_OP_TYPE_DIV;
break;
case ARM_INS_MUL:
case ARM_INS_SMULL:
case ARM_INS_UMULL:
op->cycles = 4;
case ARM_INS_VMUL:
op->type = R_ANAL_OP_TYPE_MUL;
break;
case ARM_INS_TRAP:
op->type = R_ANAL_OP_TYPE_TRAP;
op->cycles = 2;
break;
case ARM_INS_MOV:
if (REGID(0) == ARM_REG_PC) {
if (REGID(1) == ARM_REG_LR) {
op->type = R_ANAL_OP_TYPE_RET;
} else {
op->type = R_ANAL_OP_TYPE_UJMP;
}
}
if (ISIMM (1)) {
op->val = IMM(1);
}
case ARM_INS_MOVT:
case ARM_INS_MOVW:
case ARM_INS_VMOVL:
case ARM_INS_VMOVN:
case ARM_INS_VQMOVUN:
case ARM_INS_VQMOVN:
case ARM_INS_SBFX:
case ARM_INS_UBFX:
case ARM_INS_BIC:
case ARM_INS_BFI:
op->type = R_ANAL_OP_TYPE_MOV;
break;
case ARM_INS_VMOV:
op->type = R_ANAL_OP_TYPE_MOV;
op->family = R_ANAL_OP_FAMILY_FPU;
op->cycles = 2;
break;
case ARM_INS_UDF:
op->type = R_ANAL_OP_TYPE_TRAP;
op->cycles = 4;
break;
case ARM_INS_SVC:
op->type = R_ANAL_OP_TYPE_SWI;
op->val = IMM(0);
break;
case ARM_INS_ROR:
case ARM_INS_RRX:
op->cycles = 1;
op->type = R_ANAL_OP_TYPE_ROR;
break;
case ARM_INS_AND:
op->type = R_ANAL_OP_TYPE_AND;
break;
case ARM_INS_ORR:
case ARM_INS_ORN:
op->type = R_ANAL_OP_TYPE_OR;
break;
case ARM_INS_EOR:
op->type = R_ANAL_OP_TYPE_XOR;
break;
case ARM_INS_CMP:
case ARM_INS_CMN:
case ARM_INS_TST:
if (ISIMM(1)) {
op->ptr = IMM(1);
}
op->reg = r_str_get (cs_reg_name (handle, INSOP (0).reg));
case ARM_INS_VCMP:
op->type = R_ANAL_OP_TYPE_CMP;
break;
case ARM_INS_LSL:
op->cycles = 1;
op->type = R_ANAL_OP_TYPE_SHL;
break;
case ARM_INS_LSR:
op->cycles = 1;
op->type = R_ANAL_OP_TYPE_SHR;
break;
case ARM_INS_ASR:
op->cycles = 1;
op->type = R_ANAL_OP_TYPE_SAR;
break;
case ARM_INS_PUSH:
case ARM_INS_STM:
case ARM_INS_STMDA:
case ARM_INS_STMDB:
op->type = R_ANAL_OP_TYPE_PUSH;
if (REGBASE(1) == ARM_REG_FP) {
op->stackop = R_ANAL_STACK_SET;
op->stackptr = 0;
op->ptr = MEMDISP(1);
}
break;
case ARM_INS_STREX:
case ARM_INS_STREXB:
case ARM_INS_STREXD:
case ARM_INS_STREXH:
op->family = R_ANAL_OP_FAMILY_THREAD;
case ARM_INS_STR:
case ARM_INS_STRB:
case ARM_INS_STRD:
case ARM_INS_STRBT:
case ARM_INS_STRH:
case ARM_INS_STRHT:
case ARM_INS_STRT:
op->cycles = 4;
op->type = R_ANAL_OP_TYPE_STORE;
if (REGBASE(1) == ARM_REG_FP) {
op->stackop = R_ANAL_STACK_SET;
op->stackptr = 0;
op->ptr = -MEMDISP(1);
}
break;
case ARM_INS_SXTB:
case ARM_INS_SXTH:
op->cycles = 1;
op->type = R_ANAL_OP_TYPE_MOV;
break;
case ARM_INS_LDREX:
case ARM_INS_LDREXB:
case ARM_INS_LDREXD:
case ARM_INS_LDREXH:
op->family = R_ANAL_OP_FAMILY_THREAD;
case ARM_INS_LDR:
case ARM_INS_LDRD:
case ARM_INS_LDRB:
case ARM_INS_LDRBT:
case ARM_INS_LDRH:
case ARM_INS_LDRHT:
case ARM_INS_LDRSB:
case ARM_INS_LDRSBT:
case ARM_INS_LDRSH:
case ARM_INS_LDRSHT:
case ARM_INS_LDRT:
op->cycles = 4;
if (REGID(0) == ARM_REG_PC) {
op->type = R_ANAL_OP_TYPE_UJMP;
if (insn->detail->arm.cc != ARM_CC_AL) {
op->type = R_ANAL_OP_TYPE_UCJMP;
}
} else {
op->type = R_ANAL_OP_TYPE_LOAD;
}
switch (insn->id) {
case ARM_INS_LDRB:
op->ptrsize = 1;
break;
case ARM_INS_LDRH:
case ARM_INS_LDRHT:
op->ptrsize = 2;
break;
}
if (REGBASE(1) == ARM_REG_FP) {
op->stackop = R_ANAL_STACK_GET;
op->stackptr = 0;
op->ptr = -MEMDISP(1);
} else if (REGBASE(1) == ARM_REG_PC) {
op->ptr = (addr & ~3LL) + (thumb? 4: 8) + MEMDISP(1);
op->refptr = 4;
if (REGID(0) == ARM_REG_PC && insn->detail->arm.cc != ARM_CC_AL) {
op->type = R_ANAL_OP_TYPE_UCJMP;
op->fail = addr+op->size;
op->jump = ((addr & ~3LL) + (thumb? 4: 8) + MEMDISP(1)) & UT64_MAX;
op->ireg = r_str_get (cs_reg_name (handle, INSOP (1).mem.index));
break;
}
}
break;
case ARM_INS_MRS:
case ARM_INS_MSR:
op->type = R_ANAL_OP_TYPE_MOV;
op->family = R_ANAL_OP_FAMILY_PRIV;
break;
case ARM_INS_BLX:
op->cycles = 4;
if (ISREG(0)) {
op->type = R_ANAL_OP_TYPE_RCALL;
} else {
op->type = R_ANAL_OP_TYPE_CALL;
op->jump = IMM(0) & UT32_MAX;
op->fail = addr + op->size;
op->hint.new_bits = (a->bits == 32)? 16 : 32;
}
break;
case ARM_INS_BL:
op->cycles = 4;
op->type = R_ANAL_OP_TYPE_CALL;
op->jump = IMM(0) & UT32_MAX;
op->fail = addr + op->size;
op->hint.new_bits = a->bits;
break;
case ARM_INS_CBZ:
case ARM_INS_CBNZ:
op->cycles = 4;
op->type = R_ANAL_OP_TYPE_CJMP;
op->jump = IMM(1) & UT32_MAX;
op->fail = addr + op->size;
if (op->jump == op->fail) {
op->type = R_ANAL_OP_TYPE_JMP;
op->fail = UT64_MAX;
}
break;
case ARM_INS_B:
op->cycles = 4;
if (insn->detail->arm.cc == ARM_CC_INVALID) {
op->type = R_ANAL_OP_TYPE_ILL;
op->fail = addr+op->size;
} else if (insn->detail->arm.cc == ARM_CC_AL) {
op->type = R_ANAL_OP_TYPE_JMP;
op->fail = UT64_MAX;
} else {
op->type = R_ANAL_OP_TYPE_CJMP;
op->fail = addr+op->size;
}
op->jump = IMM(0) & UT32_MAX;
op->hint.new_bits = a->bits;
break;
case ARM_INS_BX:
case ARM_INS_BXJ:
op->cycles = 4;
switch (REGID(0)) {
case ARM_REG_LR:
op->type = R_ANAL_OP_TYPE_RET;
break;
case ARM_REG_IP:
op->type = R_ANAL_OP_TYPE_UJMP;
break;
case ARM_REG_PC:
op->type = R_ANAL_OP_TYPE_UJMP;
op->jump = (addr & ~3LL) + pcdelta;
op->hint.new_bits = 32;
break;
default:
op->type = R_ANAL_OP_TYPE_UJMP;
op->eob = true;
break;
}
break;
case ARM_INS_ADR:
op->cycles = 2;
op->type = R_ANAL_OP_TYPE_LEA;
op->ptr = IMM(1) + addr + 4 - (addr%4);
op->refptr = 1;
break;
default:
break;
}
}
static bool is_valid(arm_reg reg) {
return reg != ARM_REG_INVALID;
}
static int parse_reg_name(RRegItem *reg_base, RRegItem *reg_delta, csh handle, cs_insn *insn, int reg_num) {
cs_arm_op armop = INSOP (reg_num);
switch (armop.type) {
case ARM_OP_REG:
reg_base->name = (char *)cs_reg_name (handle, armop.reg);
break;
case ARM_OP_MEM:
if (is_valid (armop.mem.base) && is_valid (armop.mem.index)) {
reg_base->name = (char *)cs_reg_name (handle, armop.mem.base);
reg_delta->name = (char *)cs_reg_name (handle, armop.mem.index);
} else if (is_valid (armop.mem.base)) {
reg_base->name = (char *)cs_reg_name (handle, armop.mem.base);
} else if (is_valid (armop.mem.index)) {
reg_base->name = (char *)cs_reg_name (handle, armop.mem.index);
}
break;
default:
break;
}
return 0;
}
static bool is_valid64 (arm64_reg reg) {
return reg != ARM64_REG_INVALID;
}
static int parse_reg64_name(RRegItem *reg_base, RRegItem *reg_delta, csh handle, cs_insn *insn, int reg_num) {
cs_arm64_op armop = INSOP64 (reg_num);
switch (armop.type) {
case ARM64_OP_REG:
reg_base->name = (char *)cs_reg_name (handle, armop.reg);
break;
case ARM64_OP_MEM:
if (is_valid64 (armop.mem.base) && is_valid64 (armop.mem.index)) {
reg_base->name = (char *)cs_reg_name (handle, armop.mem.base);
reg_delta->name = (char *)cs_reg_name (handle, armop.mem.index);
} else if (is_valid64 (armop.mem.base)) {
reg_base->name = (char *)cs_reg_name (handle, armop.mem.base);
} else if (is_valid64 (armop.mem.index)) {
reg_base->name = (char *)cs_reg_name (handle, armop.mem.index);
}
break;
default:
break;
}
return 0;
}
static void set_opdir(RAnalOp *op) {
switch (op->type & R_ANAL_OP_TYPE_MASK) {
case R_ANAL_OP_TYPE_LOAD:
op->direction = R_ANAL_OP_DIR_READ;
break;
case R_ANAL_OP_TYPE_STORE:
op->direction = R_ANAL_OP_DIR_WRITE;
break;
case R_ANAL_OP_TYPE_LEA:
op->direction = R_ANAL_OP_DIR_REF;
break;
case R_ANAL_OP_TYPE_CALL:
case R_ANAL_OP_TYPE_JMP:
case R_ANAL_OP_TYPE_UJMP:
case R_ANAL_OP_TYPE_UCALL:
op->direction = R_ANAL_OP_DIR_EXEC;
break;
default:
break;
}
}
static void set_src_dst(RAnalValue *val, csh *handle, cs_insn *insn, int x, int bits) {
cs_arm_op armop = INSOP (x);
cs_arm64_op arm64op = INSOP64 (x);
if (bits == 64) {
parse_reg64_name (&base_regs[x], &regdelta_regs[x], *handle, insn, x);
} else {
parse_reg_name (&base_regs[x], &regdelta_regs[x], *handle, insn, x);
}
switch (armop.type) {
case ARM_OP_REG:
break;
case ARM_OP_MEM:
if (bits == 64) {
val->delta = arm64op.mem.disp;
} else {
val->mul = armop.mem.scale;
val->delta = armop.mem.disp;
}
val->regdelta = &regdelta_regs[x];
break;
default:
break;
}
val->reg = &base_regs[x];
}
static void create_src_dst(RAnalOp *op) {
op->src[0] = r_anal_value_new ();
op->src[1] = r_anal_value_new ();
op->src[2] = r_anal_value_new ();
op->dst = r_anal_value_new ();
ZERO_FILL (base_regs[0]);
ZERO_FILL (base_regs[1]);
ZERO_FILL (base_regs[2]);
ZERO_FILL (base_regs[3]);
ZERO_FILL (regdelta_regs[0]);
ZERO_FILL (regdelta_regs[1]);
ZERO_FILL (regdelta_regs[2]);
ZERO_FILL (regdelta_regs[3]);
}
static void op_fillval(RAnalOp *op , csh handle, cs_insn *insn, int bits) {
create_src_dst (op);
switch (op->type & R_ANAL_OP_TYPE_MASK) {
case R_ANAL_OP_TYPE_LOAD:
case R_ANAL_OP_TYPE_MOV:
case R_ANAL_OP_TYPE_SUB:
case R_ANAL_OP_TYPE_ADD:
case R_ANAL_OP_TYPE_AND:
case R_ANAL_OP_TYPE_XOR:
case R_ANAL_OP_TYPE_MUL:
case R_ANAL_OP_TYPE_CMP:
case R_ANAL_OP_TYPE_OR:
set_src_dst (op->src[2], &handle, insn, 3, bits);
set_src_dst (op->src[1], &handle, insn, 2, bits);
set_src_dst (op->src[0], &handle, insn, 1, bits);
set_src_dst (op->dst, &handle, insn, 0, bits);
break;
case R_ANAL_OP_TYPE_STORE:
set_src_dst (op->dst, &handle, insn, 1, bits);
set_src_dst (op->src[0], &handle, insn, 0, bits);
break;
default:
break;
}
if ((bits == 64) && HASMEMINDEX64 (1)) {
op->ireg = r_str_get (cs_reg_name (handle, INSOP64 (1).mem.index));
} else if (HASMEMINDEX (1)) {
op->ireg = r_str_get (cs_reg_name (handle, INSOP (1).mem.index));
op->scale = INSOP (1).mem.scale;
}
}
static int analop(RAnal *a, RAnalOp *op, ut64 addr, const ut8 *buf, int len, RAnalOpMask mask) {
static csh handle = 0;
static int omode = -1;
static int obits = 32;
cs_insn *insn = NULL;
int mode = (a->bits==16)? CS_MODE_THUMB: CS_MODE_ARM;
int n, ret;
mode |= (a->big_endian)? CS_MODE_BIG_ENDIAN: CS_MODE_LITTLE_ENDIAN;
if (a->cpu && strstr (a->cpu, "cortex")) {
mode |= CS_MODE_MCLASS;
}
if (mode != omode || a->bits != obits) {
cs_close (&handle);
handle = 0; 
omode = mode;
obits = a->bits;
}
op->type = R_ANAL_OP_TYPE_NULL; 
op->size = (a->bits==16)? 2: 4;
op->stackop = R_ANAL_STACK_NULL;
op->delay = 0;
op->jump = op->fail = -1;
op->addr = addr;
op->ptr = op->val = -1;
op->refptr = 0;
r_strbuf_init (&op->esil);
if (handle == 0) {
ret = (a->bits == 64)?
cs_open (CS_ARCH_ARM64, mode, &handle):
cs_open (CS_ARCH_ARM, mode, &handle);
cs_option (handle, CS_OPT_DETAIL, CS_OPT_ON);
if (ret != CS_ERR_OK) {
handle = 0;
return -1;
}
}
int haa = hackyArmAnal (a, op, buf, len);
if (haa > 0) {
return haa;
}
n = cs_disasm (handle, (ut8*)buf, len, addr, 1, &insn);
if (n < 1) {
op->type = R_ANAL_OP_TYPE_ILL;
if (mask & R_ANAL_OP_MASK_DISASM) {
op->mnemonic = strdup ("invalid");
}
} else {
if (mask & R_ANAL_OP_MASK_DISASM) {
op->mnemonic = r_str_newf ("%s%s%s",
insn->mnemonic,
insn->op_str[0]?" ":"",
insn->op_str);
}
bool thumb = a->bits == 16;
op->size = insn->size;
op->id = insn->id;
if (a->bits == 64) {
anop64 (handle, op, insn);
if (mask & R_ANAL_OP_MASK_OPEX) {
opex64 (&op->opex, handle, insn);
}
if (mask & R_ANAL_OP_MASK_ESIL) {
analop64_esil (a, op, addr, buf, len, &handle, insn);
}
} else {
anop32 (a, handle, op, insn, thumb, (ut8*)buf, len);
if (mask & R_ANAL_OP_MASK_OPEX) {
opex (&op->opex, handle, insn);
}
if (mask & R_ANAL_OP_MASK_ESIL) {
analop_esil (a, op, addr, buf, len, &handle, insn, thumb);
}
}
set_opdir (op);
if (mask & R_ANAL_OP_MASK_VAL) {
op_fillval (op, handle, insn, a->bits);
}
cs_free (insn, n);
}
return op->size;
}
static char *get_reg_profile(RAnal *anal) {
const char *p;
if (anal->bits == 64) {
p = \
"=PC pc\n"
"=SP sp\n"
"=BP x29\n"
"=A0 x0\n"
"=A1 x1\n"
"=A2 x2\n"
"=A3 x3\n"
"=ZF zf\n"
"=SF nf\n"
"=OF vf\n"
"=CF cf\n"
"=SN x16\n" 
"gpr b0 .8 0 0\n"
"gpr b1 .8 8 0\n"
"gpr b2 .8 16 0\n"
"gpr b3 .8 24 0\n"
"gpr b4 .8 32 0\n"
"gpr b5 .8 40 0\n"
"gpr b6 .8 48 0\n"
"gpr b7 .8 56 0\n"
"gpr b8 .8 64 0\n"
"gpr b9 .8 72 0\n"
"gpr b10 .8 80 0\n"
"gpr b11 .8 88 0\n"
"gpr b12 .8 96 0\n"
"gpr b13 .8 104 0\n"
"gpr b14 .8 112 0\n"
"gpr b15 .8 120 0\n"
"gpr b16 .8 128 0\n"
"gpr b17 .8 136 0\n"
"gpr b18 .8 144 0\n"
"gpr b19 .8 152 0\n"
"gpr b20 .8 160 0\n"
"gpr b21 .8 168 0\n"
"gpr b22 .8 176 0\n"
"gpr b23 .8 184 0\n"
"gpr b24 .8 192 0\n"
"gpr b25 .8 200 0\n"
"gpr b26 .8 208 0\n"
"gpr b27 .8 216 0\n"
"gpr b28 .8 224 0\n"
"gpr b29 .8 232 0\n"
"gpr b30 .8 240 0\n"
"gpr bsp .8 248 0\n"
"gpr h0 .16 0 0\n"
"gpr h1 .16 8 0\n"
"gpr h2 .16 16 0\n"
"gpr h3 .16 24 0\n"
"gpr h4 .16 32 0\n"
"gpr h5 .16 40 0\n"
"gpr h6 .16 48 0\n"
"gpr h7 .16 56 0\n"
"gpr h8 .16 64 0\n"
"gpr h9 .16 72 0\n"
"gpr h10 .16 80 0\n"
"gpr h11 .16 88 0\n"
"gpr h12 .16 96 0\n"
"gpr h13 .16 104 0\n"
"gpr h14 .16 112 0\n"
"gpr h15 .16 120 0\n"
"gpr h16 .16 128 0\n"
"gpr h17 .16 136 0\n"
"gpr h18 .16 144 0\n"
"gpr h19 .16 152 0\n"
"gpr h20 .16 160 0\n"
"gpr h21 .16 168 0\n"
"gpr h22 .16 176 0\n"
"gpr h23 .16 184 0\n"
"gpr h24 .16 192 0\n"
"gpr h25 .16 200 0\n"
"gpr h26 .16 208 0\n"
"gpr h27 .16 216 0\n"
"gpr h28 .16 224 0\n"
"gpr h29 .16 232 0\n"
"gpr h30 .16 240 0\n"
"gpr w0 .32 0 0\n"
"gpr w1 .32 8 0\n"
"gpr w2 .32 16 0\n"
"gpr w3 .32 24 0\n"
"gpr w4 .32 32 0\n"
"gpr w5 .32 40 0\n"
"gpr w6 .32 48 0\n"
"gpr w7 .32 56 0\n"
"gpr w8 .32 64 0\n"
"gpr w9 .32 72 0\n"
"gpr w10 .32 80 0\n"
"gpr w11 .32 88 0\n"
"gpr w12 .32 96 0\n"
"gpr w13 .32 104 0\n"
"gpr w14 .32 112 0\n"
"gpr w15 .32 120 0\n"
"gpr w16 .32 128 0\n"
"gpr w17 .32 136 0\n"
"gpr w18 .32 144 0\n"
"gpr w19 .32 152 0\n"
"gpr w20 .32 160 0\n"
"gpr w21 .32 168 0\n"
"gpr w22 .32 176 0\n"
"gpr w23 .32 184 0\n"
"gpr w24 .32 192 0\n"
"gpr w25 .32 200 0\n"
"gpr w26 .32 208 0\n"
"gpr w27 .32 216 0\n"
"gpr w28 .32 224 0\n"
"gpr w29 .32 232 0\n"
"gpr w30 .32 240 0\n"
"gpr wsp .32 248 0\n"
"gpr wzr .32 ? 0\n"
"gpr s0 .32 0 0\n"
"gpr s1 .32 8 0\n"
"gpr s2 .32 16 0\n"
"gpr s3 .32 24 0\n"
"gpr s4 .32 32 0\n"
"gpr s5 .32 40 0\n"
"gpr s6 .32 48 0\n"
"gpr s7 .32 56 0\n"
"gpr s8 .32 64 0\n"
"gpr s9 .32 72 0\n"
"gpr s10 .32 80 0\n"
"gpr s11 .32 88 0\n"
"gpr s12 .32 96 0\n"
"gpr s13 .32 104 0\n"
"gpr s14 .32 112 0\n"
"gpr s15 .32 120 0\n"
"gpr s16 .32 128 0\n"
"gpr s17 .32 136 0\n"
"gpr s18 .32 144 0\n"
"gpr s19 .32 152 0\n"
"gpr s20 .32 160 0\n"
"gpr s21 .32 168 0\n"
"gpr s22 .32 176 0\n"
"gpr s23 .32 184 0\n"
"gpr s24 .32 192 0\n"
"gpr s25 .32 200 0\n"
"gpr s26 .32 208 0\n"
"gpr s27 .32 216 0\n"
"gpr s28 .32 224 0\n"
"gpr s29 .32 232 0\n"
"gpr s30 .32 240 0\n"
"gpr x0 .64 0 0\n" 
"gpr x1 .64 8 0\n" 
"gpr x2 .64 16 0\n" 
"gpr x3 .64 24 0\n" 
"gpr x4 .64 32 0\n" 
"gpr x5 .64 40 0\n" 
"gpr x6 .64 48 0\n" 
"gpr x7 .64 56 0\n" 
"gpr x8 .64 64 0\n" 
"gpr x9 .64 72 0\n" 
"gpr x10 .64 80 0\n" 
"gpr x11 .64 88 0\n" 
"gpr x12 .64 96 0\n" 
"gpr x13 .64 104 0\n" 
"gpr x14 .64 112 0\n" 
"gpr x15 .64 120 0\n" 
"gpr x16 .64 128 0\n" 
"gpr x17 .64 136 0\n" 
"gpr x18 .64 144 0\n" 
"gpr x19 .64 152 0\n" 
"gpr x20 .64 160 0\n" 
"gpr x21 .64 168 0\n" 
"gpr x22 .64 176 0\n" 
"gpr x23 .64 184 0\n" 
"gpr x24 .64 192 0\n" 
"gpr x25 .64 200 0\n" 
"gpr x26 .64 208 0\n" 
"gpr x27 .64 216 0\n"
"gpr x28 .64 224 0\n"
"gpr x29 .64 232 0\n"
"gpr x30 .64 240 0\n"
"gpr tmp .64 288 0\n"
"gpr d0 .64 0 0\n" 
"gpr d1 .64 8 0\n" 
"gpr d2 .64 16 0\n" 
"gpr d3 .64 24 0\n" 
"gpr d4 .64 32 0\n" 
"gpr d5 .64 40 0\n" 
"gpr d6 .64 48 0\n" 
"gpr d7 .64 56 0\n" 
"gpr d8 .64 64 0\n" 
"gpr d9 .64 72 0\n" 
"gpr d10 .64 80 0\n" 
"gpr d11 .64 88 0\n" 
"gpr d12 .64 96 0\n" 
"gpr d13 .64 104 0\n" 
"gpr d14 .64 112 0\n" 
"gpr d15 .64 120 0\n" 
"gpr d16 .64 128 0\n" 
"gpr d17 .64 136 0\n" 
"gpr d18 .64 144 0\n" 
"gpr d19 .64 152 0\n" 
"gpr d20 .64 160 0\n" 
"gpr d21 .64 168 0\n" 
"gpr d22 .64 176 0\n" 
"gpr d23 .64 184 0\n" 
"gpr d24 .64 192 0\n" 
"gpr d25 .64 200 0\n" 
"gpr d26 .64 208 0\n" 
"gpr d27 .64 216 0\n"
"gpr d28 .64 224 0\n"
"gpr d29 .64 232 0\n"
"gpr d30 .64 240 0\n"
"gpr dsp .64 248 0\n"
"gpr fp .64 232 0\n" 
"gpr lr .64 240 0\n" 
"gpr sp .64 248 0\n"
"gpr pc .64 256 0\n"
"gpr zr .64 ? 0\n"
"gpr xzr .64 ? 0\n"
"flg pstate .64 280 0 _____tfiae_____________j__qvczn\n" 
"flg vf .1 280.28 0 overflow\n" 
"flg cf .1 280.29 0 carry\n" 
"flg zf .1 280.30 0 zero\n" 
"flg nf .1 280.31 0 sign\n"; 
} else {
p = \
"=PC r15\n"
"=LR r14\n"
"=SP sp\n"
"=BP fp\n"
"=A0 r0\n"
"=A1 r1\n"
"=A2 r2\n"
"=A3 r3\n"
"=ZF zf\n"
"=SF nf\n"
"=OF vf\n"
"=CF cf\n"
"=SN r7\n"
"gpr sb .32 36 0\n" 
"gpr sl .32 40 0\n" 
"gpr fp .32 44 0\n" 
"gpr ip .32 48 0\n" 
"gpr sp .32 52 0\n" 
"gpr lr .32 56 0\n" 
"gpr pc .32 60 0\n" 
"gpr r0 .32 0 0\n"
"gpr r1 .32 4 0\n"
"gpr r2 .32 8 0\n"
"gpr r3 .32 12 0\n"
"gpr r4 .32 16 0\n"
"gpr r5 .32 20 0\n"
"gpr r6 .32 24 0\n"
"gpr r7 .32 28 0\n"
"gpr r8 .32 32 0\n"
"gpr r9 .32 36 0\n"
"gpr r10 .32 40 0\n"
"gpr r11 .32 44 0\n"
"gpr r12 .32 48 0\n"
"gpr r13 .32 52 0\n"
"gpr r14 .32 56 0\n"
"gpr r15 .32 60 0\n"
"flg cpsr .32 64 0\n"
"gpr blank .32 68 0\n" 
"flg tf .1 .517 0 thumb\n" 
"flg ef .1 .521 0 endian\n" 
"flg itc .4 .522 0 if_then_count\n" 
"flg gef .4 .528 0 great_or_equal\n" 
"flg jf .1 .536 0 java\n" 
"flg qf .1 .539 0 sticky_overflow\n" 
"flg vf .1 .540 0 overflow\n" 
"flg cf .1 .541 0 carry\n" 
"flg zf .1 .542 0 zero\n" 
"flg nf .1 .543 0 negative\n" 
;
}
return strdup (p);
}
static int archinfo(RAnal *anal, int q) {
if (q == R_ANAL_ARCHINFO_DATA_ALIGN) {
return 4;
}
if (q == R_ANAL_ARCHINFO_ALIGN) {
if (anal && anal->bits == 16) {
return 2;
}
return 4;
}
if (q == R_ANAL_ARCHINFO_MAX_OP_SIZE) {
return 4;
}
if (q == R_ANAL_ARCHINFO_MIN_OP_SIZE) {
if (anal && anal->bits == 16) {
return 2;
}
return 4;
}
return 4; 
}
static ut8 *anal_mask(RAnal *anal, int size, const ut8 *data, ut64 at) {
RAnalOp *op = NULL;
ut8 *ret = NULL;
int oplen, idx = 0, obits = anal->bits;
RAnalHint *hint = NULL;
if (!data) {
return NULL;
}
op = r_anal_op_new ();
ret = malloc (size);
memset (ret, 0xff, size);
while (idx < size) {
hint = r_anal_hint_get (anal, at + idx);
if (hint) {
if (hint->bits != 0) {
anal->bits = hint->bits;
}
free (hint);
}
if ((oplen = analop (anal, op, at + idx, data + idx, size - idx, R_ANAL_OP_MASK_BASIC)) < 1) {
break;
}
if (op->ptr != UT64_MAX || op->jump != UT64_MAX) {
ut32 opcode = r_read_ble (data + idx, anal->big_endian, oplen * 8);
switch (oplen) {
case 2:
memcpy (ret + idx, "\xf0\x00", 2);
break;
case 4:
if (anal->bits == 64) {
switch (op->id) {
case ARM64_INS_LDP:
case ARM64_INS_LDXP:
case ARM64_INS_LDXR:
case ARM64_INS_LDXRB:
case ARM64_INS_LDXRH:
case ARM64_INS_LDPSW:
case ARM64_INS_LDNP:
case ARM64_INS_LDTR:
case ARM64_INS_LDTRB:
case ARM64_INS_LDTRH:
case ARM64_INS_LDTRSB:
case ARM64_INS_LDTRSH:
case ARM64_INS_LDTRSW:
case ARM64_INS_LDUR:
case ARM64_INS_LDURB:
case ARM64_INS_LDURH:
case ARM64_INS_LDURSB:
case ARM64_INS_LDURSH:
case ARM64_INS_LDURSW:
case ARM64_INS_STP:
case ARM64_INS_STNP:
case ARM64_INS_STXR:
case ARM64_INS_STXRB:
case ARM64_INS_STXRH:
r_write_ble (ret + idx, 0xffffffff, anal->big_endian, 32);
break;
case ARM64_INS_STRB:
case ARM64_INS_STURB:
case ARM64_INS_STURH:
case ARM64_INS_STUR:
case ARM64_INS_STR:
case ARM64_INS_STTR:
case ARM64_INS_STTRB:
case ARM64_INS_STRH:
case ARM64_INS_STTRH:
case ARM64_INS_LDR:
case ARM64_INS_LDRB:
case ARM64_INS_LDRH:
case ARM64_INS_LDRSB:
case ARM64_INS_LDRSW:
case ARM64_INS_LDRSH: {
bool is_literal = (opcode & 0x38000000) == 0x18000000;
if (is_literal) {
r_write_ble (ret + idx, 0xff000000, anal->big_endian, 32);
} else {
r_write_ble (ret + idx, 0xffffffff, anal->big_endian, 32);
}
break;
}
case ARM64_INS_B:
case ARM64_INS_BL:
case ARM64_INS_CBZ:
case ARM64_INS_CBNZ:
if (op->type == R_ANAL_OP_TYPE_CJMP) {
r_write_ble (ret + idx, 0xff00001f, anal->big_endian, 32);
} else {
r_write_ble (ret + idx, 0xfc000000, anal->big_endian, 32);
}
break;
case ARM64_INS_TBZ:
case ARM64_INS_TBNZ:
r_write_ble (ret + idx, 0xfff8001f, anal->big_endian, 32);
break;
case ARM64_INS_ADR:
case ARM64_INS_ADRP:
r_write_ble (ret + idx, 0xff00001f, anal->big_endian, 32);
break;
default:
r_write_ble (ret + idx, 0xfff00000, anal->big_endian, 32);
}
} else {
r_write_ble (ret + idx, 0xfff00000, anal->big_endian, 32);
}
break;
}
}
idx += oplen;
}
anal->bits = obits;
r_anal_op_free (op);
return ret;
}
static RList *anal_preludes(RAnal *anal) {
#define KW(d,ds,m,ms) r_list_append (l, r_search_keyword_new((const ut8*)d,ds,(const ut8*)m, ms, NULL))
RList *l = r_list_newf ((RListFree)r_search_keyword_free);
switch (anal->bits) {
case 16:
KW ("\x00\xb5", 2, "\x0f\xff", 2);
KW ("\x08\xb5", 2, "\x0f\xff", 2);
break;
case 32:
KW("\x00\x00\x2d\xe9", 4, "\x0f\x0f\xff\xff", 4);
break;
case 64:
KW ("\xf0\x00\x00\xd1", 4, "\xf0\x00\x00\xff", 4);
KW ("\xf0\x00\x00\xa9", 4, "\xf0\x00\x00\xff", 4);
KW ("\x7f\x23\x03\xd5\xff", 5, NULL, 0);
break;
default:
r_list_free (l);
l = NULL;
}
return l;
}
RAnalPlugin r_anal_plugin_arm_cs = {
.name = "arm",
.desc = "Capstone ARM analyzer",
.license = "BSD",
.esil = true,
.arch = "arm",
.archinfo = archinfo,
.get_reg_profile = get_reg_profile,
.anal_mask = anal_mask,
.preludes = anal_preludes,
.bits = 16 | 32 | 64,
.op = &analop,
};
#if !defined(R2_PLUGIN_INCORE)
R_API RLibStruct radare_plugin = {
.type = R_LIB_TYPE_ANAL,
.data = &r_anal_plugin_arm_cs,
.version = R2_VERSION
};
#endif
