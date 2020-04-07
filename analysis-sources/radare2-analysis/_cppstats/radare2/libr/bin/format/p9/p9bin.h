#include <r_util.h>
struct plan9_exec {
unsigned long magic; 
unsigned long text; 
unsigned long data; 
unsigned long bss; 
unsigned long syms; 
unsigned long entry; 
unsigned long spsz; 
unsigned long pcsz; 
};
#define HDR_MAGIC 0x00008000 
#define _MAGIC(f, b) ((f)|((((4*(b))+0)*(b))+7))
#define A_MAGIC _MAGIC(0, 8) 
#define I_MAGIC _MAGIC(0, 11) 
#define J_MAGIC _MAGIC(0, 12) 
#define K_MAGIC _MAGIC(0, 13) 
#define V_MAGIC _MAGIC(0, 16) 
#define X_MAGIC _MAGIC(0, 17) 
#define M_MAGIC _MAGIC(0, 18) 
#define D_MAGIC _MAGIC(0, 19) 
#define E_MAGIC _MAGIC(0, 20) 
#define Q_MAGIC _MAGIC(0, 21) 
#define N_MAGIC _MAGIC(0, 22) 
#define L_MAGIC _MAGIC(0, 23) 
#define P_MAGIC _MAGIC(0, 24) 
#define U_MAGIC _MAGIC(0, 25) 
#define S_MAGIC _MAGIC(HDR_MAGIC, 26) 
#define T_MAGIC _MAGIC(HDR_MAGIC, 27) 
#define TOS_SIZE 14 
#define HDR_SIZE 0x20
#define STR_ADDR 0x1000 
#define TXT_ADDR HDR_SIZE + ex.text 
#define DAT_ADDR STR_ADDR + PAGE_ALIGN(TXT_ADDR) 
#define p9bin_open(x) fopen(x,"r")
#define p9bin_close(x) fclose(x)
int r_bin_p9_get_arch(RBuffer *b, int *bits, int *big_endian);
