#include <string.h>
#include <r_types.h>
#include <r_lib.h>
#include <r_asm.h>
#include <r_anal.h>
#include "../../asm/arch/riscv/riscv-opc.c"
#include "../../asm/arch/riscv/riscv.h"
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*a))
#define RISCVARGSMAX (8)
#define RISCVARGSIZE (64)
#define RISCVARGN(x) ((x)->arg[(x)->num++])
static bool init = false;
static const char * const *riscv_gpr_names = riscv_gpr_names_abi;
static const char * const *riscv_fpr_names = riscv_fpr_names_abi;
typedef struct riscv_args {
int num;
char arg[RISCVARGSMAX][RISCVARGSIZE];
} riscv_args_t;
#define is_any(...) _is_any(o->name, __VA_ARGS__, NULL)
static bool _is_any(const char *str, ...) {
char *cur;
va_list va;
va_start (va, str);
while (true) {
cur = va_arg (va, char *);
if (!cur) {
break;
}
if (!strcmp (str, cur)) {
va_end (va);
return true;
}
}
va_end (va);
return false;
}
static void arg_p2(char *buf, unsigned long val, const char* const* array, size_t size) {
const char *s = val >= size || array[val] ? array[val] : "unknown";
snprintf (buf, RISCVARGSIZE, "%s", s);
}
static struct riscv_opcode *get_opcode(insn_t word) {
struct riscv_opcode *op = NULL;
static const struct riscv_opcode *riscv_hash[OP_MASK_OP + 1] = {0};
#define OP_HASH_IDX(i) ((i) & (riscv_insn_length (i) == 2 ? 3 : OP_MASK_OP))
if (!init) {
int i;
for (i=0;i<OP_MASK_OP+1; i++) {
riscv_hash[i] = 0;
}
for (op=riscv_opcodes; op <= &riscv_opcodes[NUMOPCODES - 1]; op++) {
if (!riscv_hash[OP_HASH_IDX (op->match)]) {
riscv_hash[OP_HASH_IDX (op->match)] = op;
}
}
init = true;
}
return (struct riscv_opcode *)riscv_hash[OP_HASH_IDX (word)];
}
static void get_insn_args(riscv_args_t *args, const char *d, insn_t l, uint64_t pc) {
int rs1 = (l >> OP_SH_RS1) & OP_MASK_RS1;
int rd = (l >> OP_SH_RD) & OP_MASK_RD;
uint64_t target;
args->num = 0;
for (; *d != '\0' && args->num < RISCVARGSMAX; d++) {
switch (*d) {
case '^':
switch (*++d) {
case 'd':
snprintf (RISCVARGN(args), RISCVARGSIZE , "%d", rd);
break;
case 's':
snprintf (RISCVARGN(args), RISCVARGSIZE , "%d", rs1);
break;
case 't':
snprintf (RISCVARGN(args), RISCVARGSIZE , "%d", (int) EXTRACT_OPERAND (RS2, l));
break;
case 'j':
snprintf (RISCVARGN(args), RISCVARGSIZE , "%d", (int) EXTRACT_OPERAND (CUSTOM_IMM, l));
break;
}
break;
case 'C': 
switch (*++d) {
case 's': 
case 'w': 
snprintf (RISCVARGN(args), RISCVARGSIZE , "%s",
riscv_gpr_names[EXTRACT_OPERAND (CRS1S, l) + 8]);
break;
case 't': 
case 'x': 
snprintf (RISCVARGN(args), RISCVARGSIZE , "%s",
riscv_gpr_names[EXTRACT_OPERAND (CRS2S, l) + 8]);
break;
case 'U': 
snprintf (RISCVARGN(args), RISCVARGSIZE , "%s", riscv_gpr_names[rd]);
break;
case 'c': 
snprintf (RISCVARGN(args), RISCVARGSIZE , "%s", riscv_gpr_names[X_SP]);
break;
case 'V': 
snprintf (RISCVARGN(args), RISCVARGSIZE , "%s",
riscv_gpr_names[EXTRACT_OPERAND (CRS2, l)]);
break;
case 'i':
snprintf (RISCVARGN(args), RISCVARGSIZE , "%d", (int)EXTRACT_RVC_SIMM3 (l));
break;
case 'j':
snprintf (RISCVARGN(args), RISCVARGSIZE , "%d", (int)EXTRACT_RVC_IMM (l));
break;
case 'k':
snprintf (RISCVARGN(args), RISCVARGSIZE , "%d", (int)EXTRACT_RVC_LW_IMM (l));
break;
case 'l':
snprintf (RISCVARGN(args), RISCVARGSIZE , "%d", (int)EXTRACT_RVC_LD_IMM (l));
break;
case 'm':
snprintf (RISCVARGN(args), RISCVARGSIZE , "%d", (int)EXTRACT_RVC_LWSP_IMM (l));
break;
case 'n':
snprintf (RISCVARGN(args), RISCVARGSIZE , "%d", (int)EXTRACT_RVC_LDSP_IMM (l));
break;
case 'K':
snprintf (RISCVARGN(args), RISCVARGSIZE , "%d", (int)EXTRACT_RVC_ADDI4SPN_IMM (l));
break;
case 'L':
snprintf (RISCVARGN(args), RISCVARGSIZE , "%d", (int)EXTRACT_RVC_ADDI16SP_IMM (l));
break;
case 'M':
snprintf (RISCVARGN(args), RISCVARGSIZE , "%d", (int)EXTRACT_RVC_SWSP_IMM (l));
break;
case 'N':
snprintf (RISCVARGN(args), RISCVARGSIZE , "%d", (int)EXTRACT_RVC_SDSP_IMM (l));
break;
case 'p':
target = EXTRACT_RVC_B_IMM (l) + pc;
snprintf (RISCVARGN(args), RISCVARGSIZE , "0x%"PFMT64x, (ut64) target);
break;
case 'a':
target = EXTRACT_RVC_J_IMM (l) + pc;
snprintf (RISCVARGN(args), RISCVARGSIZE , "0x%"PFMT64x, (ut64)target);
break;
case 'u':
snprintf (RISCVARGN(args), RISCVARGSIZE , "0x%x",
(int) (EXTRACT_RVC_IMM (l) & (RISCV_BIGIMM_REACH-1)));
break;
case '>':
snprintf (RISCVARGN(args), RISCVARGSIZE , "0x%x", (int) EXTRACT_RVC_IMM (l) & 0x3f);
break;
case '<':
snprintf (RISCVARGN(args), RISCVARGSIZE , "0x%x", (int) EXTRACT_RVC_IMM (l) & 0x1f);
break;
case 'T': 
snprintf (RISCVARGN(args), RISCVARGSIZE , "%s",
riscv_fpr_names[EXTRACT_OPERAND (CRS2, l)]);
break;
case 'D': 
snprintf (RISCVARGN(args), RISCVARGSIZE , "%s",
riscv_fpr_names[EXTRACT_OPERAND (CRS2S, l) + 8]);
break;
}
break;
case ',':
case '(':
case ')':
case '[':
case ']':
break;
case '0':
if (!d[1]) {
snprintf (RISCVARGN(args), RISCVARGSIZE , "0");
}
break;
case 'b':
case 's':
snprintf (RISCVARGN(args), RISCVARGSIZE , "%s", riscv_gpr_names[rs1]);
break;
case 't':
snprintf (RISCVARGN(args), RISCVARGSIZE , "%s",
riscv_gpr_names[EXTRACT_OPERAND (RS2, l)]);
break;
case 'u':
snprintf (RISCVARGN(args), RISCVARGSIZE , "0x%x",
(unsigned) EXTRACT_UTYPE_IMM (l) >> RISCV_IMM_BITS);
break;
case 'm':
arg_p2 (RISCVARGN(args), EXTRACT_OPERAND (RM, l),
riscv_rm, ARRAY_SIZE (riscv_rm));
break;
case 'P':
arg_p2 (RISCVARGN(args), EXTRACT_OPERAND (PRED, l),
riscv_pred_succ, ARRAY_SIZE (riscv_pred_succ));
break;
case 'Q':
arg_p2 (RISCVARGN(args), EXTRACT_OPERAND (SUCC, l),
riscv_pred_succ, ARRAY_SIZE (riscv_pred_succ));
break;
case 'o':
case 'j':
snprintf (RISCVARGN(args), RISCVARGSIZE , "%d", (int) EXTRACT_ITYPE_IMM (l));
break;
case 'q':
snprintf (RISCVARGN(args), RISCVARGSIZE , "%d", (int) EXTRACT_STYPE_IMM (l));
break;
case 'a':
target = EXTRACT_UJTYPE_IMM (l) + pc;
snprintf (RISCVARGN(args), RISCVARGSIZE , "0x%"PFMT64x, (ut64)target);
break;
case 'p':
target = EXTRACT_SBTYPE_IMM (l) + pc;
snprintf (RISCVARGN(args), RISCVARGSIZE , "0x%"PFMT64x, (ut64)target);
break;
case 'd':
snprintf (RISCVARGN(args), RISCVARGSIZE , "%s", riscv_gpr_names[rd]);
break;
case 'z':
snprintf (RISCVARGN(args), RISCVARGSIZE , "%s", riscv_gpr_names[0]);
break;
case '>':
snprintf (RISCVARGN(args), RISCVARGSIZE , "0x%x", (int) EXTRACT_OPERAND (SHAMT, l));
break;
case '<':
snprintf (RISCVARGN(args), RISCVARGSIZE , "0x%x", (int) EXTRACT_OPERAND (SHAMTW, l));
break;
case 'S':
case 'U':
snprintf (RISCVARGN(args), RISCVARGSIZE , "%s", riscv_fpr_names[rs1]);
break;
case 'T':
snprintf (RISCVARGN(args), RISCVARGSIZE , "%s", riscv_fpr_names[EXTRACT_OPERAND (RS2, l)]);
break;
case 'D':
snprintf (RISCVARGN(args), RISCVARGSIZE , "%s", riscv_fpr_names[rd]);
break;
case 'R':
snprintf (RISCVARGN(args), RISCVARGSIZE , "%s", riscv_fpr_names[EXTRACT_OPERAND (RS3, l)]);
break;
case 'E':
{
const char* csr_name = NULL;
unsigned int csr = EXTRACT_OPERAND (CSR, l);
switch (csr)
{
#define DECLARE_CSR(name, num) case num: csr_name = #name; break;
#include "../../asm/arch/riscv/riscv-opc.h"
#undef DECLARE_CSR
}
if (csr_name) {
snprintf (RISCVARGN(args), RISCVARGSIZE , "%s", csr_name);
} else {
snprintf (RISCVARGN(args), RISCVARGSIZE , "0x%x", csr);
}
break;
}
case 'Z':
snprintf (RISCVARGN(args), RISCVARGSIZE , "%d", rs1);
break;
default:
snprintf (RISCVARGN(args), RISCVARGSIZE , "#internal error, undefined modifier (%c)",
*d);
return;
}
}
}
static const char* arg_n(riscv_args_t* args, int n) {
if (n >= args->num || !strcmp (args->arg[n], "zero")) {
return "0";
}
return args->arg[n];
}
static int riscv_op(RAnal *anal, RAnalOp *op, ut64 addr, const ut8 *data, int len, RAnalOpMask mask) {
const int no_alias = 1;
riscv_args_t args = {0};
ut64 word = 0;
int xlen = anal->bits;
op->size = 4;
op->addr = addr;
op->type = R_ANAL_OP_TYPE_UNK;
if (len >= sizeof (ut64)) {
word = r_read_ble64 (data, anal->big_endian);
} else if (len >= sizeof (ut32)) {
word = r_read_ble16 (data, anal->big_endian);
} else {
op->type = R_ANAL_OP_TYPE_ILL;
return -1;
}
struct riscv_opcode *o = get_opcode (word);
if (word == UT64_MAX) {
op->type = R_ANAL_OP_TYPE_ILL;
return -1;
}
if (!o || !o->name) {
return op->size;
}
if (mask & R_ANAL_OP_MASK_DISASM) {
op->mnemonic = strdup (o->name);
}
for (; o <= &riscv_opcodes[NUMOPCODES - 1]; o++) {
if (no_alias && (o->pinfo & INSN_ALIAS)) {
continue;
}
if (isdigit ((ut8)(o->subset[0])) && atoi (o->subset) != xlen) {
continue;
}
if (o->match_func && !(o->match_func)(o, word)) {
continue;
}
break;
}
if (o > &riscv_opcodes[NUMOPCODES - 1]) {
return -1;
}
if (o->args) {
const char* name = o->name;
if (o->name[0] == 'c' && o->name[1] == '.') {
name += 2;
}
#define ARG(x) (arg_n (&args, (x)))
get_insn_args (&args, o->args, word, addr);
if (!strcmp (name, "nop")) {
esilprintf (op, ",");
}
else if (!strncmp (name, "addi16sp", 8)) {
esilprintf (op, "%s,sp,+,%s,=", ARG (1), ARG (0));
if (!strcmp (ARG (0), riscv_gpr_names[X_SP])) {
op->stackop = R_ANAL_STACK_INC;
op->stackptr = r_num_math (NULL, ARG (1));
}
} else if (!strncmp (name, "addw", 4)) {
esilprintf (op, "0xffffffff,%s,&,", ARG (2));
r_strbuf_appendf (&op->esil, "0xffffffff,%s,&,", ARG (1));
r_strbuf_appendf (&op->esil, "+,%s,=,", ARG (0));
r_strbuf_appendf (&op->esil, "31,%s,>>,?{,0xffffffff00000000,%s,|=,}", ARG (0), ARG (0));
} else if (!strncmp (name, "add", 3)) {
esilprintf (op, "%s,%s,+,%s,=", ARG (2), ARG (1), ARG (0));
if (name[3] == 'i' && !strcmp (ARG (0), riscv_gpr_names[X_SP]) &&
!strcmp (ARG (1), riscv_gpr_names[X_SP])) {
op->stackop = R_ANAL_STACK_INC;
op->stackptr = -(signed)r_num_math (NULL, ARG (2));
}
} else if (!strncmp (name, "subw", 4)) {
esilprintf (op, "0xffffffff,%s,&,", ARG (2));
r_strbuf_appendf (&op->esil, "0xffffffff,%s,&,", ARG (1));
r_strbuf_appendf (&op->esil, "-,%s,=,", ARG (0));
r_strbuf_appendf (&op->esil, "31,%s,>>,?{,0xffffffff00000000,%s,|=,}", ARG (0), ARG (0));
} else if (!strncmp (name, "sub", 3)) {
esilprintf (op, "%s,%s,-,%s,=", ARG (2), ARG (1), ARG (0));
if (name[3] == 'i' && !strcmp (ARG (0), riscv_gpr_names[X_SP]) &&
!strcmp (ARG (1), riscv_gpr_names[X_SP])) {
op->stackop = R_ANAL_STACK_INC;
op->stackptr = r_num_math (NULL, ARG (2));
}
} else if (!strncmp (name, "mulw", 4)) {
esilprintf (op, "0xffffffff,%s,&,", ARG (2));
r_strbuf_appendf (&op->esil, "0xffffffff,%s,&,", ARG (1));
r_strbuf_appendf (&op->esil, "*,%s,=,", ARG (0));
r_strbuf_appendf (&op->esil, "31,%s,>>,?{,0xffffffff00000000,%s,|=,}", ARG (0), ARG (0));
} else if (!strncmp (name, "mul", 3)) {
esilprintf (op, "%s,%s,*,%s,=", ARG (2), ARG (1), ARG (0));
} else if (!strncmp (name, "div", 3)) {
esilprintf (op, "%s,%s,/,%s,=", ARG (2), ARG (1), ARG (0));
} else if (!strncmp (name, "rem", 3)) {
esilprintf (op, "%s,%s,%%,%s,=", ARG (2), ARG (1), ARG (0));
} else if (!strncmp (name, "xor", 3)) {
esilprintf (op, "%s,%s,^,%s,=", ARG (2), ARG (1), ARG (0));
} else if (!strncmp (name, "or", 2)) {
esilprintf (op, "%s,%s,|,%s,=", ARG (2), ARG (1), ARG (0));
} else if (!strncmp (name, "and", 3)) {
esilprintf (op, "%s,%s,&,%s,=", ARG (2), ARG (1), ARG (0));
} else if (!strcmp (name, "auipc")) {
esilprintf (op, "%s,pc,+,%s,=", ARG (1), ARG (0));
} else if (!strncmp (name, "sll", 3)) {
esilprintf (op, "%s,%s,<<,%s,=", ARG (2), ARG (1), ARG (0));
} else if (!strncmp (name, "srl", 3)) {
esilprintf (op, "%s,%s,>>,%s,=", ARG (2), ARG (1), ARG (0));
} else if (!strncmp (name, "sra", 3)) {
esilprintf (op, "%s,%s,>>,%s,=", ARG (2), ARG (1), ARG (0));
}
else if (!strcmp (name, "mv")) {
esilprintf (op, "%s,%s,=", ARG (1), ARG (0));
} else if (!strcmp (name, "li")) {
esilprintf (op, "%s,%s,=", ARG (1), ARG (0));
} else if (!strcmp (name, "lui")) {
esilprintf (op, "%s0000,%s,=", ARG (1), ARG (0));
}
else if (!strncmp (name, "csrrw", 5)) {
esilprintf (op, "%s,0,+,%s,%s,=,%s,=", ARG (1), ARG (2), ARG (1), ARG (0));
} else if (!strncmp (name, "csrrs", 5)) {
esilprintf (op, "%s,0,+,%s,%s,|=,%s,=", ARG (1), ARG (2), ARG (1), ARG (0));
} else if (!strncmp (name, "csrrc", 5)) {
esilprintf (op, "%s,0,+,%s,1,+,0,-,%s,&=,%s,=", ARG (1), ARG (1), ARG (2), ARG (0));
}
else if (!strcmp (name, "sd") || !strcmp (name, "sdsp")) {
esilprintf (op, "%s,%s,%s,+,=[8]", ARG (0), ARG (2), ARG (1));
} else if (!strcmp (name, "sw") || !strcmp (name, "swsp")) {
esilprintf (op, "%s,%s,%s,+,=[4]", ARG (0), ARG (2), ARG (1));
} else if (!strcmp (name, "sh") || !strcmp (name, "shsp")) {
esilprintf (op, "%s,%s,%s,+,=[2]", ARG (0), ARG (2), ARG (1));
} else if (!strcmp (name, "sb") || !strcmp (name, "sbsp")) {
esilprintf (op, "%s,%s,%s,+,=[1]", ARG (0), ARG (2), ARG (1));
} else if (!strcmp (name, "fsq") || !strcmp (name, "fsqsp")) {
esilprintf (op, "%s,%s,+,[16],%s,=", ARG (2), ARG (1), ARG (0));
} else if (!strcmp (name, "fsd") || !strcmp (name, "fsdsp")) {
esilprintf (op, "%s,%s,%s,+,=[8]", ARG (0), ARG (2), ARG (1));
} else if (!strcmp (name, "fsw") || !strcmp (name, "fswsp")) {
esilprintf (op, "%s,%s,%s,+,=[4]", ARG (0), ARG (2), ARG (1));
} else if (!strcmp (name, "fsh") || !strcmp (name, "fshsp")) {
esilprintf (op, "%s,%s,%s,+,=[2]", ARG (0), ARG (2), ARG (1));
} else if (!strcmp (name, "fsb") || !strcmp (name, "fsbsp")) {
esilprintf (op, "%s,%s,%s,+,=[1]", ARG (0), ARG (2), ARG (1));
}
else if (!strcmp (name, "ld") || !strcmp (name, "ldsp")) {
esilprintf (op, "%s,%s,+,[8],%s,=", ARG (2), ARG (1), ARG (0));
} else if (!strcmp (name, "lw") || !strcmp (name, "lwu") || !strcmp (name, "lwsp")) {
esilprintf (op, "%s,%s,+,[4],%s,=", ARG (2), ARG (1), ARG (0));
} else if (!strcmp (name, "lh") || !strcmp (name, "lhu") || !strcmp (name, "lhsp")) {
esilprintf (op, "%s,%s,+,[2],%s,=", ARG (2), ARG (1), ARG (0));
} else if (!strcmp (name, "lb") || !strcmp (name, "lbu") || !strcmp (name, "lbsp")) {
esilprintf (op, "%s,%s,+,[1],%s,=", ARG (2), ARG (1), ARG (0));
} else if (!strcmp (name, "flq") || !strcmp (name, "flqsp")) {
esilprintf (op, "%s,%s,+,[16],%s,=", ARG (2), ARG (1), ARG (0));
} else if (!strcmp (name, "fld") || !strcmp (name, "fldsp")) {
esilprintf (op, "%s,%s,+,[8],%s,=", ARG (2), ARG (1), ARG (0));
} else if (!strcmp (name, "flw") || !strcmp (name, "flwsp")) {
esilprintf (op, "%s,%s,+,[4],%s,=", ARG (2), ARG (1), ARG (0));
} else if (!strcmp (name, "flh") || !strcmp (name, "flhsp")) {
esilprintf (op, "%s,%s,+,[2],%s,=", ARG (2), ARG (1), ARG (0));
} else if (!strcmp (name, "flb") || !strcmp (name, "flbsp")) {
esilprintf (op, "%s,%s,+,[1],%s,=", ARG (2), ARG (1), ARG (0));
}
else if (!strcmp (name, "jalr")) {
esilprintf (op, "%d,pc,+,%s,=,%s,%s,+,pc,=", op->size, ARG (0), ARG (2), ARG (1));
} else if (!strcmp (name, "jal")) {
esilprintf (op, "%d,pc,+,%s,=,%s,pc,+,pc,=", op->size, ARG (0), ARG (1));
} else if (!strcmp (name, "jr") || !strcmp (name, "j")) {
esilprintf (op, "%s,pc,=", ARG (0));
} else if (!strcmp (name, "ecall") || !strcmp (name, "ebreak")) {
esilprintf (op, "TRAP", ARG (0));
}
else if (!strcmp (name, "beq")) {
esilprintf (op, "%s,%s,==,?{,%s,pc,=,},", ARG (1), ARG (0), ARG (2));
} else if (!strcmp (name, "bne")) {
esilprintf (op, "%s,%s,!=,?{,%s,pc,=,},", ARG (1), ARG (0), ARG (2));
} else if (!strcmp (name, "ble") || !strcmp (name, "bleu")) {
esilprintf (op, "%s,%s,<=,?{,%s,pc,=,},", ARG (1), ARG (0), ARG (2));
} else if (!strcmp (name, "blt") || !strcmp (name, "bltu")) {
esilprintf (op, "%s,%s,<,?{,%s,pc,=,},", ARG (1), ARG (0), ARG (2));
} else if (!strcmp (name, "bge") || !strcmp (name, "bgeu")) {
esilprintf (op, "%s,%s,>=,?{,%s,pc,=,},", ARG (1), ARG (0), ARG (2));
} else if (!strcmp (name, "bgt") || !strcmp (name, "bgtu")) {
esilprintf (op, "%s,%s,>,?{,%s,pc,=,},", ARG (1), ARG (0), ARG (2));
} else if (!strcmp (name, "beqz")) {
esilprintf (op, "%s,0,==,?{,%s,pc,=,},", ARG (0), ARG (1));
} else if (!strcmp (name, "bnez")) {
esilprintf (op, "%s,0,!=,?{,%s,pc,=,},", ARG (0), ARG (1));
} else if (!strcmp (name, "blez")) {
esilprintf (op, "%s,0,<=,?{,%s,pc,=,},", ARG (0), ARG (1));
} else if (!strcmp (name, "bltz")) {
esilprintf (op, "%s,0,<,?{,%s,pc,=,},", ARG (0), ARG (1));
} else if (!strcmp (name, "bgez")) {
esilprintf (op, "%s,0,>=,?{,%s,pc,=,},", ARG (0), ARG (1));
} else if (!strcmp (name, "bgtz")) {
esilprintf (op, "%s,0,>,?{,%s,pc,=,},", ARG (0), ARG (1));
} else if (!strncmp (name, "seq", 3)) {
esilprintf (op, "%s,%s,==,%s,=", ARG (2), ARG (1), ARG (0));
} else if (!strncmp (name, "sne", 3)) {
esilprintf (op, "%s,%s,!=,%s,=", ARG (2), ARG (1), ARG (0));
} else if (!strncmp (name, "sle", 3)) {
esilprintf (op, "%s,%s,<=,%s,=", ARG (2), ARG (1), ARG (0));
} else if (!strncmp (name, "slt", 3)) {
esilprintf (op, "%s,%s,<,%s,=", ARG (2), ARG (1), ARG (0));
} else if (!strncmp (name, "sge", 3)) {
esilprintf (op, "%s,%s,>=,%s,=", ARG (2), ARG (1), ARG (0));
} else if (!strncmp (name, "sgt", 3)) {
esilprintf (op, "%s,%s,>,%s,=", ARG (2), ARG (1), ARG (0));
}
#undef ARG
}
if (is_any ("jal")) {
int rd = (word >> OP_SH_RD) & OP_MASK_RD;
op->type = (rd == 0) ? R_ANAL_OP_TYPE_RET: R_ANAL_OP_TYPE_CALL;
op->jump = EXTRACT_UJTYPE_IMM (word) + addr;
op->fail = addr + 4;
} else if (is_any ("jr")) {
op->type = R_ANAL_OP_TYPE_JMP;
} else if (is_any ("j", "jump")) {
op->type = R_ANAL_OP_TYPE_JMP;
} else if (is_any ("jalr", "ret")) { 
op->type = R_ANAL_OP_TYPE_UCALL;
} else if (is_any ("ret")) {
op->type = R_ANAL_OP_TYPE_RET;
} else if (is_any ("beqz", "beq", "blez", "bgez", "ble",
"bleu", "bge", "bgeu", "bltz", "bgtz", "blt", "bltu",
"bgt", "bgtu", "bnez", "bne")) {
op->type = R_ANAL_OP_TYPE_CJMP;
op->jump = EXTRACT_SBTYPE_IMM (word) + addr;
op->fail = addr + 4;
} else if (is_any ("addi", "addw", "addiw", "add", "auipc")) {
op->type = R_ANAL_OP_TYPE_ADD;
} else if (is_any ("subi", "subw", "sub")) {
op->type = R_ANAL_OP_TYPE_SUB;
} else if (is_any ("xori", "xor")) {
op->type = R_ANAL_OP_TYPE_XOR;
} else if (is_any ("andi", "and")) {
op->type = R_ANAL_OP_TYPE_AND;
} else if (is_any ("ori", "or")) {
op->type = R_ANAL_OP_TYPE_OR;
} else if (is_any ("not")) {
op->type = R_ANAL_OP_TYPE_NOT;
} else if (is_any ("mul", "mulh", "mulhu", "mulhsu", "mulw")) {
op->type = R_ANAL_OP_TYPE_MUL;
} else if (is_any ("div", "divu", "divw", "divuw")) {
op->type = R_ANAL_OP_TYPE_DIV;
} else if (is_any ("sd", "sb", "sh", "sw")) {
op->type = R_ANAL_OP_TYPE_STORE;
} else if (is_any ("ld", "lw", "lwu", "lui", "li",
"lb", "lbu", "lh", "lhu", "la", "lla")) {
op->type = R_ANAL_OP_TYPE_LOAD;
}
return op->size;
}
static char *get_reg_profile(RAnal *anal) {
const char *p = NULL;
switch (anal->bits) {
case 32: p =
"=PC pc\n"
"=SP sp\n" 
"=LR ra\n" 
"=BP s0\n" 
"gpr pc .32 0 0\n"
"gpr ra .32 4 0\n" 
"gpr sp .32 8 0\n" 
"gpr gp .32 12 0\n" 
"gpr tp .32 16 0\n" 
"gpr t0 .32 20 0\n" 
"gpr t1 .32 24 0\n" 
"gpr t2 .32 28 0\n" 
"gpr s0 .32 32 0\n" 
"gpr s1 .32 36 0\n" 
"gpr a0 .32 40 0\n" 
"gpr a1 .32 44 0\n" 
"gpr a2 .32 48 0\n" 
"gpr a3 .32 52 0\n" 
"gpr a4 .32 56 0\n" 
"gpr a5 .32 60 0\n" 
"gpr a6 .32 64 0\n" 
"gpr a7 .32 68 0\n" 
"gpr s2 .32 72 0\n" 
"gpr s3 .32 76 0\n" 
"gpr s4 .32 80 0\n" 
"gpr s5 .32 84 0\n" 
"gpr s6 .32 88 0\n" 
"gpr s7 .32 92 0\n" 
"gpr s8 .32 96 0\n" 
"gpr s9 .32 100 0\n" 
"gpr s10 .32 104 0\n" 
"gpr s11 .32 108 0\n" 
"gpr t3 .32 112 0\n" 
"gpr t4 .32 116 0\n" 
"gpr t5 .32 120 0\n" 
"gpr t6 .32 124 0\n" 
"gpr ft0 .64 128 0\n" 
"gpr ft1 .64 136 0\n" 
"gpr ft2 .64 144 0\n" 
"gpr ft3 .64 152 0\n" 
"gpr ft4 .64 160 0\n" 
"gpr ft5 .64 168 0\n" 
"gpr ft6 .64 176 0\n" 
"gpr ft7 .64 184 0\n" 
"gpr fs0 .64 192 0\n" 
"gpr fs1 .64 200 0\n" 
"gpr fa0 .64 208 0\n" 
"gpr fa1 .64 216 0\n" 
"gpr fa2 .64 224 0\n" 
"gpr fa3 .64 232 0\n" 
"gpr fa4 .64 240 0\n" 
"gpr fa5 .64 248 0\n" 
"gpr fa6 .64 256 0\n" 
"gpr fa7 .64 264 0\n" 
"gpr fs2 .64 272 0\n" 
"gpr fs3 .64 280 0\n" 
"gpr fs4 .64 288 0\n" 
"gpr fs5 .64 296 0\n" 
"gpr fs6 .64 304 0\n" 
"gpr fs7 .64 312 0\n" 
"gpr fs8 .64 320 0\n" 
"gpr fs9 .64 328 0\n" 
"gpr fs10 .64 336 0\n" 
"gpr fs11 .64 344 0\n" 
"gpr ft8 .64 352 0\n" 
"gpr ft9 .64 360 0\n" 
"gpr ft10 .64 368 0\n" 
"gpr ft11 .64 376 0\n" 
"gpr fcsr .32 384 0\n"
"flg nx .1 3072 0\n"
"flg uf .1 3073 0\n"
"flg of .1 3074 0\n"
"flg dz .1 3075 0\n"
"flg nv .1 3076 0\n"
"flg frm .3 3077 0\n"
;
break;
case 64: p =
"=PC pc\n"
"=SP sp\n" 
"=LR ra\n" 
"=BP s0\n" 
"gpr pc .64 0 0\n"
"gpr ra .64 8 0\n" 
"gpr sp .64 16 0\n" 
"gpr gp .64 24 0\n" 
"gpr tp .64 32 0\n" 
"gpr t0 .64 40 0\n" 
"gpr t1 .64 48 0\n" 
"gpr t2 .64 56 0\n" 
"gpr s0 .64 64 0\n" 
"gpr s1 .64 72 0\n" 
"gpr a0 .64 80 0\n" 
"gpr a1 .64 88 0\n" 
"gpr a2 .64 96 0\n" 
"gpr a3 .64 104 0\n" 
"gpr a4 .64 112 0\n" 
"gpr a5 .64 120 0\n" 
"gpr a6 .64 128 0\n" 
"gpr a7 .64 136 0\n" 
"gpr s2 .64 144 0\n" 
"gpr s3 .64 152 0\n" 
"gpr s4 .64 160 0\n" 
"gpr s5 .64 168 0\n" 
"gpr s6 .64 176 0\n" 
"gpr s7 .64 184 0\n" 
"gpr s8 .64 192 0\n" 
"gpr s9 .64 200 0\n" 
"gpr s10 .64 208 0\n" 
"gpr s11 .64 216 0\n" 
"gpr t3 .64 224 0\n" 
"gpr t4 .64 232 0\n" 
"gpr t5 .64 240 0\n" 
"gpr t6 .64 248 0\n" 
"gpr ft0 .64 256 0\n" 
"gpr ft1 .64 264 0\n" 
"gpr ft2 .64 272 0\n" 
"gpr ft3 .64 280 0\n" 
"gpr ft4 .64 288 0\n" 
"gpr ft5 .64 296 0\n" 
"gpr ft6 .64 304 0\n" 
"gpr ft7 .64 312 0\n" 
"gpr fs0 .64 320 0\n" 
"gpr fs1 .64 328 0\n" 
"gpr fa0 .64 336 0\n" 
"gpr fa1 .64 344 0\n" 
"gpr fa2 .64 352 0\n" 
"gpr fa3 .64 360 0\n" 
"gpr fa4 .64 368 0\n" 
"gpr fa5 .64 376 0\n" 
"gpr fa6 .64 384 0\n" 
"gpr fa7 .64 392 0\n" 
"gpr fs2 .64 400 0\n" 
"gpr fs3 .64 408 0\n" 
"gpr fs4 .64 416 0\n" 
"gpr fs5 .64 424 0\n" 
"gpr fs6 .64 432 0\n" 
"gpr fs7 .64 440 0\n" 
"gpr fs8 .64 448 0\n" 
"gpr fs9 .64 456 0\n" 
"gpr fs10 .64 464 0\n" 
"gpr fs11 .64 472 0\n" 
"gpr ft8 .64 480 0\n" 
"gpr ft9 .64 488 0\n" 
"gpr ft10 .64 496 0\n" 
"gpr ft11 .64 504 0\n" 
"gpr fcsr .32 512 0\n"
"flg nx .1 4096 0\n"
"flg uf .1 4097 0\n"
"flg of .1 4098 0\n"
"flg dz .1 4099 0\n"
"flg nv .1 4100 0\n"
"flg frm .3 4101 0\n"
;
break;
}
return (p && *p)? strdup (p): NULL;
}
RAnalPlugin r_anal_plugin_riscv = {
.name = "riscv",
.desc = "RISC-V analysis plugin",
.license = "GPL",
.arch = "riscv",
.bits = 32|64,
.op = &riscv_op,
.get_reg_profile = &get_reg_profile,
};
#if !defined(R2_PLUGIN_INCORE)
R_API RLibStruct radare_plugin = {
.type = R_LIB_TYPE_ANAL,
.data = &r_anal_plugin_riscv,
.version = R2_VERSION
};
#endif
