#define END 0 
#define BOL 1 
#define EOL 2 
#define BRANCH 3 
#define BACK 4 
#define EXACTLY 5 
#define NOTHING 6 
#define STAR 7 
#define PLUS 8 
#define MATCH 9 
#define NOMATCH 10 
#define BEHIND 11 
#define NOBEHIND 12 
#define SUBPAT 13 
#define BRACE_SIMPLE 14 
#define BOW 15 
#define EOW 16 
#define BRACE_LIMITS 17 
#define NEWL 18 
#define BHPOS 19 
#define ADD_NL 30
#define FIRST_NL ANY + ADD_NL
#define ANY 20 
#define ANYOF 21 
#define ANYBUT 22 
#define IDENT 23 
#define SIDENT 24 
#define KWORD 25 
#define SKWORD 26 
#define FNAME 27 
#define SFNAME 28 
#define PRINT 29 
#define SPRINT 30 
#define WHITE 31 
#define NWHITE 32 
#define DIGIT 33 
#define NDIGIT 34 
#define HEX 35 
#define NHEX 36 
#define OCTAL 37 
#define NOCTAL 38 
#define WORD 39 
#define NWORD 40 
#define HEAD 41 
#define NHEAD 42 
#define ALPHA 43 
#define NALPHA 44 
#define LOWER 45 
#define NLOWER 46 
#define UPPER 47 
#define NUPPER 48 
#define LAST_NL NUPPER + ADD_NL
#define WITH_NL(op) ((op) >= FIRST_NL && (op) <= LAST_NL)
#define MOPEN 80 
#define MCLOSE 90 
#define BACKREF 100 
#if defined(FEAT_SYN_HL)
#define ZOPEN 110 
#define ZCLOSE 120 
#define ZREF 130 
#endif
#define BRACE_COMPLEX 140 
#define NOPEN 150 
#define NCLOSE 151 
#define MULTIBYTECODE 200 
#define RE_BOF 201 
#define RE_EOF 202 
#define CURSOR 203 
#define RE_LNUM 204 
#define RE_COL 205 
#define RE_VCOL 206 
#define RE_MARK 207 
#define RE_VISUAL 208 
#define RE_COMPOSING 209 
#define HASWIDTH 0x1 
#define SIMPLE 0x2 
#define SPSTART 0x4 
#define HASNL 0x8 
#define HASLOOKBH 0x10 
#define WORST 0 
static int num_complex_braces; 
static char_u *regcode; 
static long regsize; 
static int reg_toolong; 
static char_u had_endbrace[NSUBEXP]; 
static long brace_min[10]; 
static long brace_max[10]; 
static int brace_count[10]; 
static int one_exactly = FALSE; 
static char_u *classchars = (char_u *)".iIkKfFpPsSdDxXoOwWhHaAlLuU";
static int classcodes[] = {
ANY, IDENT, SIDENT, KWORD, SKWORD,
FNAME, SFNAME, PRINT, SPRINT,
WHITE, NWHITE, DIGIT, NDIGIT,
HEX, NHEX, OCTAL, NOCTAL,
WORD, NWORD, HEAD, NHEAD,
ALPHA, NALPHA, LOWER, NLOWER,
UPPER, NUPPER
};
#define JUST_CALC_SIZE ((char_u *) -1)
typedef enum regstate_E
{
RS_NOPEN = 0 
, RS_MOPEN 
, RS_MCLOSE 
#if defined(FEAT_SYN_HL)
, RS_ZOPEN 
, RS_ZCLOSE 
#endif
, RS_BRANCH 
, RS_BRCPLX_MORE 
, RS_BRCPLX_LONG 
, RS_BRCPLX_SHORT 
, RS_NOMATCH 
, RS_BEHIND1 
, RS_BEHIND2 
, RS_STAR_LONG 
, RS_STAR_SHORT 
} regstate_T;
typedef struct
{
union
{
char_u *ptr; 
lpos_T pos; 
} rs_u;
int rs_len;
} regsave_T;
typedef struct
{
union
{
char_u *ptr;
lpos_T pos;
} se_u;
} save_se_T;
typedef struct regbehind_S
{
regsave_T save_after;
regsave_T save_behind;
int save_need_clear_subexpr;
save_se_T save_start[NSUBEXP];
save_se_T save_end[NSUBEXP];
} regbehind_T;
typedef struct regitem_S
{
regstate_T rs_state; 
short rs_no; 
char_u *rs_scan; 
union
{
save_se_T sesave;
regsave_T regsave;
} rs_un; 
} regitem_T;
typedef struct regstar_S
{
int nextb; 
int nextb_ic; 
long count;
long minval;
long maxval;
} regstar_T;
typedef struct backpos_S
{
char_u *bp_scan; 
regsave_T bp_pos; 
} backpos_T;
static garray_T regstack = {0, 0, 0, 0, NULL};
static garray_T backpos = {0, 0, 0, 0, NULL};
static regsave_T behind_pos;
#define REGSTACK_INITIAL 2048
#define BACKPOS_INITIAL 64
#define OP(p) ((int)*(p))
#define NEXT(p) (((*((p) + 1) & 0377) << 8) + (*((p) + 2) & 0377))
#define OPERAND(p) ((p) + 3)
#define OPERAND_MIN(p) (((long)(p)[3] << 24) + ((long)(p)[4] << 16) + ((long)(p)[5] << 8) + (long)(p)[6])
#define OPERAND_MAX(p) OPERAND_MIN((p) + 4)
#define OPERAND_CMP(p) (p)[7]
static char_u *reg(int paren, int *flagp);
#if defined(BT_REGEXP_DUMP)
static void regdump(char_u *, bt_regprog_T *);
#endif
static int re_num_cmp(long_u val, char_u *scan);
#if defined(DEBUG)
static char_u *regprop(char_u *);
static int regnarrate = 0;
#endif
static void
regcomp_start(
char_u *expr,
int re_flags) 
{
initchr(expr);
if (re_flags & RE_MAGIC)
reg_magic = MAGIC_ON;
else
reg_magic = MAGIC_OFF;
reg_string = (re_flags & RE_STRING);
reg_strict = (re_flags & RE_STRICT);
get_cpo_flags();
num_complex_braces = 0;
regnpar = 1;
vim_memset(had_endbrace, 0, sizeof(had_endbrace));
#if defined(FEAT_SYN_HL)
regnzpar = 1;
re_has_z = 0;
#endif
regsize = 0L;
reg_toolong = FALSE;
regflags = 0;
#if defined(FEAT_SYN_HL) || defined(PROTO)
had_eol = FALSE;
#endif
}
static int
use_multibytecode(int c)
{
return has_mbyte && (*mb_char2len)(c) > 1
&& (re_multi_type(peekchr()) != NOT_MULTI
|| (enc_utf8 && utf_iscomposing(c)));
}
static void
regc(int b)
{
if (regcode == JUST_CALC_SIZE)
regsize++;
else
*regcode++ = b;
}
static void
regmbc(int c)
{
if (!has_mbyte && c > 0xff)
return;
if (regcode == JUST_CALC_SIZE)
regsize += (*mb_char2len)(c);
else
regcode += (*mb_char2bytes)(c, regcode);
}
#define REGMBC(x) regmbc(x);
#define CASEMBC(x) case x:
static void
reg_equi_class(int c)
{
if (enc_utf8 || STRCMP(p_enc, "latin1") == 0
|| STRCMP(p_enc, "iso-8859-15") == 0)
{
#if defined(EBCDIC)
int i;
for (i = 0; i < 16; i++)
{
if (vim_strchr(EQUIVAL_CLASS_C[i], c) != NULL)
{
char *p = EQUIVAL_CLASS_C[i];
while (*p != 0)
regmbc(*p++);
return;
}
}
#else
switch (c)
{
case 'A': case 0xc0: case 0xc1: case 0xc2:
case 0xc3: case 0xc4: case 0xc5:
CASEMBC(0x100) CASEMBC(0x102) CASEMBC(0x104) CASEMBC(0x1cd)
CASEMBC(0x1de) CASEMBC(0x1e0) CASEMBC(0x1ea2)
regmbc('A'); regmbc(0xc0); regmbc(0xc1);
regmbc(0xc2); regmbc(0xc3); regmbc(0xc4);
regmbc(0xc5);
REGMBC(0x100) REGMBC(0x102) REGMBC(0x104)
REGMBC(0x1cd) REGMBC(0x1de) REGMBC(0x1e0)
REGMBC(0x1ea2)
return;
case 'B': CASEMBC(0x1e02) CASEMBC(0x1e06)
regmbc('B'); REGMBC(0x1e02) REGMBC(0x1e06)
return;
case 'C': case 0xc7:
CASEMBC(0x106) CASEMBC(0x108) CASEMBC(0x10a) CASEMBC(0x10c)
regmbc('C'); regmbc(0xc7);
REGMBC(0x106) REGMBC(0x108) REGMBC(0x10a)
REGMBC(0x10c)
return;
case 'D': CASEMBC(0x10e) CASEMBC(0x110) CASEMBC(0x1e0a)
CASEMBC(0x1e0e) CASEMBC(0x1e10)
regmbc('D'); REGMBC(0x10e) REGMBC(0x110)
REGMBC(0x1e0a) REGMBC(0x1e0e) REGMBC(0x1e10)
return;
case 'E': case 0xc8: case 0xc9: case 0xca: case 0xcb:
CASEMBC(0x112) CASEMBC(0x114) CASEMBC(0x116) CASEMBC(0x118)
CASEMBC(0x11a) CASEMBC(0x1eba) CASEMBC(0x1ebc)
regmbc('E'); regmbc(0xc8); regmbc(0xc9);
regmbc(0xca); regmbc(0xcb);
REGMBC(0x112) REGMBC(0x114) REGMBC(0x116)
REGMBC(0x118) REGMBC(0x11a) REGMBC(0x1eba)
REGMBC(0x1ebc)
return;
case 'F': CASEMBC(0x1e1e)
regmbc('F'); REGMBC(0x1e1e)
return;
case 'G': CASEMBC(0x11c) CASEMBC(0x11e) CASEMBC(0x120)
CASEMBC(0x122) CASEMBC(0x1e4) CASEMBC(0x1e6) CASEMBC(0x1f4)
CASEMBC(0x1e20)
regmbc('G'); REGMBC(0x11c) REGMBC(0x11e)
REGMBC(0x120) REGMBC(0x122) REGMBC(0x1e4)
REGMBC(0x1e6) REGMBC(0x1f4) REGMBC(0x1e20)
return;
case 'H': CASEMBC(0x124) CASEMBC(0x126) CASEMBC(0x1e22)
CASEMBC(0x1e26) CASEMBC(0x1e28)
regmbc('H'); REGMBC(0x124) REGMBC(0x126)
REGMBC(0x1e22) REGMBC(0x1e26) REGMBC(0x1e28)
return;
case 'I': case 0xcc: case 0xcd: case 0xce: case 0xcf:
CASEMBC(0x128) CASEMBC(0x12a) CASEMBC(0x12c) CASEMBC(0x12e)
CASEMBC(0x130) CASEMBC(0x1cf) CASEMBC(0x1ec8)
regmbc('I'); regmbc(0xcc); regmbc(0xcd);
regmbc(0xce); regmbc(0xcf);
REGMBC(0x128) REGMBC(0x12a) REGMBC(0x12c)
REGMBC(0x12e) REGMBC(0x130) REGMBC(0x1cf)
REGMBC(0x1ec8)
return;
case 'J': CASEMBC(0x134)
regmbc('J'); REGMBC(0x134)
return;
case 'K': CASEMBC(0x136) CASEMBC(0x1e8) CASEMBC(0x1e30)
CASEMBC(0x1e34)
regmbc('K'); REGMBC(0x136) REGMBC(0x1e8)
REGMBC(0x1e30) REGMBC(0x1e34)
return;
case 'L': CASEMBC(0x139) CASEMBC(0x13b) CASEMBC(0x13d)
CASEMBC(0x13f) CASEMBC(0x141) CASEMBC(0x1e3a)
regmbc('L'); REGMBC(0x139) REGMBC(0x13b)
REGMBC(0x13d) REGMBC(0x13f) REGMBC(0x141)
REGMBC(0x1e3a)
return;
case 'M': CASEMBC(0x1e3e) CASEMBC(0x1e40)
regmbc('M'); REGMBC(0x1e3e) REGMBC(0x1e40)
return;
case 'N': case 0xd1:
CASEMBC(0x143) CASEMBC(0x145) CASEMBC(0x147) CASEMBC(0x1e44)
CASEMBC(0x1e48)
regmbc('N'); regmbc(0xd1);
REGMBC(0x143) REGMBC(0x145) REGMBC(0x147)
REGMBC(0x1e44) REGMBC(0x1e48)
return;
case 'O': case 0xd2: case 0xd3: case 0xd4: case 0xd5:
case 0xd6: case 0xd8:
CASEMBC(0x14c) CASEMBC(0x14e) CASEMBC(0x150) CASEMBC(0x1a0)
CASEMBC(0x1d1) CASEMBC(0x1ea) CASEMBC(0x1ec) CASEMBC(0x1ece)
regmbc('O'); regmbc(0xd2); regmbc(0xd3);
regmbc(0xd4); regmbc(0xd5); regmbc(0xd6);
regmbc(0xd8);
REGMBC(0x14c) REGMBC(0x14e) REGMBC(0x150)
REGMBC(0x1a0) REGMBC(0x1d1) REGMBC(0x1ea)
REGMBC(0x1ec) REGMBC(0x1ece)
return;
case 'P': case 0x1e54: case 0x1e56:
regmbc('P'); REGMBC(0x1e54) REGMBC(0x1e56)
return;
case 'R': CASEMBC(0x154) CASEMBC(0x156) CASEMBC(0x158)
CASEMBC(0x1e58) CASEMBC(0x1e5e)
regmbc('R'); REGMBC(0x154) REGMBC(0x156) REGMBC(0x158)
REGMBC(0x1e58) REGMBC(0x1e5e)
return;
case 'S': CASEMBC(0x15a) CASEMBC(0x15c) CASEMBC(0x15e)
CASEMBC(0x160) CASEMBC(0x1e60)
regmbc('S'); REGMBC(0x15a) REGMBC(0x15c)
REGMBC(0x15e) REGMBC(0x160) REGMBC(0x1e60)
return;
case 'T': CASEMBC(0x162) CASEMBC(0x164) CASEMBC(0x166)
CASEMBC(0x1e6a) CASEMBC(0x1e6e)
regmbc('T'); REGMBC(0x162) REGMBC(0x164)
REGMBC(0x166) REGMBC(0x1e6a) REGMBC(0x1e6e)
return;
case 'U': case 0xd9: case 0xda: case 0xdb: case 0xdc:
CASEMBC(0x168) CASEMBC(0x16a) CASEMBC(0x16c) CASEMBC(0x16e)
CASEMBC(0x170) CASEMBC(0x172) CASEMBC(0x1af) CASEMBC(0x1d3)
CASEMBC(0x1ee6)
regmbc('U'); regmbc(0xd9); regmbc(0xda);
regmbc(0xdb); regmbc(0xdc);
REGMBC(0x168) REGMBC(0x16a) REGMBC(0x16c)
REGMBC(0x16e) REGMBC(0x170) REGMBC(0x172)
REGMBC(0x1af) REGMBC(0x1d3) REGMBC(0x1ee6)
return;
case 'V': CASEMBC(0x1e7c)
regmbc('V'); REGMBC(0x1e7c)
return;
case 'W': CASEMBC(0x174) CASEMBC(0x1e80) CASEMBC(0x1e82)
CASEMBC(0x1e84) CASEMBC(0x1e86)
regmbc('W'); REGMBC(0x174) REGMBC(0x1e80)
REGMBC(0x1e82) REGMBC(0x1e84) REGMBC(0x1e86)
return;
case 'X': CASEMBC(0x1e8a) CASEMBC(0x1e8c)
regmbc('X'); REGMBC(0x1e8a) REGMBC(0x1e8c)
return;
case 'Y': case 0xdd:
CASEMBC(0x176) CASEMBC(0x178) CASEMBC(0x1e8e) CASEMBC(0x1ef2)
CASEMBC(0x1ef6) CASEMBC(0x1ef8)
regmbc('Y'); regmbc(0xdd);
REGMBC(0x176) REGMBC(0x178) REGMBC(0x1e8e)
REGMBC(0x1ef2) REGMBC(0x1ef6) REGMBC(0x1ef8)
return;
case 'Z': CASEMBC(0x179) CASEMBC(0x17b) CASEMBC(0x17d)
CASEMBC(0x1b5) CASEMBC(0x1e90) CASEMBC(0x1e94)
regmbc('Z'); REGMBC(0x179) REGMBC(0x17b)
REGMBC(0x17d) REGMBC(0x1b5) REGMBC(0x1e90)
REGMBC(0x1e94)
return;
case 'a': case 0xe0: case 0xe1: case 0xe2:
case 0xe3: case 0xe4: case 0xe5:
CASEMBC(0x101) CASEMBC(0x103) CASEMBC(0x105) CASEMBC(0x1ce)
CASEMBC(0x1df) CASEMBC(0x1e1) CASEMBC(0x1ea3)
regmbc('a'); regmbc(0xe0); regmbc(0xe1);
regmbc(0xe2); regmbc(0xe3); regmbc(0xe4);
regmbc(0xe5);
REGMBC(0x101) REGMBC(0x103) REGMBC(0x105)
REGMBC(0x1ce) REGMBC(0x1df) REGMBC(0x1e1)
REGMBC(0x1ea3)
return;
case 'b': CASEMBC(0x1e03) CASEMBC(0x1e07)
regmbc('b'); REGMBC(0x1e03) REGMBC(0x1e07)
return;
case 'c': case 0xe7:
CASEMBC(0x107) CASEMBC(0x109) CASEMBC(0x10b) CASEMBC(0x10d)
regmbc('c'); regmbc(0xe7);
REGMBC(0x107) REGMBC(0x109) REGMBC(0x10b)
REGMBC(0x10d)
return;
case 'd': CASEMBC(0x10f) CASEMBC(0x111) CASEMBC(0x1e0b)
CASEMBC(0x1e0f) CASEMBC(0x1e11)
regmbc('d'); REGMBC(0x10f) REGMBC(0x111)
REGMBC(0x1e0b) REGMBC(0x1e0f) REGMBC(0x1e11)
return;
case 'e': case 0xe8: case 0xe9: case 0xea: case 0xeb:
CASEMBC(0x113) CASEMBC(0x115) CASEMBC(0x117) CASEMBC(0x119)
CASEMBC(0x11b) CASEMBC(0x1ebb) CASEMBC(0x1ebd)
regmbc('e'); regmbc(0xe8); regmbc(0xe9);
regmbc(0xea); regmbc(0xeb);
REGMBC(0x113) REGMBC(0x115) REGMBC(0x117)
REGMBC(0x119) REGMBC(0x11b) REGMBC(0x1ebb)
REGMBC(0x1ebd)
return;
case 'f': CASEMBC(0x1e1f)
regmbc('f'); REGMBC(0x1e1f)
return;
case 'g': CASEMBC(0x11d) CASEMBC(0x11f) CASEMBC(0x121)
CASEMBC(0x123) CASEMBC(0x1e5) CASEMBC(0x1e7) CASEMBC(0x1f5)
CASEMBC(0x1e21)
regmbc('g'); REGMBC(0x11d) REGMBC(0x11f)
REGMBC(0x121) REGMBC(0x123) REGMBC(0x1e5)
REGMBC(0x1e7) REGMBC(0x1f5) REGMBC(0x1e21)
return;
case 'h': CASEMBC(0x125) CASEMBC(0x127) CASEMBC(0x1e23)
CASEMBC(0x1e27) CASEMBC(0x1e29) CASEMBC(0x1e96)
regmbc('h'); REGMBC(0x125) REGMBC(0x127)
REGMBC(0x1e23) REGMBC(0x1e27) REGMBC(0x1e29)
REGMBC(0x1e96)
return;
case 'i': case 0xec: case 0xed: case 0xee: case 0xef:
CASEMBC(0x129) CASEMBC(0x12b) CASEMBC(0x12d) CASEMBC(0x12f)
CASEMBC(0x1d0) CASEMBC(0x1ec9)
regmbc('i'); regmbc(0xec); regmbc(0xed);
regmbc(0xee); regmbc(0xef);
REGMBC(0x129) REGMBC(0x12b) REGMBC(0x12d)
REGMBC(0x12f) REGMBC(0x1d0) REGMBC(0x1ec9)
return;
case 'j': CASEMBC(0x135) CASEMBC(0x1f0)
regmbc('j'); REGMBC(0x135) REGMBC(0x1f0)
return;
case 'k': CASEMBC(0x137) CASEMBC(0x1e9) CASEMBC(0x1e31)
CASEMBC(0x1e35)
regmbc('k'); REGMBC(0x137) REGMBC(0x1e9)
REGMBC(0x1e31) REGMBC(0x1e35)
return;
case 'l': CASEMBC(0x13a) CASEMBC(0x13c) CASEMBC(0x13e)
CASEMBC(0x140) CASEMBC(0x142) CASEMBC(0x1e3b)
regmbc('l'); REGMBC(0x13a) REGMBC(0x13c)
REGMBC(0x13e) REGMBC(0x140) REGMBC(0x142)
REGMBC(0x1e3b)
return;
case 'm': CASEMBC(0x1e3f) CASEMBC(0x1e41)
regmbc('m'); REGMBC(0x1e3f) REGMBC(0x1e41)
return;
case 'n': case 0xf1:
CASEMBC(0x144) CASEMBC(0x146) CASEMBC(0x148) CASEMBC(0x149)
CASEMBC(0x1e45) CASEMBC(0x1e49)
regmbc('n'); regmbc(0xf1);
REGMBC(0x144) REGMBC(0x146) REGMBC(0x148)
REGMBC(0x149) REGMBC(0x1e45) REGMBC(0x1e49)
return;
case 'o': case 0xf2: case 0xf3: case 0xf4: case 0xf5:
case 0xf6: case 0xf8:
CASEMBC(0x14d) CASEMBC(0x14f) CASEMBC(0x151) CASEMBC(0x1a1)
CASEMBC(0x1d2) CASEMBC(0x1eb) CASEMBC(0x1ed) CASEMBC(0x1ecf)
regmbc('o'); regmbc(0xf2); regmbc(0xf3);
regmbc(0xf4); regmbc(0xf5); regmbc(0xf6);
regmbc(0xf8);
REGMBC(0x14d) REGMBC(0x14f) REGMBC(0x151)
REGMBC(0x1a1) REGMBC(0x1d2) REGMBC(0x1eb)
REGMBC(0x1ed) REGMBC(0x1ecf)
return;
case 'p': CASEMBC(0x1e55) CASEMBC(0x1e57)
regmbc('p'); REGMBC(0x1e55) REGMBC(0x1e57)
return;
case 'r': CASEMBC(0x155) CASEMBC(0x157) CASEMBC(0x159)
CASEMBC(0x1e59) CASEMBC(0x1e5f)
regmbc('r'); REGMBC(0x155) REGMBC(0x157) REGMBC(0x159)
REGMBC(0x1e59) REGMBC(0x1e5f)
return;
case 's': CASEMBC(0x15b) CASEMBC(0x15d) CASEMBC(0x15f)
CASEMBC(0x161) CASEMBC(0x1e61)
regmbc('s'); REGMBC(0x15b) REGMBC(0x15d)
REGMBC(0x15f) REGMBC(0x161) REGMBC(0x1e61)
return;
case 't': CASEMBC(0x163) CASEMBC(0x165) CASEMBC(0x167)
CASEMBC(0x1e6b) CASEMBC(0x1e6f) CASEMBC(0x1e97)
regmbc('t'); REGMBC(0x163) REGMBC(0x165) REGMBC(0x167)
REGMBC(0x1e6b) REGMBC(0x1e6f) REGMBC(0x1e97)
return;
case 'u': case 0xf9: case 0xfa: case 0xfb: case 0xfc:
CASEMBC(0x169) CASEMBC(0x16b) CASEMBC(0x16d) CASEMBC(0x16f)
CASEMBC(0x171) CASEMBC(0x173) CASEMBC(0x1b0) CASEMBC(0x1d4)
CASEMBC(0x1ee7)
regmbc('u'); regmbc(0xf9); regmbc(0xfa);
regmbc(0xfb); regmbc(0xfc);
REGMBC(0x169) REGMBC(0x16b) REGMBC(0x16d)
REGMBC(0x16f) REGMBC(0x171) REGMBC(0x173)
REGMBC(0x1b0) REGMBC(0x1d4) REGMBC(0x1ee7)
return;
case 'v': CASEMBC(0x1e7d)
regmbc('v'); REGMBC(0x1e7d)
return;
case 'w': CASEMBC(0x175) CASEMBC(0x1e81) CASEMBC(0x1e83)
CASEMBC(0x1e85) CASEMBC(0x1e87) CASEMBC(0x1e98)
regmbc('w'); REGMBC(0x175) REGMBC(0x1e81)
REGMBC(0x1e83) REGMBC(0x1e85) REGMBC(0x1e87)
REGMBC(0x1e98)
return;
case 'x': CASEMBC(0x1e8b) CASEMBC(0x1e8d)
regmbc('x'); REGMBC(0x1e8b) REGMBC(0x1e8d)
return;
case 'y': case 0xfd: case 0xff:
CASEMBC(0x177) CASEMBC(0x1e8f) CASEMBC(0x1e99)
CASEMBC(0x1ef3) CASEMBC(0x1ef7) CASEMBC(0x1ef9)
regmbc('y'); regmbc(0xfd); regmbc(0xff);
REGMBC(0x177) REGMBC(0x1e8f) REGMBC(0x1e99)
REGMBC(0x1ef3) REGMBC(0x1ef7) REGMBC(0x1ef9)
return;
case 'z': CASEMBC(0x17a) CASEMBC(0x17c) CASEMBC(0x17e)
CASEMBC(0x1b6) CASEMBC(0x1e91) CASEMBC(0x1e95)
regmbc('z'); REGMBC(0x17a) REGMBC(0x17c)
REGMBC(0x17e) REGMBC(0x1b6) REGMBC(0x1e91)
REGMBC(0x1e95)
return;
}
#endif
}
regmbc(c);
}
static char_u *
regnode(int op)
{
char_u *ret;
ret = regcode;
if (ret == JUST_CALC_SIZE)
regsize += 3;
else
{
*regcode++ = op;
*regcode++ = NUL; 
*regcode++ = NUL;
}
return ret;
}
static char_u *
re_put_long(char_u *p, long_u val)
{
*p++ = (char_u) ((val >> 24) & 0377);
*p++ = (char_u) ((val >> 16) & 0377);
*p++ = (char_u) ((val >> 8) & 0377);
*p++ = (char_u) (val & 0377);
return p;
}
static char_u *
regnext(char_u *p)
{
int offset;
if (p == JUST_CALC_SIZE || reg_toolong)
return NULL;
offset = NEXT(p);
if (offset == 0)
return NULL;
if (OP(p) == BACK)
return p - offset;
else
return p + offset;
}
static void
regtail(char_u *p, char_u *val)
{
char_u *scan;
char_u *temp;
int offset;
if (p == JUST_CALC_SIZE)
return;
scan = p;
for (;;)
{
temp = regnext(scan);
if (temp == NULL)
break;
scan = temp;
}
if (OP(scan) == BACK)
offset = (int)(scan - val);
else
offset = (int)(val - scan);
if (offset > 0xffff)
reg_toolong = TRUE;
else
{
*(scan + 1) = (char_u) (((unsigned)offset >> 8) & 0377);
*(scan + 2) = (char_u) (offset & 0377);
}
}
static void
regoptail(char_u *p, char_u *val)
{
if (p == NULL || p == JUST_CALC_SIZE
|| (OP(p) != BRANCH
&& (OP(p) < BRACE_COMPLEX || OP(p) > BRACE_COMPLEX + 9)))
return;
regtail(OPERAND(p), val);
}
static void
reginsert(int op, char_u *opnd)
{
char_u *src;
char_u *dst;
char_u *place;
if (regcode == JUST_CALC_SIZE)
{
regsize += 3;
return;
}
src = regcode;
regcode += 3;
dst = regcode;
while (src > opnd)
*--dst = *--src;
place = opnd; 
*place++ = op;
*place++ = NUL;
*place = NUL;
}
static void
reginsert_nr(int op, long val, char_u *opnd)
{
char_u *src;
char_u *dst;
char_u *place;
if (regcode == JUST_CALC_SIZE)
{
regsize += 7;
return;
}
src = regcode;
regcode += 7;
dst = regcode;
while (src > opnd)
*--dst = *--src;
place = opnd; 
*place++ = op;
*place++ = NUL;
*place++ = NUL;
re_put_long(place, (long_u)val);
}
static void
reginsert_limits(
int op,
long minval,
long maxval,
char_u *opnd)
{
char_u *src;
char_u *dst;
char_u *place;
if (regcode == JUST_CALC_SIZE)
{
regsize += 11;
return;
}
src = regcode;
regcode += 11;
dst = regcode;
while (src > opnd)
*--dst = *--src;
place = opnd; 
*place++ = op;
*place++ = NUL;
*place++ = NUL;
place = re_put_long(place, (long_u)minval);
place = re_put_long(place, (long_u)maxval);
regtail(opnd, place);
}
static int
seen_endbrace(int refnum)
{
if (!had_endbrace[refnum])
{
char_u *p;
for (p = regparse; *p != NUL; ++p)
if (p[0] == '@' && p[1] == '<' && (p[2] == '!' || p[2] == '='))
break;
if (*p == NUL)
{
emsg(_("E65: Illegal back reference"));
rc_did_emsg = TRUE;
return FALSE;
}
}
return TRUE;
}
static char_u *
regatom(int *flagp)
{
char_u *ret;
int flags;
int c;
char_u *p;
int extra = 0;
int save_prev_at_start = prev_at_start;
*flagp = WORST; 
c = getchr();
switch (c)
{
case Magic('^'):
ret = regnode(BOL);
break;
case Magic('$'):
ret = regnode(EOL);
#if defined(FEAT_SYN_HL) || defined(PROTO)
had_eol = TRUE;
#endif
break;
case Magic('<'):
ret = regnode(BOW);
break;
case Magic('>'):
ret = regnode(EOW);
break;
case Magic('_'):
c = no_Magic(getchr());
if (c == '^') 
{
ret = regnode(BOL);
break;
}
if (c == '$') 
{
ret = regnode(EOL);
#if defined(FEAT_SYN_HL) || defined(PROTO)
had_eol = TRUE;
#endif
break;
}
extra = ADD_NL;
*flagp |= HASNL;
if (c == '[')
goto collection;
case Magic('.'):
case Magic('i'):
case Magic('I'):
case Magic('k'):
case Magic('K'):
case Magic('f'):
case Magic('F'):
case Magic('p'):
case Magic('P'):
case Magic('s'):
case Magic('S'):
case Magic('d'):
case Magic('D'):
case Magic('x'):
case Magic('X'):
case Magic('o'):
case Magic('O'):
case Magic('w'):
case Magic('W'):
case Magic('h'):
case Magic('H'):
case Magic('a'):
case Magic('A'):
case Magic('l'):
case Magic('L'):
case Magic('u'):
case Magic('U'):
p = vim_strchr(classchars, no_Magic(c));
if (p == NULL)
EMSG_RET_NULL(_("E63: invalid use of \\_"));
if (enc_utf8 && c == Magic('.') && utf_iscomposing(peekchr()))
{
c = getchr();
goto do_multibyte;
}
ret = regnode(classcodes[p - classchars] + extra);
*flagp |= HASWIDTH | SIMPLE;
break;
case Magic('n'):
if (reg_string)
{
ret = regnode(EXACTLY);
regc(NL);
regc(NUL);
*flagp |= HASWIDTH | SIMPLE;
}
else
{
ret = regnode(NEWL);
*flagp |= HASWIDTH | HASNL;
}
break;
case Magic('('):
if (one_exactly)
EMSG_ONE_RET_NULL;
ret = reg(REG_PAREN, &flags);
if (ret == NULL)
return NULL;
*flagp |= flags & (HASWIDTH | SPSTART | HASNL | HASLOOKBH);
break;
case NUL:
case Magic('|'):
case Magic('&'):
case Magic(')'):
if (one_exactly)
EMSG_ONE_RET_NULL;
IEMSG_RET_NULL(_(e_internal)); 
case Magic('='):
case Magic('?'):
case Magic('+'):
case Magic('@'):
case Magic('{'):
case Magic('*'):
c = no_Magic(c);
EMSG3_RET_NULL(_("E64: %s%c follows nothing"),
(c == '*' ? reg_magic >= MAGIC_ON : reg_magic == MAGIC_ALL), c);
case Magic('~'): 
if (reg_prev_sub != NULL)
{
char_u *lp;
ret = regnode(EXACTLY);
lp = reg_prev_sub;
while (*lp != NUL)
regc(*lp++);
regc(NUL);
if (*reg_prev_sub != NUL)
{
*flagp |= HASWIDTH;
if ((lp - reg_prev_sub) == 1)
*flagp |= SIMPLE;
}
}
else
EMSG_RET_NULL(_(e_nopresub));
break;
case Magic('1'):
case Magic('2'):
case Magic('3'):
case Magic('4'):
case Magic('5'):
case Magic('6'):
case Magic('7'):
case Magic('8'):
case Magic('9'):
{
int refnum;
refnum = c - Magic('0');
if (!seen_endbrace(refnum))
return NULL;
ret = regnode(BACKREF + refnum);
}
break;
case Magic('z'):
{
c = no_Magic(getchr());
switch (c)
{
#if defined(FEAT_SYN_HL)
case '(': if ((reg_do_extmatch & REX_SET) == 0)
EMSG_RET_NULL(_(e_z_not_allowed));
if (one_exactly)
EMSG_ONE_RET_NULL;
ret = reg(REG_ZPAREN, &flags);
if (ret == NULL)
return NULL;
*flagp |= flags & (HASWIDTH|SPSTART|HASNL|HASLOOKBH);
re_has_z = REX_SET;
break;
case '1':
case '2':
case '3':
case '4':
case '5':
case '6':
case '7':
case '8':
case '9': if ((reg_do_extmatch & REX_USE) == 0)
EMSG_RET_NULL(_(e_z1_not_allowed));
ret = regnode(ZREF + c - '0');
re_has_z = REX_USE;
break;
#endif
case 's': ret = regnode(MOPEN + 0);
if (re_mult_next("\\zs") == FAIL)
return NULL;
break;
case 'e': ret = regnode(MCLOSE + 0);
if (re_mult_next("\\ze") == FAIL)
return NULL;
break;
default: EMSG_RET_NULL(_("E68: Invalid character after \\z"));
}
}
break;
case Magic('%'):
{
c = no_Magic(getchr());
switch (c)
{
case '(':
if (one_exactly)
EMSG_ONE_RET_NULL;
ret = reg(REG_NPAREN, &flags);
if (ret == NULL)
return NULL;
*flagp |= flags & (HASWIDTH | SPSTART | HASNL | HASLOOKBH);
break;
case '^':
ret = regnode(RE_BOF);
break;
case '$':
ret = regnode(RE_EOF);
break;
case '#':
ret = regnode(CURSOR);
break;
case 'V':
ret = regnode(RE_VISUAL);
break;
case 'C':
ret = regnode(RE_COMPOSING);
break;
case '[':
if (one_exactly) 
EMSG_ONE_RET_NULL;
{
char_u *lastbranch;
char_u *lastnode = NULL;
char_u *br;
ret = NULL;
while ((c = getchr()) != ']')
{
if (c == NUL)
EMSG2_RET_NULL(_(e_missing_sb),
reg_magic == MAGIC_ALL);
br = regnode(BRANCH);
if (ret == NULL)
ret = br;
else
{
regtail(lastnode, br);
if (reg_toolong)
return NULL;
}
ungetchr();
one_exactly = TRUE;
lastnode = regatom(flagp);
one_exactly = FALSE;
if (lastnode == NULL)
return NULL;
}
if (ret == NULL)
EMSG2_RET_NULL(_(e_empty_sb),
reg_magic == MAGIC_ALL);
lastbranch = regnode(BRANCH);
br = regnode(NOTHING);
if (ret != JUST_CALC_SIZE)
{
regtail(lastnode, br);
regtail(lastbranch, br);
for (br = ret; br != lastnode; )
{
if (OP(br) == BRANCH)
{
regtail(br, lastbranch);
if (reg_toolong)
return NULL;
br = OPERAND(br);
}
else
br = regnext(br);
}
}
*flagp &= ~(HASWIDTH | SIMPLE);
break;
}
case 'd': 
case 'o': 
case 'x': 
case 'u': 
case 'U': 
{
long i;
switch (c)
{
case 'd': i = getdecchrs(); break;
case 'o': i = getoctchrs(); break;
case 'x': i = gethexchrs(2); break;
case 'u': i = gethexchrs(4); break;
case 'U': i = gethexchrs(8); break;
default: i = -1; break;
}
if (i < 0 || i > INT_MAX)
EMSG2_RET_NULL(
_("E678: Invalid character after %s%%[dxouU]"),
reg_magic == MAGIC_ALL);
if (use_multibytecode(i))
ret = regnode(MULTIBYTECODE);
else
ret = regnode(EXACTLY);
if (i == 0)
regc(0x0a);
else
regmbc(i);
regc(NUL);
*flagp |= HASWIDTH;
break;
}
default:
if (VIM_ISDIGIT(c) || c == '<' || c == '>'
|| c == '\'')
{
long_u n = 0;
int cmp;
cmp = c;
if (cmp == '<' || cmp == '>')
c = getchr();
while (VIM_ISDIGIT(c))
{
n = n * 10 + (c - '0');
c = getchr();
}
if (c == '\'' && n == 0)
{
c = getchr();
ret = regnode(RE_MARK);
if (ret == JUST_CALC_SIZE)
regsize += 2;
else
{
*regcode++ = c;
*regcode++ = cmp;
}
break;
}
else if (c == 'l' || c == 'c' || c == 'v')
{
if (c == 'l')
{
ret = regnode(RE_LNUM);
if (save_prev_at_start)
at_start = TRUE;
}
else if (c == 'c')
ret = regnode(RE_COL);
else
ret = regnode(RE_VCOL);
if (ret == JUST_CALC_SIZE)
regsize += 5;
else
{
regcode = re_put_long(regcode, n);
*regcode++ = cmp;
}
break;
}
}
EMSG2_RET_NULL(_("E71: Invalid character after %s%%"),
reg_magic == MAGIC_ALL);
}
}
break;
case Magic('['):
collection:
{
char_u *lp;
lp = skip_anyof(regparse);
if (*lp == ']') 
{
int startc = -1; 
int endc;
if (*regparse == '^') 
{
ret = regnode(ANYBUT + extra);
regparse++;
}
else
ret = regnode(ANYOF + extra);
if (*regparse == ']' || *regparse == '-')
{
startc = *regparse;
regc(*regparse++);
}
while (*regparse != NUL && *regparse != ']')
{
if (*regparse == '-')
{
++regparse;
if (*regparse == ']' || *regparse == NUL
|| startc == -1
|| (regparse[0] == '\\' && regparse[1] == 'n'))
{
regc('-');
startc = '-'; 
}
else
{
endc = 0;
if (*regparse == '[')
endc = get_coll_element(&regparse);
if (endc == 0)
{
if (has_mbyte)
endc = mb_ptr2char_adv(&regparse);
else
endc = *regparse++;
}
if (endc == '\\' && !reg_cpo_lit && !reg_cpo_bsl)
endc = coll_get_char();
if (startc > endc)
EMSG_RET_NULL(_(e_reverse_range));
if (has_mbyte && ((*mb_char2len)(startc) > 1
|| (*mb_char2len)(endc) > 1))
{
if (endc > startc + 256)
EMSG_RET_NULL(_(e_large_class));
while (++startc <= endc)
regmbc(startc);
}
else
{
#if defined(EBCDIC)
int alpha_only = FALSE;
if (isalpha(startc) && isalpha(endc))
alpha_only = TRUE;
#endif
while (++startc <= endc)
#if defined(EBCDIC)
if (!alpha_only || isalpha(startc))
#endif
regc(startc);
}
startc = -1;
}
}
else if (*regparse == '\\'
&& !reg_cpo_bsl
&& (vim_strchr(REGEXP_INRANGE, regparse[1]) != NULL
|| (!reg_cpo_lit
&& vim_strchr(REGEXP_ABBR,
regparse[1]) != NULL)))
{
regparse++;
if (*regparse == 'n')
{
if (ret != JUST_CALC_SIZE)
{
if (*ret == ANYOF)
{
*ret = ANYOF + ADD_NL;
*flagp |= HASNL;
}
}
regparse++;
startc = -1;
}
else if (*regparse == 'd'
|| *regparse == 'o'
|| *regparse == 'x'
|| *regparse == 'u'
|| *regparse == 'U')
{
startc = coll_get_char();
if (startc == 0)
regc(0x0a);
else
regmbc(startc);
}
else
{
startc = backslash_trans(*regparse++);
regc(startc);
}
}
else if (*regparse == '[')
{
int c_class;
int cu;
c_class = get_char_class(&regparse);
startc = -1;
switch (c_class)
{
case CLASS_NONE:
c_class = get_equi_class(&regparse);
if (c_class != 0)
{
reg_equi_class(c_class);
}
else if ((c_class =
get_coll_element(&regparse)) != 0)
{
regmbc(c_class);
}
else
{
startc = *regparse++;
regc(startc);
}
break;
case CLASS_ALNUM:
for (cu = 1; cu < 128; cu++)
if (isalnum(cu))
regmbc(cu);
break;
case CLASS_ALPHA:
for (cu = 1; cu < 128; cu++)
if (isalpha(cu))
regmbc(cu);
break;
case CLASS_BLANK:
regc(' ');
regc('\t');
break;
case CLASS_CNTRL:
for (cu = 1; cu <= 127; cu++)
if (iscntrl(cu))
regmbc(cu);
break;
case CLASS_DIGIT:
for (cu = 1; cu <= 127; cu++)
if (VIM_ISDIGIT(cu))
regmbc(cu);
break;
case CLASS_GRAPH:
for (cu = 1; cu <= 127; cu++)
if (isgraph(cu))
regmbc(cu);
break;
case CLASS_LOWER:
for (cu = 1; cu <= 255; cu++)
if (MB_ISLOWER(cu) && cu != 170
&& cu != 186)
regmbc(cu);
break;
case CLASS_PRINT:
for (cu = 1; cu <= 255; cu++)
if (vim_isprintc(cu))
regmbc(cu);
break;
case CLASS_PUNCT:
for (cu = 1; cu < 128; cu++)
if (ispunct(cu))
regmbc(cu);
break;
case CLASS_SPACE:
for (cu = 9; cu <= 13; cu++)
regc(cu);
regc(' ');
break;
case CLASS_UPPER:
for (cu = 1; cu <= 255; cu++)
if (MB_ISUPPER(cu))
regmbc(cu);
break;
case CLASS_XDIGIT:
for (cu = 1; cu <= 255; cu++)
if (vim_isxdigit(cu))
regmbc(cu);
break;
case CLASS_TAB:
regc('\t');
break;
case CLASS_RETURN:
regc('\r');
break;
case CLASS_BACKSPACE:
regc('\b');
break;
case CLASS_ESCAPE:
regc('\033');
break;
case CLASS_IDENT:
for (cu = 1; cu <= 255; cu++)
if (vim_isIDc(cu))
regmbc(cu);
break;
case CLASS_KEYWORD:
for (cu = 1; cu <= 255; cu++)
if (reg_iswordc(cu))
regmbc(cu);
break;
case CLASS_FNAME:
for (cu = 1; cu <= 255; cu++)
if (vim_isfilec(cu))
regmbc(cu);
break;
}
}
else
{
if (has_mbyte)
{
int len;
startc = mb_ptr2char(regparse);
len = (*mb_ptr2len)(regparse);
if (enc_utf8 && utf_char2len(startc) != len)
startc = -1; 
while (--len >= 0)
regc(*regparse++);
}
else
{
startc = *regparse++;
regc(startc);
}
}
}
regc(NUL);
prevchr_len = 1; 
if (*regparse != ']')
EMSG_RET_NULL(_(e_toomsbra)); 
skipchr(); 
*flagp |= HASWIDTH | SIMPLE;
break;
}
else if (reg_strict)
EMSG2_RET_NULL(_(e_missingbracket), reg_magic > MAGIC_OFF);
}
default:
{
int len;
if (use_multibytecode(c))
{
do_multibyte:
ret = regnode(MULTIBYTECODE);
regmbc(c);
*flagp |= HASWIDTH | SIMPLE;
break;
}
ret = regnode(EXACTLY);
for (len = 0; c != NUL && (len == 0
|| (re_multi_type(peekchr()) == NOT_MULTI
&& !one_exactly
&& !is_Magic(c))); ++len)
{
c = no_Magic(c);
if (has_mbyte)
{
regmbc(c);
if (enc_utf8)
{
int l;
for (;;)
{
l = utf_ptr2len(regparse);
if (!UTF_COMPOSINGLIKE(regparse, regparse + l))
break;
regmbc(utf_ptr2char(regparse));
skipchr();
}
}
}
else
regc(c);
c = getchr();
}
ungetchr();
regc(NUL);
*flagp |= HASWIDTH;
if (len == 1)
*flagp |= SIMPLE;
}
break;
}
return ret;
}
static char_u *
regpiece(int *flagp)
{
char_u *ret;
int op;
char_u *next;
int flags;
long minval;
long maxval;
ret = regatom(&flags);
if (ret == NULL)
return NULL;
op = peekchr();
if (re_multi_type(op) == NOT_MULTI)
{
*flagp = flags;
return ret;
}
*flagp = (WORST | SPSTART | (flags & (HASNL | HASLOOKBH)));
skipchr();
switch (op)
{
case Magic('*'):
if (flags & SIMPLE)
reginsert(STAR, ret);
else
{
reginsert(BRANCH, ret); 
regoptail(ret, regnode(BACK)); 
regoptail(ret, ret); 
regtail(ret, regnode(BRANCH)); 
regtail(ret, regnode(NOTHING)); 
}
break;
case Magic('+'):
if (flags & SIMPLE)
reginsert(PLUS, ret);
else
{
next = regnode(BRANCH); 
regtail(ret, next);
regtail(regnode(BACK), ret); 
regtail(next, regnode(BRANCH)); 
regtail(ret, regnode(NOTHING)); 
}
*flagp = (WORST | HASWIDTH | (flags & (HASNL | HASLOOKBH)));
break;
case Magic('@'):
{
int lop = END;
long nr;
nr = getdecchrs();
switch (no_Magic(getchr()))
{
case '=': lop = MATCH; break; 
case '!': lop = NOMATCH; break; 
case '>': lop = SUBPAT; break; 
case '<': switch (no_Magic(getchr()))
{
case '=': lop = BEHIND; break; 
case '!': lop = NOBEHIND; break; 
}
}
if (lop == END)
EMSG2_RET_NULL(_("E59: invalid character after %s@"),
reg_magic == MAGIC_ALL);
if (lop == BEHIND || lop == NOBEHIND)
{
regtail(ret, regnode(BHPOS));
*flagp |= HASLOOKBH;
}
regtail(ret, regnode(END)); 
if (lop == BEHIND || lop == NOBEHIND)
{
if (nr < 0)
nr = 0; 
reginsert_nr(lop, nr, ret);
}
else
reginsert(lop, ret);
break;
}
case Magic('?'):
case Magic('='):
reginsert(BRANCH, ret); 
regtail(ret, regnode(BRANCH)); 
next = regnode(NOTHING); 
regtail(ret, next);
regoptail(ret, next);
break;
case Magic('{'):
if (!read_limits(&minval, &maxval))
return NULL;
if (flags & SIMPLE)
{
reginsert(BRACE_SIMPLE, ret);
reginsert_limits(BRACE_LIMITS, minval, maxval, ret);
}
else
{
if (num_complex_braces >= 10)
EMSG2_RET_NULL(_("E60: Too many complex %s{...}s"),
reg_magic == MAGIC_ALL);
reginsert(BRACE_COMPLEX + num_complex_braces, ret);
regoptail(ret, regnode(BACK));
regoptail(ret, ret);
reginsert_limits(BRACE_LIMITS, minval, maxval, ret);
++num_complex_braces;
}
if (minval > 0 && maxval > 0)
*flagp = (HASWIDTH | (flags & (HASNL | HASLOOKBH)));
break;
}
if (re_multi_type(peekchr()) != NOT_MULTI)
{
if (peekchr() == Magic('*'))
EMSG2_RET_NULL(_("E61: Nested %s*"), reg_magic >= MAGIC_ON);
EMSG3_RET_NULL(_("E62: Nested %s%c"), reg_magic == MAGIC_ALL,
no_Magic(peekchr()));
}
return ret;
}
static char_u *
regconcat(int *flagp)
{
char_u *first = NULL;
char_u *chain = NULL;
char_u *latest;
int flags;
int cont = TRUE;
*flagp = WORST; 
while (cont)
{
switch (peekchr())
{
case NUL:
case Magic('|'):
case Magic('&'):
case Magic(')'):
cont = FALSE;
break;
case Magic('Z'):
regflags |= RF_ICOMBINE;
skipchr_keepstart();
break;
case Magic('c'):
regflags |= RF_ICASE;
skipchr_keepstart();
break;
case Magic('C'):
regflags |= RF_NOICASE;
skipchr_keepstart();
break;
case Magic('v'):
reg_magic = MAGIC_ALL;
skipchr_keepstart();
curchr = -1;
break;
case Magic('m'):
reg_magic = MAGIC_ON;
skipchr_keepstart();
curchr = -1;
break;
case Magic('M'):
reg_magic = MAGIC_OFF;
skipchr_keepstart();
curchr = -1;
break;
case Magic('V'):
reg_magic = MAGIC_NONE;
skipchr_keepstart();
curchr = -1;
break;
default:
latest = regpiece(&flags);
if (latest == NULL || reg_toolong)
return NULL;
*flagp |= flags & (HASWIDTH | HASNL | HASLOOKBH);
if (chain == NULL) 
*flagp |= flags & SPSTART;
else
regtail(chain, latest);
chain = latest;
if (first == NULL)
first = latest;
break;
}
}
if (first == NULL) 
first = regnode(NOTHING);
return first;
}
static char_u *
regbranch(int *flagp)
{
char_u *ret;
char_u *chain = NULL;
char_u *latest;
int flags;
*flagp = WORST | HASNL; 
ret = regnode(BRANCH);
for (;;)
{
latest = regconcat(&flags);
if (latest == NULL)
return NULL;
*flagp |= flags & (HASWIDTH | SPSTART | HASLOOKBH);
*flagp &= ~HASNL | (flags & HASNL);
if (chain != NULL)
regtail(chain, latest);
if (peekchr() != Magic('&'))
break;
skipchr();
regtail(latest, regnode(END)); 
if (reg_toolong)
break;
reginsert(MATCH, latest);
chain = latest;
}
return ret;
}
static char_u *
reg(
int paren, 
int *flagp)
{
char_u *ret;
char_u *br;
char_u *ender;
int parno = 0;
int flags;
*flagp = HASWIDTH; 
#if defined(FEAT_SYN_HL)
if (paren == REG_ZPAREN)
{
if (regnzpar >= NSUBEXP)
EMSG_RET_NULL(_("E50: Too many \\z("));
parno = regnzpar;
regnzpar++;
ret = regnode(ZOPEN + parno);
}
else
#endif
if (paren == REG_PAREN)
{
if (regnpar >= NSUBEXP)
EMSG2_RET_NULL(_("E51: Too many %s("), reg_magic == MAGIC_ALL);
parno = regnpar;
++regnpar;
ret = regnode(MOPEN + parno);
}
else if (paren == REG_NPAREN)
{
ret = regnode(NOPEN);
}
else
ret = NULL;
br = regbranch(&flags);
if (br == NULL)
return NULL;
if (ret != NULL)
regtail(ret, br); 
else
ret = br;
if (!(flags & HASWIDTH))
*flagp &= ~HASWIDTH;
*flagp |= flags & (SPSTART | HASNL | HASLOOKBH);
while (peekchr() == Magic('|'))
{
skipchr();
br = regbranch(&flags);
if (br == NULL || reg_toolong)
return NULL;
regtail(ret, br); 
if (!(flags & HASWIDTH))
*flagp &= ~HASWIDTH;
*flagp |= flags & (SPSTART | HASNL | HASLOOKBH);
}
ender = regnode(
#if defined(FEAT_SYN_HL)
paren == REG_ZPAREN ? ZCLOSE + parno :
#endif
paren == REG_PAREN ? MCLOSE + parno :
paren == REG_NPAREN ? NCLOSE : END);
regtail(ret, ender);
for (br = ret; br != NULL; br = regnext(br))
regoptail(br, ender);
if (paren != REG_NOPAREN && getchr() != Magic(')'))
{
#if defined(FEAT_SYN_HL)
if (paren == REG_ZPAREN)
EMSG_RET_NULL(_("E52: Unmatched \\z("));
else
#endif
if (paren == REG_NPAREN)
EMSG2_RET_NULL(_(e_unmatchedpp), reg_magic == MAGIC_ALL);
else
EMSG2_RET_NULL(_(e_unmatchedp), reg_magic == MAGIC_ALL);
}
else if (paren == REG_NOPAREN && peekchr() != NUL)
{
if (curchr == Magic(')'))
EMSG2_RET_NULL(_(e_unmatchedpar), reg_magic == MAGIC_ALL);
else
EMSG_RET_NULL(_(e_trailing)); 
}
if (paren == REG_PAREN)
had_endbrace[parno] = TRUE; 
return ret;
}
static regprog_T *
bt_regcomp(char_u *expr, int re_flags)
{
bt_regprog_T *r;
char_u *scan;
char_u *longest;
int len;
int flags;
if (expr == NULL)
EMSG_RET_NULL(_(e_null));
init_class_tab();
regcomp_start(expr, re_flags);
regcode = JUST_CALC_SIZE;
regc(REGMAGIC);
if (reg(REG_NOPAREN, &flags) == NULL)
return NULL;
r = alloc(offsetof(bt_regprog_T, program) + regsize);
if (r == NULL)
return NULL;
r->re_in_use = FALSE;
regcomp_start(expr, re_flags);
regcode = r->program;
regc(REGMAGIC);
if (reg(REG_NOPAREN, &flags) == NULL || reg_toolong)
{
vim_free(r);
if (reg_toolong)
EMSG_RET_NULL(_("E339: Pattern too long"));
return NULL;
}
r->regstart = NUL; 
r->reganch = 0;
r->regmust = NULL;
r->regmlen = 0;
r->regflags = regflags;
if (flags & HASNL)
r->regflags |= RF_HASNL;
if (flags & HASLOOKBH)
r->regflags |= RF_LOOKBH;
#if defined(FEAT_SYN_HL)
r->reghasz = re_has_z;
#endif
scan = r->program + 1; 
if (OP(regnext(scan)) == END) 
{
scan = OPERAND(scan);
if (OP(scan) == BOL || OP(scan) == RE_BOF)
{
r->reganch++;
scan = regnext(scan);
}
if (OP(scan) == EXACTLY)
{
if (has_mbyte)
r->regstart = (*mb_ptr2char)(OPERAND(scan));
else
r->regstart = *OPERAND(scan);
}
else if ((OP(scan) == BOW
|| OP(scan) == EOW
|| OP(scan) == NOTHING
|| OP(scan) == MOPEN + 0 || OP(scan) == NOPEN
|| OP(scan) == MCLOSE + 0 || OP(scan) == NCLOSE)
&& OP(regnext(scan)) == EXACTLY)
{
if (has_mbyte)
r->regstart = (*mb_ptr2char)(OPERAND(regnext(scan)));
else
r->regstart = *OPERAND(regnext(scan));
}
if ((flags & SPSTART || OP(scan) == BOW || OP(scan) == EOW)
&& !(flags & HASNL))
{
longest = NULL;
len = 0;
for (; scan != NULL; scan = regnext(scan))
if (OP(scan) == EXACTLY && STRLEN(OPERAND(scan)) >= (size_t)len)
{
longest = OPERAND(scan);
len = (int)STRLEN(OPERAND(scan));
}
r->regmust = longest;
r->regmlen = len;
}
}
#if defined(BT_REGEXP_DUMP)
regdump(expr, r);
#endif
r->engine = &bt_regengine;
return (regprog_T *)r;
}
#if defined(FEAT_SYN_HL) || defined(PROTO)
int
vim_regcomp_had_eol(void)
{
return had_eol;
}
#endif
static int
coll_get_char(void)
{
long nr = -1;
switch (*regparse++)
{
case 'd': nr = getdecchrs(); break;
case 'o': nr = getoctchrs(); break;
case 'x': nr = gethexchrs(2); break;
case 'u': nr = gethexchrs(4); break;
case 'U': nr = gethexchrs(8); break;
}
if (nr < 0 || nr > INT_MAX)
{
--regparse;
nr = '\\';
}
return nr;
}
static void
bt_regfree(regprog_T *prog)
{
vim_free(prog);
}
#define ADVANCE_REGINPUT() MB_PTR_ADV(rex.input)
static long bl_minval;
static long bl_maxval;
static void
reg_save(regsave_T *save, garray_T *gap)
{
if (REG_MULTI)
{
save->rs_u.pos.col = (colnr_T)(rex.input - rex.line);
save->rs_u.pos.lnum = rex.lnum;
}
else
save->rs_u.ptr = rex.input;
save->rs_len = gap->ga_len;
}
static void
reg_restore(regsave_T *save, garray_T *gap)
{
if (REG_MULTI)
{
if (rex.lnum != save->rs_u.pos.lnum)
{
rex.lnum = save->rs_u.pos.lnum;
rex.line = reg_getline(rex.lnum);
}
rex.input = rex.line + save->rs_u.pos.col;
}
else
rex.input = save->rs_u.ptr;
gap->ga_len = save->rs_len;
}
static int
reg_save_equal(regsave_T *save)
{
if (REG_MULTI)
return rex.lnum == save->rs_u.pos.lnum
&& rex.input == rex.line + save->rs_u.pos.col;
return rex.input == save->rs_u.ptr;
}
#define save_se(savep, posp, pp) REG_MULTI ? save_se_multi((savep), (posp)) : save_se_one((savep), (pp))
#define restore_se(savep, posp, pp) { if (REG_MULTI) *(posp) = (savep)->se_u.pos; else *(pp) = (savep)->se_u.ptr; }
static void
save_se_multi(save_se_T *savep, lpos_T *posp)
{
savep->se_u.pos = *posp;
posp->lnum = rex.lnum;
posp->col = (colnr_T)(rex.input - rex.line);
}
static void
save_se_one(save_se_T *savep, char_u **pp)
{
savep->se_u.ptr = *pp;
*pp = rex.input;
}
static int
regrepeat(
char_u *p,
long maxcount) 
{
long count = 0;
char_u *scan;
char_u *opnd;
int mask;
int testval = 0;
scan = rex.input; 
opnd = OPERAND(p);
switch (OP(p))
{
case ANY:
case ANY + ADD_NL:
while (count < maxcount)
{
while (*scan != NUL && count < maxcount)
{
++count;
MB_PTR_ADV(scan);
}
if (!REG_MULTI || !WITH_NL(OP(p)) || rex.lnum > rex.reg_maxline
|| rex.reg_line_lbr || count == maxcount)
break;
++count; 
reg_nextline();
scan = rex.input;
if (got_int)
break;
}
break;
case IDENT:
case IDENT + ADD_NL:
testval = TRUE;
case SIDENT:
case SIDENT + ADD_NL:
while (count < maxcount)
{
if (vim_isIDc(PTR2CHAR(scan)) && (testval || !VIM_ISDIGIT(*scan)))
{
MB_PTR_ADV(scan);
}
else if (*scan == NUL)
{
if (!REG_MULTI || !WITH_NL(OP(p)) || rex.lnum > rex.reg_maxline
|| rex.reg_line_lbr)
break;
reg_nextline();
scan = rex.input;
if (got_int)
break;
}
else if (rex.reg_line_lbr && *scan == '\n' && WITH_NL(OP(p)))
++scan;
else
break;
++count;
}
break;
case KWORD:
case KWORD + ADD_NL:
testval = TRUE;
case SKWORD:
case SKWORD + ADD_NL:
while (count < maxcount)
{
if (vim_iswordp_buf(scan, rex.reg_buf)
&& (testval || !VIM_ISDIGIT(*scan)))
{
MB_PTR_ADV(scan);
}
else if (*scan == NUL)
{
if (!REG_MULTI || !WITH_NL(OP(p)) || rex.lnum > rex.reg_maxline
|| rex.reg_line_lbr)
break;
reg_nextline();
scan = rex.input;
if (got_int)
break;
}
else if (rex.reg_line_lbr && *scan == '\n' && WITH_NL(OP(p)))
++scan;
else
break;
++count;
}
break;
case FNAME:
case FNAME + ADD_NL:
testval = TRUE;
case SFNAME:
case SFNAME + ADD_NL:
while (count < maxcount)
{
if (vim_isfilec(PTR2CHAR(scan)) && (testval || !VIM_ISDIGIT(*scan)))
{
MB_PTR_ADV(scan);
}
else if (*scan == NUL)
{
if (!REG_MULTI || !WITH_NL(OP(p)) || rex.lnum > rex.reg_maxline
|| rex.reg_line_lbr)
break;
reg_nextline();
scan = rex.input;
if (got_int)
break;
}
else if (rex.reg_line_lbr && *scan == '\n' && WITH_NL(OP(p)))
++scan;
else
break;
++count;
}
break;
case PRINT:
case PRINT + ADD_NL:
testval = TRUE;
case SPRINT:
case SPRINT + ADD_NL:
while (count < maxcount)
{
if (*scan == NUL)
{
if (!REG_MULTI || !WITH_NL(OP(p)) || rex.lnum > rex.reg_maxline
|| rex.reg_line_lbr)
break;
reg_nextline();
scan = rex.input;
if (got_int)
break;
}
else if (vim_isprintc(PTR2CHAR(scan)) == 1
&& (testval || !VIM_ISDIGIT(*scan)))
{
MB_PTR_ADV(scan);
}
else if (rex.reg_line_lbr && *scan == '\n' && WITH_NL(OP(p)))
++scan;
else
break;
++count;
}
break;
case WHITE:
case WHITE + ADD_NL:
testval = mask = RI_WHITE;
do_class:
while (count < maxcount)
{
int l;
if (*scan == NUL)
{
if (!REG_MULTI || !WITH_NL(OP(p)) || rex.lnum > rex.reg_maxline
|| rex.reg_line_lbr)
break;
reg_nextline();
scan = rex.input;
if (got_int)
break;
}
else if (has_mbyte && (l = (*mb_ptr2len)(scan)) > 1)
{
if (testval != 0)
break;
scan += l;
}
else if ((class_tab[*scan] & mask) == testval)
++scan;
else if (rex.reg_line_lbr && *scan == '\n' && WITH_NL(OP(p)))
++scan;
else
break;
++count;
}
break;
case NWHITE:
case NWHITE + ADD_NL:
mask = RI_WHITE;
goto do_class;
case DIGIT:
case DIGIT + ADD_NL:
testval = mask = RI_DIGIT;
goto do_class;
case NDIGIT:
case NDIGIT + ADD_NL:
mask = RI_DIGIT;
goto do_class;
case HEX:
case HEX + ADD_NL:
testval = mask = RI_HEX;
goto do_class;
case NHEX:
case NHEX + ADD_NL:
mask = RI_HEX;
goto do_class;
case OCTAL:
case OCTAL + ADD_NL:
testval = mask = RI_OCTAL;
goto do_class;
case NOCTAL:
case NOCTAL + ADD_NL:
mask = RI_OCTAL;
goto do_class;
case WORD:
case WORD + ADD_NL:
testval = mask = RI_WORD;
goto do_class;
case NWORD:
case NWORD + ADD_NL:
mask = RI_WORD;
goto do_class;
case HEAD:
case HEAD + ADD_NL:
testval = mask = RI_HEAD;
goto do_class;
case NHEAD:
case NHEAD + ADD_NL:
mask = RI_HEAD;
goto do_class;
case ALPHA:
case ALPHA + ADD_NL:
testval = mask = RI_ALPHA;
goto do_class;
case NALPHA:
case NALPHA + ADD_NL:
mask = RI_ALPHA;
goto do_class;
case LOWER:
case LOWER + ADD_NL:
testval = mask = RI_LOWER;
goto do_class;
case NLOWER:
case NLOWER + ADD_NL:
mask = RI_LOWER;
goto do_class;
case UPPER:
case UPPER + ADD_NL:
testval = mask = RI_UPPER;
goto do_class;
case NUPPER:
case NUPPER + ADD_NL:
mask = RI_UPPER;
goto do_class;
case EXACTLY:
{
int cu, cl;
if (rex.reg_ic)
{
cu = MB_TOUPPER(*opnd);
cl = MB_TOLOWER(*opnd);
while (count < maxcount && (*scan == cu || *scan == cl))
{
count++;
scan++;
}
}
else
{
cu = *opnd;
while (count < maxcount && *scan == cu)
{
count++;
scan++;
}
}
break;
}
case MULTIBYTECODE:
{
int i, len, cf = 0;
if ((len = (*mb_ptr2len)(opnd)) > 1)
{
if (rex.reg_ic && enc_utf8)
cf = utf_fold(utf_ptr2char(opnd));
while (count < maxcount && (*mb_ptr2len)(scan) >= len)
{
for (i = 0; i < len; ++i)
if (opnd[i] != scan[i])
break;
if (i < len && (!rex.reg_ic || !enc_utf8
|| utf_fold(utf_ptr2char(scan)) != cf))
break;
scan += len;
++count;
}
}
}
break;
case ANYOF:
case ANYOF + ADD_NL:
testval = TRUE;
case ANYBUT:
case ANYBUT + ADD_NL:
while (count < maxcount)
{
int len;
if (*scan == NUL)
{
if (!REG_MULTI || !WITH_NL(OP(p)) || rex.lnum > rex.reg_maxline
|| rex.reg_line_lbr)
break;
reg_nextline();
scan = rex.input;
if (got_int)
break;
}
else if (rex.reg_line_lbr && *scan == '\n' && WITH_NL(OP(p)))
++scan;
else if (has_mbyte && (len = (*mb_ptr2len)(scan)) > 1)
{
if ((cstrchr(opnd, (*mb_ptr2char)(scan)) == NULL) == testval)
break;
scan += len;
}
else
{
if ((cstrchr(opnd, *scan) == NULL) == testval)
break;
++scan;
}
++count;
}
break;
case NEWL:
while (count < maxcount
&& ((*scan == NUL && rex.lnum <= rex.reg_maxline
&& !rex.reg_line_lbr && REG_MULTI)
|| (*scan == '\n' && rex.reg_line_lbr)))
{
count++;
if (rex.reg_line_lbr)
ADVANCE_REGINPUT();
else
reg_nextline();
scan = rex.input;
if (got_int)
break;
}
break;
default: 
emsg(_(e_re_corr));
#if defined(DEBUG)
printf("Called regrepeat with op code %d\n", OP(p));
#endif
break;
}
rex.input = scan;
return (int)count;
}
static regitem_T *
regstack_push(regstate_T state, char_u *scan)
{
regitem_T *rp;
if ((long)((unsigned)regstack.ga_len >> 10) >= p_mmp)
{
emsg(_(e_maxmempat));
return NULL;
}
if (ga_grow(&regstack, sizeof(regitem_T)) == FAIL)
return NULL;
rp = (regitem_T *)((char *)regstack.ga_data + regstack.ga_len);
rp->rs_state = state;
rp->rs_scan = scan;
regstack.ga_len += sizeof(regitem_T);
return rp;
}
static void
regstack_pop(char_u **scan)
{
regitem_T *rp;
rp = (regitem_T *)((char *)regstack.ga_data + regstack.ga_len) - 1;
*scan = rp->rs_scan;
regstack.ga_len -= sizeof(regitem_T);
}
static void
save_subexpr(regbehind_T *bp)
{
int i;
bp->save_need_clear_subexpr = rex.need_clear_subexpr;
if (!rex.need_clear_subexpr)
{
for (i = 0; i < NSUBEXP; ++i)
{
if (REG_MULTI)
{
bp->save_start[i].se_u.pos = rex.reg_startpos[i];
bp->save_end[i].se_u.pos = rex.reg_endpos[i];
}
else
{
bp->save_start[i].se_u.ptr = rex.reg_startp[i];
bp->save_end[i].se_u.ptr = rex.reg_endp[i];
}
}
}
}
static void
restore_subexpr(regbehind_T *bp)
{
int i;
rex.need_clear_subexpr = bp->save_need_clear_subexpr;
if (!rex.need_clear_subexpr)
{
for (i = 0; i < NSUBEXP; ++i)
{
if (REG_MULTI)
{
rex.reg_startpos[i] = bp->save_start[i].se_u.pos;
rex.reg_endpos[i] = bp->save_end[i].se_u.pos;
}
else
{
rex.reg_startp[i] = bp->save_start[i].se_u.ptr;
rex.reg_endp[i] = bp->save_end[i].se_u.ptr;
}
}
}
}
static int
regmatch(
char_u *scan, 
proftime_T *tm UNUSED, 
int *timed_out UNUSED) 
{
char_u *next; 
int op;
int c;
regitem_T *rp;
int no;
int status; 
#if defined(FEAT_RELTIME)
int tm_count = 0;
#endif
regstack.ga_len = 0;
backpos.ga_len = 0;
for (;;)
{
fast_breakcheck();
#if defined(DEBUG)
if (scan != NULL && regnarrate)
{
mch_errmsg((char *)regprop(scan));
mch_errmsg("(\n");
}
#endif
for (;;)
{
if (got_int || scan == NULL)
{
status = RA_FAIL;
break;
}
#if defined(FEAT_RELTIME)
if (tm != NULL && ++tm_count == 100)
{
tm_count = 0;
if (profile_passed_limit(tm))
{
if (timed_out != NULL)
*timed_out = TRUE;
status = RA_FAIL;
break;
}
}
#endif
status = RA_CONT;
#if defined(DEBUG)
if (regnarrate)
{
mch_errmsg((char *)regprop(scan));
mch_errmsg("...\n");
#if defined(FEAT_SYN_HL)
if (re_extmatch_in != NULL)
{
int i;
mch_errmsg(_("External submatches:\n"));
for (i = 0; i < NSUBEXP; i++)
{
mch_errmsg(" \"");
if (re_extmatch_in->matches[i] != NULL)
mch_errmsg((char *)re_extmatch_in->matches[i]);
mch_errmsg("\"\n");
}
}
#endif
}
#endif
next = regnext(scan);
op = OP(scan);
if (!rex.reg_line_lbr && WITH_NL(op) && REG_MULTI
&& *rex.input == NUL && rex.lnum <= rex.reg_maxline)
{
reg_nextline();
}
else if (rex.reg_line_lbr && WITH_NL(op) && *rex.input == '\n')
{
ADVANCE_REGINPUT();
}
else
{
if (WITH_NL(op))
op -= ADD_NL;
if (has_mbyte)
c = (*mb_ptr2char)(rex.input);
else
c = *rex.input;
switch (op)
{
case BOL:
if (rex.input != rex.line)
status = RA_NOMATCH;
break;
case EOL:
if (c != NUL)
status = RA_NOMATCH;
break;
case RE_BOF:
if (rex.lnum != 0 || rex.input != rex.line
|| (REG_MULTI && rex.reg_firstlnum > 1))
status = RA_NOMATCH;
break;
case RE_EOF:
if (rex.lnum != rex.reg_maxline || c != NUL)
status = RA_NOMATCH;
break;
case CURSOR:
if (rex.reg_win == NULL
|| (rex.lnum + rex.reg_firstlnum
!= rex.reg_win->w_cursor.lnum)
|| ((colnr_T)(rex.input - rex.line)
!= rex.reg_win->w_cursor.col))
status = RA_NOMATCH;
break;
case RE_MARK:
{
int mark = OPERAND(scan)[0];
int cmp = OPERAND(scan)[1];
pos_T *pos;
pos = getmark_buf(rex.reg_buf, mark, FALSE);
if (pos == NULL 
|| pos->lnum <= 0 
|| (pos->lnum == rex.lnum + rex.reg_firstlnum
? (pos->col == (colnr_T)(rex.input - rex.line)
? (cmp == '<' || cmp == '>')
: (pos->col < (colnr_T)(rex.input - rex.line)
? cmp != '>'
: cmp != '<'))
: (pos->lnum < rex.lnum + rex.reg_firstlnum
? cmp != '>'
: cmp != '<')))
status = RA_NOMATCH;
}
break;
case RE_VISUAL:
if (!reg_match_visual())
status = RA_NOMATCH;
break;
case RE_LNUM:
if (!REG_MULTI || !re_num_cmp((long_u)(rex.lnum + rex.reg_firstlnum),
scan))
status = RA_NOMATCH;
break;
case RE_COL:
if (!re_num_cmp((long_u)(rex.input - rex.line) + 1, scan))
status = RA_NOMATCH;
break;
case RE_VCOL:
if (!re_num_cmp((long_u)win_linetabsize(
rex.reg_win == NULL ? curwin : rex.reg_win,
rex.line, (colnr_T)(rex.input - rex.line)) + 1, scan))
status = RA_NOMATCH;
break;
case BOW: 
if (c == NUL) 
status = RA_NOMATCH;
else if (has_mbyte)
{
int this_class;
this_class = mb_get_class_buf(rex.input, rex.reg_buf);
if (this_class <= 1)
status = RA_NOMATCH; 
else if (reg_prev_class() == this_class)
status = RA_NOMATCH; 
}
else
{
if (!vim_iswordc_buf(c, rex.reg_buf) || (rex.input > rex.line
&& vim_iswordc_buf(rex.input[-1], rex.reg_buf)))
status = RA_NOMATCH;
}
break;
case EOW: 
if (rex.input == rex.line) 
status = RA_NOMATCH;
else if (has_mbyte)
{
int this_class, prev_class;
this_class = mb_get_class_buf(rex.input, rex.reg_buf);
prev_class = reg_prev_class();
if (this_class == prev_class
|| prev_class == 0 || prev_class == 1)
status = RA_NOMATCH;
}
else
{
if (!vim_iswordc_buf(rex.input[-1], rex.reg_buf)
|| (rex.input[0] != NUL
&& vim_iswordc_buf(c, rex.reg_buf)))
status = RA_NOMATCH;
}
break; 
case ANY:
if (c == NUL)
status = RA_NOMATCH;
else
ADVANCE_REGINPUT();
break;
case IDENT:
if (!vim_isIDc(c))
status = RA_NOMATCH;
else
ADVANCE_REGINPUT();
break;
case SIDENT:
if (VIM_ISDIGIT(*rex.input) || !vim_isIDc(c))
status = RA_NOMATCH;
else
ADVANCE_REGINPUT();
break;
case KWORD:
if (!vim_iswordp_buf(rex.input, rex.reg_buf))
status = RA_NOMATCH;
else
ADVANCE_REGINPUT();
break;
case SKWORD:
if (VIM_ISDIGIT(*rex.input)
|| !vim_iswordp_buf(rex.input, rex.reg_buf))
status = RA_NOMATCH;
else
ADVANCE_REGINPUT();
break;
case FNAME:
if (!vim_isfilec(c))
status = RA_NOMATCH;
else
ADVANCE_REGINPUT();
break;
case SFNAME:
if (VIM_ISDIGIT(*rex.input) || !vim_isfilec(c))
status = RA_NOMATCH;
else
ADVANCE_REGINPUT();
break;
case PRINT:
if (!vim_isprintc(PTR2CHAR(rex.input)))
status = RA_NOMATCH;
else
ADVANCE_REGINPUT();
break;
case SPRINT:
if (VIM_ISDIGIT(*rex.input) || !vim_isprintc(PTR2CHAR(rex.input)))
status = RA_NOMATCH;
else
ADVANCE_REGINPUT();
break;
case WHITE:
if (!VIM_ISWHITE(c))
status = RA_NOMATCH;
else
ADVANCE_REGINPUT();
break;
case NWHITE:
if (c == NUL || VIM_ISWHITE(c))
status = RA_NOMATCH;
else
ADVANCE_REGINPUT();
break;
case DIGIT:
if (!ri_digit(c))
status = RA_NOMATCH;
else
ADVANCE_REGINPUT();
break;
case NDIGIT:
if (c == NUL || ri_digit(c))
status = RA_NOMATCH;
else
ADVANCE_REGINPUT();
break;
case HEX:
if (!ri_hex(c))
status = RA_NOMATCH;
else
ADVANCE_REGINPUT();
break;
case NHEX:
if (c == NUL || ri_hex(c))
status = RA_NOMATCH;
else
ADVANCE_REGINPUT();
break;
case OCTAL:
if (!ri_octal(c))
status = RA_NOMATCH;
else
ADVANCE_REGINPUT();
break;
case NOCTAL:
if (c == NUL || ri_octal(c))
status = RA_NOMATCH;
else
ADVANCE_REGINPUT();
break;
case WORD:
if (!ri_word(c))
status = RA_NOMATCH;
else
ADVANCE_REGINPUT();
break;
case NWORD:
if (c == NUL || ri_word(c))
status = RA_NOMATCH;
else
ADVANCE_REGINPUT();
break;
case HEAD:
if (!ri_head(c))
status = RA_NOMATCH;
else
ADVANCE_REGINPUT();
break;
case NHEAD:
if (c == NUL || ri_head(c))
status = RA_NOMATCH;
else
ADVANCE_REGINPUT();
break;
case ALPHA:
if (!ri_alpha(c))
status = RA_NOMATCH;
else
ADVANCE_REGINPUT();
break;
case NALPHA:
if (c == NUL || ri_alpha(c))
status = RA_NOMATCH;
else
ADVANCE_REGINPUT();
break;
case LOWER:
if (!ri_lower(c))
status = RA_NOMATCH;
else
ADVANCE_REGINPUT();
break;
case NLOWER:
if (c == NUL || ri_lower(c))
status = RA_NOMATCH;
else
ADVANCE_REGINPUT();
break;
case UPPER:
if (!ri_upper(c))
status = RA_NOMATCH;
else
ADVANCE_REGINPUT();
break;
case NUPPER:
if (c == NUL || ri_upper(c))
status = RA_NOMATCH;
else
ADVANCE_REGINPUT();
break;
case EXACTLY:
{
int len;
char_u *opnd;
opnd = OPERAND(scan);
if (*opnd != *rex.input
&& (!rex.reg_ic
|| (!enc_utf8
&& MB_TOLOWER(*opnd) != MB_TOLOWER(*rex.input))))
status = RA_NOMATCH;
else if (*opnd == NUL)
{
}
else
{
if (opnd[1] == NUL && !(enc_utf8 && rex.reg_ic))
{
len = 1; 
}
else
{
len = (int)STRLEN(opnd);
if (cstrncmp(opnd, rex.input, &len) != 0)
status = RA_NOMATCH;
}
if (status != RA_NOMATCH
&& enc_utf8
&& UTF_COMPOSINGLIKE(rex.input, rex.input + len)
&& !rex.reg_icombine
&& OP(next) != RE_COMPOSING)
{
status = RA_NOMATCH;
}
if (status != RA_NOMATCH)
rex.input += len;
}
}
break;
case ANYOF:
case ANYBUT:
if (c == NUL)
status = RA_NOMATCH;
else if ((cstrchr(OPERAND(scan), c) == NULL) == (op == ANYOF))
status = RA_NOMATCH;
else
ADVANCE_REGINPUT();
break;
case MULTIBYTECODE:
if (has_mbyte)
{
int i, len;
char_u *opnd;
int opndc = 0, inpc;
opnd = OPERAND(scan);
if ((len = (*mb_ptr2len)(opnd)) < 2)
{
status = RA_NOMATCH;
break;
}
if (enc_utf8)
opndc = utf_ptr2char(opnd);
if (enc_utf8 && utf_iscomposing(opndc))
{
status = RA_NOMATCH;
for (i = 0; rex.input[i] != NUL;
i += utf_ptr2len(rex.input + i))
{
inpc = utf_ptr2char(rex.input + i);
if (!utf_iscomposing(inpc))
{
if (i > 0)
break;
}
else if (opndc == inpc)
{
len = i + utfc_ptr2len(rex.input + i);
status = RA_MATCH;
break;
}
}
}
else
for (i = 0; i < len; ++i)
if (opnd[i] != rex.input[i])
{
status = RA_NOMATCH;
break;
}
rex.input += len;
}
else
status = RA_NOMATCH;
break;
case RE_COMPOSING:
if (enc_utf8)
{
while (utf_iscomposing(utf_ptr2char(rex.input)))
MB_CPTR_ADV(rex.input);
}
break;
case NOTHING:
break;
case BACK:
{
int i;
backpos_T *bp;
bp = (backpos_T *)backpos.ga_data;
for (i = 0; i < backpos.ga_len; ++i)
if (bp[i].bp_scan == scan)
break;
if (i == backpos.ga_len)
{
if (ga_grow(&backpos, 1) == FAIL)
status = RA_FAIL;
else
{
bp = (backpos_T *)backpos.ga_data;
bp[i].bp_scan = scan;
++backpos.ga_len;
}
}
else if (reg_save_equal(&bp[i].bp_pos))
status = RA_NOMATCH;
if (status != RA_FAIL && status != RA_NOMATCH)
reg_save(&bp[i].bp_pos, &backpos);
}
break;
case MOPEN + 0: 
case MOPEN + 1: 
case MOPEN + 2:
case MOPEN + 3:
case MOPEN + 4:
case MOPEN + 5:
case MOPEN + 6:
case MOPEN + 7:
case MOPEN + 8:
case MOPEN + 9:
{
no = op - MOPEN;
cleanup_subexpr();
rp = regstack_push(RS_MOPEN, scan);
if (rp == NULL)
status = RA_FAIL;
else
{
rp->rs_no = no;
save_se(&rp->rs_un.sesave, &rex.reg_startpos[no],
&rex.reg_startp[no]);
}
}
break;
case NOPEN: 
case NCLOSE: 
if (regstack_push(RS_NOPEN, scan) == NULL)
status = RA_FAIL;
break;
#if defined(FEAT_SYN_HL)
case ZOPEN + 1:
case ZOPEN + 2:
case ZOPEN + 3:
case ZOPEN + 4:
case ZOPEN + 5:
case ZOPEN + 6:
case ZOPEN + 7:
case ZOPEN + 8:
case ZOPEN + 9:
{
no = op - ZOPEN;
cleanup_zsubexpr();
rp = regstack_push(RS_ZOPEN, scan);
if (rp == NULL)
status = RA_FAIL;
else
{
rp->rs_no = no;
save_se(&rp->rs_un.sesave, &reg_startzpos[no],
&reg_startzp[no]);
}
}
break;
#endif
case MCLOSE + 0: 
case MCLOSE + 1: 
case MCLOSE + 2:
case MCLOSE + 3:
case MCLOSE + 4:
case MCLOSE + 5:
case MCLOSE + 6:
case MCLOSE + 7:
case MCLOSE + 8:
case MCLOSE + 9:
{
no = op - MCLOSE;
cleanup_subexpr();
rp = regstack_push(RS_MCLOSE, scan);
if (rp == NULL)
status = RA_FAIL;
else
{
rp->rs_no = no;
save_se(&rp->rs_un.sesave, &rex.reg_endpos[no],
&rex.reg_endp[no]);
}
}
break;
#if defined(FEAT_SYN_HL)
case ZCLOSE + 1: 
case ZCLOSE + 2:
case ZCLOSE + 3:
case ZCLOSE + 4:
case ZCLOSE + 5:
case ZCLOSE + 6:
case ZCLOSE + 7:
case ZCLOSE + 8:
case ZCLOSE + 9:
{
no = op - ZCLOSE;
cleanup_zsubexpr();
rp = regstack_push(RS_ZCLOSE, scan);
if (rp == NULL)
status = RA_FAIL;
else
{
rp->rs_no = no;
save_se(&rp->rs_un.sesave, &reg_endzpos[no],
&reg_endzp[no]);
}
}
break;
#endif
case BACKREF + 1:
case BACKREF + 2:
case BACKREF + 3:
case BACKREF + 4:
case BACKREF + 5:
case BACKREF + 6:
case BACKREF + 7:
case BACKREF + 8:
case BACKREF + 9:
{
int len;
no = op - BACKREF;
cleanup_subexpr();
if (!REG_MULTI) 
{
if (rex.reg_startp[no] == NULL || rex.reg_endp[no] == NULL)
{
len = 0;
}
else
{
len = (int)(rex.reg_endp[no] - rex.reg_startp[no]);
if (cstrncmp(rex.reg_startp[no], rex.input, &len) != 0)
status = RA_NOMATCH;
}
}
else 
{
if (rex.reg_startpos[no].lnum < 0
|| rex.reg_endpos[no].lnum < 0)
{
len = 0;
}
else
{
if (rex.reg_startpos[no].lnum == rex.lnum
&& rex.reg_endpos[no].lnum == rex.lnum)
{
len = rex.reg_endpos[no].col
- rex.reg_startpos[no].col;
if (cstrncmp(rex.line + rex.reg_startpos[no].col,
rex.input, &len) != 0)
status = RA_NOMATCH;
}
else
{
int r = match_with_backref(
rex.reg_startpos[no].lnum,
rex.reg_startpos[no].col,
rex.reg_endpos[no].lnum,
rex.reg_endpos[no].col,
&len);
if (r != RA_MATCH)
status = r;
}
}
}
rex.input += len;
}
break;
#if defined(FEAT_SYN_HL)
case ZREF + 1:
case ZREF + 2:
case ZREF + 3:
case ZREF + 4:
case ZREF + 5:
case ZREF + 6:
case ZREF + 7:
case ZREF + 8:
case ZREF + 9:
{
int len;
cleanup_zsubexpr();
no = op - ZREF;
if (re_extmatch_in != NULL
&& re_extmatch_in->matches[no] != NULL)
{
len = (int)STRLEN(re_extmatch_in->matches[no]);
if (cstrncmp(re_extmatch_in->matches[no],
rex.input, &len) != 0)
status = RA_NOMATCH;
else
rex.input += len;
}
else
{
}
}
break;
#endif
case BRANCH:
{
if (OP(next) != BRANCH) 
next = OPERAND(scan); 
else
{
rp = regstack_push(RS_BRANCH, scan);
if (rp == NULL)
status = RA_FAIL;
else
status = RA_BREAK; 
}
}
break;
case BRACE_LIMITS:
{
if (OP(next) == BRACE_SIMPLE)
{
bl_minval = OPERAND_MIN(scan);
bl_maxval = OPERAND_MAX(scan);
}
else if (OP(next) >= BRACE_COMPLEX
&& OP(next) < BRACE_COMPLEX + 10)
{
no = OP(next) - BRACE_COMPLEX;
brace_min[no] = OPERAND_MIN(scan);
brace_max[no] = OPERAND_MAX(scan);
brace_count[no] = 0;
}
else
{
internal_error("BRACE_LIMITS");
status = RA_FAIL;
}
}
break;
case BRACE_COMPLEX + 0:
case BRACE_COMPLEX + 1:
case BRACE_COMPLEX + 2:
case BRACE_COMPLEX + 3:
case BRACE_COMPLEX + 4:
case BRACE_COMPLEX + 5:
case BRACE_COMPLEX + 6:
case BRACE_COMPLEX + 7:
case BRACE_COMPLEX + 8:
case BRACE_COMPLEX + 9:
{
no = op - BRACE_COMPLEX;
++brace_count[no];
if (brace_count[no] <= (brace_min[no] <= brace_max[no]
? brace_min[no] : brace_max[no]))
{
rp = regstack_push(RS_BRCPLX_MORE, scan);
if (rp == NULL)
status = RA_FAIL;
else
{
rp->rs_no = no;
reg_save(&rp->rs_un.regsave, &backpos);
next = OPERAND(scan);
}
break;
}
if (brace_min[no] <= brace_max[no])
{
if (brace_count[no] <= brace_max[no])
{
rp = regstack_push(RS_BRCPLX_LONG, scan);
if (rp == NULL)
status = RA_FAIL;
else
{
rp->rs_no = no;
reg_save(&rp->rs_un.regsave, &backpos);
next = OPERAND(scan);
}
}
}
else
{
if (brace_count[no] <= brace_min[no])
{
rp = regstack_push(RS_BRCPLX_SHORT, scan);
if (rp == NULL)
status = RA_FAIL;
else
{
reg_save(&rp->rs_un.regsave, &backpos);
}
}
}
}
break;
case BRACE_SIMPLE:
case STAR:
case PLUS:
{
regstar_T rst;
if (OP(next) == EXACTLY)
{
rst.nextb = *OPERAND(next);
if (rex.reg_ic)
{
if (MB_ISUPPER(rst.nextb))
rst.nextb_ic = MB_TOLOWER(rst.nextb);
else
rst.nextb_ic = MB_TOUPPER(rst.nextb);
}
else
rst.nextb_ic = rst.nextb;
}
else
{
rst.nextb = NUL;
rst.nextb_ic = NUL;
}
if (op != BRACE_SIMPLE)
{
rst.minval = (op == STAR) ? 0 : 1;
rst.maxval = MAX_LIMIT;
}
else
{
rst.minval = bl_minval;
rst.maxval = bl_maxval;
}
rst.count = regrepeat(OPERAND(scan), rst.maxval);
if (got_int)
{
status = RA_FAIL;
break;
}
if (rst.minval <= rst.maxval
? rst.count >= rst.minval : rst.count >= rst.maxval)
{
if ((long)((unsigned)regstack.ga_len >> 10) >= p_mmp)
{
emsg(_(e_maxmempat));
status = RA_FAIL;
}
else if (ga_grow(&regstack, sizeof(regstar_T)) == FAIL)
status = RA_FAIL;
else
{
regstack.ga_len += sizeof(regstar_T);
rp = regstack_push(rst.minval <= rst.maxval
? RS_STAR_LONG : RS_STAR_SHORT, scan);
if (rp == NULL)
status = RA_FAIL;
else
{
*(((regstar_T *)rp) - 1) = rst;
status = RA_BREAK; 
}
}
}
else
status = RA_NOMATCH;
}
break;
case NOMATCH:
case MATCH:
case SUBPAT:
rp = regstack_push(RS_NOMATCH, scan);
if (rp == NULL)
status = RA_FAIL;
else
{
rp->rs_no = op;
reg_save(&rp->rs_un.regsave, &backpos);
next = OPERAND(scan);
}
break;
case BEHIND:
case NOBEHIND:
if ((long)((unsigned)regstack.ga_len >> 10) >= p_mmp)
{
emsg(_(e_maxmempat));
status = RA_FAIL;
}
else if (ga_grow(&regstack, sizeof(regbehind_T)) == FAIL)
status = RA_FAIL;
else
{
regstack.ga_len += sizeof(regbehind_T);
rp = regstack_push(RS_BEHIND1, scan);
if (rp == NULL)
status = RA_FAIL;
else
{
save_subexpr(((regbehind_T *)rp) - 1);
rp->rs_no = op;
reg_save(&rp->rs_un.regsave, &backpos);
}
}
break;
case BHPOS:
if (REG_MULTI)
{
if (behind_pos.rs_u.pos.col != (colnr_T)(rex.input - rex.line)
|| behind_pos.rs_u.pos.lnum != rex.lnum)
status = RA_NOMATCH;
}
else if (behind_pos.rs_u.ptr != rex.input)
status = RA_NOMATCH;
break;
case NEWL:
if ((c != NUL || !REG_MULTI || rex.lnum > rex.reg_maxline
|| rex.reg_line_lbr)
&& (c != '\n' || !rex.reg_line_lbr))
status = RA_NOMATCH;
else if (rex.reg_line_lbr)
ADVANCE_REGINPUT();
else
reg_nextline();
break;
case END:
status = RA_MATCH; 
break;
default:
emsg(_(e_re_corr));
#if defined(DEBUG)
printf("Illegal op code %d\n", op);
#endif
status = RA_FAIL;
break;
}
}
if (status != RA_CONT)
break;
scan = next;
} 
while (regstack.ga_len > 0 && status != RA_FAIL)
{
rp = (regitem_T *)((char *)regstack.ga_data + regstack.ga_len) - 1;
switch (rp->rs_state)
{
case RS_NOPEN:
regstack_pop(&scan);
break;
case RS_MOPEN:
if (status == RA_NOMATCH)
restore_se(&rp->rs_un.sesave, &rex.reg_startpos[rp->rs_no],
&rex.reg_startp[rp->rs_no]);
regstack_pop(&scan);
break;
#if defined(FEAT_SYN_HL)
case RS_ZOPEN:
if (status == RA_NOMATCH)
restore_se(&rp->rs_un.sesave, &reg_startzpos[rp->rs_no],
&reg_startzp[rp->rs_no]);
regstack_pop(&scan);
break;
#endif
case RS_MCLOSE:
if (status == RA_NOMATCH)
restore_se(&rp->rs_un.sesave, &rex.reg_endpos[rp->rs_no],
&rex.reg_endp[rp->rs_no]);
regstack_pop(&scan);
break;
#if defined(FEAT_SYN_HL)
case RS_ZCLOSE:
if (status == RA_NOMATCH)
restore_se(&rp->rs_un.sesave, &reg_endzpos[rp->rs_no],
&reg_endzp[rp->rs_no]);
regstack_pop(&scan);
break;
#endif
case RS_BRANCH:
if (status == RA_MATCH)
regstack_pop(&scan);
else
{
if (status != RA_BREAK)
{
reg_restore(&rp->rs_un.regsave, &backpos);
scan = rp->rs_scan;
}
if (scan == NULL || OP(scan) != BRANCH)
{
status = RA_NOMATCH;
regstack_pop(&scan);
}
else
{
rp->rs_scan = regnext(scan);
reg_save(&rp->rs_un.regsave, &backpos);
scan = OPERAND(scan);
}
}
break;
case RS_BRCPLX_MORE:
if (status == RA_NOMATCH)
{
reg_restore(&rp->rs_un.regsave, &backpos);
--brace_count[rp->rs_no]; 
}
regstack_pop(&scan);
break;
case RS_BRCPLX_LONG:
if (status == RA_NOMATCH)
{
reg_restore(&rp->rs_un.regsave, &backpos);
--brace_count[rp->rs_no];
status = RA_CONT;
}
regstack_pop(&scan);
if (status == RA_CONT)
scan = regnext(scan);
break;
case RS_BRCPLX_SHORT:
if (status == RA_NOMATCH)
reg_restore(&rp->rs_un.regsave, &backpos);
regstack_pop(&scan);
if (status == RA_NOMATCH)
{
scan = OPERAND(scan);
status = RA_CONT;
}
break;
case RS_NOMATCH:
if (status == (rp->rs_no == NOMATCH ? RA_MATCH : RA_NOMATCH))
status = RA_NOMATCH;
else
{
status = RA_CONT;
if (rp->rs_no != SUBPAT) 
reg_restore(&rp->rs_un.regsave, &backpos);
}
regstack_pop(&scan);
if (status == RA_CONT)
scan = regnext(scan);
break;
case RS_BEHIND1:
if (status == RA_NOMATCH)
{
regstack_pop(&scan);
regstack.ga_len -= sizeof(regbehind_T);
}
else
{
reg_save(&(((regbehind_T *)rp) - 1)->save_after, &backpos);
(((regbehind_T *)rp) - 1)->save_behind = behind_pos;
behind_pos = rp->rs_un.regsave;
rp->rs_state = RS_BEHIND2;
reg_restore(&rp->rs_un.regsave, &backpos);
scan = OPERAND(rp->rs_scan) + 4;
}
break;
case RS_BEHIND2:
if (status == RA_MATCH && reg_save_equal(&behind_pos))
{
behind_pos = (((regbehind_T *)rp) - 1)->save_behind;
if (rp->rs_no == BEHIND)
reg_restore(&(((regbehind_T *)rp) - 1)->save_after,
&backpos);
else
{
status = RA_NOMATCH;
restore_subexpr(((regbehind_T *)rp) - 1);
}
regstack_pop(&scan);
regstack.ga_len -= sizeof(regbehind_T);
}
else
{
long limit;
no = OK;
limit = OPERAND_MIN(rp->rs_scan);
if (REG_MULTI)
{
if (limit > 0
&& ((rp->rs_un.regsave.rs_u.pos.lnum
< behind_pos.rs_u.pos.lnum
? (colnr_T)STRLEN(rex.line)
: behind_pos.rs_u.pos.col)
- rp->rs_un.regsave.rs_u.pos.col >= limit))
no = FAIL;
else if (rp->rs_un.regsave.rs_u.pos.col == 0)
{
if (rp->rs_un.regsave.rs_u.pos.lnum
< behind_pos.rs_u.pos.lnum
|| reg_getline(
--rp->rs_un.regsave.rs_u.pos.lnum)
== NULL)
no = FAIL;
else
{
reg_restore(&rp->rs_un.regsave, &backpos);
rp->rs_un.regsave.rs_u.pos.col =
(colnr_T)STRLEN(rex.line);
}
}
else
{
if (has_mbyte)
{
char_u *line =
reg_getline(rp->rs_un.regsave.rs_u.pos.lnum);
rp->rs_un.regsave.rs_u.pos.col -=
(*mb_head_off)(line, line
+ rp->rs_un.regsave.rs_u.pos.col - 1) + 1;
}
else
--rp->rs_un.regsave.rs_u.pos.col;
}
}
else
{
if (rp->rs_un.regsave.rs_u.ptr == rex.line)
no = FAIL;
else
{
MB_PTR_BACK(rex.line, rp->rs_un.regsave.rs_u.ptr);
if (limit > 0 && (long)(behind_pos.rs_u.ptr
- rp->rs_un.regsave.rs_u.ptr) > limit)
no = FAIL;
}
}
if (no == OK)
{
reg_restore(&rp->rs_un.regsave, &backpos);
scan = OPERAND(rp->rs_scan) + 4;
if (status == RA_MATCH)
{
status = RA_NOMATCH;
restore_subexpr(((regbehind_T *)rp) - 1);
}
}
else
{
behind_pos = (((regbehind_T *)rp) - 1)->save_behind;
if (rp->rs_no == NOBEHIND)
{
reg_restore(&(((regbehind_T *)rp) - 1)->save_after,
&backpos);
status = RA_MATCH;
}
else
{
if (status == RA_MATCH)
{
status = RA_NOMATCH;
restore_subexpr(((regbehind_T *)rp) - 1);
}
}
regstack_pop(&scan);
regstack.ga_len -= sizeof(regbehind_T);
}
}
break;
case RS_STAR_LONG:
case RS_STAR_SHORT:
{
regstar_T *rst = ((regstar_T *)rp) - 1;
if (status == RA_MATCH)
{
regstack_pop(&scan);
regstack.ga_len -= sizeof(regstar_T);
break;
}
if (status != RA_BREAK)
reg_restore(&rp->rs_un.regsave, &backpos);
for (;;)
{
if (status != RA_BREAK)
{
if (rp->rs_state == RS_STAR_LONG)
{
if (--rst->count < rst->minval)
break;
if (rex.input == rex.line)
{
--rex.lnum;
rex.line = reg_getline(rex.lnum);
if (rex.line == NULL)
break;
rex.input = rex.line + STRLEN(rex.line);
fast_breakcheck();
}
else
MB_PTR_BACK(rex.line, rex.input);
}
else
{
if (rst->count == rst->minval
|| regrepeat(OPERAND(rp->rs_scan), 1L) == 0)
break;
++rst->count;
}
if (got_int)
break;
}
else
status = RA_NOMATCH;
if (rst->nextb == NUL || *rex.input == rst->nextb
|| *rex.input == rst->nextb_ic)
{
reg_save(&rp->rs_un.regsave, &backpos);
scan = regnext(rp->rs_scan);
status = RA_CONT;
break;
}
}
if (status != RA_CONT)
{
regstack_pop(&scan);
regstack.ga_len -= sizeof(regstar_T);
status = RA_NOMATCH;
}
}
break;
}
if (status == RA_CONT || rp == (regitem_T *)
((char *)regstack.ga_data + regstack.ga_len) - 1)
break;
}
if (status == RA_CONT)
continue;
if (regstack.ga_len == 0 || status == RA_FAIL)
{
if (scan == NULL)
{
emsg(_(e_re_corr));
#if defined(DEBUG)
printf("Premature EOL\n");
#endif
}
return (status == RA_MATCH);
}
} 
}
static long
regtry(
bt_regprog_T *prog,
colnr_T col,
proftime_T *tm, 
int *timed_out) 
{
rex.input = rex.line + col;
rex.need_clear_subexpr = TRUE;
#if defined(FEAT_SYN_HL)
rex.need_clear_zsubexpr = (prog->reghasz == REX_SET);
#endif
if (regmatch(prog->program + 1, tm, timed_out) == 0)
return 0;
cleanup_subexpr();
if (REG_MULTI)
{
if (rex.reg_startpos[0].lnum < 0)
{
rex.reg_startpos[0].lnum = 0;
rex.reg_startpos[0].col = col;
}
if (rex.reg_endpos[0].lnum < 0)
{
rex.reg_endpos[0].lnum = rex.lnum;
rex.reg_endpos[0].col = (int)(rex.input - rex.line);
}
else
rex.lnum = rex.reg_endpos[0].lnum;
}
else
{
if (rex.reg_startp[0] == NULL)
rex.reg_startp[0] = rex.line + col;
if (rex.reg_endp[0] == NULL)
rex.reg_endp[0] = rex.input;
}
#if defined(FEAT_SYN_HL)
unref_extmatch(re_extmatch_out);
re_extmatch_out = NULL;
if (prog->reghasz == REX_SET)
{
int i;
cleanup_zsubexpr();
re_extmatch_out = make_extmatch();
if (re_extmatch_out == NULL)
return 0;
for (i = 0; i < NSUBEXP; i++)
{
if (REG_MULTI)
{
if (reg_startzpos[i].lnum >= 0
&& reg_endzpos[i].lnum == reg_startzpos[i].lnum
&& reg_endzpos[i].col >= reg_startzpos[i].col)
re_extmatch_out->matches[i] =
vim_strnsave(reg_getline(reg_startzpos[i].lnum)
+ reg_startzpos[i].col,
reg_endzpos[i].col - reg_startzpos[i].col);
}
else
{
if (reg_startzp[i] != NULL && reg_endzp[i] != NULL)
re_extmatch_out->matches[i] =
vim_strnsave(reg_startzp[i],
(int)(reg_endzp[i] - reg_startzp[i]));
}
}
}
#endif
return 1 + rex.lnum;
}
static long
bt_regexec_both(
char_u *line,
colnr_T col, 
proftime_T *tm, 
int *timed_out) 
{
bt_regprog_T *prog;
char_u *s;
long retval = 0L;
if (regstack.ga_data == NULL)
{
ga_init2(&regstack, 1, REGSTACK_INITIAL);
(void)ga_grow(&regstack, REGSTACK_INITIAL);
regstack.ga_growsize = REGSTACK_INITIAL * 8;
}
if (backpos.ga_data == NULL)
{
ga_init2(&backpos, sizeof(backpos_T), BACKPOS_INITIAL);
(void)ga_grow(&backpos, BACKPOS_INITIAL);
backpos.ga_growsize = BACKPOS_INITIAL * 8;
}
if (REG_MULTI)
{
prog = (bt_regprog_T *)rex.reg_mmatch->regprog;
line = reg_getline((linenr_T)0);
rex.reg_startpos = rex.reg_mmatch->startpos;
rex.reg_endpos = rex.reg_mmatch->endpos;
}
else
{
prog = (bt_regprog_T *)rex.reg_match->regprog;
rex.reg_startp = rex.reg_match->startp;
rex.reg_endp = rex.reg_match->endp;
}
if (prog == NULL || line == NULL)
{
emsg(_(e_null));
goto theend;
}
if (prog_magic_wrong())
goto theend;
if (rex.reg_maxcol > 0 && col >= rex.reg_maxcol)
goto theend;
if (prog->regflags & RF_ICASE)
rex.reg_ic = TRUE;
else if (prog->regflags & RF_NOICASE)
rex.reg_ic = FALSE;
if (prog->regflags & RF_ICOMBINE)
rex.reg_icombine = TRUE;
if (prog->regmust != NULL)
{
int c;
if (has_mbyte)
c = (*mb_ptr2char)(prog->regmust);
else
c = *prog->regmust;
s = line + col;
if (!rex.reg_ic && !has_mbyte)
while ((s = vim_strbyte(s, c)) != NULL)
{
if (cstrncmp(s, prog->regmust, &prog->regmlen) == 0)
break; 
++s;
}
else if (!rex.reg_ic || (!enc_utf8 && mb_char2len(c) > 1))
while ((s = vim_strchr(s, c)) != NULL)
{
if (cstrncmp(s, prog->regmust, &prog->regmlen) == 0)
break; 
MB_PTR_ADV(s);
}
else
while ((s = cstrchr(s, c)) != NULL)
{
if (cstrncmp(s, prog->regmust, &prog->regmlen) == 0)
break; 
MB_PTR_ADV(s);
}
if (s == NULL) 
goto theend;
}
rex.line = line;
rex.lnum = 0;
reg_toolong = FALSE;
if (prog->reganch)
{
int c;
if (has_mbyte)
c = (*mb_ptr2char)(rex.line + col);
else
c = rex.line[col];
if (prog->regstart == NUL
|| prog->regstart == c
|| (rex.reg_ic
&& (((enc_utf8 && utf_fold(prog->regstart) == utf_fold(c)))
|| (c < 255 && prog->regstart < 255 &&
MB_TOLOWER(prog->regstart) == MB_TOLOWER(c)))))
retval = regtry(prog, col, tm, timed_out);
else
retval = 0;
}
else
{
#if defined(FEAT_RELTIME)
int tm_count = 0;
#endif
while (!got_int)
{
if (prog->regstart != NUL)
{
if (!rex.reg_ic && !has_mbyte)
s = vim_strbyte(rex.line + col, prog->regstart);
else
s = cstrchr(rex.line + col, prog->regstart);
if (s == NULL)
{
retval = 0;
break;
}
col = (int)(s - rex.line);
}
if (rex.reg_maxcol > 0 && col >= rex.reg_maxcol)
{
retval = 0;
break;
}
retval = regtry(prog, col, tm, timed_out);
if (retval > 0)
break;
if (rex.lnum != 0)
{
rex.lnum = 0;
rex.line = reg_getline((linenr_T)0);
}
if (rex.line[col] == NUL)
break;
if (has_mbyte)
col += (*mb_ptr2len)(rex.line + col);
else
++col;
#if defined(FEAT_RELTIME)
if (tm != NULL && ++tm_count == 20)
{
tm_count = 0;
if (profile_passed_limit(tm))
{
if (timed_out != NULL)
*timed_out = TRUE;
break;
}
}
#endif
}
}
theend:
if (reg_tofreelen > 400)
VIM_CLEAR(reg_tofree);
if (regstack.ga_maxlen > REGSTACK_INITIAL)
ga_clear(&regstack);
if (backpos.ga_maxlen > BACKPOS_INITIAL)
ga_clear(&backpos);
return retval;
}
static int
bt_regexec_nl(
regmatch_T *rmp,
char_u *line, 
colnr_T col, 
int line_lbr)
{
rex.reg_match = rmp;
rex.reg_mmatch = NULL;
rex.reg_maxline = 0;
rex.reg_line_lbr = line_lbr;
rex.reg_buf = curbuf;
rex.reg_win = NULL;
rex.reg_ic = rmp->rm_ic;
rex.reg_icombine = FALSE;
rex.reg_maxcol = 0;
return bt_regexec_both(line, col, NULL, NULL);
}
static long
bt_regexec_multi(
regmmatch_T *rmp,
win_T *win, 
buf_T *buf, 
linenr_T lnum, 
colnr_T col, 
proftime_T *tm, 
int *timed_out) 
{
init_regexec_multi(rmp, win, buf, lnum);
return bt_regexec_both(NULL, col, tm, timed_out);
}
static int
re_num_cmp(long_u val, char_u *scan)
{
long_u n = OPERAND_MIN(scan);
if (OPERAND_CMP(scan) == '>')
return val > n;
if (OPERAND_CMP(scan) == '<')
return val < n;
return val == n;
}
#if defined(BT_REGEXP_DUMP)
static void
regdump(char_u *pattern, bt_regprog_T *r)
{
char_u *s;
int op = EXACTLY; 
char_u *next;
char_u *end = NULL;
FILE *f;
#if defined(BT_REGEXP_LOG)
f = fopen("bt_regexp_log.log", "a");
#else
f = stdout;
#endif
if (f == NULL)
return;
fprintf(f, "-------------------------------------\n\r\nregcomp(%s):\r\n", pattern);
s = r->program + 1;
while (op != END || s <= end)
{
op = OP(s);
fprintf(f, "%2d%s", (int)(s - r->program), regprop(s)); 
next = regnext(s);
if (next == NULL) 
fprintf(f, "(0)");
else
fprintf(f, "(%d)", (int)((s - r->program) + (next - s)));
if (end < next)
end = next;
if (op == BRACE_LIMITS)
{
fprintf(f, " minval %ld, maxval %ld", OPERAND_MIN(s), OPERAND_MAX(s));
s += 8;
}
else if (op == BEHIND || op == NOBEHIND)
{
fprintf(f, " count %ld", OPERAND_MIN(s));
s += 4;
}
else if (op == RE_LNUM || op == RE_COL || op == RE_VCOL)
{
fprintf(f, " count %ld", OPERAND_MIN(s));
s += 5;
}
s += 3;
if (op == ANYOF || op == ANYOF + ADD_NL
|| op == ANYBUT || op == ANYBUT + ADD_NL
|| op == EXACTLY)
{
fprintf(f, "\nxxxxxxxxx\n");
while (*s != NUL)
fprintf(f, "%c", *s++);
fprintf(f, "\nxxxxxxxxx\n");
s++;
}
fprintf(f, "\r\n");
}
if (r->regstart != NUL)
fprintf(f, "start `%s' 0x%x; ", r->regstart < 256
? (char *)transchar(r->regstart)
: "multibyte", r->regstart);
if (r->reganch)
fprintf(f, "anchored; ");
if (r->regmust != NULL)
fprintf(f, "must have \"%s\"", r->regmust);
fprintf(f, "\r\n");
#if defined(BT_REGEXP_LOG)
fclose(f);
#endif
}
#endif 
#if defined(DEBUG)
static char_u *
regprop(char_u *op)
{
char *p;
static char buf[50];
STRCPY(buf, ":");
switch ((int) OP(op))
{
case BOL:
p = "BOL";
break;
case EOL:
p = "EOL";
break;
case RE_BOF:
p = "BOF";
break;
case RE_EOF:
p = "EOF";
break;
case CURSOR:
p = "CURSOR";
break;
case RE_VISUAL:
p = "RE_VISUAL";
break;
case RE_LNUM:
p = "RE_LNUM";
break;
case RE_MARK:
p = "RE_MARK";
break;
case RE_COL:
p = "RE_COL";
break;
case RE_VCOL:
p = "RE_VCOL";
break;
case BOW:
p = "BOW";
break;
case EOW:
p = "EOW";
break;
case ANY:
p = "ANY";
break;
case ANY + ADD_NL:
p = "ANY+NL";
break;
case ANYOF:
p = "ANYOF";
break;
case ANYOF + ADD_NL:
p = "ANYOF+NL";
break;
case ANYBUT:
p = "ANYBUT";
break;
case ANYBUT + ADD_NL:
p = "ANYBUT+NL";
break;
case IDENT:
p = "IDENT";
break;
case IDENT + ADD_NL:
p = "IDENT+NL";
break;
case SIDENT:
p = "SIDENT";
break;
case SIDENT + ADD_NL:
p = "SIDENT+NL";
break;
case KWORD:
p = "KWORD";
break;
case KWORD + ADD_NL:
p = "KWORD+NL";
break;
case SKWORD:
p = "SKWORD";
break;
case SKWORD + ADD_NL:
p = "SKWORD+NL";
break;
case FNAME:
p = "FNAME";
break;
case FNAME + ADD_NL:
p = "FNAME+NL";
break;
case SFNAME:
p = "SFNAME";
break;
case SFNAME + ADD_NL:
p = "SFNAME+NL";
break;
case PRINT:
p = "PRINT";
break;
case PRINT + ADD_NL:
p = "PRINT+NL";
break;
case SPRINT:
p = "SPRINT";
break;
case SPRINT + ADD_NL:
p = "SPRINT+NL";
break;
case WHITE:
p = "WHITE";
break;
case WHITE + ADD_NL:
p = "WHITE+NL";
break;
case NWHITE:
p = "NWHITE";
break;
case NWHITE + ADD_NL:
p = "NWHITE+NL";
break;
case DIGIT:
p = "DIGIT";
break;
case DIGIT + ADD_NL:
p = "DIGIT+NL";
break;
case NDIGIT:
p = "NDIGIT";
break;
case NDIGIT + ADD_NL:
p = "NDIGIT+NL";
break;
case HEX:
p = "HEX";
break;
case HEX + ADD_NL:
p = "HEX+NL";
break;
case NHEX:
p = "NHEX";
break;
case NHEX + ADD_NL:
p = "NHEX+NL";
break;
case OCTAL:
p = "OCTAL";
break;
case OCTAL + ADD_NL:
p = "OCTAL+NL";
break;
case NOCTAL:
p = "NOCTAL";
break;
case NOCTAL + ADD_NL:
p = "NOCTAL+NL";
break;
case WORD:
p = "WORD";
break;
case WORD + ADD_NL:
p = "WORD+NL";
break;
case NWORD:
p = "NWORD";
break;
case NWORD + ADD_NL:
p = "NWORD+NL";
break;
case HEAD:
p = "HEAD";
break;
case HEAD + ADD_NL:
p = "HEAD+NL";
break;
case NHEAD:
p = "NHEAD";
break;
case NHEAD + ADD_NL:
p = "NHEAD+NL";
break;
case ALPHA:
p = "ALPHA";
break;
case ALPHA + ADD_NL:
p = "ALPHA+NL";
break;
case NALPHA:
p = "NALPHA";
break;
case NALPHA + ADD_NL:
p = "NALPHA+NL";
break;
case LOWER:
p = "LOWER";
break;
case LOWER + ADD_NL:
p = "LOWER+NL";
break;
case NLOWER:
p = "NLOWER";
break;
case NLOWER + ADD_NL:
p = "NLOWER+NL";
break;
case UPPER:
p = "UPPER";
break;
case UPPER + ADD_NL:
p = "UPPER+NL";
break;
case NUPPER:
p = "NUPPER";
break;
case NUPPER + ADD_NL:
p = "NUPPER+NL";
break;
case BRANCH:
p = "BRANCH";
break;
case EXACTLY:
p = "EXACTLY";
break;
case NOTHING:
p = "NOTHING";
break;
case BACK:
p = "BACK";
break;
case END:
p = "END";
break;
case MOPEN + 0:
p = "MATCH START";
break;
case MOPEN + 1:
case MOPEN + 2:
case MOPEN + 3:
case MOPEN + 4:
case MOPEN + 5:
case MOPEN + 6:
case MOPEN + 7:
case MOPEN + 8:
case MOPEN + 9:
sprintf(buf + STRLEN(buf), "MOPEN%d", OP(op) - MOPEN);
p = NULL;
break;
case MCLOSE + 0:
p = "MATCH END";
break;
case MCLOSE + 1:
case MCLOSE + 2:
case MCLOSE + 3:
case MCLOSE + 4:
case MCLOSE + 5:
case MCLOSE + 6:
case MCLOSE + 7:
case MCLOSE + 8:
case MCLOSE + 9:
sprintf(buf + STRLEN(buf), "MCLOSE%d", OP(op) - MCLOSE);
p = NULL;
break;
case BACKREF + 1:
case BACKREF + 2:
case BACKREF + 3:
case BACKREF + 4:
case BACKREF + 5:
case BACKREF + 6:
case BACKREF + 7:
case BACKREF + 8:
case BACKREF + 9:
sprintf(buf + STRLEN(buf), "BACKREF%d", OP(op) - BACKREF);
p = NULL;
break;
case NOPEN:
p = "NOPEN";
break;
case NCLOSE:
p = "NCLOSE";
break;
#if defined(FEAT_SYN_HL)
case ZOPEN + 1:
case ZOPEN + 2:
case ZOPEN + 3:
case ZOPEN + 4:
case ZOPEN + 5:
case ZOPEN + 6:
case ZOPEN + 7:
case ZOPEN + 8:
case ZOPEN + 9:
sprintf(buf + STRLEN(buf), "ZOPEN%d", OP(op) - ZOPEN);
p = NULL;
break;
case ZCLOSE + 1:
case ZCLOSE + 2:
case ZCLOSE + 3:
case ZCLOSE + 4:
case ZCLOSE + 5:
case ZCLOSE + 6:
case ZCLOSE + 7:
case ZCLOSE + 8:
case ZCLOSE + 9:
sprintf(buf + STRLEN(buf), "ZCLOSE%d", OP(op) - ZCLOSE);
p = NULL;
break;
case ZREF + 1:
case ZREF + 2:
case ZREF + 3:
case ZREF + 4:
case ZREF + 5:
case ZREF + 6:
case ZREF + 7:
case ZREF + 8:
case ZREF + 9:
sprintf(buf + STRLEN(buf), "ZREF%d", OP(op) - ZREF);
p = NULL;
break;
#endif
case STAR:
p = "STAR";
break;
case PLUS:
p = "PLUS";
break;
case NOMATCH:
p = "NOMATCH";
break;
case MATCH:
p = "MATCH";
break;
case BEHIND:
p = "BEHIND";
break;
case NOBEHIND:
p = "NOBEHIND";
break;
case SUBPAT:
p = "SUBPAT";
break;
case BRACE_LIMITS:
p = "BRACE_LIMITS";
break;
case BRACE_SIMPLE:
p = "BRACE_SIMPLE";
break;
case BRACE_COMPLEX + 0:
case BRACE_COMPLEX + 1:
case BRACE_COMPLEX + 2:
case BRACE_COMPLEX + 3:
case BRACE_COMPLEX + 4:
case BRACE_COMPLEX + 5:
case BRACE_COMPLEX + 6:
case BRACE_COMPLEX + 7:
case BRACE_COMPLEX + 8:
case BRACE_COMPLEX + 9:
sprintf(buf + STRLEN(buf), "BRACE_COMPLEX%d", OP(op) - BRACE_COMPLEX);
p = NULL;
break;
case MULTIBYTECODE:
p = "MULTIBYTECODE";
break;
case NEWL:
p = "NEWL";
break;
default:
sprintf(buf + STRLEN(buf), "corrupt %d", OP(op));
p = NULL;
break;
}
if (p != NULL)
STRCAT(buf, p);
return (char_u *)buf;
}
#endif 
