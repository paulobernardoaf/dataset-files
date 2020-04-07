#include <r_bp.h>
#include <r_lib.h>
static struct r_bp_arch_t r_bp_plugin_arm_bps[] = {
{ 64, 4, 0, (const ut8*)"\x00\x00\x20\xd4" }, 
{ 64, 4, 1, (const ut8*)"\xd4\x20\x00\x00" }, 
{32, 4, 0, (const ut8*)"\xf0\x01\xf0\xe7" }, 
{32, 4, 1, (const ut8*)"\xe7\xf0\x01\xf0" }, 
#if 0
{ 4, 0, (const ut8*)"\xfe\xde\xff\xe7" }, 
{ 4, 1, (const ut8*)"\xe7\xff\xde\xfe" }, 
{ 4, 0, (const ut8*)"\xf0\x01\xf0\xe7" }, 
{ 4, 1, (const ut8*)"\xe7\xf0\x01\xf0" }, 
#endif
{ 16, 2, 0, (const ut8*)"\x01\xbe" }, 
{ 16, 2, 1, (const ut8*)"\xbe\x01" }, 
{ 16, 2, 0, (const ut8*)"\xfe\xdf" }, 
{ 16, 2, 1, (const ut8*)"\xdf\xfe" }, 
{ 16, 4, 0, (const ut8*)"\xff\xff\xff\xff" }, 
{ 16, 4, 1, (const ut8*)"\xff\xff\xff\xff" }, 
{ 0, 0, 0, NULL }
};
struct r_bp_plugin_t r_bp_plugin_arm = {
.name = "arm",
.arch = "arm",
.nbps = 9,
.bps = r_bp_plugin_arm_bps,
};
#if !defined(R2_PLUGIN_INCORE)
R_API RLibStruct radare_plugin = {
.type = R_LIB_TYPE_BP,
.data = &r_bp_plugin_arm,
.version = R2_VERSION
};
#endif
