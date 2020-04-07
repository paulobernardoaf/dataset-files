





















#define xmalloc malloc

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sysdep.h"
#include "opcode/sparc.h"
#include "disas-asm.h"
#if !defined(_MSC_VER)
#include "libiberty.h"
#else
#include <stdlib.h>
#define XNEWVEC(T, N) ((T *) malloc (sizeof (T) * (N)))
#define XCNEWVEC(T, N) ((T *) calloc ((N), sizeof (T)))
#define XNEW(T) ((T *) malloc (sizeof (T)))
#define xmalloc malloc
#endif
#include "opintl.h"


#define MASK_V9 ((1 << SPARC_OPCODE_ARCH_V9) | (1 << SPARC_OPCODE_ARCH_V9A) | (1 << SPARC_OPCODE_ARCH_V9B))



#define V9_ONLY_P(insn) (! ((insn)->architecture & ~MASK_V9))

#define V9_P(insn) (((insn)->architecture & MASK_V9) != 0)


static const sparc_opcode **sorted_opcodes;




#define HASH_SIZE 256



static int opcode_bits[4] = { 0x01c00000, 0x0, 0x01f80000, 0x01f80000 };
#define HASH_INSN(INSN) ((((INSN) >> 24) & 0xc0) | (((INSN) & opcode_bits[((INSN) >> 30) & 3]) >> 19))

typedef struct sparc_opcode_hash
{
struct sparc_opcode_hash *next;
const sparc_opcode *opcode;
} sparc_opcode_hash;

static sparc_opcode_hash *opcode_hash_table[HASH_SIZE];


#define SEX(value, bits) ((((int)(value)) << ((8 * sizeof (int)) - (bits))) >> ((8 * sizeof (int)) - (bits)) )



static char *reg_names[] =
{ "g0", "g1", "g2", "g3", "g4", "g5", "g6", "g7",
"o0", "o1", "o2", "o3", "o4", "o5", "sp", "o7",
"l0", "l1", "l2", "l3", "l4", "l5", "l6", "l7",
"i0", "i1", "i2", "i3", "i4", "i5", "fp", "i7",
"f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7",
"f8", "f9", "f10", "f11", "f12", "f13", "f14", "f15",
"f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23",
"f24", "f25", "f26", "f27", "f28", "f29", "f30", "f31",
"f32", "f33", "f34", "f35", "f36", "f37", "f38", "f39",
"f40", "f41", "f42", "f43", "f44", "f45", "f46", "f47",
"f48", "f49", "f50", "f51", "f52", "f53", "f54", "f55",
"f56", "f57", "f58", "f59", "f60", "f61", "f62", "f63",

"y", "psr", "wim", "tbr", "pc", "npc", "fpsr", "cpsr"
};

#define freg_names (&reg_names[4 * 8])



static char *v9_priv_reg_names[] =
{
"tpc", "tnpc", "tstate", "tt", "tick", "tba", "pstate", "tl",
"pil", "cwp", "cansave", "canrestore", "cleanwin", "otherwin",
"wstate", "fq", "gl"

};



static char *v9_hpriv_reg_names[] =
{
"hpstate", "htstate", "resv2", "hintp", "resv4", "htba", "hver",
"resv7", "resv8", "resv9", "resv10", "resv11", "resv12", "resv13", 
"resv14", "resv15", "resv16", "resv17", "resv18", "resv19", "resv20",
"resv21", "resv22", "resv23", "resv24", "resv25", "resv26", "resv27",
"resv28", "resv29", "resv30", "hstick_cmpr"
};



static char *v9a_asr_reg_names[] =
{
"pcr", "pic", "dcr", "gsr", "set_softint", "clear_softint",
"softint", "tick_cmpr", "stick", "stick_cmpr"
};




#define X_RD(i) (((i) >> 25) & 0x1f)
#define X_RS1(i) (((i) >> 14) & 0x1f)
#define X_LDST_I(i) (((i) >> 13) & 1)
#define X_ASI(i) (((i) >> 5) & 0xff)
#define X_RS2(i) (((i) >> 0) & 0x1f)
#define X_IMM(i,n) (((i) >> 0) & ((1 << (n)) - 1))
#define X_SIMM(i,n) SEX (X_IMM ((i), (n)), (n))
#define X_DISP22(i) (((i) >> 0) & 0x3fffff)
#define X_IMM22(i) X_DISP22 (i)
#define X_DISP30(i) (((i) >> 0) & 0x3fffffff)


#define X_DISP16(i) (((((i) >> 20) & 3) << 14) | (((i) >> 0) & 0x3fff))
#define X_DISP19(i) (((i) >> 0) & 0x7ffff)
#define X_MEMBAR(i) ((i) & 0x7f)





























































static int
is_delayed_branch (unsigned long insn)
{
sparc_opcode_hash *op;

for (op = opcode_hash_table[HASH_INSN (insn)]; op; op = op->next)
{
const sparc_opcode *opcode = op->opcode;

if ((opcode->match & insn) == opcode->match && (opcode->lose & insn) == 0) {
return opcode->flags & F_DELAYED;
}
}
return 0;
}





static unsigned int current_arch_mask;



static int
compute_arch_mask (unsigned long mach)
{
switch (mach)
{
case 0 :
case bfd_mach_sparc :
return SPARC_OPCODE_ARCH_MASK (SPARC_OPCODE_ARCH_V8);
case bfd_mach_sparc_sparclet :
return SPARC_OPCODE_ARCH_MASK (SPARC_OPCODE_ARCH_SPARCLET);
case bfd_mach_sparc_sparclite :
case bfd_mach_sparc_sparclite_le :



return (SPARC_OPCODE_ARCH_MASK (SPARC_OPCODE_ARCH_SPARCLITE)
| SPARC_OPCODE_ARCH_MASK (SPARC_OPCODE_ARCH_V8));
case bfd_mach_sparc_v8plus :
case bfd_mach_sparc_v9 :
return SPARC_OPCODE_ARCH_MASK (SPARC_OPCODE_ARCH_V9);
case bfd_mach_sparc_v8plusa :
case bfd_mach_sparc_v9a :
return SPARC_OPCODE_ARCH_MASK (SPARC_OPCODE_ARCH_V9A);
case bfd_mach_sparc_v8plusb :
case bfd_mach_sparc_v9b :
return SPARC_OPCODE_ARCH_MASK (SPARC_OPCODE_ARCH_V9B);
}
abort ();
}



static int
compare_opcodes (const void * a, const void * b)
{
sparc_opcode *op0 = * (sparc_opcode **) a;
sparc_opcode *op1 = * (sparc_opcode **) b;
unsigned long int match0 = op0->match, match1 = op1->match;
unsigned long int lose0 = op0->lose, lose1 = op1->lose;
register unsigned int i;






if (op0->architecture & current_arch_mask)
{
if (!(op1->architecture & current_arch_mask)) {
return -1;
}
}
else
{
if (op1->architecture & current_arch_mask) {
return 1;
} else if (op0->architecture != op1->architecture) {
return op0->architecture - op1->architecture;
}
}



if (match0 & lose0)
{
fprintf
(stderr,

_("Internal error: bad sparc-opcode.h: \"%s\", %#.8lx, %#.8lx\n"),
op0->name, match0, lose0);
op0->lose &= ~op0->match;
lose0 = op0->lose;
}

if (match1 & lose1)
{
fprintf
(stderr,

_("Internal error: bad sparc-opcode.h: \"%s\", %#.8lx, %#.8lx\n"),
op1->name, match1, lose1);
op1->lose &= ~op1->match;
lose1 = op1->lose;
}



for (i = 0; i < 32; i++)
{
unsigned long int x = 1 << i;
int x0 = (match0 & x) != 0;
int x1 = (match1 & x) != 0;

if (x0 != x1) {
return x1 - x0;
}
}

for (i = 0; i < 32; i++)
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
if (op0->flags & F_ALIAS) { 
return i;
} else {
fprintf (stderr,

_ ("Internal error: bad sparc-opcode.h: \"%s\" == \"%s\"\n"),
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
char *p0 = (char *) strchr (op0->args, '+');
char *p1 = (char *) strchr (op1->args, '+');

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


{
int i0 = strncmp (op0->args, "i,1", 3) == 0;
int i1 = strncmp (op1->args, "i,1", 3) == 0;

if (i0 ^ i1) {
return i0 - i1;
}
}







return 0;
}




static void
build_hash_table (const sparc_opcode **opcode_table,
sparc_opcode_hash **hash_table,
int num_opcodes)
{
int i;
int hash_count[HASH_SIZE];
static sparc_opcode_hash *hash_buf = NULL;




memset (hash_table, 0, HASH_SIZE * sizeof (hash_table[0]));
memset (hash_count, 0, HASH_SIZE * sizeof (hash_count[0]));
if (hash_buf != NULL) {
free (hash_buf);
}
hash_buf = calloc (sizeof (* hash_buf), num_opcodes);
if (!hash_buf) {
return;
}
for (i = num_opcodes - 1; i >= 0; --i)
{
int hash = HASH_INSN (opcode_table[i]->match);
sparc_opcode_hash *h = &hash_buf[i];

h->next = hash_table[hash];
h->opcode = opcode_table[i];
hash_table[hash] = h;
++hash_count[hash];
}

#if 0 
{
int min_count = num_opcodes, max_count = 0;
int total;

for (i = 0; i < HASH_SIZE; i++)
{
if (hash_count[i] < min_count)
min_count = hash_count[i];
if (hash_count[i] > max_count)
max_count = hash_count[i];
total += hash_count[i];
}

printf ("Opcode hash table stats: min %d, max %d, ave %f\n",
min_count, max_count, (double) total / HASH_SIZE);
}
#endif
}









int
print_insn_sparc (bfd_vma memaddr, disassemble_info *info)
{
FILE *stream = info->stream;
bfd_byte buffer[4];
unsigned long insn;
sparc_opcode_hash *op;

static int opcodes_initialized = 0;

static unsigned long current_mach = 0;
bfd_vma (*getword) (const void *);

if (!opcodes_initialized
|| info->mach != current_mach)
{
int i;

current_arch_mask = compute_arch_mask (info->mach);

if (!opcodes_initialized) {
sorted_opcodes =
xmalloc (sparc_num_opcodes * sizeof (sparc_opcode *));
}

for (i = 0; i < sparc_num_opcodes; i++) {
sorted_opcodes[i] = &sparc_opcodes[i];
}
qsort ((char *) sorted_opcodes, sparc_num_opcodes,
sizeof (sorted_opcodes[0]), compare_opcodes);

build_hash_table (sorted_opcodes, opcode_hash_table, sparc_num_opcodes);
current_mach = info->mach;
opcodes_initialized = 1;
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



if (info->endian == BFD_ENDIAN_BIG || info->mach == bfd_mach_sparc_sparclite) {
getword = bfd_getb32;
} else {
getword = bfd_getl32;
}

insn = getword (buffer);

info->insn_info_valid = 1; 
info->insn_type = dis_nonbranch; 
info->branch_delay_insns = 0; 
info->target = 0; 

for (op = opcode_hash_table[HASH_INSN (insn)]; op; op = op->next)
{
const sparc_opcode *opcode = op->opcode;


if (!(opcode->architecture & current_arch_mask)) {
continue;
}

if ((opcode->match & insn) == opcode->match
&& (opcode->lose & insn) == 0)
{


int imm_added_to_rs1 = 0;
int imm_ored_to_rs1 = 0;



int found_plus = 0;


int is_annulled = 0;



if (opcode->match == 0x80102000) { 
imm_ored_to_rs1 = 1;
}
if (opcode->match == 0x80002000) { 
imm_added_to_rs1 = 1;
}

if (X_RS1 (insn) != X_RD (insn) && strchr (opcode->args, 'r') != 0) {

continue;
}
if (X_RS2 (insn) != X_RD (insn) && strchr (opcode->args, 'O') != 0) {

continue;
}

(*info->fprintf_func) (stream, "%s", opcode->name);

{
const char *s;
int brackets = 0;

for (s = opcode->args; *s != '\0'; s++)
{
while (*s == ',')
{
(*info->fprintf_func) (stream, ",");
++s;
switch (*s)
{
case 'a':
(*info->fprintf_func) (stream, "a");
is_annulled = 1;
++s;
continue;
case 'N':
(*info->fprintf_func) (stream, "pn");
++s;
continue;

case 'T':
(*info->fprintf_func) (stream, "pt");
++s;
continue;

default:
break;
}
}

if (*s != '[' && !brackets) {
(*info->fprintf_func) (stream, " ");
} else if (*s == ']') {
(*info->fprintf_func) (stream, "%c", *s);
brackets = 0;
continue;
} else if (*s == '[') {
(*info->fprintf_func) (stream, " %c", *s);
brackets = 1;
continue;
}

switch (*s)
{
case '+':
found_plus = 1;


default:
(*info->fprintf_func) (stream, "%c", *s);
break;

case '#':
(*info->fprintf_func) (stream, "0");
break;

#define reg(n) (*info->fprintf_func) (stream, "%%%s", reg_names[n])
case '1':
case 'r':
reg (X_RS1 (insn));
break;

case '2':
case 'O':
reg (X_RS2 (insn));
break;

case 'd':
reg (X_RD (insn));
break;
#undef reg

#define freg(n) (*info->fprintf_func) (stream, "%%%s", freg_names[n])
#define fregx(n) (*info->fprintf_func) (stream, "%%%s", freg_names[((n) & ~1) | (((n) & 1) << 5)])
case 'e':
freg (X_RS1 (insn));
break;
case 'v': 
case 'V': 
fregx (X_RS1 (insn));
break;

case 'f':
freg (X_RS2 (insn));
break;
case 'B': 
case 'R': 
fregx (X_RS2 (insn));
break;

case 'g':
freg (X_RD (insn));
break;
case 'H': 
case 'J': 
fregx (X_RD (insn));
break;
#undef freg
#undef fregx

#define creg(n) (*info->fprintf_func) (stream, "%%c%u", (unsigned int) (n))
case 'b':
creg (X_RS1 (insn));
break;

case 'c':
creg (X_RS2 (insn));
break;

case 'D':
creg (X_RD (insn));
break;
#undef creg

case 'h':
(*info->fprintf_func) (stream, "%%hi(%#x)",
((unsigned) 0xFFFFFFFF
& ((int) X_IMM22 (insn) << 10)));
break;

case 'i': 
case 'I': 
case 'j': 
{
int imm;

if (*s == 'i') {
imm = X_SIMM (insn, 13);
} else if (*s == 'I') {
imm = X_SIMM (insn, 11);
} else {
imm = X_SIMM (insn, 10);
}








if (found_plus) {
imm_added_to_rs1 = 1;
}

if (imm <= 9) {
(*info->fprintf_func) (stream, "%d", imm);
} else {
(*info->fprintf_func) (stream, "%#x", imm);
}
}
break;

case 'X': 
case 'Y': 
{
int imm = X_IMM (insn, *s == 'X' ? 5 : 6);

if (imm <= 9) {
(info->fprintf_func) (stream, "%d", imm);
} else {
(info->fprintf_func) (stream, "%#x", (unsigned)imm);
}
}
break;

case '3':
(info->fprintf_func) (stream, "%ld", X_IMM (insn, 3));
break;

case 'K':
{
int mask = X_MEMBAR (insn);
int bit = 0x40, printed_one = 0;
const char *name;

if (mask == 0) {
(info->fprintf_func) (stream, "0");
} else {
while (bit) {
if (mask & bit) {
if (printed_one) {
(info->fprintf_func) (stream, "|");
}
name = sparc_decode_membar (bit);
(info->fprintf_func) (stream, "%s", name);
printed_one = 1;
}
bit >>= 1;
}
}
break;
}

case 'k':
info->target = memaddr + SEX (X_DISP16 (insn), 16) * 4;
(*info->print_address_func) (info->target, info);
break;

case 'G':
info->target = memaddr + SEX (X_DISP19 (insn), 19) * 4;
(*info->print_address_func) (info->target, info);
break;

case '6':
case '7':
case '8':
case '9':
(*info->fprintf_func) (stream, "%%fcc%c", *s - '6' + '0');
break;

case 'z':
(*info->fprintf_func) (stream, "%%icc");
break;

case 'Z':
(*info->fprintf_func) (stream, "%%xcc");
break;

case 'E':
(*info->fprintf_func) (stream, "%%ccr");
break;

case 's':
(*info->fprintf_func) (stream, "%%fprs");
break;

case 'o':
(*info->fprintf_func) (stream, "%%asi");
break;

case 'W':
(*info->fprintf_func) (stream, "%%tick");
break;

case 'P':
(*info->fprintf_func) (stream, "%%pc");
break;

case '?':
if (X_RS1 (insn) == 31) {
(*info->fprintf_func) (stream, "%%ver");
} else if ((unsigned)X_RS1 (insn) < 17) {
(*info->fprintf_func) (stream, "%%%s",
v9_priv_reg_names[X_RS1 (insn)]);
} else {
(*info->fprintf_func) (stream, "%%reserved");
}
break;

case '!':
if ((unsigned)X_RD (insn) < 17) {
(*info->fprintf_func) (stream, "%%%s",
v9_priv_reg_names[X_RD (insn)]);
} else {
(*info->fprintf_func) (stream, "%%reserved");
}
break;

case '$':
if ((unsigned)X_RS1 (insn) < 32) {
(*info->fprintf_func) (stream, "%%%s",
v9_hpriv_reg_names[X_RS1 (insn)]);
} else {
(*info->fprintf_func) (stream, "%%reserved");
}
break;

case '%':
if ((unsigned)X_RD (insn) < 32) {
(*info->fprintf_func) (stream, "%%%s",
v9_hpriv_reg_names[X_RD (insn)]);
} else {
(*info->fprintf_func) (stream, "%%reserved");
}
break;

case '/':
if (X_RS1 (insn) < 16 || X_RS1 (insn) > 25) {
(*info->fprintf_func) (stream, "%%reserved");
} else {
(*info->fprintf_func) (stream, "%%%s",
v9a_asr_reg_names[X_RS1 (insn) - 16]);
}
break;

case '_':
if (X_RD (insn) < 16 || X_RD (insn) > 25) {
(*info->fprintf_func) (stream, "%%reserved");
} else {
(*info->fprintf_func) (stream, "%%%s",
v9a_asr_reg_names[X_RD (insn) - 16]);
}
break;

case '*':
{
const char *name = sparc_decode_prefetch (X_RD (insn));

if (name) {
(*info->fprintf_func) (stream, "%s", name);
} else {
(*info->fprintf_func) (stream, "%ld", X_RD (insn));
}
break;
}

case 'M':
(*info->fprintf_func) (stream, "%%asr%ld", X_RS1 (insn));
break;

case 'm':
(*info->fprintf_func) (stream, "%%asr%ld", X_RD (insn));
break;

case 'L':
info->target = memaddr + SEX (X_DISP30 (insn), 30) * 4;
(*info->print_address_func) (info->target, info);
break;

case 'n':
(*info->fprintf_func)
(stream, "%#x", SEX (X_DISP22 (insn), 22));
break;

case 'l':
info->target = memaddr + SEX (X_DISP22 (insn), 22) * 4;
(*info->print_address_func) (info->target, info);
break;

case 'A':
{
const char *name = sparc_decode_asi (X_ASI (insn));

if (name) {
(*info->fprintf_func) (stream, "%s", name);
} else {
(*info->fprintf_func) (stream, "(%ld)", X_ASI (insn));
}
break;
}

case 'C':
(*info->fprintf_func) (stream, "%%csr");
break;

case 'F':
(*info->fprintf_func) (stream, "%%fsr");
break;

case 'p':
(*info->fprintf_func) (stream, "%%psr");
break;

case 'q':
(*info->fprintf_func) (stream, "%%fq");
break;

case 'Q':
(*info->fprintf_func) (stream, "%%cq");
break;

case 't':
(*info->fprintf_func) (stream, "%%tbr");
break;

case 'w':
(*info->fprintf_func) (stream, "%%wim");
break;

case 'x':
(*info->fprintf_func) (stream, "%ld",
((X_LDST_I (insn) << 8)
+ X_ASI (insn)));
break;

case 'y':
(*info->fprintf_func) (stream, "%%y");
break;

case 'u':
case 'U':
{
int val = *s == 'U' ? X_RS1 (insn) : X_RD (insn);
const char *name = sparc_decode_sparclet_cpreg (val);

if (name) {
(*info->fprintf_func) (stream, "%s", name);
} else {
(*info->fprintf_func) (stream, "%%cpreg(%d)", val);
}
break;
}
}
}
}







if (imm_ored_to_rs1 || imm_added_to_rs1)
{
unsigned long prev_insn;
int errcode;

if (memaddr >= 4) {
errcode =
(*info->read_memory_func) (memaddr - 4, buffer, sizeof (buffer), info);
} else {
errcode = 1;
}

prev_insn = getword (buffer);

if (errcode == 0)
{








if (is_delayed_branch (prev_insn))
{
if (memaddr >= 8) {
errcode = (*info->read_memory_func) (memaddr - 8, buffer, sizeof (buffer), info);
} else {
errcode = 1;
}

prev_insn = getword (buffer);
}
}



if (errcode == 0)
{

if ((prev_insn & 0xc1c00000) == 0x01000000
&& X_RD (prev_insn) == X_RS1 (insn))
{
(*info->fprintf_func) (stream, "\t! ");
info->target =
((unsigned) 0xFFFFFFFF
& ((int) X_IMM22 (prev_insn) << 10));
if (imm_added_to_rs1) {
info->target += X_SIMM (insn, 13);
} else {
info->target |= X_SIMM (insn, 13);
}
(*info->print_address_func) (info->target, info);
info->insn_type = dis_dref;
info->data_size = 4; 
}
}
}

if (opcode->flags & (F_UNBR|F_CONDBR|F_JSR))
{

(void) is_annulled;
if (opcode->flags & F_UNBR) {
info->insn_type = dis_branch;
}
if (opcode->flags & F_CONDBR) {
info->insn_type = dis_condbranch;
}
if (opcode->flags & F_JSR) {
info->insn_type = dis_jsr;
}
if (opcode->flags & F_DELAYED) {
info->branch_delay_insns = 1;
}
}

return sizeof (buffer);
}
}

info->insn_type = dis_noninsn; 
(*info->fprintf_func) (stream, _("unknown"));
return sizeof (buffer);
}

#if defined(_MSC_VER)
#undef xmalloc
#endif
