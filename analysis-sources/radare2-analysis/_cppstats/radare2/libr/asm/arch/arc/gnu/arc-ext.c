#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "arc-ext.h"
#include "arc.h"
#if !defined(_MSC_VER)
#include "libiberty.h"
#include "sysdep.h"
#endif
#define FIRST_EXTENSION_CORE_REGISTER 32
#define LAST_EXTENSION_CORE_REGISTER 59
#define FIRST_EXTENSION_CONDITION_CODE 0x10
#define LAST_EXTENSION_CONDITION_CODE 0x1f
#define NUM_EXT_CORE (LAST_EXTENSION_CORE_REGISTER - FIRST_EXTENSION_CORE_REGISTER + 1)
#define NUM_EXT_COND (LAST_EXTENSION_CONDITION_CODE - FIRST_EXTENSION_CONDITION_CODE + 1)
#define INST_HASH_BITS 6
#define INST_HASH_SIZE (1 << INST_HASH_BITS)
#define INST_HASH_MASK (INST_HASH_SIZE - 1)
struct ExtInstruction
{
char major;
char minor;
char flags;
char* name;
struct ExtInstruction* next;
};
struct ExtAuxRegister
{
long address;
char* name;
struct ExtAuxRegister* next;
};
struct ExtCoreRegister
{
short number;
enum ExtReadWrite rw;
char* name;
};
struct arcExtMap
{
struct ExtAuxRegister* auxRegisters;
struct ExtInstruction* instructions[INST_HASH_SIZE];
struct ExtCoreRegister coreRegisters[NUM_EXT_CORE];
char* condCodes[NUM_EXT_COND];
};
static struct arcExtMap arc_extension_map;
#define INST_HASH(MAJOR, MINOR) ((((MAJOR) << 3) ^ (MINOR)) & INST_HASH_MASK)
#if 0
static void create_map(unsigned char *block, unsigned long length)
{
unsigned char *p = block;
while (p && p < (block + length))
{
if (p[0] == 0)
break;
switch (p[1])
{ 
case EXT_INSTRUCTION:
{
struct ExtInstruction *insn = XNEW (struct ExtInstruction);
int major = p[2];
int minor = p[3];
struct ExtInstruction **bucket =
&arc_extension_map.instructions[INST_HASH (major, minor)];
insn->name = strdup ((char *) (p+5));
insn->major = major;
insn->minor = minor;
insn->flags = p[4];
insn->next = *bucket;
*bucket = insn;
break;
}
case EXT_CORE_REGISTER:
{
unsigned char number = p[2];
char* name = (char *) p+3;
arc_extension_map.coreRegisters[number - FIRST_EXTENSION_CORE_REGISTER].number = number;
arc_extension_map.coreRegisters[number - FIRST_EXTENSION_CORE_REGISTER].rw = REG_READWRITE;
arc_extension_map.coreRegisters[number - FIRST_EXTENSION_CORE_REGISTER].name = strdup (name);
break;
}
case EXT_LONG_CORE_REGISTER:
{
unsigned char number = p[2];
char* name = (char *) p+7;
enum ExtReadWrite rw = p[6];
arc_extension_map.coreRegisters[number - FIRST_EXTENSION_CORE_REGISTER].number = number;
arc_extension_map.coreRegisters[number - FIRST_EXTENSION_CORE_REGISTER].rw = rw;
arc_extension_map.coreRegisters[number - FIRST_EXTENSION_CORE_REGISTER].name = strdup (name);
}
case EXT_COND_CODE:
{
char *cc_name = strdup ((char *) (p+3));
arc_extension_map.condCodes[p[2] - FIRST_EXTENSION_CONDITION_CODE] = cc_name;
break;
}
case EXT_AUX_REGISTER:
{
struct ExtAuxRegister *newAuxRegister = XNEW (struct ExtAuxRegister);
char *aux_name = strdup ((char *) (p+6));
newAuxRegister->name = aux_name;
newAuxRegister->address = p[2]<<24 | p[3]<<16 | p[4]<<8 | p[5];
newAuxRegister->next = arc_extension_map.auxRegisters;
arc_extension_map.auxRegisters = newAuxRegister;
break;
}
default:
break;
}
p += p[0]; 
}
}
static void destroy_map(void)
{
struct ExtAuxRegister *r;
unsigned int i;
r = arc_extension_map.auxRegisters;
while (r)
{
struct ExtAuxRegister* next = r->next;
free (r->name);
free (r);
r = next;
}
for (i = 0; i < INST_HASH_SIZE; i++)
{
struct ExtInstruction *insn = arc_extension_map.instructions[i];
while (insn)
{
struct ExtInstruction *next = insn->next;
free (insn->name);
free (insn);
insn = next;
}
}
for (i = 0; i < NUM_EXT_CORE; i++)
{
if (arc_extension_map.coreRegisters[i].name)
free (arc_extension_map.coreRegisters[i].name);
}
for (i = 0; i < NUM_EXT_COND; i++)
{
if (arc_extension_map.condCodes[i])
free (arc_extension_map.condCodes[i]);
}
memset (&arc_extension_map, 0, sizeof (arc_extension_map));
}
#endif
static const char* ExtReadWrite_image(enum ExtReadWrite val)
{
switch (val)
{
case REG_INVALID : return "INVALID";
case REG_READ : return "RO";
case REG_WRITE : return "WO";
case REG_READWRITE: return "R/W";
default : return "???";
}
}
const char *
arcExtMap_instName (int opcode, int insn, int *flags)
{
unsigned char minor;
struct ExtInstruction *temp;
if (*flags != E_ARC_MACH_A4) 
{
if (0x08 <= opcode && opcode <= 0x0b)
{
unsigned char b, c, i;
b = (insn & 0x0700) >> 8;
c = (insn & 0x00e0) >> 5;
i = (insn & 0x001f);
if (i) {
minor = i;
} else {
minor = (c == 0x07) ? b : c;
}
}
else
{
unsigned char I, A, B;
I = (insn & 0x003f0000) >> 16;
A = (insn & 0x0000003f);
B = ((insn & 0x07000000) >> 24) | ((insn & 0x00007000) >> 9);
if (I != 0x2f)
{
#if !defined(UNMANGLED)
switch (P)
{
case 3:
if (M)
{
minor = I;
break;
}
case 0:
case 2:
minor = (I >> 1) | ((I & 0x1) << 5);
break;
case 1:
minor = (I >> 1) | (I & 0x1) | ((I & 0x2) << 4);
}
#else
minor = I;
#endif
}
else
{
if (A != 0x3f) {
minor = A;
} else {
minor = B;
}
}
}
} else { 
minor = insn;
}
temp = arc_extension_map.instructions[INST_HASH (opcode, minor)];
while (temp)
{
if ((temp->major == opcode) && (temp->minor == minor))
{
*flags = temp->flags;
return temp->name;
}
temp = temp->next;
}
return NULL;
}
const char *
arcExtMap_coreRegName (int regnum)
{
if (regnum < FIRST_EXTENSION_CORE_REGISTER || regnum > LAST_EXTENSION_CORE_REGISTER) {
return NULL;
}
return arc_extension_map.coreRegisters[regnum - FIRST_EXTENSION_CORE_REGISTER].name;
}
enum ExtReadWrite
arcExtMap_coreReadWrite (int regnum)
{
if (regnum < FIRST_EXTENSION_CORE_REGISTER || regnum > LAST_EXTENSION_CORE_REGISTER) {
return REG_INVALID;
}
return arc_extension_map.coreRegisters[regnum - FIRST_EXTENSION_CORE_REGISTER].rw;
}
const char *
arcExtMap_condCodeName (int code)
{
if (code < FIRST_EXTENSION_CONDITION_CODE || code > LAST_EXTENSION_CONDITION_CODE) {
return NULL;
}
return arc_extension_map.condCodes[code - FIRST_EXTENSION_CONDITION_CODE];
}
const char *
arcExtMap_auxRegName (long address)
{
struct ExtAuxRegister *r;
for (r = arc_extension_map.auxRegisters; r; r = r->next)
{
if (r->address == address) {
return (const char *)r->name;
}
}
return NULL;
}
void
build_ARC_extmap (void *text_bfd)
{
#if 0
asection *sect;
destroy_map();
for (sect = text_bfd->sections; sect != NULL; sect = sect->next)
if (!strncmp (sect->name,
".gnu.linkonce.arcextmap.",
sizeof (".gnu.linkonce.arcextmap.") - 1)
|| !strcmp (sect->name,".arcextmap"))
{
bfd_size_type count = bfd_get_section_size (sect);
unsigned char* buffer = xmalloc (count);
if (buffer)
{
if (bfd_get_section_contents (text_bfd, sect, buffer, 0, count))
create_map(buffer, count);
free (buffer);
}
}
#endif
}
void dump_ARC_extmap (void)
{
struct ExtAuxRegister* r;
int i;
r = arc_extension_map.auxRegisters;
while (r)
{
printf("AUX : %s %ld\n", r->name, r->address);
r = r->next;
}
for (i = 0; i < INST_HASH_SIZE; i++)
{
struct ExtInstruction *insn;
for (insn = arc_extension_map.instructions[i]; insn != NULL; insn = insn->next) {
printf ("INST: %d %d %x %s\n", insn->major, insn->minor, insn->flags, insn->name);
}
}
for (i = 0; i < NUM_EXT_CORE; i++)
{
struct ExtCoreRegister reg = arc_extension_map.coreRegisters[i];
if (reg.name) {
printf ("CORE: %s %d %s\n", reg.name, reg.number, ExtReadWrite_image (reg.rw));
}
}
for (i = 0; i < NUM_EXT_COND; i++) {
if (arc_extension_map.condCodes[i]) {
printf ("COND: %s\n", arc_extension_map.condCodes[i]);
}
}
}
