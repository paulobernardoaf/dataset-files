#include "sysdep.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include "xtensa-isa.h"
#include "ansidecl.h"
#include <setjmp.h>
#include "disas-asm.h"
#include "libiberty.h"
#if defined(_MSC_VER)
__declspec(dllimport)
#endif
extern xtensa_isa xtensa_default_isa;
#if !defined(MAX)
#define MAX(a,b) (((a)) > ((b)) ? ((a)) : ((b)))
#endif
#if 1
static void nothing() {
return;
}
#define OPCODES_SIGJMP_BUF void*
#define OPCODES_SIGSETJMP(buf) nothing()
#define OPCODES_SIGLONGJMP(buf,val) nothing()
#else
#define OPCODES_SIGJMP_BUF sigjmp_buf
#define OPCODES_SIGSETJMP(buf) sigsetjmp((buf), 0)
#define OPCODES_SIGLONGJMP(buf,val) siglongjmp((buf), (val))
#endif
int show_raw_fields;
struct dis_private
{
bfd_byte *byte_buf;
OPCODES_SIGJMP_BUF bailout;
};
static int
fetch_data (struct disassemble_info *info, bfd_vma memaddr)
{
int length, status = 0;
struct dis_private *priv = (struct dis_private *) info->private_data;
int insn_size = xtensa_isa_maxlength (xtensa_default_isa);
memset (priv->byte_buf, 0, insn_size);
for (length = insn_size; length > 0; length--)
{
status = (*info->read_memory_func) (memaddr, priv->byte_buf, length,
info);
if (status == 0) {
return length;
}
}
(*info->memory_error_func) (status, memaddr, info);
OPCODES_SIGLONGJMP (priv->bailout, 1);
return -1;
}
static void
print_xtensa_operand (bfd_vma memaddr,
struct disassemble_info *info,
xtensa_opcode opc,
int opnd,
unsigned operand_val)
{
xtensa_isa isa = xtensa_default_isa;
int signed_operand_val;
if (show_raw_fields)
{
if (operand_val < 0xa) {
(*info->fprintf_func) (info->stream, "%u", operand_val);
} else {
(*info->fprintf_func) (info->stream, "0x%x", operand_val);
}
return;
}
(void) xtensa_operand_decode (isa, opc, opnd, &operand_val);
signed_operand_val = (int) operand_val;
if (xtensa_operand_is_register (isa, opc, opnd) == 0)
{
if (xtensa_operand_is_PCrelative (isa, opc, opnd) == 1)
{
(void) xtensa_operand_undo_reloc (isa, opc, opnd,
&operand_val, memaddr);
info->target = operand_val;
(*info->print_address_func) (info->target, info);
}
else
{
if ((signed_operand_val > -256) && (signed_operand_val < 256)) {
(*info->fprintf_func) (info->stream, "%d", signed_operand_val);
} else {
(*info->fprintf_func) (info->stream, "0x%x", signed_operand_val);
}
}
}
else
{
int i = 1;
xtensa_regfile opnd_rf = xtensa_operand_regfile (isa, opc, opnd);
(*info->fprintf_func) (info->stream, "%s%u",
xtensa_regfile_shortname (isa, opnd_rf),
operand_val);
while (i < xtensa_operand_num_regs (isa, opc, opnd))
{
operand_val++;
(*info->fprintf_func) (info->stream, ":%s%u",
xtensa_regfile_shortname (isa, opnd_rf),
operand_val);
i++;
} 
}
}
int
print_insn_xtensa (bfd_vma memaddr, struct disassemble_info *info)
{
unsigned operand_val;
int bytes_fetched, size, maxsize, i, n, noperands, nslots;
xtensa_isa isa;
xtensa_opcode opc;
xtensa_format fmt;
struct dis_private priv;
static bfd_byte *byte_buf = NULL;
static xtensa_insnbuf insn_buffer = NULL;
static xtensa_insnbuf slot_buffer = NULL;
int first, first_slot, valid_insn;
if (!xtensa_default_isa) {
xtensa_default_isa = xtensa_isa_init (0, 0);
}
info->target = 0;
maxsize = xtensa_isa_maxlength (xtensa_default_isa);
info->bytes_per_line = MAX (maxsize, 4);
if (!insn_buffer)
{
insn_buffer = xtensa_insnbuf_alloc (xtensa_default_isa);
slot_buffer = xtensa_insnbuf_alloc (xtensa_default_isa);
byte_buf = (bfd_byte *) xmalloc (MAX (maxsize, 4));
}
priv.byte_buf = byte_buf;
info->private_data = (void *) &priv;
#if 0
if (OPCODES_SIGSETJMP (priv.bailout) != 0)
return -1;
#endif
isa = xtensa_default_isa;
size = 0;
nslots = 0;
bytes_fetched = fetch_data (info, memaddr);
memset (insn_buffer, 0, (xtensa_insnbuf_size (isa) *
sizeof (xtensa_insnbuf_word)));
xtensa_insnbuf_from_chars (isa, insn_buffer, priv.byte_buf, bytes_fetched);
fmt = xtensa_format_decode (isa, insn_buffer);
if (fmt == XTENSA_UNDEFINED || ((size = xtensa_format_length (isa, fmt)) > bytes_fetched)) {
valid_insn = 0;
} else {
valid_insn = 1;
nslots = xtensa_format_num_slots (isa, fmt);
for (n = 0; n < nslots; n++) {
xtensa_format_get_slot (isa, fmt, n, insn_buffer, slot_buffer);
if (xtensa_opcode_decode (isa, fmt, n, slot_buffer) == XTENSA_UNDEFINED) {
valid_insn = 0;
break;
}
}
}
if (!valid_insn)
{
(*info->fprintf_func) (info->stream, ".byte %#02x", priv.byte_buf[0]);
return 1;
}
if (nslots > 1) {
(*info->fprintf_func) (info->stream, "{ ");
}
first_slot = 1;
for (n = 0; n < nslots; n++) {
if (first_slot) {
first_slot = 0;
} else {
(*info->fprintf_func) (info->stream, "; ");
}
xtensa_format_get_slot (isa, fmt, n, insn_buffer, slot_buffer);
opc = xtensa_opcode_decode (isa, fmt, n, slot_buffer);
(*info->fprintf_func) (info->stream, "%s",
xtensa_opcode_name (isa, opc));
noperands = xtensa_opcode_num_operands (isa, opc);
first = 1;
for (i = 0; i < noperands; i++) {
if (xtensa_operand_is_visible (isa, opc, i) == 0) {
continue;
}
if (first) {
(*info->fprintf_func) (info->stream, " ");
first = 0;
} else {
(*info->fprintf_func) (info->stream, ", ");
}
(void)xtensa_operand_get_field (isa, opc, i, fmt, n,
slot_buffer, &operand_val);
print_xtensa_operand (memaddr, info, opc, i, operand_val);
}
}
if (nslots > 1) {
(*info->fprintf_func) (info->stream, " }");
}
info->bytes_per_chunk = size;
info->display_endian = info->endian;
return size;
}
