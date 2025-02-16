
return strdup (
"=PC rip\n"
"=SP rsp\n"
"=BP rbp\n"
"=R0 rax\n"
"=A0 rdi\n"
"=A1 rsi\n"
"=A2 rdx\n"
"=A3 rcx\n"
"=A4 r8\n"
"=A5 r9\n"
"=SN orax\n"
"#no profile defined for x86-64\n"
"gpr rax .64 80 0\n"
"gpr eax .32 80 0\n"
"gpr ax .16 80 0\n"
"gpr ah .8 81 0\n"
"gpr al .8 80 0\n"
"gpr rbx .64 40 0\n"
"gpr ebx .32 40 0\n"
"gpr bx .16 40 0\n"
"gpr bh .8 41 0\n"
"gpr bl .8 40 0\n"
"gpr rcx .64 88 0\n"
"gpr ecx .32 88 0\n"
"gpr cx .16 88 0\n"
"gpr ch .8 89 0\n"
"gpr cl .8 88 0\n"
"gpr rdx .64 96 0\n"
"gpr edx .32 96 0\n"
"gpr dx .16 96 0\n"
"gpr dh .8 97 0\n"
"gpr dl .8 96 0\n"
"gpr r8 .64 72 0\n"
"gpr r8d .32 72 0\n"
"gpr r8w .16 72 0\n"
"gpr r8b .8 72 0\n"
"gpr r9 .64 64 0\n"
"gpr r9d .32 64 0\n"
"gpr r9w .16 64 0\n"
"gpr r9b .8 64 0\n"
"gpr r10 .64 56 0\n"
"gpr r10d .32 56 0\n"
"gpr r10w .16 56 0\n"
"gpr r10b .8 56 0\n"
"gpr r11 .64 48 0\n"
"gpr r11d .32 48 0\n"
"gpr r11w .16 48 0\n"
"gpr r11b .8 48 0\n"
"gpr r12 .64 24 0\n"
"gpr r12d .32 24 0\n"
"gpr r12w .16 24 0\n"
"gpr r12b .8 24 0\n"
"gpr r13 .64 16 0\n"
"gpr r13d .32 16 0\n"
"gpr r13w .16 16 0\n"
"gpr r13b .8 16 0\n"
"gpr r14 .64 8 0\n"
"gpr r14d .32 8 0\n"
"gpr r14w .16 8 0\n"
"gpr r14b .8 8 0\n"
"gpr r15 .64 0 0\n"
"gpr r15d .32 0 0\n"
"gpr r15w .16 0 0\n"
"gpr r15b .8 0 0\n"
"gpr rsi .64 104 0\n"
"gpr esi .32 104 0\n"
"gpr si .16 104 0\n"
"gpr sil .8 104 0\n"
"gpr rdi .64 112 0\n"
"gpr edi .32 112 0\n"
"gpr di .16 112 0\n"
"gpr dil .8 112 0\n"
"gpr rsp .64 152 0\n"
"gpr esp .32 152 0\n"
"gpr sp .16 152 0\n"
"gpr spl .8 152 0\n"
"gpr rbp .64 32 0\n"
"gpr ebp .32 32 0\n"
"gpr bp .16 32 0\n"
"gpr bpl .8 32 0\n"
"gpr rip .64 128 0\n"
"seg@gpr cs .64 136 0\n"
"gpr rflags .64 144 0 c1p.a.zstido.n.rv\n"
"gpr eflags .32 144 0 c1p.a.zstido.n.rv\n"
"gpr cf .1 .1152 0 carry\n"
"gpr pf .1 .1154 0 parity\n"
"gpr af .1 .1156 0 adjust\n"
"gpr zf .1 .1158 0 zero\n"
"gpr sf .1 .1159 0 sign\n"
"gpr tf .1 .1160 0 trap\n"
"gpr if .1 .1161 0 interrupt\n"
"gpr df .1 .1162 0 direction\n"
"gpr of .1 .1163 0 overflow\n"
"gpr orax .64 120 0\n"

"seg@gpr ss .64 160 0\n"
"seg@gpr fs_base .64 168 0\n"
"seg@gpr gs_base .64 176 0\n"
"seg@gpr ds .64 184 0\n"
"seg@gpr es .64 192 0\n"
"seg@gpr fs .64 200 0\n"
"seg@gpr gs .64 208 0\n"
"drx dr0 .64 0 0\n"
"drx dr1 .64 8 0\n"
"drx dr2 .64 16 0\n"
"drx dr3 .64 24 0\n"


"drx dr6 .64 48 0\n"
"drx dr7 .64 56 0\n"
















"fpu cwd .16 0 0\n"
"fpu swd .16 2 0\n"
"fpu ftw .16 4 0\n"
"fpu fop .16 6 0\n"
"fpu frip .64 8 0\n"
"fpu frdp .64 16 0\n"
"fpu mxcsr .32 24 0\n"
"fpu mxcr_mask .32 28 0\n"

"fpu st0 .80 32 0\n"
"fpu st1 .80 48 0\n"
"fpu st2 .80 64 0\n"
"fpu st3 .80 80 0\n"
"fpu st4 .80 96 0\n"
"fpu st5 .80 112 0\n"
"fpu st6 .80 128 0\n"
"fpu st7 .80 144 0\n"

"mmx@fpu mm0 .64 32 8\n"
"mmx@fpu mm1 .64 48 8\n"
"mmx@fpu mm2 .64 64 8\n"
"mmx@fpu mm3 .64 80 8\n"
"mmx@fpu mm4 .64 96 8\n"
"mmx@fpu mm5 .64 112 8\n"
"mmx@fpu mm6 .64 128 8\n"
"mmx@fpu mm7 .64 144 8\n"

"xmm@fpu xmm0 .128 160 16\n"
"fpu xmm0h .64 160 8\n"
"fpu xmm0l .64 168 8\n"
"xmm@fpu xmm1 .128 176 16\n"
"fpu xmm1h .64 176 8\n"
"fpu xmm1l .64 184 8\n"
"xmm@fpu xmm2 .128 192 16\n"
"fpu xmm2h .64 192 8\n"
"fpu xmm2l .64 200 8\n"
"xmm@fpu xmm3 .128 208 16\n"
"fpu xmm3h .64 208 8\n"
"fpu xmm3l .64 216 8\n"
"xmm@fpu xmm4 .128 224 16\n"
"fpu xmm4h .64 224 8\n"
"fpu xmm4l .64 232 8\n"
"xmm@fpu xmm5 .128 240 16\n"
"fpu xmm5h .64 240 8\n"
"fpu xmm5l .64 248 8\n"
"xmm@fpu xmm6 .128 256 16\n"
"fpu xmm6h .64 256 8\n"
"fpu xmm6l .64 264 8\n"
"xmm@fpu xmm7 .128 272 16\n"
"fpu xmm7h .64 272 8\n"
"fpu xmm7l .64 280 8\n"
"xmm@fpu xmm8 .128 288 16\n"
"fpu xmm8h .64 288 8\n"
"fpu xmm8l .64 296 8\n"
"xmm@fpu xmm9 .128 304 16\n"
"fpu xmm9h .64 304 8\n"
"fpu xmm9l .64 312 8\n"
"xmm@fpu xmm10 .128 320 16\n"
"fpu xmm10h .64 320 8\n"
"fpu xmm10l .64 328 8\n"
"xmm@fpu xmm11 .128 336 16\n"
"fpu xmm11h .64 336 8\n"
"fpu xmm11l .64 344 8\n"
"xmm@fpu xmm12 .128 352 16\n"
"fpu xmm12h .64 352 8\n"
"fpu xmm12l .64 360 8\n"
"xmm@fpu xmm13 .128 368 16\n"
"fpu xmm13h .64 368 8\n"
"fpu xmm13l .64 376 8\n"
"xmm@fpu xmm14 .128 384 16\n"
"fpu xmm14h .64 384 8\n"
"fpu xmm14l .64 392 8\n"
"xmm@fpu xmm15 .128 400 16\n"
"fpu xmm15h .64 400 8\n"
"fpu x64 .64 416 0\n"

"ymm ymm0 .256 0 32\n"
"ymm ymm1 .256 32 32\n"
"ymm ymm2 .256 64 32\n"
"ymm ymm3 .256 96 32\n"
"ymm ymm4 .256 128 32\n"
"ymm ymm5 .256 160 32\n"
"ymm ymm6 .256 192 32\n"
"ymm ymm7 .256 224 32\n"
"ymm ymm8 .256 256 32\n"
"ymm ymm9 .256 288 32\n"
"ymm ymm10 .256 320 32\n"
"ymm ymm11 .256 352 32\n"
"ymm ymm12 .256 384 32\n"
"ymm ymm13 .256 416 32\n"
"ymm ymm14 .256 448 32\n"
"ymm ymm15 .256 480 32\n"

);
