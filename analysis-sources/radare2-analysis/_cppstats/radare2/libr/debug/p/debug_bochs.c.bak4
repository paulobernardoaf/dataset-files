

#include <r_asm.h>
#include <r_debug.h>
#include <libbochs.h>

static bool bCapturaRegs = true;
static bool bStep = false;
static bool bBreak = false;
static bool bAjusta = true;
static char *saveRegs;
static ut64 ripStop = 0LL;

typedef struct {
libbochs_t desc;
} RIOBochs;

static libbochs_t *desc = NULL;

static bool isBochs(RDebug *dbg) {
RIODesc *d = dbg->iob.io->desc;
if (d && d->plugin && d->plugin->name) {
if (!strcmp ("bochs", d->plugin->name)) {
return true;
}
}
eprintf ("error: the iodesc data is not bochs friendly\n");
return false;
}

static int r_debug_bochs_breakpoint (RBreakpoint *bp, RBreakpointItem *b, bool set) {
char cmd[64];
char num[4];
char addr[19];
char bufcmd[100];
ut64 a;
int n,i,lenRec;

if (!b) {
return false;
}
if (set) {

sprintf (cmd, "lb 0x%x", (ut32)b->addr);
bochs_send_cmd (desc, cmd, true);
bCapturaRegs = true;
} else {







bochs_send_cmd (desc,"blist",true);
lenRec = strlen (desc->data);
a = -1;
n = 0;
if (!strncmp (desc->data, "Num Type", 8)) {
i = 37;
do {
if (desc->data[i + 24] == 'y') {
strncpy(num, &desc->data[i], 3);
num[3] = 0;
strncpy(addr, &desc->data[i + 28], 18);
addr[18] = 0;
n = r_num_get (NULL,num);
a = r_num_get (NULL,addr);

if (a == b->addr) {
break;
}
}
i += 48;
} while (desc->data[i] != '<' && i<lenRec-4);
}
if (a == b->addr) {
snprintf (bufcmd, sizeof (bufcmd), "d %i", n);

bochs_send_cmd (desc, bufcmd, true);
}

}
return true;
}

static int r_debug_bochs_reg_read(RDebug *dbg, int type, ut8 *buf, int size) {
char strReg[19];
char regname[4];
char strBase[19];
char strLimit[19];
int i = 0, pos = 0, lenRec = 0;
ut64 val = 0, valRIP = 0; 
if (!isBochs (dbg)) {
return 0;
}


if (bCapturaRegs == true) {
bochs_send_cmd (desc, "regs", true);




pos = 0x78;
lenRec = strlen (desc->data);
while (desc->data[i] != 0 && i < lenRec -4 ) {
if ( (desc->data[i] == (ut8)'r' && desc->data[i + 3] == (ut8)':')) {
strncpy (regname, &desc->data[i], 3);
regname[3] = 0;
strncpy (&strReg[2], &desc->data[i + 5], 8);
strncpy (&strReg[10], &desc->data[i + 14], 8);
strReg[0]='0';
strReg[1]='x';
strReg[18] = 0;
i += 22;
val = r_num_get (NULL, strReg);

memcpy (&buf[pos], &val, 8);

if (!strncmp (regname, "rip", 3)) {

valRIP = val;
}
pos+= 8;

} else {
i++;
}
}

bochs_send_cmd (desc, "info cpu", true);
if (strstr (desc->data,"PC_32")) {
bAjusta = true;

} else if (strstr (desc->data,"PC_80")) {
bAjusta = false;

} else if (strstr (desc->data,"PC_64")) {
bAjusta = false;

} else {
eprintf ("[unknown mode]\n%s\n", desc->data);
}


















bochs_send_cmd (desc, "sreg", true);

pos = 0x38;
char * s [] = { "es:0x", "cs:0x","ss:0x","ds:0x","fs:0x","gs:0x",0};
const char *x;
int n;
for (n = 0; s[n] != 0; n++) {
if ((x = strstr (desc->data,s[n]))) {
strncpy (&strReg[0], x+3, 7);
strReg[6] = 0;
val = r_num_get (NULL, strReg);
strncpy (regname, s[n], 2);
regname[2] = 0;
if ((x = strstr (x, "base="))) {
strncpy (strBase, x + 5, 10);
strBase[10] = 0;
if ((x = strstr (x, "limit="))) {
strncpy (strLimit, x + 6, 10);
strLimit[10] = 0;
}
}

memcpy (&buf[pos], &val, 2);
pos += 2;
if (bAjusta) {
if (!strncmp (regname,"cs",2)) {
valRIP += (val*0x10); 

}
}
}
}

if (ripStop != 0) {
memcpy (&buf[0], &ripStop, 8);
} else {
memcpy (&buf[0], &valRIP, 8); 
}

memcpy (saveRegs,buf,size);
bCapturaRegs = false;

} else {
memcpy (buf, saveRegs, size);

}
return size;
}

static int r_debug_bochs_reg_write(RDebug *dbg, int type, const ut8 *buf, int size) {

return -1;
}

void map_free(RDebugMap *map) {
free (map->name);
free (map);
}

static RList *r_debug_bochs_map_get(RDebug* dbg) { 
if (!isBochs (dbg)) {
return NULL;
}

RDebugMap *mr;
RList *list = r_list_newf ((RListFree)map_free);
if (!list) {
return NULL;
}
mr = R_NEW0 (RDebugMap);
if (!mr) {
r_list_free (list);
return NULL;
}
mr->name = strdup ("fake");
mr->addr = 0;
mr->addr_end = UT32_MAX;
mr->size = UT32_MAX;
mr->perm = 0;
mr->user = 0;
r_list_append (list, mr);
return list;
}

static int r_debug_bochs_step(RDebug *dbg) {
if (!isBochs (dbg)) {
return false;
}

bochs_send_cmd (desc,"s",true);
bCapturaRegs = true;
bStep = true;
return true;
}

static int r_debug_bochs_continue(RDebug *dbg, int pid, int tid, int sig) {

bochs_send_cmd (desc, "c", false);
bCapturaRegs = true;
bBreak = false;
return true;
}

static void bochs_debug_break(void *u) {
eprintf("bochs_debug_break: Sending break...\n");
bochs_cmd_stop (desc);
bBreak = true;
}

static int r_debug_bochs_wait(RDebug *dbg, int pid) {
if (!isBochs (dbg)) {
return false;
}
char strIP[19];
int i = 0;
const char *x;
char *ini = 0, *fin = 0;



if (bStep) {
bStep = false;
} else {
r_cons_break_push (bochs_debug_break, dbg);
i = 500;
do {
bochs_wait (desc);
if (bBreak) {
if (desc->data[0]) {
eprintf ("ctrl+c %s\n", desc->data);
bBreak = false;
break;
}
i--;
if (!i) {
bBreak = false;
eprintf ("empty ctrl+c.\n");
break;
}
} else if (desc->data[0]) {

break;
}
} while(1);
r_cons_break_pop ();
}



ripStop = 0;
if ((x = strstr (desc->data, "Next at"))) {
if ((ini = strstr (x, "[0x"))) {
if ((fin = strchr (ini, ']'))) {
int len = fin - ini - 1;
strncpy (strIP, ini+1, len);
strIP[len] = 0;

ripStop = r_num_get (NULL, strIP);
}
}
}
desc->data[0] = 0;

return true;
}

static int r_debug_bochs_stop(RDebug *dbg) {




return true;
}

static int r_debug_bochs_attach(RDebug *dbg, int pid) {
RIODesc *d = dbg->iob.io->desc;

dbg->swstep = false;
if (d && d->plugin && d->plugin->name && d->data) {
if (!strcmp ("bochs", d->plugin->name)) {
RIOBochs *g = d->data;


if (( desc = &g->desc )) {
eprintf("bochs attach: ok\n");
saveRegs = malloc(1024);
bCapturaRegs = true;
bStep = false;
bBreak = false;
}
}
}
return true;
}

static int r_debug_bochs_detach(RDebug *dbg, int pid) {

free (saveRegs);
return true;
}

static const char *r_debug_bochs_reg_profile(RDebug *dbg) {
int bits = dbg->anal->bits;

if (bits == 16 || bits == 32 || bits == 64) {
return strdup (
"=PC csip\n"
"=SP rsp\n"
"=BP rbp\n"
"=A0 rax\n"
"=A1 rbx\n"
"=A2 rcx\n"
"=A3 rdi\n"

"seg es 2 0x038 0 \n"
"seg cs 2 0x03A 0 \n"
"seg ss 2 0x03C 0 \n"
"seg ds 2 0x03E 0 \n"
"seg fs 2 0x040 0 \n"
"seg gs 2 0x042 0 \n"

"gpr rax 8 0x078 0 \n"
"gpr eax 4 0x078 0 \n"
"gpr ax 2 0x078 0 \n"
"gpr al 1 0x078 0 \n"
"gpr rcx 8 0x080 0 \n"
"gpr ecx 4 0x080 0 \n"
"gpr cx 2 0x080 0 \n"
"gpr cl 1 0x078 0 \n"
"gpr rdx 8 0x088 0 \n"
"gpr edx 4 0x088 0 \n"
"gpr dx 2 0x088 0 \n"
"gpr dl 1 0x088 0 \n"
"gpr rbx 8 0x090 0 \n"
"gpr ebx 4 0x090 0 \n"
"gpr bx 2 0x090 0 \n"
"gpr bl 1 0x090 0 \n"
"gpr rsp 8 0x098 0 \n"
"gpr esp 4 0x098 0 \n"
"gpr sp 2 0x098 0 \n"
"gpr spl 1 0x098 0 \n"
"gpr rbp 8 0x0A0 0 \n"
"gpr ebp 4 0x0A0 0 \n"
"gpr bp 2 0x0A0 0 \n"
"gpr bpl 1 0x0A0 0 \n"
"gpr rsi 8 0x0A8 0 \n"
"gpr esi 4 0x0A8 0 \n"
"gpr si 2 0x0A8 0 \n"
"gpr sil 1 0x0A8 0 \n"
"gpr rdi 8 0x0B0 0 \n"
"gpr edi 4 0x0B0 0 \n"
"gpr di 2 0x0B0 0 \n"
"gpr dil 1 0x0B0 0 \n"
"gpr r8 8 0x0B8 0 \n"
"gpr r8d 4 0x0B8 0 \n"
"gpr r8w 2 0x0B8 0 \n"
"gpr r8b 1 0x0B8 0 \n"
"gpr r9 8 0x0C0 0 \n"
"gpr r9d 4 0x0C0 0 \n"
"gpr r9w 2 0x0C0 0 \n"
"gpr r9b 1 0x0C0 0 \n"
"gpr r10 8 0x0C8 0 \n"
"gpr r10d 4 0x0C8 0 \n"
"gpr r10w 2 0x0C8 0 \n"
"gpr r10b 1 0x0C8 0 \n"
"gpr r11 8 0x0D0 0 \n"
"gpr r11d 4 0x0D0 0 \n"
"gpr r11w 2 0x0D0 0 \n"
"gpr r11b 1 0x0D0 0 \n"
"gpr r12 8 0x0D8 0 \n"
"gpr r12d 4 0x0D8 0 \n"
"gpr r12w 2 0x0D8 0 \n"
"gpr r12b 1 0x0D8 0 \n"
"gpr r13 8 0x0E0 0 \n"
"gpr r13d 4 0x0E0 0 \n"
"gpr r13w 2 0x0E0 0 \n"
"gpr r13b 1 0x0E0 0 \n"
"gpr r14 8 0x0E8 0 \n"
"gpr r14d 4 0x0E8 0 \n"
"gpr r14w 2 0x0E8 0 \n"
"gpr r14b 1 0x0E8 0 \n"
"gpr r15 8 0x0F0 0 \n"
"gpr r15d 4 0x0F0 0 \n"
"gpr r15w 2 0x0F0 0 \n"
"gpr r15b 1 0x0F0 0 \n"
"gpr rip 8 0x0F8 0 \n"
"gpr eip 4 0x0F8 0 \n"
"gpr csip 8 0x000 0 \n"


























































































);
}
return NULL;
}

RDebugPlugin r_debug_plugin_bochs = {
.name = "bochs",
.license = "LGPL3",
.arch = "x86",
.bits = R_SYS_BITS_16 | R_SYS_BITS_32 | R_SYS_BITS_64,
.step = r_debug_bochs_step,
.cont = r_debug_bochs_continue,
.attach = &r_debug_bochs_attach,
.detach = &r_debug_bochs_detach,
.canstep = 1,
.stop = &r_debug_bochs_stop,
.wait = &r_debug_bochs_wait,
.map_get = r_debug_bochs_map_get,
.breakpoint = r_debug_bochs_breakpoint,
.reg_read = &r_debug_bochs_reg_read,
.reg_write = &r_debug_bochs_reg_write,
.reg_profile = (void *)r_debug_bochs_reg_profile,
};

#if !defined(R2_PLUGIN_INCORE)
R_API RLibStruct radare_plugin = {
.type = R_LIB_TYPE_DBG,
.data = &r_debug_plugin_bochs,
.version = R2_VERSION
};
#endif
