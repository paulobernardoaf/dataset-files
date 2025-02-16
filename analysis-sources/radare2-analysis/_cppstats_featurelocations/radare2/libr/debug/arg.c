

#include <r_debug.h>

R_API ut64 r_debug_arg_get (RDebug *dbg, int cctype, int num) {
ut32 n32;
ut64 n64, sp;
char reg[32];

switch (cctype) {
case R_ANAL_CC_TYPE_SYSV:
case R_ANAL_CC_TYPE_FASTCALL:
snprintf (reg, sizeof (reg)-1, "A%d", num);
return r_debug_reg_get (dbg, reg);
case R_ANAL_CC_TYPE_STDCALL:
case R_ANAL_CC_TYPE_PASCAL:
sp = r_debug_reg_get (dbg, "SP");
if (dbg->bits == 64) {
sp += 8; 
sp += 8 * num;
dbg->iob.read_at (dbg->iob.io, sp, (ut8*)&n64, sizeof(ut64));

return (ut64)n64;
} else {
sp += 4; 
sp += 4 * num;
dbg->iob.read_at (dbg->iob.io, sp, (ut8*)&n32, sizeof(ut32));

return (ut64)n32;
}
}
snprintf (reg, sizeof (reg)-1, "A%d", num);
return r_debug_reg_get (dbg, reg);
}

R_API bool r_debug_arg_set (RDebug *dbg, int cctype, int num, ut64 val) {

char reg[32];
switch (cctype) {
case R_ANAL_CC_TYPE_SYSV:
case R_ANAL_CC_TYPE_FASTCALL:
snprintf (reg, 30, "A%d", num);
return r_debug_reg_set (dbg, reg, val);
case R_ANAL_CC_TYPE_STDCALL:
case R_ANAL_CC_TYPE_PASCAL:

break;
}
return false;
}
