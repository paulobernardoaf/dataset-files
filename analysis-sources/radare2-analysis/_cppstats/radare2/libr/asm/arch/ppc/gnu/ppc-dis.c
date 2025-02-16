#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sysdep.h>
#include <disas-asm.h>
#include <opcode/ppc.h>
static int print_insn_powerpc (bfd_vma, struct disassemble_info *, int,
ppc_cpu_t);
static bfd_vma __bfd_getb32 (const void *p) {
const bfd_byte *addr = p;
unsigned long v;
v = (unsigned long) addr[0] << 24;
v |= (unsigned long) addr[1] << 16;
v |= (unsigned long) addr[2] << 8;
v |= (unsigned long) addr[3];
return v;
}
static bfd_vma __bfd_getl32 (const void *p) {
const bfd_byte *addr = p;
unsigned long v;
v = (unsigned long) addr[0];
v |= (unsigned long) addr[1] << 8;
v |= (unsigned long) addr[2] << 16;
v |= (unsigned long) addr[3] << 24;
return v;
}
struct dis_private
{
ppc_cpu_t dialect;
};
#define POWERPC_DIALECT(INFO) (((struct dis_private *) ((INFO)->private_data))->dialect)
static int
powerpc_init_dialect (struct disassemble_info *info)
{
ppc_cpu_t dialect = PPC_OPCODE_PPC;
struct dis_private *priv = calloc (sizeof (*priv), 1);
if (priv == NULL) {
return FALSE;
}
if (BFD_DEFAULT_TARGET_SIZE == 64) {
dialect |= PPC_OPCODE_64;
}
if (info->disassembler_options && strstr (info->disassembler_options, "ppcps") != NULL) {
dialect |= PPC_OPCODE_PPCPS;
} else if (info->disassembler_options && strstr (info->disassembler_options, "booke") != NULL) {
dialect |= PPC_OPCODE_BOOKE | PPC_OPCODE_BOOKE64;
} else if ((info->mach == bfd_mach_ppc_e500mc) || (info->disassembler_options && strstr (info->disassembler_options, "e500mc") != NULL)) {
dialect |= (PPC_OPCODE_BOOKE | PPC_OPCODE_ISEL | PPC_OPCODE_PMR | PPC_OPCODE_CACHELCK | PPC_OPCODE_RFMCI | PPC_OPCODE_E500MC);
} else if ((info->mach == bfd_mach_ppc_e500) || (info->disassembler_options && strstr (info->disassembler_options, "e500") != NULL)) {
dialect |= (PPC_OPCODE_BOOKE | PPC_OPCODE_SPE | PPC_OPCODE_ISEL | PPC_OPCODE_EFS | PPC_OPCODE_BRLOCK | PPC_OPCODE_PMR | PPC_OPCODE_CACHELCK | PPC_OPCODE_RFMCI | PPC_OPCODE_E500MC);
} else if (info->disassembler_options && strstr (info->disassembler_options, "efs") != NULL) {
dialect |= PPC_OPCODE_EFS;
} else if (info->disassembler_options && strstr (info->disassembler_options, "e300") != NULL) {
dialect |= PPC_OPCODE_E300 | PPC_OPCODE_CLASSIC | PPC_OPCODE_COMMON;
} else if (info->disassembler_options && (strstr (info->disassembler_options, "440") != NULL || strstr (info->disassembler_options, "464") != NULL)) {
dialect |= PPC_OPCODE_BOOKE | PPC_OPCODE_32 | PPC_OPCODE_440 | PPC_OPCODE_ISEL | PPC_OPCODE_RFMCI;
} else {
dialect |= (PPC_OPCODE_403 | PPC_OPCODE_601 | PPC_OPCODE_CLASSIC | PPC_OPCODE_COMMON | PPC_OPCODE_ALTIVEC);
}
if (info->disassembler_options && strstr (info->disassembler_options, "power4") != NULL) {
dialect |= PPC_OPCODE_POWER4;
}
if (info->disassembler_options && strstr (info->disassembler_options, "power5") != NULL) {
dialect |= PPC_OPCODE_POWER4 | PPC_OPCODE_POWER5;
}
if (info->disassembler_options && strstr (info->disassembler_options, "cell") != NULL) {
dialect |= PPC_OPCODE_POWER4 | PPC_OPCODE_CELL | PPC_OPCODE_ALTIVEC;
}
if (info->disassembler_options && strstr (info->disassembler_options, "power6") != NULL) {
dialect |= PPC_OPCODE_POWER4 | PPC_OPCODE_POWER5 | PPC_OPCODE_POWER6 | PPC_OPCODE_ALTIVEC;
}
if (info->disassembler_options && strstr (info->disassembler_options, "power7") != NULL) {
dialect |= PPC_OPCODE_POWER4 | PPC_OPCODE_POWER5 | PPC_OPCODE_POWER6 | PPC_OPCODE_ALTIVEC | PPC_OPCODE_VSX;
}
if (info->disassembler_options && strstr (info->disassembler_options, "vsx") != NULL) {
dialect |= PPC_OPCODE_VSX;
}
if (info->disassembler_options && strstr (info->disassembler_options, "any") != NULL) {
dialect |= PPC_OPCODE_ANY;
}
if (info->disassembler_options)
{
if (strstr (info->disassembler_options, "32") != NULL) {
dialect &= ~PPC_OPCODE_64;
} else if (strstr (info->disassembler_options, "64") != NULL) {
dialect |= PPC_OPCODE_64;
}
}
info->private_data = priv;
POWERPC_DIALECT(info) = dialect;
return TRUE;
}
int
print_insn_big_powerpc (bfd_vma memaddr, struct disassemble_info *info)
{
if (info->private_data == NULL && !powerpc_init_dialect (info)) {
return -1;
}
return print_insn_powerpc (memaddr, info, 1, POWERPC_DIALECT (info));
}
int
print_insn_little_powerpc (bfd_vma memaddr, struct disassemble_info *info)
{
if (info->private_data == NULL && !powerpc_init_dialect (info)) {
return -1;
}
return print_insn_powerpc (memaddr, info, 0, POWERPC_DIALECT (info));
}
int
print_insn_rs6000 (bfd_vma memaddr, struct disassemble_info *info)
{
return print_insn_powerpc (memaddr, info, 1, PPC_OPCODE_POWER);
}
static long
operand_value_powerpc (const struct powerpc_operand *operand,
unsigned long insn, ppc_cpu_t dialect)
{
long value;
int invalid;
if (operand->extract) {
value = (*operand->extract) (insn, dialect, &invalid);
} else {
value = (insn >> operand->shift) & operand->bitm;
if ((operand->flags & PPC_OPERAND_SIGNED) != 0) {
unsigned long top = operand->bitm;
top |= (top & -top) - 1;
top &= ~(top >> 1);
value = (value ^ top) - top;
}
}
return value;
}
static int
skip_optional_operands (const unsigned char *opindex,
unsigned long insn, ppc_cpu_t dialect)
{
const struct powerpc_operand *operand;
for (; *opindex != 0; opindex++)
{
operand = &powerpc_operands[*opindex];
if ((operand->flags & PPC_OPERAND_NEXT) != 0 || ((operand->flags & PPC_OPERAND_OPTIONAL) != 0 && operand_value_powerpc (operand, insn, dialect) != 0)) {
return 0;
}
}
return 1;
}
static int
print_insn_powerpc (bfd_vma memaddr,
struct disassemble_info *info,
int bigendian,
ppc_cpu_t dialect)
{
bfd_byte buffer[4];
int status;
unsigned long insn;
const struct powerpc_opcode *opcode;
const struct powerpc_opcode *opcode_end;
unsigned long op;
status = (*info->read_memory_func) (memaddr, buffer, 4, info);
if (status != 0)
{
(*info->memory_error_func) (status, memaddr, info);
return -1;
}
if (bigendian) {
insn = __bfd_getb32 (buffer);
} else {
insn = __bfd_getl32 (buffer);
}
op = PPC_OP (insn);
opcode_end = powerpc_opcodes + powerpc_num_opcodes;
again:
for (opcode = powerpc_opcodes; opcode < opcode_end; opcode++)
{
unsigned long table_op;
const unsigned char *opindex;
const struct powerpc_operand *operand;
int invalid;
int need_comma;
int need_paren;
int skip_optional;
table_op = PPC_OP (opcode->opcode);
if (op < table_op) {
break;
}
if (op > table_op) {
continue;
}
if ((insn & opcode->mask) != opcode->opcode || (opcode->flags & dialect) == 0) {
continue;
}
invalid = 0;
for (opindex = opcode->operands; *opindex != 0; opindex++)
{
operand = powerpc_operands + *opindex;
if (operand->extract) {
(*operand->extract) (insn, dialect, &invalid);
}
}
if (invalid) {
continue;
}
if (opcode->operands[0] != 0) {
(*info->fprintf_func) (info->stream, "%s ", opcode->name);
} else {
(*info->fprintf_func) (info->stream, "%s", opcode->name);
}
need_comma = 0;
need_paren = 0;
skip_optional = -1;
for (opindex = opcode->operands; *opindex != 0; opindex++) {
long value;
operand = powerpc_operands + *opindex;
if ((operand->flags & PPC_OPERAND_FAKE) != 0) {
continue;
}
if ((operand->flags & PPC_OPERAND_OPTIONAL) != 0) {
if (skip_optional < 0) {
skip_optional = skip_optional_operands (opindex, insn,
dialect);
}
if (skip_optional) {
continue;
}
}
value = operand_value_powerpc (operand, insn, dialect);
if (need_comma) {
(*info->fprintf_func) (info->stream, ", ");
need_comma = 0;
}
if ((operand->flags & PPC_OPERAND_GPR) != 0 || ((operand->flags & PPC_OPERAND_GPR_0) != 0 && value != 0)) {
(*info->fprintf_func) (info->stream, "r%ld", value);
} else if ((operand->flags & PPC_OPERAND_FPR) != 0) {
(*info->fprintf_func) (info->stream, "f%ld", value);
} else if ((operand->flags & PPC_OPERAND_VR) != 0) {
(*info->fprintf_func) (info->stream, "v%ld", value);
} else if ((operand->flags & PPC_OPERAND_VSR) != 0) {
(*info->fprintf_func) (info->stream, "vs%ld", value);
} else if ((operand->flags & PPC_OPERAND_RELATIVE) != 0) {
(*info->print_address_func) (memaddr + value, info);
} else if ((operand->flags & PPC_OPERAND_ABSOLUTE) != 0) {
(*info->print_address_func) ((bfd_vma)value & 0xffffffff, info);
} else if ((operand->flags & PPC_OPERAND_CR) == 0 || (dialect & PPC_OPCODE_PPC) == 0) {
(*info->fprintf_func) (info->stream, "%ld", value);
} else if ((operand->flags & PPC_OPERAND_FSL) != 0) {
(*info->fprintf_func) (info->stream, "fsl%ld", value);
} else if ((operand->flags & PPC_OPERAND_FCR) != 0) {
(*info->fprintf_func) (info->stream, "fcr%ld", value);
} else if ((operand->flags & PPC_OPERAND_UDI) != 0) {
(*info->fprintf_func) (info->stream, "%ld", value);
} else {
if (operand->bitm == 7) {
(*info->fprintf_func) (info->stream, "cr%ld", value);
} else {
static const char *cbnames[4] = {"lt", "gt", "eq", "so"};
int cr;
int cc;
cr = value >> 2;
if (cr != 0) {
(*info->fprintf_func) (info->stream, "4*cr%d+", cr);
}
cc = value & 3;
(*info->fprintf_func) (info->stream, "%s", cbnames[cc]);
}
}
if (need_paren) {
(*info->fprintf_func) (info->stream, ")");
need_paren = 0;
}
if ((operand->flags & PPC_OPERAND_PARENS) == 0) {
need_comma = 1;
} else {
(*info->fprintf_func) (info->stream, "(");
need_paren = 1;
}
}
return 4;
}
if ((dialect & PPC_OPCODE_ANY) != 0)
{
dialect = ~PPC_OPCODE_ANY;
goto again;
}
return 4;
}
void
print_ppc_disassembler_options (FILE *stream)
{
fprintf (stream, "\n\
The following PPC specific disassembler options are supported for use with\n\
the -M switch:\n");
fprintf (stream, " booke|booke32|booke64 Disassemble the BookE instructions\n");
fprintf (stream, " e300 Disassemble the e300 instructions\n");
fprintf (stream, " e500|e500x2 Disassemble the e500 instructions\n");
fprintf (stream, " e500mc Disassemble the e500mc instructions\n");
fprintf (stream, " 440 Disassemble the 440 instructions\n");
fprintf (stream, " 464 Disassemble the 464 instructions\n");
fprintf (stream, " efs Disassemble the EFS instructions\n");
fprintf (stream, " ppcps Disassemble the PowerPC paired singles instructions\n");
fprintf (stream, " power4 Disassemble the Power4 instructions\n");
fprintf (stream, " power5 Disassemble the Power5 instructions\n");
fprintf (stream, " power6 Disassemble the Power6 instructions\n");
fprintf (stream, " power7 Disassemble the Power7 instructions\n");
fprintf (stream, " vsx Disassemble the Vector-Scalar (VSX) instructions\n");
fprintf (stream, " 32 Do not disassemble 64-bit instructions\n");
fprintf (stream, " 64 Allow disassembly of 64-bit instructions\n");
}
