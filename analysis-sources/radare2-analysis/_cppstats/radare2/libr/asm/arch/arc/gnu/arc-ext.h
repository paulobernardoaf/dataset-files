#define IGNORE_FIRST_OPD 1
#define UNMANGLED
enum ExtOperType
{
EXT_INSTRUCTION = 0,
EXT_CORE_REGISTER = 1,
EXT_AUX_REGISTER = 2,
EXT_COND_CODE = 3,
EXT_INSTRUCTION32 = 4, 
EXT_AC_INSTRUCTION = 4, 
EXT_REMOVE_CORE_REG = 5,
EXT_LONG_CORE_REGISTER = 6,
EXT_AUX_REGISTER_EXTENDED = 7,
EXT_INSTRUCTION32_EXTENDED = 8,
EXT_CORE_REGISTER_CLASS = 9
};
enum ExtReadWrite
{
REG_INVALID,
REG_READ,
REG_WRITE,
REG_READWRITE
};
extern void build_ARC_extmap (void* text_bfd);
extern enum ExtReadWrite arcExtMap_coreReadWrite (int regnum);
extern const char* arcExtMap_coreRegName (int regnum);
extern const char* arcExtMap_auxRegName (long regnum);
extern const char* arcExtMap_condCodeName (int code);
extern const char* arcExtMap_instName (int opcode, int insn, int* flags);
void dump_ARC_extmap (void);
