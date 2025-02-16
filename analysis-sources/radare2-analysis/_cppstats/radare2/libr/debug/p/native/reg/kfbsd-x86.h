return strdup (
"=PC eip\n"
"=SP esp\n"
"=BP ebp\n"
"=A0 eax\n"
"=A1 ebx\n"
"=A2 ecx\n"
"=A3 edi\n"
"seg fs .32 0 0\n"
"seg es .32 4 0\n"
"seg ds .32 8 0\n"
"gpr edi .32 12 0\n"
"gpr di .16 12 0\n"
"gpr esi .32 16 0\n"
"gpr si .16 16 0\n"
"gpr ebp .32 20 0\n"
"gpr bp .16 20 0\n"
"gpr isp .32 24 0\n"
"gpr ebx .32 28 0\n"
"gpr bx .16 28 0\n"
"gpr bh .8 29 0\n"
"gpr bl .8 28 0\n"
"gpr edx .32 32 0\n"
"gpr dx .16 32 0\n"
"gpr dh .8 33 0\n"
"gpr dl .8 32 0\n"
"gpr ecx .32 36 0\n"
"gpr cx .16 36 0\n"
"gpr ch .8 37 0\n"
"gpr cl .8 36 0\n"
"gpr eax .32 40 0\n"
"gpr ax .16 40 0\n"
"gpr ah .8 41 0\n"
"gpr al .8 40 0\n"
"gpr trapno .32 44 0\n"
"gpr err .32 48 0\n"
"gpr eip .32 52 0\n"
"gpr ip .16 52 0\n"
"seg cs .32 56 0\n"
"gpr eflags .32 60 0 c1p.a.zstido.n.rv\n"
"gpr cf .1 .480 0 carry\n"
"gpr pf .1 .482 0 parity\n"
"gpr af .1 .484 0 adjust\n"
"gpr zf .1 .486 0 zero\n"
"gpr sf .1 .487 0 sign\n"
"gpr tf .1 .488 0 trap\n"
"gpr if .1 .489 0 interrupt\n"
"gpr df .1 .490 0 direction\n"
"gpr of .1 .491 0 overflow\n"
"gpr esp .32 64 0\n"
"gpr sp .16 64 0\n"
"seg ss .32 68 0\n"
"seg gs .32 72 0\n"
);
