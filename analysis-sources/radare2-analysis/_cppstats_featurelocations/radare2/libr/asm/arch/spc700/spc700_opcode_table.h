#include <r_types.h>

#define SPC_OP 0
#define SPC_ARG8_1 1
#define SPC_ARG8_2 2
#define SPC_ARG16 3

typedef struct spc_700_op_t {
char *name;
ut8 type;
} Spc700Op;

static const Spc700Op spc_op_table[] = {
{"nop", SPC_OP},
{"call [0xffde]", SPC_OP},
{"set1 0x%02x.0", SPC_ARG8_1},
{"bbs 0x%02x.0,0x%02x", SPC_ARG8_2},
{"or A,0x%02x", SPC_ARG8_1},
{"or A,!0x%04x", SPC_ARG16},
{"or A,(X)", SPC_OP},
{"or A,[0x%02x+X]", SPC_ARG8_1},
{"or A,#0x%02x", SPC_ARG8_1},
{"or 0x%02x,0x%02x", SPC_ARG8_2},
{"or1 C,0x%02x.0x%02x", SPC_ARG8_2},
{"asl 0x%02x", SPC_ARG8_1},
{"asl !0x%04x", SPC_ARG16},
{"push psw", SPC_OP},
{"tset1 !0x%04x", SPC_ARG16},
{"brk", SPC_OP},
{"bpl 0x%02x", SPC_ARG8_1},
{"call [0xffdc]", SPC_OP},
{"clr1 0x%02x,0", SPC_ARG8_1},
{"bbc 0x%02x.0,0x%02x", SPC_ARG8_2},
{"or A,0x%02x+X", SPC_ARG8_1},
{"or A,!0x%04x+X", SPC_ARG8_1},
{"or A,!0x%04x+Y", SPC_ARG16},
{"or A,[0x%02x]+Y", SPC_ARG8_1},
{"or 0x%02x,#0x%02x", SPC_ARG8_2},
{"or (X),(Y)", SPC_OP},
{"decw 0x%02x", SPC_ARG8_1},
{"asl 0x%02x+X", SPC_ARG8_1},
{"asl A", SPC_OP},
{"dec X", SPC_OP},
{"cmp X,!0x%04x", SPC_ARG16},
{"jmp [!0x%04x+X]", SPC_ARG16},
{"clrp", SPC_OP},
{"call [0xffda]", SPC_OP},
{"set1 0x%02x.1", SPC_ARG8_1},
{"bbs 0x%02x.1,0x%02x", SPC_ARG8_2},
{"and A,0x%02x", SPC_ARG8_1},
{"and A,!0x%04x", SPC_ARG16},
{"and A,(X)", SPC_OP},
{"and A,[0x%02x+X]", SPC_ARG8_1},
{"and A,#0x%02x", SPC_ARG8_1},
{"and 0x%02x,0x%02x", SPC_ARG8_2},
{"or1 C,/0x%02x.0x%02x", SPC_ARG8_2},
{"rol 0x%02x", SPC_ARG8_1},
{"rol !0x%04x", SPC_ARG16},
{"push A", SPC_OP},
{"cbne 0x%02x,0x%02x", SPC_ARG8_2},
{"bra 0x%02x", SPC_ARG8_1},
{"bmi 0x%02x", SPC_ARG8_1},
{"call [0xffd8]", SPC_OP},
{"clr1 0x%02x.1", SPC_ARG8_1},
{"bbc 0x%02x.1,0x%02x", SPC_ARG8_2},
{"and A,0x%02x+X", SPC_ARG8_1},
{"and A,!0x%04x+X", SPC_ARG16},
{"and A,!0x%04x+Y", SPC_ARG16},
{"and A,[0x%02x]+Y", SPC_ARG8_1},
{"and 0x%02x,#0x%02x", SPC_ARG8_2},
{"and (X),(Y)", SPC_OP},
{"incw 0x%02x", SPC_ARG8_1},
{"rol 0x%02x+X", SPC_ARG8_1},
{"rol A", SPC_OP},
{"inc X", SPC_OP},
{"cmp X,0x%02x", SPC_ARG8_1},
{"call !0x%04x", SPC_ARG16},
{"call [0xffd6]", SPC_OP},
{"set1 0x%02x.2", SPC_ARG8_1},
{"bbs 0x%02x.2,0x%02x", SPC_ARG8_2},
{"setp", SPC_OP},
{"eor A,0x%02x", SPC_ARG8_1},
{"eor A,!0x%04x", SPC_ARG16},
{"eor A,(X)", SPC_OP},
{"eor A,[0x%02x+X]", SPC_ARG8_1},
{"eor A,#0x%02x", SPC_ARG8_1},
{"eor 0x%02x,0x%02x", SPC_ARG8_2},
{"and1 C,0x%02x.0x%02x", SPC_ARG8_2},
{"lsr 0x%02x", SPC_ARG8_1},
{"lsr !0x%04x", SPC_ARG16},
{"push X", SPC_OP},
{"tclr1 !0x%04x", SPC_ARG16},
{"pcall 0x%02x", SPC_ARG8_1},
{"bvc 0x%02x", SPC_ARG8_1},
{"call [0xffd4]", SPC_OP},
{"clr1 0x%02x.2", SPC_ARG8_1},
{"bbc 0x%02x.2,0x%02x", SPC_ARG8_2},
{"eor A,0x%02x+X", SPC_ARG8_1},
{"eor A,!0x%04x+X", SPC_ARG16},
{"eor A,!0x%04x+Y", SPC_ARG16},
{"eor A,[0x%02x]+Y", SPC_ARG8_1},
{"eor 0x%02x,#0x%02x", SPC_ARG8_2},
{"eor (X),(Y)", SPC_OP},
{"cmpw YA,0x%02x", SPC_ARG8_1},
{"lsr 0x%02x+X", SPC_ARG8_1},
{"lsr A", SPC_OP},
{"mov X,A", SPC_OP},
{"cmp Y,!0x%04x", SPC_ARG16},
{"jmp !0x%04x", SPC_ARG16},
{"clrc", SPC_OP},
{"call [0xffd2]", SPC_OP},
{"set1 0x%02x.3", SPC_ARG8_1},
{"bbs 0x%02x.3,0x%02x", SPC_ARG8_2},
{"cmp A,0x%02x", SPC_ARG8_1},
{"cmp A,!0x%04x", SPC_ARG16},
{"cmp A,(X)", SPC_OP},
{"cmp A,[0x%02x+X]", SPC_ARG8_1},
{"cmp A,#0x%02x", SPC_ARG8_1},
{"cmp 0x%02x,0x%02x", SPC_ARG8_2},
{"and1 C,/0x%02x.0x%02x", SPC_ARG8_2},
{"ror 0x%02x", SPC_ARG8_1},
{"ror !0x%04x", SPC_ARG16},
{"push Y", SPC_OP},
{"dbnz 0x%02x,0x%02x", SPC_ARG8_2},
{"ret", SPC_OP},
{"bvs 0x%02x", SPC_ARG8_1},
{"call [0xffd0]", SPC_OP},
{"clr1 0x%02x.3", SPC_ARG8_1},
{"bbc 0x%02x.3,0x%02x", SPC_ARG8_2},
{"cmp A,0x%02x+X", SPC_ARG8_1},
{"cmp A,!0x%04x+X", SPC_ARG16},
{"cmp A,!0x%04x+Y", SPC_ARG16},
{"cmp A,[0x%02x]+Y", SPC_ARG8_1},
{"cmp 0x%02x,#0x%02x", SPC_ARG8_2},
{"cmp (X),(Y)", SPC_OP},
{"addw YA,0x%02x", SPC_ARG8_1},
{"ror 0x%02x+X", SPC_ARG8_1},
{"ror A", SPC_OP},
{"mov A,X", SPC_OP},
{"cmp Y,0x%02x", SPC_ARG8_1},
{"ret1", SPC_OP},
{"setc", SPC_OP},
{"call [0xffce]", SPC_OP},
{"set1 0x%02x.4", SPC_ARG8_1},
{"bbs 0x%02x.4,0x%02x", SPC_ARG8_2},
{"adc A,0x%02x", SPC_ARG8_1},
{"adc A,!0x%04x", SPC_ARG16},
{"adc A,(X)", SPC_OP},
{"adc A,[0x%02x+X]", SPC_ARG8_1},
{"adc A,#0x%02x", SPC_ARG8_1},
{"adc 0x%02x,0x%02x", SPC_ARG8_2},
{"eor1 C,0x%02x.0x%02x", SPC_ARG8_2},
{"dec 0x%02x", SPC_ARG8_1},
{"dec !0x%04x", SPC_ARG16},
{"mov Y,#0x%02x", SPC_ARG8_1},
{"pop psw", SPC_OP},
{"mov 0x%02x,#0x%02x", SPC_ARG8_2},
{"bcc 0x%02x", SPC_ARG8_1},
{"call [0xffcc]", SPC_OP},
{"clr1 0x%02x.4", SPC_ARG8_1},
{"bbc 0x%02x.4,0x%02x", SPC_ARG8_2},
{"adc A,0x%02x+X", SPC_ARG8_1},
{"adc A,!0x%04x+X", SPC_ARG16},
{"adc A,!0x%04x+Y", SPC_ARG16},
{"adc A,[0x%02x]+Y", SPC_ARG8_1},
{"adc 0x%02x,#0x%02x", SPC_ARG8_2},
{"adc (X),(Y)", SPC_OP},
{"subw YA,0x%02x", SPC_ARG8_1},
{"dec 0x%02x+X", SPC_ARG8_1},
{"dec A", SPC_OP},
{"mov X,SP", SPC_OP},
{"div YA,X", SPC_OP},
{"xcn A", SPC_OP},
{"ei", SPC_OP},
{"call [0xffca]", SPC_OP},
{"set1 0x%02x.5", SPC_ARG8_1},
{"bbs 0x%02x.5,0x%02x", SPC_ARG8_2},
{"sbc A,0x%02x", SPC_ARG8_1},
{"sbc A,!0x%04x", SPC_ARG16},
{"sbc A,(X)", SPC_OP},
{"sbc A,[0x%02x+X]", SPC_ARG8_1},
{"sbc A,#0x%02x", SPC_ARG8_1},
{"sbc 0x%02x,0x%02x", SPC_ARG8_2},
{"mov1 C,0x%02x.0x%02x", SPC_ARG8_2},
{"inc 0x%02x", SPC_ARG8_1},
{"inc !0x%04x", SPC_ARG16},
{"cmp Y,#0x%02x", SPC_ARG8_1},
{"pop A", SPC_OP},
{"mov (X)+,A", SPC_OP},
{"bcs 0x%02x", SPC_ARG8_1},
{"call [0xffc8]", SPC_OP},
{"clr1 0x%02x.5", SPC_ARG8_1},
{"bbc 0x%02x.5,0x%02x", SPC_ARG8_2},
{"sbc A,0x%02x+X", SPC_ARG8_1},
{"sbc A,!0x%04x+X", SPC_ARG16},
{"sbc A,!0x%04x+Y", SPC_ARG16},
{"sbc A,[0x%02x]+Y", SPC_ARG8_1},
{"sbc 0x%02x,#0x%02x", SPC_ARG8_2},
{"sbc (X),(Y)", SPC_OP},
{"movw YA,0x%02x", SPC_ARG8_1},
{"inc 0x%02x+X", SPC_ARG8_1},
{"inc A", SPC_OP},
{"mov SP,X", SPC_OP},
{"das", SPC_OP},
{"mov A,(X)+", SPC_OP},
{"di", SPC_OP},
{"call [0xffc6]", SPC_OP},
{"set1 0x%02x.6", SPC_ARG8_1},
{"bbs 0x%02x.6,0x%02x", SPC_ARG8_2},
{"mov 0x%02x,A", SPC_ARG8_1},
{"mov !0x%04x,A", SPC_ARG16},
{"mov (X),A", SPC_OP},
{"mov [0x%02x+X],A", SPC_ARG8_1},
{"cmp X,#0x%02x", SPC_ARG8_1},
{"mov !0x%04x,X", SPC_ARG16},
{"and1 0x%02x.0x%02x,C", SPC_ARG8_2},
{"mov 0x%02x,Y", SPC_ARG8_1},
{"mov !0x%04x,Y", SPC_ARG16},
{"mov X,#0x%02x", SPC_ARG8_1},
{"pop X", SPC_OP},
{"mul YA", SPC_OP},
{"bne 0x%02x", SPC_ARG8_1},
{"call [0xffc4]", SPC_OP},
{"clr1 0x%02x.6", SPC_ARG8_1},
{"bbc 0x%02x.6,0x%02x", SPC_ARG8_2},
{"mov 0x%02x+X,A", SPC_ARG8_1},
{"mov !0x%04x+X,A", SPC_ARG16},
{"mov !0x%04x+Y,A", SPC_ARG16},
{"mov [0x%02x]+Y,A", SPC_ARG8_1},
{"mov 0x%02x,X", SPC_ARG8_1},
{"mov 0x%02x+Y,X", SPC_ARG8_1},
{"movw 0x%02x,YA", SPC_ARG8_1},
{"mov 0x%02x+X,Y", SPC_ARG8_1},
{"dec Y", SPC_OP},
{"mov A,Y", SPC_OP},
{"cbne 0x%02x+X,0x%02x", SPC_ARG8_2},
{"daa", SPC_OP},
{"clrv", SPC_OP},
{"call [0xffc2]", SPC_OP},
{"set1 0x%02x.7", SPC_ARG8_1},
{"bbs 0x%02x.7,0x%02x", SPC_ARG8_2},
{"mov A,0x%02x", SPC_ARG8_1},
{"mov A,!0x%04x", SPC_ARG16},
{"mov A,(X)", SPC_OP},
{"mov A,[0x%02x+X]", SPC_ARG8_1},
{"mov A,#0x%02x", SPC_ARG8_1},
{"mov X,!0x%04x", SPC_ARG16},
{"not1 0x%02x.0x%02x", SPC_ARG8_2},
{"mov Y,0x%02x", SPC_ARG8_1},
{"mov Y,!0x%04x", SPC_ARG16},
{"notc", SPC_OP},
{"pop Y", SPC_OP},
{"sleep", SPC_OP},
{"beq 0x%02x", SPC_ARG8_1},
{"call [0xffc0]", SPC_OP},
{"clr1 0x%02x.7", SPC_ARG8_1},
{"bbc 0x%02x.7,0x%02x", SPC_ARG8_2},
{"mov A,0x%02x+X", SPC_ARG8_1},
{"mov A,!0x%04x+X", SPC_ARG16},
{"mov A,!0x%04x+Y", SPC_ARG16},
{"mov A,[0x%02x]+Y", SPC_ARG8_1},
{"mov X,0x%02x", SPC_ARG8_1},
{"mov X,0x%02x+Y", SPC_ARG8_1},
{"mov 0x%02x,0x%02x", SPC_ARG8_2},
{"mov Y,0x%02x+X", SPC_ARG8_1},
{"inc Y", SPC_OP},
{"mov Y,A", SPC_OP},
{"dbnz Y,0x%02x", SPC_ARG8_1},
{"stop", SPC_OP}
};
