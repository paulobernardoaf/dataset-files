#include "ansidecl.h"
#include "libiberty.h"
#include "disas-asm.h"
#include "opcode/hexagon.h"
#include "mybfd.h"
#include <string.h>
#include "opintl.h"
#include <stdarg.h>
#include "safe-ctype.h"
static int
hexagon_dis_inst(
bfd_vma address,
hexagon_insn insn,
char *instrBuffer,
disassemble_info * info
)
{
const hexagon_opcode *opcode;
int len;
char *errmsg = NULL;
len = HEXAGON_INSN_LEN;
opcode = hexagon_lookup_insn(insn);
if (opcode) {
if (!hexagon_dis_opcode(instrBuffer, insn, address, opcode, &errmsg)) {
if (errmsg) {
(*info->fprintf_func) (info->stream, "%s", errmsg);
strcpy(instrBuffer, "");
}
}
return len;
}
strcpy(instrBuffer, "<unknown>");
return 4;
}
static int
hexagon_decode_inst(
bfd_vma address, 
disassemble_info * info
)
{
int status;
hexagon_insn insn;
bfd_byte buffer[4];
void *stream = info->stream; 
fprintf_ftype func = info->fprintf_func;
char instrBuffer[100];
int bytes;
char *str;
#if 0
status = (*info->read_memory_func)(address, buffer, 2, info);
if (status != 0) {
(*info->memory_error_func)(status, address, info);
return -1;
}
if (info->endian == BFD_ENDIAN_LITTLE) {
insn = bfd_getl16 (buffer);
}
else {
insn = bfd_getb16 (buffer);
}
#endif
status = (*info->read_memory_func)(address, buffer, 4, info);
if (status != 0) {
(*info->memory_error_func)(status, address, info);
return -1;
}
if (info->endian == BFD_ENDIAN_LITTLE) {
insn = bfd_getl32(buffer);
}
else {
insn = bfd_getb32(buffer);
}
bytes = hexagon_dis_inst(address, insn, instrBuffer, info);
if (bytes == 2) {
(*func) (stream, " %04x ", insn);
}
else {
(*func) (stream, "%08x ", insn);
}
(*func) (stream, " ");
str = instrBuffer;
while (*str) {
char ch = *str++;
if (ch == '@') {
bfd_vma addr = 0;
while (ISDIGIT(*str)) {
ch = *str++;
addr = 10*addr + (ch - '0');
}
(*info->print_address_func)(addr, info);
}
else {
(*func)(stream, "%c", ch);
}
}
return bytes;
}
disassembler_ftype
hexagon_get_disassembler_from_mach(
unsigned long machine,
unsigned long big_p
)
{
hexagon_opcode_init_tables(hexagon_get_opcode_mach(machine, big_p));
return hexagon_decode_inst;
}
disassembler_ftype
hexagon_get_disassembler(
bfd *abfd
)
{
unsigned long machine = 0; 
unsigned long big_p = bfd_big_endian(abfd);
return (hexagon_get_disassembler_from_mach(machine, big_p));
}
