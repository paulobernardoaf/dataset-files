#if HAVE_INLINE_ASM && HAVE_FAST_CMOV && HAVE_6REGS
#define vp56_rac_get_prob vp56_rac_get_prob
static av_always_inline int vp56_rac_get_prob(VP56RangeCoder *c, uint8_t prob)
{
unsigned int code_word = vp56_rac_renorm(c);
unsigned int low = 1 + (((c->high - 1) * prob) >> 8);
unsigned int low_shift = low << 16;
int bit = 0;
c->code_word = code_word;
__asm__(
"subl %4, %1 \n\t"
"subl %3, %2 \n\t"
"setae %b0 \n\t"
"cmovb %4, %1 \n\t"
"cmovb %5, %2 \n\t"
: "+q"(bit), "+&r"(c->high), "+&r"(c->code_word)
: "r"(low_shift), "r"(low), "r"(code_word)
);
return bit;
}
#endif
