






















#if !defined(ARC_DIS_H)
#define ARC_DIS_H

enum NullifyMode
{
BR_exec_when_no_jump,
BR_exec_always,
BR_exec_when_jump
};

enum ARC_Debugger_OperandType
{
ARC_UNDEFINED,
ARC_LIMM,
ARC_SHIMM,
ARC_REGISTER,
ARCOMPACT_REGISTER 


};

enum Flow
{
noflow,
direct_jump,
direct_call,
indirect_jump,
indirect_call,
invalid_instr
};

enum { no_reg = 99 };
enum { allOperandsSize = 256 };

struct arcDisState
{
void *_this;
int instructionLen;
void (*err)(void*, const char*);
const char *(*coreRegName)(void*, int);
const char *(*auxRegName)(void*, int);
const char *(*condCodeName)(void*, int);
const char *(*instName)(void*, int, int, int*);

unsigned char* instruction;
unsigned index;
const char *comm[6]; 

union {
unsigned int registerNum;
unsigned int shortimm;
unsigned int longimm;
} source_operand;
enum ARC_Debugger_OperandType sourceType;

int opWidth;
int targets[4];

unsigned int addresses[4];



enum Flow flow;
int register_for_indirect_jump;
int ea_reg1, ea_reg2, _offset;
int _cond, _opcode;
unsigned long words[2];
char *commentBuffer;
char instrBuffer[40];
char operandBuffer[allOperandsSize];
char _ea_present;
char _addrWriteBack; 
char _mem_load;
char _load_len;
enum NullifyMode nullifyMode;
unsigned char commNum;
unsigned char isBranch;
unsigned char tcnt;
unsigned char acnt;
};

#if 0
int ARCTangent_decodeInstr(bfd_vma address, disassemble_info* info);
#endif
int ARCompact_decodeInstr (bfd_vma address, disassemble_info* info);

#define __TRANSLATION_REQUIRED(state) ((state).acnt != 0)

#endif 
