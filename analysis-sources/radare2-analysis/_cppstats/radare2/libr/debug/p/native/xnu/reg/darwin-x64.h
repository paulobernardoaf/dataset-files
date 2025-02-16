return strdup (
"=PC rip\n"
"=SP rsp\n"
"=BP rbp\n"
"=SN rax\n"
"=R0 rax\n"
"=A0 rdi\n"
"=A1 rsi\n"
"=A2 rdx\n"
"=A3 rcx\n"
"=A4 r8\n"
"=A5 r9\n"
"=ZF zf\n"
"=OF of\n"
"=SF sf\n"
"=CF cf\n"
"gpr rax .64 0 0\n"
"gpr eax .32 0 0\n"
"gpr ax .16 0 0\n"
"gpr ah .8 1 0\n"
"gpr al .8 0 0\n"
"gpr rbx .64 8 0\n"
"gpr ebx .32 8 0\n"
"gpr bx .16 8 0\n"
"gpr bh .8 9 0\n"
"gpr bl .8 8 0\n"
"gpr rcx .64 16 0\n"
"gpr ecx .32 16 0\n"
"gpr cx .16 16 0\n"
"gpr ch .8 17 0\n"
"gpr cl .8 16 0\n"
"gpr rdx .64 24 0\n"
"gpr edx .32 24 0\n"
"gpr dx .16 24 0\n"
"gpr dh .8 25 0\n"
"gpr dl .8 24 0\n"
"gpr rdi .64 32 0\n"
"gpr edi .32 32 0\n"
"gpr di .16 32 0\n"
"gpr dil .8 32 0\n"
"gpr rsi .64 40 0\n"
"gpr esi .32 40 0\n"
"gpr si .16 40 0\n"
"gpr sil .8 40 0\n"
"gpr rbp .64 48 0\n"
"gpr ebp .32 48 0\n"
"gpr bp .16 48 0\n"
"gpr bpl .8 48 0\n"
"gpr rsp .64 56 0\n"
"gpr esp .32 56 0\n"
"gpr sp .16 56 0\n"
"gpr spl .8 56 0\n"
"gpr r8 .64 64 0\n"
"gpr r8d .32 64 0\n"
"gpr r8w .16 64 0\n"
"gpr r8b .8 64 0\n"
"gpr r9 .64 72 0\n"
"gpr r9d .32 72 0\n"
"gpr r9w .16 72 0\n"
"gpr r9b .8 72 0\n"
"gpr r10 .64 80 0\n"
"gpr r10d .32 80 0\n"
"gpr r10w .16 80 0\n"
"gpr r10b .8 80 0\n"
"gpr r11 .64 88 0\n"
"gpr r11d .32 88 0\n"
"gpr r11w .16 88 0\n"
"gpr r11b .8 88 0\n"
"gpr r12 .64 96 0\n"
"gpr r12d .32 96 0\n"
"gpr r12w .16 96 0\n"
"gpr r12b .8 96 0\n"
"gpr r13 .64 104 0\n"
"gpr r13d .32 104 0\n"
"gpr r13w .16 104 0\n"
"gpr r13b .8 104 0\n"
"gpr r14 .64 112 0\n"
"gpr r14d .32 112 0\n"
"gpr r14w .16 112 0\n"
"gpr r14b .8 112 0\n"
"gpr r15 .64 120 0\n"
"gpr r15d .32 120 0\n"
"gpr r15w .16 120 0\n"
"gpr r15b .8 120 0\n"
"gpr rip .64 128 0\n"
"gpr eflags .32 136 0 c1p.a.zstido.n.rv\n"
"gpr rflags .64 136 0 c1p.a.zstido.n.rv\n"
"gpr cf .1 .1088 0 carry\n"
"gpr pf .1 .1090 0 parity\n"
"gpr af .1 .1092 0 adjust\n"
"gpr zf .1 .1094 0 zero\n"
"gpr sf .1 .1095 0 sign\n"
"gpr tf .1 .1096 0 trap\n"
"gpr if .1 .1097 0 interrupt\n"
"gpr df .1 .1098 0 direction\n"
"gpr of .1 .1099 0 overflow\n"
"seg@gpr cs .64 136 0\n"
"seg@gpr fs .64 144 0\n"
"seg@gpr gs .64 152 0\n"
"drx dr0 .64 0 0\n"
"drx dr1 .64 8 0\n"
"drx dr2 .64 16 0\n"
"drx dr3 .64 24 0\n"
"drx dr4 .64 32 0\n"
"drx dr5 .64 40 0\n"
"drx dr6 .64 48 0\n"
"drx dr7 .64 56 0\n"
);
