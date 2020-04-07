




































































#if _MSC_VER >= 1400
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#endif
#if !defined(CYGWIN) && defined(__CYGWIN__)
#define CYGWIN
#endif

#include <stdio.h>
#if defined(VAXC)
#include <file.h>
#else
#include <fcntl.h>
#endif
#if defined(WIN32) || defined(CYGWIN)
#include <io.h> 
#else
#if defined(UNIX)
#include <unistd.h>
#endif
#endif
#include <stdlib.h>
#include <string.h> 
#include <ctype.h> 
#include <limits.h>
#if __MWERKS__ && !defined(BEBOX)
#include <unix.h> 
#endif






#if defined(__GNUC__) && defined(__STDC__)
#if !defined(__USE_FIXED_PROTOTYPES__)
#define __USE_FIXED_PROTOTYPES__
#endif
#endif

#if !defined(__USE_FIXED_PROTOTYPES__)







#if defined(sun) && defined(FILE) && !defined(__SVR4) && defined(__STDC__)
#define __P(a) a

extern int fprintf __P((FILE *, char *, ...));
extern int fputs __P((char *, FILE *));
extern int _flsbuf __P((unsigned char, FILE *));
extern int _filbuf __P((FILE *));
extern int fflush __P((FILE *));
extern int fclose __P((FILE *));
extern int fseek __P((FILE *, long, int));
extern int rewind __P((FILE *));

extern void perror __P((char *));
#endif
#endif

extern long int strtol();
extern long int ftell();

char version[] = "xxd V1.10 27oct98 by Juergen Weigert";
#if defined(WIN32)
char osver[] = " (Win32)";
#else
char osver[] = "";
#endif

#if defined(WIN32)
#define BIN_READ(yes) ((yes) ? "rb" : "rt")
#define BIN_WRITE(yes) ((yes) ? "wb" : "wt")
#define BIN_CREAT(yes) ((yes) ? (O_CREAT|O_BINARY) : O_CREAT)
#define BIN_ASSIGN(fp, yes) setmode(fileno(fp), (yes) ? O_BINARY : O_TEXT)
#define PATH_SEP '\\'
#elif defined(CYGWIN)
#define BIN_READ(yes) ((yes) ? "rb" : "rt")
#define BIN_WRITE(yes) ((yes) ? "wb" : "w")
#define BIN_CREAT(yes) ((yes) ? (O_CREAT|O_BINARY) : O_CREAT)
#define BIN_ASSIGN(fp, yes) ((yes) ? (void) setmode(fileno(fp), O_BINARY) : (void) (fp))
#define PATH_SEP '/'
#else
#if defined(VMS)
#define BIN_READ(dummy) "r"
#define BIN_WRITE(dummy) "w"
#define BIN_CREAT(dummy) O_CREAT
#define BIN_ASSIGN(fp, dummy) fp
#define PATH_SEP ']'
#define FILE_SEP '.'
#else
#define BIN_READ(dummy) "r"
#define BIN_WRITE(dummy) "w"
#define BIN_CREAT(dummy) O_CREAT
#define BIN_ASSIGN(fp, dummy) fp
#define PATH_SEP '/'
#endif
#endif


#if __MWERKS__
#define OPEN(name, mode, umask) open(name, mode)
#else
#define OPEN(name, mode, umask) open(name, mode, umask)
#endif

#if defined(AMIGA)
#define STRNCMP(s1, s2, l) strncmp(s1, s2, (size_t)l)
#else
#define STRNCMP(s1, s2, l) strncmp(s1, s2, l)
#endif

#if !defined(__P)
#if defined(__STDC__) || defined(WIN32)
#define __P(a) a
#else
#define __P(a) ()
#endif
#endif



static void exit_with_usage __P((void));
static void die __P((int));
static int huntype __P((FILE *, FILE *, FILE *, int, int, long));
static void xxdline __P((FILE *, char *, int));

#define TRY_SEEK 
#define COLS 256 
#define LLEN ((2*(int)sizeof(unsigned long)) + 4 + (9*COLS-1) + COLS + 2)

char hexxa[] = "0123456789abcdef0123456789ABCDEF", *hexx = hexxa;


#define HEX_NORMAL 0
#define HEX_POSTSCRIPT 1
#define HEX_CINCLUDE 2
#define HEX_BITS 3 
#define HEX_LITTLEENDIAN 4

#define CONDITIONAL_CAPITALIZE(c) (capitalize ? toupper((int)c) : c)

static char *pname;

static void
exit_with_usage(void)
{
fprintf(stderr, "Usage:\n %s [options] [infile [outfile]]\n", pname);
fprintf(stderr, " or\n %s -r [-s [-]offset] [-c cols] [-ps] [infile [outfile]]\n", pname);
fprintf(stderr, "Options:\n");
fprintf(stderr, " -a toggle autoskip: A single '*' replaces nul-lines. Default off.\n");
fprintf(stderr, " -b binary digit dump (incompatible with -ps,-i,-r). Default hex.\n");
fprintf(stderr, " -C capitalize variable names in C include file style (-i).\n");
fprintf(stderr, " -c cols format <cols> octets per line. Default 16 (-i: 12, -ps: 30).\n");
fprintf(stderr, " -E show characters in EBCDIC. Default ASCII.\n");
fprintf(stderr, " -e little-endian dump (incompatible with -ps,-i,-r).\n");
fprintf(stderr, " -g number of octets per group in normal output. Default 2 (-e: 4).\n");
fprintf(stderr, " -h print this summary.\n");
fprintf(stderr, " -i output in C include file style.\n");
fprintf(stderr, " -l len stop after <len> octets.\n");
fprintf(stderr, " -o off add <off> to the displayed file position.\n");
fprintf(stderr, " -ps output in postscript plain hexdump style.\n");
fprintf(stderr, " -r reverse operation: convert (or patch) hexdump into binary.\n");
fprintf(stderr, " -r -s off revert with <off> added to file positions found in hexdump.\n");
fprintf(stderr, " -s %sseek start at <seek> bytes abs. %sinfile offset.\n",
#if defined(TRY_SEEK)
"[+][-]", "(or +: rel.) ");
#else
"", "");
#endif
fprintf(stderr, " -u use upper case hex letters.\n");
fprintf(stderr, " -v show version: \"%s%s\".\n", version, osver);
exit(1);
}

static void
die(int ret)
{
fprintf(stderr, "%s: ", pname);
perror(NULL);
exit(ret);
}








static int
huntype(
FILE *fpi,
FILE *fpo,
FILE *fperr,
int cols,
int hextype,
long base_off)
{
int c, ign_garb = 1, n1 = -1, n2 = 0, n3, p = cols;
long have_off = 0, want_off = 0;

rewind(fpi);

while ((c = getc(fpi)) != EOF)
{
if (c == '\r') 
continue;




if (hextype == HEX_POSTSCRIPT && (c == ' ' || c == '\n' || c == '\t'))
continue;

n3 = n2;
n2 = n1;

if (c >= '0' && c <= '9')
n1 = c - '0';
else if (c >= 'a' && c <= 'f')
n1 = c - 'a' + 10;
else if (c >= 'A' && c <= 'F')
n1 = c - 'A' + 10;
else
{
n1 = -1;
if (ign_garb)
continue;
}

ign_garb = 0;

if (p >= cols)
{
if (!hextype)
{
if (n1 < 0)
{
p = 0;
continue;
}
want_off = (want_off << 4) | n1;
continue;
}
else
p = 0;
}

if (base_off + want_off != have_off)
{
if (fflush(fpo) != 0)
die(3);
#if defined(TRY_SEEK)
c = fseek(fpo, base_off + want_off - have_off, 1);
if (c >= 0)
have_off = base_off + want_off;
#endif
if (base_off + want_off < have_off)
{
fprintf(fperr, "%s: sorry, cannot seek backwards.\n", pname);
return 5;
}
for (; have_off < base_off + want_off; have_off++)
if (putc(0, fpo) == EOF)
die(3);
}

if (n2 >= 0 && n1 >= 0)
{
if (putc((n2 << 4) | n1, fpo) == EOF)
die(3);
have_off++;
want_off++;
n1 = -1;
if ((++p >= cols) && !hextype)
{

want_off = 0;
while ((c = getc(fpi)) != '\n' && c != EOF)
;
if (c == EOF && ferror(fpi))
die(2);
ign_garb = 1;
}
}
else if (n1 < 0 && n2 < 0 && n3 < 0)
{

if (!hextype)
want_off = 0;
while ((c = getc(fpi)) != '\n' && c != EOF)
;
if (c == EOF && ferror(fpi))
die(2);
ign_garb = 1;
}
}
if (fflush(fpo) != 0)
die(3);
#if defined(TRY_SEEK)
fseek(fpo, 0L, 2);
#endif
if (fclose(fpo) != 0)
die(3);
if (fclose(fpi) != 0)
die(2);
return 0;
}













static void
xxdline(FILE *fp, char *l, int nz)
{
static char z[LLEN+1];
static int zero_seen = 0;

if (!nz && zero_seen == 1)
strcpy(z, l);

if (nz || !zero_seen++)
{
if (nz)
{
if (nz < 0)
zero_seen--;
if (zero_seen == 2)
if (fputs(z, fp) == EOF)
die(3);
if (zero_seen > 2)
if (fputs("*\n", fp) == EOF)
die(3);
}
if (nz >= 0 || zero_seen > 0)
if (fputs(l, fp) == EOF)
die(3);
if (nz)
zero_seen = 0;
}
}



static unsigned char etoa64[] =
{
0040,0240,0241,0242,0243,0244,0245,0246,
0247,0250,0325,0056,0074,0050,0053,0174,
0046,0251,0252,0253,0254,0255,0256,0257,
0260,0261,0041,0044,0052,0051,0073,0176,
0055,0057,0262,0263,0264,0265,0266,0267,
0270,0271,0313,0054,0045,0137,0076,0077,
0272,0273,0274,0275,0276,0277,0300,0301,
0302,0140,0072,0043,0100,0047,0075,0042,
0303,0141,0142,0143,0144,0145,0146,0147,
0150,0151,0304,0305,0306,0307,0310,0311,
0312,0152,0153,0154,0155,0156,0157,0160,
0161,0162,0136,0314,0315,0316,0317,0320,
0321,0345,0163,0164,0165,0166,0167,0170,
0171,0172,0322,0323,0324,0133,0326,0327,
0330,0331,0332,0333,0334,0335,0336,0337,
0340,0341,0342,0343,0344,0135,0346,0347,
0173,0101,0102,0103,0104,0105,0106,0107,
0110,0111,0350,0351,0352,0353,0354,0355,
0175,0112,0113,0114,0115,0116,0117,0120,
0121,0122,0356,0357,0360,0361,0362,0363,
0134,0237,0123,0124,0125,0126,0127,0130,
0131,0132,0364,0365,0366,0367,0370,0371,
0060,0061,0062,0063,0064,0065,0066,0067,
0070,0071,0372,0373,0374,0375,0376,0377
};

int
main(int argc, char *argv[])
{
FILE *fp, *fpo;
int c, e, p = 0, relseek = 1, negseek = 0, revert = 0;
int cols = 0, nonzero = 0, autoskip = 0, hextype = HEX_NORMAL, capitalize = 0;
int ebcdic = 0;
int octspergrp = -1; 
int grplen; 
long length = -1, n = 0, seekoff = 0;
unsigned long displayoff = 0;
static char l[LLEN+1]; 
char *pp;
int addrlen = 9;

#if defined(AMIGA)

if (argc == 0)
exit(1);
#endif

pname = argv[0];
for (pp = pname; *pp; )
if (*pp++ == PATH_SEP)
pname = pp;
#if defined(FILE_SEP)
for (pp = pname; *pp; pp++)
if (*pp == FILE_SEP)
{
*pp = '\0';
break;
}
#endif

while (argc >= 2)
{
pp = argv[1] + (!STRNCMP(argv[1], "--", 2) && argv[1][2]);
if (!STRNCMP(pp, "-a", 2)) autoskip = 1 - autoskip;
else if (!STRNCMP(pp, "-b", 2)) hextype = HEX_BITS;
else if (!STRNCMP(pp, "-e", 2)) hextype = HEX_LITTLEENDIAN;
else if (!STRNCMP(pp, "-u", 2)) hexx = hexxa + 16;
else if (!STRNCMP(pp, "-p", 2)) hextype = HEX_POSTSCRIPT;
else if (!STRNCMP(pp, "-i", 2)) hextype = HEX_CINCLUDE;
else if (!STRNCMP(pp, "-C", 2)) capitalize = 1;
else if (!STRNCMP(pp, "-r", 2)) revert++;
else if (!STRNCMP(pp, "-E", 2)) ebcdic++;
else if (!STRNCMP(pp, "-v", 2))
{
fprintf(stderr, "%s%s\n", version, osver);
exit(0);
}
else if (!STRNCMP(pp, "-c", 2))
{
if (pp[2] && !STRNCMP("apitalize", pp + 2, 9))
capitalize = 1;
else if (pp[2] && STRNCMP("ols", pp + 2, 3))
cols = (int)strtol(pp + 2, NULL, 0);
else
{
if (!argv[2])
exit_with_usage();
cols = (int)strtol(argv[2], NULL, 0);
argv++;
argc--;
}
}
else if (!STRNCMP(pp, "-g", 2))
{
if (pp[2] && STRNCMP("roup", pp + 2, 4))
octspergrp = (int)strtol(pp + 2, NULL, 0);
else
{
if (!argv[2])
exit_with_usage();
octspergrp = (int)strtol(argv[2], NULL, 0);
argv++;
argc--;
}
}
else if (!STRNCMP(pp, "-o", 2))
{
int reloffset = 0;
int negoffset = 0;
if (pp[2] && STRNCMP("ffset", pp + 2, 5))
displayoff = strtoul(pp + 2, NULL, 0);
else
{
if (!argv[2])
exit_with_usage();

if (argv[2][0] == '+')
reloffset++;
if (argv[2][reloffset] == '-')
negoffset++;

if (negoffset)
displayoff = ULONG_MAX - strtoul(argv[2] + reloffset+negoffset, NULL, 0) + 1;
else
displayoff = strtoul(argv[2] + reloffset+negoffset, NULL, 0);

argv++;
argc--;
}
}
else if (!STRNCMP(pp, "-s", 2))
{
relseek = 0;
negseek = 0;
if (pp[2] && STRNCMP("kip", pp+2, 3) && STRNCMP("eek", pp+2, 3))
{
#if defined(TRY_SEEK)
if (pp[2] == '+')
relseek++;
if (pp[2+relseek] == '-')
negseek++;
#endif
seekoff = strtol(pp + 2+relseek+negseek, (char **)NULL, 0);
}
else
{
if (!argv[2])
exit_with_usage();
#if defined(TRY_SEEK)
if (argv[2][0] == '+')
relseek++;
if (argv[2][relseek] == '-')
negseek++;
#endif
seekoff = strtol(argv[2] + relseek+negseek, (char **)NULL, 0);
argv++;
argc--;
}
}
else if (!STRNCMP(pp, "-l", 2))
{
if (pp[2] && STRNCMP("en", pp + 2, 2))
length = strtol(pp + 2, (char **)NULL, 0);
else
{
if (!argv[2])
exit_with_usage();
length = strtol(argv[2], (char **)NULL, 0);
argv++;
argc--;
}
}
else if (!strcmp(pp, "--")) 
{
argv++;
argc--;
break;
}
else if (pp[0] == '-' && pp[1]) 
exit_with_usage();
else
break; 

argv++; 
argc--;
}

if (!cols)
switch (hextype)
{
case HEX_POSTSCRIPT: cols = 30; break;
case HEX_CINCLUDE: cols = 12; break;
case HEX_BITS: cols = 6; break;
case HEX_NORMAL:
case HEX_LITTLEENDIAN:
default: cols = 16; break;
}

if (octspergrp < 0)
switch (hextype)
{
case HEX_BITS: octspergrp = 1; break;
case HEX_NORMAL: octspergrp = 2; break;
case HEX_LITTLEENDIAN: octspergrp = 4; break;
case HEX_POSTSCRIPT:
case HEX_CINCLUDE:
default: octspergrp = 0; break;
}

if (cols < 1 || ((hextype == HEX_NORMAL || hextype == HEX_BITS || hextype == HEX_LITTLEENDIAN)
&& (cols > COLS)))
{
fprintf(stderr, "%s: invalid number of columns (max. %d).\n", pname, COLS);
exit(1);
}

if (octspergrp < 1 || octspergrp > cols)
octspergrp = cols;
else if (hextype == HEX_LITTLEENDIAN && (octspergrp & (octspergrp-1)))
{
fprintf(stderr,
"%s: number of octets per group must be a power of 2 with -e.\n",
pname);
exit(1);
}

if (argc > 3)
exit_with_usage();

if (argc == 1 || (argv[1][0] == '-' && !argv[1][1]))
BIN_ASSIGN(fp = stdin, !revert);
else
{
if ((fp = fopen(argv[1], BIN_READ(!revert))) == NULL)
{
fprintf(stderr,"%s: ", pname);
perror(argv[1]);
return 2;
}
}

if (argc < 3 || (argv[2][0] == '-' && !argv[2][1]))
BIN_ASSIGN(fpo = stdout, revert);
else
{
int fd;
int mode = revert ? O_WRONLY : (O_TRUNC|O_WRONLY);

if (((fd = OPEN(argv[2], mode | BIN_CREAT(revert), 0666)) < 0) ||
(fpo = fdopen(fd, BIN_WRITE(revert))) == NULL)
{
fprintf(stderr, "%s: ", pname);
perror(argv[2]);
return 3;
}
rewind(fpo);
}

if (revert)
{
if (hextype && (hextype != HEX_POSTSCRIPT))
{
fprintf(stderr, "%s: sorry, cannot revert this type of hexdump\n", pname);
return -1;
}
return huntype(fp, fpo, stderr, cols, hextype,
negseek ? -seekoff : seekoff);
}

if (seekoff || negseek || !relseek)
{
#if defined(TRY_SEEK)
if (relseek)
e = fseek(fp, negseek ? -seekoff : seekoff, 1);
else
e = fseek(fp, negseek ? -seekoff : seekoff, negseek ? 2 : 0);
if (e < 0 && negseek)
{
fprintf(stderr, "%s: sorry cannot seek.\n", pname);
return 4;
}
if (e >= 0)
seekoff = ftell(fp);
else
#endif
{
long s = seekoff;

while (s--)
if (getc(fp) == EOF)
{
if (ferror(fp))
{
die(2);
}
else
{
fprintf(stderr, "%s: sorry cannot seek.\n", pname);
return 4;
}
}
}
}

if (hextype == HEX_CINCLUDE)
{
if (fp != stdin)
{
if (fprintf(fpo, "unsigned char %s", isdigit((int)argv[1][0]) ? "__" : "") < 0)
die(3);
for (e = 0; (c = argv[1][e]) != 0; e++)
if (putc(isalnum(c) ? CONDITIONAL_CAPITALIZE(c) : '_', fpo) == EOF)
die(3);
if (fputs("[] = {\n", fpo) == EOF)
die(3);
}

p = 0;
c = 0;
while ((length < 0 || p < length) && (c = getc(fp)) != EOF)
{
if (fprintf(fpo, (hexx == hexxa) ? "%s0x%02x" : "%s0X%02X",
(p % cols) ? ", " : &",\n "[2*!p], c) < 0)
die(3);
p++;
}
if (c == EOF && ferror(fp))
die(2);

if (p && fputs("\n", fpo) == EOF)
die(3);
if (fputs(&"};\n"[3 * (fp == stdin)], fpo) == EOF)
die(3);

if (fp != stdin)
{
if (fprintf(fpo, "unsigned int %s", isdigit((int)argv[1][0]) ? "__" : "") < 0)
die(3);
for (e = 0; (c = argv[1][e]) != 0; e++)
if (putc(isalnum(c) ? CONDITIONAL_CAPITALIZE(c) : '_', fpo) == EOF)
die(3);
if (fprintf(fpo, "_%s = %d;\n", capitalize ? "LEN" : "len", p) < 0)
die(3);
}

if (fclose(fp))
die(2);
if (fclose(fpo))
die(3);
return 0;
}

if (hextype == HEX_POSTSCRIPT)
{
p = cols;
e = 0;
while ((length < 0 || n < length) && (e = getc(fp)) != EOF)
{
if (putc(hexx[(e >> 4) & 0xf], fpo) == EOF
|| putc(hexx[e & 0xf], fpo) == EOF)
die(3);
n++;
if (!--p)
{
if (putc('\n', fpo) == EOF)
die(3);
p = cols;
}
}
if (e == EOF && ferror(fp))
die(2);
if (p < cols)
if (putc('\n', fpo) == EOF)
die(3);
if (fclose(fp))
die(2);
if (fclose(fpo))
die(3);
return 0;
}



if (hextype != HEX_BITS)
grplen = octspergrp + octspergrp + 1; 
else 
grplen = 8 * octspergrp + 1;

e = 0;
while ((length < 0 || n < length) && (e = getc(fp)) != EOF)
{
if (p == 0)
{
addrlen = sprintf(l, "%08lx:",
((unsigned long)(n + seekoff + displayoff)));
for (c = addrlen; c < LLEN; l[c++] = ' ');
}
if (hextype == HEX_NORMAL)
{
l[c = (addrlen + 1 + (grplen * p) / octspergrp)] = hexx[(e >> 4) & 0xf];
l[++c] = hexx[ e & 0xf];
}
else if (hextype == HEX_LITTLEENDIAN)
{
int x = p ^ (octspergrp-1);
l[c = (addrlen + 1 + (grplen * x) / octspergrp)] = hexx[(e >> 4) & 0xf];
l[++c] = hexx[ e & 0xf];
}
else 
{
int i;

c = (addrlen + 1 + (grplen * p) / octspergrp) - 1;
for (i = 7; i >= 0; i--)
l[++c] = (e & (1 << i)) ? '1' : '0';
}
if (e)
nonzero++;
if (ebcdic)
e = (e < 64) ? '.' : etoa64[e-64];

l[addrlen + 3 + (grplen * cols - 1)/octspergrp + p] =
#if defined(__MVS__)
(e >= 64)
#else
(e > 31 && e < 127)
#endif
? e : '.';
n++;
if (++p == cols)
{
l[c = (addrlen + 3 + (grplen * cols - 1)/octspergrp + p)] = '\n'; l[++c] = '\0';
xxdline(fpo, l, autoskip ? nonzero : 1);
nonzero = 0;
p = 0;
}
}
if (e == EOF && ferror(fp))
die(2);
if (p)
{
l[c = (addrlen + 3 + (grplen * cols - 1)/octspergrp + p)] = '\n'; l[++c] = '\0';
xxdline(fpo, l, 1);
}
else if (autoskip)
xxdline(fpo, l, -1); 

if (fclose(fp))
die(2);
if (fclose(fpo))
die(3);
return 0;
}


