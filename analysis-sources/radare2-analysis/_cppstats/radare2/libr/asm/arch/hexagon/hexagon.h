typedef enum {
HEX_NOPRED, 
HEX_PRED_TRUE, 
HEX_PRED_FALSE, 
HEX_PRED_TRUE_NEW, 
HEX_PRED_FALSE_NEW, 
} HexPred;
typedef enum {
HEX_PF_RND = 1, 
HEX_PF_CRND = 1<<1, 
HEX_PF_RAW = 1<<2, 
HEX_PF_CHOP = 1<<3, 
HEX_PF_SAT = 1<<4, 
HEX_PF_HI = 1<<5, 
HEX_PF_LO = 1<<6, 
HEX_PF_LSH1 = 1<<7, 
HEX_PF_LSH16 = 1<<8, 
HEX_PF_RSH1 = 1<<9, 
HEX_PF_NEG = 1<<10, 
HEX_PF_POS = 1<<11, 
HEX_PF_SCALE = 1<<12, 
HEX_PF_DEPRECATED = 1<<15, 
} HexPf;
typedef enum {
HEX_OP_TYPE_IMM,
HEX_OP_TYPE_REG,
HEX_OP_TYPE_PREDICATE,
HEX_OP_TYPE_CONTROL,
HEX_OP_TYPE_SYSTEM,
HEX_OP_TYPE_OPT, 
} HexOpType;
typedef enum {
HEX_OP_CONST_EXT = 1 << 0, 
HEX_OP_REG_HI = 1 << 1, 
HEX_OP_REG_LO = 1 << 2, 
HEX_OP_REG_PAIR = 1 << 3, 
} HexOpAttr;
typedef struct {
ut8 type;
union {
ut8 reg; 
ut32 imm;
ut8 pred; 
ut8 cr; 
ut8 sys; 
} op;
ut8 attr;
} HexOp;
typedef struct {
int instruction;
ut32 mask;
HexPred predicate; 
ut16 pf; 
bool duplex; 
bool compound; 
bool last; 
int shift; 
ut8 op_count;
HexOp ops[6];
char mnem[128]; 
} HexInsn;
typedef struct {
bool duplex;
HexInsn ins[2]; 
} HexInsnCont;
typedef struct {
bool loop0; 
bool loop1; 
int cont_cnt;
HexInsnCont ins[4]; 
} HexInsnPkt;
typedef enum {
HEX_INSN_CLASS_CEXT = 0, 
HEX_INSN_CLASS_J1 = 1, 
HEX_INSN_CLASS_J2 = 2, 
HEX_INSN_CLASS_LD_ST = 3, 
HEX_INSN_CLASS_LD_ST_COND_GP = 4, 
HEX_INSN_CLASS_J3 = 5, 
HEX_INSN_CLASS_CR = 6, 
HEX_INSN_CLASS_ALU32 = 7, 
HEX_INSN_CLASS_XTYPE = 8, 
HEX_INSN_CLASS_LD = 9, 
HEX_INSN_CLASS_ST = 10, 
HEX_INSN_CLASS_ALU32_1 = 11, 
HEX_INSN_CLASS_XTYPE_1 = 12, 
HEX_INSN_CLASS_XTYPE_2 = 13, 
HEX_INSN_CLASS_XTYPE_3 = 14, 
HEX_INSN_CLASS_ALU32_2 = 12, 
} HEX_INSN_CLASS;
typedef enum {
HEX_REG_R0 = 0,
HEX_REG_R1 = 1,
HEX_REG_R2 = 2,
HEX_REG_R3 = 3,
HEX_REG_R4 = 4,
HEX_REG_R5 = 5,
HEX_REG_R6 = 6,
HEX_REG_R7 = 7,
HEX_REG_R8 = 8,
HEX_REG_R9 = 9,
HEX_REG_R10 = 10,
HEX_REG_R11 = 11,
HEX_REG_R12 = 12,
HEX_REG_R13 = 13,
HEX_REG_R14 = 14,
HEX_REG_R15 = 15,
HEX_REG_R16 = 16,
HEX_REG_R17 = 17,
HEX_REG_R18 = 18,
HEX_REG_R19 = 19,
HEX_REG_R20 = 20,
HEX_REG_R21 = 21,
HEX_REG_R22 = 22,
HEX_REG_R23 = 23,
HEX_REG_R24 = 24,
HEX_REG_R25 = 25,
HEX_REG_R26 = 26,
HEX_REG_R27 = 27,
HEX_REG_R28 = 28,
HEX_REG_R29 = 29,
HEX_REG_R30 = 30,
HEX_REG_R31 = 31,
} HEX_REG;
typedef enum {
HEX_REG_SA0 = 0, 
HEX_REG_LC0 = 1, 
HEX_REG_SA1 = 2, 
HEX_REG_LC1 = 3, 
HEX_REG_P = 4, 
HEX_REG_M0 = 6, 
HEX_REG_M1 = 7, 
HEX_REG_USR = 8, 
HEX_REG_PC = 9, 
HEX_REG_UGP = 10, 
HEX_REG_GP = 11, 
HEX_REG_CS0 = 12, 
HEX_REG_CS1 = 13, 
HEX_REG_UPCYCLELO = 14, 
HEX_REG_UPCYCLEHI = 15, 
HEX_REG_FRAMELIMIT = 16, 
HEX_REG_FRAMEKEY = 17, 
HEX_REG_PKTCOUNTLO = 18, 
HEX_REG_PKTCOUNTHI = 19, 
HEX_REG_UTIMERLO = 30, 
HEX_REG_UTIMERHI = 31, 
} HEX_CR_REG;
typedef enum {
HEX_REG_SGP0 = 0, 
HEX_REG_SGP1 = 1, 
HEX_REG_STID = 2, 
HEX_REG_ELR = 3, 
HEX_REG_BADVA0 = 4, 
HEX_REG_BADVA1 = 5, 
HEX_REG_SSR = 6, 
HEX_REG_CCR = 7, 
HEX_REG_HTID = 8, 
HEX_REG_BADVA = 9, 
HEX_REG_IMASK = 10, 
HEX_REG_EVB = 16, 
HEX_REG_MODECTL = 17, 
HEX_REG_SYSCFG = 18, 
HEX_REG_IPEND = 20, 
HEX_REG_VID = 21, 
HEX_REG_IAD = 22, 
HEX_REG_IEL = 24, 
HEX_REG_IAHL = 26, 
HEX_REG_CFGBASE = 27, 
HEX_REG_DIAG = 28, 
HEX_REG_REV = 29, 
HEX_REG_PCYCLELO = 30, 
HEX_REG_PCYCLEHI = 31, 
HEX_REG_ISDBST = 32, 
HEX_REG_ISDBCFG0 = 33, 
HEX_REG_ISDBCFG1 = 34, 
HEX_REG_BRKPTPC0 = 36, 
HEX_REG_BRKPTCFG0 = 37, 
HEX_REG_BRKPTPC1 = 38, 
HEX_REG_BRKPTCFG1 = 39, 
HEX_REG_ISDBMBXIN = 40, 
HEX_REG_ISDBMBXOUT = 41, 
HEX_REG_ISDBEN = 42, 
HEX_REG_ISDBGPR = 43, 
HEX_REG_PMUCNT0 = 48, 
HEX_REG_PMUCNT1 = 49, 
HEX_REG_PMUCNT2 = 50, 
HEX_REG_PMUCNT3 = 51, 
HEX_REG_PMUEVTCFG = 52, 
HEX_REG_PMUCFG = 53, 
} HEX_SYSCR_REG;
typedef enum {
HEX_SUB_REG_R0 = 0, 
HEX_SUB_REG_R1 = 1, 
HEX_SUB_REG_R2 = 2, 
HEX_SUB_REG_R3 = 3, 
HEX_SUB_REG_R4 = 4, 
HEX_SUB_REG_R5 = 5, 
HEX_SUB_REG_R6 = 6, 
HEX_SUB_REG_R7 = 7, 
HEX_SUB_REG_R16 = 8, 
HEX_SUB_REG_R17 = 9, 
HEX_SUB_REG_R18 = 10, 
HEX_SUB_REG_R19 = 11, 
HEX_SUB_REG_R20 = 12, 
HEX_SUB_REG_R21 = 13, 
HEX_SUB_REG_R22 = 14, 
HEX_SUB_REG_R23 = 15, 
} HEX_SUB_REG;
typedef enum {
HEX_SUB_REGPAIR_R1_R0 = 0, 
HEX_SUB_REGPAIR_R3_R2 = 1, 
HEX_SUB_REGPAIR_R5_R4 = 2, 
HEX_SUB_REGPAIR_R7_R6 = 3, 
HEX_SUB_REGPAIR_R17_R16 = 4, 
HEX_SUB_REGPAIR_R19_R18 = 5, 
HEX_SUB_REGPAIR_R21_R20 = 6, 
HEX_SUB_REGPAIR_R23_R22 = 7, 
} HEX_SUB_REGPAIR;
#define BIT_MASK(len) (BIT(len)-1)
#define BF_MASK(start, len) (BIT_MASK(len)<<(start))
#define BF_PREP(x, start, len) (((x)&BIT_MASK(len))<<(start))
#define BF_GET(y, start, len) (((y)>>(start)) & BIT_MASK(len))
#define BF_GETB(y, start, end) (BF_GET((y), (start), (end) - (start) + 1)
char* hex_get_cntl_reg(int opreg);
char* hex_get_sys_reg(int opreg);
char* hex_get_sub_reg(int opreg);
char* hex_get_sub_regpair(int opreg);
bool hex_if_duplex(ut32 insn_word);
void hex_op_extend(HexOp *op);
void hex_op_extend_off(HexOp *op, int offset);
int hexagon_disasm_instruction(ut32 hi_u32, HexInsn *hi, ut32 addr);
