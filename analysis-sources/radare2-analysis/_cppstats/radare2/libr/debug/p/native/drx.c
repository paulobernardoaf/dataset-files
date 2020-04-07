#include <r_types.h>
#define DRXN 8
#define DR_STATUS 6
#define DR_CONTROL 7
#define DR_LOCAL_ENABLE_SHIFT 0 
#define DR_GLOBAL_ENABLE_SHIFT 1 
#define DR_ENABLE_SIZE 2 
#define DR_CONTROL_RESERVED (0xFC00)
#define I386_DR_CONTROL_MASK (~DR_CONTROL_RESERVED)
#define DR_LOCAL_SLOWDOWN (0x100)
#define DR_GLOBAL_SLOWDOWN (0x200)
#define DR_CONTROL_SHIFT 16
#define DR_CONTROL_SIZE 4
#define DR_RW_EXECUTE (0x0) 
#define DR_RW_WRITE (0x1) 
#define DR_RW_IORW (0x2) 
#define DR_RW_READ (0x3) 
#define DR_NADDR 4 
#define DR_STATUS 6 
#define DR_CONTROL 7 
#define drxt size_t
#define DR_LEN_1 (0 << 2) 
#define DR_LEN_2 (1 << 2) 
#define DR_LEN_4 (3 << 2) 
#define DR_LEN_8 (2 << 2) 
#define I386_DR_CONTROL_MASK (~DR_CONTROL_RESERVED)
#define I386_DR_VACANT(control, i)((control & (3 << (DR_ENABLE_SIZE * (i)))) == 0)
#define I386_DR_LOCAL_ENABLE(control, i)control |= (1 << (DR_LOCAL_ENABLE_SHIFT + DR_ENABLE_SIZE * (i)))
#define I386_DR_GLOBAL_ENABLE(control, i)control |= (1 << (DR_GLOBAL_ENABLE_SHIFT + DR_ENABLE_SIZE * (i)))
#define I386_DR_IS_LOCAL_ENABLED(control, i)(control & (1 << (DR_LOCAL_ENABLE_SHIFT + DR_ENABLE_SIZE * (i))))
#define I386_DR_IS_ENABLED(control, i)control & (3 << (DR_ENABLE_SIZE * (i)))
#define I386_DR_ENABLE(control, i)control |= (3 << (DR_ENABLE_SIZE * (i)))
#define I386_DR_DISABLE(control, i)control &= ~(3 << (DR_ENABLE_SIZE * (i)))
#define I386_DR_SET_RW_LEN(control, i, rwlen) do { control &= ~(0x0f << (DR_CONTROL_SHIFT + DR_CONTROL_SIZE * (i)));control |= ((rwlen) << (DR_CONTROL_SHIFT + DR_CONTROL_SIZE * (i)));} while (0)
#define I386_DR_GET_RW_LEN(control, i)((control >> (DR_CONTROL_SHIFT + DR_CONTROL_SIZE * (i))) & 0x0f)
int drx_set(drxt *drx, int n, ut64 addr, int len, int rwx, int global) {
ut32 control = drx[DR_CONTROL];
if (n < 0 || n >= DR_NADDR) {
eprintf ("Invalid DRX index (0-%d)\n", DR_NADDR - 1);
return false;
}
switch (rwx) {
case 1: rwx = 0; break;
case 2: rwx = 1; break;
case 4: rwx = 3; break;
default:
rwx = 0;
}
switch (len) {
case 1: len = 0; break;
case 2: len = 1 << 2; break;
case 4: len = 3 << 2; break;
case 8: len = 2 << 2; break; 
case -1:
{
I386_DR_DISABLE (control, n);
control &= I386_DR_CONTROL_MASK;
drx[DR_CONTROL] = control;
drx[n] = 0;
return true;
}
default:
eprintf ("Invalid DRX length (%d) must be 1, 2, 4, 8 bytes\n", len);
return false;
}
I386_DR_SET_RW_LEN (control, n, len | rwx);
if (global) {
I386_DR_GLOBAL_ENABLE (control, n);
} else {
I386_DR_LOCAL_ENABLE (control, n);
}
control &= I386_DR_CONTROL_MASK;
drx[n] = addr;
drx[DR_CONTROL] = control;
return true;
}
ut64 drx_get(drxt *drx, int n, int *rwx, int *len, int *global, int *enabled) {
int ret = I386_DR_GET_RW_LEN (drx[DR_CONTROL], n);
if (global) {
*global = I386_DR_IS_LOCAL_ENABLED (drx[7], n);
}
if (len) {
switch (ret & 0xC) {
case DR_LEN_1: *len = 1; break;
case DR_LEN_2: *len = 2; break;
case DR_LEN_4: *len = 4; break;
case DR_LEN_8: *len = 8; break;
default: *len = 0; break;
}
}
if (enabled) {
*enabled = I386_DR_IS_ENABLED (drx[7], n);
}
if (rwx) {
*rwx = ret & 0x3;
}
return (ut64) drx[n];
}
int drx_next(drxt *drx) {
int i;
for (i = 0; i < 4; i++) {
if (!drx[i]) {
return i;
}
}
return -1;
}
int drx_get_at(drxt *drx, ut64 at_addr) {
ut64 addr;
int i, rwx, len, g, en;
for (i = 0; i < 8; i++) {
if (i == 4 || i == 5) {
continue;
}
rwx = len = g = en = 0;
addr = drx_get (drx, i, &rwx, &len, &g, &en);
if (addr == at_addr) {
return i;
}
}
return -1;
}
void drx_list(drxt *drx) {
ut64 addr;
int i, rwx, len, g, en;
for (i = 0; i < 8; i++) {
if (i == 4 || i == 5) {
continue;
}
rwx = len = g = en = 0;
addr = drx_get (drx, i, &rwx, &len, &g, &en);
printf ("%c dr%d %c%c 0x%08"PFMT64x " %d\n",
en? '*': '-', i, g? 'G': 'L',
(rwx == DR_RW_READ)? 'r':
(rwx == DR_RW_WRITE)? 'w':
(rwx == DR_RW_EXECUTE)? 'x':
(rwx == DR_RW_IORW)? 'i': '?',
addr, len);
}
}
void drx_init(drxt *r) {
memset (r, 0, sizeof (drxt) * (DRXN + 1));
}
void drx_enable(drxt *r, int n, int enabled) {
if (enabled) {
I386_DR_ENABLE (r[DR_CONTROL], n);
} else {
I386_DR_DISABLE (r[DR_CONTROL], n);
}
}
bool drx_add(RDebug *dbg, RBreakpoint *bp, RBreakpointItem *b) {
if (bp->nhwbps < 4) {
r_debug_reg_sync (dbg, R_REG_TYPE_DRX, false);
r_debug_drx_set (dbg, bp->nhwbps, b->addr, b->size, b->perm, 0);
r_debug_reg_sync (dbg, R_REG_TYPE_DRX, true);
bp->nhwbps++;
return true;
}
return false;
}
bool drx_del(RDebug *dbg, RBreakpoint *bp, RBreakpointItem *b) {
if (bp->nhwbps > 0) {
r_debug_reg_sync (dbg, R_REG_TYPE_DRX, false);
r_debug_drx_unset (dbg, bp->nhwbps - 1);
r_debug_reg_sync (dbg, R_REG_TYPE_DRX, true);
bp->nhwbps--;
}
return true;
}
#if MAIN
int main() {
drxt regs[DRXN + 1];
drx_init (regs);
drx_set (regs, 1, 0x8048123, 1, DR_RW_EXECUTE, 0);
drx_set (regs, 0, 0x8048123, 4, DR_RW_READ, 1);
drx_list (regs);
}
#endif
