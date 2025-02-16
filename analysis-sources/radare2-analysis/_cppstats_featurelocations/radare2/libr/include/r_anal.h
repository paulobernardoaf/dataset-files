

#if !defined(R2_ANAL_H)
#define R2_ANAL_H



#define USE_VARSUBS 0

#include <r_types.h>
#include <r_io.h>
#include <r_reg.h>
#include <r_list.h>
#include <r_search.h>
#include <r_util.h>
#include <r_bind.h>
#include <r_syscall.h>
#include <sdb/set.h>
#include <r_flag.h>
#include <r_bin.h>

#define esilprintf(op, fmt, ...) r_strbuf_setf (&op->esil, fmt, ##__VA_ARGS__)

#if defined(__cplusplus)
extern "C" {
#endif

R_LIB_VERSION_HEADER(r_anal);










typedef struct r_anal_meta_item_t {
ut64 from;
ut64 to;
ut64 size;
int type;
int subtype;
char *str;
const RSpace *space;
} RAnalMetaItem;

typedef struct {
struct r_anal_t *anal;
int type;
int rad;
SdbForeachCallback cb;
void *user;
int count;
struct r_anal_function_t *fcn;
PJ *pj;
} RAnalMetaUserItem;

typedef struct r_anal_range_t {
ut64 from;
ut64 to;
int bits;
ut64 rb_max_addr;
RBNode rb;
} RAnalRange;

#define R_ANAL_GET_OFFSET(x,y,z) (x && x->binb.bin && x->binb.get_offset)? x->binb.get_offset (x->binb.bin, y, z): -1


enum {
R_ANAL_DATA_TYPE_NULL = 0,
R_ANAL_DATA_TYPE_UNKNOWN = 1,
R_ANAL_DATA_TYPE_STRING = 2,
R_ANAL_DATA_TYPE_WIDE_STRING = 3,
R_ANAL_DATA_TYPE_POINTER = 4,
R_ANAL_DATA_TYPE_NUMBER = 5,
R_ANAL_DATA_TYPE_INVALID = 6,
R_ANAL_DATA_TYPE_HEADER = 7,
R_ANAL_DATA_TYPE_SEQUENCE = 8,
R_ANAL_DATA_TYPE_PATTERN = 9,
};


#define R_ANAL_ADDR_TYPE_EXEC 1
#define R_ANAL_ADDR_TYPE_READ 1 << 1
#define R_ANAL_ADDR_TYPE_WRITE 1 << 2
#define R_ANAL_ADDR_TYPE_FLAG 1 << 3
#define R_ANAL_ADDR_TYPE_FUNC 1 << 4
#define R_ANAL_ADDR_TYPE_HEAP 1 << 5
#define R_ANAL_ADDR_TYPE_STACK 1 << 6
#define R_ANAL_ADDR_TYPE_REG 1 << 7
#define R_ANAL_ADDR_TYPE_PROGRAM 1 << 8
#define R_ANAL_ADDR_TYPE_LIBRARY 1 << 9
#define R_ANAL_ADDR_TYPE_ASCII 1 << 10
#define R_ANAL_ADDR_TYPE_SEQUENCE 1 << 11

#define R_ANAL_ARCHINFO_MIN_OP_SIZE 0
#define R_ANAL_ARCHINFO_MAX_OP_SIZE 1
#define R_ANAL_ARCHINFO_ALIGN 2
#define R_ANAL_ARCHINFO_DATA_ALIGN 4



#define R_ANAL_GET_OFFSET(x,y,z) (x && x->binb.bin && x->binb.get_offset)? x->binb.get_offset (x->binb.bin, y, z): -1



#define R_ANAL_GET_NAME(x,y,z) (x && x->binb.bin && x->binb.get_name)? x->binb.get_name (x->binb.bin, y, z): NULL







typedef struct r_anal_type_var_t {
char *name;
int index;
int scope;
ut16 type; 
ut8 size;
union {
ut8 v8;
ut16 v16;
ut32 v32;
ut64 v64;
} value;
} RAnalTypeVar;

typedef struct r_anal_type_ptr_t {
char *name;
ut16 type; 
ut8 size;
union {
ut8 v8;
ut16 v16;
ut32 v32;
ut64 v64;
} value;
} RAnalTypePtr;

typedef struct r_anal_type_array_t {
char *name;
ut16 type; 
ut8 size;
ut64 count;
union {
ut8 *v8;
ut16 *v16;
ut32 *v32;
ut64 *v64;
} value;
} RAnalTypeArray;

typedef struct r_anal_type_struct_t RAnalTypeStruct;
typedef struct r_anal_type_t RAnalType;

struct r_anal_type_struct_t {
char *name;
ut8 type;
ut32 size;
void *parent;
RAnalType *items;
};

typedef struct r_anal_type_union_t {
char *name;
ut8 type;
ut32 size;
void *parent;
RAnalType *items;
} RAnalTypeUnion;

typedef struct r_anal_type_alloca_t {
long address;
long size;
void *parent;
RAnalType *items;
} RAnalTypeAlloca;

enum {
R_ANAL_FQUALIFIER_NONE = 0,
R_ANAL_FQUALIFIER_STATIC = 1,
R_ANAL_FQUALIFIER_VOLATILE = 2,
R_ANAL_FQUALIFIER_INLINE = 3,
R_ANAL_FQUALIFIER_NAKED = 4,
R_ANAL_FQUALIFIER_VIRTUAL = 5,
};



#define R_ANAL_CC_TYPE_STDCALL 0
#define R_ANAL_CC_TYPE_PASCAL 1
#define R_ANAL_CC_TYPE_FASTCALL 'A' 
#define R_ANAL_CC_TYPE_SYSV 8
#define R_ANAL_CC_MAXARG 16

enum {
R_ANAL_FCN_TYPE_NULL = 0,
R_ANAL_FCN_TYPE_FCN = 1 << 0,
R_ANAL_FCN_TYPE_LOC = 1 << 1,
R_ANAL_FCN_TYPE_SYM = 1 << 2,
R_ANAL_FCN_TYPE_IMP = 1 << 3,
R_ANAL_FCN_TYPE_INT = 1 << 4, 
R_ANAL_FCN_TYPE_ROOT = 1 << 5, 
R_ANAL_FCN_TYPE_ANY = -1 
};

#define RAnalBlock struct r_anal_bb_t

enum {
R_ANAL_DIFF_TYPE_NULL = 0,
R_ANAL_DIFF_TYPE_MATCH = 'm',
R_ANAL_DIFF_TYPE_UNMATCH = 'u'
};

typedef struct r_anal_diff_t {
int type;
ut64 addr;
double dist;
char *name;
ut32 size;
} RAnalDiff;
typedef struct r_anal_attr_t RAnalAttr;
struct r_anal_attr_t {
char *key;
long value;
RAnalAttr *next;
};



typedef struct r_anal_fcn_meta_t {



ut64 _min; 
ut64 _max; 

int numrefs; 
int numcallrefs; 
} RAnalFcnMeta;




typedef struct r_anal_function_t {
char* name;
char* dsc; 
int bits; 
int type;

char *rets;
short fmod; 
const char *cc; 
char* attr; 
ut64 addr;
ut64 rb_max_addr; 
int stack; 
int maxstack;
int ninstr;
int nargs; 
int depth;
bool folded;
bool is_pure;
bool has_changed; 
bool bp_frame;
bool is_noreturn; 
RAnalType *args; 
ut8 *fingerprint; 
size_t fingerprint_size;
RAnalDiff *diff;

RList *bbs; 
RAnalFcnMeta meta;
RBNode addr_rb;
RList *imports; 
struct r_anal_t *anal; 
} RAnalFunction;

typedef struct r_anal_func_arg_t {
const char *name;
const char *fmt;
const char *cc_source;
char *orig_c_type;
char *c_type;
ut64 size;
ut64 src; 
} RAnalFuncArg;

struct r_anal_type_t {
char *name;
ut32 type;
ut32 size;
RList *content;
};

enum {
R_META_WHERE_PREV = -1,
R_META_WHERE_HERE = 0,
R_META_WHERE_NEXT = 1,
};

enum {
R_META_TYPE_NONE = 0,
R_META_TYPE_ANY = -1,
R_META_TYPE_DATA = 'd',
R_META_TYPE_CODE = 'c',
R_META_TYPE_STRING = 's',
R_META_TYPE_FORMAT = 'f',
R_META_TYPE_MAGIC = 'm',
R_META_TYPE_HIDE = 'h',
R_META_TYPE_COMMENT = 'C',
R_META_TYPE_RUN = 'r',
R_META_TYPE_HIGHLIGHT = 'H',
R_META_TYPE_VARTYPE = 't',
};


typedef enum {
R_ANAL_OP_FAMILY_UNKNOWN = -1,
R_ANAL_OP_FAMILY_CPU = 0,
R_ANAL_OP_FAMILY_FPU, 
R_ANAL_OP_FAMILY_MMX, 
R_ANAL_OP_FAMILY_SSE, 
R_ANAL_OP_FAMILY_PRIV, 
R_ANAL_OP_FAMILY_CRYPTO, 
R_ANAL_OP_FAMILY_THREAD, 
R_ANAL_OP_FAMILY_VIRT, 
R_ANAL_OP_FAMILY_PAC, 
R_ANAL_OP_FAMILY_IO, 
R_ANAL_OP_FAMILY_LAST
} RAnalOpFamily;

#if 0
On x86 according to Wikipedia

Prefix group 1
0xF0: LOCK prefix
0xF2: REPNE/REPNZ prefix
0xF3: REP or REPE/REPZ prefix
Prefix group 2
0x2E: CS segment override
0x36: SS segment override
0x3E: DS segment override
0x26: ES segment override
0x64: FS segment override
0x65: GS segment override
0x2E: Branch not taken (hinting)
0x3E: Branch taken
Prefix group 3
0x66: Operand-size override prefix
Prefix group 4
0x67: Address-size override prefix
#endif
typedef enum {
R_ANAL_OP_PREFIX_COND = 1,
R_ANAL_OP_PREFIX_REP = 1<<1,
R_ANAL_OP_PREFIX_REPNE = 1<<2,
R_ANAL_OP_PREFIX_LOCK = 1<<3,
R_ANAL_OP_PREFIX_LIKELY = 1<<4,
R_ANAL_OP_PREFIX_UNLIKELY = 1<<5

} RAnalOpPrefix;


#define R_ANAL_OP_TYPE_MASK 0x8000ffff
#define R_ANAL_OP_HINT_MASK 0xf0000000
typedef enum {
R_ANAL_OP_TYPE_COND = 0x80000000, 

R_ANAL_OP_TYPE_REP = 0x40000000, 
R_ANAL_OP_TYPE_MEM = 0x20000000, 
R_ANAL_OP_TYPE_REG = 0x10000000, 
R_ANAL_OP_TYPE_IND = 0x08000000, 
R_ANAL_OP_TYPE_NULL = 0,
R_ANAL_OP_TYPE_JMP = 1, 
R_ANAL_OP_TYPE_UJMP = 2, 
R_ANAL_OP_TYPE_RJMP = R_ANAL_OP_TYPE_REG | R_ANAL_OP_TYPE_UJMP,
R_ANAL_OP_TYPE_IJMP = R_ANAL_OP_TYPE_IND | R_ANAL_OP_TYPE_UJMP,
R_ANAL_OP_TYPE_IRJMP = R_ANAL_OP_TYPE_IND | R_ANAL_OP_TYPE_REG | R_ANAL_OP_TYPE_UJMP,
R_ANAL_OP_TYPE_CJMP = R_ANAL_OP_TYPE_COND | R_ANAL_OP_TYPE_JMP, 
R_ANAL_OP_TYPE_RCJMP = R_ANAL_OP_TYPE_REG | R_ANAL_OP_TYPE_CJMP, 
R_ANAL_OP_TYPE_MJMP = R_ANAL_OP_TYPE_MEM | R_ANAL_OP_TYPE_JMP, 
R_ANAL_OP_TYPE_MCJMP = R_ANAL_OP_TYPE_MEM | R_ANAL_OP_TYPE_CJMP, 
R_ANAL_OP_TYPE_UCJMP = R_ANAL_OP_TYPE_COND | R_ANAL_OP_TYPE_UJMP, 
R_ANAL_OP_TYPE_CALL = 3, 
R_ANAL_OP_TYPE_UCALL = 4, 
R_ANAL_OP_TYPE_RCALL = R_ANAL_OP_TYPE_REG | R_ANAL_OP_TYPE_UCALL,
R_ANAL_OP_TYPE_ICALL = R_ANAL_OP_TYPE_IND | R_ANAL_OP_TYPE_UCALL,
R_ANAL_OP_TYPE_IRCALL= R_ANAL_OP_TYPE_IND | R_ANAL_OP_TYPE_REG | R_ANAL_OP_TYPE_UCALL,
R_ANAL_OP_TYPE_CCALL = R_ANAL_OP_TYPE_COND | R_ANAL_OP_TYPE_CALL, 
R_ANAL_OP_TYPE_UCCALL= R_ANAL_OP_TYPE_COND | R_ANAL_OP_TYPE_UCALL, 
R_ANAL_OP_TYPE_RET = 5, 
R_ANAL_OP_TYPE_CRET = R_ANAL_OP_TYPE_COND | R_ANAL_OP_TYPE_RET, 
R_ANAL_OP_TYPE_ILL = 6, 
R_ANAL_OP_TYPE_UNK = 7, 
R_ANAL_OP_TYPE_NOP = 8, 
R_ANAL_OP_TYPE_MOV = 9, 
R_ANAL_OP_TYPE_CMOV = 9 | R_ANAL_OP_TYPE_COND, 
R_ANAL_OP_TYPE_TRAP = 10, 
R_ANAL_OP_TYPE_SWI = 11, 
R_ANAL_OP_TYPE_CSWI = 11 | R_ANAL_OP_TYPE_COND, 
R_ANAL_OP_TYPE_UPUSH = 12, 
R_ANAL_OP_TYPE_RPUSH = R_ANAL_OP_TYPE_UPUSH | R_ANAL_OP_TYPE_REG, 
R_ANAL_OP_TYPE_PUSH = 13, 
R_ANAL_OP_TYPE_POP = 14, 
R_ANAL_OP_TYPE_CMP = 15, 
R_ANAL_OP_TYPE_ACMP = 16, 
R_ANAL_OP_TYPE_ADD = 17,
R_ANAL_OP_TYPE_SUB = 18,
R_ANAL_OP_TYPE_IO = 19,
R_ANAL_OP_TYPE_MUL = 20,
R_ANAL_OP_TYPE_DIV = 21,
R_ANAL_OP_TYPE_SHR = 22,
R_ANAL_OP_TYPE_SHL = 23,
R_ANAL_OP_TYPE_SAL = 24,
R_ANAL_OP_TYPE_SAR = 25,
R_ANAL_OP_TYPE_OR = 26,
R_ANAL_OP_TYPE_AND = 27,
R_ANAL_OP_TYPE_XOR = 28,
R_ANAL_OP_TYPE_NOR = 29,
R_ANAL_OP_TYPE_NOT = 30,
R_ANAL_OP_TYPE_STORE = 31, 
R_ANAL_OP_TYPE_LOAD = 32, 
R_ANAL_OP_TYPE_LEA = 33, 
R_ANAL_OP_TYPE_LEAVE = 34,
R_ANAL_OP_TYPE_ROR = 35,
R_ANAL_OP_TYPE_ROL = 36,
R_ANAL_OP_TYPE_XCHG = 37,
R_ANAL_OP_TYPE_MOD = 38,
R_ANAL_OP_TYPE_SWITCH = 39,
R_ANAL_OP_TYPE_CASE = 40,
R_ANAL_OP_TYPE_LENGTH = 41,
R_ANAL_OP_TYPE_CAST = 42,
R_ANAL_OP_TYPE_NEW = 43,
R_ANAL_OP_TYPE_ABS = 44,
R_ANAL_OP_TYPE_CPL = 45, 
R_ANAL_OP_TYPE_CRYPTO = 46,
R_ANAL_OP_TYPE_SYNC = 47,

#if 0
R_ANAL_OP_TYPE_PRIV = 40, 
R_ANAL_OP_TYPE_FPU = 41, 
#endif
} _RAnalOpType;

typedef enum {
R_ANAL_OP_MASK_BASIC = 0, 
R_ANAL_OP_MASK_ESIL = 1, 
R_ANAL_OP_MASK_VAL = 2, 
R_ANAL_OP_MASK_HINT = 4, 
R_ANAL_OP_MASK_OPEX = 8, 
R_ANAL_OP_MASK_DISASM = 16, 
R_ANAL_OP_MASK_ALL = 1 | 2 | 4 | 8 | 16
} RAnalOpMask;


typedef enum {
R_ANAL_COND_AL = 0, 
R_ANAL_COND_EQ, 
R_ANAL_COND_NE, 
R_ANAL_COND_GE, 
R_ANAL_COND_GT, 
R_ANAL_COND_LE, 
R_ANAL_COND_LT, 
R_ANAL_COND_NV, 
R_ANAL_COND_HS, 
R_ANAL_COND_LO, 
R_ANAL_COND_MI, 
R_ANAL_COND_PL, 
R_ANAL_COND_VS, 
R_ANAL_COND_VC, 
R_ANAL_COND_HI, 
R_ANAL_COND_LS 
} _RAnalCond;

typedef enum {
R_ANAL_VAR_SCOPE_NULL = 0,
R_ANAL_VAR_SCOPE_GLOBAL = 0x00,
R_ANAL_VAR_SCOPE_LOCAL = 0x01,
R_ANAL_VAR_SCOPE_ARG = 0x04,
R_ANAL_VAR_SCOPE_ARGREG = 0x08,
R_ANAL_VAR_SCOPE_RET = 0x10,
} _RAnalVarScope;

typedef enum {
R_ANAL_VAR_DIR_NONE = 0,
R_ANAL_VAR_DIR_IN = 0x100,
R_ANAL_VAR_DIR_OUT = 0x200
} _RAnalVarDir;

typedef enum {
R_ANAL_DATA_NULL = 0,
R_ANAL_DATA_HEX, 
R_ANAL_DATA_STR, 
R_ANAL_DATA_CODE, 
R_ANAL_DATA_FUN, 
R_ANAL_DATA_STRUCT, 
R_ANAL_DATA_LAST
} _RAnalData;

typedef enum {
R_ANAL_STACK_NULL = 0,
R_ANAL_STACK_NOP,
R_ANAL_STACK_INC,
R_ANAL_STACK_GET,
R_ANAL_STACK_SET,
R_ANAL_STACK_RESET,
R_ANAL_STACK_ALIGN,
} RAnalStackOp;

enum {
R_ANAL_REFLINE_TYPE_UTF8 = 1,
R_ANAL_REFLINE_TYPE_WIDE = 2, 
R_ANAL_REFLINE_TYPE_MIDDLE_BEFORE = 4, 

R_ANAL_REFLINE_TYPE_MIDDLE_AFTER = 8 
};

enum {
R_ANAL_RET_NOP = 0,
R_ANAL_RET_ERROR = -1,
R_ANAL_RET_DUP = -2,
R_ANAL_RET_NEW = -3,
R_ANAL_RET_END = -4
};

typedef struct r_anal_case_obj_t {
ut64 addr;
ut64 jump;
ut64 value;
} RAnalCaseOp;

typedef struct r_anal_switch_obj_t {
ut64 addr;
ut64 min_val;
ut64 def_val;
ut64 max_val;
RList *cases;
} RAnalSwitchOp;

struct r_anal_t;
struct r_anal_bb_t;
typedef struct r_anal_callbacks_t {
int (*on_fcn_new) (struct r_anal_t *, void *user, RAnalFunction *fcn);
int (*on_fcn_delete) (struct r_anal_t *, void *user, RAnalFunction *fcn);
int (*on_fcn_rename) (struct r_anal_t *, void *user, RAnalFunction *fcn, const char *oldname);
int (*on_fcn_bb_new) (struct r_anal_t *, void *user, RAnalFunction *fcn, struct r_anal_bb_t *bb);
} RAnalCallbacks;

#define R_ANAL_ESIL_GOTO_LIMIT 4096

typedef struct r_anal_options_t {
int depth;
int graph_depth;
bool vars; 
int cjmpref;
int jmpref;
int jmpabove;
bool ijmp;
bool jmpmid; 
bool loads;
bool ignbithints;
int followdatarefs;
int searchstringrefs;
int followbrokenfcnsrefs;
int bb_max_size;
bool trycatch;
bool norevisit;
int afterjmp; 
int recont; 
int noncode;
int nopskip; 
int hpskip; 
int jmptbl; 
int nonull;
bool pushret; 
bool armthumb; 
bool endsize; 
bool delay;
int tailcall;
bool retpoline;
} RAnalOptions;

typedef enum {
R_ANAL_CPP_ABI_ITANIUM = 0,
R_ANAL_CPP_ABI_MSVC
} RAnalCPPABI;

typedef struct r_anal_hint_cb_t {

void (*on_bits) (struct r_anal_t *a, ut64 addr, int bits, bool set);
} RHintCb;

typedef struct r_anal_t {
char *cpu;
char *os;
int bits;
int lineswidth; 
int big_endian;
int sleep; 
RAnalCPPABI cpp_abi;
void *user;
ut64 gp; 
RBTree bb_tree; 
RList *fcns;
HtUP *ht_addr_fun; 
HtPP *ht_name_fun; 
RList *refs;
RList *vartypes;
RReg *reg;
ut8 *last_disasm_reg;
RSyscall *syscall;
struct r_anal_op_t *queued;
int diff_ops;
double diff_thbb;
double diff_thfcn;
RIOBind iob;
RFlagBind flb;
RFlagSet flg_class_set;
RFlagGet flg_class_get;
RFlagSet flg_fcn_set;
RBinBind binb; 
RCoreBind coreb;
int maxreflines;
int trace;
int esil_goto_limit;
int pcalign;
int bitshift;

struct r_anal_esil_t *esil;
struct r_anal_plugin_t *cur;
RAnalRange *limit;
RList *plugins;
Sdb *sdb_types;
Sdb *sdb_fmts;
Sdb *sdb_meta; 
Sdb *sdb_zigns;
HtUP *dict_refs;
HtUP *dict_xrefs;
bool recursive_noreturn;
RSpaces meta_spaces;
RSpaces zign_spaces;
char *zign_path;
PrintfCallback cb_printf;

Sdb *sdb; 
Sdb *sdb_fcns;
Sdb *sdb_pins;
#define DEPRECATE 1
#if DEPRECATE
Sdb *sdb_args; 
Sdb *sdb_vars; 
#endif
HtUP *addr_hints; 
RBTree arch_hints;
RBTree bits_hints;
RHintCb hint_cbs;
Sdb *sdb_fcnsign; 
Sdb *sdb_cc; 
Sdb *sdb_classes;
Sdb *sdb_classes_attrs;
RAnalCallbacks cb;
RAnalOptions opt;
RList *reflines;

RListComparator columnSort;
int stackptr;
bool (*log)(struct r_anal_t *anal, const char *msg);
bool (*read_at)(struct r_anal_t *anal, ut64 addr, ut8 *buf, int len);
bool verbose;
int seggrn;
RFlagGetAtAddr flag_get;
REvent *ev;
RList *imports; 
SetU *visited;
RStrConstPool constpool;
RList *leaddrs;
} RAnal;

typedef enum r_anal_addr_hint_type_t {
R_ANAL_ADDR_HINT_TYPE_IMMBASE,
R_ANAL_ADDR_HINT_TYPE_JUMP,
R_ANAL_ADDR_HINT_TYPE_FAIL,
R_ANAL_ADDR_HINT_TYPE_STACKFRAME,
R_ANAL_ADDR_HINT_TYPE_PTR,
R_ANAL_ADDR_HINT_TYPE_NWORD,
R_ANAL_ADDR_HINT_TYPE_RET,
R_ANAL_ADDR_HINT_TYPE_NEW_BITS,
R_ANAL_ADDR_HINT_TYPE_SIZE,
R_ANAL_ADDR_HINT_TYPE_SYNTAX,
R_ANAL_ADDR_HINT_TYPE_OPTYPE,
R_ANAL_ADDR_HINT_TYPE_OPCODE,
R_ANAL_ADDR_HINT_TYPE_TYPE_OFFSET,
R_ANAL_ADDR_HINT_TYPE_ESIL,
R_ANAL_ADDR_HINT_TYPE_HIGH,
R_ANAL_ADDR_HINT_TYPE_VAL
} RAnalAddrHintType;

typedef struct r_anal_addr_hint_record_t {
RAnalAddrHintType type;
union {
char *type_offset;
int nword;
ut64 jump;
ut64 fail;
int newbits;
int immbase;
ut64 ptr;
ut64 retval;
char *syntax;
char *opcode;
char *esil;
int optype;
ut64 size;
ut64 stackframe;
ut64 val;
};
} RAnalAddrHintRecord;

typedef struct r_anal_hint_t {
ut64 addr;
ut64 ptr;
ut64 val; 
ut64 jump;
ut64 fail;
ut64 ret; 
char *arch;
char *opcode;
char *syntax;
char *esil;
char *offset;
ut32 type;
ut64 size;
int bits;
int new_bits; 
int immbase;
bool high; 
int nword;
ut64 stackframe;
} RAnalHint;

typedef struct r_anal_var_access_t {
ut64 addr;
int set;
} RAnalVarAccess;

typedef RAnalFunction *(* RAnalGetFcnIn)(RAnal *anal, ut64 addr, int type);
typedef RAnalHint *(* RAnalGetHint)(RAnal *anal, ut64 addr);

typedef struct r_anal_bind_t {
RAnal *anal;
RAnalGetFcnIn get_fcn_in;
RAnalGetHint get_hint;
} RAnalBind;

typedef const char *(*RAnalLabelAt) (RAnal *a, RAnalFunction *fcn, ut64);

#define R_ANAL_VAR_KIND_ANY 0
#define R_ANAL_VAR_KIND_ARG 'a'
#define R_ANAL_VAR_KIND_REG 'r'
#define R_ANAL_VAR_KIND_BPV 'b'
#define R_ANAL_VAR_KIND_SPV 's'

#define VARPREFIX "var"
#define ARGPREFIX "arg"


typedef struct r_anal_var_t {
char *name; 
char *regname; 
char *type; 
char kind; 
ut64 addr; 
ut64 eaddr; 
int size;
bool isarg;
int argnum;
int delta; 
int scope; 

RList *accesses; 
RList *stores; 
} RAnalVar;


typedef struct r_anal_value_t {
int absolute; 
int memref; 
ut64 base ; 
st64 delta; 
st64 imm; 
int mul; 
ut16 sel; 
RRegItem *reg; 
RRegItem *regdelta; 
} RAnalValue;

typedef enum {
R_ANAL_OP_DIR_READ = 1,
R_ANAL_OP_DIR_WRITE = 2,
R_ANAL_OP_DIR_EXEC = 4,
R_ANAL_OP_DIR_REF = 8,
} RAnalOpDirection;

typedef enum r_anal_data_type_t {
R_ANAL_DATATYPE_NULL = 0,
R_ANAL_DATATYPE_ARRAY,
R_ANAL_DATATYPE_OBJECT, 
R_ANAL_DATATYPE_STRING,
R_ANAL_DATATYPE_CLASS,
R_ANAL_DATATYPE_BOOLEAN,
R_ANAL_DATATYPE_INT16,
R_ANAL_DATATYPE_INT32,
R_ANAL_DATATYPE_INT64,
R_ANAL_DATATYPE_FLOAT,
} RAnalDataType;

typedef struct r_anal_op_t {
char *mnemonic; 
ut64 addr; 
ut32 type; 
RAnalOpPrefix prefix; 
ut32 type2; 
RAnalStackOp stackop; 
_RAnalCond cond; 
int size; 
int nopcode; 
int cycles; 
int failcycles; 
RAnalOpFamily family; 
int id; 
bool eob; 
bool sign; 

int delay; 
ut64 jump; 
ut64 fail; 
RAnalOpDirection direction;
st64 ptr; 
ut64 val; 
int ptrsize; 
st64 stackptr; 
int refptr; 
RAnalVar *var; 
RAnalValue *src[3];
RAnalValue *dst;
RStrBuf esil;
RStrBuf opex;
const char *reg; 
const char *ireg; 
int scale;
ut64 disp;
RAnalSwitchOp *switch_op;
RAnalHint hint;
RAnalDataType datatype;
} RAnalOp;

#define R_ANAL_COND_SINGLE(x) (!x->arg[1] || x->arg[0]==x->arg[1])

typedef struct r_anal_cond_t {
int type; 
RAnalValue *arg[2]; 
} RAnalCond;

typedef struct r_anal_bb_t {
RBNode _rb; 
ut64 _max_end; 

ut64 addr;
ut64 size;
ut64 jump;
ut64 fail;
bool traced;
bool folded;
ut32 colorize;
ut8 *fingerprint;
RAnalDiff *diff;
RAnalCond *cond;
RAnalSwitchOp *switch_op;
ut16 *op_pos; 
ut8 *op_bytes;
ut8 *parent_reg_arena;
int op_pos_size; 
int ninstr;
int stackptr;
int parent_stackptr;
ut64 cmpval;
const char *cmpreg;

RList *fcns;
RAnal *anal;
int ref;
#undef RAnalBlock
} RAnalBlock;

typedef enum {
R_ANAL_REF_TYPE_NULL = 0,
R_ANAL_REF_TYPE_CODE = 'c', 
R_ANAL_REF_TYPE_CALL = 'C', 
R_ANAL_REF_TYPE_DATA = 'd', 
R_ANAL_REF_TYPE_STRING='s' 
} RAnalRefType;

typedef struct r_anal_ref_t {
ut64 addr;
ut64 at;
RAnalRefType type;
} RAnalRef;
R_API const char *r_anal_ref_type_tostring(RAnalRefType t);


typedef struct r_anal_refline_t {
ut64 from;
ut64 to;
int index;
int level;
int type;
int direction;
} RAnalRefline;

typedef struct r_anal_cycle_frame_t {
ut64 naddr; 
RList *hooks;
struct r_anal_cycle_frame_t *prev;
} RAnalCycleFrame;

typedef struct r_anal_cycle_hook_t { 
ut64 addr;
int cycles;
} RAnalCycleHook;

typedef struct r_anal_esil_word_t {
int type;
const char *str;
} RAnalEsilWord;


enum {
R_ANAL_ESIL_FLAG_ZERO = 1,
R_ANAL_ESIL_FLAG_CARRY = 2,
R_ANAL_ESIL_FLAG_OVERFLOW = 4,
R_ANAL_ESIL_FLAG_PARITY = 8,
R_ANAL_ESIL_FLAG_SIGN = 16,

};

enum {
R_ANAL_TRAP_NONE = 0,
R_ANAL_TRAP_UNHANDLED = 1,
R_ANAL_TRAP_BREAKPOINT = 2,
R_ANAL_TRAP_DIVBYZERO = 3,
R_ANAL_TRAP_WRITE_ERR = 4,
R_ANAL_TRAP_READ_ERR = 5,
R_ANAL_TRAP_EXEC_ERR = 6,
R_ANAL_TRAP_INVALID = 7,
R_ANAL_TRAP_UNALIGNED = 8,
R_ANAL_TRAP_TODO = 9,
R_ANAL_TRAP_HALT = 10,
};

enum {
R_ANAL_ESIL_PARM_INVALID = 0,
R_ANAL_ESIL_PARM_REG,
R_ANAL_ESIL_PARM_NUM,
};


#define FOREACHOP(GENERATE) GENERATE(NOP) GENERATE(UNK) GENERATE(JCC) GENERATE(STR) GENERATE(STM) GENERATE(LDM) GENERATE(ADD) GENERATE(SUB) GENERATE(NEG) GENERATE(MUL) GENERATE(DIV) GENERATE(MOD) GENERATE(SMUL) GENERATE(SDIV) GENERATE(SMOD) GENERATE(SHL) GENERATE(SHR) GENERATE(AND) GENERATE(OR) GENERATE(XOR) GENERATE(NOT) GENERATE(EQ) GENERATE(LT)
























#define MAKE_ENUM(OP) REIL_##OP,
#define REIL_OP_STRING(STRING) #STRING,

typedef enum {
FOREACHOP(MAKE_ENUM)
} RAnalReilOpcode;

typedef enum {
ARG_REG, 
ARG_TEMP, 
ARG_CONST, 
ARG_ESIL_INTERNAL, 
ARG_NONE 
} RAnalReilArgType;


typedef struct r_anal_reil_arg {
RAnalReilArgType type; 
ut8 size; 
char name[32]; 
} RAnalReilArg;

typedef struct r_anal_ref_char {
char *str;
char *cols;
} RAnalRefStr;


typedef struct r_anal_reil_inst {
RAnalReilOpcode opcode;
RAnalReilArg *arg[3];
} RAnalReilInst;

typedef struct r_anal_reil {
char old[32]; 
char cur[32];
ut8 lastsz;
ut64 reilNextTemp; 
ut64 addr; 
ut8 seq_num; 
int skip;
int cmd_count;
char if_buf[64];
char pc[8];
} RAnalReil;


#define ESIL_INTERNAL_PREFIX '$'
#define ESIL_STACK_NAME "esil.ram"
#define ESIL struct r_anal_esil_t

typedef struct r_anal_esil_source_t {
ut32 id;
ut32 claimed;
void *content;
} RAnalEsilSource;

R_API void r_anal_esil_sources_init(ESIL *esil);
R_API ut32 r_anal_esil_load_source(ESIL *esil, const char *path);
R_API void *r_anal_esil_get_source(ESIL *esil, ut32 src_id);
R_API bool r_anal_esil_claim_source(ESIL *esil, ut32 src_id);
R_API void r_anal_esil_release_source(ESIL *esil, ut32 src_id);
R_API void r_anal_esil_sources_fini(ESIL *esil);

typedef bool (*RAnalEsilInterruptCB)(ESIL *esil, ut32 interrupt, void *user);

typedef struct r_anal_esil_interrupt_handler_t {
const ut32 num;
const char* name;
void *(*init)(ESIL *esil);
RAnalEsilInterruptCB cb;
void (*fini)(void *user);
} RAnalEsilInterruptHandler;

typedef struct r_anal_esil_interrupt_t {
RAnalEsilInterruptHandler *handler;
void *user;
ut32 src_id;
} RAnalEsilInterrupt;

typedef struct r_anal_esil_session_t {
ut64 key;
ut64 addr;
ut64 size;
ut8 *data;
RListIter *reg[R_REG_TYPE_LAST];
} RAnalEsilSession;

typedef int (*RAnalEsilHookRegWriteCB)(ESIL *esil, const char *name, ut64 *val);

typedef struct r_anal_esil_callbacks_t {
void *user;

int (*hook_flag_read)(ESIL *esil, const char *flag, ut64 *num);
int (*hook_command)(ESIL *esil, const char *op);
int (*hook_mem_read)(ESIL *esil, ut64 addr, ut8 *buf, int len);
int (*mem_read)(ESIL *esil, ut64 addr, ut8 *buf, int len);
int (*hook_mem_write)(ESIL *esil, ut64 addr, const ut8 *buf, int len);
int (*mem_write)(ESIL *esil, ut64 addr, const ut8 *buf, int len);
int (*hook_reg_read)(ESIL *esil, const char *name, ut64 *res, int *size);
int (*reg_read)(ESIL *esil, const char *name, ut64 *res, int *size);
RAnalEsilHookRegWriteCB hook_reg_write;
int (*reg_write)(ESIL *esil, const char *name, ut64 val);
} RAnalEsilCallbacks;

typedef struct r_anal_esil_t {
RAnal *anal;
char **stack;
ut64 addrmask;
int stacksize;
int stackptr;
ut32 skip;
int nowrite;
int iotrap;
int exectrap;
int repeat;
int parse_stop;
int parse_goto;
int parse_goto_count;
int verbose;
ut64 flags;
ut64 address;
ut64 stack_addr;
ut32 stack_size;
int delay; 
ut64 jump_target; 
int jump_target_set; 
int trap;
ut32 trap_code; 

ut64 old; 
ut64 cur; 
ut8 lastsz; 

Sdb *ops;
char *current_opstr;
RIDStorage *sources;
SdbMini *interrupts;

RAnalEsilInterrupt *intr0;

Sdb *stats;
Sdb *db_trace;
int trace_idx;
RAnalEsilCallbacks cb;
RAnalReil *Reil;


char *cmd_step; 
char *cmd_step_out; 
char *cmd_intr; 
char *cmd_trap; 
char *cmd_mdev; 
char *cmd_todo; 
char *cmd_ioer; 
char *mdev_range; 
bool (*cmd)(ESIL *esil, const char *name, ut64 a0, ut64 a1);
void *user;
int stack_fd; 
RList *sessions; 
} RAnalEsil;

#undef ESIL


enum {
R_ANAL_ESIL_OP_TYPE_UNKNOWN = 0x1,
R_ANAL_ESIL_OP_TYPE_CONTROL_FLOW,
R_ANAL_ESIL_OP_TYPE_MEM_READ = 0x4,
R_ANAL_ESIL_OP_TYPE_MEM_WRITE = 0x8,
R_ANAL_ESIL_OP_TYPE_REG_WRITE = 0x10,
R_ANAL_ESIL_OP_TYPE_MATH = 0x20,
R_ANAL_ESIL_OP_TYPE_CUSTOM = 0x40
};


typedef bool (*RAnalEsilOpCb)(RAnalEsil *esil);

typedef struct r_anal_esil_operation_t {
RAnalEsilOpCb code;
ut32 push; 
ut32 pop; 
ut32 type;
} RAnalEsilOp;



typedef struct r_anal_esil_expr_offset_t {
ut64 off;
ut16 idx;
} RAnalEsilEOffset;

typedef enum {
R_ANAL_ESIL_BLOCK_ENTER_NORMAL = 0,
R_ANAL_ESIL_BLOCK_ENTER_TRUE,
R_ANAL_ESIL_BLOCK_ENTER_FALSE,
R_ANAL_ESIL_BLOCK_ENTER_GLUE,
} RAnalEsilBlockEnterType;

typedef struct r_anal_esil_basic_block_t {
RAnalEsilEOffset first;
RAnalEsilEOffset last;
char *expr; 
RAnalEsilBlockEnterType enter; 
} RAnalEsilBB;

typedef struct r_anal_esil_cfg_t {
RGraphNode *start;
RGraphNode *end;
RGraph *g;
} RAnalEsilCFG;

typedef enum {
R_ANAL_ESIL_DFG_BLOCK_CONST = 1,
R_ANAL_ESIL_DFG_BLOCK_VAR = 2,
R_ANAL_ESIL_DFG_BLOCK_PTR = 4,
R_ANAL_ESIL_DFG_BLOCK_RESULT = 8,
R_ANAL_ESIL_DFG_BLOCK_GENERATIVE = 16,
} RAnalEsilDFGBlockType;

typedef struct r_anal_esil_dfg_t {
ut32 idx;
Sdb *regs; 
RContRBTree *reg_vars; 
RQueue *todo; 
void *insert; 
RGraph *flow;
RGraphNode *cur;
RGraphNode *old;
bool malloc_failed;
} RAnalEsilDFG;

typedef struct r_anal_esil_dfg_node_t {

ut32 idx;
RStrBuf *content;
RAnalEsilDFGBlockType type;
} RAnalEsilDFGNode;

typedef int (*RAnalCmdExt)(RAnal *anal, const char* input);


typedef int (*RAnalOpCallback)(RAnal *a, RAnalOp *op, ut64 addr, const ut8 *data, int len, RAnalOpMask mask);

typedef int (*RAnalRegProfCallback)(RAnal *a);
typedef char*(*RAnalRegProfGetCallback)(RAnal *a);
typedef int (*RAnalFPBBCallback)(RAnal *a, RAnalBlock *bb);
typedef int (*RAnalFPFcnCallback)(RAnal *a, RAnalFunction *fcn);
typedef int (*RAnalDiffBBCallback)(RAnal *anal, RAnalFunction *fcn, RAnalFunction *fcn2);
typedef int (*RAnalDiffFcnCallback)(RAnal *anal, RList *fcns, RList *fcns2);
typedef int (*RAnalDiffEvalCallback)(RAnal *anal);

typedef int (*RAnalEsilCB)(RAnalEsil *esil);
typedef int (*RAnalEsilLoopCB)(RAnalEsil *esil, RAnalOp *op);
typedef int (*RAnalEsilTrapCB)(RAnalEsil *esil, int trap_type, int trap_code);

typedef struct r_anal_plugin_t {
char *name;
char *desc;
char *license;
char *arch;
char *author;
char *version;
int bits;
int esil; 
int fileformat_type;
int (*init)(void *user);
int (*fini)(void *user);

int (*archinfo)(RAnal *anal, int query);
ut8* (*anal_mask)(RAnal *anal, int size, const ut8 *data, ut64 at);
RList* (*preludes)(RAnal *anal);


RAnalOpCallback op;


RAnalCmdExt cmd_ext;

RAnalRegProfCallback set_reg_profile;
RAnalRegProfGetCallback get_reg_profile;
RAnalFPBBCallback fingerprint_bb;
RAnalFPFcnCallback fingerprint_fcn;
RAnalDiffBBCallback diff_bb;
RAnalDiffFcnCallback diff_fcn;
RAnalDiffEvalCallback diff_eval;

RAnalEsilCB esil_init; 
RAnalEsilLoopCB esil_post_loop; 
RAnalEsilTrapCB esil_trap; 
RAnalEsilCB esil_fini; 
} RAnalPlugin;


int * (r_anal_compare) (RAnalFunction , RAnalFunction );


#if defined(R_API)

R_API RListRange* r_listrange_new (void);
R_API void r_listrange_free(RListRange *s);
R_API void r_listrange_add(RListRange *s, RAnalFunction *f);
R_API void r_listrange_del(RListRange *s, RAnalFunction *f);
R_API void r_listrange_resize(RListRange *s, RAnalFunction *f, int newsize);
R_API RAnalFunction *r_listrange_find_in_range(RListRange* s, ut64 addr);
R_API RAnalFunction *r_listrange_find_root(RListRange* s, ut64 addr);


R_API RAnalType *r_anal_type_new(void);
R_API void r_anal_type_add(RAnal *l, RAnalType *t);
R_API RAnalType *r_anal_type_find(RAnal *a, const char* name);
R_API void r_anal_type_list(RAnal *a, short category, short enabled);
R_API const char *r_anal_datatype_to_string(RAnalDataType t);
R_API RAnalType *r_anal_str_to_type(RAnal *a, const char* s);
R_API bool r_anal_op_nonlinear(int t);
R_API bool r_anal_op_ismemref(int t);
R_API const char *r_anal_optype_to_string(int t);
R_API int r_anal_optype_from_string(const char *type);
R_API const char *r_anal_op_family_to_string (int n);
R_API int r_anal_op_family_from_string(const char *f);
R_API int r_anal_op_hint(RAnalOp *op, RAnalHint *hint);
R_API RAnalType *r_anal_type_free(RAnalType *t);
R_API RAnalType *r_anal_type_loadfile(RAnal *a, const char *path);


typedef bool (*RAnalBlockCb)(RAnalBlock *block, void *user);
typedef bool (*RAnalAddrCb)(ut64 addr, void *user);


R_API void r_anal_block_ref(RAnalBlock *bb);
R_API void r_anal_block_unref(RAnalBlock *bb);



R_API RAnalBlock *r_anal_create_block(RAnal *anal, ut64 addr, ut64 size);

static inline bool r_anal_block_contains(RAnalBlock *bb, ut64 addr) {
return addr >= bb->addr && addr < bb->addr + bb->size;
}




R_API RAnalBlock *r_anal_block_split(RAnalBlock *bb, ut64 addr);





R_API bool r_anal_block_merge(RAnalBlock *a, RAnalBlock *b);



R_API void r_anal_delete_block(RAnalBlock *bb);

R_API void r_anal_block_set_size(RAnalBlock *block, ut64 size);



R_API bool r_anal_block_relocate(RAnalBlock *block, ut64 addr, ut64 size);

R_API RAnalBlock *r_anal_get_block_at(RAnal *anal, ut64 addr);
R_API bool r_anal_blocks_foreach_in(RAnal *anal, ut64 addr, RAnalBlockCb cb, void *user);
R_API RList *r_anal_get_blocks_in(RAnal *anal, ut64 addr); 
R_API void r_anal_blocks_foreach_intersect(RAnal *anal, ut64 addr, ut64 size, RAnalBlockCb cb, void *user);
R_API RList *r_anal_get_blocks_intersect(RAnal *anal, ut64 addr, ut64 size); 



R_API bool r_anal_block_successor_addrs_foreach(RAnalBlock *block, RAnalAddrCb cb, void *user);



R_API bool r_anal_block_recurse(RAnalBlock *block, RAnalBlockCb cb, void *user);


R_API RList *r_anal_block_recurse_list(RAnalBlock *block);


R_API R_NULLABLE RList *r_anal_block_shortest_path(RAnalBlock *block, ut64 dst);



R_API void r_anal_block_add_switch_case(RAnalBlock *block, ut64 switch_addr, ut64 case_addr);





R_API RAnalFunction *r_anal_function_new(RAnal *anal);
R_API void r_anal_function_free(void *fcn);


R_API bool r_anal_add_function(RAnal *anal, RAnalFunction *fcn);


R_API RAnalFunction *r_anal_create_function(RAnal *anal, const char *name, ut64 addr, int type, RAnalDiff *diff);


R_API RList *r_anal_get_functions_in(RAnal *anal, ut64 addr);


R_API RAnalFunction *r_anal_get_function_at(RAnal *anal, ut64 addr);

R_API bool r_anal_function_delete(RAnalFunction *fcn);



R_API bool r_anal_function_relocate(RAnalFunction *fcn, ut64 addr);



R_API bool r_anal_function_rename(RAnalFunction *fcn, const char *name);

R_API void r_anal_function_add_block(RAnalFunction *fcn, RAnalBlock *bb);
R_API void r_anal_function_remove_block(RAnalFunction *fcn, RAnalBlock *bb);




R_API ut64 r_anal_function_linear_size(RAnalFunction *fcn);


R_API ut64 r_anal_function_min_addr(RAnalFunction *fcn);


R_API ut64 r_anal_function_max_addr(RAnalFunction *fcn);


R_API ut64 r_anal_function_size_from_entry(RAnalFunction *fcn);



R_API ut64 r_anal_function_realsize(const RAnalFunction *fcn);



R_API bool r_anal_function_contains(RAnalFunction *fcn, ut64 addr);


R_API RAnal *r_anal_new(void);
R_API int r_anal_purge (RAnal *anal);
R_API RAnal *r_anal_free(RAnal *r);
R_API void r_anal_set_user_ptr(RAnal *anal, void *user);
R_API void r_anal_plugin_free (RAnalPlugin *p);
R_API int r_anal_add(RAnal *anal, RAnalPlugin *foo);
R_API int r_anal_archinfo(RAnal *anal, int query);
R_API bool r_anal_use(RAnal *anal, const char *name);
R_API bool r_anal_set_reg_profile(RAnal *anal);
R_API char *r_anal_get_reg_profile(RAnal *anal);
R_API ut64 r_anal_get_bbaddr(RAnal *anal, ut64 addr);
R_API bool r_anal_set_bits(RAnal *anal, int bits);
R_API bool r_anal_set_os(RAnal *anal, const char *os);
R_API void r_anal_set_cpu(RAnal *anal, const char *cpu);
R_API int r_anal_set_big_endian(RAnal *anal, int boolean);
R_API ut8 *r_anal_mask(RAnal *anal, int size, const ut8 *data, ut64 at);
R_API void r_anal_trace_bb(RAnal *anal, ut64 addr);
R_API const char *r_anal_fcntype_tostring(int type);
R_API int r_anal_fcn_bb (RAnal *anal, RAnalFunction *fcn, ut64 addr, int depth);
R_API void r_anal_bind(RAnal *b, RAnalBind *bnd);


R_API bool r_anal_set_triplet(RAnal *anal, const char *os, const char *arch, int bits);
R_API bool r_anal_set_fcnsign(RAnal *anal, const char *name);
R_API const char *r_anal_get_fcnsign(RAnal *anal, const char *sym);


R_API RAnalBlock *r_anal_bb_from_offset(RAnal *anal, ut64 off);
R_API bool r_anal_bb_set_offset(RAnalBlock *bb, int i, ut16 v);
R_API ut16 r_anal_bb_offset_inst(RAnalBlock *bb, int i);
R_API ut64 r_anal_bb_opaddr_i(RAnalBlock *bb, int i);
R_API ut64 r_anal_bb_opaddr_at(RAnalBlock *bb, ut64 addr);
R_API bool r_anal_bb_op_starts_at(RAnalBlock *bb, ut64 addr);
R_API ut64 r_anal_bb_size_i(RAnalBlock *bb, int i);


R_API const char *r_anal_stackop_tostring(int s);
R_API RAnalOp *r_anal_op_new(void);
R_API void r_anal_op_free(void *op);
R_API void r_anal_op_init(RAnalOp *op);
R_API bool r_anal_op_fini(RAnalOp *op);
R_API RAnalVar *get_link_var(RAnal *anal, ut64 faddr, RAnalVar *var);
R_API int r_anal_op_reg_delta(RAnal *anal, ut64 addr, const char *name);
R_API bool r_anal_op_is_eob(RAnalOp *op);
R_API RList *r_anal_op_list_new(void);
R_API int r_anal_op(RAnal *anal, RAnalOp *op, ut64 addr, const ut8 *data, int len, RAnalOpMask mask);
R_API RAnalOp *r_anal_op_hexstr(RAnal *anal, ut64 addr, const char *hexstr);
R_API char *r_anal_op_to_string(RAnal *anal, RAnalOp *op);

R_API RAnalEsil *r_anal_esil_new(int stacksize, int iotrap, unsigned int addrsize);
R_API void r_anal_esil_trace(RAnalEsil *esil, RAnalOp *op);
R_API void r_anal_esil_trace_list(RAnalEsil *esil);
R_API void r_anal_esil_trace_show(RAnalEsil *esil, int idx);
R_API bool r_anal_esil_set_pc(RAnalEsil *esil, ut64 addr);
R_API bool r_anal_esil_setup(RAnalEsil *esil, RAnal *anal, int romem, int stats, int nonull);
R_API void r_anal_esil_free(RAnalEsil *esil);
R_API bool r_anal_esil_runword(RAnalEsil *esil, const char *word);
R_API bool r_anal_esil_parse(RAnalEsil *esil, const char *str);
R_API bool r_anal_esil_dumpstack(RAnalEsil *esil);
R_API int r_anal_esil_mem_read(RAnalEsil *esil, ut64 addr, ut8 *buf, int len);
R_API int r_anal_esil_mem_write(RAnalEsil *esil, ut64 addr, const ut8 *buf, int len);
R_API int r_anal_esil_reg_read(RAnalEsil *esil, const char *regname, ut64 *num, int *size);
R_API int r_anal_esil_reg_write(RAnalEsil *esil, const char *dst, ut64 num);
R_API bool r_anal_esil_pushnum(RAnalEsil *esil, ut64 num);
R_API bool r_anal_esil_push(RAnalEsil *esil, const char *str);
R_API char *r_anal_esil_pop(RAnalEsil *esil);
R_API bool r_anal_esil_set_op(RAnalEsil *esil, const char *op, RAnalEsilOpCb code, ut32 push, ut32 pop, ut32 type);
R_API void r_anal_esil_stack_free(RAnalEsil *esil);
R_API int r_anal_esil_get_parm_type(RAnalEsil *esil, const char *str);
R_API int r_anal_esil_get_parm(RAnalEsil *esil, const char *str, ut64 *num);
R_API int r_anal_esil_condition(RAnalEsil *esil, const char *str);


R_API void r_anal_esil_interrupts_init(RAnalEsil *esil);
R_API RAnalEsilInterrupt *r_anal_esil_interrupt_new(RAnalEsil *esil, ut32 src_id, RAnalEsilInterruptHandler *ih);
R_API void r_anal_esil_interrupt_free(RAnalEsil *esil, RAnalEsilInterrupt *intr);
R_API bool r_anal_esil_set_interrupt(RAnalEsil *esil, RAnalEsilInterrupt *intr);
R_API int r_anal_esil_fire_interrupt(RAnalEsil *esil, ut32 intr_num);
R_API bool r_anal_esil_load_interrupts(RAnalEsil *esil, RAnalEsilInterruptHandler **handlers, ut32 src_id);
R_API bool r_anal_esil_load_interrupts_from_lib(RAnalEsil *esil, const char *path);
R_API void r_anal_esil_interrupts_fini(RAnalEsil *esil);

R_API void r_anal_esil_mem_ro(RAnalEsil *esil, int mem_readonly);
R_API void r_anal_esil_stats(RAnalEsil *esil, int enable);


R_API void r_anal_esil_session_list(RAnalEsil *esil);
R_API RAnalEsilSession *r_anal_esil_session_add(RAnalEsil *esil);
R_API void r_anal_esil_session_set(RAnalEsil *esil, RAnalEsilSession *session);
R_API void r_anal_esil_session_free(void *p);


R_API void r_anal_pin_init(RAnal *a);
R_API void r_anal_pin_fini(RAnal *a);
R_API void r_anal_pin (RAnal *a, ut64 addr, const char *name);
R_API void r_anal_pin_unset (RAnal *a, ut64 addr);
R_API const char *r_anal_pin_call(RAnal *a, ut64 addr);
R_API void r_anal_pin_list(RAnal *a);


R_API ut32 r_anal_function_cost(RAnalFunction *fcn);
R_API int r_anal_function_count_edges(const RAnalFunction *fcn, R_NULLABLE int *ebbs);


R_DEPRECATE R_API RAnalFunction *r_anal_get_fcn_in(RAnal *anal, ut64 addr, int type);
R_DEPRECATE R_API RAnalFunction *r_anal_get_fcn_in_bounds(RAnal *anal, ut64 addr, int type);

R_API RAnalFunction *r_anal_get_function_byname(RAnal *anal, const char *name);

R_API int r_anal_fcn(RAnal *anal, RAnalFunction *fcn, ut64 addr, ut64 len, int reftype);
R_API int r_anal_fcn_del(RAnal *anal, ut64 addr);
R_API int r_anal_fcn_del_locs(RAnal *anal, ut64 addr);
R_API bool r_anal_fcn_add_bb(RAnal *anal, RAnalFunction *fcn,
ut64 addr, ut64 size,
ut64 jump, ut64 fail, R_BORROW RAnalDiff *diff);
R_API bool r_anal_check_fcn(RAnal *anal, ut8 *buf, ut16 bufsz, ut64 addr, ut64 low, ut64 high);
R_API void r_anal_fcn_invalidate_read_ahead_cache(void);

R_API void r_anal_function_check_bp_use(RAnalFunction *fcn);


#define R_ANAL_FCN_VARKIND_LOCAL 'v'
#define R_ANAL_FCN_VARKIND_ARG 'a'
#define R_ANAL_FCN_VARKIND_FASTARG 'A'

#define r_anal_fcn_local_del_index(x,y,z) r_anal_fcn_var_del_byindex(x, y,R_ANAL_FCN_VARKIND_LOCAL, z)

#define r_anal_fcn_local_del_name(x,y,z) error


R_API int r_anal_fcn_var_del_bydelta (RAnal *a, ut64 fna, const char kind, int scope, ut32 delta);
R_API int r_anal_fcn_var_del_byindex (RAnal *a, ut64 fna, const char kind, int scope, ut32 idx);

R_API int r_anal_var_count(RAnal *a, RAnalFunction *fcn, int kind, int type);


R_API bool r_anal_var_display(RAnal *anal, int delta, char kind, const char *type);

R_API int r_anal_function_complexity(RAnalFunction *fcn);
R_API int r_anal_function_loops(RAnalFunction *fcn);
R_API RAnalVar *r_anal_fcn_get_var(RAnalFunction *fs, int num, int dir);
R_API void r_anal_trim_jmprefs(RAnal *anal, RAnalFunction *fcn);
R_API void r_anal_del_jmprefs(RAnal *anal, RAnalFunction *fcn);
R_API char *r_anal_function_get_json(RAnalFunction *function);
R_API RAnalFunction *r_anal_fcn_next(RAnal *anal, ut64 addr);
R_API char *r_anal_function_get_signature(RAnalFunction *function);
R_API int r_anal_str_to_fcn(RAnal *a, RAnalFunction *f, const char *_str);
R_API int r_anal_fcn_count (RAnal *a, ut64 from, ut64 to);
R_API RAnalBlock *r_anal_fcn_bbget_in(const RAnal *anal, RAnalFunction *fcn, ut64 addr);
R_API RAnalBlock *r_anal_fcn_bbget_at(RAnal *anal, RAnalFunction *fcn, ut64 addr);
R_API bool r_anal_fcn_bbadd(RAnalFunction *fcn, RAnalBlock *bb);
R_API int r_anal_function_resize(RAnalFunction *fcn, int newsize);
R_API bool r_anal_function_purity(RAnalFunction *fcn);

typedef bool (* RAnalRefCmp)(RAnalRef *ref, void *data);
R_API RList *r_anal_ref_list_new(void);
R_API int r_anal_xrefs_count(RAnal *anal);
R_API const char *r_anal_xrefs_type_tostring(RAnalRefType type);
R_API RAnalRefType r_anal_xrefs_type(char ch);
R_API RList *r_anal_xrefs_get(RAnal *anal, ut64 to);
R_API RList *r_anal_refs_get(RAnal *anal, ut64 to);
R_API RList *r_anal_xrefs_get_from(RAnal *anal, ut64 from);
R_API void r_anal_xrefs_list(RAnal *anal, int rad);
R_API RList *r_anal_function_get_refs(RAnalFunction *fcn);
R_API RList *r_anal_function_get_xrefs(RAnalFunction *fcn);
R_API int r_anal_xrefs_from(RAnal *anal, RList *list, const char *kind, const RAnalRefType type, ut64 addr);
R_API int r_anal_xrefs_set(RAnal *anal, ut64 from, ut64 to, const RAnalRefType type);
R_API int r_anal_xrefs_deln(RAnal *anal, ut64 from, ut64 to, const RAnalRefType type);
R_API int r_anal_xref_del(RAnal *anal, ut64 at, ut64 addr);

R_API RList* r_anal_fcn_get_vars(RAnalFunction *anal);
R_API RList* r_anal_get_fcns(RAnal *anal);


R_API void r_anal_remove_parsed_type(RAnal *anal, const char *name);
R_API void r_anal_save_parsed_type(RAnal *anal, const char *parsed);


R_API void r_anal_var_access_clear (RAnal *a, ut64 var_addr, int scope, int index);
R_API int r_anal_var_access (RAnal *a, ut64 var_addr, char kind, int scope, int delta, int ptr, int xs_type, ut64 xs_addr);
R_API RAnalVar *r_anal_var_new(void);
R_API int r_anal_var_rename (RAnal *a, ut64 var_addr, int scope, char kind,
const char *old_name, const char *new_name, bool verbose);
R_API bool r_anal_var_rebase(RAnal *a, RAnalFunction *fcn, ut64 diff);
R_API int r_anal_var_retype (RAnal *a, ut64 addr, int scope, int delta, char kind,
const char *type, int size, bool isarg, const char *name);
R_API RAnalVarAccess *r_anal_var_access_new(void);
R_API RList *r_anal_var_list_new(void);
R_API RList *r_anal_var_access_list_new(void);
R_API void r_anal_var_free(RAnalVar *var);
R_API void r_anal_var_access_free(void *access);
R_API int r_anal_var_delete_all (RAnal *a, ut64 addr, const char kind);
R_API int r_anal_var_delete (RAnal *a, ut64 var_addr, const char kind, int scope, int delta);
R_API bool r_anal_var_delete_byname (RAnal *a, RAnalFunction *fcn, int type, const char *name);
R_API bool r_anal_var_add (RAnal *a, ut64 addr, int scope, int delta, char kind,
const char *type, int size, bool isarg, const char *name);
R_API int r_anal_var_del(RAnal *anal, RAnalFunction *fcn, int delta, int scope);
R_API ut64 r_anal_var_addr(RAnal *a, RAnalFunction *fcn, const char *name);
R_API RAnalVar *r_anal_var_get (RAnal *a, ut64 addr, char kind, int scope, int index);
R_API const char *r_anal_var_scope_to_str(RAnal *anal, int scope);
R_API int r_anal_var_access_add(RAnal *anal, RAnalVar *var, ut64 from, int set);
R_API int r_anal_var_access_del(RAnal *anal, RAnalVar *var, ut64 from);
R_API RAnalVarAccess *r_anal_var_access_get(RAnal *anal, RAnalVar *var, ut64 from);
R_API RAnalVar *r_anal_var_get_byname (RAnal *anal, ut64 addr, const char* name);
R_API void r_anal_extract_vars(RAnal *anal, RAnalFunction *fcn, RAnalOp *op);
R_API void r_anal_extract_rarg(RAnal *anal, RAnalOp *op, RAnalFunction *fcn, int *reg_set, int *count);

typedef struct r_anal_fcn_vars_cache {
RList *bvars;
RList *rvars;
RList *svars;
} RAnalFcnVarsCache;
R_API void r_anal_fcn_vars_cache_init(RAnal *anal, RAnalFcnVarsCache *cache, RAnalFunction *fcn);
R_API void r_anal_fcn_vars_cache_fini(RAnalFcnVarsCache *cache);

R_API char *r_anal_fcn_format_sig(R_NONNULL RAnal *anal, R_NONNULL RAnalFunction *fcn, R_NULLABLE char *fcn_name,
R_NULLABLE RAnalFcnVarsCache *reuse_cache, R_NULLABLE const char *fcn_name_pre, R_NULLABLE const char *fcn_name_post);



R_API bool r_anal_xrefs_init (RAnal *anal);

#define R_ANAL_THRESHOLDFCN 0.7F
#define R_ANAL_THRESHOLDBB 0.7F


R_API RAnalDiff *r_anal_diff_new(void);
R_API void r_anal_diff_setup(RAnal *anal, int doops, double thbb, double thfcn);
R_API void r_anal_diff_setup_i(RAnal *anal, int doops, int thbb, int thfcn);
R_API void* r_anal_diff_free(RAnalDiff *diff);
R_API int r_anal_diff_fingerprint_bb(RAnal *anal, RAnalBlock *bb);
R_API size_t r_anal_diff_fingerprint_fcn(RAnal *anal, RAnalFunction *fcn);
R_API bool r_anal_diff_bb(RAnal *anal, RAnalFunction *fcn, RAnalFunction *fcn2);
R_API int r_anal_diff_fcn(RAnal *anal, RList *fcns, RList *fcns2);
R_API int r_anal_diff_eval(RAnal *anal);


R_API RAnalValue *r_anal_value_new(void);
R_API RAnalValue *r_anal_value_copy (RAnalValue *ov);
R_API RAnalValue *r_anal_value_new_from_string(const char *str);
R_API st64 r_anal_value_eval(RAnalValue *value);
R_API char *r_anal_value_to_string (RAnalValue *value);
R_API ut64 r_anal_value_to_ut64(RAnal *anal, RAnalValue *val);
R_API int r_anal_value_set_ut64(RAnal *anal, RAnalValue *val, ut64 num);
R_API void r_anal_value_free(RAnalValue *value);

R_API RAnalCond *r_anal_cond_new(void);
R_API RAnalCond *r_anal_cond_new_from_op(RAnalOp *op);
R_API void r_anal_cond_fini(RAnalCond *c);
R_API void r_anal_cond_free(RAnalCond *c);
R_API char *r_anal_cond_to_string(RAnalCond *cond);
R_API int r_anal_cond_eval(RAnal *anal, RAnalCond *cond);
R_API RAnalCond *r_anal_cond_new_from_string(const char *str);
R_API const char *r_anal_cond_tostring(int cc);


R_API bool r_anal_jmptbl(RAnal *anal, RAnalFunction *fcn, RAnalBlock *block, ut64 jmpaddr, ut64 table, ut64 tablesize, ut64 default_addr);


R_API bool try_get_delta_jmptbl_info(RAnal *anal, RAnalFunction *fcn, ut64 jmp_addr, ut64 lea_addr, ut64 *table_size, ut64 *default_case);
R_API bool try_walkthrough_jmptbl(RAnal *anal, RAnalFunction *fcn, RAnalBlock *block, int depth, ut64 ip, ut64 jmptbl_loc, ut64 jmptbl_off, ut64 sz, int jmptbl_size, ut64 default_case, bool ret0);
R_API bool try_get_jmptbl_info(RAnal *anal, RAnalFunction *fcn, ut64 addr, RAnalBlock *my_bb, ut64 *table_size, ut64 *default_case);
R_API int walkthrough_arm_jmptbl_style(RAnal *anal, RAnalFunction *fcn, RAnalBlock *block, int depth, ut64 ip, ut64 jmptbl_loc, ut64 sz, ut64 jmptbl_size, ut64 default_case, int ret0);


R_API RList* r_anal_reflines_get(RAnal *anal,
ut64 addr, const ut8 *buf, ut64 len, int nlines, int linesout, int linescall);
R_API int r_anal_reflines_middle(RAnal *anal, RList *list, ut64 addr, int len);
R_API RAnalRefStr *r_anal_reflines_str(void *core, ut64 addr, int opts);
R_API void r_anal_reflines_str_free(RAnalRefStr *refstr);

R_API void r_anal_var_list_show(RAnal *anal, RAnalFunction *fcn, int kind, int mode, PJ* pj);
R_API RList *r_anal_var_list(RAnal *anal, RAnalFunction *fcn, int kind);
R_API RList *r_anal_var_all_list(RAnal *anal, RAnalFunction *fcn);
R_API RList *r_anal_var_list_dynamic(RAnal *anal, RAnalFunction *fcn, int kind);


R_API bool r_anal_cc_exist(RAnal *anal, const char *convention);
R_API void r_anal_cc_del(RAnal *anal, const char *name);
R_API void r_anal_cc_set(RAnal *anal, const char *expr);
R_API char *r_anal_cc_get(RAnal *anal, const char *name);
R_API const char *r_anal_cc_arg(RAnal *anal, const char *convention, int n);
R_API const char *r_anal_cc_self(RAnal *anal, const char *convention);
R_API const char *r_anal_cc_error(RAnal *anal, const char *convention);
R_API int r_anal_cc_max_arg(RAnal *anal, const char *cc);
R_API const char *r_anal_cc_ret(RAnal *anal, const char *convention);
R_API const char *r_anal_cc_default(RAnal *anal);
R_API const char *r_anal_cc_func(RAnal *anal, const char *func_name);
R_API bool r_anal_noreturn_at(RAnal *anal, ut64 addr);

typedef struct r_anal_data_t {
ut64 addr;
int type;
ut64 ptr;
char *str;
int len;
ut8 *buf;
ut8 sbuf[8];
} RAnalData;

R_API RAnalData *r_anal_data (RAnal *anal, ut64 addr, const ut8 *buf, int size, int wordsize);
R_API const char *r_anal_data_kind (RAnal *anal, ut64 addr, const ut8 *buf, int len);
R_API RAnalData *r_anal_data_new_string (ut64 addr, const char *p, int size, int wide);
R_API RAnalData *r_anal_data_new (ut64 addr, int type, ut64 n, const ut8 *buf, int len);
R_API void r_anal_data_free (RAnalData *d);
#include <r_cons.h>
R_API char *r_anal_data_to_string(RAnalData *d, RConsPrintablePalette *pal);

R_API void r_meta_free(RAnal *m);
R_API RList *r_meta_find_list_in(RAnal *a, ut64 at, int type, int where);
R_API void r_meta_space_unset_for(RAnal *a, const RSpace *space);
R_API int r_meta_space_count_for(RAnal *a, const RSpace *space_name);
R_API RList *r_meta_enumerate(RAnal *a, int type);
R_API int r_meta_count(RAnal *m, int type, ut64 from, ut64 to);
R_API char *r_meta_get_string(RAnal *m, int type, ut64 addr);
R_API char *r_meta_get_var_comment (RAnal *a, int type, ut64 idx, ut64 addr);
R_API bool r_meta_set_string(RAnal *m, int type, ut64 addr, const char *s);
R_API bool r_meta_set_var_comment (RAnal *a, int type, ut64 idx, ut64 addr, const char *s);
R_API int r_meta_get_size(RAnal *a, int type);
R_API int r_meta_del(RAnal *m, int type, ut64 from, ut64 size);
R_API int r_meta_var_comment_del(RAnal *a, int type, ut64 idx, ut64 addr);
R_API int r_meta_add(RAnal *m, int type, ut64 from, ut64 to, const char *str);
R_API int r_meta_add_with_subtype(RAnal *m, int type, int subtype, ut64 from, ut64 to, const char *str);
R_API RAnalMetaItem *r_meta_find(RAnal *m, ut64 off, int type, int where);
R_API RAnalMetaItem *r_meta_find_any_except(RAnal *m, ut64 at, int type, int where);
R_API RAnalMetaItem *r_meta_find_in(RAnal *m, ut64 off, int type, int where);
R_API int r_meta_cleanup(RAnal *m, ut64 from, ut64 to);
R_API const char *r_meta_type_to_string(int type);
R_API RList *r_meta_enumerate(RAnal *a, int type);
R_API int r_meta_list(RAnal *m, int type, int rad);
R_API int r_meta_list_at(RAnal *m, int type, int rad, ut64 addr);
R_API int r_meta_list_cb(RAnal *m, int type, int rad, SdbForeachCallback cb, void *user, ut64 addr);
R_API void r_meta_list_offset(RAnal *m, ut64 addr, char input);
R_API void r_meta_item_free(void *_item);
R_API RAnalMetaItem *r_meta_item_new(int type);
R_API bool r_meta_deserialize_val(RAnal *a, RAnalMetaItem *it, int type, ut64 from, const char *v);
R_API void r_meta_print(RAnal *a, RAnalMetaItem *d, int rad, PJ *pj, bool show_full);
R_API void r_meta_set_data_at(RAnal *a, ut64 addr, ut64 wordsz);



R_API void r_anal_hint_del(RAnal *anal, ut64 addr, ut64 size); 
R_API void r_anal_hint_clear (RAnal *a);
R_API void r_anal_hint_free (RAnalHint *h);
R_API void r_anal_hint_set_syntax (RAnal *a, ut64 addr, const char *syn);
R_API void r_anal_hint_set_type(RAnal *a, ut64 addr, int type);
R_API void r_anal_hint_set_jump(RAnal *a, ut64 addr, ut64 jump);
R_API void r_anal_hint_set_fail(RAnal *a, ut64 addr, ut64 fail);
R_API void r_anal_hint_set_newbits(RAnal *a, ut64 addr, int bits);
R_API void r_anal_hint_set_nword(RAnal *a, ut64 addr, int nword);
R_API void r_anal_hint_set_offset(RAnal *a, ut64 addr, const char *typeoff);
R_API void r_anal_hint_set_immbase(RAnal *a, ut64 addr, int base);
R_API void r_anal_hint_set_size(RAnal *a, ut64 addr, ut64 size);
R_API void r_anal_hint_set_opcode(RAnal *a, ut64 addr, const char *str);
R_API void r_anal_hint_set_esil(RAnal *a, ut64 addr, const char *str);
R_API void r_anal_hint_set_pointer(RAnal *a, ut64 addr, ut64 ptr);
R_API void r_anal_hint_set_ret(RAnal *a, ut64 addr, ut64 val);
R_API void r_anal_hint_set_high(RAnal *a, ut64 addr);
R_API void r_anal_hint_set_stackframe(RAnal *a, ut64 addr, ut64 size);
R_API void r_anal_hint_set_val(RAnal *a, ut64 addr, ut64 v);
R_API void r_anal_hint_set_arch(RAnal *a, ut64 addr, R_NULLABLE const char *arch); 
R_API void r_anal_hint_set_bits(RAnal *a, ut64 addr, int bits); 
R_API void r_anal_hint_unset_val (RAnal *a, ut64 addr);
R_API void r_anal_hint_unset_high(RAnal *a, ut64 addr);
R_API void r_anal_hint_unset_immbase(RAnal *a, ut64 addr);
R_API void r_anal_hint_unset_nword(RAnal *a, ut64 addr);
R_API void r_anal_hint_unset_size(RAnal *a, ut64 addr);
R_API void r_anal_hint_unset_type(RAnal *a, ut64 addr);
R_API void r_anal_hint_unset_esil(RAnal *a, ut64 addr);
R_API void r_anal_hint_unset_opcode(RAnal *a, ut64 addr);
R_API void r_anal_hint_unset_syntax(RAnal *a, ut64 addr);
R_API void r_anal_hint_unset_pointer(RAnal *a, ut64 addr);
R_API void r_anal_hint_unset_ret(RAnal *a, ut64 addr);
R_API void r_anal_hint_unset_offset(RAnal *a, ut64 addr);
R_API void r_anal_hint_unset_jump(RAnal *a, ut64 addr);
R_API void r_anal_hint_unset_fail(RAnal *a, ut64 addr);
R_API void r_anal_hint_unset_newbits(RAnal *a, ut64 addr);
R_API void r_anal_hint_unset_stackframe(RAnal *a, ut64 addr);
R_API void r_anal_hint_unset_arch(RAnal *a, ut64 addr);
R_API void r_anal_hint_unset_bits(RAnal *a, ut64 addr);
R_API R_NULLABLE const RVector *r_anal_addr_hints_at(RAnal *anal, ut64 addr);
typedef bool (*RAnalAddrHintRecordsCb)(ut64 addr, const RVector *records, void *user);
R_API void r_anal_addr_hints_foreach(RAnal *anal, RAnalAddrHintRecordsCb cb, void *user);
typedef bool (*RAnalArchHintCb)(ut64 addr, R_NULLABLE const char *arch, void *user);
R_API void r_anal_arch_hints_foreach(RAnal *anal, RAnalArchHintCb cb, void *user);
typedef bool (*RAnalBitsHintCb)(ut64 addr, int bits, void *user);
R_API void r_anal_bits_hints_foreach(RAnal *anal, RAnalBitsHintCb cb, void *user);




R_API R_NULLABLE R_BORROW const char *r_anal_hint_arch_at(RAnal *anal, ut64 addr, R_NULLABLE ut64 *hint_addr);




R_API int r_anal_hint_bits_at(RAnal *anal, ut64 addr, R_NULLABLE ut64 *hint_addr);

R_API RAnalHint *r_anal_hint_get(RAnal *anal, ut64 addr); 


R_API RAnalSwitchOp *r_anal_switch_op_new(ut64 addr, ut64 min_val, ut64 max_val, ut64 def_val);
R_API void r_anal_switch_op_free(RAnalSwitchOp * swop);
R_API RAnalCaseOp* r_anal_switch_op_add_case(RAnalSwitchOp * swop, ut64 addr, ut64 value, ut64 jump);


R_API RAnalCycleFrame* r_anal_cycle_frame_new (void);
R_API void r_anal_cycle_frame_free (RAnalCycleFrame *cf);



R_API ut64 r_anal_fcn_label_get (RAnal *anal, RAnalFunction *fcn, const char *name);
R_API const char *r_anal_fcn_label_at (RAnal *anal, RAnalFunction *fcn, ut64 addr);
R_API int r_anal_fcn_label_set (RAnal *anal, RAnalFunction *fcn, const char *name, ut64 addr);
R_API int r_anal_fcn_label_del (RAnal *anal, RAnalFunction *fcn, const char *name, ut64 addr);
R_API int r_anal_fcn_labels (RAnal *anal, RAnalFunction *fcn, int rad);


R_API void r_anal_set_limits(RAnal *anal, ut64 from, ut64 to);
R_API void r_anal_unset_limits(RAnal *anal);


R_API int r_anal_esil_to_reil_setup (RAnalEsil *esil, RAnal *anal, int romem, int stats);


R_API void r_anal_noreturn_list(RAnal *anal, int mode);
R_API bool r_anal_noreturn_add(RAnal *anal, const char *name, ut64 addr);
R_API bool r_anal_noreturn_drop(RAnal *anal, const char *expr);
R_API bool r_anal_noreturn_at_addr(RAnal *anal, ut64 addr);


R_API int r_sign_space_count_for(RAnal *a, const RSpace *space);
R_API void r_sign_space_unset_for(RAnal *a, const RSpace *space);
R_API void r_sign_space_rename_for(RAnal *a, const RSpace *space, const char *oname, const char *nname);


typedef struct {
RAnal *anal;
RAnalCPPABI abi;
ut8 word_size;
bool (*read_addr) (RAnal *anal, ut64 addr, ut64 *buf);
} RVTableContext;

typedef struct vtable_info_t {
ut64 saddr; 
RVector methods;
} RVTableInfo;

typedef struct vtable_method_info_t {
ut64 addr; 
ut64 vtable_offset; 
} RVTableMethodInfo;

R_API void r_anal_vtable_info_free(RVTableInfo *vtable);
R_API ut64 r_anal_vtable_info_get_size(RVTableContext *context, RVTableInfo *vtable);
R_API bool r_anal_vtable_begin(RAnal *anal, RVTableContext *context);
R_API RVTableInfo *r_anal_vtable_parse_at(RVTableContext *context, ut64 addr);
R_API RList *r_anal_vtable_search(RVTableContext *context);
R_API void r_anal_list_vtables(RAnal *anal, int rad);


R_API char *r_anal_rtti_msvc_demangle_class_name(RVTableContext *context, const char *name);
R_API void r_anal_rtti_msvc_print_complete_object_locator(RVTableContext *context, ut64 addr, int mode);
R_API void r_anal_rtti_msvc_print_type_descriptor(RVTableContext *context, ut64 addr, int mode);
R_API void r_anal_rtti_msvc_print_class_hierarchy_descriptor(RVTableContext *context, ut64 addr, int mode);
R_API void r_anal_rtti_msvc_print_base_class_descriptor(RVTableContext *context, ut64 addr, int mode);
R_API bool r_anal_rtti_msvc_print_at_vtable(RVTableContext *context, ut64 addr, int mode, bool strict);
R_API void r_anal_rtti_msvc_recover_all(RVTableContext *vt_context, RList *vtables);

R_API void r_anal_rtti_itanium_print_class_type_info(RVTableContext *context, ut64 addr, int mode);
R_API void r_anal_rtti_itanium_print_si_class_type_info(RVTableContext *context, ut64 addr, int mode);
R_API void r_anal_rtti_itanium_print_vmi_class_type_info(RVTableContext *context, ut64 addr, int mode);
R_API void r_anal_rtti_itanium_print_at_vtable(RVTableContext *context, ut64 addr, int mode);

R_API char *r_anal_rtti_demangle_class_name(RAnal *anal, const char *name);
R_API void r_anal_rtti_print_at_vtable(RAnal *anal, ut64 addr, int mode);
R_API void r_anal_rtti_print_all(RAnal *anal, int mode);
R_API void r_anal_rtti_recover_all(RAnal *anal);

R_API void r_anal_colorize_bb(RAnal *anal, ut64 addr, ut32 color);

R_API RList *r_anal_preludes(RAnal *anal);
R_API bool r_anal_is_prelude(RAnal *anal, const ut8 *data, int len);


typedef struct r_anal_method_t {
char *name;
ut64 addr;
st64 vtable_offset; 
} RAnalMethod;

typedef struct r_anal_base_class_t {
char *id; 
ut64 offset; 
char *class_name;
} RAnalBaseClass;

typedef struct r_anal_vtable_t {
char *id; 
ut64 offset; 
ut64 addr; 
ut64 size; 
} RAnalVTable;

typedef enum {
R_ANAL_CLASS_ERR_SUCCESS = 0,
R_ANAL_CLASS_ERR_CLASH,
R_ANAL_CLASS_ERR_NONEXISTENT_ATTR,
R_ANAL_CLASS_ERR_NONEXISTENT_CLASS,
R_ANAL_CLASS_ERR_OTHER
} RAnalClassErr;

R_API void r_anal_class_create(RAnal *anal, const char *name);
R_API void r_anal_class_delete(RAnal *anal, const char *name);
R_API bool r_anal_class_exists(RAnal *anal, const char *name);
R_API SdbList *r_anal_class_get_all(RAnal *anal, bool sorted);
R_API void r_anal_class_foreach(RAnal *anal, SdbForeachCallback cb, void *user);
R_API RAnalClassErr r_anal_class_rename(RAnal *anal, const char *old_name, const char *new_name);

R_API void r_anal_class_method_fini(RAnalMethod *meth);
R_API RAnalClassErr r_anal_class_method_get(RAnal *anal, const char *class_name, const char *meth_name, RAnalMethod *meth);
R_API RVector *r_anal_class_method_get_all(RAnal *anal, const char *class_name);
R_API RAnalClassErr r_anal_class_method_set(RAnal *anal, const char *class_name, RAnalMethod *meth);
R_API RAnalClassErr r_anal_class_method_rename(RAnal *anal, const char *class_name, const char *old_meth_name, const char *new_meth_name);
R_API RAnalClassErr r_anal_class_method_delete(RAnal *anal, const char *class_name, const char *meth_name);

R_API void r_anal_class_base_fini(RAnalBaseClass *base);
R_API RAnalClassErr r_anal_class_base_get(RAnal *anal, const char *class_name, const char *base_id, RAnalBaseClass *base);
R_API RVector *r_anal_class_base_get_all(RAnal *anal, const char *class_name);
R_API RAnalClassErr r_anal_class_base_set(RAnal *anal, const char *class_name, RAnalBaseClass *base);
R_API RAnalClassErr r_anal_class_base_delete(RAnal *anal, const char *class_name, const char *base_id);

R_API void r_anal_class_vtable_fini(RAnalVTable *vtable);
R_API RAnalClassErr r_anal_class_vtable_get(RAnal *anal, const char *class_name, const char *vtable_id, RAnalVTable *vtable);
R_API RVector *r_anal_class_vtable_get_all(RAnal *anal, const char *class_name);
R_API RAnalClassErr r_anal_class_vtable_set(RAnal *anal, const char *class_name, RAnalVTable *vtable);
R_API RAnalClassErr r_anal_class_vtable_delete(RAnal *anal, const char *class_name, const char *vtable_id);

R_API void r_anal_class_list(RAnal *anal, int mode);
R_API void r_anal_class_list_bases(RAnal *anal, const char *class_name);
R_API void r_anal_class_list_vtables(RAnal *anal, const char *class_name);
R_API void r_anal_class_list_vtable_offset_functions(RAnal *anal, const char *class_name, ut64 offset);

R_API RAnalEsilCFG *r_anal_esil_cfg_expr(RAnalEsilCFG *cfg, RAnal *anal, const ut64 off, char *expr);
R_API RAnalEsilCFG *r_anal_esil_cfg_op(RAnalEsilCFG *cfg, RAnal *anal, RAnalOp *op);
R_API void r_anal_esil_cfg_merge_blocks(RAnalEsilCFG *cfg);
R_API void r_anal_esil_cfg_free(RAnalEsilCFG *cfg);

R_API RAnalEsilDFGNode *r_anal_esil_dfg_node_new (RAnalEsilDFG *edf, const char *c);
R_API RAnalEsilDFG *r_anal_esil_dfg_new(RReg *regs);
R_API void r_anal_esil_dfg_free(RAnalEsilDFG *dfg);
R_API RAnalEsilDFG *r_anal_esil_dfg_expr(RAnal *anal, RAnalEsilDFG *dfg, const char *expr);
R_API RStrBuf *r_anal_esil_dfg_filter (RAnalEsilDFG *dfg, const char *reg);
R_API RStrBuf *r_anal_esil_dfg_filter_expr(RAnal *anal, const char *expr, const char *reg);
R_API RList *r_anal_types_from_fcn(RAnal *anal, RAnalFunction *fcn);


extern RAnalPlugin r_anal_plugin_null;
extern RAnalPlugin r_anal_plugin_6502;
extern RAnalPlugin r_anal_plugin_6502_cs;
extern RAnalPlugin r_anal_plugin_8051;
extern RAnalPlugin r_anal_plugin_amd29k;
extern RAnalPlugin r_anal_plugin_arc;
extern RAnalPlugin r_anal_plugin_arm_cs;
extern RAnalPlugin r_anal_plugin_arm_gnu;
extern RAnalPlugin r_anal_plugin_avr;
extern RAnalPlugin r_anal_plugin_bf;
extern RAnalPlugin r_anal_plugin_chip8;
extern RAnalPlugin r_anal_plugin_cr16;
extern RAnalPlugin r_anal_plugin_cris;
extern RAnalPlugin r_anal_plugin_dalvik;
extern RAnalPlugin r_anal_plugin_ebc;
extern RAnalPlugin r_anal_plugin_gb;
extern RAnalPlugin r_anal_plugin_h8300;
extern RAnalPlugin r_anal_plugin_hexagon;
extern RAnalPlugin r_anal_plugin_i4004;
extern RAnalPlugin r_anal_plugin_i8080;
extern RAnalPlugin r_anal_plugin_java;
extern RAnalPlugin r_anal_plugin_m68k_cs;
extern RAnalPlugin r_anal_plugin_m680x_cs;
extern RAnalPlugin r_anal_plugin_malbolge;
extern RAnalPlugin r_anal_plugin_mcore;
extern RAnalPlugin r_anal_plugin_mips_cs;
extern RAnalPlugin r_anal_plugin_mips_gnu;
extern RAnalPlugin r_anal_plugin_msp430;
extern RAnalPlugin r_anal_plugin_nios2;
extern RAnalPlugin r_anal_plugin_or1k;
extern RAnalPlugin r_anal_plugin_pic;
extern RAnalPlugin r_anal_plugin_ppc_cs;
extern RAnalPlugin r_anal_plugin_ppc_gnu;
extern RAnalPlugin r_anal_plugin_propeller;
extern RAnalPlugin r_anal_plugin_riscv;
extern RAnalPlugin r_anal_plugin_riscv_cs;
extern RAnalPlugin r_anal_plugin_rsp;
extern RAnalPlugin r_anal_plugin_sh;
extern RAnalPlugin r_anal_plugin_snes;
extern RAnalPlugin r_anal_plugin_sparc_cs;
extern RAnalPlugin r_anal_plugin_sparc_gnu;
extern RAnalPlugin r_anal_plugin_sysz;
extern RAnalPlugin r_anal_plugin_tms320;
extern RAnalPlugin r_anal_plugin_tms320c64x;
extern RAnalPlugin r_anal_plugin_v810;
extern RAnalPlugin r_anal_plugin_v850;
extern RAnalPlugin r_anal_plugin_vax;
extern RAnalPlugin r_anal_plugin_wasm;
extern RAnalPlugin r_anal_plugin_ws;
extern RAnalPlugin r_anal_plugin_x86;
extern RAnalPlugin r_anal_plugin_x86_cs;
extern RAnalPlugin r_anal_plugin_x86_im;
extern RAnalPlugin r_anal_plugin_x86_simple;
extern RAnalPlugin r_anal_plugin_x86_udis;
extern RAnalPlugin r_anal_plugin_xap;
extern RAnalPlugin r_anal_plugin_xcore_cs;
extern RAnalPlugin r_anal_plugin_xtensa;
extern RAnalPlugin r_anal_plugin_z80;
#if defined(__cplusplus)
}
#endif

#endif
#endif
