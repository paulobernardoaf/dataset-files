













































































#include "radare.h"

#if __FreeBSD__ || __linux__ || __NetBSD__ || __OpenBSD__
#define __UNIX__ 1
#else
#define __UNIX__ 0
#endif

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <getopt.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#if __UNIX__
#include <sys/mman.h>
#include <sys/ioctl.h>
#endif

ut32 base = 0;
ut32 delta = 0;
ut32 range = 0;
ut32 xylum = 0;
ut32 gamme = 0;
ut32 size = 4;
int sysendian = 0; 
int endian = -1; 
int verbose = 0;
int found = 0;
int quite = 0;
int arch = ARCH_NULL;
unsigned char *ma = NULL;

static int show_usage()
{
printf(
"Usage: xrefs [-options] [file] [offset]\n"
" -v Verbose mode\n"
" -V Show version\n"
" -q quite mode\n"
" -h Show this helpy message\n"
" -e Use big endian for offsets to search\n"
" -a [arch] Architecture profile (fmi: help) (autodetects ELF and PE hdrs)\n"
" -b [address] base address (0x8048000 f.ex)\n"
" -f [from] start scanning from this offset (default 0)\n"
" -t [to] limit address (default 99999999)\n"
" -r [range] Range in bytes of allowed relative offsets\n"
" -s [size] Size of offset (4 for intel, 3 for ppc, ...)\n"
" -d [offset] Sets a negative delta offset as padding (default 1)\n"
" -X [offset] Print out debugging information of a certain relative offset\n");

return 1;
}

static ut32 file_size_fd(int fd)
{
ut32 curr = lseek(fd, 0, SEEK_CUR);
ut32 size = lseek(fd, 0, SEEK_END); 
lseek(fd, curr, SEEK_SET);

return size;
}


static ut32 get_value32(const char *arg)
{
int i;
ut32 ret;

for(i=0;arg[i]==' ';i++);
for(;arg[i]=='\\';i++); i++;

if (arg[i] == 'x')
sscanf(arg, "0x%08"PFMT64x"", (ut64 *)&ret);
else
sscanf(arg, "%"PFMT64d"", (ut64 *)&ret);

return ret;
}

int get_system_endian()
{
int a = 1;
char *b = (char*)&a;
return (int)(b[0]);
}

int set_arch_settings()
{
switch(arch) {
case ARCH_PPC:
gamme = 1;
delta = 1;
size = 3;
break;
case ARCH_ARM:
gamme = -1;
delta = 1;
size = 3;
break;
case ARCH_X86:
gamme = 1;
delta = 0; 
size = 4;
break;
case ARCH_NULL:


if (!memcmp(ma, "\x7f\x45\x4c\x46", 4)) {
short ar = (ma[0x12]<<8) + ma[0x13];
switch(ar) {
case 0x0300:
if (endian==-1)
endian = 1;
if (!quite)
printf("#-a x86\n");
arch = ARCH_X86;
endian = 1;
return 1;
case 0x0014:
if (endian==-1)
endian = 0;
if (!quite)
printf("#-a ppc\n");
arch = ARCH_PPC;
return 1;
case 0x2800:
if (endian==-1)
endian = 1;
if (!quite)
printf("#-a arm\n");
arch = ARCH_ARM;
return 1;
default:
printf("Unsupported architecture '%04x'.\n", ar);
exit(1);
}
} else

if (!memcmp(ma, "\x4d\x5a",2)) {
unsigned short off = ma[0x3c];
if (!memcmp(ma+off, "PE\x00\x00",4)) {
unsigned short ar = (ma[off+4]<<8)+ma[off+5];
switch(ar) {
case 0x4c01: 
if (endian==-1)
endian = 1;
printf("#-a x86\n");
arch = ARCH_X86;
endian = 1;
return 1;
case 0xc001: 
if (endian==-1)
endian = 1;
printf("#-a arm\n");
arch = ARCH_ARM;
endian = 1;
return 1;
default:
fprintf(stderr, "Unknown architecture.\n");
break;
}
}
} else {
fprintf(stderr, "Plz. gimmie an architecture. (xrefs -a [arch])\n");
exit(1);
}
}

return 0;
}


int main(int argc, char **argv)
{
ut64 i, c, src;
ut64 offset = 0;
ut64 from = 1,
to = INT_MAX;
ut64 sa;

if (argc==2)
if (!strcmp(argv[1],"-V")) {
printf("%s\n", VERSION);
return 0;
}

if (argc<3)
return show_usage();


while ((c = getopt(argc, argv, "qa:d:hves:f:t:r:X:b:")) != -1) {
switch( c ) {
case 'q':
quite = 1;
break;
case 'a':
if (!strcmp(optarg, "intel"))
arch = ARCH_X86;
else
if (!strcmp(optarg, "x86"))
arch = ARCH_X86;
else
if (!strcmp(optarg, "arm"))
arch = ARCH_ARM;
else
if (!strcmp(optarg, "ppc")) {
arch = ARCH_PPC;
} else {
printf("arm ppc x86\n");
return 1;
}
break;
case 'b':
base = get_value32(optarg);
break;
case 'd':
delta = get_value32(optarg);
break;
case 'X':
xylum = get_value32(optarg);
break;
case 'e':
endian = 1;
break;
case 'r':
range = get_value32(optarg);
if (range<0) range = -range;
break;
case 'v':
verbose = 1;
break;
case 'f':
from = get_value32(optarg);
break;
case 't':
to = get_value32(optarg);
break;
case 's':
size = get_value32(optarg);
break;
case 'h':
return show_usage();
}
}

if (optind+2 != argc) {
fprintf(stderr, "Plz. gimmie a file and offset.\n");
return 1;
}


src = open(argv[optind], O_RDONLY);
if (src == -1) {
fprintf(stderr, "Cannot open file source %s\n", argv[optind]);
return -1;
}

offset = get_value32(argv[optind+1]);
if (offset >= base)
offset -= base;

sa = file_size_fd(src) - size;
#if __UNIX__
ma = mmap(NULL, sa, PROT_READ, MAP_SHARED, src, 0);
if (sa < 0x50) {
fprintf(stderr, "Minimum length is 0x50 bytes.\n");
return 1;
}
#elif __WINDOWS__
fprintf(stderr, "Not yet implemented\n");
#else
fprintf(stderr, "No MMAP for this platform? report it!\n");
#endif
if (!ma) {
perror("Error mmaping");
fprintf(stderr, "cannot open %s\n", argv[optind]);
return 1;
}


sysendian = get_system_endian();

while( set_arch_settings() );

if (endian == -1)
endian = 0;


for(i=from; i<sa && i<to; i++) {
ut32 value = offset - i + delta;
ut32 ovalue = value;
ut32 tmpvalue = 0;
unsigned char *buf = (unsigned char *)&value;

if (range!=0) {
if (value<0 && -value>range)
continue;
else
if (value>0 && value>range)
continue;
}

if (verbose)
printf("0x%08"PFMT64x" try %02x %02x %02x %02x (0x%08"PFMT64x") - %"PFMT64d"\n",
(ut64)i, buf[0], buf[1], buf[2], buf[3], (ut64) base+value, (ut64) (base+value));

if (xylum && i == xylum) {
printf("#offset: 0x%08"PFMT64x"\n", (ut64)i);
printf("#delta: %"PFMT64d"\n", (ut64)delta);
printf("#size: %"PFMT64d"\n", (ut64)size);
printf("#value: %"PFMT64d"\n", (ut64)value);
printf("#bytes: %02x %02x %02x %02x (0x%08"PFMT64x") - %"PFMT64d"\n",
buf[0], buf[1], buf[2], buf[3], (ut64)value, (ut64)value);
tmpvalue = ma[i+gamme];
printf("#found: %02x %02x %02x %02x\n",
ma[i+gamme+0], ma[i+gamme+1],
ma[i+gamme+2], ma[i+gamme+3]);
}

switch(arch) {
case ARCH_ARM:
value = (value-8)/4;
break;
case ARCH_X86:
value-=5;
break;
default:
break;
}


if (sysendian) {
unsigned char tmp;
tmp = buf[0]; buf[0]= buf[3]; buf[3] = tmp;
tmp = buf[1]; buf[1]= buf[2]; buf[2] = tmp;
}

if (endian) {
unsigned char tmp;
tmp = buf[0]; buf[0] = buf[3]; buf[3] = tmp;
tmp = buf[1]; buf[1] = buf[2]; buf[2] = tmp;
}
if (arch==ARCH_ARM) {
buf[3] = buf[2]; buf[2] = buf[1]; buf[1] = buf[0];
}

if (xylum && ovalue == xylum) {
printf("#buf: %02x %02x %02x %02x (+%"PFMT64d")\n",
buf[0], buf[1], buf[2], buf[3], (ut64)(4-size));
printf("#map: %02x %02x %02x \n",
ma[i+gamme], ma[i+1+gamme], ma[i+2+gamme]);
printf("#cmp: %02x %02x %02x\n", ma[i], ma[i+1], ma[i+2]);
}

if (xylum && i == xylum) {
printf("#a: %02x %02x %02x %02x\n",
ma[i+gamme+0], ma[i+gamme+1],
ma[i+gamme+2], ma[i+gamme+3]);
printf("#b: %02x %02x %02x %02x\n",
buf[0], buf[1], buf[2], buf[3]);
}

if (memcmp((unsigned char *)ma+i+gamme, (unsigned char *)buf+(4-size), size) == 0) {
if (quite)
printf("0x%08"PFMT64x"\n", (ut64)i);
else
printf("match value 0x%08"PFMT64x" (%02x%02x%02x) at offset 0x%08"PFMT64x"\n",
(ut64)ovalue,
buf[0+(4-size)], buf[1+(4-size)], buf[2+(4-size)],
(ut64)((ut32)i)+((gamme<0)?-1:0));
found++;
}
}

if (found == 0 && !quite)
puts("no matches found.");

return 0;
}
