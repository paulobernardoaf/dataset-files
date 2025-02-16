return strdup(
"=PC eip\n"
"=SP esp\n"
"=BP ebp\n"
"=A0 eax\n"
"=A1 ebx\n"
"=A2 ecx\n"
"=A3 edi\n"
"drx@gpr dr0 .32 4 0\n"
"drx@gpr dr1 .32 8 0\n"
"drx@gpr dr2 .32 12 0\n"
"drx@gpr dr3 .32 16 0\n"
"drx@gpr dr6 .32 20 0\n"
"drx@gpr dr7 .32 24 0\n"

"fpu@gpr ctw .32 28 0\n"
"fpu@gpr stw .32 32 0\n"
"fpu@gpr tag .32 36 0\n"
"fpu@gpr ero .32 40 0\n"
"fpu@gpr ers .32 44 0\n"
"fpu@gpr dao .32 48 0\n"
"fpu@gpr das .32 52 0\n"
"fpu@gpr st0 .80 56 0\n"
"fpu@gpr st1 .80 66 0\n"
"fpu@gpr st2 .80 76 0\n"
"fpu@gpr st3 .80 86 0\n"
"fpu@gpr st4 .80 96 0\n"
"fpu@gpr st5 .80 106 0\n"
"fpu@gpr st6 .80 116 0\n"
"fpu@gpr st7 .80 126 0\n"
"fpu@gpr spare .32 136 0\n"
"mmx@gpr mm0 .64 56 0\n"
"mmx@gpr mm1 .64 66 0\n"
"mmx@gpr mm2 .64 76 0\n"
"mmx@gpr mm3 .64 86 0\n"
"mmx@gpr mm4 .64 96 0\n"
"mmx@gpr mm5 .64 106 0\n"
"mmx@gpr mm6 .64 116 0\n"
"mmx@gpr mm7 .64 126 0\n"

"gpr@gpr edi .32 156 0\n"
"gpr@gpr esi .32 160 0\n"
"gpr@gpr ebx .32 164 0\n"
"gpr@gpr edx .32 168 0\n"
"gpr@gpr ecx .32 172 0\n"
"gpr@gpr eax .32 176 0\n"
"gpr@gpr ebp .32 180 0\n"
"gpr@gpr eip .32 184 0\n"
"gpr@gpr eflags .32 192 0 c1p.a.zstido.n.rv\n" 
"gpr@gpr esp .32 196 0\n"
"seg@gpr gs .32 140 0\n"
"seg@gpr fs .32 144 0\n"
"seg@gpr es .32 148 0\n"
"seg@gpr ds .32 152 0\n"
"seg@gpr cs .32 188 0\n"
"seg@gpr ss .32 200 0\n"
"gpr@gpr cf .1 .1536 0 carry\n"
"gpr@gpr pf .1 .1538 0 parity\n"
"gpr@gpr af .1 .1540 0 adjust\n"
"gpr@gpr zf .1 .1542 0 zero\n"
"gpr@gpr sf .1 .1543 0 sign\n"
"gpr@gpr tf .1 .1544 0 trap\n"
"gpr@gpr if .1 .1545 0 interrupt\n"
"gpr@gpr df .1 .1546 0 direction\n"
"gpr@gpr of .1 .1547 0 overflow\n"

"xmm@gpr xmm0 .128 364 0\n"
"xmm@gpr xmm1 .128 380 0\n"
"xmm@gpr xmm2 .128 396 0\n"
"xmm@gpr xmm3 .128 412 0\n"
"xmm@gpr xmm4 .128 428 0\n"
"xmm@gpr xmm5 .128 444 0\n"
"xmm@gpr xmm6 .128 460 0\n"
"xmm@gpr xmm7 .128 476 0\n"



);
