return strdup (
"=PC eip\n"
"=SP esp\n"
"=BP ebp\n"
"=A0 eax\n"
"=A1 ebx\n"
"=A2 ecx\n"
"=A3 edi\n"
"gpr eax .32 0 0\n"
"gpr ax .16 0 0\n"
"gpr ah .8 1 0\n"
"gpr al .8 0 0\n"
"gpr ecx .32 4 0\n"
"gpr cx .16 4 0\n"
"gpr ch .8 5 0\n"
"gpr cl .8 4 0\n"
"gpr edx .32 8 0\n"
"gpr dx .16 8 0\n"
"gpr dh .8 9 0\n"
"gpr dl .8 8 0\n"
"gpr ebx .32 12 0\n"
"gpr bx .16 12 0\n"
"gpr bh .8 13 0\n"
"gpr bl .8 12 0\n"
"gpr esp .32 16 0\n"
"gpr sp .16 16 0\n"
"gpr ebp .32 20 0\n"
"gpr bp .16 20 0\n"
"gpr esi .32 24 0\n"
"gpr si .16 24 0\n"
"gpr edi .32 28 0\n"
"gpr di .16 28 0\n"
"gpr eip .32 32 0\n"
"gpr ip .16 32 0\n"
"gpr eflags .32 36 0 c1p.a.zstido.n.rv\n"
"gpr cf .1 .288 0 carry\n"
"gpr pf .1 .290 0 parity\n"
"gpr af .1 .292 0 adjust\n"
"gpr zf .1 .294 0 zero\n"
"gpr sf .1 .295 0 sign\n"
"gpr tf .1 .296 0 trap\n"
"gpr if .1 .297 0 interrupt\n"
"gpr df .1 .298 0 direction\n"
"gpr of .1 .299 0 overflow\n"
"seg cs .32 40 0\n"
"seg ss .32 44 0\n"
"seg ds .32 48 0\n"
"seg es .32 52 0\n"
"seg fs .32 56 0\n"
"seg gs .32 60 0\n"

);
