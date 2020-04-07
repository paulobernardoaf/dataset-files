





















#include "ansidecl.h"
















enum sparc_opcode_arch_val
{
SPARC_OPCODE_ARCH_V6 = 0,
SPARC_OPCODE_ARCH_V7,
SPARC_OPCODE_ARCH_V8,
SPARC_OPCODE_ARCH_SPARCLET,
SPARC_OPCODE_ARCH_SPARCLITE,

SPARC_OPCODE_ARCH_V9,
SPARC_OPCODE_ARCH_V9A, 
SPARC_OPCODE_ARCH_V9B, 
SPARC_OPCODE_ARCH_BAD 
};


#define SPARC_OPCODE_ARCH_MAX (SPARC_OPCODE_ARCH_BAD - 1)



#define SPARC_OPCODE_ARCH_MASK(arch) (1 << (arch))


#define SPARC_OPCODE_ARCH_V9_P(arch) ((arch) >= SPARC_OPCODE_ARCH_V9)



typedef struct sparc_opcode_arch
{
const char *name;




short supported;
} sparc_opcode_arch;

extern const struct sparc_opcode_arch sparc_opcode_archs[];


extern enum sparc_opcode_arch_val sparc_opcode_lookup_arch (const char *);


#define SPARC_OPCODE_SUPPORTED(ARCH) (sparc_opcode_archs[ARCH].supported)



#define SPARC_OPCODE_CONFLICT_P(ARCH1, ARCH2) (((SPARC_OPCODE_SUPPORTED (ARCH1) & SPARC_OPCODE_SUPPORTED (ARCH2)) != SPARC_OPCODE_SUPPORTED (ARCH1)) && ((SPARC_OPCODE_SUPPORTED (ARCH1) & SPARC_OPCODE_SUPPORTED (ARCH2)) != SPARC_OPCODE_SUPPORTED (ARCH2)))







typedef struct sparc_opcode
{
const char *name;
unsigned long match; 
unsigned long lose; 
const char *args;

char flags;
short architecture; 
} sparc_opcode;

#define F_DELAYED 1 
#define F_ALIAS 2 
#define F_UNBR 4 
#define F_CONDBR 8 
#define F_JSR 16 
#define F_FLOAT 32 
#define F_FBR 64 


















































































#define OP2(x) (((x) & 0x7) << 22) 
#define OP3(x) (((x) & 0x3f) << 19) 
#define OP(x) ((unsigned) ((x) & 0x3) << 30) 
#define OPF(x) (((x) & 0x1ff) << 5) 
#define OPF_LOW5(x) OPF ((x) & 0x1f) 
#define F3F(x, y, z) (OP (x) | OP3 (y) | OPF (z)) 
#define F3I(x) (((x) & 0x1) << 13) 
#define F2(x, y) (OP (x) | OP2(y)) 
#define F3(x, y, z) (OP (x) | OP3(y) | F3I(z)) 
#define F1(x) (OP (x))
#define DISP30(x) ((x) & 0x3fffffff)
#define ASI(x) (((x) & 0xff) << 5) 
#define RS2(x) ((x) & 0x1f) 
#define SIMM13(x) ((x) & 0x1fff) 
#define RD(x) (((x) & 0x1f) << 25) 
#define RS1(x) (((x) & 0x1f) << 14) 
#define ASI_RS2(x) (SIMM13 (x))
#define MEMBAR(x) ((x) & 0x7f)
#define SLCPOP(x) (((x) & 0x7f) << 6) 

#define ANNUL (1 << 29)
#define BPRED (1 << 19) 
#define IMMED F3I (1)
#define RD_G0 RD (~0)
#define RS1_G0 RS1 (~0)
#define RS2_G0 RS2 (~0)

extern const struct sparc_opcode sparc_opcodes[];
extern const int sparc_num_opcodes;

extern int sparc_encode_asi (const char *);
extern const char *sparc_decode_asi (int);
extern int sparc_encode_membar (const char *);
extern const char *sparc_decode_membar (int);
extern int sparc_encode_prefetch (const char *);
extern const char *sparc_decode_prefetch (int);
extern int sparc_encode_sparclet_cpreg (const char *);
extern const char *sparc_decode_sparclet_cpreg (int);






