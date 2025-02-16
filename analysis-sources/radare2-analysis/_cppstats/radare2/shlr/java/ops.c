#include <stdio.h>
#include "code.h"
#include "ops.h"
R_API struct java_op JAVA_OPS[JAVA_OPS_COUNT] = {
{ "nop" , 0x00, 1, R_ANAL_JAVA_NOP },
{ "aconst_null" , 0x01, 1, R_ANAL_JAVA_LDST_LOAD_FROM_CONST_REF_TO_STACK },
{ "iconst_m1" , 0x02, 1, R_ANAL_JAVA_LDST_LOAD_FROM_CONST_TO_STACK },
{ "iconst_0" , 0x03, 1, R_ANAL_JAVA_LDST_LOAD_FROM_CONST_TO_STACK },
{ "iconst_1" , 0x04, 1, R_ANAL_JAVA_LDST_LOAD_FROM_CONST_TO_STACK },
{ "iconst_2" , 0x05, 1, R_ANAL_JAVA_LDST_LOAD_FROM_CONST_TO_STACK },
{ "iconst_3" , 0x06, 1, R_ANAL_JAVA_LDST_LOAD_FROM_CONST_TO_STACK },
{ "iconst_4" , 0x07, 1, R_ANAL_JAVA_LDST_LOAD_FROM_CONST_TO_STACK },
{ "iconst_5" , 0x08, 1, R_ANAL_JAVA_LDST_LOAD_FROM_CONST_TO_STACK },
{ "lconst_0" , 0x09, 1, R_ANAL_JAVA_LDST_LOAD_FROM_CONST_TO_STACK },
{ "lconst_1" , 0x0a, 1, R_ANAL_JAVA_LDST_LOAD_FROM_CONST_TO_STACK },
{ "fconst_0" , 0x0b, 1, R_ANAL_JAVA_LDST_LOAD_FROM_CONST_TO_STACK },
{ "fconst_1" , 0x0c, 1, R_ANAL_JAVA_LDST_LOAD_FROM_CONST_TO_STACK },
{ "fconst_2" , 0x0d, 1, R_ANAL_JAVA_LDST_LOAD_FROM_CONST_TO_STACK },
{ "dconst_0" , 0x0e, 1, R_ANAL_JAVA_LDST_LOAD_FROM_CONST_TO_STACK },
{ "dconst_1" , 0x0f, 1, R_ANAL_JAVA_LDST_LOAD_FROM_CONST_TO_STACK },
{ "bipush" , 0x10, 2, R_ANAL_JAVA_LOAD_OP | R_ANAL_JAVA_STACK_OP | R_ANAL_JAVA_LDST_OP_PUSH },
{ "sipush" , 0x11, 3, R_ANAL_JAVA_LOAD_OP | R_ANAL_JAVA_STACK_OP | R_ANAL_JAVA_LDST_OP_PUSH },
{ "ldc" , 0x12, 2, R_ANAL_JAVA_LDST_LOAD_FROM_CONST_INDIRECT_TO_STACK },
{ "ldc_w" , 0x13, 3, R_ANAL_JAVA_LDST_LOAD_FROM_CONST_INDIRECT_TO_STACK },
{ "ldc2_w" , 0x14, 3, R_ANAL_JAVA_LDST_LOAD_FROM_CONST_INDIRECT_TO_STACK },
{ "iload" , 0x15, 2, R_ANAL_JAVA_LDST_LOAD_FROM_VAR_INDIRECT_TO_STACK },
{ "lload" , 0x16, 2, R_ANAL_JAVA_LDST_LOAD_FROM_VAR_INDIRECT_TO_STACK },
{ "fload" , 0x17, 2, R_ANAL_JAVA_LDST_LOAD_FROM_VAR_INDIRECT_TO_STACK },
{ "dload" , 0x18, 2, R_ANAL_JAVA_LDST_LOAD_FROM_VAR_INDIRECT_TO_STACK },
{ "aload" , 0x19, 2, R_ANAL_JAVA_LDST_LOAD_FROM_VAR_INDIRECT_TO_STACK_REF },
{ "iload_0" , 0x1a, 1, R_ANAL_JAVA_LDST_LOAD_FROM_VAR_TO_STACK },
{ "iload_1" , 0x1b, 1, R_ANAL_JAVA_LDST_LOAD_FROM_VAR_TO_STACK },
{ "iload_2" , 0x1c, 1, R_ANAL_JAVA_LDST_LOAD_FROM_VAR_TO_STACK },
{ "iload_3" , 0x1d, 1, R_ANAL_JAVA_LDST_LOAD_FROM_VAR_TO_STACK },
{ "lload_0" , 0x1e, 1, R_ANAL_JAVA_LDST_LOAD_FROM_VAR_TO_STACK },
{ "lload_1" , 0x1f, 1, R_ANAL_JAVA_LDST_LOAD_FROM_VAR_TO_STACK },
{ "lload_2" , 0x20, 1, R_ANAL_JAVA_LDST_LOAD_FROM_VAR_TO_STACK },
{ "lload_3" , 0x21, 1, R_ANAL_JAVA_LDST_LOAD_FROM_VAR_TO_STACK },
{ "fload_0" , 0x22, 1, R_ANAL_JAVA_LDST_LOAD_FROM_VAR_TO_STACK },
{ "fload_1" , 0x23, 1, R_ANAL_JAVA_LDST_LOAD_FROM_VAR_TO_STACK },
{ "fload_2" , 0x24, 1, R_ANAL_JAVA_LDST_LOAD_FROM_VAR_TO_STACK },
{ "fload_3" , 0x25, 1, R_ANAL_JAVA_LDST_LOAD_FROM_VAR_TO_STACK },
{ "dload_0" , 0x26, 1, R_ANAL_JAVA_LDST_LOAD_FROM_VAR_TO_STACK },
{ "dload_1" , 0x27, 1, R_ANAL_JAVA_LDST_LOAD_FROM_VAR_TO_STACK },
{ "dload_2" , 0x28, 1, R_ANAL_JAVA_LDST_LOAD_FROM_VAR_TO_STACK },
{ "dload_3" , 0x29, 1, R_ANAL_JAVA_LDST_LOAD_FROM_VAR_TO_STACK },
{ "aload_0" , 0x2a, 1, R_ANAL_JAVA_LDST_LOAD_FROM_VAR_TO_STACK_REF },
{ "aload_1" , 0x2b, 1, R_ANAL_JAVA_LDST_LOAD_FROM_VAR_TO_STACK_REF },
{ "aload_2" , 0x2c, 1, R_ANAL_JAVA_LDST_LOAD_FROM_VAR_TO_STACK_REF },
{ "aload_3" , 0x2d, 1, R_ANAL_JAVA_LDST_LOAD_FROM_VAR_TO_STACK_REF },
{ "iaload" , 0x2e, 1, R_ANAL_JAVA_LDST_LOAD_FROM_REF_INDIRECT_TO_STACK },
{ "laload" , 0x2f, 1, R_ANAL_JAVA_LDST_LOAD_FROM_REF_INDIRECT_TO_STACK },
{ "faload" , 0x30, 1, R_ANAL_JAVA_LDST_LOAD_FROM_REF_INDIRECT_TO_STACK },
{ "daload" , 0x31, 1, R_ANAL_JAVA_LDST_LOAD_FROM_REF_INDIRECT_TO_STACK },
{ "aaload" , 0x32, 1, R_ANAL_JAVA_LDST_LOAD_FROM_REF_INDIRECT_TO_STACK_REF },
{ "baload" , 0x33, 1, R_ANAL_JAVA_LDST_LOAD_FROM_REF_INDIRECT_TO_STACK },
{ "caload" , 0x34, 1, R_ANAL_JAVA_LDST_LOAD_FROM_REF_INDIRECT_TO_STACK },
{ "saload" , 0x35, 1, R_ANAL_JAVA_LDST_LOAD_FROM_REF_INDIRECT_TO_STACK },
{ "istore" , 0x36, 2, R_ANAL_JAVA_LDST_STORE_FROM_STACK_INDIRECT_TO_VAR },
{ "lstore" , 0x37, 2, R_ANAL_JAVA_LDST_STORE_FROM_STACK_INDIRECT_TO_VAR },
{ "fstore" , 0x38, 2, R_ANAL_JAVA_LDST_STORE_FROM_STACK_INDIRECT_TO_VAR },
{ "dstore" , 0x39, 2, R_ANAL_JAVA_LDST_STORE_FROM_STACK_INDIRECT_TO_VAR },
{ "astore" , 0x3a, 2, R_ANAL_JAVA_LDST_STORE_FROM_STACK_INDIRECT_TO_VAR_REF },
{ "istore_0" , 0x3b, 1, R_ANAL_JAVA_LDST_STORE_FROM_STACK_TO_VAR },
{ "istore_1" , 0x3c, 1, R_ANAL_JAVA_LDST_STORE_FROM_STACK_TO_VAR },
{ "istore_2" , 0x3d, 1, R_ANAL_JAVA_LDST_STORE_FROM_STACK_TO_VAR },
{ "istore_3" , 0x3e, 1, R_ANAL_JAVA_LDST_STORE_FROM_STACK_TO_VAR },
{ "lstore_0" , 0x3f, 1, R_ANAL_JAVA_LDST_STORE_FROM_STACK_TO_VAR },
{ "lstore_1" , 0x40, 1, R_ANAL_JAVA_LDST_STORE_FROM_STACK_TO_VAR },
{ "lstore_2" , 0x41, 1, R_ANAL_JAVA_LDST_STORE_FROM_STACK_TO_VAR },
{ "lstore_3" , 0x42, 1, R_ANAL_JAVA_LDST_STORE_FROM_STACK_TO_VAR },
{ "fstore_0" , 0x43, 1, R_ANAL_JAVA_LDST_STORE_FROM_STACK_TO_VAR },
{ "fstore_1" , 0x44, 1, R_ANAL_JAVA_LDST_STORE_FROM_STACK_TO_VAR },
{ "fstore_2" , 0x45, 1, R_ANAL_JAVA_LDST_STORE_FROM_STACK_TO_VAR },
{ "fstore_3" , 0x46, 1, R_ANAL_JAVA_LDST_STORE_FROM_STACK_TO_VAR },
{ "dstore_0" , 0x47, 1, R_ANAL_JAVA_LDST_STORE_FROM_STACK_TO_VAR },
{ "dstore_1" , 0x48, 1, R_ANAL_JAVA_LDST_STORE_FROM_STACK_TO_VAR },
{ "dstore_2" , 0x49, 1, R_ANAL_JAVA_LDST_STORE_FROM_STACK_TO_VAR },
{ "dstore_3" , 0x4a, 1, R_ANAL_JAVA_LDST_STORE_FROM_STACK_TO_VAR },
{ "astore_0" , 0x4b, 1, R_ANAL_JAVA_LDST_STORE_FROM_STACK_TO_VAR_REF },
{ "astore_1" , 0x4c, 1, R_ANAL_JAVA_LDST_STORE_FROM_STACK_TO_VAR_REF },
{ "astore_2" , 0x4d, 1, R_ANAL_JAVA_LDST_STORE_FROM_STACK_TO_VAR_REF },
{ "astore_3" , 0x4e, 1, R_ANAL_JAVA_LDST_STORE_FROM_STACK_TO_VAR_REF },
{ "iastore" , 0x4f, 1, R_ANAL_JAVA_LDST_STORE_FROM_STACK_INDIRECT_TO_REF },
{ "lastore" , 0x50, 1, R_ANAL_JAVA_LDST_STORE_FROM_STACK_INDIRECT_TO_REF },
{ "fastore" , 0x51, 1, R_ANAL_JAVA_LDST_STORE_FROM_STACK_INDIRECT_TO_REF },
{ "dastore" , 0x52, 1, R_ANAL_JAVA_LDST_STORE_FROM_STACK_INDIRECT_TO_REF },
{ "aastore" , 0x53, 1, R_ANAL_JAVA_LDST_STORE_FROM_STACK_INDIRECT_TO_REF_REF },
{ "bastore" , 0x54, 1, R_ANAL_JAVA_LDST_STORE_FROM_STACK_INDIRECT_TO_REF },
{ "castore" , 0x55, 1, R_ANAL_JAVA_LDST_STORE_FROM_STACK_INDIRECT_TO_REF },
{ "sastore" , 0x56, 1, R_ANAL_JAVA_LDST_STORE_FROM_STACK_INDIRECT_TO_REF },
{ "pop" , 0x57, 1, R_ANAL_JAVA_STORE_OP | R_ANAL_JAVA_STACK_OP | R_ANAL_JAVA_LDST_OP_POP },
{ "pop2" , 0x58, 1, R_ANAL_JAVA_STORE_OP | R_ANAL_JAVA_STACK_OP | R_ANAL_JAVA_LDST_OP_POP | R_ANAL_JAVA_REP_OP },
{ "dup" , 0x59, 1, R_ANAL_JAVA_LOAD_OP | R_ANAL_JAVA_STACK_OP | R_ANAL_JAVA_LDST_OP_PUSH | R_ANAL_JAVA_REP_OP },
{ "dup_x1" , 0x5a, 1, R_ANAL_JAVA_LOAD_OP | R_ANAL_JAVA_STACK_OP | R_ANAL_JAVA_LDST_OP_PUSH | R_ANAL_JAVA_REP_OP },
{ "dup_x2" , 0x5b, 1, R_ANAL_JAVA_LOAD_OP | R_ANAL_JAVA_STACK_OP | R_ANAL_JAVA_LDST_OP_PUSH | R_ANAL_JAVA_REP_OP },
{ "dup2" , 0x5c, 1, R_ANAL_JAVA_LOAD_OP | R_ANAL_JAVA_STACK_OP | R_ANAL_JAVA_LDST_OP_PUSH | R_ANAL_JAVA_REP_OP },
{ "dup2_x1" , 0x5d, 1, R_ANAL_JAVA_LOAD_OP | R_ANAL_JAVA_STACK_OP | R_ANAL_JAVA_LDST_OP_PUSH | R_ANAL_JAVA_REP_OP },
{ "dup2_x2" , 0x5e, 1, R_ANAL_JAVA_LOAD_OP | R_ANAL_JAVA_STACK_OP | R_ANAL_JAVA_LDST_OP_PUSH | R_ANAL_JAVA_REP_OP },
{ "swap" , 0x5f, 1, R_ANAL_JAVA_BINOP_XCHG },
{ "iadd" , 0x60, 1, R_ANAL_JAVA_BINOP_ADD },
{ "ladd" , 0x61, 1, R_ANAL_JAVA_BINOP_ADD },
{ "fadd" , 0x62, 1, R_ANAL_JAVA_BINOP_ADD },
{ "dadd" , 0x63, 1, R_ANAL_JAVA_BINOP_ADD },
{ "isub" , 0x64, 1, R_ANAL_JAVA_BINOP_SUB },
{ "lsub" , 0x65, 1, R_ANAL_JAVA_BINOP_SUB },
{ "fsub" , 0x66, 1, R_ANAL_JAVA_BINOP_SUB },
{ "dsub" , 0x67, 1, R_ANAL_JAVA_BINOP_SUB },
{ "imul" , 0x68, 1, R_ANAL_JAVA_BINOP_MUL },
{ "lmul" , 0x69, 1, R_ANAL_JAVA_BINOP_MUL },
{ "fmul" , 0x6a, 1, R_ANAL_JAVA_BINOP_MUL },
{ "dmul" , 0x6b, 1, R_ANAL_JAVA_BINOP_MUL },
{ "idiv" , 0x6c, 1, R_ANAL_JAVA_BINOP_DIV },
{ "ldiv" , 0x6d, 1, R_ANAL_JAVA_BINOP_DIV },
{ "fdiv" , 0x6e, 1, R_ANAL_JAVA_BINOP_DIV },
{ "ddiv" , 0x6f, 1, R_ANAL_JAVA_BINOP_DIV },
{ "irem" , 0x70, 1, R_ANAL_JAVA_BINOP_MOD },
{ "lrem" , 0x71, 1, R_ANAL_JAVA_BINOP_MOD },
{ "frem" , 0x72, 1, R_ANAL_JAVA_BINOP_MOD },
{ "drem" , 0x73, 1, R_ANAL_JAVA_BINOP_MOD },
{ "ineg" , 0x74, 1, R_ANAL_JAVA_BINOP_NOT },
{ "lneg" , 0x75, 1, R_ANAL_JAVA_BINOP_NOT },
{ "fneg" , 0x76, 1, R_ANAL_JAVA_BINOP_NOT },
{ "dneg" , 0x77, 1, R_ANAL_JAVA_BINOP_NOT },
{ "ishl" , 0x78, 1, R_ANAL_JAVA_BINOP_SHL },
{ "lshl" , 0x79, 1, R_ANAL_JAVA_BINOP_SHL },
{ "ishr" , 0x7a, 1, R_ANAL_JAVA_BINOP_SHR },
{ "lshr" , 0x7b, 1, R_ANAL_JAVA_BINOP_SHR },
{ "iushr" , 0x7c, 1, R_ANAL_JAVA_BINOP_SHR },
{ "lushr" , 0x7d, 1, R_ANAL_JAVA_BINOP_SHR },
{ "iand" , 0x7e, 1, R_ANAL_JAVA_BINOP_AND },
{ "land" , 0x7f, 1, R_ANAL_JAVA_BINOP_AND },
{ "ior" , 0x80, 1, R_ANAL_JAVA_BINOP_OR },
{ "lor" , 0x81, 1, R_ANAL_JAVA_BINOP_OR },
{ "ixor" , 0x82, 1, R_ANAL_JAVA_BINOP_XOR },
{ "lxor" , 0x83, 1, R_ANAL_JAVA_BINOP_XOR },
{ "iinc" , 0x84, 3, R_ANAL_JAVA_BINOP_ADD },
{ "i2l" , 0x85, 1, R_ANAL_JAVA_OBJOP_CAST | R_ANAL_JAVA_OBJ_OP },
{ "i2f" , 0x86, 1, R_ANAL_JAVA_OBJOP_CAST | R_ANAL_JAVA_OBJ_OP },
{ "i2d" , 0x87, 1, R_ANAL_JAVA_OBJOP_CAST | R_ANAL_JAVA_OBJ_OP },
{ "l2i" , 0x88, 1, R_ANAL_JAVA_OBJOP_CAST | R_ANAL_JAVA_OBJ_OP },
{ "l2f" , 0x89, 1, R_ANAL_JAVA_OBJOP_CAST | R_ANAL_JAVA_OBJ_OP },
{ "l2d" , 0x8a, 1, R_ANAL_JAVA_OBJOP_CAST | R_ANAL_JAVA_OBJ_OP },
{ "f2i" , 0x8b, 1, R_ANAL_JAVA_OBJOP_CAST | R_ANAL_JAVA_OBJ_OP },
{ "f2l" , 0x8c, 1, R_ANAL_JAVA_OBJOP_CAST | R_ANAL_JAVA_OBJ_OP },
{ "f2d" , 0x8d, 1, R_ANAL_JAVA_OBJOP_CAST | R_ANAL_JAVA_OBJ_OP },
{ "d2i" , 0x8e, 1, R_ANAL_JAVA_OBJOP_CAST | R_ANAL_JAVA_OBJ_OP },
{ "d2l" , 0x8f, 1, R_ANAL_JAVA_OBJOP_CAST | R_ANAL_JAVA_OBJ_OP },
{ "d2f" , 0x90, 1, R_ANAL_JAVA_OBJOP_CAST | R_ANAL_JAVA_OBJ_OP },
{ "i2b" , 0x91, 1, R_ANAL_JAVA_OBJOP_CAST | R_ANAL_JAVA_OBJ_OP },
{ "i2c" , 0x92, 1, R_ANAL_JAVA_OBJOP_CAST | R_ANAL_JAVA_OBJ_OP },
{ "i2s" , 0x93, 1, R_ANAL_JAVA_OBJOP_CAST | R_ANAL_JAVA_OBJ_OP },
{ "lcmp" , 0x94, 1, R_ANAL_JAVA_BINOP_CMP },
{ "fcmpl" , 0x95, 1, R_ANAL_JAVA_BINOP_CMP },
{ "fcmpg" , 0x96, 1, R_ANAL_JAVA_BINOP_CMP },
{ "dcmpl" , 0x97, 1, R_ANAL_JAVA_BINOP_CMP },
{ "dcmpg" , 0x98, 1, R_ANAL_JAVA_BINOP_CMP },
{ "ifeq" , 0x99, 3, R_ANAL_JAVA_CODEOP_CJMP | R_ANAL_JAVA_COND_EQ },
{ "ifne" , 0x9a, 3, R_ANAL_JAVA_CODEOP_CJMP | R_ANAL_JAVA_COND_NE },
{ "iflt" , 0x9b, 3, R_ANAL_JAVA_CODEOP_CJMP | R_ANAL_JAVA_COND_LT },
{ "ifge" , 0x9c, 3, R_ANAL_JAVA_CODEOP_CJMP | R_ANAL_JAVA_COND_GE },
{ "ifgt" , 0x9d, 3, R_ANAL_JAVA_CODEOP_CJMP | R_ANAL_JAVA_COND_GT },
{ "ifle" , 0x9e, 3, R_ANAL_JAVA_CODEOP_CJMP | R_ANAL_JAVA_COND_LE },
{ "if_icmpeq" , 0x9f, 3, R_ANAL_JAVA_CODEOP_CJMP | R_ANAL_JAVA_COND_EQ },
{ "if_icmpne" , 0xa0, 3, R_ANAL_JAVA_CODEOP_CJMP | R_ANAL_JAVA_COND_NE },
{ "if_icmplt" , 0xa1, 3, R_ANAL_JAVA_CODEOP_CJMP | R_ANAL_JAVA_COND_LT },
{ "if_icmpge" , 0xa2, 3, R_ANAL_JAVA_CODEOP_CJMP | R_ANAL_JAVA_COND_GE },
{ "if_icmpgt" , 0xa3, 3, R_ANAL_JAVA_CODEOP_CJMP | R_ANAL_JAVA_COND_GT },
{ "if_icmple" , 0xa4, 3, R_ANAL_JAVA_CODEOP_CJMP | R_ANAL_JAVA_COND_LE },
{ "if_acmpeq" , 0xa5, 3, R_ANAL_JAVA_CODEOP_CJMP | R_ANAL_JAVA_COND_EQ },
{ "if_acmpne" , 0xa6, 3, R_ANAL_JAVA_CODEOP_CJMP | R_ANAL_JAVA_COND_NE },
{ "goto" , 0xa7, 3, R_ANAL_JAVA_CODEOP_JMP },
{ "jsr" , 0xa8, 3, R_ANAL_JAVA_CODEOP_CALL },
{ "ret" , 0xa9, 2, R_ANAL_JAVA_CODEOP_RET },
{ "tableswitch" , 0xaa, 1, R_ANAL_JAVA_CODEOP_SWITCH },
{ "lookupswitch" , 0xab, 1, R_ANAL_JAVA_CODEOP_SWITCH },
{ "ireturn" , 0xac, 1, R_ANAL_JAVA_CODEOP_RET | R_ANAL_JAVA_RET_TYPE_PRIM },
{ "lreturn" , 0xad, 1, R_ANAL_JAVA_CODEOP_RET | R_ANAL_JAVA_RET_TYPE_PRIM },
{ "freturn" , 0xae, 1, R_ANAL_JAVA_CODEOP_RET | R_ANAL_JAVA_RET_TYPE_PRIM },
{ "dreturn" , 0xaf, 1, R_ANAL_JAVA_CODEOP_RET | R_ANAL_JAVA_RET_TYPE_PRIM },
{ "areturn" , 0xb0, 1, R_ANAL_JAVA_CODEOP_RET | R_ANAL_JAVA_RET_TYPE_REF },
{ "return" , 0xb1, 1, R_ANAL_JAVA_CODEOP_RET },
{ "getstatic" , 0xb2, 3, R_ANAL_JAVA_LDST_LOAD_GET_STATIC },
{ "putstatic" , 0xb3, 3, R_ANAL_JAVA_LDST_STORE_PUT_STATIC },
{ "getfield" , 0xb4, 3, R_ANAL_JAVA_LDST_LOAD_GET_FIELD },
{ "putfield" , 0xb5, 3, R_ANAL_JAVA_LDST_STORE_PUT_FIELD },
{ "invokevirtual" , 0xb6, 3, R_ANAL_JAVA_CODEOP_CALL },
{ "invokespecial" , 0xb7, 3, R_ANAL_JAVA_CODEOP_CALL },
{ "invokestatic" , 0xb8, 3, R_ANAL_JAVA_CODEOP_CALL },
{ "invokeinterface", 0xb9, 5, R_ANAL_JAVA_CODEOP_CALL },
{ "invokedynamic" , 0xba, 5, R_ANAL_JAVA_CODEOP_CALL },
{ "new" , 0xbb, 3, R_ANAL_JAVA_OBJOP_NEW },
{ "newarray" , 0xbc, 2, R_ANAL_JAVA_OBJOP_NEW },
{ "anewarray" , 0xbd, 3, R_ANAL_JAVA_OBJOP_NEW },
{ "arraylength" , 0xbe, 1, R_ANAL_JAVA_OBJOP_SIZE },
{ "athrow" , 0xbf, 1, R_ANAL_JAVA_CODEOP_TRAP },
{ "checkcast" , 0xc0, 3, R_ANAL_JAVA_CODEOP_CALL | R_ANAL_JAVA_OBJOP_CHECK | R_ANAL_JAVA_OBJOP_CAST },
{ "instanceof" , 0xc1, 3, R_ANAL_JAVA_CODEOP_CALL | R_ANAL_JAVA_OBJOP_CHECK },
{ "monitorenter" , 0xc2, 1, R_ANAL_JAVA_CODEOP_CALL | R_ANAL_JAVA_LOAD_OP | R_ANAL_JAVA_TYPE_REF },
{ "monitorexit" , 0xc3, 1, R_ANAL_JAVA_CODEOP_CALL | R_ANAL_JAVA_STORE_OP | R_ANAL_JAVA_TYPE_REF },
{ "wide" , 0xc4, 1, R_ANAL_JAVA_REP_OP },
{ "multinewarray" , 0xc5, 4, R_ANAL_JAVA_CODEOP_CALL | R_ANAL_JAVA_OBJOP_NEW | R_ANAL_JAVA_TYPE_REF },
{ "ifnull" , 0xc6, 3, R_ANAL_JAVA_CODEOP_CJMP | R_ANAL_JAVA_COND_NULL | R_ANAL_JAVA_COND_EQ },
{ "ifnonnull" , 0xc7, 3, R_ANAL_JAVA_CODEOP_CJMP | R_ANAL_JAVA_COND_NULL | R_ANAL_JAVA_COND_NE },
{ "goto_w" , 0xc8, 5, R_ANAL_JAVA_CODEOP_JMP },
{ "jsr_w" , 0xc9, 5, R_ANAL_JAVA_CODEOP_JMP | R_ANAL_JAVA_CODEOP_CALL },
{ "breakpoint" , 0xca, 1, R_ANAL_JAVA_CODEOP_SWI },
{ "undefined" , 0xcb, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xcc, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xcd, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xce, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xcf, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xd0, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xd1, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xd2, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xd3, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xd4, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xd5, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xd6, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xd7, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xd8, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xd9, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xda, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xdb, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xdc, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xdd, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xde, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xdf, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xe0, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xe1, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xe2, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xe3, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xe4, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xe5, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xe6, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xe7, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xe8, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xe9, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xea, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xeb, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xec, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xed, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xee, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xef, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xf0, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xf1, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xf2, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xf3, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xf4, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xf5, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xf6, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xf7, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xf8, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xf9, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xfa, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xfb, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xfc, 1, R_ANAL_JAVA_UNK_OP },
{ "undefined" , 0xfd, 1, R_ANAL_JAVA_UNK_OP },
{ "impdep1" , 0xfe, 1, R_ANAL_JAVA_ILL_OP },
{ "impdep2" , 0xff, 1, R_ANAL_JAVA_ILL_OP },
{ NULL, 0, 0, 0 }
};
