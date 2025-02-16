

#include <r_core.h>


static int magicdepth = 99; 
static RMagic *ck = NULL; 
static char *ofile = NULL;
static int kw_count = 0;

static void r_core_magic_reset(RCore *core) {
kw_count = 0;
}

static int r_core_magic_at(RCore *core, const char *file, ut64 addr, int depth, int v, bool json, int *hits) {
const char *fmt;
char *q, *p;
const char *str;
int found = 0, delta = 0, adelta = 0, ret;
ut64 curoffset = core->offset;
int maxHits = r_config_get_i (core->config, "search.maxhits");
if (maxHits > 0 && *hits >= maxHits) {
return 0;
}
#define NAH 32

if (--depth<0) {
ret = 0;
goto seek_exit;
}
if (addr != core->offset) {
#if 1
if (addr >= core->offset && (addr+NAH) < (core->offset + core->blocksize)) {
delta = addr - core->offset;
} else {
r_core_seek (core, addr, true);
}
#endif
}
if (core->search->align) {
int mod = addr % core->search->align;
if (mod) {
eprintf ("Unaligned search at %d\n", mod);
ret = mod;
goto seek_exit;
}
}
if (((addr&7)==0) && ((addr&(7<<8))==0))
if (!json) { 
eprintf ("0x%08" PFMT64x " [%d matches found]\r", addr, *hits);
}
if (file) {
if (*file == ' ') file++;
if (!*file) file = NULL;
}
if (file && ofile && file != ofile) {
if (strcmp (file, ofile)) {
r_magic_free (ck);
ck = NULL;
}
}
if (!ck) {

r_magic_free (ck);

ck = r_magic_new (0);
if (file) {
free (ofile);
ofile = strdup (file);
if (!r_magic_load (ck, file)) {
eprintf ("failed r_magic_load (\"%s\") %s\n", file, r_magic_error (ck));
ck = NULL;
ret = -1;
goto seek_exit;
}
} else {
const char *magicpath = r_config_get (core->config, "dir.magic");
if (!r_magic_load (ck, magicpath)) {
ck = NULL;
eprintf ("failed r_magic_load (dir.magic) %s\n", r_magic_error (ck));
ret = -1;
goto seek_exit;
}
}
}


if (delta + 2 > core->blocksize) {
eprintf ("EOB\n");
ret = -1;
goto seek_exit;
}
str = r_magic_buffer (ck, core->block+delta, core->blocksize - delta);
if (str) {
const char *cmdhit;
#if USE_LIB_MAGIC
if (!v && (!strcmp (str, "data") || strstr(str, "ASCII") || strstr(str, "ISO") || strstr(str, "no line terminator"))) {
#else
if (!v && (!strcmp (str, "data"))) {
#endif
int mod = core->search->align;
if (mod < 1) {
mod = 1;
}



ret = mod + 1;
goto seek_exit;
}
p = strdup (str);
fmt = p;

for (q=p; *q; q++) {
if (q[0]=='\\' && q[1]=='n') {
*q = '\n';
strcpy (q + 1, q + ((q[2] == ' ')? 3: 2));
}
}
(*hits)++;
cmdhit = r_config_get (core->config, "cmd.hit");
if (cmdhit && *cmdhit) {
r_core_cmd0 (core, cmdhit);
}
{
const char *searchprefix = r_config_get (core->config, "search.prefix");
const char *flag = sdb_fmt ("%s%d_%d", searchprefix, 0, kw_count++);
r_flag_set (core->flags, flag, addr + adelta, 1);
}

if (!json) {
r_cons_printf ("0x%08"PFMT64x" %d %s\n", addr + adelta, magicdepth-depth, p);
} else {
if (found >= 1) {
r_cons_printf (",");
}
r_cons_printf ("{\"offset\":%"PFMT64d ",\"depth\":%d,\"info\":\"%s\"}",
addr + adelta, magicdepth-depth, p);
}
r_cons_clear_line (1);


for (q = p; *q; q++) {
switch (*q) {
case ' ':
fmt = q + 1;
break;
case '@':
{
ut64 addr = 0LL;
*q = 0;
if (!strncmp (q + 1, "0x", 2)) {
sscanf (q + 3, "%"PFMT64x, &addr);
} else {
sscanf (q + 1, "%"PFMT64d, &addr);
}
if (!fmt || !*fmt) {
fmt = file;
}
r_core_magic_at (core, fmt, addr, depth, 1, json, hits);
*q = '@';
}
break;
}
}
free (p);
r_magic_free (ck);
ck = NULL;

found ++;

}
adelta ++;
delta ++;
#if 0
r_magic_free (ck);
ck = NULL;
#endif
{
int mod = core->search->align;
if (mod) {
ret = mod; 
goto seek_exit;
}
}
ret = adelta; 

seek_exit:
r_core_seek (core, curoffset, true);
return ret;
}

static void r_core_magic(RCore *core, const char *file, int v, int json) {
ut64 addr = core->offset;
int hits = 0;
magicdepth = r_config_get_i (core->config, "magic.depth"); 
r_core_magic_at (core, file, addr, magicdepth, v, json, &hits);
if (json) {
r_cons_newline ();
}
if (addr != core->offset) {
r_core_seek (core, addr, true);
}
}
