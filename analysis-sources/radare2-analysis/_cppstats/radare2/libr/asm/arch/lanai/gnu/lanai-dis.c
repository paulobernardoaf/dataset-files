#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ansidecl.h"
#include "opcode/lanai.h"
#include "disas-asm.h"
static char *reg_names[] =
{ "r0", "r1", "pc", "ps", "sp", "fp", "r6", "r7", 
"r8", "r9","r10","r11","r12","r13","r14","r15", 
"r16","r17","r18","r19","r20","r21","r22","r23",
"r24","r25","r26","r27","r28","r29","r30","r31",
};
static char *op_names[] = 
{ "add", "addc", "sub", "subb", "and", "or", "xor", "sh" };
static int is_delayed_branch (unsigned long insn);
static int
is_delayed_branch (insn)
unsigned long insn;
{
int i;
for (i = 0; i < NUMOPCODES; i++)
{
CONST struct lanai_opcode *opcode = &lanai_opcodes[i];
if ((opcode->match & insn) == opcode->match && (opcode->lose & insn) == 0) {
return (opcode->flags & F_BR);
}
}
return 0;
}
static int opcodes_sorted = 0;
static int compare_opcodes (char *a, char *b);
int
print_insn_lanai (memaddr, info)
bfd_vma memaddr;
disassemble_info *info;
{
FILE *stream = info->stream;
bfd_byte buffer[4];
unsigned int insn;
register int i;
if (!opcodes_sorted)
{
qsort ((char *) lanai_opcodes, NUMOPCODES,
sizeof (lanai_opcodes[0]),
(int (*)(const void *,const void *))compare_opcodes);
opcodes_sorted = 1;
}
{
int status =
(*info->read_memory_func) (memaddr, buffer, sizeof (buffer), info);
if (status != 0)
{
(*info->memory_error_func) (status, memaddr, info);
return -1;
}
}
insn = bfd_getb32 (buffer);
info->insn_info_valid = 1; 
info->insn_type = dis_nonbranch; 
info->branch_delay_insns = 0; 
info->target = 0; 
for (i = 0; i < NUMOPCODES; i++)
{
CONST struct lanai_opcode *opcode = &lanai_opcodes[i];
if ((opcode->match & insn) == opcode->match
&& (opcode->lose & insn) == 0)
{
int imm_added_to_rs1 = 0;
if (((!(opcode->match & 0x80000000) 
&& ( !(opcode->match & 0x70000000) 
|| (opcode->match & 0x70000000) == 0x50000000 
))
|| ((opcode->match & 0xf0000000) == 0xc0000000 
&& ( !(opcode->match & 0x00000700) 
|| (opcode->match & 0x00000700) == 0x00000500 )))
&& X_RS1(insn) == X_RD(insn))
{
imm_added_to_rs1 = 1;
}
#if defined(BAD)
if (X_RS1 (insn) != X_RD (insn)
&& strchr (opcode->args, 'r') != 0)
continue;
#endif
(*info->fprintf_func) (stream, "%s", opcode->name);
{
register CONST char *s;
unsigned int imm;
for (s = opcode->args; *s != '\0'; ++s)
{
(*info->fprintf_func) (stream, " ");
switch (*s)
{
default:
(*info->fprintf_func) (stream, "%c", *s);
break;
#define reg(n) (*info->fprintf_func) (stream, "%s", reg_names[n])
case '1':
reg (X_RS1 (insn));
break;
case '2':
reg (X_RS2 (insn));
break;
case '3':
reg (X_RS3 (insn));
break;
case 'd':
reg (X_RD (insn));
break;
#undef reg
case '4': 
(*info->fprintf_func) (stream, "%s", op_names[X_OP1(insn)]);
break;
case '5': 
(*info->fprintf_func) (stream, "%s", op_names[X_OP2(insn)]);
if (insn & L3_RRR_F) {
(*info->fprintf_func) (stream, ".f");
}
break;
case '6': 
(*info->fprintf_func) (stream, "%s", op_names[X_OP2(insn)]);
break;
case 'J':
imm = X_C16(insn)<<16;
goto print_immediate;
case 'j':
imm = X_C16(insn);
goto print_immediate;
case 'L':
imm = (X_C16(insn)<<16)|0xffff;
goto print_immediate;
case 'l':
imm = X_C16(insn)|0xffff0000;
goto print_immediate;
case 'k':
(*info->fprintf_func) (stream, "***ERROR***");
break;
case 'o':
imm = SEX (X_C16(insn), 16);
if (X_RS1 (insn) == 0) {
goto print_address;
}
goto print_immediate;
case 's':
imm = SEX (X_C16(insn), 16);
goto print_immediate;
case 'i':
imm = SEX (X_C10(insn), 10);
if (X_RS1 (insn) == 0) {
goto print_address;
}
goto print_immediate;
case 'I':
imm = X_C21(insn);
goto print_address;
case 'Y':
imm = X_C21(insn);
goto print_address;
case 'B':
imm = X_C25(insn);
goto print_address;
case 'b':
imm = SEX (X_C25(insn), 25);
goto print_address;
print_immediate:
(*info->fprintf_func) (stream, "0x%x", imm);
break;
print_address:
info->target = imm;
(*info->print_address_func) (imm, info);
break;
case 'P':
(*info->fprintf_func) (stream, "%%pc");
break;
case 'p':
(*info->fprintf_func) (stream, "%%ps");
break;
case 'Q':
(*info->fprintf_func) (stream, "%%apc");
break;
case 'q':
(*info->fprintf_func) (stream, "%%aps");
break;
case 'S':
(*info->fprintf_func) (stream, "%%isr");
break;
case 'M':
(*info->fprintf_func) (stream, "%%imr");
break;
case '!':
(*info->fprintf_func) (stream, "%%r1");
break;
case '0':
(*info->fprintf_func) (stream, "%%r0");
break;
}
}
}
if (imm_added_to_rs1)
{
unsigned long prev_insn;
int errcode;
errcode =
(*info->read_memory_func)
(memaddr - 4, buffer, sizeof (buffer), info);
prev_insn = bfd_getb32 (buffer);
if (errcode == 0)
{
if (is_delayed_branch (prev_insn))
{
errcode = (*info->read_memory_func)
(memaddr - 8, buffer, sizeof (buffer), info);
prev_insn = bfd_getb32 (buffer);
}
}
if (errcode == 0)
{
if (((prev_insn & 0xf07c0000) == 0x00000000
|| (prev_insn & 0xf07c0000) == 0x50000000 )
&& X_RD (prev_insn) == X_RS1 (insn)
&& X_RD (prev_insn) )
{
(*info->fprintf_func) (stream, "\t! ");
info->target 
= X_C16( insn) << (L3_RI_H& insn ? 16 : 0);
if((prev_insn & 0xf07c0000) == 0x50000000 ){
info->target 
|= X_C16(prev_insn) << (L3_RI_H&prev_insn ? 16 : 0);
}else{
info->target 
+= X_C16(prev_insn) << (L3_RI_H&prev_insn ? 16 : 0);
}
(*info->print_address_func) (info->target, info);
info->insn_type = dis_dref;
info->data_size = 4; 
}
}
}
info->data_size = F_DATA_SIZE(opcode->flags);
if (opcode->flags & (F_UNBR|F_CONDBR|F_JSR))
{
if (opcode->flags & F_UNBR) {
info->insn_type = dis_branch;
} else if (opcode->flags & F_CONDBR) {
info->insn_type = dis_condbranch;
} else if (opcode->flags & F_JSR) {
info->insn_type = dis_jsr;
} else if (opcode->flags & F_BR) {
info->branch_delay_insns = 1;
}
}
return sizeof (buffer);
}
}
info->insn_type = dis_noninsn; 
(*info->fprintf_func) (stream, "%#8x", insn);
return sizeof (buffer);
}
static int
compare_opcodes (a, b)
char *a, *b;
{
struct lanai_opcode *op0 = (struct lanai_opcode *) a;
struct lanai_opcode *op1 = (struct lanai_opcode *) b;
unsigned long int match0 = op0->match, match1 = op1->match;
unsigned long int lose0 = op0->lose, lose1 = op1->lose;
register unsigned int i;
if (match0 & lose0)
{
fprintf (stderr, "Internal error: bad lanai-opcode.h: \"%s\", %#.8lx, %#.8lx\n",
op0->name, match0, lose0);
op0->lose &= ~op0->match;
lose0 = op0->lose;
}
if (match1 & lose1)
{
fprintf (stderr, "Internal error: bad lanai-opcode.h: \"%s\", %#.8lx, %#.8lx\n",
op1->name, match1, lose1);
op1->lose &= ~op1->match;
lose1 = op1->lose;
}
for (i = 0; i < 32; ++i)
{
unsigned long int x = 1 << i;
int x0 = (match0 & x) != 0;
int x1 = (match1 & x) != 0;
if (x0 != x1) {
return x1 - x0;
}
}
for (i = 0; i < 32; ++i)
{
unsigned long int x = 1 << i;
int x0 = (lose0 & x) != 0;
int x1 = (lose1 & x) != 0;
if (x0 != x1) {
return x1 - x0;
}
}
{
int alias_diff = (op0->flags & F_ALIAS) - (op1->flags & F_ALIAS);
if (alias_diff != 0) {
return alias_diff;
}
}
i = strcmp (op0->name, op1->name);
if (i)
{
if (op0->flags & F_ALIAS) 
{
return i;
}
else
{
fprintf (stderr,
"Internal error: bad lanai-opcode.h: \"%s\" == \"%s\"\n",
op0->name, op1->name);
}
}
{
int length_diff = strlen (op0->args) - strlen (op1->args);
if (length_diff != 0) {
return length_diff;
}
}
{
char *p0 = (char *) strchr(op0->args, '+');
char *p1 = (char *) strchr(op1->args, '+');
if (p0 && p1)
{
if (p0[-1] == 'i' && p1[1] == 'i') {
return 1;
}
if (p0[1] == 'i' && p1[-1] == 'i') {
return -1;
}
}
}
return 0;
}
