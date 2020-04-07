





















#include <ctype.h>
#include <stdarg.h>
#include <ansidecl.h>
#include <string.h>

#include "disas-asm.h"
#include "arc.h"
#include "arc-ext.h"
#include "arc-dis.h"
#include "arcompact-dis.h"
#include "elf-bfd.h"
#include "r_types.h"





static bfd_vma bfd_getm32 (unsigned int);
static bfd_vma bfd_getm32_ac (unsigned int) ATTRIBUTE_UNUSED;


#if !defined(dbg)
#define dbg (0)
#endif






#undef _NELEM
#define _NELEM(ary) (sizeof(ary) / sizeof((ary)[0]))

#define BIT(word,n) ((word) & (1 << (n)))

#define BITS(word,s,e) (((word) << (sizeof(word)*8-1 - (e))) >> ((s)+(sizeof(word)*8-1 - (e))))

#define OPCODE(word) (BITS ((word), 27, 31))
#define FIELDA(word) (BITS ((word), 0, 5))
#define FIELDb(word) (BITS ((word), 24, 26))
#define FIELDB(word) (BITS ((word), 12, 14))
#define FIELDC(word) (BITS ((word), 6, 11))
#define OPCODE_AC(word) (BITS ((word), 11, 15))
#define FIELDA_AC(word) (BITS ((word), 0, 2))
#define FIELDB_AC(word) (BITS ((word), 8, 10))
#define FIELDC_AC(word) (BITS ((word), 5, 7))
#define FIELDU_AC(word) (BITS ((word), 0, 4))





#define FIELDS_AC(word) (BITS (((signed int) (word)), 0, 8))





#define FIELDD(word) (BITS (((signed int) (word)), 16, 23))





#define FIELDD9(word) ((BITS(((signed int)(word)),15,15) << 8) | (BITS((word),16,23)))




#define FIELDS(word) ((BITS(((signed int)(word)),0,5) << 6) | (BITS((word),6,11))) 





#define FIELDS9(word) (((BITS(((signed int)(word)),15,15) << 7) | (BITS((word),17,23))) << 1)
#define FIELDS9_FLAG(word) (((BITS(((signed int)(word)),0,5) << 6) | (BITS((word),6,11))) )

#define PUT_NEXT_WORD_IN(a) { if (is_limm==1 && !NEXT_WORD(1)) mwerror(state, "Illegal limm reference in last instruction!\n"); if (info->endian == BFD_ENDIAN_LITTLE) { (a) = ((state->words[1] & 0xff00) | (state->words[1] & 0xff)) << 16; (a) |= ((state->words[1] & 0xff0000) | (state->words[1] & 0xff000000)) >> 16; } else { (a) = state->words[1]; } }











#define CHECK_NULLIFY() do{ state->nullifyMode = BITS(state->words[0],5,5); }while(0)



#define CHECK_COND_NULLIFY() do { state->nullifyMode = BITS(state->words[0],5,5); cond = BITS(state->words[0],0,4); }while(0)




#define CHECK_FLAG_COND_NULLIFY() do{ if (is_shimm == 0) { flag = BIT(state->words[0],15); state->nullifyMode = BITS(state->words[0],5,5); cond = BITS(state->words[0],0,4); } }while(0)







#define CHECK_FLAG_COND() { if (is_shimm == 0) { flag = BIT(state->words[0],15); cond = BITS(state->words[0],0,4); } }






#define CHECK_FLAG() { flag = BIT(state->words[0],15); }



#define CHECK_COND() { if (is_shimm == 0) { cond = BITS(state->words[0],0,4); } }





#define CHECK_FIELD(field) { if ((field) == 62) { is_limm++; field##isReg = 0; PUT_NEXT_WORD_IN(field); } }







#define CHECK_FIELD_A() { fieldA = FIELDA(state->words[0]); if (fieldA == 62) { fieldAisReg = 0; fieldA = 0; } }







#define FIELD_B() { fieldB = (FIELDB(state->words[0]) << 3);fieldB |= FIELDb(state->words[0]); if (fieldB == 62) { fieldBisReg = 0; fieldB = 0; } }








#define FIELD_C() { fieldC = FIELDC(state->words[0]); if (fieldC == 62) { fieldCisReg = 0; } }






#define FIELD_U8() { fieldC = BITS(state->words[0],15,16);fieldC = fieldC <<6; fieldC |= FIELDC(state->words[0]); fieldCisReg = 0; }







#define CHECK_FIELD_B() { fieldB = (FIELDB(state->words[0]) << 3);fieldB |= FIELDb(state->words[0]); CHECK_FIELD(fieldB); }





#define CHECK_FIELD_C() { fieldC = FIELDC(state->words[0]); CHECK_FIELD(fieldC); }




#define FIELD_C_S() { fieldC_S = (FIELDC_S(state->words[0]) << 3); }



#define FIELD_B_S() { fieldB_S = (FIELDB_S(state->words[0]) << 3); }



#define CHECK_FIELD_H_AC() { fieldC = ((FIELDA_AC(state->words[0])) << 3); fieldC |= FIELDC_AC(state->words[0]); CHECK_FIELD(fieldC); }





#define FIELD_H_AC() { fieldC = ((FIELDA_AC(state->words[0])) << 3); fieldC |= FIELDC_AC(state->words[0]); if (fieldC > 60) { fieldCisReg = 0; fieldC = 0; } }








#define FIELD_C_AC() { fieldC = FIELDC_AC(state->words[0]); if (fieldC > 3) { fieldC += 8; } }






#define FIELD_B_AC() { fieldB = FIELDB_AC(state->words[0]); if (fieldB > 3) { fieldB += 8; } }






#define FIELD_A_AC() { fieldA = FIELDA_AC(state->words[0]); if (fieldA > 3) { fieldA += 8; } }






#define IS_SMALL(x) (((field##x) < 256) && ((field##x) > -257))
#define IS_REG(x) (field##x##isReg)
#define IS_SIMD_128_REG(x) (usesSimdReg##x == 1)
#define IS_SIMD_16_REG(x) (usesSimdReg##x == 2)
#define IS_SIMD_DATA_REG(x) (usesSimdReg##x == 3)
#define WRITE_FORMAT_LB_Rx_RB(x) WRITE_FORMAT(x,"[","]","","")
#define WRITE_FORMAT_x_COMMA_LB(x) WRITE_FORMAT(x,"",", [","",",[")
#define WRITE_FORMAT_COMMA_x_RB(x) WRITE_FORMAT(x,", ","]",", ","]")
#define WRITE_FORMAT_x_RB(x) WRITE_FORMAT(x,"","]","","]")
#define WRITE_FORMAT_COMMA_x(x) WRITE_FORMAT(x,", ","",", ","")
#define WRITE_FORMAT_x_COMMA(x) WRITE_FORMAT(x,"",", ","",", ")
#define WRITE_FORMAT_x(x) WRITE_FORMAT(x,"","","","")
#define WRITE_FORMAT(x,cb1,ca1,cb,ca) strcat(formatString, (IS_SIMD_128_REG(x) ? cb1"%S"ca1: IS_SIMD_16_REG(x) ? cb1"%I"ca1: IS_SIMD_DATA_REG(x)? cb1"%D"ca1: IS_REG(x) ? cb1"%r"ca1: usesAuxReg ? cb"%a"ca : IS_SMALL(x) ? cb"%d"ca : cb"%h"ca))







#define WRITE_FORMAT_LB() strcat(formatString, "[")
#define WRITE_FORMAT_RB() strcat(formatString, "]")
#define WRITE_COMMENT(str) (state->comm[state->commNum++] = (str))
#define WRITE_NOP_COMMENT() if (!fieldAisReg && !flag) WRITE_COMMENT("nop");

#define NEXT_WORD(x) (offset += 4, state->words[x])

#define NEXT_WORD_AC(x) (offset += 2, state->words[x])

#define add_target(x) (state->targets[state->tcnt++] = (x))

static short int enable_simd = 0;
static short int enable_insn_stream = 0;


static const char *
core_reg_name(struct arcDisState *state, int val)
{
if (state->coreRegName) {
return (*state->coreRegName) (state->_this, val);
}
return 0;
}

static const char *
aux_reg_name(struct arcDisState *state, int val)
{
if (state->auxRegName) {
return (*state->auxRegName) (state->_this, val);
}
return 0;
}

static const char *
cond_code_name(struct arcDisState *state, int val)
{
if (state->condCodeName) {
return (*state->condCodeName) (state->_this, val);
}
return 0;
}

static const char *
instruction_name(struct arcDisState *state, int op1, int op2, int *flags)
{
if (state->instName) {
return (*state->instName) (state->_this, op1, op2, flags);
}
return 0;
}

static void
mwerror(struct arcDisState *state, const char *msg)
{
if (state->err != 0) {
(*state->err) (state->_this, (msg));
}
}

static const char *
post_address(struct arcDisState *state, int addr)
{
static char id[3*_NELEM(state->addresses)];
unsigned int j, i = state->acnt;
if (i < _NELEM(state->addresses)) {
state->addresses[i] = addr;
++state->acnt;
j = i*3;
id[j+0] = '@';
id[j+1] = '0'+i;
id[j+2] = 0;
return id+j;
}
return "";
}

static void
my_sprintf (struct arcDisState *state, char *buf, const char*format, ...)
{
char *bp;
const char *p;
int size, leading_zero, regMap[2];
va_list ap;

va_start(ap,format);
bp = buf;
*bp = 0;
p = format;
regMap[0] = 0;
regMap[1] = 0;
while (1) {
switch (*p++) {
case 0: goto DOCOMM; 
default:
*bp++ = p[-1];
break;
case '%':
size = 0;
leading_zero = 0;
RETRY:;
switch (*p++) {
case '0':
case '1':
case '2':
case '3':
case '4':
case '5':
case '6':
case '7':
case '8':
case '9': {

size = p[-1] - '0';
if (size == 0) {
leading_zero = 1; 
}
while (*p >= '0' && *p <= '9') {
size = size * 10 + *p - '0', p++;
}
goto RETRY;
}
#define inc_bp() bp = bp+strlen(bp)

case 'h':
{
unsigned u = va_arg(ap,int);





#define CDT_DEBUG
if (u > 65536) {
#if !defined(CDT_DEBUG)
sprintf(bp,"0x%x_%04x",u >> 16, u & 0xffff);
#else
sprintf(bp,"0x%08x",u);
#endif 
} else {
sprintf (bp, "0x%x", u);
}
inc_bp();
}
break;
case 'X': case 'x':
{
int val = va_arg(ap,int);
if (size != 0) {
if (leading_zero) {
sprintf (bp, "%0*x", size, val);
} else {
sprintf (bp, "%*x", size, val);
}
} else {
sprintf (bp, "%x", val);
}
inc_bp();
}
break;
case 'd':
{
int val = va_arg(ap,int);
if (size != 0) {
sprintf (bp, "%*d", size, val);
} else {
sprintf (bp, "%d", val);
}
inc_bp();
}
break;
case 'r':
{

int val = va_arg(ap,int);

#define REG2NAME(num, name) case num: sprintf(bp,""name); regMap[((num)<32)?0:1] |= 1<<((num)-(((num)<32)?0:32)); break;

switch (val)
{
REG2NAME(26, "gp");
REG2NAME(27, "fp");
REG2NAME(28, "sp");
REG2NAME(29, "ilink1");
REG2NAME(30, "ilink2");
REG2NAME(31, "blink");
REG2NAME(60, "lp_count");
REG2NAME(63, "pcl");
default:
{
const char *ext;
ext = core_reg_name(state, val);
if (ext) {
sprintf (bp, "%s", ext);
} else {
sprintf (bp, "r%d", val);
}
}break;
}
inc_bp();
} break;

case 'a':
{

int val = va_arg(ap,int);
char *ret;
ret = arc_aux_reg_name(val);
if (ret) {
sprintf (bp, "%s", ret);
} else {
const char *ext;
ext = aux_reg_name (state, val);
if (ext) {
sprintf (bp, "%s", ext);
} else {
my_sprintf (state, bp, "%h", val);
}
}

inc_bp();
}
break;
case 's':
{
sprintf(bp,"%s",va_arg(ap,char*));
inc_bp();
}
break;
case '*':
{
#if 0
va_arg(ap,char*);
inc_bp();
break;
#elif 1 
va_arg(ap,int);
break;
#else
extern void abort (void);

abort ();
#endif
}


case 'S':
{
int val = va_arg (ap,int);

sprintf (bp, "vr%d",val);
inc_bp ();
break;
}
case 'I':
{
int val = va_arg (ap,int);

sprintf (bp, "i%d",val);
inc_bp ();
break;
}
case 'D':
{
int val = va_arg (ap,int);

sprintf (bp, "dr%d",val);
inc_bp ();
break;
}

default:
fprintf(stderr,"?? format %c\n",p[-1]);
break;
}
}
}

DOCOMM:
*bp = 0;
va_end (ap);
}

static const char *condName[] =
{

"" , "z" , "nz" , "p" , "n" , "c" , "nc" , "v" ,
"nv" , "gt" , "ge" , "lt" , "le" , "hi" , "ls" , "pnz",
"ss" , "sc"

};

static void
write_instr_name_(struct arcDisState *state,
const char *instrName,
int cond,
int condCodeIsPartOfName,
int flag,
int signExtend,
int addrWriteBack,
int directMem)
{
if (!instrName) {
return;
}
strncpy (state->instrBuffer, instrName, sizeof (state->instrBuffer) - 1);
if (cond > 0) {
int condlim = 0; 
const char *cc = 0;
if (!condCodeIsPartOfName) {
strcat (state->instrBuffer, ".");
}
condlim = 18;
if (cond < condlim) {
cc = condName[cond];
} else {
cc = cond_code_name (state, cond);
}
if (!cc) {
cc = "???";
}
strcat (state->instrBuffer, cc);
}
if (flag) {
strcat (state->instrBuffer, ".f");
}
if (state->nullifyMode) {
if (strstr (state->instrBuffer, ".d") == NULL) {
strcat (state->instrBuffer, ".d");
}
}
if (signExtend) {
strcat (state->instrBuffer, ".x");
}
switch (addrWriteBack) {
case 1: strcat(state->instrBuffer, ".a"); break;
case 2: strcat(state->instrBuffer, ".ab"); break;
case 3: strcat(state->instrBuffer, ".as"); break;
}
if (directMem) {
strcat (state->instrBuffer, ".di");
}
}

#define write_instr_name() {write_instr_name_(state, instrName,cond, condCodeIsPartOfName, flag, signExtend, addrWriteBack, directMem); formatString[0] = '\0'; }




enum
{
op_BC = 0, op_BLC = 1, op_LD = 2, op_ST = 3, op_MAJOR_4 = 4,

op_MAJOR_5 = 5, op_MAJOR_6 = 6, op_SIMD=9, op_LD_ADD = 12, op_ADD_SUB_SHIFT = 13,

op_ADD_MOV_CMP = 14, op_S = 15, op_LD_S = 16, op_LDB_S = 17,
op_LDW_S = 18, op_LDWX_S = 19, op_ST_S = 20, op_STB_S = 21,
op_STW_S = 22, op_Su5 = 23, op_SP = 24, op_GP = 25, op_Pcl = 26,
op_MOV_S = 27, op_ADD_CMP = 28, op_BR_S = 29, op_B_S = 30, op_BL_S = 31
};

extern disassemble_info tm_print_insn_info;





static bfd_vma
bfd_getm32 (data)
unsigned int data;
{
bfd_vma value = 0;

value = ((data & 0xff00) | (data & 0xff)) << 16;
value |= ((data & 0xff0000) | (data & 0xff000000)) >> 16;
return value;
}





static bfd_vma
bfd_getm32_ac (data)
unsigned int data;
{
bfd_vma value = 0;

value = ((data & 0xff) << 8 | (data & 0xff00) >> 8);
return value;
}





static int
sign_extend (int value, int bits)
{
if (BIT (value, (bits - 1))) {
value |= (0xffffffff << bits);
}
return value;
}












static int
dsmOneArcInst (bfd_vma addr, struct arcDisState *state, disassemble_info * info)
{

int subopcode, mul;
int condCodeIsPartOfName=0;
int decodingClass;
const char *instrName;
int fieldAisReg=1, fieldBisReg=1, fieldCisReg=1;
int fieldA=0, fieldB=0, fieldC=0;
int flag=0, cond=0, is_shimm=0, is_limm=0;
int signExtend=0, addrWriteBack=0, directMem=0;
int is_linked=0;
int offset=0;
int usesAuxReg = 0;
int usesSimdRegA= 0, usesSimdRegB=0, usesSimdRegC=0,simd_scale_u8=-1;
int flags = !E_ARC_MACH_A4;
char formatString[60];

state->nullifyMode = BR_exec_when_no_jump;
state->isBranch = 0;

state->_mem_load = 0;
state->_ea_present = 0;
state->_load_len = 0;
state->ea_reg1 = no_reg;
state->ea_reg2 = no_reg;
state->_offset = 0;

state->sourceType = ARC_UNDEFINED;


if ((info->endian == BFD_ENDIAN_LITTLE) && (state->instructionLen == 4)) {
state->words[0] = bfd_getm32 (state->words[0]);
}

if (state->instructionLen == 4)
{
if (!NEXT_WORD (0)) {
return 0;
}

state->_opcode = OPCODE (state->words[0]);
}
else
{

if (!NEXT_WORD_AC (0)) {
return 0;
}

state->_opcode = OPCODE_AC(state->words[0]);
}

instrName = 0;
decodingClass = 0; 
mul = 0;
condCodeIsPartOfName=0;
state->commNum = 0;
state->tcnt = 0;
state->acnt = 0;
state->flow = noflow;









switch (state->_opcode)
{
case op_BC:

instrName = "b";
decodingClass = 13;
condCodeIsPartOfName = 1;
state->isBranch = 1;
break;

case op_BLC:

decodingClass = 9;
state->isBranch = 1;
switch (BITS(state->words[0],16,16))
{
case 0:
if (!instrName) {
instrName = "bl";
}
decodingClass = 13;
condCodeIsPartOfName = 1;
break;
case 1:
switch (BITS(state->words[0],0,3))
{
case 0: instrName = "breq"; break;
case 1: instrName = "brne"; break;
case 2: instrName = "brlt"; break;
case 3: instrName = "brge"; break;
case 4: instrName = "brlo"; break;
case 5: instrName = "brhs"; break;
case 14: instrName = "bbit0"; break;
case 15: instrName = "bbit1"; break;
default:
instrName = "??? (0[3])";
state->flow = invalid_instr;
break;
}
break;
default:
instrName = "??? (0[3])";
state->flow = invalid_instr;
break;
}
break;

case op_LD:

decodingClass = 6;
switch (BITS(state->words[0],7,8))
{
case 0: instrName = "ld"; state->_load_len = 4; break;
case 1: instrName = "ldb"; state->_load_len = 1; break;
case 2: instrName = "ldw"; state->_load_len = 2; break;
default:
instrName = "??? (0[3])";
state->flow = invalid_instr;
break;
}
break;

case op_ST:

decodingClass = 7;
switch (BITS(state->words[0],1,2))
{
case 0: instrName = "st"; break;
case 1: instrName = "stb"; break;
case 2: instrName = "stw"; break;
default:
instrName = "??? (2[3])";
state->flow = invalid_instr;
break;
}
break;

case op_MAJOR_4:

decodingClass = 0; 
subopcode = BITS(state->words[0],16,21);
switch (subopcode)
{
case 0: instrName = "add"; break;
case 1: instrName = "adc"; break;
case 2: instrName = "sub"; break;
case 3: instrName = "sbc"; break;
case 4: instrName = "and"; break;
case 5: instrName = "or"; break;
case 6: instrName = "bic"; break;
case 7: instrName = "xor"; break;
case 8: instrName = "max"; break;
case 9: instrName = "min"; break;
case 10:
{
if(state->words[0] == 0x264a7000)
{
instrName = "nop";
decodingClass = 26;
}
else
{
instrName = "mov";
decodingClass = 12;
}
break;
}
case 11: instrName = "tst"; decodingClass = 2; break;
case 12: instrName = "cmp"; decodingClass = 2; break;
case 13: instrName = "rcmp"; decodingClass = 2; break;
case 14: instrName = "rsub"; break;
case 15: instrName = "bset"; break;
case 16: instrName = "bclr"; break;
case 17: instrName = "btst"; decodingClass = 2; break;
case 18: instrName = "bxor"; break;
case 19: instrName = "bmsk"; break;
case 20: instrName = "add1"; break;
case 21: instrName = "add2"; break;
case 22: instrName = "add3"; break;
case 23: instrName = "sub1"; break;
case 24: instrName = "sub2"; break;
case 25: instrName = "sub3"; break;
case 30: instrName = "mpyw"; break;
case 31: instrName = "mpyuw"; break;
case 32:
case 33:
instrName = "j";
case 34:
case 35:
if (!instrName) {
instrName = "jl";
}
decodingClass = 4;
condCodeIsPartOfName = 1;
state->isBranch = 1;
break;
case 40:
instrName = "lp";
decodingClass = 11;
condCodeIsPartOfName = 1;
state->isBranch = 1;
break;
case 41: instrName = "flag"; decodingClass = 3; break;
case 42: instrName = "lr"; decodingClass = 10; break;
case 43: instrName = "sr"; decodingClass = 8; break;
case 47:
decodingClass = 1;
switch (BITS(state->words[0],0,5)) 
{
case 0: instrName = "asl"; break;
case 1: instrName = "asr"; break;
case 2: instrName = "lsr"; break;
case 3: instrName = "ror"; break;
case 4: instrName = "rrc"; break;
case 5: instrName = "sexb"; break;
case 6: instrName = "sexw"; break;
case 7: instrName = "extb"; break;
case 8: instrName = "extw"; break;
case 9: instrName = "abs"; break;
case 10: instrName = "not"; break;
case 11: instrName = "rlc"; break;
case 12: instrName = "ex";


decodingClass = 34;
break; 


case 16: instrName = "llock"; decodingClass = 34; break;
case 17: instrName = "scond"; decodingClass = 34; break;


case 63:
decodingClass = 26;
switch (BITS(state->words[0],24,26))
{
case 1 : instrName = "sleep"; decodingClass = 32; break;
case 2 :
if ((info->mach) == ARC_MACH_ARC7) {
instrName = "trap0";
} else {
instrName = "swi";
}
break;
case 3:

if (BITS (state->words[0], 22, 23) == 1) {
instrName = "sync";
}

break;
case 4 : instrName = "rtie" ; break;
case 5 : instrName = "brk"; break;
default:

instrName = "???";
state->flow=invalid_instr;
break;
}
break;
}
break;
}

if (!instrName)
{
subopcode = BITS(state->words[0],17,21);
decodingClass = 5;
switch (subopcode)
{
case 24: instrName = "ld"; state->_load_len = 4; break;
case 25: instrName = "ldb"; state->_load_len = 1; break;
case 26: instrName = "ldw"; state->_load_len = 2; break;
default:
instrName = "??? (0[3])";
state->flow = invalid_instr;
break;
}
}
break;

case op_MAJOR_5:

decodingClass = 0; 
subopcode = BITS(state->words[0],16,21);
switch (subopcode)
{
case 0: instrName = "asl"; break;
case 1: instrName = "lsr"; break;
case 2: instrName = "asr"; break;
case 3: instrName = "ror"; break;
case 4: instrName = "mul64"; mul =1; decodingClass = 2; break;
case 5: instrName = "mulu64"; mul =1; decodingClass = 2; break;


case 6: instrName = "adds" ;break;

case 7: instrName = "subs"; break;
case 8: instrName = "divaw"; break;
case 0xA: instrName = "asls"; break;
case 0xB: instrName = "asrs"; break;
case 0x28: instrName = "addsdw";break;
case 0x29: instrName = "subsdw"; break;

case 47:
switch(BITS(state->words[0],0,5))
{
case 0: instrName = "swap"; decodingClass = 1; break;
case 1: instrName = "norm"; decodingClass = 1; break;

case 2: instrName = "sat16"; decodingClass = 1; break;
case 3: instrName = "rnd16"; decodingClass = 1; break;
case 4: instrName = "abssw"; decodingClass = 1; break;
case 5: instrName = "abss"; decodingClass = 1; break;
case 6: instrName = "negsw"; decodingClass = 1; break;
case 7: instrName = "negs"; decodingClass = 1; break;

case 8: instrName = "normw"; decodingClass = 1; break;


case 9: instrName = "swape"; decodingClass = 1; break;


default:
instrName = "???";
state->flow =invalid_instr;
break;

}
break;
default:
instrName = "??? (2[3])";
state->flow = invalid_instr;
break;
}
break;


case op_MAJOR_6:
decodingClass = 44; 
subopcode = BITS(state->words[0],0,5);
switch (subopcode)
{
case 26: instrName = "rtsc"; break;
default:
instrName = "??? (2[3])";
state->flow = invalid_instr;
break;
}
break;



case op_SIMD:

if (enable_simd)
{
decodingClass = 42;
subopcode = BITS(state->words[0], 17, 23);

switch (subopcode)
{

case 68:
instrName = "vld32";
decodingClass = 37;
usesSimdRegA=1;
usesSimdRegB=2;
usesSimdRegC=0;
simd_scale_u8 = 2;
break;

case 72:
instrName = "vld64";
decodingClass = 37;
usesSimdRegA = 1;
usesSimdRegB = 2;
usesSimdRegC = 0;
simd_scale_u8 = 3;
break;

case 74:
instrName = "vld64w";
decodingClass = 37;
usesSimdRegA = 1;
usesSimdRegB = 2;
usesSimdRegC = 0;
simd_scale_u8 = 3;
break;

case 70:
instrName = "vld32wl";
decodingClass = 37;
usesSimdRegA = 1;
usesSimdRegB = 2;
usesSimdRegC = 0;
simd_scale_u8 = 2;
break;

case 66:
instrName = "vld32wh";
decodingClass = 37;
usesSimdRegA = 1;
usesSimdRegB = 2;
usesSimdRegC = 0;
simd_scale_u8 = 2;
break;

case 76:
instrName = "vld128";
decodingClass = 37;
usesSimdRegA = 1;
usesSimdRegB = 2;
usesSimdRegC = 0;
simd_scale_u8 = 4;
break;

case 78:
{
short sub_subopcode = BITS(state->words[0], 15, 16);
switch (sub_subopcode)
{
case 0:
instrName = "vld128r";
decodingClass = 38;
usesSimdRegA = 1;
usesSimdRegB = usesSimdRegC = 0;
break;
default:
instrName = "SIMD";
state->flow = invalid_instr;
}
}
break;
case 71:
instrName = "vst16_0";
decodingClass = 37;
usesSimdRegA = 1;
usesSimdRegB = 2;
usesSimdRegC = 0;
simd_scale_u8 = 1;
break;

case 81:
instrName = "vst16_1";
decodingClass = 37;
usesSimdRegA = 1;
usesSimdRegB = 2;
usesSimdRegC = 0;
simd_scale_u8 = 1;
break;

case 67:
instrName = "vst16_2";
decodingClass = 37;
usesSimdRegA = 1;
usesSimdRegB = 2;
usesSimdRegC = 0;
simd_scale_u8 = 1;
break;

case 75:
instrName = "vst16_3";
decodingClass = 37;
usesSimdRegA = 1;
usesSimdRegB = 2;
usesSimdRegC = 0;
simd_scale_u8 = 1;
break;

case 83:
instrName = "vst16_4";
decodingClass = 37;
usesSimdRegA = 1;
usesSimdRegB = 2;
usesSimdRegC = 0;
simd_scale_u8 = 1;
break;

case 89:
instrName = "vst16_5";
decodingClass = 37;
usesSimdRegA = 1;
usesSimdRegB = 2;
usesSimdRegC = 0;
simd_scale_u8 = 1;
break;

case 91:
instrName = "vst16_6";
decodingClass = 37;
usesSimdRegA = 1;
usesSimdRegB = 2;
usesSimdRegC = 0;
simd_scale_u8 = 1;
break;

case 93:
instrName = "vst16_7";
decodingClass = 37;
usesSimdRegA = 1;
usesSimdRegB = 2;
usesSimdRegC = 0;
simd_scale_u8 = 1;
break;

case 69:
instrName = "vst32_0";
decodingClass = 37;
usesSimdRegA = 1;
usesSimdRegB = 2;
usesSimdRegC = 0;
simd_scale_u8 = 2;
break;

case 82:
instrName = "vst32_2";
decodingClass = 37;
usesSimdRegA = 1;
usesSimdRegB = 2;
usesSimdRegC = 0;
simd_scale_u8 = 2;
break;

case 86:
instrName = "vst32_4";
decodingClass = 37;
usesSimdRegA = 1;
usesSimdRegB = 2;
usesSimdRegC = 0;
simd_scale_u8 = 2;
break;

case 88:
instrName = "vst32_6";
decodingClass = 37;
usesSimdRegA = 1;
usesSimdRegB = 2;
usesSimdRegC = 0;
simd_scale_u8 = 2;
break;

case 73:
instrName = "vst64";
decodingClass = 37 ;
usesSimdRegA = 1;
usesSimdRegB = 2;
usesSimdRegC = 0;
simd_scale_u8 = 3;
break;

case 77:
instrName = "vst128";
decodingClass = 37;
usesSimdRegA = 1;
usesSimdRegB = 2;
usesSimdRegC = 0;
simd_scale_u8 = 4;
break;

case 79:
{
short sub_subopcode = BITS(state->words[0], 15, 16);
switch (sub_subopcode)
{
case 0:
instrName = "vst128r";
decodingClass = 38;
usesSimdRegA = 1;
usesSimdRegB = usesSimdRegC = 0;
break;

default:
instrName = "SIMD";
state->flow = invalid_instr;
}

}
break;
case 80:
instrName = "vmvw";
usesSimdRegA = usesSimdRegB = 1;
usesSimdRegC = 0;
decodingClass = 39;
break;

case 84:
instrName = "vmvzw";
decodingClass = 39;
usesSimdRegA = usesSimdRegB = 1;
usesSimdRegC = 0;
break;

case 90:
instrName = "vmovw";
decodingClass = 39;
usesSimdRegA = 1;
usesSimdRegB = usesSimdRegC = 0;
break;

case 94:
instrName = "vmovzw";
decodingClass = 39;
usesSimdRegA = 1;
usesSimdRegB = usesSimdRegC = 0;
break;

case 85:
instrName = "vmvaw";
decodingClass = 39;
usesSimdRegA = usesSimdRegB = 1;
usesSimdRegC = 0;
break;

case 95:
instrName = "vmovaw";
decodingClass = 39;
usesSimdRegA = 1;
usesSimdRegB = usesSimdRegC = 0;
break;

case 10:
{
short sub_subopcode = BITS(state->words[0], 15, 16);
switch (sub_subopcode)
{
case 0:
instrName = "vaddw"; decodingClass = 42;
usesSimdRegA = usesSimdRegB = usesSimdRegC =1;
break;

case 1:
instrName = "vaddaw"; decodingClass = 42;
usesSimdRegA = usesSimdRegB = usesSimdRegC =1;
break;

case 2:
instrName = "vbaddw"; decodingClass = 42;
usesSimdRegA = usesSimdRegB = 1;
usesSimdRegC = 0;
break;
}
break;
}

case 11:
{
short sub_subopcode = BITS(state->words[0], 15, 16);
switch (sub_subopcode)
{
case 0:
instrName = "vsubw";
decodingClass = 42;
usesSimdRegA = usesSimdRegB = usesSimdRegC = 1;
break;

case 1:
instrName = "vsubaw";
decodingClass = 42;
usesSimdRegA = usesSimdRegB = usesSimdRegC = 1;
break;

case 2:
instrName = "vbsubw";
decodingClass = 42;
usesSimdRegA = usesSimdRegB = 1;
usesSimdRegC = 0;
break;
}
}
break;

case 12:
{
short sub_subopcode = BITS(state->words[0], 15, 16);
switch (sub_subopcode)
{
case 0:
instrName = "vmulw";
decodingClass = 42;
usesSimdRegA = usesSimdRegB = usesSimdRegC = 1;
break;

case 1:
instrName = "vmulaw";
decodingClass = 42;
usesSimdRegA = usesSimdRegB = usesSimdRegC = 1;
break;

case 2:
instrName = "vbmulw";
decodingClass = 42;
usesSimdRegA = usesSimdRegB = 1;
usesSimdRegC = 0;
break;

case 3:
instrName = "vbmulaw";
decodingClass = 42;
usesSimdRegA = usesSimdRegB = 1;
usesSimdRegC = 0;
break;
}
}
break;

case 13:
{
short sub_subopcode = BITS(state->words[0], 15, 16);
switch (sub_subopcode)
{
case 0:
instrName = "vmulfw";
decodingClass = 42;
usesSimdRegA = usesSimdRegB = usesSimdRegC = 1;
break;

case 1:
instrName = "vmulfaw";
decodingClass = 42;
usesSimdRegA = usesSimdRegB = usesSimdRegC = 1;
break;

case 2:
instrName = "vbmulfw";
decodingClass = 42;
usesSimdRegA = usesSimdRegB = 1;
usesSimdRegC = 0;
break;
}
}
break;

case 15:
{
short sub_subopcode = BITS(state->words[0], 15, 16);
switch (sub_subopcode)
{
case 0:
instrName = "vsummw";
decodingClass = 42;
usesSimdRegA = usesSimdRegB = usesSimdRegC = 1;
break;
case 2:
instrName = "vbrsubw";
decodingClass = 42;
usesSimdRegA = usesSimdRegB = 1;
usesSimdRegC = 0;
break;
}
}
break;

case 23:
{
short sub_subopcode = BITS(state->words[0], 15, 16);
switch (sub_subopcode)
{
case 0:
instrName = "vmr7w";
decodingClass = 42;
usesSimdRegA = usesSimdRegB = usesSimdRegC = 1;
break;

case 1:
instrName = "vmr7aw";
decodingClass = 42;
usesSimdRegA = usesSimdRegB = usesSimdRegC = 1;
break;


case 2:
switch (BITS(state->words[0], 0, 5))
{
case 0:
instrName = "vaddsuw";
decodingClass = 40;
usesSimdRegC = usesSimdRegB = 1;
usesSimdRegA = 0;
break;

case 1:
instrName = "vabsw";
decodingClass = 40;
usesSimdRegC = usesSimdRegB = 1;
usesSimdRegA = 0;
break;

case 2:
instrName = "vsignw";
decodingClass = 40;
usesSimdRegC = usesSimdRegB = 1;
usesSimdRegA = 0;
break;

case 3:
instrName = "vupbw";
decodingClass = 40;
usesSimdRegC = usesSimdRegB = 1;
usesSimdRegA = 0;
break;

case 4:
instrName = "vexch1";
decodingClass = 40;
usesSimdRegC = usesSimdRegB = 1;
usesSimdRegA = 0;
break;

case 5:
instrName = "vexch2";
decodingClass = 40;
usesSimdRegC = usesSimdRegB = 1;
usesSimdRegA = 0;
break;

case 6:
instrName = "vexch4";
decodingClass = 40;
usesSimdRegC = usesSimdRegB = 1;
usesSimdRegA = 0;
break;

case 7:
instrName = "vupsbw";
decodingClass = 40;
usesSimdRegC = usesSimdRegB = 1;
usesSimdRegA = 0;
break;

case 8:
instrName = "vdirun";
decodingClass = 40;
usesSimdRegC = usesSimdRegB = usesSimdRegA = 0;
break;

case 9:
instrName = "vdorun";
decodingClass = 40;
usesSimdRegC = usesSimdRegB = usesSimdRegA = 0;
break;

case 10:
instrName = "vdiwr";
decodingClass = 40;
usesSimdRegB = 3;
usesSimdRegA = usesSimdRegC = 0;
break;

case 11:
instrName = "vdowr";
decodingClass = 40;
usesSimdRegB = 3;
usesSimdRegA = usesSimdRegC = 0;
break;

case 12:
instrName = "vdird";
decodingClass = 40;
usesSimdRegB = 1;
usesSimdRegC = 3;
usesSimdRegA = 0;
break;

case 13:
instrName = "vdord";
decodingClass = 40;
usesSimdRegB = 1;
usesSimdRegC = 3;
usesSimdRegA = 0;
break;

case 63:
{
switch (BITS(state->words[0], 24, 25))
{
case 0:
instrName = "vrec";
decodingClass = 43;
usesSimdRegC = 0;
usesSimdRegB = usesSimdRegA = 0;
break;

case 1:
instrName = "vrecrun";
decodingClass = 43;
usesSimdRegC = 0;
usesSimdRegA = usesSimdRegB = 0;
break;

case 2:
instrName = "vrun";
decodingClass = 43;
usesSimdRegC = 0;
usesSimdRegB = usesSimdRegA = 0;
break;

case 3:
instrName = "vendrec";
decodingClass = 43;
usesSimdRegC = 0;
usesSimdRegB = usesSimdRegA = 0;
break;
}
}
break;
}
break;

case 3:
switch (BITS(state->words[0], 0, 2))
{
case 1:
instrName = "vabsaw";
decodingClass = 40;
usesSimdRegC = usesSimdRegB = 1;
usesSimdRegA = 0;
break;
case 3:
instrName = "vupbaw";
decodingClass = 40;
usesSimdRegC = usesSimdRegB = 1;
usesSimdRegA = 0;
break;
case 7:
instrName = "vupsbaw";
decodingClass = 40;
usesSimdRegC = usesSimdRegB = 1;
usesSimdRegA = 0;
break;
}
break;
}
}
break;

case 16:
instrName = "vasrw";
decodingClass = 42;
usesSimdRegA = usesSimdRegB = 1;
usesSimdRegC = 2;
break;

case 48:
{
short sub_subopcode = BITS(state->words[0], 15, 16);
switch (sub_subopcode)
{
case 0:
instrName = "vasrwi";
decodingClass = 41;
usesSimdRegA = usesSimdRegB = 1;
usesSimdRegC = 0;
break;
case 2:
instrName = "vasrrwi";
decodingClass = 41;
usesSimdRegA = usesSimdRegB = 1;
usesSimdRegC = 0;
break;
}
}
break;

case 59:
instrName = "vasrsrwi";
decodingClass = 41;
usesSimdRegA = usesSimdRegB = 1;
usesSimdRegC = 0;
break;

case 18:
{
short sub_subopcode = BITS(state->words[0], 15, 16);
switch (sub_subopcode)
{
case 0:
instrName = "vmaxw";
usesSimdRegC = 1;
break;
case 1:
instrName = "vmaxaw";
usesSimdRegC = 1;
break;
case 2:
instrName = "vbmaxw";
usesSimdRegC = 0;
break;
}
decodingClass = 42;
usesSimdRegA = usesSimdRegB = 1;
break;
}

case 19:
{
short sub_subopcode = BITS(state->words[0], 15, 16);
switch (sub_subopcode)
{
case 0:
instrName = "vminw";
usesSimdRegC = 1;
break;
case 1:
instrName = "vminaw";
usesSimdRegC = 0;
break;
case 2:
instrName = "vbminw";
usesSimdRegC = 0;
break;
}
decodingClass = 42;
usesSimdRegA = usesSimdRegB = 1;
break;
}

case 14:
{
short sub_subopcode = BITS(state->words[0], 15, 16);
switch (sub_subopcode)
{
case 0:
instrName = "vdifw";
break;
case 1:
instrName = "vdifaw";
break;
case 2:
instrName = "vmrb";
break;
}
decodingClass = 42;
usesSimdRegA = usesSimdRegB = usesSimdRegC = 1;
break;
}

case 24:
{
short sub_subopcode = BITS(state->words[0], 15, 16);
switch (sub_subopcode)
{
case 0:
instrName = "vand";
decodingClass = 42;
usesSimdRegA = usesSimdRegB = usesSimdRegC = 1;
break;
case 1:
instrName = "vandaw";
decodingClass = 42;
usesSimdRegA = usesSimdRegB = usesSimdRegC = 1;
break;
}
break;
}

case 25:
{
short sub_subopcode = BITS(state->words[0], 15, 16);
switch (sub_subopcode)
{
case 0:
instrName = "vor";
decodingClass = 42;
usesSimdRegA = usesSimdRegB = usesSimdRegC = 1;
break;
}
break;
}

case 26:
{
short sub_subopcode = BITS(state->words[0], 15, 16);
switch (sub_subopcode)
{
case 0:
instrName = "vxor";
break;
case 1:
instrName = "vxoraw";
break;
}
decodingClass = 42;
usesSimdRegA = usesSimdRegB = usesSimdRegC = 1;
break;
}

case 27:
{
short sub_subopcode = BITS(state->words[0], 15, 16);
switch (sub_subopcode)
{
case 0:
instrName = "vbic";
break;
case 1:
instrName = "vbicaw";
break;
}
decodingClass = 42;
usesSimdRegA = usesSimdRegB = usesSimdRegC = 1;
break;
}

case 4:
{
short sub_subopcode = BITS(state->words[0], 15, 16);
switch (sub_subopcode)
{
case 0:
instrName = "vavb";
break;
case 2:
instrName = "vavrb";
break;
}
decodingClass = 42;
usesSimdRegA = usesSimdRegB = usesSimdRegC = 1;
break;
}

case 28:
instrName = "veqw";
decodingClass = 42;
usesSimdRegA = usesSimdRegB = usesSimdRegC = 1;
break;

case 29:
instrName = "vnew";
decodingClass = 42;
usesSimdRegA = usesSimdRegB = usesSimdRegC = 1;
break;

case 30:
instrName = "vlew";
decodingClass = 42;
usesSimdRegA = usesSimdRegB = usesSimdRegC = 1;
break;

case 31:
instrName = "vltw";
decodingClass = 42;
usesSimdRegA = usesSimdRegB = usesSimdRegC = 1;
break;

case 49:
{
short sub_subopcode = BITS(state->words[0], 15, 16);
switch (sub_subopcode)
{
case 0:
instrName = "vasrpwbi";
decodingClass = 41;
usesSimdRegA = usesSimdRegB = 1;
usesSimdRegC = 0;
break;
case 2:
instrName = "vasrrpwbi";
decodingClass = 41;
usesSimdRegA = usesSimdRegB = 1;
usesSimdRegC = 0;
break;
}
break;
}

case 5:
{
short sub_subopcode = BITS(state->words[0], 15, 16);
switch (sub_subopcode)
{
case 0:
instrName = "vsr8";
decodingClass = 42;
usesSimdRegA = usesSimdRegB = 1;
usesSimdRegC = 2;
break;

case 1:
instrName = "vsr8aw";
decodingClass = 42;
usesSimdRegA = usesSimdRegB = 1;
usesSimdRegC = 2;
break;
}
break;
}

case 37:
{
short sub_subopcode = BITS(state->words[0], 15, 16);
switch (sub_subopcode)
{
case 0:
instrName = "vsr8i";
decodingClass = 41;
usesSimdRegA = usesSimdRegB = 1;
usesSimdRegC = 0;
break;

case 1:
instrName = "vsr8awi";
decodingClass = 41;
usesSimdRegA = usesSimdRegB = 1;
usesSimdRegC = 0;
break;
}
break;
}

case 20:
case 21:
case 22:
{
short subopcode2 = BITS(state->words[0], 15, 18);
switch (subopcode2)
{
case 0:
instrName = "vmr1w";
break;

case 2:
instrName = "vmr2w";
break;

case 4:
instrName = "vmr3w";
break;

case 6:
instrName = "vmr4w";
break;

case 8:
instrName = "vmr5w";
break;

case 10:
instrName = "vmr6w";
break;

case 1:
instrName = "vmr1aw";
break;

case 3:
instrName = "vmr2aw";
break;

case 5:
instrName = "vmr3aw";
break;

case 7:
instrName = "vmr4aw";
break;

case 9:
instrName = "vmr5aw";
break;

case 11:
instrName = "vmr6aw";
break;

}

decodingClass = 42;
usesSimdRegA = usesSimdRegB = usesSimdRegC = 1;
break;
}


case 7:
case 6:
{
switch (BITS(state->words[0], 16, 19))
{
case 15:
instrName = "vh264ft";
break;
case 14:
instrName = "vh264f";
break;
case 13:
instrName = "vvc1ft";
break;
case 12:
instrName = "vvc1f";
break;
}
decodingClass = 42;
usesSimdRegA = usesSimdRegB = usesSimdRegC = 1;
break;

}

case 92:
instrName = "vd6tapf";
decodingClass = 39;
usesSimdRegA = usesSimdRegB = 1;
usesSimdRegC = 0;
break;

case 55:
instrName = "vinti";
decodingClass = 43;
usesSimdRegA = usesSimdRegB = usesSimdRegC = 0;
break;

default:
instrName = "SIMD";
state->flow = invalid_instr;
break;
}
}
else
{
instrName = "???_SIMD";
state->flow = invalid_instr;
}
break;


case op_LD_ADD:

decodingClass = 15; 
switch(BITS(state->words[0],3,4))
{
case 0: instrName = "ld_s"; break;
case 1: instrName = "ldb_s"; break;
case 2: instrName = "ldw_s"; break;
case 3: instrName = "add_s"; break;
default:
instrName = "??? (2[3])";
state->flow = invalid_instr;
break;
}
break;

case op_ADD_SUB_SHIFT:

decodingClass = 16; 
switch(BITS(state->words[0],3,4))
{
case 0: instrName = "add_s"; break;
case 1: instrName = "sub_s"; break;
case 2: instrName = "asl_s"; break;
case 3: instrName = "asr_s"; break;
default:
instrName = "??? (2[3])";
state->flow = invalid_instr;
break;
}
break;

case op_ADD_MOV_CMP:

decodingClass = 17; 
switch(BITS(state->words[0],3,4))
{
case 0: instrName = "add_s"; break;
case 1:
case 3: instrName = "mov_s"; decodingClass = 18; break;
case 2: instrName = "cmp_s"; decodingClass = 18; break;
default:
instrName = "??? (2[3])";
state->flow = invalid_instr;
break;
}
break;

case op_S:

decodingClass = 22; 
switch(BITS(state->words[0],0,4))
{
case 0:
decodingClass = 27;
switch(BITS(state->words[0],5,7))
{
case 0 : instrName = "j_s";
case 2 :
if (!instrName) {
instrName = "jl_s";
}
state->isBranch = 1;
state->nullifyMode = BR_exec_when_no_jump;
break;
case 1 :
if (!instrName) {
instrName = "j_s.d";
}
case 3 :
if (!instrName) {
instrName = "jl_s.d";
}
state->isBranch = 1;
state->nullifyMode = BR_exec_always;
break;
case 6 : instrName = "sub_s.ne";
decodingClass = 35;
break;
case 7 :
decodingClass = 26;
switch(BITS(state->words[0],8,10))
{
case 0 : instrName = "nop_s"; break;


case 1: instrName = "unimp_s";break;


case 4: instrName = "jeq_s [blink]";
case 5:
if (!instrName) {
instrName = "jne_s [blink]";
}
case 6:
if (!instrName) {
instrName = "j_s [blink]";
}
state->isBranch = 1;
state->nullifyMode = BR_exec_when_no_jump;
break;
case 7:
if (!instrName) {
instrName = "j_s.d [blink]";
}
state->isBranch = 1;
state->nullifyMode = BR_exec_always;
break;
default:
instrName = "??? (2[3])";
state->flow = invalid_instr;
break;
}
break;
default:
instrName = "??? (2[3])";
state->flow = invalid_instr;
break;
}
break;
case 2 : instrName = "sub_s"; break;
case 4 : instrName = "and_s"; break;
case 5 : instrName = "or_s"; break;
case 6 : instrName = "bic_s"; break;
case 7 : instrName = "xor_s"; break;
case 11: instrName = "tst_s"; decodingClass = 14; break;
case 12: instrName = "mul64_s"; mul =1; decodingClass = 14; break;
case 13: instrName = "sexb_s"; decodingClass = 14; break;
case 14: instrName = "sexw_s"; decodingClass = 14; break;
case 15: instrName = "extb_s"; decodingClass = 14; break;
case 16: instrName = "extw_s"; decodingClass = 14; break;
case 17: instrName = "abs_s"; decodingClass = 14; break;
case 18: instrName = "not_s"; decodingClass = 14; break;
case 19: instrName = "neg_s"; decodingClass = 14; break;
case 20: instrName = "add1_s"; break;
case 21: instrName = "add2_s"; break;
case 22: instrName = "add3_s"; break;
case 24: instrName = "asl_s"; break;
case 25: instrName = "lsr_s"; break;
case 26: instrName = "asr_s"; break;
case 27: instrName = "asl_s"; decodingClass = 14; break;
case 28: instrName = "asr_s"; decodingClass = 14; break;
case 29: instrName = "lsr_s"; decodingClass = 14; break;
case 30: instrName = "trap_s"; decodingClass = 33; break;
case 31: instrName = "brk_s"; decodingClass = 26; break;

default:
instrName = "??? (2[3])";
state->flow = invalid_instr;
break;
}
break;

case op_LD_S:

instrName = "ld_s";
decodingClass = 28;
break;

case op_LDB_S:

instrName = "ldb_s";
decodingClass = 28;
break;

case op_LDW_S:

instrName = "ldw_s";
decodingClass = 28;
break;

case op_LDWX_S:

instrName = "ldw_s.x";
decodingClass = 28;
break;

case op_ST_S:

instrName = "st_s";
decodingClass = 28;
break;

case op_STB_S:

instrName = "stb_s";
decodingClass = 28;
break;

case op_STW_S:

instrName = "stw_s";
decodingClass = 28;
break;

case op_Su5:

decodingClass = 23; 
switch (BITS(state->words[0],5,7))
{
case 0: instrName = "asl_s"; break;
case 1: instrName = "lsr_s"; break;
case 2: instrName = "asr_s"; break;
case 3: instrName = "sub_s"; break;
case 4: instrName = "bset_s"; break;
case 5: instrName = "bclr_s"; break;
case 6: instrName = "bmsk_s"; break;
case 7: instrName = "btst_s"; decodingClass = 21; break;
}
break;

case op_SP:

decodingClass = 19; 
switch (BITS(state->words[0],5,7))
{
case 0: instrName = "ld_s"; break;
case 1: instrName = "ldb_s"; break;
case 2: instrName = "st_s"; break;
case 3: instrName = "stb_s"; break;
case 4: instrName = "add_s"; break;
case 5:
if (!BITS (state->words[0], 8, 8)) {
instrName = "add_s";
} else {
instrName = "sub_s";
}
break;
case 6: instrName = "pop_s"; decodingClass = 31; break;
case 7: instrName = "push_s"; decodingClass = 31; break;
default:
instrName = "??? (2[3])";
state->flow = invalid_instr;
break;
}
break;

case op_GP:

decodingClass = 20; 
switch (BITS(state->words[0],9,10))
{
case 0: instrName = "ld_s"; break;
case 1: instrName = "ldb_s"; break;
case 2: instrName = "ldw_s"; break;
case 3: instrName = "add_s"; break;
}
break;

case op_Pcl:

instrName = "ld_s";
decodingClass = 29;
break;

case op_MOV_S:

instrName = "mov_s";
decodingClass = 30;
break;

case op_ADD_CMP:

decodingClass = 21; 
if (BIT (state->words[0], 7)) {
instrName = "cmp_s";
} else {
instrName = "add_s";
}
break;

case op_BR_S:

decodingClass = 25; 
if (BIT (state->words[0], 7)) {
instrName = "brne_s";
} else {
instrName = "breq_s";
}
state->isBranch = 1;
break;

case op_B_S:

decodingClass = 24; 
state->isBranch = 1;
switch (BITS(state->words[0],9,10))
{
case 0: instrName = "b_s"; break;
case 1: instrName = "beq_s"; break;
case 2: instrName = "bne_s"; break;
case 3:
switch (BITS(state->words[0],6,8))
{
case 0: instrName = "bgt_s"; break;
case 1: instrName = "bge_s"; break;
case 2: instrName = "blt_s"; break;
case 3: instrName = "ble_s"; break;
case 4: instrName = "bhi_s"; break;
case 5: instrName = "bhs_s"; break;
case 6: instrName = "blo_s"; break;
case 7: instrName = "bls_s"; break;
}
break;
}
break;

case op_BL_S:

decodingClass = 24; 
instrName = "bl_s";
state->isBranch = 1;
break;

default:

instrName = "???";
state->flow=invalid_instr;
break;
}



if (state->flow == invalid_instr)
{
if (!((state->_opcode == op_SIMD) && enable_simd)) {
instrName = instruction_name (state, state->_opcode,
state->words[0],
&flags);
}

if (state->instructionLen == 2) {
switch (flags) {
case AC_SYNTAX_3OP:
decodingClass = 22;
break;
case AC_SYNTAX_2OP:
decodingClass = 14;
break;
case AC_SYNTAX_1OP:
decodingClass = 36;
break;
case AC_SYNTAX_NOP:
decodingClass = 26;
break;
default:
mwerror (state, "Invalid syntax class\n");
}
}
else
{

switch (flags)
{
case AC_SYNTAX_3OP:
decodingClass = 0;
break;
case AC_SYNTAX_2OP:
decodingClass = 1;
break;
case AC_SYNTAX_1OP:
decodingClass = 32;
break;
case AC_SYNTAX_NOP:
break;
case AC_SYNTAX_SIMD:
break;
default:
mwerror(state, "Invalid syntax class\n");
}
}

if (!instrName)
{
instrName = "???";
state->flow=invalid_instr;
}
}

fieldAisReg = fieldBisReg = fieldCisReg = 1; 
flag = cond = is_shimm = is_limm = 0;
state->nullifyMode = BR_exec_when_no_jump; 
signExtend = addrWriteBack = directMem = 0;
usesAuxReg = 0;


switch (decodingClass)
{
case 0:



subopcode = BITS(state->words[0],22,23);
switch (subopcode)
{
case 0:





CHECK_FIELD_C();
if (!is_limm)
{

CHECK_FIELD_B();
}
else
{
FIELD_B();
if (!fieldBisReg) {
fieldB = fieldC;
}
}
CHECK_FIELD_A();
CHECK_FLAG();
break;

case 1:






CHECK_FIELD_B();
FIELD_C();
fieldCisReg = 0;


state->_offset += fieldB, state->_ea_present = 0;
CHECK_FIELD_A();
CHECK_FLAG();
break;

case 2:






fieldCisReg = 0;
fieldC = FIELDS(state->words[0]);
CHECK_FIELD_B();


state->_offset += fieldB, state->_ea_present = 0;
if (is_limm) {
fieldAisReg = fieldA = 0;
} else {
fieldA = fieldB;
}
CHECK_FLAG();
break;

case 3:







CHECK_FIELD_B();

if (is_limm)
{
fieldAisReg = fieldA = 0;
FIELD_C();
if (BIT (state->words[0], 5)) {
fieldCisReg = 0;
} else if (fieldC == 62) {
fieldCisReg = 0;
fieldC = fieldB;
}
}
else
{
fieldA = fieldB;
if (BIT(state->words[0],5))
{
FIELD_C();
fieldCisReg = 0;
}
else
{
CHECK_FIELD_C();
}
}
CHECK_FLAG_COND();
break;
}

write_instr_name();
WRITE_FORMAT_x(A);
WRITE_FORMAT_COMMA_x(B);
WRITE_FORMAT_COMMA_x(C);
WRITE_NOP_COMMENT();
my_sprintf(state, state->operandBuffer, formatString, fieldA, fieldB, fieldC);
break;

case 1:




CHECK_FIELD_C();
FIELD_B();
CHECK_FLAG();

if (BITS (state->words[0], 22, 23) == 1) {
fieldCisReg = 0;
}
if (fieldCisReg) {
state->ea_reg1 = fieldC;


} else {
state->_offset += fieldB, state->_ea_present = 0;
}

write_instr_name();
WRITE_FORMAT_x(B);
WRITE_FORMAT_COMMA_x(C);
WRITE_NOP_COMMENT();
my_sprintf(state, state->operandBuffer, formatString, fieldB, fieldC);
break;

case 2:




subopcode = BITS(state->words[0],22,23);
if (subopcode == 0 || ((subopcode == 3) && (!BIT(state->words[0],5))))
{
CHECK_FIELD_C();
if (is_limm)
{
FIELD_B();
if (!fieldBisReg) {
fieldB = fieldC;
}
}
else
{
CHECK_FIELD_B();
}
}
else if (subopcode == 1 || ((subopcode == 3) && (BIT(state->words[0],5))))
{
FIELD_C();
fieldCisReg = 0;
CHECK_FIELD_B();
}
else if (subopcode == 2)
{
FIELD_B();
fieldC = FIELDS(state->words[0]);
fieldCisReg = 0;
}
if (subopcode == 3)
CHECK_COND();

if (fieldCisReg) {
state->ea_reg1 = fieldC;


} else {
state->_offset += fieldB, state->_ea_present = 0;
}

write_instr_name();
if (mul)
{

WRITE_FORMAT_x(A);
WRITE_FORMAT_COMMA_x(B);
WRITE_FORMAT_COMMA_x(C);
WRITE_NOP_COMMENT();
my_sprintf(state, state->operandBuffer, formatString, 0, fieldB, fieldC);
}
else
{
WRITE_FORMAT_x(B);
WRITE_FORMAT_COMMA_x(C);
WRITE_NOP_COMMENT();
my_sprintf(state, state->operandBuffer, formatString, fieldB, fieldC);
}
break;

case 3:



subopcode = BITS(state->words[0],22,23);

if (subopcode == 0 || ((subopcode == 3) && (!BIT(state->words[0],5))))
{
CHECK_FIELD_C();
}
else if (subopcode == 1 || ((subopcode == 3) && (BIT(state->words[0],5))))
{
FIELD_C();
fieldCisReg = 0;
}
else if (subopcode == 2)
{
fieldC = FIELDS(state->words[0]);
fieldCisReg = 0;
}
if (subopcode == 3)
CHECK_COND();
flag = 0; 

write_instr_name();
WRITE_FORMAT_x(C);
my_sprintf(state, state->operandBuffer, formatString, fieldC);
break;

case 4:





subopcode = BITS(state->words[0],22,23);
if (subopcode == 0 || ((subopcode == 3) && (!BIT(state->words[0],5))))
{
CHECK_FIELD_C();

if (fieldC == 29 || fieldC == 31)
CHECK_FLAG();
}
else if (subopcode == 1 || ((subopcode == 3) && (BIT(state->words[0],5))))
{
FIELD_C();
fieldCisReg = 0;
}
else if (subopcode == 2)
{
fieldC = FIELDS(state->words[0]);
fieldCisReg = 0;
}

if (subopcode == 3)
CHECK_COND();

state->nullifyMode = BITS(state->words[0],16,16);

if (!fieldCisReg)
{
state->flow = is_linked ? direct_call : direct_jump;
add_target(fieldC);
}
else
{
state->flow = is_linked ? indirect_call : indirect_jump;






state->register_for_indirect_jump = fieldC;
}

write_instr_name();
strcat(formatString,
IS_REG(C)?"[%r]":"%s"); 

if (IS_REG (C)) {
my_sprintf (state, state->operandBuffer, formatString, fieldC);
} else {
my_sprintf (state, state->operandBuffer, formatString,
post_address (state, fieldC));
}
break;

case 5:


CHECK_FIELD_A();
CHECK_FIELD_B();

if(FIELDA(state->words[0]) == 62)
{
instrName = "prefetch";
}



if (is_limm)
{
FIELD_C();
if (!fieldCisReg) {
fieldC = fieldB;
}
}
else
{
CHECK_FIELD_C();
}
if (dbg) {
printf ("5:b reg %d %d c reg %d %d \n",
fieldBisReg, fieldB, fieldCisReg, fieldC);
}
state->_offset = 0;
state->_ea_present = 1;
if (fieldBisReg) {
state->ea_reg1 = fieldB;
} else {
state->_offset += fieldB;
}
if (fieldCisReg) {
state->ea_reg2 = fieldC;
} else {
state->_offset += fieldC;
}
state->_mem_load = 1;

directMem = BIT(state->words[0],15);




#if 0


if (fieldBisReg && (fieldB != 62))
#endif
addrWriteBack = BITS(state->words[0],22,23);
signExtend = BIT(state->words[0],16);

write_instr_name();


if (IS_REG (A)) {
WRITE_FORMAT_x_COMMA_LB (A);
} else {
strcat (formatString, "%*");
WRITE_FORMAT_LB ();
}

if (fieldBisReg || fieldB != 0) {
WRITE_FORMAT_x (B);
} else {
fieldB = fieldC;
}

WRITE_FORMAT_COMMA_x_RB (C);
my_sprintf (state, state->operandBuffer, formatString, fieldA, fieldB, fieldC);
break;

case 6:

CHECK_FIELD_B();
CHECK_FIELD_A();



if(FIELDA(state->words[0]) == 62)
{
instrName = "prefetch";
}

fieldC = FIELDD9(state->words[0]);
fieldCisReg = 0;

if (dbg) {
eprintf ("6:b reg %d %d c 0x%x \n",
fieldBisReg, fieldB, fieldC);
}
state->_ea_present = 1;
state->_offset = fieldC;
state->_mem_load = 1;
if (fieldBisReg) {
state->ea_reg1 = fieldB;


} else {
state->_offset += fieldB, state->_ea_present = 0;
}

directMem = BIT(state->words[0],11);


if (fieldBisReg && (fieldB != 62)) {
addrWriteBack = BITS (state->words[0], 9, 10);
}
signExtend = BIT(state->words[0],6);

write_instr_name();
if (IS_REG (A)) {
WRITE_FORMAT_x_COMMA_LB (A);
} else {
strcat (formatString, "%*");
WRITE_FORMAT_LB ();
}
if (!fieldBisReg)
{
fieldB = state->_offset;
WRITE_FORMAT_x_RB(B);
}
else
{
WRITE_FORMAT_x(B);
WRITE_FORMAT_COMMA_x_RB(C);
}
my_sprintf(state, state->operandBuffer, formatString, fieldA, fieldB, fieldC);
break;

case 7:

CHECK_FIELD_B();
CHECK_FIELD_C();
state->source_operand.registerNum = fieldC;
state->sourceType = fieldCisReg ? ARC_REGISTER : ARC_LIMM ;
fieldA = FIELDD9(state->words[0]); 
fieldAisReg=0;


if (dbg) {
eprintf ("7:b reg %d %x off %x\n",
fieldBisReg, fieldB, fieldA);
}
state->_ea_present = 1;
state->_offset = fieldA;
if (fieldBisReg) {
state->ea_reg1 = fieldB;





} else {
state->_offset += fieldB, state->_ea_present = 0;
}

directMem = BIT(state->words[0],5);
addrWriteBack = BITS(state->words[0],3,4);

write_instr_name();
WRITE_FORMAT_x_COMMA_LB(C);
if (fieldA == 0)
{
WRITE_FORMAT_x_RB(B);
}
else
{
WRITE_FORMAT_x(B);
fieldAisReg = 0;
WRITE_FORMAT_COMMA_x_RB(A);
}
my_sprintf(state, state->operandBuffer, formatString, fieldC, fieldB, fieldA);
break;

case 8:

CHECK_FIELD_B();
switch (BITS(state->words[0],22,23))
{
case 0:
if (is_limm)
{
FIELD_C();
if (!fieldCisReg) {
fieldC = fieldB;
}
}
else
{
CHECK_FIELD_C();
}
break;
case 1:
FIELD_C();
fieldCisReg = 0;
break;
case 2:
fieldC = FIELDS(state->words[0]);
fieldCisReg = 0;
break;
}

write_instr_name();
WRITE_FORMAT_x_COMMA_LB(B);

usesAuxReg = 1;
WRITE_FORMAT_x(C);
WRITE_FORMAT_RB();
my_sprintf(state, state->operandBuffer, formatString, fieldB, fieldC);
break;

case 9:


CHECK_FIELD_C();
if (is_limm || BIT(state->words[0],4))
{
fieldCisReg = 0;
FIELD_B();
}
else
{
CHECK_FIELD_B();
}
fieldAisReg = fieldA = 0;
fieldA = FIELDS9(state->words[0]);
fieldA += (addr & ~0x3);
CHECK_NULLIFY();

write_instr_name();

add_target(fieldA);
state->flow = state->_opcode == op_BLC ? direct_call : direct_jump;
WRITE_FORMAT_x(B);
WRITE_FORMAT_COMMA_x(C);
strcat(formatString, ",%s"); 
WRITE_NOP_COMMENT();
my_sprintf(state, state->operandBuffer, formatString, fieldB, fieldC, post_address(state, fieldA));
break;

case 10:

CHECK_FIELD_B();
switch (BITS(state->words[0],22,23))
{
case 0:
CHECK_FIELD_C(); break;
case 1:
FIELD_C();
fieldCisReg = 0;
break;
case 2:
fieldC = FIELDS(state->words[0]);
fieldCisReg = 0;
break;
}

write_instr_name();
WRITE_FORMAT_x_COMMA_LB(B);

usesAuxReg = 1;
WRITE_FORMAT_x(C);
WRITE_FORMAT_RB();
my_sprintf(state, state->operandBuffer, formatString, fieldB, fieldC);
break;

case 11:


if (BITS(state->words[0],22,23) == 3)
{
FIELD_C();
CHECK_COND();
}
else
{
fieldC = FIELDS(state->words[0]);
}

fieldC = fieldC << 1;
fieldC += (addr & ~0x3);

write_instr_name();


add_target(fieldC);
state->flow = state->_opcode == op_BLC ? direct_call : direct_jump;

fieldCisReg = 0;
strcat(formatString, "%s"); 
my_sprintf(state, state->operandBuffer, formatString, post_address(state, fieldC));
break;

case 12:

FIELD_B();
subopcode = BITS(state->words[0],22,23);
if (subopcode == 0 || ((subopcode == 3) && (!BIT(state->words[0],5))))
{
CHECK_FIELD_C();
}
else if (subopcode == 1 || ((subopcode == 3) && (BIT(state->words[0],5))))
{
FIELD_C();
fieldCisReg = 0;
}
else if (subopcode == 2)
{
fieldC = FIELDS(state->words[0]);
fieldCisReg = 0;
}
if (subopcode == 3)
{
CHECK_FLAG_COND();
}
else
{
CHECK_FLAG();
}

write_instr_name();
WRITE_FORMAT_x(B);
WRITE_FORMAT_COMMA_x(C);
WRITE_NOP_COMMENT();
my_sprintf(state, state->operandBuffer, formatString, fieldB, fieldC);
break;

case 13:


fieldA = 0;
if ((state->_opcode == op_BC && (BIT(state->words[0],16))) ||
(state->_opcode == op_BLC && (BIT(state->words[0],17))))
{

fieldA = (BITS(state->words[0],0,4)) << 10;
}
fieldA |= BITS(state->words[0],6,15);

if (state->_opcode == op_BLC)
{


fieldA = fieldA << 9;
fieldA |= BITS(state->words[0],18,26);
fieldA = fieldA << 2;
}
else
{
fieldA = fieldA << 10;
fieldA |= BITS(state->words[0],17,26);
fieldA = fieldA << 1;
}

if ((state->_opcode == op_BC && (BIT (state->words[0], 16))) ||
(state->_opcode == op_BLC && (BIT (state->words[0], 17)))) {

fieldA = sign_extend (fieldA, 25);
} else {

fieldA = sign_extend (fieldA, 21);
}

fieldA += (addr & ~0x3);

if (BIT (state->words[0], 16) && state->_opcode == op_BC) {
CHECK_NULLIFY ();
} else

if ((state->_opcode == op_BLC && BITS (state->words[0], 16, 17) == 2) || (state->_opcode == op_BC && (BIT (state->words[0], 16)))) {
CHECK_NULLIFY ();
} else {
CHECK_COND_NULLIFY ();
}

write_instr_name();

add_target(fieldA); 
state->flow = state->_opcode == op_BLC 
? direct_call
: direct_jump;



strcat(formatString, "%s"); 
my_sprintf(state, state->operandBuffer, formatString, post_address(state, fieldA));
break;

case 14:



FIELD_C_AC();
FIELD_B_AC();

write_instr_name();
if (mul)
{
fieldA = fieldAisReg = 0;
WRITE_FORMAT_x(A);
WRITE_FORMAT_COMMA_x(B);
} else {
WRITE_FORMAT_x (B);
}
WRITE_FORMAT_COMMA_x(C);
WRITE_NOP_COMMENT();
if (mul) {
my_sprintf (state, state->operandBuffer, formatString, 0, fieldB, fieldC);
} else {
my_sprintf (state, state->operandBuffer, formatString, fieldB, fieldC);
}
break;

case 15:



FIELD_C_AC();
FIELD_B_AC();
FIELD_A_AC();

write_instr_name();

if (BITS(state->words[0],3,4) != 3)
{
WRITE_FORMAT_x_COMMA_LB(A);
WRITE_FORMAT_x(B);
WRITE_FORMAT_COMMA_x_RB(C);
}
else
{
WRITE_FORMAT_x(A);
WRITE_FORMAT_COMMA_x(B);
WRITE_FORMAT_COMMA_x(C);
}
WRITE_NOP_COMMENT();
my_sprintf(state, state->operandBuffer, formatString, fieldA, fieldB, fieldC);
break;

case 16:



FIELD_C_AC();
FIELD_B_AC();
fieldA = FIELDA_AC(state->words[0]);
fieldAisReg = 0;

write_instr_name();
WRITE_FORMAT_x(C);
WRITE_FORMAT_COMMA_x(B);
WRITE_FORMAT_COMMA_x(A);
WRITE_NOP_COMMENT();
my_sprintf(state, state->operandBuffer, formatString, fieldC, fieldB, fieldA);
break;

case 17:



CHECK_FIELD_H_AC();
FIELD_B_AC();

write_instr_name();
WRITE_FORMAT_x(B);
WRITE_FORMAT_COMMA_x(B);
WRITE_FORMAT_COMMA_x(C);
WRITE_NOP_COMMENT();
my_sprintf(state, state->operandBuffer, formatString, fieldB, fieldB, fieldC);
break;

case 18:



if ((BITS(state->words[0],3,4) == 1) || (BITS(state->words[0],3,4) == 2))
{
CHECK_FIELD_H_AC();
}
else if (BITS(state->words[0],3,4) == 3)
{
FIELD_H_AC();
}
FIELD_B_AC();

write_instr_name();
if (BITS(state->words[0],3,4) == 3)
{
WRITE_FORMAT_x(C);
WRITE_FORMAT_COMMA_x(B);
WRITE_NOP_COMMENT();
my_sprintf(state, state->operandBuffer, formatString, fieldC, fieldB);
}
else
{
WRITE_FORMAT_x(B);
WRITE_FORMAT_COMMA_x(C);
WRITE_NOP_COMMENT();
my_sprintf(state, state->operandBuffer, formatString, fieldB, fieldC);
}
break;

case 19:



if (BITS (state->words[0], 5, 7) == 5) {
fieldA = 28;
} else {
FIELD_B_AC ();
fieldA = fieldB;
}
fieldB = 28; 
fieldC = (FIELDU_AC(state->words[0])) << 2;
fieldCisReg = 0;

write_instr_name();

switch (BITS(state->words[0],5,7))
{
case 0:
case 1:
case 2:
case 3:
WRITE_FORMAT_x_COMMA_LB(A);
WRITE_FORMAT_x(B);
WRITE_FORMAT_COMMA_x_RB(C);
break;
case 4:
case 5:
WRITE_FORMAT_x(A);
WRITE_FORMAT_COMMA_x(B);
WRITE_FORMAT_COMMA_x(C);
break;
}
WRITE_NOP_COMMENT();
my_sprintf(state, state->operandBuffer, formatString, fieldA, fieldB, fieldC);
break;

case 20:



fieldA = 0;
fieldB = 26; 
fieldC = FIELDS_AC(state->words[0]);
switch (BITS(state->words[0],9,10))
{
case 0:
case 3:
fieldC = fieldC << 2; break;
case 2:
fieldC = fieldC << 1; break;
}
fieldCisReg = 0;

write_instr_name();

if (BITS(state->words[0],9,10) != 3)
{
WRITE_FORMAT_x_COMMA_LB(A);
WRITE_FORMAT_x(B);
WRITE_FORMAT_COMMA_x_RB(C);
}
else
{
WRITE_FORMAT_x(A);
WRITE_FORMAT_COMMA_x(B);
WRITE_FORMAT_COMMA_x(C);
}
WRITE_NOP_COMMENT();
my_sprintf(state, state->operandBuffer, formatString, fieldA, fieldB, fieldC);
break;

case 21:



FIELD_B_AC();
if (state->_opcode == op_Su5) {
fieldC = (BITS (state->words[0], 0, 4));
} else {
fieldC = (BITS (state->words[0], 0, 6));
}
fieldCisReg = 0;
write_instr_name();

if (!BIT(state->words[0],7))
{
WRITE_FORMAT_x(B);
WRITE_FORMAT_COMMA_x(B);
WRITE_FORMAT_COMMA_x(C);
WRITE_NOP_COMMENT();
my_sprintf(state, state->operandBuffer, formatString, fieldB, fieldB, fieldC);
}
else
{
WRITE_FORMAT_x(B);
WRITE_FORMAT_COMMA_x(C);
WRITE_NOP_COMMENT();
my_sprintf(state, state->operandBuffer, formatString, fieldB, fieldC);
}
break;

case 22:



FIELD_C_AC();
FIELD_B_AC();

write_instr_name();

WRITE_FORMAT_x(B);
WRITE_FORMAT_COMMA_x(B);
WRITE_FORMAT_COMMA_x(C);
WRITE_NOP_COMMENT();
my_sprintf(state, state->operandBuffer, formatString, fieldB, fieldB, fieldC);
break;

case 23:



FIELD_B_AC();
fieldC = FIELDU_AC(state->words[0]);
fieldCisReg = 0;
write_instr_name();
WRITE_FORMAT_x(B);
WRITE_FORMAT_COMMA_x(B);
WRITE_FORMAT_COMMA_x(C);
WRITE_NOP_COMMENT();
my_sprintf(state, state->operandBuffer, formatString, fieldB, fieldB, fieldC);
break;

case 24:



if (state->_opcode == op_BL_S)
{
fieldA = (BITS(state->words[0],0,10)) << 2;
fieldA = sign_extend(fieldA, 13);
}
else if (BITS(state->words[0],9,10) != 3)
{
fieldA = (BITS(state->words[0],0,8)) << 1;
fieldA = sign_extend(fieldA, 10);
}
else
{
fieldA = (BITS(state->words[0],0,5)) << 1;
fieldA = sign_extend(fieldA, 7);
}
fieldA += (addr & ~0x3);

write_instr_name();

add_target(fieldA); 
state->flow = state->_opcode == op_BL_S 
? direct_call
: direct_jump;



strcat(formatString, "%s"); 
my_sprintf(state, state->operandBuffer, formatString, post_address(state, fieldA));
break;

case 25:



FIELD_B_AC();
fieldC = (BITS(state->words[0],0,6)) << 1;
fieldC = sign_extend (fieldC, 8);

fieldC += (addr & ~0x3);
fieldA = fieldAisReg = fieldCisReg = 0;

write_instr_name();

add_target(fieldC); 
state->flow = direct_jump;

WRITE_FORMAT_x(B);
WRITE_FORMAT_COMMA_x(A);
strcat(formatString, ",%s"); 
WRITE_NOP_COMMENT();
my_sprintf(state, state->operandBuffer, formatString, fieldB, fieldA, post_address(state, fieldC));
break;

case 26:



write_instr_name();
state->operandBuffer[0] = '\0';
break;

case 27:



FIELD_B_AC();
write_instr_name();
strcat(formatString,"[%r]");
my_sprintf(state, state->operandBuffer, formatString, fieldB);
break;

case 28:



FIELD_C_AC();
FIELD_B_AC();
switch (state->_opcode)
{
case op_LD_S :
case op_ST_S :
fieldA = (FIELDU_AC(state->words[0])) << 2;
break;
case op_LDB_S :
case op_STB_S :
fieldA = (FIELDU_AC(state->words[0]));
break;
case op_LDW_S :
case op_LDWX_S :
case op_STW_S :
fieldA = (FIELDU_AC(state->words[0])) << 1;
break;
}
fieldAisReg = 0;

write_instr_name();

WRITE_FORMAT_x_COMMA_LB(C);
WRITE_FORMAT_x(B);
WRITE_FORMAT_COMMA_x(A);
WRITE_FORMAT_RB();
WRITE_NOP_COMMENT();
my_sprintf(state, state->operandBuffer, formatString, fieldC, fieldB, fieldA);
break;

case 29:



FIELD_B_AC();
fieldC = 63;
fieldA = (BITS(state->words[0],0,7)) << 2;
fieldAisReg = 0;

write_instr_name();

WRITE_FORMAT_x(B);
WRITE_FORMAT_COMMA_x(C);
WRITE_FORMAT_COMMA_x(A);
WRITE_NOP_COMMENT();
my_sprintf(state, state->operandBuffer, formatString, fieldB, fieldC, fieldA);
break;

case 30:



FIELD_B_AC();
fieldC = (BITS(state->words[0],0,7));
fieldCisReg = 0;

write_instr_name();

WRITE_FORMAT_x(B);
WRITE_FORMAT_COMMA_x(C);
WRITE_NOP_COMMENT();
my_sprintf(state, state->operandBuffer, formatString, fieldB, fieldC);
break;

case 31:



if (BITS(state->words[0],0,4) == 1)
{
FIELD_B_AC();
} else if (BITS (state->words[0], 0, 4) == 17) {
fieldB = 31;
}

write_instr_name();

WRITE_FORMAT_x(B);
WRITE_NOP_COMMENT();
my_sprintf(state, state->operandBuffer, formatString, fieldB);
break;

case 32:



if (!BITS(state->words[0],22,23))
{
CHECK_FIELD_C();
}
else
{
FIELD_C();
fieldCisReg = 0;
}

write_instr_name();

if (!fieldC) {
state->operandBuffer[0] = '\0';
} else {
WRITE_FORMAT_x (C);
WRITE_NOP_COMMENT ();
my_sprintf (state, state->operandBuffer, formatString, fieldC);
}
break;

case 33:



fieldC = FIELDC_AC(state->words[0]);
fieldB = FIELDB_AC(state->words[0]);
fieldCisReg = 0;
fieldBisReg = 0;
write_instr_name();
strcat(formatString,"%d");
my_sprintf(state,state->operandBuffer,formatString, ((fieldB << 3) | fieldC));
break;

case 34:











CHECK_FIELD_C();
FIELD_B();
directMem = BIT(state->words[0],15);

if (BITS (state->words[0], 22, 23) == 1) {
fieldCisReg = 0;
}
if (fieldCisReg) {
state->ea_reg1 = fieldC;
}

write_instr_name();
WRITE_FORMAT_x_COMMA_LB(B);

WRITE_FORMAT_x_RB(C);

WRITE_NOP_COMMENT();
my_sprintf(state, state->operandBuffer, formatString, fieldB, fieldC);
break;

case 35:



FIELD_B_AC();
write_instr_name();
strcat(formatString,"%r,%r,%r");
my_sprintf(state, state->operandBuffer, formatString, fieldB, fieldB, fieldB);
break;

case 36:

FIELD_B_AC();

write_instr_name();

WRITE_FORMAT_x(B);
WRITE_NOP_COMMENT();
my_sprintf(state, state->operandBuffer, formatString, fieldB);

break;


case 44:


fieldB = fieldBisReg = 0;

write_instr_name();
WRITE_FORMAT_x(A);
WRITE_FORMAT_COMMA_x(B);
WRITE_NOP_COMMENT();
my_sprintf(state, state->operandBuffer, formatString, fieldA,
fieldB);
break;



case 37:
case 39:
case 41:










CHECK_FIELD_A();

CHECK_FIELD_B();
if (decodingClass == 41)
{
FIELD_C();
}
else
{
FIELD_U8();

if (simd_scale_u8 > 0) {
fieldC = fieldC << simd_scale_u8;
}
}

fieldCisReg = 0;

write_instr_name();
(decodingClass == 37 ? WRITE_FORMAT_x_COMMA_LB(A) :
WRITE_FORMAT_x_COMMA(A));
WRITE_FORMAT_x_COMMA(B);
(decodingClass == 37 ? WRITE_FORMAT_x_RB(C):
WRITE_FORMAT_x(C));
WRITE_NOP_COMMENT();
my_sprintf(state,state->operandBuffer, formatString, fieldA, fieldB, fieldC);


break;
case 38:




CHECK_FIELD_A();
CHECK_FIELD_B();

write_instr_name();
WRITE_FORMAT_x_COMMA_LB(A);
WRITE_FORMAT_x_RB(B);
WRITE_NOP_COMMENT();
my_sprintf(state,state->operandBuffer, formatString, fieldA, fieldB);

break;
case 40:







CHECK_FIELD_B();
CHECK_FIELD_C();

write_instr_name();
WRITE_FORMAT_x(B);
WRITE_FORMAT_COMMA_x(C);
my_sprintf(state, state->operandBuffer, formatString, fieldB, fieldC);
break;

case 42:


CHECK_FIELD_A();
CHECK_FIELD_B();
FIELD_C();

write_instr_name();
WRITE_FORMAT_x(A);
WRITE_FORMAT_COMMA_x(B);
WRITE_FORMAT_COMMA_x(C);
my_sprintf(state, state->operandBuffer, formatString, fieldA, fieldB, fieldC);
break;

case 43:


CHECK_FIELD_C();

if (BITS (state->words[0], 17, 23) == 55) {
fieldCisReg = 0;
}

write_instr_name();
WRITE_FORMAT_x(C);
my_sprintf(state, state->operandBuffer, formatString, fieldC);
break;


default:
mwerror(state, "Bad decoding class in ARC disassembler");
break;
}

state->_cond = cond;
return state->instructionLen = offset;
}






static const char *
_coreRegName
(
void *_this ATTRIBUTE_UNUSED, 
int v 
)
{
return arcExtMap_coreRegName(v);
}





static const char *
_auxRegName
( void *_this ATTRIBUTE_UNUSED, 
int v 
)
{
return arcExtMap_auxRegName(v);
}






static const char *
_condCodeName
(
void *_this ATTRIBUTE_UNUSED, 
int v 
)
{
return arcExtMap_condCodeName(v);
}





static const char *
_instName
(
void *_this ATTRIBUTE_UNUSED, 
int op1, 
int op2, 
int *flags 
)
{
return arcExtMap_instName(op1, op2, flags);
}

static void
parse_disassembler_options (char *options)
{
const char *p; 
for (p = options; p != NULL; )
{
if (CONST_STRNEQ (p, "simd"))
{
enable_simd = 1;
}
if (CONST_STRNEQ (p, "insn-stream"))
{
enable_insn_stream = 1;
}

p = strchr (p, ',');

if (p != NULL) {
p++;
}
}

}



int
ARCompact_decodeInstr (bfd_vma address, 
disassemble_info* info)
{
int status;
bfd_byte buffer[4];
struct arcDisState s; 
void *stream = info->stream; 
fprintf_ftype func = info->fprintf_func;
int bytes;
int lowbyte, highbyte;
char buf[256];

if (info->disassembler_options)
{
parse_disassembler_options (info->disassembler_options);


info->disassembler_options = NULL;
}

lowbyte = ((info->endian == BFD_ENDIAN_LITTLE) ? 1 : 0);
highbyte = ((info->endian == BFD_ENDIAN_LITTLE) ? 0 : 1);

memset(&s, 0, sizeof(struct arcDisState));


status = (*info->read_memory_func) (address, buffer, 2, info);

if (status != 0)
{
(*info->memory_error_func) (status, address, info);
return -1;
}

if (((buffer[lowbyte] & 0xf8) > 0x38) && ((buffer[lowbyte] & 0xf8) != 0x48))
{
s.instructionLen = 2;
s.words[0] = (buffer[lowbyte] << 8) | buffer[highbyte];
status = (*info->read_memory_func) (address + 2, buffer, 4, info);
if (info->endian == BFD_ENDIAN_LITTLE) {
s.words[1] = bfd_getl32 (buffer);
} else {
s.words[1] = bfd_getb32 (buffer);
}
}
else
{
s.instructionLen = 4;
status = (*info->read_memory_func) (address + 2, &buffer[2], 2, info);
if (status != 0)
{
(*info->memory_error_func) (status, address + 2, info);
return -1;
}
if (info->endian == BFD_ENDIAN_LITTLE) {
s.words[0] = bfd_getl32 (buffer);
} else {
s.words[0] = bfd_getb32 (buffer);
}



status = (*info->read_memory_func) (address + 4, buffer, 4, info);
if (info->endian == BFD_ENDIAN_LITTLE) {
s.words[1] = bfd_getl32 (buffer);
} else {
s.words[1] = bfd_getb32 (buffer);
}
}

s._this = &s;
s.coreRegName = _coreRegName;
s.auxRegName = _auxRegName;
s.condCodeName = _condCodeName;
s.instName = _instName;


bytes = dsmOneArcInst(address, (void *)&s, info);


{
char* instr = s.instrBuffer;
char* operand = s.operandBuffer;
char* space = strchr(instr, ' ');

if (enable_insn_stream)
{


if (s.instructionLen == 2) {
(*func) (stream, " %04x ", (unsigned int)s.words[0]);
} else {
(*func) (stream, "%08x ", (unsigned int)s.words[0]);
}

(*func) (stream, " ");
}


if ((space != NULL) && (operand[0] == '\0'))
{
*space = '\0';
operand = space + 1;
}

(*func) (stream, "%s ", instr);

if (__TRANSLATION_REQUIRED(s))
{
bfd_vma addr;
char *tmpBuffer;
int i = 1;

if (operand[0] != '@')
{


strncpy(buf, operand, sizeof (buf) - 1);
tmpBuffer = strtok(buf,"@");
(*func) (stream, "%s", tmpBuffer);
i = strlen(tmpBuffer) + 1;
}

addr = s.addresses[operand[i] - '0'];
(*info->print_address_func) ((bfd_vma) addr, info);

} else {
(*func) (stream, "%s", operand);
}
}


info->bytes_per_line = 8;

return bytes; 
}







struct arcDisState
arcAnalyzeInstr
(
bfd_vma address, 
disassemble_info* info
)
{
int status;
bfd_byte buffer[4];
struct arcDisState s; 
int bytes;
int lowbyte, highbyte;

lowbyte = ((info->endian == BFD_ENDIAN_LITTLE) ? 1 : 0);
highbyte = ((info->endian == BFD_ENDIAN_LITTLE) ? 0 : 1);

memset(&s, 0, sizeof(struct arcDisState));


status = (*info->read_memory_func) (address, buffer, 2, info);

if (status != 0)
{
(*info->memory_error_func) (status, address, info);
s.instructionLen = -1;
return s;
}

if (((buffer[lowbyte] & 0xf8) > 0x38) && ((buffer[lowbyte] & 0xf8) != 0x48))
{
s.instructionLen = 2;
s.words[0] = (buffer[lowbyte] << 8) | buffer[highbyte];
status = (*info->read_memory_func) (address + 2, buffer, 4, info);
if (info->endian == BFD_ENDIAN_LITTLE) {
s.words[1] = bfd_getl32 (buffer);
} else {
s.words[1] = bfd_getb32 (buffer);
}
}
else
{
s.instructionLen = 4;
status = (*info->read_memory_func) (address + 2, &buffer[2], 2, info);
if (status != 0)
{
(*info->memory_error_func) (status, address + 2, info);
s.instructionLen = -1;
return s;
}
if (info->endian == BFD_ENDIAN_LITTLE) {
s.words[0] = bfd_getl32 (buffer);
} else {
s.words[0] = bfd_getb32 (buffer);
}



status = (*info->read_memory_func) (address + 4, buffer, 4, info);
if (info->endian == BFD_ENDIAN_LITTLE) {
s.words[1] = bfd_getl32 (buffer);
} else {
s.words[1] = bfd_getb32 (buffer);
}
}

s._this = &s;
s.coreRegName = _coreRegName;
s.auxRegName = _auxRegName;
s.condCodeName = _condCodeName;
s.instName = _instName;


bytes = dsmOneArcInst(address, (void *)&s, info);

info->bytes_per_line = bytes;
return s;
}


void
arc_print_disassembler_options (FILE *stream)
{
fprintf (stream, "\n\
ARC-specific disassembler options:\n\
use with the -M switch, with options separated by commas\n\n");

fprintf (stream, " insn-stream Show the instruction byte stream from most\n");
fprintf (stream, " significant byte to least significant byte (excluding LIMM).\n");
fprintf (stream, " This option is useful for viewing the actual encoding of instructions.\n");

fprintf (stream, " simd Enable SIMD instructions disassembly.\n\n");
}
