

#include <r_lib.h>

#if !defined(OR1K_DISAS_H)
#define OR1K_DISAS_H


#define INSN_OPCODE_MASK (0b111111ULL * 0x4000000)
#define INSN_OPCODE_SHIFT 26


#define INSN_EMPTY_SHIFT 0
#define INSN_EMPTY_MASK 0


#define INSN_N_MASK 0b11111111111111111111111111


#define INSN_D_SHIFT 21

#define INSN_D_MASK (0b11111 * 0x200000)


#define INSN_K_MASK 0b1111111111111111


#define INSN_B_SHIFT 11

#define INSN_B_MASK (0b11111 * 0x800)


#define INSN_A_SHIFT 16

#define INSN_A_MASK (0b11111 * 0x10000)


#define INSN_I_MASK 0b1111111111111111


#define INSN_L_MASK 0b111111


#define INSN_K1_SHIFT 21

#define INSN_K1_MASK (0b11111 * 0x200000)


#define INSN_K2_MASK 0b11111111111

typedef enum insn_type {
INSN_END = 0, 
INSN_INVAL = 0, 
INSN_X, 
INSN_N, 
INSN_DN, 
INSN_K, 
INSN_DK, 
INSN_D, 
INSN_B, 
INSN_AI, 
INSN_DAI, 

INSN_DAK, 

INSN_DAL, 

INSN_KABK, 

INSN_AB, 
INSN_DA, 
INSN_DAB, 

INSN_IABI, 

INSN_SIZE, 
} insn_type_t;

typedef enum {
INSN_OPER_K1, 
INSN_OPER_K2, 
INSN_OPER_A, 
INSN_OPER_B, 
INSN_OPER_N, 
INSN_OPER_K, 
INSN_OPER_D, 
INSN_OPER_I, 
INSN_OPER_L, 
INSN_OPER_SIZE 
} insn_oper_t;

typedef struct {
int oper;
ut32 mask;
ut32 shift;
} insn_oper_descr_t;

typedef struct {
int type;
char *format;
insn_oper_descr_t operands[INSN_OPER_SIZE];
} insn_type_descr_t;

typedef struct {
ut32 opcode;
char *name;
int type;
int opcode_mask;
int insn_type; 
} insn_extra_t;

typedef struct {
ut32 opcode;
char *name;
int type;
int insn_type; 
insn_extra_t *extra;
} insn_t;

extern insn_type_descr_t types[];
extern size_t types_count;

extern insn_extra_t extra_0x5[];
extern insn_extra_t extra_0x6[];
extern insn_extra_t extra_0x8[];
extern insn_extra_t extra_0x2e[];
extern insn_extra_t extra_0x2f[];
extern insn_extra_t extra_0x31[];
extern insn_extra_t extra_0x32[];
extern insn_extra_t extra_0x38[];
extern insn_extra_t extra_0x39[];

extern insn_t or1k_insns[];
extern size_t insns_count;

insn_extra_t *find_extra_descriptor(insn_extra_t *extra_descr, ut32 insn);












ut32 sign_extend(ut32 number, ut32 mask);

static inline ut32 get_operand_mask(insn_type_descr_t *type_descr, insn_oper_t operand) {
return type_descr->operands[operand].mask;
}

static inline ut32 get_operand_shift(insn_type_descr_t *type_descr, insn_oper_t operand) {
return type_descr->operands[operand].shift;
}

static inline ut32 get_operand_value(ut32 insn, insn_type_descr_t *type_descr, insn_oper_t operand) {
return (insn & get_operand_mask(type_descr, operand)) >> get_operand_shift(type_descr, operand);
}

static inline int has_type_descriptor(insn_type_t type) {
return types + types_count > &types[type];
}

static inline int is_type_descriptor_defined(insn_type_t type) {
return types[type].type == type;
}

static inline insn_type_t type_of_opcode(insn_t *descr, insn_extra_t *extra_descr) {
r_return_val_if_fail (descr, INSN_END);

if (extra_descr == NULL) {
return descr->type;
} else {
return extra_descr->type;
}
}

#endif 
