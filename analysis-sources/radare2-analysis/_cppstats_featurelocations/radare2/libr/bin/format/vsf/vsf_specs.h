
#if !defined(VSF_SPECS_H)
#define VSF_SPECS_H

#include <r_types_base.h>
#include <r_util.h>
#include <r_lib.h>
#include <r_bin.h>



R_PACKED (
struct vsf_hdr {
char id[19]; 
char major;
char minor;
char machine[16]; 
});

R_PACKED (
struct vsf_module {
char module_name[16]; 
char major;
char minor;
ut32 length; 
});

R_PACKED (
struct vsf_maincpu {
ut32 clk; 
ut8 ac; 
ut8 xr; 
ut8 yr; 
ut8 sp; 
ut16 pc; 
ut8 st; 
ut32 lastopcode; 
ut32 ba_low_flags; 
});

R_PACKED (
struct vsf_c64mem {
ut8 cpudata; 
ut8 cpudir; 
ut8 exrom; 
ut8 game; 
ut8 ram[1024 * 64]; 
});

R_PACKED (
struct vsf_c64rom {
ut8 kernal[1024 * 8]; 
ut8 basic[1024 * 8]; 
ut8 chargen[1024 * 4]; 
});

R_PACKED (
struct vsf_c128mem {
ut8 mmu[12]; 
ut8 ram[1024 * 128]; 
});

R_PACKED (
struct vsf_c128rom {
ut8 kernal[1024 * 8]; 
ut8 basic[1024 * 32]; 
ut8 editor[1024 * 4]; 
ut8 chargen[1024 * 4]; 
});


struct r_bin_vsf_obj {
int machine_idx; 
ut64 rom; 
ut64 mem; 
struct vsf_maincpu* maincpu;
Sdb* kv;
};

#endif 

