#include <r_types.h>
enum {
SNES_OP_8BIT = 1,
SNES_OP_16BIT,
SNES_OP_24BIT,
SNES_OP_32BIT,
SNES_OP_IMM_M, 
SNES_OP_IMM_X 
};
typedef struct {
const char *name;
ut8 len;
ut8 flags;
} snes_op_t;
static int snes_op_get_size(int M_flag, int X_flag, snes_op_t* op) {
switch(op->len) {
case SNES_OP_IMM_M: return M_flag ? SNES_OP_16BIT : SNES_OP_24BIT;
case SNES_OP_IMM_X: return X_flag ? SNES_OP_16BIT : SNES_OP_24BIT;
default: return op->len;
}
}
static snes_op_t snes_op[]={
{"brk 0x%02x", SNES_OP_16BIT},
{"ora (0x%02x,x)", SNES_OP_16BIT},
{"cop 0x%02x", SNES_OP_16BIT},
{"ora 0x%02x,s", SNES_OP_16BIT},
{"tsb 0x%02x", SNES_OP_16BIT},
{"ora 0x%02x", SNES_OP_16BIT},
{"asl 0x%02x", SNES_OP_16BIT},
{"ora [0x%02x]", SNES_OP_16BIT},
{"php", SNES_OP_8BIT},
{"ora", SNES_OP_IMM_M},
{"asl a", SNES_OP_8BIT},
{"phd", SNES_OP_8BIT},
{"tsb 0x%04x", SNES_OP_24BIT},
{"ora 0x%04x", SNES_OP_24BIT},
{"asl 0x%04x", SNES_OP_24BIT},
{"ora 0x%06x", SNES_OP_32BIT},
{"bpl 0x%06x", SNES_OP_16BIT},
{"ora (0x%02x),y", SNES_OP_16BIT},
{"ora (0x%02x)", SNES_OP_16BIT},
{"ora (0x%02x,s),y", SNES_OP_16BIT},
{"trb 0x%02x", SNES_OP_16BIT},
{"ora 0x%02x,x", SNES_OP_16BIT},
{"asl 0x%02x,x", SNES_OP_16BIT},
{"ora [0x%02x],y", SNES_OP_16BIT},
{"clc", SNES_OP_8BIT},
{"ora 0x%04x,y", SNES_OP_24BIT},
{"inc a", SNES_OP_8BIT},
{"tas", SNES_OP_8BIT},
{"trb 0x%04x", SNES_OP_24BIT},
{"ora 0x%04x,x", SNES_OP_24BIT},
{"asl 0x%04x,x", SNES_OP_24BIT},
{"ora 0x%06x,x", SNES_OP_32BIT},
{"jsr 0x%04x", SNES_OP_24BIT},
{"and (0x%02x,x)", SNES_OP_16BIT},
{"jsr 0x%06x", SNES_OP_32BIT},
{"and 0x%02x,s", SNES_OP_16BIT},
{"bit 0x%02x", SNES_OP_16BIT},
{"and 0x%02x", SNES_OP_16BIT},
{"rol 0x%02x", SNES_OP_16BIT},
{"and [0x%02x]", SNES_OP_16BIT},
{"plp", SNES_OP_8BIT},
{"and", SNES_OP_IMM_M},
{"rol a", SNES_OP_8BIT},
{"pld", SNES_OP_8BIT},
{"bit 0x%04x", SNES_OP_24BIT},
{"and 0x%04x", SNES_OP_24BIT},
{"rol 0x%04x", SNES_OP_24BIT},
{"and 0x%06x", SNES_OP_32BIT},
{"bmi 0x%06x", SNES_OP_16BIT},
{"and (0x%02x),y", SNES_OP_16BIT},
{"and (0x%02x)", SNES_OP_16BIT},
{"and (0x%02x,s),y", SNES_OP_16BIT},
{"bit 0x%02x,x", SNES_OP_16BIT},
{"and 0x%02x,x", SNES_OP_16BIT},
{"rol 0x%02x,x", SNES_OP_16BIT},
{"and [0x%02x],y", SNES_OP_16BIT},
{"sec", SNES_OP_8BIT},
{"and 0x%04x,y", SNES_OP_24BIT},
{"dec a", SNES_OP_8BIT},
{"tsa", SNES_OP_8BIT},
{"bit 0x%04x,x", SNES_OP_24BIT},
{"and 0x%04x,x", SNES_OP_24BIT},
{"rol 0x%04x,x", SNES_OP_24BIT},
{"and 0x%06x,x", SNES_OP_32BIT},
{"rti", SNES_OP_8BIT},
{"eor (0x%02x,x)", SNES_OP_16BIT},
{"wdm", SNES_OP_8BIT},
{"eor 0x%02x,s", SNES_OP_16BIT},
{"mvp 0x%02x,0x%02x", SNES_OP_24BIT},
{"eor 0x%02x", SNES_OP_16BIT},
{"lsr 0x%02x", SNES_OP_16BIT},
{"eor [0x%02x]", SNES_OP_16BIT},
{"pha", SNES_OP_8BIT},
{"eor", SNES_OP_IMM_M},
{"lsr a", SNES_OP_8BIT},
{"phk", SNES_OP_8BIT},
{"jmp 0x%04x", SNES_OP_24BIT},
{"eor 0x%04x", SNES_OP_24BIT},
{"lsr 0x%04x", SNES_OP_24BIT},
{"eor 0x%06x", SNES_OP_32BIT},
{"bvc 0x%06x", SNES_OP_16BIT},
{"eor (0x%02x),y", SNES_OP_16BIT},
{"eor (0x%02x)", SNES_OP_16BIT},
{"eor (0x%02x,s),y", SNES_OP_16BIT},
{"mvn 0x%02x,0x%02x", SNES_OP_16BIT},
{"eor 0x%02x,x", SNES_OP_16BIT},
{"lsr 0x%02x,x", SNES_OP_16BIT},
{"eor [0x%02x],y", SNES_OP_16BIT},
{"cli", SNES_OP_8BIT},
{"eor 0x%04x,y", SNES_OP_24BIT},
{"phy", SNES_OP_8BIT},
{"tad", SNES_OP_8BIT},
{"jmp 0x%06x", SNES_OP_32BIT},
{"eor 0x%04x,x", SNES_OP_24BIT},
{"lsr 0x%04x,x", SNES_OP_24BIT},
{"eor 0x%06x,x", SNES_OP_32BIT},
{"rts", SNES_OP_8BIT},
{"adc (0x%02x,x)", SNES_OP_16BIT},
{"per 0x%04x", SNES_OP_24BIT},
{"adc 0x%02x,s", SNES_OP_16BIT},
{"stz 0x%02x", SNES_OP_16BIT},
{"adc 0x%02x", SNES_OP_16BIT},
{"ror 0x%02x", SNES_OP_16BIT},
{"adc [0x%02x]", SNES_OP_16BIT},
{"pla", SNES_OP_8BIT},
{"adc", SNES_OP_IMM_M},
{"ror a", SNES_OP_8BIT},
{"rtl", SNES_OP_8BIT},
{"jmp (0x%04x)", SNES_OP_24BIT},
{"adc 0x%04x", SNES_OP_24BIT},
{"ror 0x%04x", SNES_OP_24BIT},
{"adc 0x%06x", SNES_OP_32BIT},
{"bvs 0x%06x", SNES_OP_16BIT},
{"adc (0x%02x),y", SNES_OP_16BIT},
{"adc (0x%02x)", SNES_OP_16BIT},
{"adc (0x%02x,s),y", SNES_OP_16BIT},
{"stz 0x%02x,x", SNES_OP_16BIT},
{"adc 0x%02x,x", SNES_OP_16BIT},
{"ror 0x%02x,x", SNES_OP_16BIT},
{"adc [0x%02x],y", SNES_OP_16BIT},
{"sei", SNES_OP_8BIT},
{"adc 0x%04x,y", SNES_OP_24BIT},
{"ply", SNES_OP_8BIT},
{"tda", SNES_OP_8BIT},
{"jmp (0x%04x,x)", SNES_OP_24BIT},
{"adc 0x%04x,x", SNES_OP_24BIT},
{"ror 0x%04x,x", SNES_OP_24BIT},
{"adc 0x%06x,x", SNES_OP_32BIT},
{"bra 0x%06x", SNES_OP_16BIT},
{"sta (0x%02x,x)", SNES_OP_16BIT},
{"brl 0x%06x", SNES_OP_24BIT},
{"sta 0x%02x,s", SNES_OP_16BIT},
{"sty 0x%02x", SNES_OP_16BIT},
{"sta 0x%02x", SNES_OP_16BIT},
{"stx 0x%02x", SNES_OP_16BIT},
{"sta [0x%02x]", SNES_OP_16BIT},
{"dey", SNES_OP_8BIT},
{"bit", SNES_OP_IMM_M},
{"txa", SNES_OP_8BIT},
{"phb", SNES_OP_8BIT},
{"sty 0x%04x", SNES_OP_24BIT},
{"sta 0x%04x", SNES_OP_24BIT},
{"stx 0x%04x", SNES_OP_24BIT},
{"sta 0x%06x", SNES_OP_32BIT},
{"bcc 0x%06x", SNES_OP_16BIT},
{"sta (0x%02x),y", SNES_OP_16BIT},
{"sta (0x%02x)", SNES_OP_16BIT},
{"sta (0x%02x,s),y", SNES_OP_16BIT},
{"sty 0x%02x,x", SNES_OP_16BIT},
{"sta 0x%02x,x", SNES_OP_16BIT},
{"stx 0x%02x,y", SNES_OP_16BIT},
{"sta [0x%02x],y", SNES_OP_16BIT},
{"tya", SNES_OP_8BIT},
{"sta 0x%04x,y", SNES_OP_24BIT},
{"txs", SNES_OP_8BIT},
{"txy", SNES_OP_8BIT},
{"stz 0x%04x", SNES_OP_24BIT},
{"sta 0x%04x,x", SNES_OP_24BIT},
{"stz 0x%04x,x", SNES_OP_24BIT},
{"sta 0x%06x,x", SNES_OP_32BIT},
{"ldy", SNES_OP_IMM_X},
{"lda (0x%02x,x)", SNES_OP_16BIT},
{"ldx", SNES_OP_IMM_X},
{"lda 0x%02x,s", SNES_OP_16BIT},
{"ldy 0x%02x", SNES_OP_16BIT},
{"lda 0x%02x", SNES_OP_16BIT},
{"ldx 0x%02x", SNES_OP_16BIT},
{"lda [0x%02x]", SNES_OP_16BIT},
{"tay", SNES_OP_8BIT},
{"lda", SNES_OP_IMM_M},
{"tax", SNES_OP_8BIT},
{"plb", SNES_OP_8BIT},
{"ldy 0x%04x", SNES_OP_24BIT},
{"lda 0x%04x", SNES_OP_24BIT},
{"ldx 0x%04x", SNES_OP_24BIT},
{"lda 0x%06x", SNES_OP_32BIT},
{"bcs 0x%06x", SNES_OP_16BIT},
{"lda (0x%02x),y", SNES_OP_16BIT},
{"lda (0x%02x)", SNES_OP_16BIT},
{"lda (0x%02x,s),y", SNES_OP_16BIT},
{"ldy 0x%02x,x", SNES_OP_16BIT},
{"lda 0x%02x,x", SNES_OP_16BIT},
{"ldx 0x%02x,y", SNES_OP_16BIT},
{"lda [0x%02x],y", SNES_OP_16BIT},
{"clv", SNES_OP_8BIT},
{"lda 0x%04x,y", SNES_OP_24BIT},
{"tsx", SNES_OP_8BIT},
{"tyx", SNES_OP_8BIT},
{"ldy 0x%04x,x", SNES_OP_24BIT},
{"lda 0x%04x,x", SNES_OP_24BIT},
{"ldx 0x%04x,y", SNES_OP_24BIT},
{"lda 0x%06x,x", SNES_OP_32BIT},
{"cpy", SNES_OP_IMM_X},
{"cmp (0x%02x,x)", SNES_OP_16BIT},
{"rep #0x%02x", SNES_OP_16BIT},
{"cmp 0x%02x,s", SNES_OP_16BIT},
{"cpy 0x%02x", SNES_OP_16BIT},
{"cmp 0x%02x", SNES_OP_16BIT},
{"dec 0x%02x", SNES_OP_16BIT},
{"cmp [0x%02x]", SNES_OP_16BIT},
{"iny", SNES_OP_8BIT},
{"cmp", SNES_OP_IMM_M},
{"dex", SNES_OP_8BIT},
{"wai", SNES_OP_8BIT},
{"cpy 0x%04x", SNES_OP_24BIT},
{"cmp 0x%04x", SNES_OP_24BIT},
{"dec 0x%04x", SNES_OP_24BIT},
{"cmp 0x%06x", SNES_OP_32BIT},
{"bne 0x%06x", SNES_OP_16BIT},
{"cmp (0x%02x),y", SNES_OP_16BIT},
{"cmp (0x%02x)", SNES_OP_16BIT},
{"cmp (0x%02x,s),y", SNES_OP_16BIT},
{"pei (0x%02x)", SNES_OP_16BIT},
{"cmp 0x%02x,x", SNES_OP_16BIT},
{"dec 0x%02x,x", SNES_OP_16BIT},
{"cmp [0x%02x],y", SNES_OP_16BIT},
{"cld", SNES_OP_8BIT},
{"cmp 0x%04x,y", SNES_OP_24BIT},
{"phx", SNES_OP_8BIT},
{"stp", SNES_OP_8BIT},
{"jmp [0x%04x]", SNES_OP_24BIT},
{"cmp 0x%04x,x", SNES_OP_24BIT},
{"dec 0x%04x,x", SNES_OP_24BIT},
{"cmp 0x%06x,x", SNES_OP_32BIT},
{"cpx", SNES_OP_IMM_X},
{"sbc (0x%02x,x)", SNES_OP_16BIT},
{"sep #0x%02x", SNES_OP_16BIT},
{"sbc 0x%02x,s", SNES_OP_16BIT},
{"cpx 0x%02x", SNES_OP_16BIT},
{"sbc 0x%02x", SNES_OP_16BIT},
{"inc 0x%02x", SNES_OP_16BIT},
{"sbc [0x%02x]", SNES_OP_16BIT},
{"inx", SNES_OP_8BIT},
{"sbc", SNES_OP_IMM_M},
{"nop", SNES_OP_8BIT},
{"swa", SNES_OP_8BIT},
{"cpx 0x%04x", SNES_OP_24BIT},
{"sbc 0x%04x", SNES_OP_24BIT},
{"inc 0x%04x", SNES_OP_24BIT},
{"sbc 0x%06x", SNES_OP_32BIT},
{"beq 0x%06x", SNES_OP_16BIT},
{"sbc (0x%02x),y", SNES_OP_16BIT},
{"sbc (0x%02x)", SNES_OP_16BIT},
{"sbc (0x%02x,s),y", SNES_OP_16BIT},
{"pea 0x%04x", SNES_OP_24BIT},
{"sbc 0x%02x,x", SNES_OP_16BIT},
{"inc 0x%02x,x", SNES_OP_16BIT},
{"sbc [0x%02x],y", SNES_OP_16BIT},
{"sed", SNES_OP_8BIT},
{"sbc 0x%04x,y", SNES_OP_24BIT},
{"plx", SNES_OP_8BIT},
{"xce", SNES_OP_8BIT},
{"jsr (0x%04x,x)", SNES_OP_24BIT},
{"sbc 0x%04x,x", SNES_OP_24BIT},
{"inc 0x%04x,x", SNES_OP_24BIT},
{"sbc 0x%06x,x", SNES_OP_32BIT}
};
