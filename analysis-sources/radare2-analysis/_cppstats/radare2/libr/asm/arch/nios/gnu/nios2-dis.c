#include "sysdep.h"
#include "disas-asm.h"
#include "opcode/nios2.h"
#include "libiberty.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#if !defined(EMBEDDED_ENV)
#define SYMTAB_AVAILABLE 1
#include "elf-bfd.h"
#endif
#define INSNLEN 4
typedef struct _nios2_opcode_hash
{
const struct nios2_opcode *opcode;
struct _nios2_opcode_hash *next;
} nios2_opcode_hash;
static bfd_boolean nios2_hash_init = 0;
static nios2_opcode_hash *nios2_hash[(OP_MASK_OP) + 1];
static nios2_opcode_hash *nios2_ps_hash[(OP_MASK_OP) + 1];
static void
nios2_init_opcode_hash (void)
{
unsigned int i;
register const struct nios2_opcode *op;
for (i = 0; i <= OP_MASK_OP; ++i) {
nios2_hash[0] = NULL;
}
for (i = 0; i <= OP_MASK_OP; i++) {
for (op = nios2_opcodes; op < &nios2_opcodes[NUMOPCODES]; op++) {
nios2_opcode_hash *new_hash;
nios2_opcode_hash **bucket = NULL;
if ((op->pinfo & NIOS2_INSN_MACRO) == NIOS2_INSN_MACRO) {
if (i == ((op->match >> OP_SH_OP) & OP_MASK_OP) && (op->pinfo & (NIOS2_INSN_MACRO_MOV | NIOS2_INSN_MACRO_MOVI) & 0x7fffffff)) {
bucket = &(nios2_ps_hash[i]);
}
} else if (i == ((op->match >> OP_SH_OP) & OP_MASK_OP)) {
bucket = &(nios2_hash[i]);
}
if (bucket) {
new_hash =
(nios2_opcode_hash *)malloc (sizeof (nios2_opcode_hash));
if (!new_hash) {
fprintf (stderr,
"error allocating memory...broken disassembler\n");
abort ();
}
new_hash->opcode = op;
new_hash->next = NULL;
while (*bucket) {
bucket = &((*bucket)->next);
}
*bucket = new_hash;
}
}
}
nios2_hash_init = 1;
#if defined(DEBUG_HASHTABLE)
for (i = 0; i <= OP_MASK_OP; ++i)
{
nios2_opcode_hash *tmp_hash = nios2_hash[i];
printf ("index: 0x%02X ops: ", i);
while (tmp_hash != NULL)
{
printf ("%s ", tmp_hash->opcode->name);
tmp_hash = tmp_hash->next;
}
printf ("\n");
}
for (i = 0; i <= OP_MASK_OP; ++i)
{
nios2_opcode_hash *tmp_hash = nios2_ps_hash[i];
printf ("index: 0x%02X ops: ", i);
while (tmp_hash != NULL)
{
printf ("%s ", tmp_hash->opcode->name);
tmp_hash = tmp_hash->next;
}
printf ("\n");
}
#endif 
}
const struct nios2_opcode *
nios2_find_opcode_hash (unsigned long opcode)
{
nios2_opcode_hash *entry;
if (!nios2_hash_init) {
nios2_init_opcode_hash ();
}
for (entry = nios2_ps_hash[(opcode >> OP_SH_OP) & OP_MASK_OP];
entry; entry = entry->next) {
if (entry->opcode->match == (opcode & entry->opcode->mask)) {
return entry->opcode;
}
}
for (entry = nios2_hash[(opcode >> OP_SH_OP) & OP_MASK_OP];
entry; entry = entry->next) {
if (entry->opcode->match == (opcode & entry->opcode->mask)) {
return entry->opcode;
}
}
return NULL;
}
#define NUMREGNAMES 32
static struct nios2_reg *
nios2_coprocessor_regs (void)
{
static struct nios2_reg *cached = NULL;
if (!cached)
{
int i;
for (i = NUMREGNAMES; i < nios2_num_regs; i++) {
if (!strcmp (nios2_regs[i].name, "c0")) {
cached = nios2_regs + i;
break;
}
}
assert (cached);
}
return cached;
}
static struct nios2_reg *
nios2_control_regs (void)
{
static struct nios2_reg *cached = NULL;
if (!cached)
{
int i;
for (i = NUMREGNAMES; i < nios2_num_regs; i++) {
if (!strcmp (nios2_regs[i].name, "status")) {
cached = nios2_regs + i;
break;
}
}
assert (cached);
}
return cached;
}
static int
nios2_print_insn_arg (const char *argptr,
unsigned long opcode, bfd_vma address,
disassemble_info *info)
{
unsigned long i = 0;
struct nios2_reg *reg_base;
switch (*argptr)
{
case ',':
case '(':
case ')':
(*info->fprintf_func) (info->stream, "%c", *argptr);
break;
case 'd':
i = GET_INSN_FIELD (RRD, opcode);
if (GET_INSN_FIELD (OP, opcode) == OP_MATCH_CUSTOM && GET_INSN_FIELD (CUSTOM_C, opcode) == 0) {
reg_base = nios2_coprocessor_regs ();
} else {
reg_base = nios2_regs;
}
if (i < NUMREGNAMES) {
(*info->fprintf_func) (info->stream, "%s", reg_base[i].name);
} else {
(*info->fprintf_func) (info->stream, "unknown");
}
break;
case 's':
i = GET_INSN_FIELD (RRS, opcode);
if (GET_INSN_FIELD (OP, opcode) == OP_MATCH_CUSTOM && GET_INSN_FIELD (CUSTOM_A, opcode) == 0) {
reg_base = nios2_coprocessor_regs ();
} else {
reg_base = nios2_regs;
}
if (i < NUMREGNAMES) {
(*info->fprintf_func) (info->stream, "%s", reg_base[i].name);
} else {
(*info->fprintf_func) (info->stream, "unknown");
}
break;
case 't':
i = GET_INSN_FIELD (RRT, opcode);
if (GET_INSN_FIELD (OP, opcode) == OP_MATCH_CUSTOM && GET_INSN_FIELD (CUSTOM_B, opcode) == 0) {
reg_base = nios2_coprocessor_regs ();
} else {
reg_base = nios2_regs;
}
if (i < NUMREGNAMES) {
(*info->fprintf_func) (info->stream, "%s", reg_base[i].name);
} else {
(*info->fprintf_func) (info->stream, "unknown");
}
break;
case 'i':
i = (int) (GET_INSN_FIELD (IMM16, opcode) << 16) >> 16;
(*info->fprintf_func) (info->stream, "%ld", i);
break;
case 'u':
i = GET_INSN_FIELD (IMM16, opcode);
(*info->fprintf_func) (info->stream, "%ld", i);
break;
case 'o':
i = (int) (GET_INSN_FIELD (IMM16, opcode) << 16) >> 16;
address = address + 4 + i;
(*info->print_address_func) (address, info);
break;
case 'p':
i = GET_INSN_FIELD (CACHE_OPX, opcode);
(*info->fprintf_func) (info->stream, "%ld", i);
break;
case 'j':
i = GET_INSN_FIELD (IMM5, opcode);
(*info->fprintf_func) (info->stream, "%ld", i);
break;
case 'l':
i = GET_INSN_FIELD (CUSTOM_N, opcode);
(*info->fprintf_func) (info->stream, "%lu", i);
break;
case 'm':
i = GET_INSN_FIELD (IMM26, opcode);
address = (address & 0xf0000000) | (i << 2);
(*info->print_address_func) (address, info);
break;
case 'c':
i = GET_INSN_FIELD (IMM5, opcode);
reg_base = nios2_control_regs ();
(*info->fprintf_func) (info->stream, "%s", reg_base[i].name);
break;
case 'b':
i = GET_INSN_FIELD (IMM5, opcode);
(*info->fprintf_func) (info->stream, "%ld", i);
break;
default:
(*info->fprintf_func) (info->stream, "unknown");
break;
}
return 0;
}
static int
nios2_disassemble (bfd_vma address, unsigned long opcode,
disassemble_info *info)
{
const struct nios2_opcode *op;
info->bytes_per_line = INSNLEN;
info->bytes_per_chunk = INSNLEN;
info->display_endian = info->endian;
info->insn_info_valid = 1;
info->branch_delay_insns = 0;
info->data_size = 0;
info->insn_type = dis_nonbranch;
info->target = 0;
info->target2 = 0;
op = nios2_find_opcode_hash (opcode);
if (op != NULL)
{
bfd_boolean is_nop = FALSE;
if (op->pinfo == NIOS2_INSN_MACRO_MOV)
{
int dst, src;
dst = GET_INSN_FIELD (RRD, opcode);
src = GET_INSN_FIELD (RRS, opcode);
if (dst == 0 && src == 0)
{
(*info->fprintf_func) (info->stream, "nop");
is_nop = TRUE;
} else {
(*info->fprintf_func) (info->stream, "%s", op->name);
}
} else {
(*info->fprintf_func) (info->stream, "%s", op->name);
}
if (!is_nop)
{
const char *argstr = op->args;
if (argstr != NULL && *argstr != '\0')
{
(*info->fprintf_func) (info->stream, " ");
while (*argstr != '\0')
{
nios2_print_insn_arg (argstr, opcode, address, info);
++argstr;
}
}
}
}
else
{
info->insn_type = dis_noninsn;
(*info->fprintf_func) (info->stream, "0x%lx", opcode);
}
return INSNLEN;
}
static int
print_insn_nios2 (bfd_vma address, disassemble_info *info,
enum bfd_endian endianness)
{
bfd_byte buffer[INSNLEN];
int status;
status = (*info->read_memory_func) (address, buffer, INSNLEN, info);
if (status == 0)
{
unsigned long insn;
if (endianness == BFD_ENDIAN_BIG) {
insn = (unsigned long)bfd_getb32 (buffer);
} else {
insn = (unsigned long)bfd_getl32 (buffer);
}
status = nios2_disassemble (address, insn, info);
}
else
{
(*info->memory_error_func) (status, address, info);
status = -1;
}
return status;
}
int
print_insn_big_nios2 (bfd_vma address, disassemble_info *info)
{
return print_insn_nios2 (address, info, BFD_ENDIAN_BIG);
}
int
print_insn_little_nios2 (bfd_vma address, disassemble_info *info)
{
return print_insn_nios2 (address, info, BFD_ENDIAN_LITTLE);
}
