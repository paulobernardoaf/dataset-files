
























#include "avr_disasm.h"
#include "errorcodes.h"

#if !defined(FORMAT_DISASM_H)
#define FORMAT_DISASM_H





#define OPERAND_PREFIX_REGISTER "r" 
#define OPERAND_PREFIX_DATA_HEX "0x" 
#define OPERAND_PREFIX_DATA_BIN "0b" 
#define OPERAND_PREFIX_DATA_DEC "" 
#define OPERAND_PREFIX_BIT "" 
#define OPERAND_PREFIX_IO_REGISTER "0x" 
#define OPERAND_PREFIX_ABSOLUTE_ADDRESS "0x" 
#define OPERAND_PREFIX_BRANCH_ADDRESS "." 
#define OPERAND_PREFIX_DES_ROUND "0x" 
#define OPERAND_PREFIX_WORD_DATA "0x" 








enum AVR_Formatting_Options {
FORMAT_OPTION_ADDRESS_LABEL = 1,
FORMAT_OPTION_ADDRESS = 2,
FORMAT_OPTION_DESTINATION_ADDRESS_COMMENT = 4,
FORMAT_OPTION_DATA_HEX = 8,
FORMAT_OPTION_DATA_BIN = 16,
FORMAT_OPTION_DATA_DEC = 32,
};







struct _formattingOptions {

int options;


char addressLabelPrefix[8];


int addressFieldWidth;
};
typedef struct _formattingOptions formattingOptions;



static int printDisassembledInstruction(char *out, const disassembledInstruction dInstruction, formattingOptions fOptions);
#if defined(_MSC_VER)
extern uint32_t AVR_Long_Address;
extern int AVR_Long_Instruction;
#endif
#endif
