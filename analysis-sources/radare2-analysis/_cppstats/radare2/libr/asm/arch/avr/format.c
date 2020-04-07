#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "format.h"
static int formatDisassembledOperand(char *strOperand, int operandNum, const disassembledInstruction dInstruction, formattingOptions fOptions);
static int printDisassembledInstruction(char *out, const disassembledInstruction dInstruction, formattingOptions fOptions) {
int retVal, i;
char strOperand[256];
*out = '\0';
if (AVR_Long_Instruction == AVR_LONG_INSTRUCTION_FOUND)
return 0;
strcat (out, dInstruction.instruction->mnemonic);
if (dInstruction.instruction->numOperands > 0)
strcat (out, " ");
for (i = 0; i < dInstruction.instruction->numOperands; i++) {
if (i > 0 && i != dInstruction.instruction->numOperands)
strcat (out, ", ");
retVal = formatDisassembledOperand(strOperand, i, dInstruction, fOptions);
if (retVal < 0)
return retVal;
strcat (out, strOperand);
}
return 1;
}
static int formatDisassembledOperand(char *strOperand, int operandNum, const disassembledInstruction dInstruction, formattingOptions fOptions) {
char binary[9];
int retVal;
if (operandNum >= AVR_MAX_NUM_OPERANDS)
return 0;
switch (dInstruction.instruction->operandTypes[operandNum]) {
case OPERAND_NONE:
case OPERAND_REGISTER_GHOST:
strOperand = NULL;
retVal = 0;
break;
case OPERAND_REGISTER:
case OPERAND_REGISTER_STARTR16:
case OPERAND_REGISTER_EVEN_PAIR_STARTR24:
case OPERAND_REGISTER_EVEN_PAIR:
retVal = sprintf (strOperand, "%s%d", OPERAND_PREFIX_REGISTER,
dInstruction.operands[operandNum]);
break;
case OPERAND_DATA:
case OPERAND_COMPLEMENTED_DATA:
if (fOptions.options & FORMAT_OPTION_DATA_BIN) {
int i;
for (i = 7; i >= 0; i--) {
if (dInstruction.operands[operandNum] & (1<<i))
binary[7-i] = '1';
else
binary[7-i] = '0';
}
binary[8] = '\0';
retVal = sprintf(strOperand, "%s%s",
OPERAND_PREFIX_DATA_BIN, binary);
} else if (fOptions.options & FORMAT_OPTION_DATA_DEC) {
retVal = sprintf(strOperand, "%s%d",
OPERAND_PREFIX_DATA_DEC,
dInstruction.operands[operandNum]);
} else {
retVal = sprintf(strOperand, "%s%02x",
OPERAND_PREFIX_DATA_HEX,
dInstruction.operands[operandNum]);
}
break;
case OPERAND_BIT:
retVal = sprintf(strOperand, "%s%d", OPERAND_PREFIX_BIT,
dInstruction.operands[operandNum]);
break;
case OPERAND_BRANCH_ADDRESS:
case OPERAND_RELATIVE_ADDRESS:
#if 0
if ((fOptions.options & FORMAT_OPTION_ADDRESS_LABEL) && fOptions.addressLabelPrefix != NULL) {
retVal = sprintf(strOperand, "%s%0*X",
fOptions.addressLabelPrefix,
fOptions.addressFieldWidth,
dInstruction.address+dInstruction.operands[operandNum]+2);
} else {
#endif
#if 0
if (dInstruction.operands[operandNum] > 0) {
retVal = sprintf(strOperand, "0x%x", dInstruction.address + dInstruction.operands[operandNum]);
} else {
}
#endif
retVal = sprintf(strOperand, "0x%x",
dInstruction.address + dInstruction.operands[operandNum]);
break;
case OPERAND_LONG_ABSOLUTE_ADDRESS:
retVal = sprintf(strOperand, "%s%0*x",
OPERAND_PREFIX_ABSOLUTE_ADDRESS,
fOptions.addressFieldWidth, AVR_Long_Address);
break;
case OPERAND_IO_REGISTER:
retVal = sprintf(strOperand, "%s%02x",
OPERAND_PREFIX_IO_REGISTER,
dInstruction.operands[operandNum]);
break;
case OPERAND_WORD_DATA:
retVal = sprintf (strOperand, "%s%0*x",
OPERAND_PREFIX_WORD_DATA,
fOptions.addressFieldWidth,
dInstruction.operands[operandNum]);
break;
case OPERAND_DES_ROUND:
retVal = sprintf (strOperand, "%s%02x",
OPERAND_PREFIX_WORD_DATA,
dInstruction.operands[operandNum]);
break;
case OPERAND_YPQ:
retVal = sprintf(strOperand, "y+%d",
dInstruction.operands[operandNum]);
break;
case OPERAND_ZPQ:
retVal = sprintf(strOperand, "z+%d",
dInstruction.operands[operandNum]);
break;
case OPERAND_X: retVal = sprintf(strOperand, "x"); break;
case OPERAND_XP: retVal = sprintf(strOperand, "x+"); break;
case OPERAND_MX: retVal = sprintf(strOperand, "-x"); break;
case OPERAND_Y: retVal = sprintf(strOperand, "y"); break;
case OPERAND_YP: retVal = sprintf(strOperand, "y+"); break;
case OPERAND_MY: retVal = sprintf(strOperand, "-y"); break;
case OPERAND_Z: retVal = sprintf(strOperand, "z"); break;
case OPERAND_ZP: retVal = sprintf(strOperand, "z+"); break;
case OPERAND_MZ: retVal = sprintf(strOperand, "-z"); break;
default: return ERROR_UNKNOWN_OPERAND;
}
if (retVal < 0)
return ERROR_MEMORY_ALLOCATION_ERROR;
return 0;
}
