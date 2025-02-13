return strdup (
"=PC rip\n"
"=SP rsp\n"
"=BP rbp\n"
"=R0 rax\n"
"=A0 rcx\n"
"=A1 rdx\n"
"=A2 r8\n"
"=A3 r9\n"
"gpr mxcsr 4 0x034 0 \n"
"seg cs 2 0x038 0 \n"
"seg ds 2 0x03A 0 \n"
"seg es 2 0x03C 0 \n"
"seg fs 2 0x03E 0 \n"
"seg gs 2 0x040 0 \n"
"seg ss 2 0x042 0 \n"
"gpr eflags 4 0x044 0 c1p.a.zstido.n.rv\n"
"drx dr0 8 0x048 0 \n"
"drx dr1 8 0x050 0 \n"
"drx dr2 8 0x058 0 \n"
"drx dr3 8 0x060 0 \n"
"drx dr6 8 0x068 0 \n"
"drx dr7 8 0x070 0 \n"
"gpr rax 8 0x078 0 \n"
"gpr eax 4 0x078 0 \n"
"gpr ax 2 0x078 0 \n"
"gpr al 1 0x078 0 \n"
"gpr rcx 8 0x080 0 \n"
"gpr ecx 4 0x080 0 \n"
"gpr cx 2 0x080 0 \n"
"gpr cl 1 0x078 0 \n"
"gpr rdx 8 0x088 0 \n"
"gpr edx 4 0x088 0 \n"
"gpr dx 2 0x088 0 \n"
"gpr dl 1 0x088 0 \n"
"gpr rbx 8 0x090 0 \n"
"gpr ebx 4 0x090 0 \n"
"gpr bx 2 0x090 0 \n"
"gpr bl 1 0x090 0 \n"
"gpr rsp 8 0x098 0 \n"
"gpr esp 4 0x098 0 \n"
"gpr sp 2 0x098 0 \n"
"gpr spl 1 0x098 0 \n"
"gpr rbp 8 0x0A0 0 \n"
"gpr ebp 4 0x0A0 0 \n"
"gpr bp 2 0x0A0 0 \n"
"gpr bpl 1 0x0A0 0 \n"
"gpr rsi 8 0x0A8 0 \n"
"gpr esi 4 0x0A8 0 \n"
"gpr si 2 0x0A8 0 \n"
"gpr sil 1 0x0A8 0 \n"
"gpr rdi 8 0x0B0 0 \n"
"gpr edi 4 0x0B0 0 \n"
"gpr di 2 0x0B0 0 \n"
"gpr dil 1 0x0B0 0 \n"
"gpr r8 8 0x0B8 0 \n"
"gpr r8d 4 0x0B8 0 \n"
"gpr r8w 2 0x0B8 0 \n"
"gpr r8b 1 0x0B8 0 \n"
"gpr r9 8 0x0C0 0 \n"
"gpr r9d 4 0x0C0 0 \n"
"gpr r9w 2 0x0C0 0 \n"
"gpr r9b 1 0x0C0 0 \n"
"gpr r10 8 0x0C8 0 \n"
"gpr r10d 4 0x0C8 0 \n"
"gpr r10w 2 0x0C8 0 \n"
"gpr r10b 1 0x0C8 0 \n"
"gpr r11 8 0x0D0 0 \n"
"gpr r11d 4 0x0D0 0 \n"
"gpr r11w 2 0x0D0 0 \n"
"gpr r11b 1 0x0D0 0 \n"
"gpr r12 8 0x0D8 0 \n"
"gpr r12d 4 0x0D8 0 \n"
"gpr r12w 2 0x0D8 0 \n"
"gpr r12b 1 0x0D8 0 \n"
"gpr r13 8 0x0E0 0 \n"
"gpr r13d 4 0x0E0 0 \n"
"gpr r13w 2 0x0E0 0 \n"
"gpr r13b 1 0x0E0 0 \n"
"gpr r14 8 0x0E8 0 \n"
"gpr r14d 4 0x0E8 0 \n"
"gpr r14w 2 0x0E8 0 \n"
"gpr r14b 1 0x0E8 0 \n"
"gpr r15 8 0x0F0 0 \n"
"gpr r15d 4 0x0F0 0 \n"
"gpr r15w 2 0x0F0 0 \n"
"gpr r15b 1 0x0F0 0 \n"
"gpr rip 8 0x0F8 0 \n"
"gpr cf .1 .544 0 carry\n"
"gpr pf .1 .546 0 parity\n"
"gpr af .1 .548 0 adjust\n"
"gpr zf .1 .550 0 zero\n"
"gpr sf .1 .551 0 sign\n"
"gpr tf .1 .552 0 trap\n"
"gpr if .1 .553 0 interrupt\n"
"gpr df .1 .554 0 direction\n"
"gpr of .1 .555 0 overflow\n"
);
